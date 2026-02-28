# Phase 6 Plan 05: Crowd Testbed Actor

## Status: COMPLETE

## Execution Summary

Created testbed actor for validation and performance testing of crowd systems following the GSDVehicleTestbedActor pattern.

## Files Modified

| File | Purpose |
|------|---------|
| `Public/Actors/GSDCrowdTestbedActor.h` | Testbed actor header |
| `Private/Actors/GSDCrowdTestbedActor.cpp` | Testbed implementation |

## Must-Haves Verified

- [x] Testbed actor spawns 200+ entities for validation
- [x] FPS is tracked and averaged over 60 frames
- [x] Deterministic spawning with seeded randomization
- [x] Performance warnings logged when FPS drops below target

## Configuration Properties

| Property | Default | Purpose |
|----------|---------|---------|
| `NumEntitiesToSpawn` | 200 | Number of entities |
| `SpawnRadius` | 10000.0f | Spawn area radius |
| `bAutoSpawnOnBeginPlay` | false | Auto-spawn on play |
| `RandomSeed` | 42 | Deterministic seed |
| `bUseDeterministicSeed` | true | Use seeded spawn |
| `TargetFPS` | 60.0f | Performance target |
| `PerformanceWarningThreshold` | 10.0f | Warning threshold % |

## API Surface

| Function | Purpose |
|----------|---------|
| `SpawnTestEntities()` | Spawn test crowd |
| `DespawnTestEntities()` | Cleanup entities |
| `RespawnTestEntities()` | Respawn with new positions |
| `GetCurrentFPS()` | Current frame rate |
| `GetSpawnedEntityCount()` | Active entity count |
| `GetAverageFPS()` | 60-frame average FPS |
| `IsPerformanceAcceptable()` | FPS meets target |

## Validation Criteria

- Spawn 200+ entities
- Maintain 60 FPS (or configured TargetFPS)
- Average FPS over 60 frames
- Log warnings when FPS drops below threshold

## Next Steps

Proceed to Plan 06: Verification and Commandlet
