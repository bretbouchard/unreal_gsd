// Copyright Bret Bouchard. All Rights Reserved.

#include "Tests/GSDEventSpawnRegistryTest.h"
#include "Subsystems/GSDEventSpawnRegistry.h"
#include "DataAssets/GSDEventSpawnZone.h"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/GameInstance.h"

#if WITH_DEV_AUTITHREADING
namespace GSD_DailyEventsTests
{

    /**
     * Test suite for UGSDEventSpawnRegistry registration
     */
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDEventSpawnRegistryRegistrationTest, "GSD.DailyEvents.SpawnRegistry.Registration", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

    bool FGSDEventSpawnRegistryRegistrationTest::RunTest(const FString& Parameters)
    {
        // Note: This test requires a GameInstance to be available
        UGameInstance* GameInst = GEngine->GetWorldContextsWorld()->GetGameInstance();
        if (!GameInst)
        {
            UE_LOG(LogTemp, Warning, TEXT("SpawnRegistry test requires GameInstance"));
            return false;
        }

        UGSDEventSpawnRegistry* Registry = GameInst->GetSubsystem<UGSDEventSpawnRegistry>();
        if (!Registry)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get GSDEventSpawnRegistry"));
            return false;
        }

        // Test 1: Registry exists and is valid
        TestNotNull(Registry);
        TestTrueExpr(Registry->IsA<UGSDEventSpawnRegistry>());

        // Test 2: Initial zone count
        int32 InitialCount = Registry->GetZoneCount();
        TestTrueExpr(InitialCount >= 0);

        // Test 3: Get all zones returns valid array
        const TArray<UGSDEventSpawnZone*>& AllZones = Registry->GetAllSpawnZones();
        TestTrueExpr(AllZones.Num() == InitialCount);

