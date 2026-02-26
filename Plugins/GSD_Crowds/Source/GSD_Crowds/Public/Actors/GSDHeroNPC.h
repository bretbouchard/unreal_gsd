// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Interfaces/IGSDSpawnable.h"
#include "GSDHeroNPC.generated.h"

class UCapsuleComponent;
class USkeletalMeshComponent;
class UCharacterMovementComponent;
class UBehaviorTree;
class AGSDHeroAIController;

/**
 * Hero NPC pawn for gameplay-critical NPCs.
 * Uses Behavior Trees and AI Perception for intelligent behavior.
 * Implements IGSDSpawnable for GSD spawning integration.
 *
 * Key Features:
 * - AI Perception Stimuli Source for being detected by other NPCs
 * - Auto-possesses AI controller for autonomous behavior
 * - IGSDSpawnable interface for GSD spawning system
 * - Capsule + SkeletalMesh + CharacterMovement for pawn physics
 *
 * Usage:
 * 1. Create a Blueprint based on this class
 * 2. Assign a Behavior Tree asset
 * 3. Configure perception settings as needed
 * 4. Spawn via GSD spawning system or place in level
 */
UCLASS(BlueprintType, Blueprintable, Category = "GSD|Crowds")
class GSD_CROWDS_API AGSDHeroNPC : public APawn, public IGSDSpawnable
{
    GENERATED_BODY()

public:
    AGSDHeroNPC();

    virtual void BeginPlay() override;

    //-- IGSDSpawnable Interface --
    virtual void SpawnFromConfig(UGSDDataAsset* Config) override;
    virtual UGSDDataAsset* GetSpawnConfig() override;
    virtual bool IsSpawned() override;
    virtual void Despawn() override;
    virtual void ResetSpawnState() override;
    // ~End of IGSDSpawnable Interface

    //-- Components --
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCapsuleComponent> CapsuleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> SkeletalMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCharacterMovementComponent> CharacterMovement;

    //-- AI Perception Stimuli Source --
    // Required for other NPCs to perceive this one
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    TObjectPtr<UAIPerceptionStimuliSourceComponent> PerceptionStimuliSource;

    //-- AI Configuration --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    TObjectPtr<UBehaviorTree> BehaviorTree;

    //-- AI Controller Class --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    TSubclassOf<AGSDHeroAIController> AIControllerClass;

protected:
    //-- Default Dimensions --
    UPROPERTY(EditDefaultsOnly, Category = "Physics")
    float DefaultCapsuleRadius = 34.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Physics")
    float DefaultCapsuleHalfHeight = 88.0f;

    //-- Spawn Config --
    UPROPERTY()
    TObjectPtr<UGSDDataAsset> SpawnConfigAsset;

    bool bIsSpawned = false;
};
