// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/GSDStreamingTelemetryTypes.h"
#include "GSDStreamingTelemetry.generated.h"

/**
 * Streaming telemetry subsystem for tracking cell load performance.
 *
 * PERFORMANCE NOTES (from Council review):
 * - Broadcasting on every streaming event is expensive
 * - Use MinBroadcastInterval to throttle broadcasts
 * - Use batched mode for aggregated updates
 * - MaxRecentEvents is configurable per-platform
 */
UCLASS(Config=Game, DefaultConfig)
class GSD_CITYSTREAMING_API UGSDStreamingTelemetry : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // === Event Logging ===

    /** Log a streaming event (internal or via test) */
    UFUNCTION(BlueprintCallable, Category = "GSD|Telemetry", meta = (DevelopmentOnly))
    void LogStreamingEvent(const FString& CellName, float LoadTimeMs, const FVector& PlayerPosition, float PlayerSpeed);

    // === Data Access ===

    /** Get recent streaming events */
    UFUNCTION(BlueprintPure, Category = "GSD|Telemetry")
    const TArray<FGSDStreamingEvent>& GetRecentEvents() const { return RecentEvents; }

    /** Get average load time from recent events */
    UFUNCTION(BlueprintPure, Category = "GSD|Telemetry")
    float GetAverageLoadTimeMs() const;

    /** Get aggregated telemetry data for Phase 10 */
    UFUNCTION(BlueprintPure, Category = "GSD|Telemetry")
    FGSDStreamingTelemetryData GetAggregatedData() const;

    /** Get peak load time from recent events */
    UFUNCTION(BlueprintPure, Category = "GSD|Telemetry")
    float GetPeakLoadTimeMs() const { return PeakLoadTimeMs; }

    /** Get bottleneck cell name (slowest loading) */
    UFUNCTION(BlueprintPure, Category = "GSD|Telemetry")
    FString GetBottleneckCell() const { return BottleneckCell; }

    // === Configuration ===

    /** Reset all telemetry data */
    UFUNCTION(BlueprintCallable, Category = "GSD|Telemetry")
    void ResetTelemetry();

    // === Delegates ===

    /** Broadcast when a streaming event is logged (throttled) */
    UPROPERTY(BlueprintAssignable, Category = "GSD|Telemetry")
    FOnStreamingEventLogged OnStreamingEventLogged;

    /** Broadcast batched telemetry updates (controlled by BroadcastInterval) */
    UPROPERTY(BlueprintAssignable, Category = "GSD|Telemetry")
    FOnStreamingTelemetryUpdated OnTelemetryUpdated;

    // === Scalability Settings (Config) ===

    /** Maximum recent events to store - configurable per-platform */
    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Performance",
        meta = (ClampMin = "10", ClampMax = "500"))
    int32 MaxRecentEvents = 100;

    /** Minimum interval between broadcasts (seconds) - prevents frame budget overrun */
    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Performance",
        meta = (ClampMin = "0.01", ClampMax = "1.0"))
    float MinBroadcastInterval = 0.1f;

    /** Enable/disable telemetry broadcasting (for low-end platforms) */
    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Performance")
    bool bEnableBroadcasting = true;

    /** Use batched mode instead of per-event broadcasting */
    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Performance")
    bool bUseBatchedMode = false;

protected:
    void OnStreamingProgressUpdated(UWorld* World);
    void TryBroadcastEvent(const FGSDStreamingEvent& Event);
    void UpdateBottleneckTracking(const FGSDStreamingEvent& Event);

    FDelegateHandle ProgressDelegateHandle;
    TArray<FGSDStreamingEvent> RecentEvents;

    // Performance tracking
    float LastBroadcastTime = 0.0f;
    float PeakLoadTimeMs = 0.0f;
    FString BottleneckCell;

private:
    void BindToWorldPartition();
    void UnbindFromWorldPartition();
};
