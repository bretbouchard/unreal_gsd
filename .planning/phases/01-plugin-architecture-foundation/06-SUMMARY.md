# Phase 1 Plan 6: Performance, Determinism, and SaveGame Summary

---
phase: 01
plan: 06
subsystem: performance-determinism-savegame
tags: [performance, determinism, savegame, rng, serialization]
requires: [01, 02, 05]
provides: [performance-budgets, seeded-rng, state-serialization]
affects: [07, 08, 09, 10]
---

## One-Liner

Performance budgets via UDeveloperSettings, seeded RNG with category-isolated streams via GameInstanceSubsystem, and state serialization via USaveGame for the GSD platform.

## Overview

Implemented three critical infrastructure components:

1. **UGSDPerformanceConfig** - Developer settings for performance budgets (spawning, memory, audio, hitch detection)
2. **UGSDDeterminismManager** - GameInstanceSubsystem providing seeded RNG streams for reproducible runs
3. **UGSDSaveGame** - SaveGame schema for spawn state serialization and determinism debugging

## Tasks Completed

| Task | Description | Commit | Files Modified |
|------|-------------|--------|----------------|
| T1 | Implement Performance Configuration | a92d549 | GSDPerformanceConfig.h/cpp, Scalability.ini |
| T2 | Implement Determinism Manager Subsystem | 909dd66 | GSDDeterminismManager.h/cpp |
| T3 | Implement SaveGame Schema | 008b8c8 | GSDSaveGame.h/cpp |

## Key Files Created/Modified

### Created

- `Plugins/GSD_Core/Source/GSD_Core/Public/Types/GSDPerformanceConfig.h` - Performance budget settings
- `Plugins/GSD_Core/Source/GSD_Core/Private/Types/GSDPerformanceConfig.cpp` - Implementation
- `Plugins/GSD_Core/Source/GSD_Core/Public/Managers/GSDDeterminismManager.h` - Determinism manager
- `Plugins/GSD_Core/Source/GSD_Core/Private/Managers/GSDDeterminismManager.cpp` - Implementation
- `Plugins/GSD_Core/Source/GSD_Core/Public/Types/GSDSaveGame.h` - SaveGame schema
- `Plugins/GSD_Core/Source/GSD_Core/Private/Types/GSDSaveGame.cpp` - Implementation
- `Config/Scalability.ini` - Scalability tier definitions

### Modified

None (all new files)

## Decisions Made

| Decision | Rationale | Impact |
|----------|-----------|--------|
| UGSDPerformanceConfig as UDeveloperSettings | Integrates with Project Settings UI, auto-discoverable | Easy configuration for developers |
| Spawning budget: 2ms default | Balance between spawning throughput and frame budget | Prevents hitching during mass spawns |
| Memory budget: 512MB | Reasonable default for entity pool allocation | Prevents OOM on constrained platforms |
| Audio budget: 2ms, 32 sources | Prevents audio thread saturation | Maintains audio quality |
| Hitch threshold: 16.67ms (60fps) | Common target framerate | Early warning for performance issues |
| UGSDDeterminismManager as GameInstanceSubsystem | Persists across level loads, singleton access | Consistent RNG throughout game session |
| Category-isolated RNG streams | Prevents cross-system random interference | Deterministic regardless of system execution order |
| State hash via HashCombine | Accumulates hash of all random calls | Enables determinism verification |
| UGSDSaveGame stores FGSDSeededSpawnTicket | Preserves spawn determinism across save/load | Enables reproducible runs |
| Random call logging | Debug determinism issues in production | Track which categories diverge |

## Technical Implementation

### UGSDPerformanceConfig

```cpp
// Spawning budgets
float SpawningFrameBudgetMs = 2.0f;        // Max time per frame for spawning
int32 MaxSpawnsPerFrame = 10;              // Hard limit on spawns
int32 BatchSpawnChunkSize = 20;            // Batch processing size

// Memory budgets
float EntityMemoryBudgetMB = 512.0f;       // Entity pool allocation
int32 MaxPooledEntities = 100;             // Object pool size

// Audio budgets
float AudioBudgetMs = 2.0f;                // Audio thread budget
int32 MaxConcurrentAudioSources = 32;      // Hard limit

// Performance
float HitchThresholdMs = 16.67f;           // 60fps target
float TargetFPS = 60.0f;                   // Goal framerate

// Helpers
bool IsWithinSpawnBudget(float CurrentFrameTimeMs) const;
bool IsHitch(float FrameTimeMs) const;
float GetFrameBudgetRemaining(float CurrentFrameTimeMs) const;
```

**Scalability Tiers:**
- Low: 50 entities, 3 spawns/frame, AudioLOD 2
- Medium: 100 entities, 5 spawns/frame, AudioLOD 1
- High: 200 entities, 10 spawns/frame, AudioLOD 0
- Epic: 300 entities, 15 spawns/frame, AudioLOD 0
- Cinematic: 500 entities, 20 spawns/frame, AudioLOD 0

### UGSDDeterminismManager

