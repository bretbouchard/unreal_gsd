// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/WorldPartitionStreamingSourceComponent.h"
#include "GSDStreamingSourceComponent.generated.h"

/**
 * Custom streaming source for vehicles and predictive loading.
 * Extends base World Partition streaming source with GSD-specific features.
 *
 * PERFORMANCE NOTES:
 * - Use EVENT-DRIVEN pattern via OnVehicleStateChanged()
 * - NEVER poll velocity in Tick - use state change delegates
 * - Hysteresis prevents rapid enable/disable cycling
 * - Hibernation reduces overhead for long-parked vehicles
 */
UCLASS(ClassGroup=(GSD), meta=(BlueprintSpawnableComponent))
class GSD_CITYSTREAMING_API UGSDStreamingSourceComponent : public UWorldPartitionStreamingSourceComponent
{
    GENERATED_BODY()

public:
    UGSDStreamingSourceComponent();

    // === Basic Streaming Control ===

    // Enable/disable streaming (useful for parked vehicles)
    UFUNCTION(BlueprintCallable, Category="GSD|Streaming")
    void SetStreamingEnabled(bool bEnabled);

    // Set custom loading range multiplier (for fast vehicles)
    UFUNCTION(BlueprintCallable, Category="GSD|Streaming")
    void SetLoadingRangeMultiplier(float Multiplier);

    // Get current streaming state
    UFUNCTION(BlueprintPure, Category="GSD|Streaming")
    bool IsStreamingEnabled() const { return bStreamingEnabled; }

    // === Vehicle Configuration ===

    /**
     * Configure this streaming source for vehicle use.
     * Sets up predictive loading and range multipliers.
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Streaming|Vehicle")
    void ConfigureForVehicle(bool bIsFastVehicle, float VelocityThreshold = 1000.0f);

    /**
     * Called when vehicle state changes (EVENT-DRIVEN).
     * CRITICAL: Call this from OnVehicleStateChanged delegate, NOT from Tick.
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Streaming|Vehicle")
    void OnVehicleStateChanged(bool bIsDriving, float CurrentVelocity);

    /**
     * Enable hibernation mode - completely disable streaming after delay.
     * Use for vehicles parked for extended periods.
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Streaming|Vehicle")
    void EnableHibernationMode(float DelaySeconds);

    /** Cancel pending hibernation */
    UFUNCTION(BlueprintCallable, Category = "GSD|Streaming|Vehicle")
    void CancelHibernation();

    // === Property Getters ===

    UFUNCTION(BlueprintPure, Category = "GSD|Streaming")
    float GetLoadingRangeMultiplier() const { return LoadingRangeMultiplier; }

    UFUNCTION(BlueprintPure, Category = "GSD|Streaming")
    bool IsPredictiveLoadingEnabled() const { return bPredictiveLoading; }

    UFUNCTION(BlueprintPure, Category = "GSD|Streaming")
    float GetPredictiveLoadingThreshold() const { return PredictiveLoadingVelocityThreshold; }

    UFUNCTION(BlueprintPure, Category = "GSD|Streaming|Vehicle")
    bool IsStreamingEnabledForVehicle() const { return bStreamingEnabled; }

    UFUNCTION(BlueprintPure, Category = "GSD|Streaming|Vehicle")
    bool IsHibernating() const { return bIsHibernating; }

protected:
    virtual void BeginPlay() override;

    // Override to apply custom settings
    virtual void UpdateStreamingSourceState();

    // === Vehicle-Specific Properties ===

    /** Hysteresis delay before disabling streaming for parked vehicles (seconds) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GSD|Streaming|Vehicle",
        meta = (ClampMin = "0.0", ClampMax = "60.0"))
    float ParkingHysteresisDelay = 5.0f;

    /** Velocity threshold to consider vehicle "fast" (cm/s) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GSD|Streaming|Vehicle")
    float FastVehicleThreshold = 2000.0f;

    /** Loading range multiplier for fast vehicles */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GSD|Streaming|Vehicle",
        meta = (ClampMin = "1.0", ClampMax = "5.0"))
    float FastVehicleRangeMultiplier = 2.0f;

    /** Hibernation delay for long-parked vehicles (seconds) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GSD|Streaming|Vehicle",
        meta = (ClampMin = "10.0", ClampMax = "300.0"))
    float HibernationDelay = 30.0f;

    // Hysteresis timer handle
    FTimerHandle ParkingHysteresisTimer;

    // Hibernation timer handle
    FTimerHandle HibernationTimer;

    // State tracking
    bool bIsHibernating = false;
    bool bPendingDisable = false;

private:
    // Is streaming currently enabled for this source?
    UPROPERTY(EditAnywhere, Category="GSD|Streaming")
    bool bStreamingEnabled = true;

    // Multiplier for loading range (1.0 = default, 2.0 = double range for fast vehicles)
    UPROPERTY(EditAnywhere, Category="GSD|Streaming", meta=(ClampMin="0.5", ClampMax="5.0"))
    float LoadingRangeMultiplier = 1.0f;

    // Should this source load cells ahead of movement direction?
    UPROPERTY(EditAnywhere, Category="GSD|Streaming")
    bool bPredictiveLoading = false;

    // Velocity threshold for predictive loading (in cm/s)
    UPROPERTY(EditAnywhere, Category="GSD|Streaming")
    float PredictiveLoadingVelocityThreshold = 1000.0f;

    // Cached owner velocity for predictive calculations
    FVector CachedVelocity;
};
