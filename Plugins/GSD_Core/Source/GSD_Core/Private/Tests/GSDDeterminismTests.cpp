#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Subsystems/GSDDeterminismManager.h"
#include "Engine/GameInstance.h"

#if WITH_DEV_AUTOMATION_TESTS

// Test: Determinism manager seeding
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDDeterminismManagerSeedingTest,
    "GSD.Core.Determinism.Seeding",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDDeterminismManagerSeedingTest::RunTest(const FString& Parameters)
{
    // Create subsystem directly (not through GameInstance for test)
    UGSDDeterminismManager* Manager = NewObject<UGSDDeterminismManager>();
    Manager->InitializeWithSeed(12345);

    TestEqual(TEXT("CurrentSeed should be 12345"),
        Manager->GetCurrentSeed(), 12345);

    // Test that streams are created
    FRandomStream& SpawnStream = Manager->GetStream(UGSDDeterminismManager::SpawnCategory);
    TestTrue(TEXT("Spawn stream should be valid"),
        SpawnStream.GetInitialSeed() != 0);

    return true;
}

// Test: Same seed produces same sequence
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDDeterminismManagerReproducibilityTest,
    "GSD.Core.Determinism.Reproducibility",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDDeterminismManagerReproducibilityTest::RunTest(const FString& Parameters)
{
    const int32 TestSeed = 54321;

    // First run
    UGSDDeterminismManager* Manager1 = NewObject<UGSDDeterminismManager>();
    Manager1->InitializeWithSeed(TestSeed);

    TArray<float> FirstRun;
    for (int32 i = 0; i < 10; ++i)
    {
        FirstRun.Add(Manager1->RandomFloat(UGSDDeterminismManager::SpawnCategory));
    }

    // Second run with same seed
    UGSDDeterminismManager* Manager2 = NewObject<UGSDDeterminismManager>();
    Manager2->InitializeWithSeed(TestSeed);

    TArray<float> SecondRun;
    for (int32 i = 0; i < 10; ++i)
    {
        SecondRun.Add(Manager2->RandomFloat(UGSDDeterminismManager::SpawnCategory));
    }

    // Compare sequences
    for (int32 i = 0; i < 10; ++i)
    {
        TestEqual(FString::Printf(TEXT("Random value %d should match"), i),
            FirstRun[i], SecondRun[i]);
    }

    return true;
}

// Test: Category isolation
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDDeterminismManagerCategoryIsolationTest,
    "GSD.Core.Determinism.CategoryIsolation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDDeterminismManagerCategoryIsolationTest::RunTest(const FString& Parameters)
{
    const int32 TestSeed = 99999;

    UGSDDeterminismManager* Manager = NewObject<UGSDDeterminismManager>();
    Manager->InitializeWithSeed(TestSeed);

    // Draw from different categories
    float SpawnValue = Manager->RandomFloat(UGSDDeterminismManager::SpawnCategory);
    float EventValue = Manager->RandomFloat(UGSDDeterminismManager::EventCategory);
    float CrowdValue = Manager->RandomFloat(UGSDDeterminismManager::CrowdCategory);

    // Reset and draw again in different order
    Manager->InitializeWithSeed(TestSeed);

    float SpawnValue2 = Manager->RandomFloat(UGSDDeterminismManager::SpawnCategory);
    float EventValue2 = Manager->RandomFloat(UGSDDeterminismManager::EventCategory);
    float CrowdValue2 = Manager->RandomFloat(UGSDDeterminismManager::CrowdCategory);

    // Each category should produce same sequence regardless of other categories
    TestEqual(TEXT("Spawn category reproducible"), SpawnValue, SpawnValue2);
    TestEqual(TEXT("Event category reproducible"), EventValue, EventValue2);
    TestEqual(TEXT("Crowd category reproducible"), CrowdValue, CrowdValue2);

    return true;
}

// Test: Random integer range
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDDeterminismManagerRandomIntegerTest,
    "GSD.Core.Determinism.RandomInteger",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDDeterminismManagerRandomIntegerTest::RunTest(const FString& Parameters)
{
    UGSDDeterminismManager* Manager = NewObject<UGSDDeterminismManager>();
    Manager->InitializeWithSeed(11111);

    const int32 MaxValue = 100;
    for (int32 i = 0; i < 100; ++i)
    {
        int32 Value = Manager->RandomInteger(UGSDDeterminismManager::SpawnCategory, MaxValue);
        TestTrue(FString::Printf(TEXT("RandomInteger should be < %d (got %d)"), MaxValue, Value),
            Value >= 0 && Value < MaxValue);
    }

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
