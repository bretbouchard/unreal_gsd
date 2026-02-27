---
phase: 11-council-fixes
plan: 07
subsystem: crowds
tags: [dataasset, configuration, crowds, lod, behavior, audio, mass-entity]

# Dependency graph
requires:
  - phase: 06-crowd-core-systems
    provides: Crowd processors (GSDZombieBehaviorProcessor, GSDCrowdLODProcessor)
  - phase: 11-06
    provides: Audio LOD configuration pattern
provides:
  - UGSDCrowdConfig DataAsset for centralized crowd configuration
  - Config-driven processors with fallback defaults
  - All hardcoded values replaced with DataAsset lookups
affects: [crowds, audio, performance-tuning]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - DataAsset configuration pattern for processor parameters
    - Cached config lookup with fallback defaults
    - Backward-compatible config migration

key-files:
  created:
    - Plugins/GSD_Crowds/Source/GSD_Crowds/Public/DataAssets/GSDCrowdConfig.h
    - Plugins/GSD_Crowds/Source/GSD_Crowds/Private/DataAssets/GSDCrowdConfig.cpp
  modified:
    - Plugins/GSD_Crowds/Source/GSD_Crowds/Public/Processors/GSDZombieBehaviorProcessor.h
    - Plugins/GSD_Crowds/Source/GSD_Crowds/Private/Processors/GSDZombieBehaviorProcessor.cpp
    - Plugins/GSD_Crowds/Source/GSD_Crowds/Public/Processors/GSDCrowdLODProcessor.h
    - Plugins/GSD_Crowds/Source/GSD_Crowds/Private/Processors/GSDCrowdLODProcessor.cpp

key-decisions:
  - "All config properties use EditDefaultsOnly (configured in editor, not at runtime)"
  - "Fallback default values as static constexpr for backward compatibility"
  - "Cached config pointer per processor for efficient per-frame access"
  - "Accessor methods for external config value access"

patterns-established:
  - "Config lookup: GetDefaultConfig() once per Execute(), cache in member"
  - "Fallback pattern: Config ? Config->Value : DefaultValue"
  - "Static constexpr defaults for compile-time constants"

# Metrics
duration: 3min
completed: 2026-02-27
---

# Phase 11 Plan 07: DataAsset Configuration Summary

**UGSDCrowdConfig DataAsset with centralized crowd parameters, replacing all hardcoded values in processors with config-driven lookups**

## Performance

- **Duration:** 3 min
- **Started:** 2026-02-27T20:54:00Z
- **Completed:** 2026-02-27T20:57:00Z
- **Tasks:** 2
- **Files modified:** 6

## Accomplishments
- Created UGSDCrowdConfig DataAsset with 40+ configurable parameters
- Replaced all hardcoded values in GSDZombieBehaviorProcessor with config lookups
- Replaced all hardcoded values in GSDCrowdLODProcessor with config lookups
- Added fallback defaults for backward compatibility
- Added accessor methods for external config access

## Task Commits

Each task was committed atomically:

1. **Task 1: Create UGSDCrowdConfig DataAsset** - `d3de7a3` (feat)
2. **Task 2: Update Processors to Use Config** - `b05e371` (feat)

## Files Created/Modified
- `Plugins/GSD_Crowds/Source/GSD_Crowds/Public/DataAssets/GSDCrowdConfig.h` - DataAsset with all crowd parameters
- `Plugins/GSD_Crowds/Source/GSD_Crowds/Private/DataAssets/GSDCrowdConfig.cpp` - GetDefaultConfig() implementation
- `Plugins/GSD_Crowds/Source/GSD_Crowds/Public/Processors/GSDZombieBehaviorProcessor.h` - Added CachedConfig and defaults
- `Plugins/GSD_Crowds/Source/GSD_Crowds/Private/Processors/GSDZombieBehaviorProcessor.cpp` - Config-driven behavior
- `Plugins/GSD_Crowds/Source/GSD_Crowds/Public/Processors/GSDCrowdLODProcessor.h` - Added CachedConfig, defaults, accessors
- `Plugins/GSD_Crowds/Source/GSD_Crowds/Private/Processors/GSDCrowdLODProcessor.cpp` - Config-driven LOD and audio

## Decisions Made
- Used EditDefaultsOnly for all properties (editor configuration, not runtime)
- Added static constexpr defaults for backward compatibility when config not found
- Cached config pointer per processor for efficient per-frame access
- Added accessor methods (GetHighActorDistance, etc.) for external config value access
- Grouped parameters by category: Detection, LOD, Limits, Behavior, Navigation, Audio, Debug

## Config Parameters Added

### Detection
- DetectionRadius (5000.0f) - Entity detection range
- VisionAngle (90.0f) - Vision cone angle
- HearingRange (1500.0f) - Sound detection range

### LOD Distances
- LOD0Distance, LOD1Distance, LOD2Distance, LOD3CullDistance
- HighActorDistance (2000.0f), LowActorDistance (5000.0f)
- ISMDistance (10000.0f), CullDistance (20000.0f)

### Entity Limits
- MaxEntityCount (200), EntitiesPerBatch (10)
- ProcessingFrameBudget (2.0f)

### Behavior
- BaseMoveSpeed (150.0f), SpeedVariationPercent (0.2f)
- MaxSpeedMultiplier (1.5f), WanderingRadius (500.0f)
- BehaviorUpdateInterval (0.5f), WanderDirectionChange (45.0f)
- SpeedInterpolationRate (2.0f)

### Navigation
- LaneSearchRadius (1000.0f), SmartObjectSearchCooldown (5.0f)
- InteractionDurationMin (3.0f), InteractionDurationMax (8.0f)

### Audio
- bEnableAudioLOD (true)
- AudioLOD0Distance (500.0f), AudioLOD1Distance (2000.0f)
- AudioLOD2Distance (4000.0f), AudioCullDistance (5000.0f)
- AudioLOD1VolumeMultiplier (0.5f), AudioLOD2VolumeMultiplier (0.25f)

### Debug
- bEnableDebugVisualization (false), DebugDrawDuration (0.0f)

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered
None - straightforward implementation following established DataAsset patterns from GSD_Vehicles.

## User Setup Required

**Editor asset creation required.** Create the default config asset:
1. In Unreal Editor, create new DataAsset: Right-click -> Miscellaneous -> Data Asset -> GSDCrowdConfig
2. Save to: `/GSD_Crowds/Config/DefaultCrowdConfig`
3. Configure parameters as needed for your project

## Next Phase Readiness
- Crowd system is now fully data-driven
- Designers can tune all crowd parameters without code changes
- Config asset creation is the only remaining step for production use

---
*Phase: 11-council-fixes*
*Completed: 2026-02-27*
