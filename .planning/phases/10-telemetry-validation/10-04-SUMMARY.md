# Phase 10 Plan 04: Streaming Telemetry Extensions

**Completed:** 2026-02-27
**Duration:** ~2 minutes
**Status:** Complete

## Objective Achieved

Extended GSDStreamingTelemetry to capture cell load times by district, implementing TEL-03 (streaming cell load times in telemetry).

## Files Created

| File | Purpose |
|------|---------|
| `Plugins/GSD_Telemetry/Source/GSD_Telemetry/Public/Subsystems/GSDStreamingTelemetry.h` | Streaming telemetry subsystem interface |
| `Plugins/GSD_Telemetry/Source/GSD_Telemetry/Private/Subsystems/GSDStreamingTelemetry.cpp` | Streaming telemetry implementation |

## Implementation Details

### UGSDStreamingTelemetry Subsystem

**Base Class:** UGameInstanceSubsystem (cross-level persistence)

**Key Features:**
1. **Cell Load Time Tracking**
   - RecordCellLoadTime(CellName, LoadTimeMs, DistrictName)
   - Per-district storage using TMap<FName, TArray<FGSDCellLoadTimeRecord>>
   - Circular buffer with MaxRecordsPerDistrict (100) limit

2. **Query Methods**
   - GetCellLoadTimesByDistrict(DistrictName) - Returns all records for district
   - GetAverageCellLoadTimeMs(DistrictName) - Calculates average from district records
   - GetAllCellLoadTimes(OutRecords) - Returns all records across all districts
   - GetAllDistrictNames(OutDistrictNames) - Returns list of tracked districts

3. **Statistics**
   - GetTotalCellsLoaded() - Total cells loaded across all districts
   - GetMaxCellLoadTimeMs() - Maximum load time encountered

4. **Slow Load Detection**
   - SlowLoadThresholdMs (default: 100ms)
   - bLogSlowLoads (configurable)
   - Warning log when cell load exceeds threshold

5. **Real-time Notifications**
   - FOnCellLoaded delegate broadcasts on each cell load
   - Provides CellName and LoadTimeMs parameters

6. **Performance Tracking**
   - SCOPE_CYCLE_COUNTER(STAT_GSDRecordCellLoadTime)
   - Stats integration for profiling

**Configuration (Config=Game):**
- MaxRecordsPerDistrict: 100 (circular buffer size)
- SlowLoadThresholdMs: 100.0f (warning threshold)
- bLogSlowLoads: true (enable/disable warnings)

## Verification Results

All verification criteria met:

```bash
✓ Subsystem header exists
✓ Correct base class (UGameInstanceSubsystem)
✓ RecordCellLoadTime method declared
✓ GetCellLoadTimesByDistrict method declared
✓ Implementation file exists
✓ Initialize implemented
✓ RecordCellLoadTime implemented
✓ Pruning logic present
✓ Slow load warning configured
```

## Success Criteria Met

- [x] UGSDStreamingTelemetry extends UGameInstanceSubsystem
- [x] Cell load times tracked per-district using TArray<FGSDCellLoadTimeRecord>
- [x] Circular buffer maintains MaxRecordsPerDistrict (100) records per district
- [x] Slow load threshold (100ms) generates warnings
- [x] GetAverageCellLoadTimeMs calculates average from district records

## Key Decisions

1. **GameInstanceSubsystem** - Chosen over WorldSubsystem for cross-level persistence (telemetry accumulates across district changes)
2. **TMap Storage** - Per-district arrays enable efficient district-specific queries
3. **Circular Buffer** - PruneOldRecords removes oldest entries to prevent unbounded memory growth
4. **100ms Threshold** - Default warning threshold based on 60fps frame budget (16.67ms), allowing for multi-frame loads

## Integration Points

- **FGSDCellLoadTimeRecord** - Uses existing telemetry type (defined in 10-01)
- **STAT_GSDRecordCellLoadTime** - Uses existing stats declaration (defined in 10-01)
- **GSDTELEMETRY_LOG** - Uses existing logging macros (defined in 10-01)

## Next Steps

Plan 10-05 (Actor Count Tracker) will track vehicle/zombie/human counts with periodic snapshots.

## Commit

```
27f5204 feat(10-04): implement streaming telemetry subsystem
```

**Files Modified:** 2 files, 191 insertions
- Plugins/GSD_Telemetry/Source/GSD_Telemetry/Private/Subsystems/GSDStreamingTelemetry.cpp (new)
- Plugins/GSD_Telemetry/Source/GSD_Telemetry/Public/Subsystems/GSDStreamingTelemetry.h (new)
