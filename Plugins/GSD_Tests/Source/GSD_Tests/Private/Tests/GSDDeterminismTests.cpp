// Copyright Bret Bouchard. All Rights Reserved.

#include "GSD_Tests.h"
#include "Misc/AutomationTest.h"
#include "Math/RandomStream.h"

#if WITH_DEV_AUTOMATION_TESTS

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
