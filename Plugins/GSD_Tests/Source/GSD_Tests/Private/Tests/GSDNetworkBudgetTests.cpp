// Copyright Bret Bouchard. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Subsystems/GSDNetworkBudgetSubsystem.h"
#include "DataAssets/GSDNetworkBudgetConfig.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDNetworkBudgetTrackingTest,
    "GSD.Network.Budget.Tracking",
    EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ContextMask)

bool FGSDNetworkBudgetTrackingTest::RunTest(const FString& Parameters)
{
    // Create test config
    UGSDNetworkBudgetConfig* Config = NewObject<UGSDNetworkBudgetConfig>();
    Config->TotalBitsPerSecond = 10000;  // 10 kbps for testing
    Config->CategoryAllocations.Add(EGSDBudgetCategory::Crowd, 0.5f);  // 50% = 5000 bits

    // Create subsystem
    UGSDNetworkBudgetSubsystem* Subsystem = NewObject<UGSDNetworkBudgetSubsystem>();
    Subsystem->SetConfig(Config);

    // Test 1: Track replication
    Subsystem->TrackReplication(EGSDBudgetCategory::Crowd, 1000);
    TestEqual("Crowd usage tracked", Subsystem->GetCurrentBandwidthUsage(EGSDBudgetCategory::Crowd), 1000.0f);

    // Test 2: Track more replication
    Subsystem->TrackReplication(EGSDBudgetCategory::Crowd, 500);
    TestEqual("Crowd usage accumulated", Subsystem->GetCurrentBandwidthUsage(EGSDBudgetCategory::Crowd), 1500.0f);

    // Test 3: Check total usage
    Subsystem->TrackReplication(EGSDBudgetCategory::Vehicle, 200);
    TestEqual("Total usage correct", Subsystem->GetTotalBandwidthUsage(), 1700.0f);

    // Test 4: Check remaining budget
    TestEqual("Remaining budget correct", Subsystem->GetRemainingBudget(EGSDBudgetCategory::Crowd), 3500);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDNetworkBudgetEnforcementTest,
    "GSD.Network.Budget.Enforcement",
    EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ContextMask)

bool FGSDNetworkBudgetEnforcementTest::RunTest(const FString& Parameters)
{
    // Create test config with small budget
    UGSDNetworkBudgetConfig* Config = NewObject<UGSDNetworkBudgetConfig>();
    Config->TotalBitsPerSecond = 1000;  // 1 kbps for testing
    Config->CategoryAllocations.Add(EGSDBudgetCategory::Crowd, 1.0f);  // 100% = 1000 bits

    // Create subsystem
    UGSDNetworkBudgetSubsystem* Subsystem = NewObject<UGSDNetworkBudgetSubsystem>();
    Subsystem->SetConfig(Config);

    // Test 1: Can replicate under budget
    TestTrue("Can replicate under budget", Subsystem->CanReplicateThisFrame(EGSDBudgetCategory::Crowd, 0));

    // Test 2: Exhaust budget
    Subsystem->TrackReplication(EGSDBudgetCategory::Crowd, 2000);  // Exceeds 1000 bit budget

    // Test 3: Should be blocked
    TestFalse("Cannot replicate over budget", Subsystem->CanReplicateThisFrame(EGSDBudgetCategory::Crowd, 0));

    // Test 4: Reset counters
    Subsystem->ResetFrameCounters();

    // Test 5: Should be allowed again
    TestTrue("Can replicate after reset", Subsystem->CanReplicateThisFrame(EGSDBudgetCategory::Crowd, 0));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDNetworkBudgetLODTest,
    "GSD.Network.Budget.LOD",
    EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ContextMask)

bool FGSDNetworkBudgetLODTest::RunTest(const FString& Parameters)
{
    // Create test config
    UGSDNetworkBudgetConfig* Config = NewObject<UGSDNetworkBudgetConfig>();
    Config->TotalBitsPerSecond = 100000;
    Config->CategoryAllocations.Add(EGSDBudgetCategory::Crowd, 1.0f);

    // Set up LOD configs with low limits for testing
    Config->LODConfigs.Empty();
    FGSDLODReplicationConfig LOD0;
    LOD0.MaxEntitiesPerFrame = 2;  // Only 2 entities per frame at LOD 0
    Config->LODConfigs.Add(LOD0);

    // Create subsystem
    UGSDNetworkBudgetSubsystem* Subsystem = NewObject<UGSDNetworkBudgetSubsystem>();
    Subsystem->SetConfig(Config);

    // Test 1: First entity allowed
    TestTrue("First LOD 0 entity allowed", Subsystem->CanReplicateThisFrame(EGSDBudgetCategory::Crowd, 0));

    // Test 2: Second entity allowed
    TestTrue("Second LOD 0 entity allowed", Subsystem->CanReplicateThisFrame(EGSDBudgetCategory::Crowd, 0));

    // Test 3: Third entity blocked (exceeds MaxEntitiesPerFrame)
    TestFalse("Third LOD 0 entity blocked", Subsystem->CanReplicateThisFrame(EGSDBudgetCategory::Crowd, 0));

    // Test 4: Reset and try again
    Subsystem->ResetFrameCounters();
    TestTrue("Can replicate after reset", Subsystem->CanReplicateThisFrame(EGSDBudgetCategory::Crowd, 0));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDNetworkBudgetConfigTest,
    "GSD.Network.Budget.Config",
    EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ContextMask)

bool FGSDNetworkBudgetConfigTest::RunTest(const FString& Parameters)
{
    // Create config
    UGSDNetworkBudgetConfig* Config = NewObject<UGSDNetworkBudgetConfig>();

    // Test 1: Default allocations after PostLoad
    Config->PostLoad();
    TestEqual("Crowd allocation", Config->CategoryAllocations[EGSDBudgetCategory::Crowd], 0.3f);
    TestEqual("Vehicle allocation", Config->CategoryAllocations[EGSDBudgetCategory::Vehicle], 0.2f);
    TestEqual("Player allocation", Config->CategoryAllocations[EGSDBudgetCategory::Player], 0.35f);

    // Test 2: Category budget calculation
    Config->TotalBitsPerSecond = 100000;  // 100 kbps
    TestEqual("Crowd budget", Config->GetCategoryBudget(EGSDBudgetCategory::Crowd), 30000);
    TestEqual("Vehicle budget", Config->GetCategoryBudget(EGSDBudgetCategory::Vehicle), 20000);

    // Test 3: LOD configs after PostLoad
    TestEqual("LOD configs count", Config->LODConfigs.Num(), 4);
    TestEqual("LOD 0 frequency", Config->LODConfigs[0].UpdateFrequency, 60.0f);
    TestEqual("LOD 0 max entities", Config->LODConfigs[0].MaxEntitiesPerFrame, 100);
    TestEqual("LOD 3 frequency", Config->LODConfigs[3].UpdateFrequency, 2.0f);

    // Test 4: GetLODConfig with valid index
    const FGSDLODReplicationConfig& LOD1 = Config->GetLODConfig(1);
    TestEqual("LOD 1 frequency correct", LOD1.UpdateFrequency, 30.0f);

    // Test 5: GetLODConfig with invalid index (should return default)
    const FGSDLODReplicationConfig& InvalidLOD = Config->GetLODConfig(10);
    TestEqual("Invalid LOD returns default", InvalidLOD.UpdateFrequency, 30.0f);

    return true;
}
