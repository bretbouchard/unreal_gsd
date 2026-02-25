# Phase 1 Context: Plugin Architecture Foundation

**Phase Goal:** Platform provides clean plugin structure with core interfaces that all feature plugins depend on

**Requirements:** PLUG-01, PLUG-08, PLUG-09, PLUG-10

**Last Updated:** 2026-02-24 (Council Review - All Issues Addressed)

---

## Philosophy: Opinionated & Rigid

The platform has a way. You follow it. You ship faster.

- Optimized for speed, not flexibility
- One way to do things, not multiple options
- Game developers push pixels and make games, platform handles the rest

---

## Performance Budgets

> **Council Issue Addressed**: unreal-performance-rick CRITICAL - Missing Performance Budget Definition

All spawning and streaming operations MUST respect these budgets:

| Budget | Value | Rationale |
|--------|-------|-----------|
| Spawning Frame Budget | 2.0 ms | Max time per frame for spawn work |
| Max Spawns Per Frame | 10 | Cap spawns even if under time budget |
| Entity Memory Budget | 512 MB | Per-entity-type memory limit |
| Max Pooled Entities | 100 | Pool ceiling per entity type |
| Audio Budget | 2.0 ms | Audio processing per frame |
| Hitch Threshold | 16.67 ms | 60fps target - any frame over this is a hitch |

```cpp
// GSDPerformanceConfig.h
UCLASS(Config=Game, DefaultConfig)
class UGSDPerformanceConfig : public UDeveloperSettings
{
    GENERATED_BODY()

    // Spawning budgets
    UPROPERTY(Config, EditDefaultsOnly, Category = "Spawning")
    float SpawningFrameBudgetMs = 2.0f;

    UPROPERTY(Config, EditDefaultsOnly, Category = "Spawning")
    int32 MaxSpawnsPerFrame = 10;

    // Memory budgets
    UPROPERTY(Config, EditDefaultsOnly, Category = "Memory")
    float EntityMemoryBudgetMB = 512.0f;

    UPROPERTY(Config, EditDefaultsOnly, Category = "Memory")
    int32 MaxPooledEntities = 100;
};
```

### Scalability Tiers

> **Council Issue Addressed**: unreal-performance-rick MEDIUM - Missing Scalability Tiers

```ini
; Config/Scalability.ini

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

### Profiling Verification Strategy

> **Council Issue Addressed**: unreal-performance-rick MEDIUM - No Profiling Verification Strategy

| Test | Pass Criteria | Tool |
|------|---------------|------|
| Frame Time | GameThread < 8ms during spawn bursts | `stat unit` |
| FPS Stability | No drops below 55fps (60fps target) | `stat fps` |
| Spawn Benchmark | 100 entities spawn with no frame > 20ms | Automated test |
| Memory Growth | No memory growth over 30min spawn/despawn cycle | `stat memory` |
| Trace Capture | Required for every PR touching spawning code | Unreal Insights |

---

## Interface Design

### IGSDSpawnable (Actor Spawning)

**Scope:** Spawning + initialization for actors

```cpp
// IGSDSpawnable.h
UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
class UGSDSpawnable : public UInterface
{
    GENERATED_BODY()
};

class GSD_CORE_API IGSDSpawnable
{
    GENERATED_BODY()

public:
    // Async spawn with config and callback
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Spawning")
    void SpawnAsync(UGSDDataAsset* Config, FOnSpawnComplete Callback);
    virtual void SpawnAsync_Implementation(UGSDDataAsset* Config, FOnSpawnComplete Callback);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Spawning")
    UGSDDataAsset* GetSpawnConfig();
    virtual UGSDDataAsset* GetSpawnConfig_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Spawning")
    bool IsSpawned();
    virtual bool IsSpawned_Implementation();
};
```

**Key decisions:**
- **Async only** — consistent behavior, no hitches on large spawns (200+ zombies)
- **Blueprint-friendly** — expose to visual scripting from day one
- **Config-driven** — all spawn data passed in, no hardcoded values

### IGSDComponentSpawnable (Component Spawning)

> **Council Issue Addressed**: unreal-audio-rick CRITICAL - No Audio Component Spawning Support

**Scope:** Spawning for non-actor components (audio, particles, etc.)

```cpp
// IGSDComponentSpawnable.h
UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
class UGSDComponentSpawnable : public UInterface
{
    GENERATED_BODY()
};

