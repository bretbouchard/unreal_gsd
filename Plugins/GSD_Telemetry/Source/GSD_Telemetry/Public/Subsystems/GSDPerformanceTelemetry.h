// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/GSDTelemetryTypes.h"
#include "GSDPerformanceTelemetry.generated.h"

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHitchDetected, float, HitchTimeMs, const FName&, DistrictName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorCountUpdated, const FGSDActorCountSnapshot&, Snapshot);

/**
 * Performance telemetry subsystem for tracking frame time, hitches, and actor counts by district
 * Implements TEL-01 (frame time/hitch tracking) and TEL-02 (actor counts)
 */
UCLASS(Config=Game, DefaultConfig)
class GSD_TELEMETRY_API UGSDPerformanceTelemetry : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Frame time tracking
    UFUNCTION(BlueprintCallable, Category = "GSD|Telemetry")
    void RecordFrameTime(float FrameTimeMs, const FName& DistrictName);

    UFUNCTION(BlueprintPure, Category = "GSD|Telemetry")
    float GetAverageFrameTimeMs(const FName& DistrictName) const;

    UFUNCTION(BlueprintPure, Category = "GSD|Telemetry")
    int32 GetHitchCount(const FName& DistrictName) const;

    // Actor counting (called periodically by timer)
    UFUNCTION(BlueprintCallable, Category = "GSD|Telemetry")
    void CountActors();

    UFUNCTION(BlueprintPure, Category = "GSD|Telemetry")
    FGSDActorCountSnapshot GetLatestActorCount() const { return LatestActorCount; }

    // Data access
    UFUNCTION(BlueprintPure, Category = "GSD|Telemetry")
    void GetAllDistrictNames(TArray<FName>& OutDistrictNames) const;

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "GSD|Telemetry")
    FOnHitchDetected OnHitchDetected;

    UPROPERTY(BlueprintAssignable, Category = "GSD|Telemetry")
    FOnActorCountUpdated OnActorCountUpdated;

    // Configuration
    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Performance")
    float HitchThresholdMs = 16.67f;  // 60fps target (16.67ms)

    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Performance")
    float ActorCountInterval = 5.0f;  // Log actor counts every 5 seconds

    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Performance")
    bool bEnableActorCounting = true;

    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Performance")
    bool bEnableHitchDetection = true;

protected:
    void RecordHitch(float HitchTimeMs, const FName& DistrictName);
    void StartActorCountTimer();

private:
    // Per-district frame time history (circular buffers)
    UPROPERTY()
    TMap<FName, FGSDFrameTimeHistory> DistrictFrameTimes;

    // Per-district hitch counts
    UPROPERTY()
    TMap<FName, int32> DistrictHitchCounts;

    // Recent hitch events (for debugging)
    UPROPERTY()
    TArray<FGSDHitchEvent> RecentHitches;

    // Latest actor count snapshot
    UPROPERTY()
    FGSDActorCountSnapshot LatestActorCount;

    // Timer handle for periodic actor counting
    FTimerHandle ActorCountTimerHandle;

    // Circular buffer for recent hitches
    static constexpr int32 MaxRecentHitches = 100;
};
