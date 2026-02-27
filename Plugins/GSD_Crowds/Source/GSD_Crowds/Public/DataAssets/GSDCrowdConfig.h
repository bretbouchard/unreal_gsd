// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GSDCrowdConfig.generated.h"

/**
 * Configurable parameters for crowd systems.
 * Replaces hardcoded values throughout crowd processors.
 *
 * This config centralizes all tunable crowd parameters including:
 * - Detection and perception settings
 * - LOD distance thresholds
 * - Entity limits and performance budgets
 * - Behavior parameters (speed, wandering)
 * - Navigation settings
 * - Audio LOD configuration
 * - Debug visualization options
 *
 * Create instances of this DataAsset in /GSD_Crowds/Config/ to customize
 * crowd behavior without code changes.
 */
UCLASS(BlueprintType, Category = "GSD|Crowds")
class GSD_CROWDS_API UGSDCrowdConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    // === Detection and Perception ===

    /** Detection radius for entities (replaces hardcoded 5000.0f in processors) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection")
    float DetectionRadius = 5000.0f;

    /** Vision angle for entity perception (degrees) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection")
    float VisionAngle = 90.0f;

    /** Hearing range for sound detection */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection")
    float HearingRange = 1500.0f;

    // === LOD Distances ===

    /** Distance for LOD 0 (full detail / high actor) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LOD")
    float LOD0Distance = 500.0f;

    /** Distance for LOD 1 (low actor representation) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LOD")
    float LOD1Distance = 2000.0f;

    /** Distance for LOD 2 (ISM representation) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LOD")
    float LOD2Distance = 4000.0f;

    /** Distance for LOD 3 (culled / invisible) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LOD")
    float LOD3CullDistance = 5000.0f;

    /** High actor distance threshold for LOD significance */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LOD")
    float HighActorDistance = 2000.0f;

    /** Low actor distance threshold for LOD significance */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LOD")
    float LowActorDistance = 5000.0f;

    /** ISM distance threshold for LOD significance */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LOD")
    float ISMDistance = 10000.0f;

    /** Cull distance threshold for LOD significance */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LOD")
    float CullDistance = 20000.0f;

    // === Entity Limits ===

    /** Maximum number of entities to spawn */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limits")
    int32 MaxEntityCount = 200;

    /** Entities per batch during spawning */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limits")
    int32 EntitiesPerBatch = 10;

    /** Frame budget for entity processing (ms) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limits")
    float ProcessingFrameBudget = 2.0f;

    // === Behavior Parameters ===

    /** Base movement speed (replaces hardcoded values) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
    float BaseMoveSpeed = 150.0f;

    /** Speed variation percentage (0.2 = 20% variation) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
    float SpeedVariationPercent = 0.2f;

    /** Maximum speed multiplier */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
    float MaxSpeedMultiplier = 1.5f;

    /** Wandering radius for random movement */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
    float WanderingRadius = 500.0f;

    /** Interval between behavior updates (seconds) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
    float BehaviorUpdateInterval = 0.5f;

    /** Wander direction change range (degrees) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
    float WanderDirectionChange = 45.0f;

    /** Speed interpolation rate */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
    float SpeedInterpolationRate = 2.0f;

    // === Navigation ===

    /** Lane search radius for ZoneGraph */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
    float LaneSearchRadius = 1000.0f;

    /** Smart object search cooldown (seconds) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
    float SmartObjectSearchCooldown = 5.0f;

    /** Interaction duration range (min) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
    float InteractionDurationMin = 3.0f;

    /** Interaction duration range (max) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
    float InteractionDurationMax = 8.0f;

    // === Audio ===

    /** Enable audio LOD system */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    bool bEnableAudioLOD = true;

    /** Audio LOD 0 distance (full volume) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    float AudioLOD0Distance = 500.0f;

    /** Audio LOD 1 distance (reduced volume) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    float AudioLOD1Distance = 2000.0f;

    /** Audio LOD 2 distance (minimal volume) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    float AudioLOD2Distance = 4000.0f;

    /** Audio cull distance (silent) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    float AudioCullDistance = 5000.0f;

    /** Audio LOD 1 volume multiplier */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    float AudioLOD1VolumeMultiplier = 0.5f;

    /** Audio LOD 2 volume multiplier */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    float AudioLOD2VolumeMultiplier = 0.25f;

    // === Debug ===

    /** Enable debug visualization */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
    bool bEnableDebugVisualization = false;

    /** Debug draw duration (0 = single frame) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
    float DebugDrawDuration = 0.0f;

    /**
     * Get the default crowd config.
     * @return Default config asset, or nullptr if not found
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowd")
    static UGSDCrowdConfig* GetDefaultConfig();
};