class GSD_CORE_API IGSDComponentSpawnable
{
    GENERATED_BODY()

public:
    // Spawn component on owner actor
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Components")
    UActorComponent* SpawnComponent(AActor* Owner, const FTransform& Transform);
    virtual UActorComponent* SpawnComponent_Implementation(AActor* Owner, const FTransform& Transform);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Components")
    FName GetComponentType() const;
    virtual FName GetComponentType_Implementation() const;
};
```

### IGSDMassSpawnerInterface (Batch Spawning)

> **Council Issue Addressed**: unreal-swarm-rick CRITICAL - No Batch Spawning Strategy

**Scope:** Batch spawning for 200+ entities (zombies, crowds)

```cpp
// IGSDMassSpawnerInterface.h
UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
class UGSDMassSpawnerInterface : public UInterface
{
    GENERATED_BODY()
};

class GSD_CORE_API IGSDMassSpawnerInterface
{
    GENERATED_BODY()

public:
    // Batch spawn - single delegate for entire batch
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Mass")
    void SpawnBatch(
        const TArray<FTransform>& Transforms,
        UGSDDataAsset* Config,
        FOnMassSpawnComplete OnComplete
    );
    virtual void SpawnBatch_Implementation(
        const TArray<FTransform>& Transforms,
        UGSDDataAsset* Config,
        FOnMassSpawnComplete OnComplete
    );

    // Chunked spawning to avoid callback storms
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Mass")
    void SpawnBatchChunked(
        const TArray<FTransform>& Transforms,
        UGSDDataAsset* Config,
        int32 ChunkSize,
        FOnMassSpawnChunkComplete OnChunkComplete
    );
    virtual void SpawnBatchChunked_Implementation(
        const TArray<FTransform>& Transforms,
        UGSDDataAsset* Config,
        int32 ChunkSize,
        FOnMassSpawnChunkComplete OnChunkComplete
    );

    // Get current spawn queue status
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Mass")
    int32 GetPendingSpawnCount() const;
    virtual int32 GetPendingSpawnCount_Implementation() const;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMassSpawnComplete, const TArray<AActor*>&, SpawnedActors);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMassSpawnChunkComplete, int32, ChunkIndex, int32, TotalChunks);
```

### IGSDStreamable

**Scope:** Full streaming control

```cpp
// IGSDStreamable.h
UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
class UGSDStreamable : public UInterface
{
    GENERATED_BODY()
};

class GSD_CORE_API IGSDStreamable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Streaming")
    FBoxSphereBounds GetStreamingBounds();
    virtual FBoxSphereBounds GetStreamingBounds_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Streaming")
    void OnStreamIn();
    virtual void OnStreamIn_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Streaming")
    void OnStreamOut();
    virtual void OnStreamOut_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Streaming")
    int32 GetStreamingPriority();
    virtual int32 GetStreamingPriority_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Streaming")
    bool ShouldPersist();
    virtual bool ShouldPersist_Implementation();
};
```

**Key decisions:**
- **Blueprint-friendly** — expose to visual scripting from day one
- **Full control** — distance, callbacks, persistence, priority

### IGSDSpatialAudioStreamable

> **Council Issue Addressed**: unreal-audio-rick CRITICAL - No Spatial Audio in IGSDStreamable

**Scope:** Audio-specific streaming with spatial state preservation

```cpp
// IGSDSpatialAudioStreamable.h
USTRUCT(BlueprintType)
struct FGSDSpatialAudioState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    float DistanceToListener = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float OcclusionFactor = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float ReverbSendLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bIsOccluded = false;
};

UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
class UGSDSpatialAudioStreamable : public UGSDStreamable
{
    GENERATED_BODY()
};

class GSD_CORE_API IGSDSpatialAudioStreamable : public IGSDStreamable
{
    GENERATED_BODY()

public:
    // Get spatial state before stream out
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
    FGSDSpatialAudioState GetSpatialState() const;
    virtual FGSDSpatialAudioState GetSpatialState_Implementation() const;

    // Restore spatial state on stream in
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
    void RestoreSpatialState(const FGSDSpatialAudioState& State);
    virtual void RestoreSpatialState_Implementation(const FGSDSpatialAudioState& State);

