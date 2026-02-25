---
phase: 01-plugin-architecture-foundation
plan: 06
type: execute
wave: 4
depends_on: ["01", "02", "05"]
files_modified:
  - Plugins/GSD_Core/Source/GSD_Core/Public/Types/GSDPerformanceConfig.h
  - Plugins/GSD_Core/Source/GSD_Core/Private/Types/GSDPerformanceConfig.cpp
  - Plugins/GSD_Core/Source/GSD_Core/Public/Types/GSDSaveGame.h
  - Plugins/GSD_Core/Source/GSD_Core/Private/Types/GSDSaveGame.cpp
  - Plugins/GSD_Core/Source/GSD_Core/Public/Managers/GSDDeterminismManager.h
  - Plugins/GSD_Core/Source/GSD_Core/Private/Managers/GSDDeterminismManager.cpp
  - Config/Scalability.ini
autonomous: true
must_haves:
  truths:
    - "Performance budgets are defined and accessible"
    - "Spawning respects frame time budget (2ms)"
    - "Determinism manager provides seeded RNG streams"
    - "SaveGame schema supports spawn state serialization"
  artifacts:
    - path: "Plugins/GSD_Core/Source/GSD_Core/Public/Types/GSDPerformanceConfig.h"
      provides: "Performance budget configuration"
      contains: "SpawningFrameBudgetMs"
    - path: "Plugins/GSD_Core/Source/GSD_Core/Public/Managers/GSDDeterminismManager.h"
      provides: "Seeded RNG for reproducible runs"
      contains: "UGSDDeterminismManager"
    - path: "Plugins/GSD_Core/Source/GSD_Core/Public/Types/GSDSaveGame.h"
      provides: "SaveGame schema for state persistence"
      contains: "UGSDSaveGame"
  key_links:
    - from: "UGSDDeterminismManager"
      to: "GameInstance"
      via: "Subsystem"
      pattern: "UGameInstanceSubsystem"
    - from: "UGSDPerformanceConfig"
      to: "DeveloperSettings"
      via: "Inheritance"
      pattern: "UDeveloperSettings"
---

# 06-PLAN: Performance, Determinism, and SaveGame

**Requirements:** PLUG-08

**Objective:** Implement performance configuration, determinism manager subsystem, and SaveGame schema for the GSD platform.

**Purpose:** Provide performance budgets for spawning operations, seeded RNG for reproducible runs, and state serialization for save/load functionality.

**Output:** UGSDPerformanceConfig, UGSDDeterminismManager, and UGSDSaveGame classes.

---

## Context

@.planning/PROJECT.md
@.planning/ROADMAP.md
@.planning/phases/01-plugin-architecture-foundation/01-RESEARCH.md
@.planning/phases/01-plugin-architecture-foundation/1-CONTEXT.md

---

## Tasks

