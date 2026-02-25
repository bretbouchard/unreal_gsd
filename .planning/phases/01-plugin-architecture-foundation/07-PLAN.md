---
phase: 01-plugin-architecture-foundation
plan: 07
type: execute
wave: 5
depends_on: ["01", "02", "03", "04", "05", "06"]
files_modified:
  - Plugins/GSD_Core/Source/GSD_Core/Private/Tests/GSDInterfaceTests.cpp
  - Plugins/GSD_Core/Source/GSD_Core/Private/Tests/GSDMassSpawnerTests.cpp
  - Plugins/GSD_Core/Source/GSD_Core/Private/Tests/GSDDeterminismTests.cpp
  - Plugins/GSD_Core/Source/GSD_Core/Private/Tests/GSDPerformanceTests.cpp
  - Plugins/GSD_Core/Source/GSD_Core/GSD_Core.Build.cs (update for tests)
autonomous: true
must_haves:
  truths:
    - "All interfaces have automation tests verifying contracts"
    - "Mass spawning tests verify batch operations"
    - "Determinism tests verify seeded RNG reproducibility"
    - "Performance tests verify budget enforcement"
  artifacts:
    - path: "Plugins/GSD_Core/Source/GSD_Core/Private/Tests/GSDInterfaceTests.cpp"
      provides: "Interface contract verification"
      contains: "IMPLEMENT_SIMPLE_AUTOMATION_TEST"
    - path: "Plugins/GSD_Core/Source/GSD_Core/Private/Tests/GSDMassSpawnerTests.cpp"
      provides: "Batch spawning verification"
      contains: "SpawnBatch"
    - path: "Plugins/GSD_Core/Source/GSD_Core/Private/Tests/GSDDeterminismTests.cpp"
      provides: "Determinism verification"
      contains: "InitializeWithSeed"
  key_links:
    - from: "Tests"
      to: "Automation System"
      via: "Session Frontend"
      pattern: "EAutomationTestFlags"
---

# 07-PLAN: Unit Tests and Verification

**Requirements:** PLUG-08

**Objective:** Create comprehensive unit tests for all GSD_Core interfaces, base classes, managers, and performance configuration.

**Purpose:** Verify that all interfaces, base classes, and managers work correctly before depending plugins are developed. Tests serve as documentation of expected behavior.

**Output:** Automation test suite covering interfaces, spawning, determinism, and performance.

---

## Context

@.planning/PROJECT.md
@.planning/ROADMAP.md
@.planning/phases/01-plugin-architecture-foundation/01-RESEARCH.md
@.planning/phases/01-plugin-architecture-foundation/1-CONTEXT.md

---

## Tasks

