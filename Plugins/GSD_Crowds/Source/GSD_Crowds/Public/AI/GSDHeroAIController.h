// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GSDHeroAIController.generated.h"

class UBehaviorTree;
class UBlackboardComponent;

/**
 * Hero NPC AI Controller with Behavior Tree and AI Perception.
 * Used for gameplay-critical NPCs that need complex decision making.
 *
 * Key Features:
 * - Behavior Tree support for complex AI decision making
 * - AI Perception with Sight and Hearing senses
 * - Blackboard integration for state tracking
 *
 * Usage:
 * 1. Create a Behavior Tree asset with a Blackboard
 * 2. Assign the Behavior Tree to the controller
 * 3. Controller will automatically start BT on possession
 * 4. Use blackboard keys (TargetActor, TargetLocation, etc.) in BT logic
 */
UCLASS(BlueprintType, Blueprintable, Category = "GSD|Crowds")
class GSD_CROWDS_API AGSDHeroAIController : public AAIController
{
    GENERATED_BODY()

public:
    AGSDHeroAIController();

    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

    //-- Behavior Tree --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Behavior")
    TObjectPtr<UBehaviorTree> BehaviorTree;

    //-- AI Perception --
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
    TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

    //-- Sight Config --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    //-- Hearing Config --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
    TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

    //-- Perception Event --
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    //-- Blackboard Key Names --
    static const FName TargetActorKey;
    static const FName TargetLocationKey;
    static const FName CanSeeTargetKey;
    static const FName LastKnownPositionKey;
    static const FName HeardNoiseKey;
    static const FName NoiseLocationKey;

protected:
    //-- Default Perception Settings --
    UPROPERTY(EditDefaultsOnly, Category = "AI|Perception")
    float DefaultSightRadius = 2000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "AI|Perception")
    float DefaultLoseSightRadius = 2500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "AI|Perception")
    float DefaultPeripheralVisionAngle = 90.0f;

    UPROPERTY(EditDefaultsOnly, Category = "AI|Perception")
    float DefaultHearingRange = 1500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "AI|Perception")
    float DefaultStimulusMaxAge = 5.0f;
};