<task type="auto">
  <name>T1: Implement Performance Configuration</name>
  <files>
    Plugins/GSD_Core/Source/GSD_Core/Public/Types/GSDPerformanceConfig.h
    Plugins/GSD_Core/Source/GSD_Core/Private/Types/GSDPerformanceConfig.cpp
    Config/Scalability.ini
  </files>
  <action>
    Create the performance budget configuration as developer settings.

    GSDPerformanceConfig.h:
    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "Engine/DeveloperSettings.h"
    #include "GSDPerformanceConfig.generated.h"

    /**
     * Performance budget configuration for the GSD platform.
     * Access via GetDefault<UGSDPerformanceConfig>().
     */
    UCLASS(Config = Game, DefaultConfig, Category = "GSD")
    class GSD_CORE_API UGSDPerformanceConfig : public UDeveloperSettings
    {
        GENERATED_BODY()

    public:
        UGSDPerformanceConfig();

        // ~ Spawning budgets
        UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning",
            meta = (ClampMin = "0.1", ClampMax = "10.0", Units = "ms"))
        float SpawningFrameBudgetMs = 2.0f;

        UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning",
            meta = (ClampMin = "1", ClampMax = "50"))
        int32 MaxSpawnsPerFrame = 10;

        UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning",
            meta = (ClampMin = "1", ClampMax = "1000"))
        int32 BatchSpawnChunkSize = 20;

        // ~ Memory budgets
        UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Memory",
            meta = (ClampMin = "64", ClampMax = "4096", Units = "MB"))
        float EntityMemoryBudgetMB = 512.0f;

        UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Memory",
            meta = (ClampMin = "10", ClampMax = "500"))
        int32 MaxPooledEntities = 100;

        // ~ Audio budgets
        UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Audio",
            meta = (ClampMin = "0.1", ClampMax = "10.0", Units = "ms"))
        float AudioBudgetMs = 2.0f;

        UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Audio",
            meta = (ClampMin = "8", ClampMax = "128"))
        int32 MaxConcurrentAudioSources = 32;

        // ~ Hitch thresholds
        UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Performance",
            meta = (ClampMin = "8.0", ClampMax = "33.0", Units = "ms"))
        float HitchThresholdMs = 16.67f; // 60fps target

        UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Performance",
            meta = (ClampMin = "30", ClampMax = "120"))
        float TargetFPS = 60.0f;

        // ~ Debug settings
        UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
        bool bLogSpawnPerformance = false;

        UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
        bool bLogMemoryUsage = false;

        UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
        bool bWarnOnBudgetExceeded = true;

        // Helper functions
        UFUNCTION(BlueprintPure, Category = "GSD|Performance")
        bool IsWithinSpawnBudget(float CurrentFrameTimeMs) const;

        UFUNCTION(BlueprintPure, Category = "GSD|Performance")
        bool IsHitch(float FrameTimeMs) const;

        UFUNCTION(BlueprintPure, Category = "GSD|Performance")
        float GetFrameBudgetRemaining(float CurrentFrameTimeMs) const;
    };
    ```

    GSDPerformanceConfig.cpp:
    ```cpp
    #include "Types/GSDPerformanceConfig.h"

    UGSDPerformanceConfig::UGSDPerformanceConfig()
    {
        CategoryName = TEXT("GSD");
        SectionName = TEXT("Performance");
    }

    bool UGSDPerformanceConfig::IsWithinSpawnBudget(float CurrentFrameTimeMs) const
    {
        return CurrentFrameTimeMs <= SpawningFrameBudgetMs;
    }

    bool UGSDPerformanceConfig::IsHitch(float FrameTimeMs) const
    {
        return FrameTimeMs > HitchThresholdMs;
    }

    float UGSDPerformanceConfig::GetFrameBudgetRemaining(float CurrentFrameTimeMs) const
    {
        return FMath::Max(0.0f, SpawningFrameBudgetMs - CurrentFrameTimeMs);
    }
    ```

    Config/Scalability.ini:
    ```ini
    ; GSD Scalability Settings
    ; These integrate with UE5's scalability system

    [Spawning@Low]
    MaxActiveEntities=50
    MaxSpawnsPerFrame=3
    AudioLOD=2

    [Spawning@Medium]
    MaxActiveEntities=100
    MaxSpawnsPerFrame=5
    AudioLOD=1

    [Spawning@High]
    MaxActiveEntities=200
    MaxSpawnsPerFrame=10
    AudioLOD=0

    [Spawning@Epic]
    MaxActiveEntities=300
    MaxSpawnsPerFrame=15
    AudioLOD=0

    [Spawning@Cinematic]
    MaxActiveEntities=500
    MaxSpawnsPerFrame=20
    AudioLOD=0
    ```
  </action>
  <verify>
    - UGSDPerformanceConfig compiles without errors
    - Settings appear in Project Settings > GSD
    - Default values are applied
    - Scalability.ini is valid
    - IsWithinSpawnBudget works correctly
  </verify>
  <done>
    UGSDPerformanceConfig exists with spawning, memory, audio budgets. Settings appear in Project Settings > GSD. Scalability.ini provides scalability tiers.
  </done>
</task>

<task type="auto">
  <name>T2: Implement Determinism Manager Subsystem</name>
  <files>
    Plugins/GSD_Core/Source/GSD_Core/Public/Managers/GSDDeterminismManager.h
    Plugins/GSD_Core/Source/GSD_Core/Private/Managers/GSDDeterminismManager.cpp
  </files>
  <action>
    Create the determinism manager as a GameInstance subsystem for seeded RNG.

    GSDDeterminismManager.h:
    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "Subsystems/GameInstanceSubsystem.h"
    #include "GSDDeterminismManager.generated.h"

    /**
     * Determinism Manager Subsystem
     * Provides seeded RNG streams for reproducible runs.
     * MUST be first system initialized.
     */
    UCLASS()
    class GSD_CORE_API UGSDDeterminismManager : public UGameInstanceSubsystem
    {
        GENERATED_BODY()

    public:
        virtual void Initialize(FSubsystemCollectionBase& Collection) override;
        virtual void Deinitialize() override;

        // Initialize with a specific seed
        UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
        void InitializeWithSeed(int32 InSeed);

        // Get category-specific RNG stream (isolated from other categories)
        UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
        FRandomStream& GetStream(FName Category);

        // Get the current seed
        UFUNCTION(BlueprintPure, Category = "GSD|Determinism")
        int32 GetCurrentSeed() const { return CurrentSeed; }

        // Compute state hash for verification
        UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
        int32 ComputeStateHash() const;

        // Get random float from category stream
        UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
        float RandomFloat(FName Category);

        // Get random integer from category stream
        UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
        int32 RandomInteger(FName Category, int32 Max);

        // Get random bool from category stream
        UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
        bool RandomBool(FName Category);

        // Get random vector from category stream
        UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
        FVector RandomUnitVector(FName Category);

        // Shuffle array using category stream
        template<typename T>
        void ShuffleArray(FName Category, TArray<T>& Array);

        // Reset a specific category stream
        UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
        void ResetStream(FName Category);

        // Reset all streams
        UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
        void ResetAllStreams();

        // Predefined categories
        static const FName SpawnCategory;
        static const FName EventCategory;
        static const FName CrowdCategory;
        static const FName VehicleCategory;

    private:
        int32 CurrentSeed = 0;
        TMap<FName, FRandomStream> CategoryStreams;

        // Hash accumulator for state verification
        UPROPERTY()
        int32 StateHash = 0;

        void CreateCategoryStream(FName Category);
    };
    ```

    GSDDeterminismManager.cpp:
    ```cpp
    #include "Managers/GSDDeterminismManager.h"
    #include "GSDLog.h"

    const FName UGSDDeterminismManager::SpawnCategory = TEXT("Spawn");
    const FName UGSDDeterminismManager::EventCategory = TEXT("Event");
    const FName UGSDDeterminismManager::CrowdCategory = TEXT("Crowd");
    const FName UGSDDeterminismManager::VehicleCategory = TEXT("Vehicle");

    void UGSDDeterminismManager::Initialize(FSubsystemCollectionBase& Collection)
    {
        GSD_LOG(Log, "UGSDDeterminismManager initialized");

        // Create default category streams
        CreateCategoryStream(SpawnCategory);
        CreateCategoryStream(EventCategory);
        CreateCategoryStream(CrowdCategory);
        CreateCategoryStream(VehicleCategory);
    }

    void UGSDDeterminismManager::Deinitialize()
    {
        CategoryStreams.Empty();
        GSD_LOG(Log, "UGSDDeterminismManager deinitialized");
    }

    void UGSDDeterminismManager::InitializeWithSeed(int32 InSeed)
    {
        CurrentSeed = InSeed;
        StateHash = 0;

        GSD_LOG(Log, "UGSDDeterminismManager seeded with %d", CurrentSeed);

        // Reset and reseed all streams
        for (auto& Pair : CategoryStreams)
        {
            // Derive category-specific seed from main seed + category hash
            int32 CategorySeed = CurrentSeed + GetTypeHash(Pair.Key);
            Pair.Value.Initialize(CategorySeed);
        }
    }

    FRandomStream& UGSDDeterminismManager::GetStream(FName Category)
    {
        if (!CategoryStreams.Contains(Category))
        {
            CreateCategoryStream(Category);
        }
        return CategoryStreams[Category];
    }

    int32 UGSDDeterminismManager::ComputeStateHash() const
    {
        return StateHash;
    }

    float UGSDDeterminismManager::RandomFloat(FName Category)
    {
        float Value = GetStream(Category).GetFraction();
        StateHash = HashCombine(StateHash, GetTypeHash(Value));
        return Value;
    }

    int32 UGSDDeterminismManager::RandomInteger(FName Category, int32 Max)
    {
        int32 Value = GetStream(Category).RandHelper(Max);
        StateHash = HashCombine(StateHash, Value);
        return Value;
    }

    bool UGSDDeterminismManager::RandomBool(FName Category)
    {
        bool Value = GetStream(Category).GetFraction() > 0.5f;
        StateHash = HashCombine(StateHash, GetTypeHash(Value));
        return Value;
    }

    FVector UGSDDeterminismManager::RandomUnitVector(FName Category)
    {
        FVector Value = GetStream(Category).VRand();
        StateHash = HashCombine(StateHash, GetTypeHash(Value));
        return Value;
    }

    template<typename T>
    void UGSDDeterminismManager::ShuffleArray(FName Category, TArray<T>& Array)
    {
        FRandomStream& Stream = GetStream(Category);
        for (int32 i = Array.Num() - 1; i > 0; --i)
        {
            int32 j = Stream.RandHelper(i + 1);
            Array.Swap(i, j);
        }
        StateHash = HashCombine(StateHash, Array.Num());
    }

    void UGSDDeterminismManager::ResetStream(FName Category)
    {
        if (CategoryStreams.Contains(Category))
        {
            int32 CategorySeed = CurrentSeed + GetTypeHash(Category);
            CategoryStreams[Category].Initialize(CategorySeed);
            GSD_LOG(Verbose, "Reset stream for category %s", *Category.ToString());
        }
    }

    void UGSDDeterminismManager::ResetAllStreams()
    {
        for (auto& Pair : CategoryStreams)
        {
            int32 CategorySeed = CurrentSeed + GetTypeHash(Pair.Key);
            Pair.Value.Initialize(CategorySeed);
        }
        GSD_LOG(Log, "Reset all streams with seed %d", CurrentSeed);
    }

    void UGSDDeterminismManager::CreateCategoryStream(FName Category)
    {
        int32 CategorySeed = CurrentSeed + GetTypeHash(Category);
        FRandomStream NewStream(CategorySeed);
        CategoryStreams.Add(Category, NewStream);
    }

    // Explicit template instantiation for common types
    template void UGSDDeterminismManager::ShuffleArray<int32>(FName, TArray<int32>&);
    template void UGSDDeterminismManager::ShuffleArray<FString>(FName, TArray<FString>&);
    ```
  </action>
  <verify>
    - UGSDDeterminismManager compiles without errors
    - Subsystem initializes with GameInstance
    - GetStream returns valid FRandomStream
    - Same seed produces same random sequence
    - Category streams are isolated
  </verify>
  <done>
    UGSDDeterminismManager exists as GameInstanceSubsystem with seeded RNG streams, category isolation, and state hash verification.
  </done>