<task type="auto">
  <name>T1: Create Interface Contract Tests</name>
  <files>
    Plugins/GSD_Core/Source/GSD_Core/Private/Tests/GSDInterfaceTests.cpp
    Plugins/GSD_Core/Source/GSD_Core/GSD_Core.Build.cs
  </files>
  <action>
    Create automation tests verifying interface contracts.

    First, update GSD_Core.Build.cs to enable tests:
    ```csharp
    // Add to constructor
    if (Target.bBuildEditor)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {
            "UnrealEd",
            "EditorScriptingUtilities"
        });
    }
    ```

    GSDInterfaceTests.cpp:
    ```cpp
    #include "CoreMinimal.h"
    #include "Misc/AutomationTest.h"
    #include "Interfaces/IGSDSpawnable.h"
    #include "Interfaces/IGSDStreamable.h"
    #include "Interfaces/IGSDMassSpawnerInterface.h"
    #include "Interfaces/IGSDComponentSpawnable.h"
    #include "Interfaces/IGSDSpatialAudioStreamable.h"
    #include "Classes/GSDActor.h"
    #include "Classes/GSDComponent.h"
    #include "DataAssets/GSDDataAsset.h"

    #if WITH_DEV_AUTOMATION_TESTS

    // Test: IGSDSpawnable interface contract
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDSpawnableInterfaceTest,
        "GSD.Core.Interfaces.IGSDSpawnable",
        EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

    bool FGSDSpawnableInterfaceTest::RunTest(const FString& Parameters)
    {
        // Create test actor implementing IGSDSpawnable
        AGSDActor* TestActor = NewObject<AGSDActor>();

        // Test: Initial spawn state is false
        TestFalse(TEXT("Initial IsSpawned should be false"),
            IGSDSpawnable::Execute_IsSpawned(TestActor));

        // Test: GetSpawnConfig returns nullptr initially
        TestNull(TEXT("Initial GetSpawnConfig should be null"),
            IGSDSpawnable::Execute_GetSpawnConfig(TestActor));

        // Test: SpawnAsync with valid config
        UGSDDataAsset* TestConfig = NewObject<UGSDDataAsset>();
        bool bCallbackExecuted = false;
        FOnSpawnComplete Callback;
        Callback.BindLambda([&bCallbackExecuted](AActor* SpawnedActor) {
            bCallbackExecuted = true;
        });

        IGSDSpawnable::Execute_SpawnAsync(TestActor, TestConfig, Callback);

        TestTrue(TEXT("SpawnAsync should set IsSpawned to true"),
            IGSDSpawnable::Execute_IsSpawned(TestActor));

        TestTrue(TEXT("SpawnAsync should execute callback"),
            bCallbackExecuted);

        TestEqual(TEXT("GetSpawnConfig should return set config"),
            IGSDSpawnable::Execute_GetSpawnConfig(TestActor), TestConfig);

        // Test: Despawn resets state
        IGSDSpawnable::Execute_Despawn(TestActor);
        TestFalse(TEXT("Despawn should set IsSpawned to false"),
            IGSDSpawnable::Execute_IsSpawned(TestActor));

        return true;
    }

    // Test: IGSDStreamable interface contract
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDStreamableInterfaceTest,
        "GSD.Core.Interfaces.IGSDStreamable",
        EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

    bool FGSDStreamableInterfaceTest::RunTest(const FString& Parameters)
    {
        AGSDActor* TestActor = NewObject<AGSDActor>();

        // Test: GetStreamingBounds returns valid bounds
        FBoxSphereBounds Bounds = IGSDStreamable::Execute_GetStreamingBounds(TestActor);
        TestTrue(TEXT("GetStreamingBounds should return valid box"),
            Bounds.BoxExtent.Size() > 0.0f);

        // Test: Initial streaming state
        TestTrue(TEXT("Initial IsCurrentlyStreamedIn should be true"),
            IGSDStreamable::Execute_IsCurrentlyStreamedIn(TestActor));

        // Test: OnStreamIn/OnStreamOut
        IGSDStreamable::Execute_OnStreamOut(TestActor);
        TestFalse(TEXT("OnStreamOut should set IsCurrentlyStreamedIn to false"),
            IGSDStreamable::Execute_IsCurrentlyStreamedIn(TestActor));

        IGSDStreamable::Execute_OnStreamIn(TestActor);
        TestTrue(TEXT("OnStreamIn should set IsCurrentlyStreamedIn to true"),
            IGSDStreamable::Execute_IsCurrentlyStreamedIn(TestActor));

        return true;
    }

    // Test: IGSDComponentSpawnable interface contract
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDComponentSpawnableInterfaceTest,
        "GSD.Core.Interfaces.IGSDComponentSpawnable",
        EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

    bool FGSDComponentSpawnableInterfaceTest::RunTest(const FString& Parameters)
    {
        UGSDComponent* TestComponent = NewObject<UGSDComponent>();

        // Test: GetComponentType returns valid name
        FName ComponentType = IGSDComponentSpawnable::Execute_GetComponentType(TestComponent);
        TestTrue(TEXT("GetComponentType should return valid name"),
            ComponentType != NAME_None);

        // Test: Initial active state
        TestFalse(TEXT("Initial IsComponentActive should be false"),
            IGSDComponentSpawnable::Execute_IsComponentActive(TestComponent));

        return true;
    }

    // Test: IGSDSpatialAudioStreamable interface contract
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDSpatialAudioStreamableInterfaceTest,
        "GSD.Core.Interfaces.IGSDSpatialAudioStreamable",
        EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

    bool FGSDSpatialAudioStreamableInterfaceTest::RunTest(const FString& Parameters)
    {
        // This test requires an actor implementing IGSDSpatialAudioStreamable
        // For now, test that the struct works
        FGSDSpatialAudioState State;
        State.WorldLocation = FVector(100.0f, 200.0f, 300.0f);
        State.DistanceToListener = 500.0f;
        State.bIsOccluded = true;

        TestEqual(TEXT("SpatialAudioState WorldLocation X"),
            State.WorldLocation.X, 100.0f);
        TestEqual(TEXT("SpatialAudioState WorldLocation Y"),
            State.WorldLocation.Y, 200.0f);
        TestEqual(TEXT("SpatialAudioState WorldLocation Z"),
            State.WorldLocation.Z, 300.0f);
        TestEqual(TEXT("SpatialAudioState DistanceToListener"),
            State.DistanceToListener, 500.0f);
        TestTrue(TEXT("SpatialAudioState bIsOccluded"),
            State.bIsOccluded);

        // Test Reset
        State.Reset();
        TestEqual(TEXT("After Reset WorldLocation"),
            State.WorldLocation, FVector::ZeroVector);
        TestEqual(TEXT("After Reset DistanceToListener"),
            State.DistanceToListener, 0.0f);

        return true;
    }

    #endif // WITH_DEV_AUTOMATION_TESTS
    ```
  </action>
  <verify>
    - GSDInterfaceTests.cpp compiles without errors
    - Tests appear in Session Frontend
    - All tests pass when run
    - Build.cs updated for test dependencies
  </verify>
  <done>
    Interface tests exist for IGSDSpawnable, IGSDStreamable, IGSDComponentSpawnable, IGSDSpatialAudioStreamable. All tests pass in Session Frontend.
  </done>
