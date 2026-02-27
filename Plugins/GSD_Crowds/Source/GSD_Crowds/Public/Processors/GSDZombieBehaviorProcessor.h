// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "GSDZombieBehaviorProcessor.generated.h"

class UGSDCrowdConfig;
struct FGSDZombieStateFragment;
struct FDataFragment_Transform;

/**
 * Processor for zombie behavior updates.
 *
 * Updates movement speed, wander behavior, and state.
 * Runs in PrePhysics phase before movement is applied.
 *
 * Configuration is loaded from UGSDCrowdConfig DataAsset.
 * All hardcoded values have been replaced with config lookups.
 */
UCLASS()
class GSD_CROWDS_API UGSDZombieBehaviorProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UGSDZombieBehaviorProcessor();

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
    static constexpr float DefaultBehaviorUpdateInterval = 0.5f;
    static constexpr float DefaultSpeedVariation = 0.2f;
    static constexpr float DefaultWanderDirectionChange = 45.0f;
    static constexpr float DefaultSpeedInterpolationRate = 2.0f;
};
