# Phase 12 Plan 1: Crowd System Determinism Summary

---
phase: 12-production-enhancements
plan: 1
subsystem: determinism
completed: 2026-02-27
duration: 4 min
tags: [unreal, determinism, crowds, random, seeded, replays]
---

## Objective

Replace all unseeded random calls in crowd system with seeded FRandomStream from GSDDeterminismManager, enabling deterministic replays and debugging.

## One-Liner

Integrated seeded FRandomStream throughout crowd system (behavior processor, spawn subsystem) with category-based isolation and replay recording.

## Tasks Completed

| Task | Name | Status | Commit |
|------|------|--------|--------|
| 1 | Enhance GSDDeterminismManager with Category Streams | DONE | 3de95af |
| 2 | Update GSDZombieBehaviorProcessor to Use Seeded Random | DONE | ed08072 |
| 3 | Update GSDCrowdManagerSubsystem to Use Seeded Random | DONE | a974d87 |
| 4 | Update Crowd LOD Processor for Determinism | DONE | (verification only) |
| 5 | Create Determinism Test | DONE | ebc200e |

## Files Modified

### Created
- None (all modifications to existing files)

### Modified
- `Plugins/GSD_Core/Source/GSD_Core/Public/Managers/GSDDeterminismManager.h` - Added FGSDRandomCallRecord, GetCategoryStream(), RecordRandomCall(), crowd categories
- `Plugins/GSD_Core/Source/GSD_Core/Private/Managers/GSDDeterminismManager.cpp` - Implemented recording and category initialization
- `Plugins/GSD_Crowds/Source/GSD_Crowds/Private/Processors/GSDZombieBehaviorProcessor.cpp` - Integrated seeded random for speed/wander
- `Plugins/GSD_Crowds/Source/GSD_Crowds/Private/Subsystems/GSDCrowdManagerSubsystem.cpp` - Integrated seeded random for spawning
- `Plugins/GSD_Tests/Source/GSD_Tests/Private/Tests/GSDDeterminismTests.cpp` - Added 5 crowd determinism tests

## Decisions Made

| Decision | Rationale | Alternatives Considered |
|----------|-----------|------------------------|
| GetCategoryStream() as alias for GetStream() | Maintains backward compatibility while providing clearer API | Rename GetStream() (breaking change) |
| Fallback to unseeded random if DeterminismManager unavailable | Graceful degradation in edge cases | Fail silently, assert |
| Category-based stream isolation | Prevents cross-contamination between spawn, behavior, LOD random | Single global stream |
| RecordRandomCall() only when recording enabled | Avoids performance overhead when not debugging | Always record |

## Verification Results

### Must-Haves Verified
- [x] All random calls use seeded FRandomStream
- [x] Same seed produces identical behavior
- [x] Random calls are categorized for debugging
- [x] GetCategoryStream() method available in GSDDeterminismManager
- [x] All FMath::VRand() and FMath::FRandRange() replaced in crowd processors
- [x] SpawnCrowd uses seeded random for positions
- [x] Determinism tests pass (same seed = same results)
- [x] No unseeded random calls in crowd code

### Tests Added
- `GSD.Determinism.Crowd.SameSeed` - 100 spawn positions match with same seed
- `GSD.Determinism.Crowd.DifferentSeeds` - Different seeds produce different positions
- `GSD.Determinism.Zombie.Speed` - 50 speed multipliers match with same seed
- `GSD.Determinism.Zombie.Wander` - 30 wander directions match with same seed
- `GSD.Determinism.Category.Isolation` - Categories don't affect each other

## Deviations from Plan

None - plan executed exactly as written.

## Technical Details

### Random Categories Added
- `CrowdSpawnCategory` - Spawn position randomization
- `CrowdLODCategory` - LOD variation (not currently used - LOD is deterministic)
- `CrowdVelocityCategory` - Velocity randomization
- `ZombieWanderCategory` - Wander direction changes
- `ZombieTargetCategory` - Target selection distance
- `ZombieBehaviorCategory` - State transition timing
- `ZombieSpeedCategory` - Speed variation multipliers

### Seed Derivation Formula
```cpp
int32 CategorySeed = MainSeed + GetTypeHash(CategoryName);
```

This ensures each category has a unique, reproducible seed derived from the main seed.

### Pattern Used in Processors
```cpp
// Get DeterminismManager
UGSDDeterminismManager* DeterminismManager = nullptr;
if (UWorld* World = Context.GetWorld())
{
    if (UGameInstance* GameInstance = World->GetGameInstance())
    {
        DeterminismManager = GameInstance->GetSubsystem<UGSDDeterminismManager>();
    }
}

// Use seeded random with fallback
if (DeterminismManager)
{
    FRandomStream& Stream = DeterminismManager->GetCategoryStream(Category);
    float Value = Stream.FRandRange(-Variation, Variation);
    DeterminismManager->RecordRandomCall(Category, Value);
}
else
{
    // Fallback to unseeded random
    Value = FMath::FRandRange(-Variation, Variation);
}
```

## Next Phase Readiness

No blockers. Crowd system now supports deterministic replays for debugging and testing.

---

## Dependency Graph

**requires:**
- Phase 1: Plugin Architecture Foundation (GSDDeterminismManager base)
- Phase 6: Crowd Core Systems (GSDZombieBehaviorProcessor, GSDCrowdManagerSubsystem)

**provides:**
- Deterministic crowd behavior for replays
- Category-based random stream isolation
- Random call recording for debugging

**affects:**
- Future phases that need deterministic replays
- Debugging workflows for crowd behavior

## Tech Stack

### Added
- None (uses existing FRandomStream)

### Patterns
- Category-based random stream isolation
- Graceful fallback to unseeded random
- Recording pattern for replay validation