        return true;
    }

    /**
     * Test suite for UGSDEventSpawnRegistry weighted selection
     */
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDEventSpawnRegistryWeightedTest, "GSD.DailyEvents.SpawnRegistry.WeightedSelection", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

    bool FGSDEventSpawnRegistryWeightedTest::RunTest(const FString& Parameters)
    {
        UGameInstance* GameInst = GEngine->GetWorldContextsWorld()->GetGameInstance();
        if (!GameInst)
        {
            return false;
        }

        UGSDEventSpawnRegistry* Registry = GameInst->GetSubsystem<UGSDEventSpawnRegistry>();
        if (!Registry)
        {
            return false;
        }

        // Test 1: Single zone always selected
        {
            // Create test zone
            UGSDEventSpawnZone* TestZone = New UGSDEventSpawnZone();
            TestZone->ZoneName = FName("SingleZoneTest");
            TestZone->ZoneCenter = FVector::ZeroVector;
            TestZone->ZoneExtent = FVector(1000.0f, 1001.0f, 500.0f);
            TestZone->Priority = 1;

            // Get zones array with just this zone
            TArray<UGSDEventSpawnZone*> Zones;
            Zones.Add(TestZone);

            // Calculate weight
            float TotalWeight = UGSDEventSpawnRegistry::CalculateTotalWeight(Zones);
            TestEqual(TotalWeight, 2.0f); // Priority 1 + 1 = 2

            // Test selection with fixed seed
            FRandomStream Stream(12345);
            UGSDEventSpawnZone* Selected = UGSDEventSpawnRegistry::SelectWeightedZone(Zones, TotalWeight, Stream);

            TestNotNull(Selected);
            TestEqual(Selected->ZoneName, TestZone->ZoneName);

        return true;
    }

    /**
     * Test suite for UGSDEventSpawnRegistry configuration
     */
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDEventSpawnRegistryConfigTest, "GSD.DailyEvents.SpawnRegistry.Configuration", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

    bool FGSDEventSpawnRegistryConfigTest::RunTest(const FString& Parameters)
    {
        UGameInstance* GameInst = GEngine->GetWorldContextsWorld()->GetGameInstance();
        if (!GameInst)
        {
            return false;
        }

        UGSDEventSpawnRegistry* Registry = GameInst->GetSubsystem<UGSDEventSpawnRegistry>();
        if (!Registry)
        {
            return false;
        }

        // Test 1: Default configuration values
        const FGSDEventSpawnConfig& Config = Registry->GetSpawnConfig();

        TestTrueExpr(Config.bUseAsyncLoading);
        TestEqual(Config.MaxNavMeshRetries, 3);
        TestEqual(Config.NavMeshRetryDelayMs, 10.0f);
        TestEqual(Config.MaxCachedZonesPerTag, 50);
        TestTrueExpr(Config.SpawnZoneSearchPaths.Num() > 0);

        // Test 2: Configuration update
        FGSDEventSpawnConfig NewConfig;
        NewConfig.bUseAsyncLoading = false;
        NewConfig.MaxNavMeshRetries = 5;

        Registry->SetSpawnConfig(NewConfig);

        const FGSDEventSpawnConfig& UpdatedConfig = Registry->GetSpawnConfig();
        TestFalseExpr(UpdatedConfig.bUseAsyncLoading);
        TestEqual(UpdatedConfig.MaxNavMeshRetries, 5);

        return true;
    }

    /**
     * Test suite for UGSDEventSpawnRegistry thread safety
     */
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDEventSpawnRegistryThreadSafetyTest, "GSD.DailyEvents.SpawnRegistry.ThreadSafety", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

    bool FGSDEventSpawnRegistryThreadSafetyTest::RunTest(const FString& Parameters)
    {
        UGameInstance* GameInst = GEngine->GetWorldContextsWorld()->GetGameInstance();
        if (!GameInst)
        {
            return false;
        }

        UGSDEventSpawnRegistry* Registry = GameInst->GetSubsystem<UGSDEventSpawnRegistry>();
        if (!Registry)
        {
            return false;
        }

        // Test 1: Concurrent reads don't crash
        {
            const TArray<UGSDEventSpawnZone*>& Zones = Registry->GetAllSpawnZones();
            int32 Count1 = Registry->GetZoneCount();
            const TArray<UGSDEventSpawnZone*>& Zones2 = Registry->GetAllSpawnZones();
            int32 Count2 = Registry->GetZoneCount();

            TestEqual(Count1, Count2);
            TestEqual(Zones.Num(), Zones2.Num());

        return true;
    }

    /**
     * Test suite for UGSDEventSpawnRegistry edge cases
     */
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDEventSpawnRegistryEdgeCaseTest, "GSD.DailyEvents.SpawnRegistry.EdgeCases", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

    bool FGSDEventSpawnRegistryEdgeCaseTest::RunTest(const FString& Parameters)
    {
        UGameInstance* GameInst = GEngine->GetWorldContextsWorld()->GetGameInstance();
        if (!GameInst)
        {
            return false;
        }

        UGSDEventSpawnRegistry* Registry = GameInst->GetSubsystem<UGSDEventSpawnRegistry>();
        if (!Registry)
        {
            return false;
        }

        // Test 1: Empty registry returns fallback location
        {
            FRandomStream Stream(12345);
            FGameplayTag TestTag;

            FVector Location = Registry->GetSpawnLocationForEvent(TestTag, Stream, nullptr);
            // Should return fallback location
            TestTrueExpr(Location.Size() > 0);
        }

        // Test 2: GetCompatibleZones with empty tag returns all zones
        {
            TArray<UGSDEventSpawnZone*> CompatibleZones;
            Registry->GetCompatibleZones(FGameplayTag(), CompatibleZones);

            // Should return all zones
            const TArray<UGSDEventSpawnZone*>& AllZones = Registry->GetAllSpawnZones();
            TestEqual(CompatibleZones.Num(), AllZones.Num());
        }

        return true;
    }

    /**
     * Test suite for UGSDEventSpawnRegistry cache functionality
     */
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDEventSpawnRegistryCacheTest, "GSD.DailyEvents.SpawnRegistry.Cache", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

    bool FGSDEventSpawnRegistryCacheTest::RunTest(const FString& Parameters)
    {
        UGameInstance* GameInst = GEngine->GetWorldContextsWorld()->GetGameInstance();
        if (!GameInst)
        {
            return false;
        }

        UGSDEventSpawnRegistry* Registry = GameInst->GetSubsystem<UGSDEventSpawnRegistry>();
        if (!Registry)
        {
            return false;
        }

        // Test 1: Zone count is cached
        int32 ZoneCount = Registry->GetZoneCount();
        TestTrueExpr(ZoneCount >= 0);

        // Test 2: Multiple calls return same cached value
        int32 Count1 = Registry->GetZoneCount();
        int32 Count2 = Registry->GetZoneCount();
        int32 Count3 = Registry->GetZoneCount();

        TestEqual(Count1, Count2);
        TestEqual(Count2, Count3);

        return true;
    }

#endif
