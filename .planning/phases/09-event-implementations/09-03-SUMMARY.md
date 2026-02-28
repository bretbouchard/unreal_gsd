# Phase 9 Plan 03 - Bonfire Event - Summary

**Status:** ✅ COMPLETE
**Duration:** Autonomous execution
**Date:** 2026-02-26

## Objective
Implement Bonfire event (EVT-06) that spawns visual FX and reduces zombie density locally.

## Files Created

### Header
- `Plugins/GSD_DailyEvents/Source/GSD_DailyEvents/Public/DataAssets/Events/GSDEventBonfireConfig.h`

### Implementation
- `Plugins/GSD_DailyEvents/Source/GSD_DailyEvents/Private/DataAssets/Events/GSDEventBonfireConfig.cpp`

## Files Modified

### Build Configuration
- `Plugins/GSD_DailyEvents/Source/GSD_DailyEvents/GSD_DailyEvents.Build.cs`
  - Added `Niagara` module dependency
  - Added `NavigationSystem` module dependency

## Implementation Details

### UGSDEventBonfireConfig
- Extends `UGSDDailyEventConfig`
- EventTag: `Event.Daily.Bonfire`
- Default Duration: 30 minutes

### Properties
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| BonfireFX | TObjectPtr<UNiagaraSystem> | - | Niagara system for fire effect |
| FXScale | FVector | (1,1,1) | Base scale for FX |
| DensityReducer | TObjectPtr<UGSDDensityReduceModifier> | - | Reduces zombie density |

### Key Behaviors
1. **OnEventStart:**
   - Spawns Niagara FX at location with intensity-based scale
   - Sets `User.Intensity` Niagara variable
   - Applies DensityReduceModifier for local density reduction

2. **OnEventEnd:**
   - Deactivates and destroys Niagara component
   - Nulls component pointer
   - Removes density modifier

3. **ValidateConfig:**
   - Checks BonfireFX is valid
   - Checks DensityReducer is set

### Pitfall Mitigations Applied
- **Pitfall 2 (Modifier State Leak):** Always removes modifier even with null context
- **Pitfall 4 (Niagara Lifetime):** Uses bAutoDestroy=false with manual Deactivate/DestroyComponent

## Verification Results
- [x] Niagara module dependency added
- [x] UGSDEventBonfireConfig extends UGSDDailyEventConfig
- [x] OnEventStart_Implementation spawns Niagara FX
- [x] OnEventStart_Implementation applies DensityReduceModifier
- [x] OnEventEnd_Implementation deactivates and destroys FX component
- [x] OnEventEnd_Implementation removes density modifier
- [x] Intensity affects FX scale and parameters
- [x] ValidateConfig checks BonfireFX is valid
- [x] ValidateConfig checks DensityReducer is valid

## Success Criteria Met
- [x] Bonfire event spawns Niagara FX at event location
- [x] FX intensity parameter controlled by event intensity
- [x] Zombie density reduced in area around bonfire
- [x] FX deactivated and destroyed when event ends
- [x] Density modifier removed when event ends
- [x] No memory leaks from FX components
- [x] ValidateConfig provides specific error messages

## Next Steps
- Continue to Plan 09-04: Block Party Event (EVT-07)
