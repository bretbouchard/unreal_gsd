// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GSDZombieBehaviorConfig.generated.h"

/**
 * Game-specific behavior configuration for zombie-like entities.
 *
 * This config extends GSDCrowdConfig with game-specific behaviors like:
 * - Target detection and acquisition
 * - Attack behaviors and cooldowns
 * - State transitions (idle → alert → pursue → attack)
 *
 * Create instances in /GSD_Crowds/Config/ to customize zombie behavior.
 */
UCLASS(BlueprintType, Category = "GSD|Crowds")
class GSD_CROWDS_API UGSDZombieBehaviorConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    // === Target Detection ===

    /** Enable target detection behavior */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection")
    bool bEnableTargetDetection = true;

    /** Detection radius for finding targets */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection")
    float DetectionRadius = 1000.0f;

    /** Vision cone angle (degrees, 180 = full front arc) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection")
    float VisionConeAngle = 90.0f;

    /** Time between detection scans (seconds) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection")
    float DetectionScanInterval = 0.5f;

    // === Pursuit Behavior ===

    /** Enable pursuit behavior (chasing detected targets) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pursuit")
    bool bEnablePursuit = true;

    /** Speed multiplier when pursuing (relative to base speed) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pursuit")
    float PursuitSpeedMultiplier = 2.0f;

    /** Distance at which target is considered lost */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pursuit")
    float LoseTargetDistance = 2000.0f;

    /** Maximum pursuit duration before giving up (seconds, 0 = infinite) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pursuit")
    float MaxPursuitDuration = 30.0f;

    // === Attack Behavior ===

    /** Enable attack behavior */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    bool bEnableAttack = true;

    /** Attack range (distance from target) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    float AttackRange = 100.0f;

    /** Attack cooldown (seconds between attacks) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    float AttackCooldown = 1.0f;

    /** Damage per attack */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    float AttackDamage = 10.0f;

    /** Stop moving when attacking */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    bool bStopOnAttack = true;

    // === State Transition Thresholds ===

    /** Time in idle before becoming alert (seconds) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "State Transitions")
    float IdleToAlertTime = 2.0f;

    /** Time without target before returning to idle (seconds) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "State Transitions")
    float AlertToIdleTime = 5.0f;

    // === Aggression Settings ===

    /** Base aggression level (0-1, affects detection range and pursuit) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aggression")
    float BaseAggression = 0.5f;

    /** Increase aggression when health is below this percentage */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aggression")
    float LowHealthThreshold = 0.3f;

    /** Aggression multiplier when at low health */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aggression")
    float LowHealthAggressionMultiplier = 1.5f;

    /**
     * Get the default zombie behavior config.
     * @return Default config asset, or nullptr if not found
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds")
    static UGSDZombieBehaviorConfig* GetDefaultConfig();
};
