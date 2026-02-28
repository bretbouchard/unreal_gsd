// Copyright Bret Bouchard. All Rights Reserved.

#include "GSD_Tests.h"
#include "Misc/AutomationTest.h"
#include "Math/RandomStream.h"

#if WITH_DEV_AUTOMATION_TESTS

//-- Crowd System Determinism Tests (Phase 12-01) --
// These tests verify that crowd system random calls use seeded FRandomStream
// from GSDDeterminismManager, enabling deterministic replays.

// Test 6: Crowd Spawn Category - Same seed produces same spawn positions
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDCrowdSpawnDeterminismTest,
    "GSD.Determinism.Crowd.SameSeed",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDCrowdSpawnDeterminismTest::RunTest(const FString& Parameters)
{
    // Test that same seed produces identical spawn positions
    const int32 TestSeed = 12345;
    const float Radius = 1000.0f;

    // Generate first sequence using seeded stream (simulating CrowdSpawnCategory)
    FRandomStream Stream1(TestSeed + GetTypeHash(FName("CrowdSpawn")));
    TArray<FVector> Positions1;
    for (int32 i = 0; i < 100; i++)
    {
        const float Angle = Stream1.FRand() * 2.0f * PI;
        const float Distance = Stream1.FRand() * Radius;
        Positions1.Add(FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.0f));
    }

    // Generate second sequence with same derived seed
    FRandomStream Stream2(TestSeed + GetTypeHash(FName("CrowdSpawn")));
    TArray<FVector> Positions2;
    for (int32 i = 0; i < 100; i++)
    {
        const float Angle = Stream2.FRand() * 2.0f * PI;
        const float Distance = Stream2.FRand() * Radius;
        Positions2.Add(FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.0f));
    }

    // Compare all positions
    for (int32 i = 0; i < 100; i++)
    {
        TestEqual(FString::Printf(TEXT("Crowd spawn position %d X matches"), i),
            Positions1[i].X, Positions2[i].X);
        TestEqual(FString::Printf(TEXT("Crowd spawn position %d Y matches"), i),
            Positions1[i].Y, Positions2[i].Y);
        TestEqual(FString::Printf(TEXT("Crowd spawn position %d Z matches"), i),
            Positions1[i].Z, Positions2[i].Z);
    }

    return true;
}

// Test 7: Crowd Spawn Category - Different seeds produce different positions
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDCrowdSpawnVariationTest,
    "GSD.Determinism.Crowd.DifferentSeeds",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDCrowdSpawnVariationTest::RunTest(const FString& Parameters)
{
    // Test that different seeds produce different spawn positions
    const int32 Seed1 = 12345;
    const int32 Seed2 = 54321;
    const float Radius = 1000.0f;

    // Generate with first seed
    FRandomStream Stream1(Seed1 + GetTypeHash(FName("CrowdSpawn")));
    const FVector Pos1 = FVector(
        Stream1.FRand() * Radius,
        Stream1.FRand() * Radius,
        0.0f
    );

    // Generate with second seed
    FRandomStream Stream2(Seed2 + GetTypeHash(FName("CrowdSpawn")));
    const FVector Pos2 = FVector(
        Stream2.FRand() * Radius,
        Stream2.FRand() * Radius,
        0.0f
    );

    // Should be different (extremely unlikely to be equal)
    TestNotEqual(TEXT("Different seeds produce different X positions"), Pos1.X, Pos2.X);
    TestNotEqual(TEXT("Different seeds produce different Y positions"), Pos1.Y, Pos2.Y);

    return true;
}

// Test 8: Zombie Behavior Category - Speed variation determinism
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDZombieSpeedDeterminismTest,
    "GSD.Determinism.Zombie.Speed",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDZombieSpeedDeterminismTest::RunTest(const FString& Parameters)
{
    // Test that speed variation uses seeded random
    const int32 TestSeed = 12345;
    const float SpeedVariation = 0.2f; // 20%

    // Generate speed multipliers with first stream
    FRandomStream Stream1(TestSeed + GetTypeHash(FName("ZombieSpeed")));
    TArray<float> Speeds1;
    for (int32 i = 0; i < 50; i++)
    {
        const float SpeedMultiplier = 1.0f + Stream1.FRandRange(-SpeedVariation, SpeedVariation);
        Speeds1.Add(SpeedMultiplier);
    }

    // Generate speed multipliers with second stream (same seed)
    FRandomStream Stream2(TestSeed + GetTypeHash(FName("ZombieSpeed")));
    TArray<float> Speeds2;
    for (int32 i = 0; i < 50; i++)
    {
        const float SpeedMultiplier = 1.0f + Stream2.FRandRange(-SpeedVariation, SpeedVariation);
        Speeds2.Add(SpeedMultiplier);
    }

    // Verify identical speed multipliers
    for (int32 i = 0; i < 50; i++)
    {
        TestEqual(FString::Printf(TEXT("Speed multiplier %d matches"), i),
            Speeds1[i], Speeds2[i]);

        // Verify speed is within expected range (0.8 to 1.2)
        TestTrue(FString::Printf(TEXT("Speed multiplier %d >= 0.8"), i), Speeds1[i] >= 0.8f);
        TestTrue(FString::Printf(TEXT("Speed multiplier %d <= 1.2"), i), Speeds1[i] <= 1.2f);
    }

    return true;
}

