// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WorldPartition/WorldPartitionStreamingSourceComponent.h"
#include "GSDStreamingSourceComponent.generated.h"

/**
 * Custom streaming source for vehicles and predictive loading.
 * Extends base World Partition streaming source with GSD-specific features.
 */
UCLASS(ClassGroup=(GSD), meta=(BlueprintSpawnableComponent))
class GSD_CITYSTREAMING_API UGSDStreamingSourceComponent : public UWorldPartitionStreamingSourceComponent
{
    GENERATED_BODY()

public:
    UGSDStreamingSourceComponent();

    // Enable/disable streaming (useful for parked vehicles)
    UFUNCTION(BlueprintCallable, Category="GSD|Streaming")
    void SetStreamingEnabled(bool bEnabled);

    // Set custom loading range multiplier (for fast vehicles)
    UFUNCTION(BlueprintCallable, Category="GSD|Streaming")
    void SetLoadingRangeMultiplier(float Multiplier);

    // Get current streaming state
    UFUNCTION(BlueprintPure, Category="GSD|Streaming")
    bool IsStreamingEnabled() const { return bStreamingEnabled; }

protected:
    virtual void BeginPlay() override;

    // Override to apply custom settings
    virtual void UpdateStreamingSourceState();

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