</task>

<task type="auto">
  <name>T3: Implement SaveGame Schema</name>
  <files>
    Plugins/GSD_Core/Source/GSD_Core/Public/Types/GSDSaveGame.h
    Plugins/GSD_Core/Source/GSD_Core/Private/Types/GSDSaveGame.cpp
  </files>
  <action>
    Create the SaveGame schema for spawn state serialization.

    GSDSaveGame.h:
    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "GameFramework/SaveGame.h"
    #include "Types/GSDSpawnTypes.h"
    #include "GSDSaveGame.generated.h"

    /**
     * Serialized actor state for save/load
     */
    USTRUCT(BlueprintType)
    struct GSD_CORE_API FGSDSerializedActorState
    {
        GENERATED_BODY()

        UPROPERTY(SaveGame)
        FName ActorName;

        UPROPERTY(SaveGame)
        FTransform Transform;

        UPROPERTY(SaveGame)
        TArray<uint8> ComponentState;

        UPROPERTY(SaveGame)
        FString ActorClassPath;

        UPROPERTY(SaveGame)
        bool bIsActive = true;
    };

    /**
     * Random call log for debugging determinism issues
     */
    USTRUCT(BlueprintType)
    struct GSD_CORE_API FGSDRandomCallLog
    {
        GENERATED_BODY()

        UPROPERTY(SaveGame)
        FName Category;

        UPROPERTY(SaveGame)
        int32 CallCount = 0;

        UPROPERTY(SaveGame)
        int32 LastHash = 0;
    };

    /**
     * Main SaveGame object for the GSD platform
     */
    UCLASS(BlueprintType)
    class GSD_CORE_API UGSDSaveGame : public USaveGame
    {
        GENERATED_BODY()

    public:
        UGSDSaveGame();

        // ~ Game identification
        UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD")
        FString SaveName;

        UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD")
        int32 SaveVersion = 1;

        UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD")
        FDateTime SaveTimestamp;

        // ~ Determinism
        UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD|Determinism")
        int32 GameSeed;

        UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD|Determinism")
        int32 DeterminismHash;

        // ~ Spawn state
        UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD|Spawning")
        TArray<FGSDSeededSpawnTicket> PendingSpawns;

        UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD|Spawning")
        TArray<FGSDSerializedActorState> ActorStates;

        UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD|Spawning")
        int32 TotalSpawnCount = 0;

        // ~ Random history for debugging
        UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD|Debug")
        TArray<FGSDRandomCallLog> RandomHistory;

        // ~ Game time
        UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD|Game")
        float GameTime = 0.0f;

        UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD|Game")
        int32 GameDay = 1;

        // ~ Custom data (for game-specific data)
        UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD|Custom")
        TArray<uint8> CustomData;

        // Helper functions
        UFUNCTION(BlueprintCallable, Category = "GSD|Save")
        void AddActorState(const FGSDSerializedActorState& State);

        UFUNCTION(BlueprintCallable, Category = "GSD|Save")
        bool GetActorState(FName ActorName, FGSDSerializedActorState& OutState) const;

        UFUNCTION(BlueprintCallable, Category = "GSD|Save")
        void RemoveActorState(FName ActorName);

        UFUNCTION(BlueprintCallable, Category = "GSD|Save")
        void ClearAllStates();

        UFUNCTION(BlueprintPure, Category = "GSD|Save")
        int32 GetActorStateCount() const { return ActorStates.Num(); }

        UFUNCTION(BlueprintCallable, Category = "GSD|Save")
        void RecordRandomCall(FName Category, int32 Hash);

        UFUNCTION(BlueprintPure, Category = "GSD|Save")
        bool ValidateDeterminism(int32 ExpectedHash) const;
    };
    ```

    GSDSaveGame.cpp:
    ```cpp
    #include "Types/GSDSaveGame.h"

    UGSDSaveGame::UGSDSaveGame()
    {
        SaveName = TEXT("GSDSave");
        SaveTimestamp = FDateTime::Now();
    }

    void UGSDSaveGame::AddActorState(const FGSDSerializedActorState& State)
    {
        // Check if actor already exists
        for (int32 i = 0; i < ActorStates.Num(); ++i)
        {
            if (ActorStates[i].ActorName == State.ActorName)
            {
                ActorStates[i] = State;
                return;
            }
        }

        // Add new state
        ActorStates.Add(State);
    }

    bool UGSDSaveGame::GetActorState(FName ActorName, FGSDSerializedActorState& OutState) const
    {
        for (const FGSDSerializedActorState& State : ActorStates)
        {
            if (State.ActorName == ActorName)
            {
                OutState = State;
                return true;
            }
        }
        return false;
    }

    void UGSDSaveGame::RemoveActorState(FName ActorName)
    {
        ActorStates.RemoveAll([ActorName](const FGSDSerializedActorState& State)
        {
            return State.ActorName == ActorName;
        });
    }

    void UGSDSaveGame::ClearAllStates()
    {
        ActorStates.Empty();
        PendingSpawns.Empty();
        RandomHistory.Empty();
        TotalSpawnCount = 0;
    }

    void UGSDSaveGame::RecordRandomCall(FName Category, int32 Hash)
    {
        for (FGSDRandomCallLog& Log : RandomHistory)
        {
            if (Log.Category == Category)
            {
                Log.CallCount++;
                Log.LastHash = Hash;
                return;
            }
        }

        FGSDRandomCallLog NewLog;
        NewLog.Category = Category;
        NewLog.CallCount = 1;
        NewLog.LastHash = Hash;
        RandomHistory.Add(NewLog);
    }

    bool UGSDSaveGame::ValidateDeterminism(int32 ExpectedHash) const
    {
        return DeterminismHash == ExpectedHash;
    }
    ```
  </action>
  <verify>
    - UGSDSaveGame compiles without errors
    - FGSDSerializedActorState struct compiles
    - FGSDRandomCallLog struct compiles
    - AddActorState/GetActorState work correctly
    - SaveGame can be created and serialized
  </verify>
  <done>
    UGSDSaveGame exists with actor state serialization, pending spawns, determinism tracking, and random history for debugging.
  </done>
</task>

---

## Verification Checklist

- [ ] UGSDPerformanceConfig appears in Project Settings
- [ ] Default budget values are applied (2ms spawn, 512MB memory)
- [ ] Scalability.ini has all tier definitions
- [ ] UGSDDeterminismManager initializes with GameInstance
- [ ] Same seed produces same random sequence
- [ ] Category streams are isolated from each other
- [ ] UGSDSaveGame serializes correctly
- [ ] Actor states can be added/retrieved/removed
- [ ] Random history is recorded

---

## Success Criteria

| Criterion | Target |
|-----------|--------|
| Performance config | Accessible |
| Determinism manager | Works |
| SaveGame schema | Complete |
| Budget enforcement | Ready |

---

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| Subsystem initialization order | Low | Medium | GameInstance subsystems initialize early |
| SaveGame version migration | Low | Medium | Version field included |
| Hash collisions | Low | Low | Use 32-bit hash, accept small collision risk |

---

## Estimated Complexity

**Medium** - Subsystem, developer settings, and SaveGame

---

## Output

After completion, create `.planning/phases/01-plugin-architecture-foundation/06-SUMMARY.md`
