#pragma once

#include "CoreMinimal.h"
#include "GSDSpatialAudioTypes.generated.h"

// Spatial audio state for preservation across stream boundaries
USTRUCT(BlueprintType)
struct GSD_CORE_API FGSDSpatialAudioState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    float DistanceToListener = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float OcclusionFactor = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float ReverbSendLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bIsOccluded = false;

    UPROPERTY(BlueprintReadWrite)
    float VolumeMultiplier = 1.0f;

    UPROPERTY(BlueprintReadWrite)
    float PitchMultiplier = 1.0f;

    UPROPERTY(BlueprintReadWrite)
    float HighFrequencyGain = 1.0f;

    // Audio LOD level (0 = full quality, 2 = lowest)
    UPROPERTY(BlueprintReadWrite)
    int32 AudioLODLevel = 0;

    void Reset()
    {
        WorldLocation = FVector::ZeroVector;
        DistanceToListener = 0.0f;
        OcclusionFactor = 0.0f;
        ReverbSendLevel = 0.0f;
        bIsOccluded = false;
        VolumeMultiplier = 1.0f;
        PitchMultiplier = 1.0f;
        HighFrequencyGain = 1.0f;
        AudioLODLevel = 0;
    }
};

// Audio LOD configuration
USTRUCT(BlueprintType)
struct GSD_CORE_API FGSDAudioLODConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LOD0Distance = 500.0f; // 0-500m: Full spatialization

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LOD1Distance = 2000.0f; // 500-2000m: Reduced spatialization

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LOD2Distance = 5000.0f; // 2000-5000m: Mono/attenuation only

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxConcurrentLOD0 = 32;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxConcurrentLOD1 = 16;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxConcurrentLOD2 = 8;

    int32 GetLODLevelForDistance(float Distance) const
    {
        if (Distance < LOD0Distance) return 0;
        if (Distance < LOD1Distance) return 1;
        if (Distance < LOD2Distance) return 2;
        return -1; // Virtual (stopped)
    }
};