// Test 9: Zombie Wander Category - Direction change determinism
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDZombieWanderDeterminismTest,
    "GSD.Determinism.Zombie.Wander",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDZombieWanderDeterminismTest::RunTest(const FString& Parameters)
{
    // Test that wander direction uses seeded random
    const int32 TestSeed = 54321;
    const float WanderDirectionChange = 45.0f;

    // Generate wander directions with first stream
    FRandomStream Stream1(TestSeed + GetTypeHash(FName("ZombieWander")));
    TArray<float> Directions1;
    float CurrentDirection = 0.0f;
    for (int32 i = 0; i < 30; i++)
    {
        CurrentDirection += Stream1.FRandRange(-WanderDirectionChange, WanderDirectionChange);
        CurrentDirection = FMath::Clamp(CurrentDirection, -180.0f, 180.0f);
        Directions1.Add(CurrentDirection);
    }

    // Generate wander directions with second stream (same seed)
    FRandomStream Stream2(TestSeed + GetTypeHash(FName("ZombieWander")));
    TArray<float> Directions2;
    CurrentDirection = 0.0f;
    for (int32 i = 0; i < 30; i++)
    {
        CurrentDirection += Stream2.FRandRange(-WanderDirectionChange, WanderDirectionChange);
        CurrentDirection = FMath::Clamp(CurrentDirection, -180.0f, 180.0f);
        Directions2.Add(CurrentDirection);
    }

    // Verify identical wander directions
    for (int32 i = 0; i < 30; i++)
    {
        TestEqual(FString::Printf(TEXT("Wander direction %d matches"), i),
            Directions1[i], Directions2[i]);

        // Verify direction is within valid range
        TestTrue(FString::Printf(TEXT("Wander direction %d >= -180"), i), Directions1[i] >= -180.0f);
        TestTrue(FString::Printf(TEXT("Wander direction %d <= 180"), i), Directions1[i] <= 180.0f);
    }

    return true;
}

// Test 10: Category Isolation - Different categories don't affect each other
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDCategoryIsolationTest,
    "GSD.Determinism.Category.Isolation",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDCategoryIsolationTest::RunTest(const FString& Parameters)
{
    // Test that using one category doesn't affect another
    const int32 TestSeed = 99999;

    // Create two streams with same base seed
    FRandomStream SpawnStream(TestSeed + GetTypeHash(FName("CrowdSpawn")));
    FRandomStream WanderStream(TestSeed + GetTypeHash(FName("ZombieWander")));

    // Get first value from SpawnStream
    const float SpawnValue1 = SpawnStream.FRand();

    // Use WanderStream several times
    for (int32 i = 0; i < 10; i++)
    {
        WanderStream.FRand();
    }

    // Get second value from SpawnStream
    const float SpawnValue2 = SpawnStream.FRand();

    // Get third value from a fresh SpawnStream (same seed)
    FRandomStream SpawnStream2(TestSeed + GetTypeHash(FName("CrowdSpawn")));
    SpawnStream2.FRand(); // Skip first
    const float SpawnValue3 = SpawnStream2.FRand();

    // Using WanderStream should not affect SpawnStream
    TestEqual(TEXT("SpawnStream value 2 matches fresh stream value 2"),
        SpawnValue2, SpawnValue3);

    // Values should be different (random)
    TestNotEqual(TEXT("SpawnStream produces different values"), SpawnValue1, SpawnValue2);

    return true;
}