    // Distance threshold for audio LOD
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
    float GetAudioLODDistance() const;
    virtual float GetAudioLODDistance_Implementation() const;
};
```

---

## Network Architecture

> **Council Issue Addressed**: unreal-network-rick CRITICAL - No Network Replication Architecture

### Network Scope Decision

**Zombie Taxi v1.0: Single-Player Only**
- Network hooks are included for future expansion
- No replication overhead in single-player builds
- Architecture supports multiplayer in future versions

### Network Spawn Parameters

```cpp
// GSDSpawnTypes.h
USTRUCT(BlueprintType)
struct FGSDNetworkSpawnParams
{
    GENERATED_BODY()

    // Replication settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bReplicates = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENetRole RemoteRole = ROLE_SimulatedProxy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NetUpdateFrequency = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NetCullDistanceSquared = 10000.0f * 10000.0f; // 100m

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* Owner = nullptr; // For replication ownership

    // Server-authoritative validation
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bServerAuthoritative = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    APlayerController* RequestingController = nullptr;
};

// Extended spawn config with network params
USTRUCT(BlueprintType)
struct FGSDSpawnTicket
{
    GENERATED_BODY()

    // Transform
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation = FRotator::ZeroRotator;

    // Actor class
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AActor> ActorClass;

    // Config
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UGSDDataAsset* Config = nullptr;

    // Network params
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGSDNetworkSpawnParams NetworkParams;

    // Priority (static, not frame-dependent)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority = 0;
};
```

### Network Validation Interface

```cpp
// IGSDNetworkSpawnValidator.h
UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
class UGSDNetworkSpawnValidator : public UInterface
{
    GENERATED_BODY()
};

class GSD_CORE_API IGSDNetworkSpawnValidator
{
    GENERATED_BODY()

public:
    // Called on server to validate spawn request
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Network")
    bool ValidateSpawnRequest(const FGSDSpawnTicket& Request, FString& OutRejectionReason);
    virtual bool ValidateSpawnRequest_Implementation(const FGSDSpawnTicket& Request, FString& OutRejectionReason);

    // Called when spawn completes to notify clients
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Network")
    void NotifySpawnComplete(AActor* SpawnedActor);
    virtual void NotifySpawnComplete_Implementation(AActor* SpawnedActor);
};
```

---

## Determinism Architecture

> **Council Issue Addressed**: unreal-determinism-rick CRITICAL - Missing Seeded RNG Foundation

### Determinism Manager Subsystem

```cpp
// GSDDeterminismManager.h
UCLASS()
class UGSDDeterminismManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // MUST be first system initialized
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // Initialize with daily seed
    UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
    void InitializeWithSeed(int32 InSeed);

    // Category-specific streams for isolation
    UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
    FRandomStream& GetStream(FName Category);

    // Hash verification for state validation
    UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
    uint32 ComputeStateHash() const;

    // Get current seed
    UFUNCTION(BlueprintPure, Category = "GSD|Determinism")
    int32 GetCurrentSeed() const { return CurrentSeed; }

private:
    int32 CurrentSeed = 0;
    TMap<FName, FRandomStream> CategoryStreams;
};
```

### Seeded Spawn Ticket

```cpp
// Extended FGSDSpawnTicket with determinism fields
USTRUCT(BlueprintType)
struct FGSDSeededSpawnTicket : public FGSDSpawnTicket
{
    GENERATED_BODY()

    // Derived from global seed + spawn index
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SpawnSeed = 0;

    // Deterministic spawn sequence
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SpawnOrder = -1;

    // Hash of spawn params for validation
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint32 ParameterHash = 0;
};
```

### Deterministic Spawn Ordering

```cpp
// Spawn comparator for deterministic ordering
struct FGSDSpawnComparator
{
    bool operator()(const FGSDSeededSpawnTicket& A, const FGSDSeededSpawnTicket& B) const
    {
        // First by priority (static, not frame-dependent)
        if (A.Priority != B.Priority)
            return A.Priority > B.Priority;

        // Then by spawn order (deterministic tiebreaker)
        return A.SpawnOrder < B.SpawnOrder;
    }
};
```

### SaveGame Schema

> **Council Issue Addressed**: unreal-determinism-rick HIGH - No SaveGame Schema

```cpp
// GSDSaveGame.h
USTRUCT(BlueprintType)
struct FGSDSerializedActorState
{
    GENERATED_BODY()

