---
phase: 10-telemetry-validation
plan: 03
subsystem: telemetry
tags: [performance, frame-time, hitch-detection, actor-counting, game-instance-subsystem]

# Dependency graph
requires:
  - phase: 10-telemetry-validation
    provides: Telemetry plugin foundation with types and logging
provides:
  - Per-district frame time tracking with circular buffers
  - Hitch detection when frame time exceeds threshold
  - Periodic actor counting (vehicles, zombies, humans)
  - Performance telemetry subsystem for runtime monitoring
affects: [10-04, 10-05, 10-06, 10-07]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - Forward declarations to avoid hard dependencies
    - Circular buffer for O(1) averaging
    - GameInstanceSubsystem for cross-level persistence
    - Timer-based periodic sampling
    - Class name string matching for loose coupling

key-files:
  created:
    - Plugins/GSD_Telemetry/Source/GSD_Telemetry/Public/Subsystems/GSDPerformanceTelemetry.h
    - Plugins/GSD_Telemetry/Source/GSD_Telemetry/Private/Subsystems/GSDPerformanceTelemetry.cpp
  modified: []

key-decisions:
  - "UGameInstanceSubsystem base for cross-level persistence (survives level transitions)"
  - "Config=Game for configurable thresholds (HitchThresholdMs, ActorCountInterval)"
  - "Forward declarations for GSD_Vehicles and GSD_Crowds to avoid hard dependencies"
  - "Class name string matching for actor counting (flexible, no hard dependency)"
  - "60-frame circular buffer for 1-second average at 60fps"
  - "Default hitch threshold 16.67ms (60fps target)"

patterns-established:
  - "Forward declaration pattern: Avoid hard dependencies using class name string matching"
  - "Circular buffer pattern: FGSDFrameTimeHistory with O(1) AddFrameTime and GetAverageMs"
  - "Subsystem lifecycle pattern: Initialize/Deinitialize with timer management"
  - "Stats integration pattern: SCOPE_CYCLE_COUNTER and SET_DWORD_STAT for profiling"
  - "Delegate pattern: FOnHitchDetected and FOnActorCountUpdated for event notification"

# Metrics
duration: 3min
completed: 2026-02-27
---

# Phase 10 Plan 03: Performance Telemetry Subsystem Summary

**UGSDPerformanceTelemetry GameInstanceSubsystem with per-district frame time tracking, hitch detection at 16.67ms threshold, and periodic actor counting every 5 seconds using forward declarations for loose coupling**

## Performance

- **Duration:** 3 min
- **Started:** 2026-02-27T06:41:59Z
- **Completed:** 2026-02-27T06:45:09Z
- **Tasks:** 2
- **Files modified:** 2

## Accomplishments
- Created UGSDPerformanceTelemetry extending UGameInstanceSubsystem for cross-level persistence
- Implemented per-district frame time tracking using FGSDFrameTimeHistory circular buffers
- Added hitch detection when frame time exceeds configurable 16.67ms threshold
- Implemented periodic actor counting (vehicles, zombies, humans) every 5 seconds via timer
- Used forward declarations and class name string matching to avoid hard dependencies on GSD_Vehicles and GSD_Crowds
- Integrated with stats system via SCOPE_CYCLE_COUNTER and SET_DWORD_STAT
- Added delegates for hitch detection and actor count updates

## Task Commits

Each task was committed atomically:

1. **Task 1: Create Performance Telemetry Subsystem Header** - `22bd38b` (feat)
2. **Task 2: Implement Performance Telemetry Subsystem** - `22bd38b` (feat)

**Plan metadata:** Pending commit (docs: complete plan)

_Note: Both tasks were committed together as a single atomic implementation_

## Files Created/Modified
- `Plugins/GSD_Telemetry/Source/GSD_Telemetry/Public/Subsystems/GSDPerformanceTelemetry.h` - Performance telemetry subsystem interface with frame time tracking, hitch detection, and actor counting methods
- `Plugins/GSD_Telemetry/Source/GSD_Telemetry/Private/Subsystems/GSDPerformanceTelemetry.cpp` - Implementation with Initialize/Deinitialize lifecycle, RecordFrameTime, RecordHitch, CountActors, and timer management

## Decisions Made
- **UGameInstanceSubsystem base class** - Survives level transitions, persists across entire game session (vs UWorldSubsystem which is level-specific)
- **Config=Game for thresholds** - Allows designers to tune HitchThresholdMs and ActorCountInterval via DefaultGame.ini
- **Forward declarations with string matching** - Avoids hard dependency on GSD_Vehicles and GSD_Crowds plugins, enables telemetry to work even if those plugins are disabled
- **60-frame circular buffer** - Provides 1-second rolling average at 60fps, matches FGSDFrameTimeHistory pattern from Plan 10-01
- **16.67ms default hitch threshold** - Targets 60fps, configurable for different performance targets

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None - implementation straightforward, all verification checks passed.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness
- Performance telemetry subsystem ready for integration
- Next plans (10-04 Hitch Detector, 10-05 Actor Count Tracker, 10-06 Cell Load Time Tracker) can extend this subsystem
- Telemetry testbed (10-07) can verify frame time recording and hitch detection
- Forward declarations established pattern for loose coupling between telemetry and gameplay systems

---
*Phase: 10-telemetry-validation*
*Plan: 03*
*Completed: 2026-02-27*