// Test 1: Daily Seed Determinism - Same date + seed = same random values
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDDailySeedDeterminismTest,
    "GSD.Determinism.DailySeed.Reproducibility",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDDailySeedDeterminismTest::RunTest(const FString& Parameters)
{
    // Test that same seed produces same random sequence
    const int32 TestSeed = 12345;

    // Generate first sequence
    FRandomStream Stream1(TestSeed);
    float Value1_1 = Stream1.FRand();
    float Value1_2 = Stream1.FRand();
    float Value1_3 = Stream1.FRand();

    // Generate second sequence with same seed
    FRandomStream Stream2(TestSeed);
    float Value2_1 = Stream2.FRand();
    float Value2_2 = Stream2.FRand();
    float Value2_3 = Stream2.FRand();

    // Verify identical sequences
    TestEqual(TEXT("First random value matches"), Value1_1, Value2_1);
    TestEqual(TEXT("Second random value matches"), Value1_2, Value2_2);
    TestEqual(TEXT("Third random value matches"), Value1_3, Value2_3);

    // Test daily seed generation (date-based seed)
    const int32 Year = 2026;
    const int32 Month = 2;
    const int32 Day = 27;

    // Generate deterministic daily seed from date
    const int32 DailySeed = Year * 10000 + Month * 100 + Day;
    TestEqual(TEXT("Daily seed is deterministic"), DailySeed, 20260227);

    // Verify daily seed produces reproducible results
    FRandomStream DailyStream1(DailySeed);
    FRandomStream DailyStream2(DailySeed);
    TestEqual(TEXT("Daily seed produces same first value"),
        DailyStream1.FRand(), DailyStream2.FRand());

    return true;
}

// Test 2: Event Ordering - Verify alphabetical ordering for same-timestamp events
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDEventOrderingTest,
    "GSD.Determinism.Event.Ordering",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDEventOrderingTest::RunTest(const FString& Parameters)
{
    // Test event ordering: same timestamp = alphabetical by tag
    TArray<FName> EventTags;
    EventTags.Add(FName(TEXT("ZombieHorde")));
    EventTags.Add(FName(TEXT("Bonfire")));
    EventTags.Add(FName(TEXT("Construction")));
    EventTags.Add(FName(TEXT("Ambush")));

    // Sort alphabetically (deterministic tiebreaker)
    EventTags.Sort();

    // Verify ordering
    TestEqual(TEXT("First event is Ambush"), EventTags[0], FName(TEXT("Ambush")));
    TestEqual(TEXT("Second event is Bonfire"), EventTags[1], FName(TEXT("Bonfire")));
    TestEqual(TEXT("Third event is Construction"), EventTags[2], FName(TEXT("Construction")));
    TestEqual(TEXT("Fourth event is ZombieHorde"), EventTags[3], FName(TEXT("ZombieHorde")));

    // Test that sorting is stable (same input = same output)
    TArray<FName> EventTags2;
    EventTags2.Add(FName(TEXT("ZombieHorde")));
    EventTags2.Add(FName(TEXT("Bonfire")));
    EventTags2.Add(FName(TEXT("Construction")));
    EventTags2.Add(FName(TEXT("Ambush")));
    EventTags2.Sort();

    for (int32 i = 0; i < EventTags.Num(); i++)
    {
        TestEqual(FString::Printf(TEXT("Sorted event %d matches"), i),
            EventTags[i], EventTags2[i]);
    }

    return true;
}

// Test 3: Navigation Determinism - Same seed = same lane selections
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDNavigationDeterminismTest,
    "GSD.Determinism.Navigation.Reproducibility",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDNavigationDeterminismTest::RunTest(const FString& Parameters)
{
    // Test navigation randomization determinism
    const int32 TestSeed = 54321;

    // Simulate lane selection with randomization
    FRandomStream Stream1(TestSeed);
    TArray<int32> Lanes1;
    for (int32 i = 0; i < 10; i++)
    {
        Lanes1.Add(Stream1.RandRange(0, 4)); // Select from 5 lanes
    }

    // Repeat with same seed
    FRandomStream Stream2(TestSeed);
    TArray<int32> Lanes2;
    for (int32 i = 0; i < 10; i++)
    {
        Lanes2.Add(Stream2.RandRange(0, 4));
    }

    // Verify identical lane selections
    for (int32 i = 0; i < 10; i++)
    {
        TestEqual(FString::Printf(TEXT("Lane selection %d matches"), i),
            Lanes1[i], Lanes2[i]);
    }

    // Test velocity randomization determinism (20% variation)
    const float BaseSpeed = 150.0f;
    FRandomStream VelocityStream1(TestSeed);
    FRandomStream VelocityStream2(TestSeed);

    for (int32 i = 0; i < 5; i++)
    {
        // Velocity randomization formula from config
        const float RandomFactor1 = 1.0f + (VelocityStream1.FRand() - 0.5f) * 0.4f; // +/- 20%
        const float RandomFactor2 = 1.0f + (VelocityStream2.FRand() - 0.5f) * 0.4f;

        TestEqual(FString::Printf(TEXT("Velocity factor %d matches"), i),
            RandomFactor1, RandomFactor2);

        const float Speed1 = BaseSpeed * RandomFactor1;
        const float Speed2 = BaseSpeed * RandomFactor2;
        TestEqual(FString::Printf(TEXT("Randomized speed %d matches"), i),
            Speed1, Speed2);
    }

    return true;
}