</task>

<task type="auto">
  <name>T2: Create Mass Spawner Tests</name>
  <files>
    Plugins/GSD_Core/Source/GSD_Core/Private/Tests/GSDMassSpawnerTests.cpp
  </files>
  <action>
    Create automation tests for mass spawning functionality.

    GSDMassSpawnerTests.cpp:
    ```cpp
    #include "CoreMinimal.h"
    #include "Misc/AutomationTest.h"
    #include "Types/GSDSpawnTypes.h"

    #if WITH_DEV_AUTOMATION_TESTS

    // Test: FGSDSpawnTicket defaults
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDSpawnTicketDefaultsTest,
        "GSD.Core.SpawnTypes.FGSDSpawnTicket.Defaults",
        EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

    bool FGSDSpawnTicketDefaultsTest::RunTest(const FString& Parameters)
    {
        FGSDSpawnTicket Ticket;

        TestEqual(TEXT("Default Location"), Ticket.Location, FVector::ZeroVector);
        TestEqual(TEXT("Default Rotation"), Ticket.Rotation, FRotator::ZeroRotator);
        TestNull(TEXT("Default ActorClass"), Ticket.ActorClass);
        TestNull(TEXT("Default Config"), Ticket.Config);
        TestEqual(TEXT("Default Priority"), Ticket.Priority, 0);

        return true;
    }

    // Test: FGSDSeededSpawnTicket inheritance
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDSeededSpawnTicketTest,
        "GSD.Core.SpawnTypes.FGSDSeededSpawnTicket.Inheritance",
        EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

    bool FGSDSeededSpawnTicketTest::RunTest(const FString& Parameters)
    {
        FGSDSeededSpawnTicket Ticket;

        // Test base fields
        Ticket.Location = FVector(100.0f, 200.0f, 300.0f);
        TestEqual(TEXT("Location inherited"), Ticket.Location, FVector(100.0f, 200.0f, 300.0f));

        // Test seeded fields
        Ticket.SpawnSeed = 12345;
        Ticket.SpawnOrder = 42;
        Ticket.ParameterHash = 0xDEADBEEF;

        TestEqual(TEXT("SpawnSeed"), Ticket.SpawnSeed, 12345);
        TestEqual(TEXT("SpawnOrder"), Ticket.SpawnOrder, 42);
        TestEqual(TEXT("ParameterHash"), Ticket.ParameterHash, (uint32)0xDEADBEEF);

        return true;
    }

    // Test: FGSDSpawnComparator ordering
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDSpawnComparatorTest,
        "GSD.Core.SpawnTypes.FGSDSpawnComparator.Ordering",
        EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

    bool FGSDSpawnComparatorTest::RunTest(const FString& Parameters)
    {
        FGSDSpawnComparator Comparator;

        // Create tickets with different priorities
        FGSDSeededSpawnTicket LowPriority;
        LowPriority.Priority = 0;
        LowPriority.SpawnOrder = 1;

        FGSDSeededSpawnTicket HighPriority;
        HighPriority.Priority = 10;
        HighPriority.SpawnOrder = 2;

        // Higher priority should come first
        TestTrue(TEXT("High priority before low priority"),
            Comparator(HighPriority, LowPriority));

        // Same priority, lower spawn order wins
        FGSDSeededSpawnTicket SamePriorityEarly;
        SamePriorityEarly.Priority = 5;
        SamePriorityEarly.SpawnOrder = 1;

        FGSDSeededSpawnTicket SamePriorityLate;
        SamePriorityLate.Priority = 5;
        SamePriorityLate.SpawnOrder = 10;

        TestTrue(TEXT("Same priority - lower spawn order wins"),
            Comparator(SamePriorityEarly, SamePriorityLate));

        return true;
    }

    // Test: FGSDAudioLODConfig distance calculation
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDAudioLODConfigTest,
        "GSD.Core.Audio.LODConfig.DistanceCalculation",
        EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

    bool FGSDAudioLODConfigTest::RunTest(const FString& Parameters)
    {
        FGSDAudioLODConfig Config;

        // Test default distances
        TestEqual(TEXT("LOD0 distance"), Config.LOD0Distance, 500.0f);
        TestEqual(TEXT("LOD1 distance"), Config.LOD1Distance, 2000.0f);
        TestEqual(TEXT("LOD2 distance"), Config.LOD2Distance, 5000.0f);

        // Test LOD level calculation
        TestEqual(TEXT("Distance 100 -> LOD0"), Config.GetLODLevelForDistance(100.0f), 0);
        TestEqual(TEXT("Distance 1000 -> LOD1"), Config.GetLODLevelForDistance(1000.0f), 1);
        TestEqual(TEXT("Distance 3000 -> LOD2"), Config.GetLODLevelForDistance(3000.0f), 2);
        TestEqual(TEXT("Distance 10000 -> Virtual"), Config.GetLODLevelForDistance(10000.0f), -1);

        return true;
    }

    #endif // WITH_DEV_AUTOMATION_TESTS
    ```
  </action>
  <verify>
    - GSDMassSpawnerTests.cpp compiles without errors
    - Tests appear in Session Frontend
    - All tests pass
    - Comparator ordering verified
  </verify>
  <done>
    Mass spawner tests exist for FGSDSpawnTicket, FGSDSeededSpawnTicket, FGSDSpawnComparator, and FGSDAudioLODConfig.
  </done>
