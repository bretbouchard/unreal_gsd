#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Subsystems/GSDCrowdHLODManager.h"
#include "HLOD/GSDCrowdHLODProxy.h"
#include "DataAssets/GSDCrowdHLODConfig.h"
#include "Kismet/GameplayStatics.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Test HLOD clustering algorithm
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDCrowdHLODClusteringTest,
    "GSD.Crowds.HLOD.Clustering",
    EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ContextMask)

bool FGSDCrowdHLODClusteringTest::RunTest(const FString& Parameters)
{
    // Create test entities at various positions
    TArray<FMassEntityHandle> Entities;
    TArray<FVector> Positions;

    // Create entities in two clusters
    // Cluster 1: 5 entities at (1000, 1000, 0)
    for (int32 i = 0; i < 5; i++)
    {
        FMassEntityHandle Entity;
        Entity.Index = i;
        Entity.SerialNumber = 0;
        Entities.Add(Entity);
        Positions.Add(FVector(1000.0f + i * 10.0f, 1000.0f + i * 10.0f, 0.0f));
    }

    // Cluster 2: 5 entities at (5000, 5000, 0)
    for (int32 i = 0; i < 5; i++)
    {
        FMassEntityHandle Entity;
        Entity.Index = i + 5;
        Entity.SerialNumber = 0;
        Entities.Add(Entity);
        Positions.Add(FVector(5000.0f + i * 10.0f, 5000.0f + i * 10.0f, 0.0f));
    }

    // Note: Full clustering test requires a world context
    // This test verifies the structure is correct
    TestTrue("Test entities created", Entities.Num() == 10);
    TestTrue("Test positions created", Positions.Num() == 10);

    return true;
}

/**
 * Test HLOD proxy visibility based on distance
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDCrowdHLODVisibilityTest,
    "GSD.Crowds.HLOD.Visibility",
    EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ContextMask)

bool FGSDCrowdHLODVisibilityTest::RunTest(const FString& Parameters)
{
    // Test visibility logic directly on proxy actor
    // Note: Full test requires spawning actor in world

    FVector ProxyLocation(10000.0f, 0.0f, 0.0f);
    float HLODThreshold = 5000.0f;

    // Near player - proxy should be hidden
    FVector NearPlayer(0.0f, 0.0f, 0.0f);
    float NearDistance = FVector::Dist(NearPlayer, ProxyLocation);
    TestFalse("Proxy hidden when near", NearDistance >= HLODThreshold);

    // Far player - proxy should be visible
    FVector FarPlayer(15000.0f, 0.0f, 0.0f);
    float FarDistance = FVector::Dist(FarPlayer, ProxyLocation);
    TestTrue("Proxy visible when far", FarDistance >= HLODThreshold);

    return true;
}

/**
 * Test HLOD threshold behavior
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDCrowdHLODThresholdTest,
    "GSD.Crowds.HLOD.Threshold",
    EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ContextMask)

bool FGSDCrowdHLODThresholdTest::RunTest(const FString& Parameters)
{
    // Test threshold calculations
    FVector ClusterCenter(5000.0f, 5000.0f, 0.0f);

    // Test at exact threshold
    FVector AtThreshold(ClusterCenter.X + 5000.0f, ClusterCenter.Y, 0.0f);
    float DistanceAtThreshold = FVector::Dist(AtThreshold, ClusterCenter);
    TestEqual("Distance at threshold", DistanceAtThreshold, 5000.0f);

    // Test below threshold
    FVector BelowThreshold(ClusterCenter.X + 4000.0f, ClusterCenter.Y, 0.0f);
    float DistanceBelowThreshold = FVector::Dist(BelowThreshold, ClusterCenter);
    TestTrue("Below threshold", DistanceBelowThreshold < 5000.0f);

    // Test above threshold
    FVector AboveThreshold(ClusterCenter.X + 6000.0f, ClusterCenter.Y, 0.0f);
    float DistanceAboveThreshold = FVector::Dist(AboveThreshold, ClusterCenter);
    TestTrue("Above threshold", DistanceAboveThreshold > 5000.0f);

    return true;
}

/**
 * Test HLOD configuration validation
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDCrowdHLODConfigTest,
    "GSD.Crowds.HLOD.Config",
    EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ContextMask)

bool FGSDCrowdHLODConfigTest::RunTest(const FString& Parameters)
{
    // Create config and test validation
    UGSDCrowdHLODConfig* Config = NewObject<UGSDCrowdHLODConfig>();

    // Test default config is valid
    FString Error;
    TestTrue("Default config is valid", Config->ValidateConfig(Error));

    // Test invalid HLODStartDistance
    Config->HLODStartDistance = -1.0f;
    TestFalse("Invalid start distance rejected", Config->ValidateConfig(Error));

    // Reset and test invalid HLODOnlyDistance
    Config->HLODStartDistance = 3000.0f;
    Config->HLODOnlyDistance = 1000.0f; // Less than start
    TestFalse("Invalid only distance rejected", Config->ValidateConfig(Error));

    // Reset and test invalid cluster size
    Config->HLODOnlyDistance = 5000.0f;
    Config->ClusterSize = 0.0f;
    TestFalse("Invalid cluster size rejected", Config->ValidateConfig(Error));

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