    UPROPERTY(SaveGame)
    FName ActorName;

    UPROPERTY(SaveGame)
    FTransform Transform;

    UPROPERTY(SaveGame)
    TArray<uint8> ComponentState;
};

USTRUCT(BlueprintType)
struct FGSDRandomCallLog
{
    GENERATED_BODY()

    UPROPERTY(SaveGame)
    FName Category;

    UPROPERTY(SaveGame)
    int32 CallCount;

    UPROPERTY(SaveGame)
    uint32 LastHash;
};

UCLASS()
class UGSDSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    // The seed that created this run
    UPROPERTY(SaveGame)
    int32 GameSeed;

    // Pending spawns
    UPROPERTY(SaveGame)
    TArray<FGSDSeededSpawnTicket> PendingSpawns;

    // Serialized actor states
    UPROPERTY(SaveGame)
    TArray<FGSDSerializedActorState> ActorStates;

    // Random history for debugging
    UPROPERTY(SaveGame)
    TArray<FGSDRandomCallLog> RandomHistory;

    // Game timestamp
    UPROPERTY(SaveGame)
    float GameTime;
};
```

### Spawn State Validation

```cpp
// IGSDSpawnValidator.h
UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
class UGSDSpawnStateValidator : public UInterface
{
    GENERATED_BODY()
};

class GSD_CORE_API IGSDSpawnStateValidator
{
    GENERATED_BODY()

public:
    // Validate current spawn state matches recorded state
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Determinism")
    bool ValidateSpawnState(const TArray<FGSDSeededSpawnTicket>& Expected);
    virtual bool ValidateSpawnState_Implementation(const TArray<FGSDSeededSpawnTicket>& Expected);

    // Compute hash of current spawn state
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Determinism")
    uint32 ComputeSpawnHash() const;
    virtual uint32 ComputeSpawnHash_Implementation() const;

    // Log divergence for debugging
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Determinism")
    void LogDivergence(const FGSDSeededSpawnTicket& Expected, const FGSDSeededSpawnTicket& Actual);
    virtual void LogDivergence_Implementation(const FGSDSeededSpawnTicket& Expected, const FGSDSeededSpawnTicket& Actual);
};
```

---

## Audio Architecture

> **Council Issues Addressed**: unreal-audio-rick HIGH - Missing Audio LOD Integration, HIGH - No MetaSound Integration Strategy

### Audio LOD Strategy

| LOD Level | Distance | Spatialization | Occlusion | Reverb | Max Concurrent |
|-----------|----------|----------------|-----------|--------|----------------|
| LOD0 | 0-500m | ENABLED | ENABLED | ENABLED | 32 |
| LOD1 | 500-2000m | ENABLED | DISABLED | ENABLED | 16 |
| LOD2 | 2000-5000m | DISABLED (mono) | DISABLED | DISABLED | 8 |
| Virtual | 5000m+ | Sound stops, state preserved | - | - | - |

### Sound Class Hierarchy

```
Master
├── Music
│   ├── BackgroundMusic
│   └── MenuMusic
├── SFX
│   ├── Player
│   ├── Weapons
│   ├── Vehicles
│   ├── Zombies
│   └── Environment
├── Voice
│   ├── Dialogue
│   └── Radio
└── Ambience
    ├── Weather
    ├── CityAmbience
    └── InteriorAmbience
```

### MetaSound Integration Interface

```cpp
// IGSDMetaSoundInterface.h
UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
class UGSDMetaSoundInterface : public UInterface
{
    GENERATED_BODY()
};

class GSD_CORE_API IGSDMetaSoundInterface
{
    GENERATED_BODY()

public:
    // Get the MetaSound source for spawning
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
    UObject* GetMetaSoundSource() const;
    virtual UObject* GetMetaSoundSource_Implementation() const;

    // Default parameters to pass on spawn
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
    TMap<FName, float> GetDefaultParameters() const;
    virtual TMap<FName, float> GetDefaultParameters_Implementation() const;

    // Output submix routing
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
    USoundSubmix* GetTargetSubmix() const;
    virtual USoundSubmix* GetTargetSubmix_Implementation() const;

