#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Types/GSDPerformanceConfig.h"
#include "Types/GSDSaveGame.h"

#if WITH_DEV_AUTOMATION_TESTS

// Test: Performance config defaults
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDPerformanceConfigDefaultsTest,
    "GSD.Core.Performance.Defaults",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDPerformanceConfigDefaultsTest::RunTest(const FString& Parameters)
{
    UGSDPerformanceConfig* Config = NewObject<UGSDPerformanceConfig>();

    // Test spawning defaults
    TestEqual(TEXT("SpawningFrameBudgetMs default"), Config->SpawningFrameBudgetMs, 2.0f);
    TestEqual(TEXT("MaxSpawnsPerFrame default"), Config->MaxSpawnsPerFrame, 10);
    TestEqual(TEXT("BatchSpawnChunkSize default"), Config->BatchSpawnChunkSize, 20);

    // Test memory defaults
    TestEqual(TEXT("EntityMemoryBudgetMB default"), Config->EntityMemoryBudgetMB, 512.0f);
    TestEqual(TEXT("MaxPooledEntities default"), Config->MaxPooledEntities, 100);

    // Test audio defaults
    TestEqual(TEXT("AudioBudgetMs default"), Config->AudioBudgetMs, 2.0f);
    TestEqual(TEXT("MaxConcurrentAudioSources default"), Config->MaxConcurrentAudioSources, 32);

    // Test hitch threshold
    TestEqual(TEXT("HitchThresholdMs default"), Config->HitchThresholdMs, 16.67f);

    return true;
}

// Test: Budget checking
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDPerformanceConfigBudgetTest,
    "GSD.Core.Performance.BudgetChecking",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDPerformanceConfigBudgetTest::RunTest(const FString& Parameters)
{
    UGSDPerformanceConfig* Config = NewObject<UGSDPerformanceConfig>();

    // Test IsWithinSpawnBudget
    TestTrue(TEXT("1.0ms should be within budget"),
        Config->IsWithinSpawnBudget(1.0f));
    TestTrue(TEXT("2.0ms should be within budget"),
        Config->IsWithinSpawnBudget(2.0f));
    TestFalse(TEXT("3.0ms should exceed budget"),
        Config->IsWithinSpawnBudget(3.0f));

    // Test IsHitch
    TestFalse(TEXT("10ms should not be a hitch"),
        Config->IsHitch(10.0f));
    TestFalse(TEXT("16.67ms should not be a hitch"),
        Config->IsHitch(16.67f));
    TestTrue(TEXT("20ms should be a hitch"),
        Config->IsHitch(20.0f));

    // Test GetFrameBudgetRemaining
    TestEqual(TEXT("Budget remaining with 1ms used"),
        Config->GetFrameBudgetRemaining(1.0f), 1.0f);
    TestEqual(TEXT("Budget remaining with 2ms used"),
        Config->GetFrameBudgetRemaining(2.0f), 0.0f);
    TestEqual(TEXT("Budget remaining with 3ms used (capped)"),
        Config->GetFrameBudgetRemaining(3.0f), 0.0f);

    return true;
}

// Test: SaveGame basic operations
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDSaveGameBasicTest,
    "GSD.Core.SaveGame.BasicOperations",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDSaveGameBasicTest::RunTest(const FString& Parameters)
{
    UGSDSaveGame* SaveGame = NewObject<UGSDSaveGame>();

    // Test initial state
    TestEqual(TEXT("Initial ActorStates count"), SaveGame->GetActorStateCount(), 0);

    // Test AddActorState
    FGSDSerializedActorState State1;
    State1.ActorName = TEXT("TestActor1");
    State1.Transform = FTransform(FVector(100.0f, 200.0f, 300.0f));
    SaveGame->AddActorState(State1);

    TestEqual(TEXT("After add count"), SaveGame->GetActorStateCount(), 1);

    // Test GetActorState
    FGSDSerializedActorState RetrievedState;
    TestTrue(TEXT("GetActorState should find actor"),
        SaveGame->GetActorState(TEXT("TestActor1"), RetrievedState));
    TestEqual(TEXT("Retrieved transform"),
        RetrievedState.Transform.GetTranslation(), FVector(100.0f, 200.0f, 300.0f));

    // Test GetActorState for non-existent
    TestFalse(TEXT("GetActorState should not find non-existent"),
        SaveGame->GetActorState(TEXT("NonExistent"), RetrievedState));

    // Test RemoveActorState
    SaveGame->RemoveActorState(TEXT("TestActor1"));
    TestEqual(TEXT("After remove count"), SaveGame->GetActorStateCount(), 0);

    return true;
}

// Test: FGSDTickContext audio budget
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDTickContextTest,
    "GSD.Core.TickContext.AudioBudget",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDTickContextTest::RunTest(const FString& Parameters)
{
    FGSDTickContext Context;

    // Test defaults
    TestEqual(TEXT("Default DeltaTime"), Context.DeltaTime, 0.0f);
    TestEqual(TEXT("Default AudioBudgetRemainingMs"), Context.AudioBudgetRemainingMs, 2.0f);

    // Test HasAudioBudget
    TestTrue(TEXT("Should have audio budget with 2.0ms"),
        Context.HasAudioBudget());

    Context.AudioBudgetRemainingMs = 0.05f;
    TestFalse(TEXT("Should not have audio budget with 0.05ms"),
        Context.HasAudioBudget());

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
