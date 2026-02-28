# Phase 6 Plan 02: Mass Entity Fragments and Processors

## Status: COMPLETE

## Execution Summary

Created Mass Entity fragments and processors for zombie crowd simulation with data-oriented design for parallel processing.

## Files Modified

| File | Purpose |
|------|---------|
| `Public/Fragments/GSDZombieStateFragment.h` | Zombie state fragment (health, speed, behavior) |
| `Private/Fragments/GSDZombieStateFragment.cpp` | Empty implementation (header-only) |
| `Public/Processors/GSDZombieBehaviorProcessor.h` | Behavior processor header |
| `Private/Processors/GSDZombieBehaviorProcessor.cpp` | Behavior processor implementation |
| `Public/Processors/GSDCrowdLODProcessor.h` | LOD processor header |
| `Private/Processors/GSDCrowdLODProcessor.cpp` | LOD processor implementation |

## Must-Haves Verified

- [x] Mass Entity fragments define zombie state data
- [x] Processors execute logic on entities in parallel
- [x] LOD significance is calculated based on distance from viewer
- [x] Fragment access flags are correctly specified (ReadOnly vs ReadWrite)

## Key Design Decisions

1. **No UObject Pointers**: Fragments store only raw data (no GC issues)
2. **Bitfields for Flags**: Memory efficient boolean storage
3. **Speed Interpolation**: MovementSpeed + TargetMovementSpeed for smooth changes
4. **Throttled Updates**: TimeSinceLastBehaviorUpdate for behavior throttling

## Processor Configuration

### UGSDZombieBehaviorProcessor
- Execution Group: SyncWorld
- Processing Phase: PrePhysics
- Access: ReadWrite for FGSDZombieStateFragment
- Features: Speed variation (20%), wander direction updates

### UGSDCrowdLODProcessor
- Execution: After SyncWorld
- Processing Phase: PrePhysics
- LOD Thresholds:
  - High Actor: 0-2000 units (~20m)
  - Low Actor: 2000-5000 units (~50m)
  - ISM: 5000-10000 units (~100m)
  - Culled: 10000+ units (~100m+)

## Next Steps

Proceed to Plan 03: Entity Config Data Asset
