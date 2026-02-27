// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/GSDTelemetryTypes.h"
#include "GSDStreamingTelemetry.generated.h"

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCellLoaded, const FName&, CellName, float, LoadTimeMs);

/**
 * Streaming telemetry subsystem for tracking World Partition cell load times by district
 * Implements TEL-03 (streaming cell load times in telemetry)
 */
UCLASS(Config=Game, DefaultConfig)
class GSD_TELEMETRY_API UGSDStreamingTelemetry : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Cell load time tracking
    UFUNCTION(BlueprintCallable, Category = "GSD|Telemetry|Streaming")
    void RecordCellLoadTime(const FName& CellName, float LoadTimeMs, const FName& DistrictName);

    UFUNCTION(BlueprintPure, Category = "GSD|Telemetry|Streaming")
    TArray<FGSDCellLoadTimeRecord> GetCellLoadTimesByDistrict(const FName& DistrictName) const;

    UFUNCTION(BlueprintPure, Category = "GSD|Telemetry|Streaming")
    float GetAverageCellLoadTimeMs(const FName& DistrictName) const;

    UFUNCTION(BlueprintPure, Category = "GSD|Telemetry|Streaming")
    void GetAllCellLoadTimes(TArray<FGSDCellLoadTimeRecord>& OutRecords) const;

    UFUNCTION(BlueprintPure, Category = "GSD|Telemetry|Streaming")
    void GetAllDistrictNames(TArray<FName>& OutDistrictNames) const;

    // Statistics
    UFUNCTION(BlueprintPure, Category = "GSD|Telemetry|Streaming")
    int32 GetTotalCellsLoaded() const { return TotalCellsLoaded; }

    UFUNCTION(BlueprintPure, Category = "GSD|Telemetry|Streaming")
    float GetMaxCellLoadTimeMs() const { return MaxCellLoadTimeMs; }

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "GSD|Telemetry|Streaming")
    FOnCellLoaded OnCellLoaded;

    // Configuration
    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Streaming")
    int32 MaxRecordsPerDistrict = 100;  // Circular buffer size

    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Streaming")
    float SlowLoadThresholdMs = 100.0f;  // Warn if cell load exceeds this

    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Streaming")
    bool bLogSlowLoads = true;

protected:
    void PruneOldRecords(const FName& DistrictName);

private:
    // Per-district cell load time records
    UPROPERTY()
    TMap<FName, TArray<FGSDCellLoadTimeRecord>> DistrictCellLoadTimes;

    // Statistics
    int32 TotalCellsLoaded = 0;
    float MaxCellLoadTimeMs = 0.0f;
};
