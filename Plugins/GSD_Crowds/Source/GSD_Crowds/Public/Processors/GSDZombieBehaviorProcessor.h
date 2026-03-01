// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "GSDZombieBehaviorProcessor.generated.h"

class UGSDCrowdConfig;
struct FGSDZombieStateFragment;
struct FDataFragment_Transform;

/**
 * Processor for generic crowd/flock behavior updates.
 *
 * This is a GAME-AGNOSTIC behavior processor suitable for any crowd simulation:
 * - Updates movement speed with natural variation
 * - Applies wander behavior for organic movement
 * - Pursuit/attack behavior for aggressive entities
 * - Works with any entity type (zombies, NPCs, animals, etc.)
 *
 * MIGRATION NOTE (GSDCROWDS-105):
 * The "Zombie" naming is deprecated. This processor will be renamed to
 * UGSDMassBehaviorProcessor in a future phase. Game code using
 * UGSDZombieBehaviorProcessor will need to update to the new name.
 *
 * Runs in PrePhysics phase before movement is applied.
 * Configuration is loaded from UGSDCrowdConfig and UGSDZombieBehaviorConfig DataAssets.
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

    //-- Pursuit/Attack Fallbacks --
    static constexpr float DefaultDetectionRange = 1000.0f;
    static constexpr float DefaultPursuitSpeedMultiplier = 2.0f;
    static constexpr float DefaultAttackRange = 100.0f;
    static constexpr float DefaultAttackCooldown = 1.0f;
    static constexpr float DefaultLoseTargetDistance = 2000.0f;
};

//-- Backward Compatibility Typedef (GSDCROWDS-105) --
//-- TODO(GSDCROWDS-105): Remove in future phase after migration complete --
/**
 * Deprecated: Use UGSDZombieBehaviorProcessor for now.
 * Will be replaced with UGSDMassBehaviorProcessor in future phase.
 *
 * This typedef exists to document the migration path.
 * Game code should plan to update when the new name is available.
 */
// typedef UGSDZombieBehaviorProcessor UGSDMassBehaviorProcessor;  // Uncomment when ready
