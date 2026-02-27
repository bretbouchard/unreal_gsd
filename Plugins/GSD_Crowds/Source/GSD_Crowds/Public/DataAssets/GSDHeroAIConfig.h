// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GSDHeroAIConfig.generated.h"

class UBehaviorTree;
class UBlackboardData;

/**
 * Configuration for Hero NPC AI systems.
 * Provides Behavior Tree, Perception settings, and AI parameters.
 *
 * Designers create these as DataAssets to configure different AI behaviors
 * without code changes. Each Hero NPC archetype can reference a different
 * AI config for varied behaviors.
 *
 * Usage:
 * 1. Create a DataAsset derived from GSDHeroAIConfig
 * 2. Set Behavior Tree and perception parameters
 * 3. Assign to AGSDHeroAIController or UGSDHeroNPCArchetype
 * 4. Controller will use these settings on initialization
 */
UCLASS(BlueprintType)
class GSD_CROWDS_API UGSDHeroAIConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    // === Behavior Tree ===

    /** Default Behavior Tree to run on possess */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior Tree")
    UBehaviorTree* DefaultBehaviorTree;

    /** Blackboard asset (usually part of BT, but can override) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior Tree")
    UBlackboardData* BlackboardAsset;

    // === Sight Perception ===

    /** Sight detection radius (cm) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception|Sight")
    float SightRadius = 2000.0f;

    /** Distance at which sight is lost (cm) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception|Sight")
    float LoseSightRadius = 2500.0f;

    /** Peripheral vision angle (degrees) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception|Sight")
    float PeripheralVisionAngle = 90.0f;

    // === Hearing Perception ===

    /** Hearing detection range (cm) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception|Hearing")
    float HearingRange = 1500.0f;

    /** Maximum age of hearing stimulus (seconds) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception|Hearing")
    float HearingStimulusMaxAge = 3.0f;

    // === Combat ===

    /** Engagement range (start attacking) (cm) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float EngagementRange = 1000.0f;

    /** Attack range (melee distance) (cm) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float AttackRange = 150.0f;

    /** Time between attacks (seconds) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float AttackCooldown = 1.5f;

    // === Patrol ===

    /** Patrol point reach tolerance (cm) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Patrol")
    float PatrolPointTolerance = 100.0f;

    /** Wait time at patrol points (seconds) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Patrol")
    float PatrolWaitTime = 3.0f;

    // === Movement ===

    /** Maximum walk speed (cm/s) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    float WalkSpeed = 200.0f;

    /** Maximum run speed (cm/s) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    float RunSpeed = 400.0f;

    /** Maximum sprint speed (cm/s) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    float SprintSpeed = 600.0f;

    /**
     * Get the default Hero AI config.
     * @return Default config asset, or nullptr if not found
     */
    UFUNCTION(BlueprintPure, Category = "GSD|AI")
    static UGSDHeroAIConfig* GetDefaultConfig();
};
