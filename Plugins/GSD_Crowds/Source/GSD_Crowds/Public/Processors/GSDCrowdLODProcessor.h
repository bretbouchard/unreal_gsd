// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "GSDCrowdLODProcessor.generated.h"

class UGSDCrowdConfig;
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
 *
 * Configuration is loaded from UGSDCrowdConfig DataAsset.
 * All hardcoded values have been replaced with config lookups.
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

    //-- Cached Config (loaded once per frame) --
    UPROPERTY(Transient)
    TObjectPtr<UGSDCrowdConfig> CachedConfig;

    //-- Fallback values if config not found (backward compatibility) --
    static constexpr float DefaultHighActorDistance = 2000.0f;
    static constexpr float DefaultLowActorDistance = 5000.0f;
    static constexpr float DefaultISMDistance = 10000.0f;
    static constexpr float DefaultCullDistance = 20000.0f;
    static constexpr float DefaultAudioLOD0Distance = 500.0f;
    static constexpr float DefaultAudioLOD1Distance = 2000.0f;
    static constexpr float DefaultAudioLOD2Distance = 4000.0f;
    static constexpr float DefaultAudioCullDistance = 5000.0f;
    static constexpr float DefaultAudioLOD1Volume = 0.5f;
    static constexpr float DefaultAudioLOD2Volume = 0.25f;

public:
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

    //-- Accessors for config values (used by tests and external code) --

    /** Get High Actor distance threshold */
    float GetHighActorDistance() const;

    /** Get Low Actor distance threshold */
    float GetLowActorDistance() const;

    /** Get ISM distance threshold */
    float GetISMDistance() const;

    /** Get Cull distance threshold */
    float GetCullDistance() const;

    /** Check if audio LOD is enabled */
    bool IsAudioLODEnabled() const;
};
