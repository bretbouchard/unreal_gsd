// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "GSDCrowdLODProcessor.generated.h"

struct FMassRepresentationLODFragment;
struct FDataFragment_Transform;

/**
 * Processor for calculating LOD significance based on viewer distance.
 *
 * LOD Significance values:
 * - 0.0 - 0.5: High Actor (full detail)
 * - 0.5 - 1.5: Low Actor (simplified)
 * - 1.5 - 2.5: ISM (instanced mesh)
 * - 2.5 - 3.0: Culled (invisible)
 */
UCLASS()
class GSD_CROWDS_API UGSDCrowdLODProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UGSDCrowdLODProcessor();

protected:
    // ~UMassProcessor interface
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
    // ~End of UMassProcessor interface

private:
    FMassEntityQuery EntityQuery;

    //-- LOD Distance Thresholds --
    UPROPERTY(EditDefaultsOnly, Category = "LOD Configuration")
    float HighActorDistance = 2000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "LOD Configuration")
    float LowActorDistance = 5000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "LOD Configuration")
    float ISMDistance = 10000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "LOD Configuration")
    float CullDistance = 20000.0f;

public:
    //-- Audio LOD Configuration --
    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    bool bEnableAudioLOD = true;

    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    float AudioLOD0Distance = 500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    float AudioLOD1Distance = 2000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    float AudioLOD2Distance = 4000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    float AudioCullDistance = 5000.0f;

    /**
     * Calculate LOD significance from distance.
     * Returns value 0.0-3.0 based on LOD thresholds.
     */
    float CalculateLODSignificance(float Distance) const;

    /**
     * Get viewer location (player camera).
     * Returns world location of primary player camera.
     */
    FVector GetViewerLocation(FMassExecutionContext& Context) const;

    /**
     * Calculate audio LOD volume based on distance to listener.
     * Returns volume multiplier (1.0 = full, 0.0 = silent).
     */
    float CalculateAudioLODVolume(float DistanceToListener) const;

    /**
     * Check if audio should be culled based on distance.
     * Returns true if audio should not play.
     */
    bool ShouldCullAudio(float DistanceToListener) const;
};