    // Sound class for volume/priority
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
    USoundClass* GetSoundClass() const;
    virtual USoundClass* GetSoundClass_Implementation() const;
};
```

### Audio Budget Tracking

```cpp
// Extended tick context with audio budget
USTRUCT(BlueprintType)
struct FGSDTickContext
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float DeltaTime = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float GameTime = 0.0f;

    // Audio budget remaining (ms)
    UPROPERTY(BlueprintReadWrite)
    float AudioBudgetRemainingMs = 2.0f;

    // Can afford audio operations?
    UFUNCTION(BlueprintPure, Category = "GSD")
    bool HasAudioBudget() const { return AudioBudgetRemainingMs > 0.1f; }
};
```

---

## Testing Strategy

> **Council Issues Addressed**: unreal-validation-rick - No Testing Strategy

### Unit Testing

```cpp
// GSDTests/Private/SpawnInterfaceTests.cpp

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDSpawnableInterfaceTest,
    "GSD.Core.Interfaces.IGSDSpawnable",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDSpawnableInterfaceTest::RunTest(const FString& Parameters)
{
    // Test: Interface contract validation
    // Test: Execute_ prefix calls Blueprint implementations
    // Test: Config is passed correctly
    // Test: Callback is called on completion
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDMassSpawnerTest,
    "GSD.Core.Mass.Spawn200Entities",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::PerformanceFilter)

bool FGSDMassSpawnerTest::RunTest(const FString& Parameters)
{
    // Test: Spawn 200 entities in batch
    // Test: Verify no frame > 20ms
    // Test: Verify single callback (not 200)
    // Test: Verify memory budget respected
    return true;
}
```

### Performance Testing

| Test Name | Purpose | Pass Criteria |
|-----------|---------|---------------|
| `Test_200ZombieSpawn` | Batch spawn performance | Total time < 100ms |
| `Test_FrameTimeUnderLoad` | No hitches during spawn | No frame > 16.67ms |
| `Test_MemoryStability` | No memory leaks | Memory stable over 30min |
| `Test_AudioLODTransition` | Audio LOD works | Transitions smooth, no pops |

### Blueprint Testing

```cpp
// Create Blueprint Test Cases:
// - BP_Test_IGSDSpawnableImplementation - verify BP can implement interface
// - BP_Test_SpawnCallback - verify BP receives completion delegate
// - BP_Test_MassSpawnBatch - verify batch spawn from BP
```

### Regression Testing

- Baseline test fixtures in `Source/GSDTests/Fixtures/`
- Snapshot tests for save data format compatibility
- Version compatibility tests
- CI pipeline runs full test suite on every commit

---

## CI/CD Integration

> **Council Issues Addressed**: unreal-validation-rick - No CI Integration Plan

### GitHub Actions Workflow

```yaml
# .github/workflows/gsd-validation.yml
name: GSD Validation

on:
  push:
    branches: [main, develop]
  pull_request:
    branches: [main]

jobs:
  build:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v4

      - name: Setup Unreal Engine
        uses: unreal-engine/setup-unreal@v1
        with:
          version: '5.4'

      - name: Build GSD_Core
        run: |
          # Build plugin

      - name: Run Unit Tests
        run: |
          # Run automation tests

      - name: Performance Benchmark
        run: |
          # Run performance tests

      - name: Upload Results
        uses: actions/upload-artifact@v4
        with:
          name: test-results
          path: TestResults/
```

---

## Input & Accessibility

> **Council Issues Addressed**: unreal-interface-rick MEDIUM - Missing Input Responsiveness, MEDIUM - No Blueprint Accessibility Patterns

### Input Latency Requirements

| Operation | Max Latency | Rationale |
|-----------|-------------|-----------|
| Spawn Request | 1 frame | Must not block input |
| Spawn Complete | 50ms | Player feedback threshold |
| Audio Feedback | 16ms | Must feel instant |

### Accessibility Requirements

```cpp
// All spawn status widgets must implement:
// - ISlateAccessibleWidget for screen readers
// - Text alternatives for progress indicators
// - FSlateAccessibleWidgetData for state announcements
// - Color-blind friendly status (not color-only feedback)
```

### Feedback Patterns

```cpp
// GSDFeedbackConfig.h
UCLASS(BlueprintType)
class UGSDFeedbackConfig : public UGSDDataAsset
{
    GENERATED_BODY()

