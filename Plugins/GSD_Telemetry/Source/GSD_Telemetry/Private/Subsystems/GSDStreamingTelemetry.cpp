// Copyright Bret Bouchard. All Rights Reserved.

#include "Subsystems/GSDStreamingTelemetry.h"
#include "GSDTelemetryLog.h"
#include "GSDTelemetryStats.h"

void UGSDStreamingTelemetry::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    GSDTELEMETRY_LOG(Log, TEXT("GSDStreamingTelemetry initializing..."));
}

void UGSDStreamingTelemetry::Deinitialize()
{
    GSDTELEMETRY_LOG(Log, TEXT("GSDStreamingTelemetry deinitializing..."));

    // Clear data
    DistrictCellLoadTimes.Empty();
    TotalCellsLoaded = 0;
    MaxCellLoadTimeMs = 0.0f;

    Super::Deinitialize();
}

void UGSDStreamingTelemetry::RecordCellLoadTime(const FName& CellName, float LoadTimeMs, const FName& DistrictName)
{
    SCOPE_CYCLE_COUNTER(STAT_GSDRecordCellLoadTime);

    // Create record
    FGSDCellLoadTimeRecord Record;
    Record.CellName = CellName;
    Record.LoadTimeMs = LoadTimeMs;
    Record.DistrictName = DistrictName;
    Record.Timestamp = FPlatformTime::Seconds();

    // Get or create array for district
    TArray<FGSDCellLoadTimeRecord>& Records = DistrictCellLoadTimes.FindOrAdd(DistrictName);

    // Add record
    Records.Add(Record);

    // Prune old records if exceeding limit
    if (Records.Num() > MaxRecordsPerDistrict)
    {
        PruneOldRecords(DistrictName);
    }

    // Update statistics
    TotalCellsLoaded++;
    if (LoadTimeMs > MaxCellLoadTimeMs)
    {
        MaxCellLoadTimeMs = LoadTimeMs;
    }

    // Log slow loads
    if (bLogSlowLoads && LoadTimeMs > SlowLoadThresholdMs)
    {
        GSDTELEMETRY_LOG(Warning, TEXT("Slow cell load detected: %s in district %s took %.2fms (threshold: %.2fms)"),
            *CellName.ToString(), *DistrictName.ToString(), LoadTimeMs, SlowLoadThresholdMs);
    }

    // Broadcast delegate
    OnCellLoaded.Broadcast(CellName, LoadTimeMs);

    GSDTELEMETRY_LOG(Verbose, TEXT("Cell loaded: %s in %.2fms (district: %s)"),
        *CellName.ToString(), LoadTimeMs, *DistrictName.ToString());
}

void UGSDStreamingTelemetry::PruneOldRecords(const FName& DistrictName)
{
    TArray<FGSDCellLoadTimeRecord>* Records = DistrictCellLoadTimes.Find(DistrictName);
    if (Records && Records->Num() > MaxRecordsPerDistrict)
    {
        // Remove oldest records (keep most recent MaxRecordsPerDistrict)
        const int32 NumToRemove = Records->Num() - MaxRecordsPerDistrict;
        Records->RemoveAt(0, NumToRemove);
    }
}

TArray<FGSDCellLoadTimeRecord> UGSDStreamingTelemetry::GetCellLoadTimesByDistrict(const FName& DistrictName) const
{
    const TArray<FGSDCellLoadTimeRecord>* Records = DistrictCellLoadTimes.Find(DistrictName);
    return Records ? *Records : TArray<FGSDCellLoadTimeRecord>();
}

float UGSDStreamingTelemetry::GetAverageCellLoadTimeMs(const FName& DistrictName) const
{
    const TArray<FGSDCellLoadTimeRecord>* Records = DistrictCellLoadTimes.Find(DistrictName);
    if (!Records || Records->Num() == 0)
    {
        return 0.0f;
    }

    float TotalTime = 0.0f;
    for (const FGSDCellLoadTimeRecord& Record : *Records)
    {
        TotalTime += Record.LoadTimeMs;
    }

    return TotalTime / static_cast<float>(Records->Num());
}

void UGSDStreamingTelemetry::GetAllCellLoadTimes(TArray<FGSDCellLoadTimeRecord>& OutRecords) const
{
    OutRecords.Empty();

    for (const auto& Pair : DistrictCellLoadTimes)
    {
        OutRecords.Append(Pair.Value);
    }
}

void UGSDStreamingTelemetry::GetAllDistrictNames(TArray<FName>& OutDistrictNames) const
{
    DistrictCellLoadTimes.GetKeys(OutDistrictNames);
}