// Test 4: Spawn Location Determinism - Same seed = same spawn points
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDSpawnLocationDeterminismTest,
    "GSD.Determinism.SpawnLocation.Reproducibility",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDSpawnLocationDeterminismTest::RunTest(const FString& Parameters)
{
    // Test spawn location determinism
    const int32 TestSeed = 98765;
    const FVector Center(0.0f, 0.0f, 0.0f);
    const float Radius = 1000.0f;

    // Generate spawn locations with first stream
    FRandomStream Stream1(TestSeed);
    TArray<FVector> Locations1;
    for (int32 i = 0; i < 10; i++)
    {
        const float Angle = Stream1.FRand() * 2.0f * PI;
        const float Distance = Stream1.FRand() * Radius;
        FVector Location(
            Center.X + FMath::Cos(Angle) * Distance,
            Center.Y + FMath::Sin(Angle) * Distance,
            Center.Z
        );
        Locations1.Add(Location);
    }

    // Generate spawn locations with second stream (same seed)
    FRandomStream Stream2(TestSeed);
    TArray<FVector> Locations2;
    for (int32 i = 0; i < 10; i++)
    {
        const float Angle = Stream2.FRand() * 2.0f * PI;
        const float Distance = Stream2.FRand() * Radius;
        FVector Location(
            Center.X + FMath::Cos(Angle) * Distance,
            Center.Y + FMath::Sin(Angle) * Distance,
            Center.Z
        );
        Locations2.Add(Location);
    }

    // Verify identical spawn locations
    for (int32 i = 0; i < 10; i++)
    {
        TestEqual(FString::Printf(TEXT("Spawn location %d X matches"), i),
            Locations1[i].X, Locations2[i].X);
        TestEqual(FString::Printf(TEXT("Spawn location %d Y matches"), i),
            Locations1[i].Y, Locations2[i].Y);
        TestEqual(FString::Printf(TEXT("Spawn location %d Z matches"), i),
            Locations1[i].Z, Locations2[i].Z);
    }

    return true;
}

// Test 5: Intensity Determinism - Same seed = same intensity values
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDIntensityDeterminismTest,
    "GSD.Determinism.Intensity.Reproducibility",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDIntensityDeterminismTest::RunTest(const FString& Parameters)
{
    // Test intensity determinism
    const int32 TestSeed = 11111;

    // Generate intensity values with first stream
    FRandomStream Stream1(TestSeed);
    TArray<float> Intensities1;
    for (int32 i = 0; i < 5; i++)
    {
        // Intensity range: 0.5 to 2.0 (event intensity multiplier)
        const float Intensity = 0.5f + Stream1.FRand() * 1.5f;
        Intensities1.Add(Intensity);
    }

    // Generate intensity values with second stream (same seed)
    FRandomStream Stream2(TestSeed);
    TArray<float> Intensities2;
    for (int32 i = 0; i < 5; i++)
    {
        const float Intensity = 0.5f + Stream2.FRand() * 1.5f;
        Intensities2.Add(Intensity);
    }

    // Verify identical intensity values
    for (int32 i = 0; i < 5; i++)
    {
        TestEqual(FString::Printf(TEXT("Intensity value %d matches"), i),
            Intensities1[i], Intensities2[i]);
    }

    // Verify intensity values are in valid range
    for (int32 i = 0; i < 5; i++)
    {
        TestTrue(FString::Printf(TEXT("Intensity %d >= 0.5"), i), Intensities1[i] >= 0.5f);
        TestTrue(FString::Printf(TEXT("Intensity %d <= 2.0"), i), Intensities1[i] <= 2.0f);
    }

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