    // Spawn complete feedback
    UPROPERTY(EditDefaultsOnly, Category = "Feedback")
    USoundBase* SpawnCompleteSound;

    UPROPERTY(EditDefaultsOnly, Category = "Feedback")
    UForceFeedbackEffect* SpawnCompleteHaptic;

    // Spawn failed feedback
    UPROPERTY(EditDefaultsOnly, Category = "Feedback")
    USoundBase* SpawnFailedSound;

    // Visual indicator
    UPROPERTY(EditDefaultsOnly, Category = "Feedback")
    UMaterialInterface* SpawnProgressMaterial;
};
```

---

## Plugin Granularity

**GSD_Core contains:**

| Component | In Core? | Notes |
|-----------|----------|-------|
| Interfaces | ✓ | `IGSDSpawnable`, `IGSDStreamable`, `IGSDMassSpawnerInterface`, `IGSDComponentSpawnable`, `IGSDSpatialAudioStreamable` |
| Base classes | ✓ | `AGSDActor`, `UGSDComponent` — all GSD types inherit from these |
| Logging macros | ✓ | `GSD_LOG()`, `GSD_WARN()`, `GSD_ERROR()` — consistent format |
| Data Asset base | ✓ | `UGSDDataAsset` — all config goes through this |
| Utility functions | ✓ | Common helpers — validation, math, etc. |
| Determinism Manager | ✓ | `UGSDDeterminismManager` — seeded RNG for all plugins |
| Performance Config | ✓ | `UGSDPerformanceConfig` — budgets and scalability |

**Why opinionated:**
- All GSD actors inherit from `AGSDActor`
- All GSD components inherit from `UGSDComponent`
- All logging uses `GSD_LOG()` macros
- All config uses `UGSDDataAsset`
- All spawning uses `SpawnAsync()` or `SpawnBatch()`
- All randomness goes through `UGSDDeterminismManager`

---

## Dependency Model

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Plugin location | Engine plugins | Install to Unreal, all projects see them |
| Build.cs dependency | Explicit | List each plugin individually, only pull what you need |
| Versioning | Git submodules | Each game pins to specific commit of unreal_gsd |
| Discovery | Both | README index for humans, `GSD_PluginInfo` metadata for tools |

**Game setup pattern:**
```bash
# In projects/my_game/
git submodule add https://github.com/bretbouchard/unreal_gsd.git unreal_gsd
# Run install script to copy plugins to Engine
```

**Build.cs pattern:**
```cpp
PublicDependencyModuleNames.AddRange(new string[] {
    "GSD_Core",
    "GSD_Vehicles",
    "GSD_Crowds"
    // Only what you need
});
```

---

## Naming Conventions

| Category | Convention | Example |
|----------|------------|---------|
| Class prefix | GSD | `AGSDActor`, `UGSDDataAsset` |
| Plugin folders | Underscore | `GSD_Core`, `GSD_Vehicles`, `GSD_DailyEvents` |
| Interface files | I prefix | `IGSDSpawnable.h`, `IGSDStreamable.h` |
| File organization | By type | `Interfaces/`, `Classes/`, `DataAssets/`, `Macros/` |

**Folder structure:**
```
GSD_Core/
├── Source/
│   ├── GSD_Core/
│   │   ├── Private/
│   │   │   ├── Classes/
│   │   │   │   ├── GSDActor.cpp
│   │   │   │   └── GSDComponent.cpp
│   │   │   ├── DataAssets/
│   │   │   │   └── GSDDataAsset.cpp
│   │   │   ├── Managers/
│   │   │   │   └── GSDDeterminismManager.cpp
│   │   │   ├── Macros/
│   │   │   │   └── GSDLog.cpp
│   │   │   └── Tests/
│   │   │       └── SpawnInterfaceTests.cpp
│   │   └── Public/
│   │       ├── Interfaces/
│   │       │   ├── IGSDSpawnable.h
│   │       │   ├── IGSDStreamable.h
│   │       │   ├── IGSDMassSpawnerInterface.h
│   │       │   ├── IGSDComponentSpawnable.h
│   │       │   ├── IGSDSpatialAudioStreamable.h
│   │       │   ├── IGSDNetworkSpawnValidator.h
│   │       │   └── IGSDMetaSoundInterface.h
│   │       ├── Classes/
│   │       │   ├── GSDActor.h
│   │       │   └── GSDComponent.h
│   │       ├── DataAssets/
│   │       │   └── GSDDataAsset.h
│   │       ├── Managers/
│   │       │   └── GSDDeterminismManager.h
│   │       ├── Types/
│   │       │   ├── GSDSpawnTypes.h
│   │       │   ├── GSDPerformanceConfig.h
│   │       │   └── GSDSaveGame.h
│   │       └── Macros/
│   │           └── GSDLog.h
│   └── GSD_Core.Build.cs
├── GSD_Core.uplugin
└── Resources/
```

---

## Success Criteria (Measurable)

> **Council Issue Addressed**: unreal-prime-rick - Success criteria need measurable metrics

| # | Criterion | Measurable | Target |
|---|-----------|------------|--------|
| 1 | GSD_Core plugin compiles and loads | Build succeeds, plugin appears in Editor | 100% |
| 2 | Interfaces defined and accessible | Other plugins can #include and use | 100% |
| 3 | Build.cs enforces correct loading phase | PreDefault loads before Default | Verified |
| 4 | No circular dependencies | Link succeeds with no warnings | 0 warnings |
| 5 | Cross-plugin communication via interfaces | No direct includes between feature plugins | 0 violations |
| 6 | Performance budgets respected | Spawn operations < 2ms/frame | 100% |
| 7 | Batch spawn works for 200+ entities | Test passes | 100% |
| 8 | Determinism verified | Same seed = same spawn order | 100% |
| 9 | Unit test coverage | Core interfaces tested | >80% |
| 10 | CI pipeline green | All tests pass | 100% |

---

## Risk Assessment

> **Council Issue Addressed**: unreal-prime-rick - Missing Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| Batch spawning performance regression | Medium | High | Automated performance tests, profiling on every PR |
| Memory budget exceeded | Medium | High | Memory tracking, hard limits enforced |
| Determinism drift | Low | Critical | State hash validation, replay testing |
| Network replication overhead | Low | Medium | Single-player focus, hooks for future |
| Audio component spawning complexity | Medium | Medium | Component interface separate from actor interface |

---

## Deferred Ideas

(Things that came up but belong in future phases or separate discussions)

- Plugin installer script — automate copying to Engine folder
- `GSD_PluginInfo` data asset structure — define in Phase 1 or later
- Audio submix chain architecture — Phase 2+
- Actor pooling (in addition to component pooling) — Phase 2+
- Lag compensation for multiplayer — Future multiplayer phase
- Enhanced Input debug spawning — Phase 10 (Telemetry & Validation)

---

## Summary

**Phase 1 delivers:**

- `GSD_Core` plugin with opinionated architecture
- `IGSDSpawnable` — async, Blueprint-friendly, config-driven spawning
- `IGSDMassSpawnerInterface` — batch spawning for 200+ entities
- `IGSDComponentSpawnable` — component spawning for audio/particles
- `IGSDStreamable` — full streaming control with callbacks
- `IGSDSpatialAudioStreamable` — audio-specific streaming with state preservation
- `IGSDNetworkSpawnValidator` — server-authoritative validation hooks
- `IGSDMetaSoundInterface` — MetaSound integration
- `AGSDActor` / `UGSDComponent` — base classes for all GSD types
- `UGSDDataAsset` — base class for all configuration
- `UGSDDeterminismManager` — seeded RNG for reproducible runs
- `UGSDPerformanceConfig` — budgets and scalability settings
- `UGSDSaveGame` — state serialization for replay
- `GSD_LOG()` macros — consistent logging
- Build.cs structure for correct loading phases
- Unit test suite with >80% coverage
- CI/CD pipeline with automated validation

**At game level, developers:**
- Inherit from `AGSDActor` / `UGSDComponent`
- Use `SpawnAsync()` for single entities
- Use `SpawnBatch()` for crowds (200+ zombies)
- Store config in `UGSDDataAsset` derivatives
- Get randomness from `UGSDDeterminismManager`
- Log with `GSD_LOG()`
- Push pixels and make games
