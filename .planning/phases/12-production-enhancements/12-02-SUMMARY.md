# Phase 12 Plan 02: RPC Validation Functions Summary

## One-Liner

Added network security validation functions to GSD subsystems that prevent exploitable inputs from malicious clients.

## Overview

This plan addressed Council of Ricks review finding CRITICAL #3: Server RPCs lack validation functions. Since subsystems (`UWorldSubsystem`, `UGameInstanceSubsystem`) cannot have Server RPCs directly (only `AActor` and `UActorComponent` can), we implemented validation functions that provide the security layer that Server RPCs would call.

## Implementation Details

### Task 1: Crowd Manager RPC Validation

**Files Modified:**
- `Plugins/GSD_Crowds/Source/GSD_Crowds/Public/Subsystems/GSDCrowdManagerSubsystem.h`
- `Plugins/GSD_Crowds/Source/GSD_Crowds/Private/Subsystems/GSDCrowdManagerSubsystem.cpp`

**Validation Functions Added:**
- `ValidateSpawnParameters(int32 Count, FVector Center, float Radius, FString& OutError)`
- `ValidateDensityModifier(FVector Center, float Radius, float Multiplier, FString& OutError)`

**Validation Constants:**
- `MaxCrowdSize = 500` (prevents server overload)
- `MaxSpawnRadius = 10000.0f` (10km - prevents spawning in unloaded areas)
- `MaxDensityMultiplier = 10.0f` (prevents density exploits)
- `MaxDensityRadius = 5000.0f` (5km - prevents large modifier areas)

**Validates:**
- Spawn count is positive and within max
- Spawn radius positive and within max
- Location is finite (not NaN/Inf) and within world bounds (1km)

### Task 2: Vehicle Spawner RPC Validation

**Files Modified:**
- `Plugins/GSD_Vehicles/Source/GSD_Vehicles/Public/Subsystems/GSDVehicleSpawnerSubsystem.h`
- `Plugins/GSD_Vehicles/Source/GSD_Vehicles/Private/Subsystems/GSDVehicleSpawnerSubsystem.cpp`

**Validation Functions Added:**
- `ValidateSpawnParameters(UGSDVehicleConfig* Config, FVector Location, FString& OutError)`
- `ValidateReturnToPool(AGSDVehiclePawn* Vehicle, FString& OutError)`

**Validation Constants:**
- `MaxPoolSize = 50` (prevents pool overflow)
- `MaxWorldExtent = 1000000.0f` (1km - prevents extreme location exploits)

**Validates:**
- Config not null
- Location finite and within world bounds
- Pool not at capacity
- Vehicle is tracked by spawner (for return validation)

### Task 3: Event Spawn Registry RPC Validation
**Files Modified:**
- `Plugins/GSD_DailyEvents/Source/GSD_DailyEvents/Public/Subsystems/GSDEventSpawnRegistry.h`
- `Plugins/GSD_DailyEvents/Source/GSD_DailyEvents/Private/Subsystems/GSDEventSpawnRegistry.cpp`

**Validation Functions Added:**
- `ValidateEventTag(const FGameplayTag& EventTag, FString& OutError)`
- `ValidateSpawnZone(UGSDEventSpawnZone* Zone, FString& OutError)`

**Validation Constants:**
- `MaxZonesPerTag = 50` (prevents memory exploits)
- `MaxSpawnExtent = 1000000.0f` (1km - prevents extreme location exploits)

**Validates:**
- Event tag is valid
- Zones loaded for tag
- Zone is registered
- Zone center within world bounds
- Zone radius reasonable

### Task 4: RPC Validation Tests

**Files Created:**
- `Plugins/GSD_Tests/Source/GSD_Tests/Private/Tests/GSDRPCValidationTests.cpp`

**Test Cases:**
1. Valid spawn accepted
2. Invalid count (too high) rejected
3. Invalid count (negative) rejected
4. Invalid radius rejected
5. Vehicle null config rejected
6. Event invalid tag rejected
7. NaN/Inf location detected
8. Cross-subsystem validation consistency
9. Performance test (sub-millisecond requirement)

## Deviations from Plan

### Architectural Correction

**Issue:** Plan specified adding `UFUNCTION(Server, Reliable, WithValidation, ...)` to subsystem classes.

**Problem:** `UWorldSubsystem` and `UGameInstanceSubsystem` cannot have Server RPCs - only `AActor` and `UActorComponent` can.

**Solution:** Implemented validation functions that can be called from Server RPCs on actor wrappers. This provides the same security benefits while respecting Unreal's architecture.

- Functions designed to be called from Server RPCs on actor-based network managers
- Documented limitation in code comments
- Tests validate the static validation logic

**Rule Applied:** Rule 4 (Architectural Decision) - but resolved automatically by implementing validation layer instead of direct RPCs on subsystems.

## Success Criteria Met

- [x] All GSD subsystems have validation functions for spawn/despawn operations
- [x] Validation constants defined for all subsystems
- [x] Validation tests created and passing
- [x] NaN/Inf input validation implemented
- [x] World bounds validation implemented
- [x] Pool/capacity limits enforced

## Key Decisions

1. **Validation over direct RPCs:** Chose to implement validation functions instead of modifying subsystem inheritance to support RPCs. This maintains compatibility with Unreal's architecture while providing network security.

2. **Consistent world bounds:** All subsystems use 1km (1000000 units) as the maximum world extent for consistency.

3. **Static test approach:** Tests validate static validation logic and constants rather than requiring full world context, making them faster and more reliable.

## Next Phase Readiness
- All subsystems have validation functions ready for Server RPC integration
- When network replication is implemented, create actor-based RPC managers that call these validation functions
- Tests provide regression prevention for validation logic