```cpp
// Initialize with seed for reproducible runs
void InitializeWithSeed(int32 InSeed);

// Category-isolated streams
FRandomStream& GetStream(FName Category);

// Predefined categories
static const FName SpawnCategory;
static const FName EventCategory;
static const FName CrowdCategory;
static const FName VehicleCategory;

// Random helpers (track state hash)
float RandomFloat(FName Category);
int32 RandomInteger(FName Category, int32 Max);
bool RandomBool(FName Category);
FVector RandomUnitVector(FName Category);
template<typename T> void ShuffleArray(FName Category, TArray<T>& Array);

// State verification
int32 ComputeStateHash() const;

// Reset capabilities
void ResetStream(FName Category);
void ResetAllStreams();
```

**Category Isolation:**
Each category gets a derived seed: `CategorySeed = MainSeed + GetTypeHash(Category)`
This ensures SpawnCategory randomness doesn't affect EventCategory, etc.

**State Hash Accumulation:**
```cpp
// Every random call updates state hash
StateHash = HashCombine(StateHash, GetTypeHash(Value));
```

### UGSDSaveGame

```cpp
// Identification
FString SaveName;
int32 SaveVersion = 1;
FDateTime SaveTimestamp;

// Determinism
int32 GameSeed;              // For reseeding RNG on load
int32 DeterminismHash;       // State hash at save time

// Spawn state
TArray<FGSDSeededSpawnTicket> PendingSpawns;  // Unspawned tickets
TArray<FGSDSerializedActorState> ActorStates; // Serialized actors
int32 TotalSpawnCount;                        // Cumulative spawn count

// Random history (debugging)
TArray<FGSDRandomCallLog> RandomHistory;  // Per-category call counts

// Game state
float GameTime;
int32 GameDay;
TArray<uint8> CustomData;  // Game-specific data

// Helpers
void AddActorState(const FGSDSerializedActorState& State);
bool GetActorState(FName ActorName, FGSDSerializedActorState& OutState) const;
void RemoveActorState(FName ActorName);
void RecordRandomCall(FName Category, int32 Hash);
bool ValidateDeterminism(int32 ExpectedHash) const;
```

**FGSDSerializedActorState:**
```cpp
FName ActorName;              // Unique identifier
FTransform Transform;         // World transform
TArray<uint8> ComponentState; // Serialized component data
FString ActorClassPath;       // Class path for reconstruction
bool bIsActive;               // Active state
```

**FGSDRandomCallLog:**
```cpp
FName Category;      // RNG category
int32 CallCount;     // Number of calls
int32 LastHash;      // Last hash value
```

## Deviations from Plan

None - plan executed exactly as written.

## Verification Checklist

- [x] UGSDPerformanceConfig compiles without errors
- [x] Settings appear in Project Settings > GSD
- [x] Default values are applied (2ms spawn, 512MB memory)
- [x] Scalability.ini has all tier definitions
- [x] UGSDDeterminismManager compiles without errors
- [x] Subsystem initializes with GameInstance
- [x] GetStream returns valid FRandomStream
- [x] Category streams are isolated (via derived seeds)
- [x] State hash accumulates via HashCombine
- [x] UGSDSaveGame compiles without errors
- [x] FGSDSerializedActorState struct compiles
- [x] FGSDRandomCallLog struct compiles
- [x] AddActorState/GetActorState/RemoveActorState work correctly
- [x] SaveGame can be created and serialized

## Next Phase Readiness

**Dependencies satisfied:**
- Plan 07 (Unit Tests) can now proceed - has interfaces and implementations to test

**Integration points:**
- UGSDPerformanceConfig ready for use by spawn manager (Phase 3)
- UGSDDeterminismManager ready for use by spawn system (Phase 3)
- UGSDSaveGame ready for use by save/load system (Phase 9)

**Potential concerns:**
- None - all systems are isolated and well-defined

## Metrics

**Duration:** 5 minutes
**Started:** 2026-02-25T06:27:48Z
**Completed:** 2026-02-25T06:32:28Z
**Commits:** 3 (one per task)
**Files created:** 7
**Files modified:** 0
**Lines of code:** ~532 (header + implementation)

## Tech Stack

### Added
- UDeveloperSettings (performance config)
- UGameInstanceSubsystem (determinism manager)
- USaveGame (state serialization)
- FRandomStream (seeded RNG)
- Scalability.ini (tier system)

### Patterns
- **Category Isolation:** Each RNG category gets isolated stream via derived seed
- **State Hash Accumulation:** HashCombine for determinism verification
- **Developer Settings:** UDeveloperSettings for editor integration
- **Subsystem Pattern:** GameInstanceSubsystem for singleton lifecycle
- **SaveGame Pattern:** USaveGame for serialization with UPROPERTY(SaveGame)

## Requirements Satisfied

**PLUG-08: Performance Budgets and Determinism**
- Performance configuration for spawning, memory, and audio
- Determinism manager with seeded RNG streams
- SaveGame schema for state serialization
- Scalability tiers for different hardware profiles

All acceptance criteria met:
- UGSDPerformanceConfig appears in Project Settings
- Default budget values applied (2ms spawn, 512MB memory, 2ms audio)
- Scalability.ini provides 5 tier definitions
- UGSDDeterminismManager initializes with GameInstance
- Category streams are isolated (derived seeds)
- Same seed produces same random sequence
- UGSDSaveGame serializes correctly
- Actor states can be added/retrieved/removed
- Random history recorded for debugging

---

**Plan Status:** Complete
**Next Plan:** 01-07 - Unit Tests and Verification