</task>

<task type="auto">
  <name>T3: Create Determinism Tests</name>
  <files>
    Plugins/GSD_Core/Source/GSD_Core/Private/Tests/GSDDeterminismTests.cpp
  </files>
  <action>
    Create automation tests for determinism manager.

    GSDDeterminismTests.cpp:
    ```cpp
    #include "CoreMinimal.h"
    #include "Misc/AutomationTest.h"
    #include "Managers/GSDDeterminismManager.h"
    #include "Engine/GameInstance.h"

    #if WITH_DEV_AUTOMATION_TESTS

    // Test: Determinism manager seeding
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDDeterminismManagerSeedingTest,
        "GSD.Core.Determinism.Seeding",
        EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

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
        EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

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
        EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

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
        EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

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
    ```
  </action>
  <verify>
    - GSDDeterminismTests.cpp compiles without errors
    - Tests appear in Session Frontend
    - All tests pass
    - Same seed produces same sequence verified
  </verify>
  <done>
    Determinism tests exist for seeding, reproducibility, category isolation, and random integer range.
  </done>
</task>

<task type="auto">
  <name>T4: Create Performance Config Tests</name>
  <files>
    Plugins/GSD_Core/Source/GSD_Core/Private/Tests/GSDPerformanceTests.cpp
  </files>
  <action>
    Create automation tests for performance configuration.

    GSDPerformanceTests.cpp:
    ```cpp
    #include "CoreMinimal.h"
    #include "Misc/AutomationTest.h"
    #include "Types/GSDPerformanceConfig.h"

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
    ```
  </action>
  <verify>
    - GSDPerformanceTests.cpp compiles without errors
    - Tests appear in Session Frontend
    - All tests pass
    - Budget checking verified
  </verify>
  <done>
    Performance tests exist for UGSDPerformanceConfig defaults, budget checking, UGSDSaveGame operations, and FGSDTickContext.
  </done>
</task>

---

## Verification Checklist

- [ ] All test files compile without errors
- [ ] Tests appear in Session Frontend under GSD.Core
- [ ] Interface tests pass
- [ ] Mass spawner tests pass
- [ ] Determinism tests pass
- [ ] Performance config tests pass
- [ ] Build.cs updated for test dependencies
- [ ] Test coverage > 80% for core interfaces

---

## Success Criteria

| Criterion | Target |
|-----------|--------|
| Tests compile | 100% |
| Tests pass | 100% |
| Coverage | >80% |
| Automation ready | Yes |

---

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| Test dependencies | Low | Medium | Build.cs conditional include |
| Non-deterministic tests | Low | Medium | Use seeded RNG in tests |

---

## Estimated Complexity

**Medium** - Multiple test files with automation framework

---

## Output

After completion, create `.planning/phases/01-plugin-architecture-foundation/07-SUMMARY.md`
