#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GSDCityLevelActor.generated.h"

class UGSDHLODConfigAsset;

/**
 * Configuration actor for city levels with World Partition.
 * Placed in level to apply streaming and HLOD settings.
 *
 * This actor provides a centralized place for level configuration and follows
 * the AGSDActor pattern from Phase 1, but is lightweight (no spawning/streaming interfaces)
 * since it's purely for configuration purposes.
 */
UCLASS(BlueprintType, ClassGroup=(GSD))
class GSD_CITYSTREAMING_API AGSDCityLevelActor : public AActor
{
    GENERATED_BODY()

public:
    AGSDCityLevelActor();

    // === Grid Configuration ===

    /** Grid cell size in Unreal Units (default 25600 = 256m) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="World Partition|Grid", meta=(ClampMin=12800, ClampMax=102400))
    int32 GridCellSize = 25600;

    /** Loading range in Unreal Units (default 76800 = 768m) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="World Partition|Grid", meta=(ClampMin=25600))
    int32 LoadingRange = 76800;

    // === Streaming Configuration ===

    /** Enable predictive loading for fast-moving entities */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="World Partition|Streaming")
    bool bEnablePredictiveLoading = true;

    /** Block on slow streaming (prevents pop-in) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="World Partition|Streaming")
    bool bBlockOnSlowStreaming = true;

    // === HLOD Configuration ===

    /** HLOD configuration reference */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HLOD")
    UGSDHLODConfigAsset* HLODConfig;

protected:
    virtual void BeginPlay() override;

private:
    /** Log configuration on startup for debugging */
    void LogConfiguration();
};
