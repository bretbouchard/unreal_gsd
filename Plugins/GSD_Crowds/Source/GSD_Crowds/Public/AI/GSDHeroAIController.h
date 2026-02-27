// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GSDHeroAIController.generated.h"

class UBehaviorTree;
class UGSDHeroAIConfig;

/**
 * Hero NPC AI Controller with Behavior Tree and AI Perception.
 * Used for gameplay-critical NPCs that need complex decision making.
 *
 * Key Features:
 * - Behavior Tree support for complex AI decision making
 * - AI Perception with Sight and Hearing senses
 * - Blackboard integration for state tracking
 * - Configurable via UGSDHeroAIConfig DataAsset
 *
 * Usage:
 * 1. Create a Behavior Tree asset with a Blackboard
 * 2. Create a UGSDHeroAIConfig DataAsset with perception settings
 * 3. Assign the config to the controller (or archetype)
 * 4. Controller will automatically start BT on possession
 * 5. Use blackboard keys (TargetActor, TargetLocation, etc.) in BT logic
 */
UCLASS(BlueprintType, Blueprintable, Category = "GSD|Crowds")
class GSD_CROWDS_API AGSDHeroAIController : public AAIController
{
    GENERATED_BODY()

public:
    AGSDHeroAIController();

    // AAIController interface
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void Tick(float DeltaTime) override;

    //-- Behavior Tree --

    /**
     * Start running a Behavior Tree asset.
     * @param BTAsset The behavior tree to run
     * @return True if successfully started
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|AI")
    bool RunBehaviorTreeAsset(UBehaviorTree* BTAsset);

    /**
     * Stop the currently running Behavior Tree.
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|AI")
    void StopBehaviorTree();

    //-- Perception --

    /**
     * Get all actors currently perceived by a specific sense.
     * @param Sense The sense class to query (nullptr = all senses)
     * @return Array of perceived actors
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|AI")
    TArray<AActor*> GetPerceivedActors(TSubclassOf<UAISense> Sense) const;

    /**
     * Check if this controller has line of sight to a target actor.
     * @param Target The actor to check
     * @return True if line of sight exists
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|AI")
    bool HasLineOfSightTo(AActor* Target) const;

    //-- Configuration --

    /**
     * Set the AI configuration at runtime.
     * @param NewConfig The new config to apply
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|AI")
    void SetAIConfig(UGSDHeroAIConfig* NewConfig);

    //-- Components --

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UBlackboardComponent> BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UAIPerceptionComponent> PerceptionComponent;

    //-- Legacy Behavior Tree Reference (for backwards compatibility) --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Behavior")
    TObjectPtr<UBehaviorTree> BehaviorTree;

    //-- Perception Configs --

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
    TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

    //-- AI Configuration --

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
    UGSDHeroAIConfig* AIConfig;

    //-- State --

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bBehaviorTreeRunning;

    //-- Blackboard Key Names --
    static const FName TargetActorKey;
    static const FName TargetLocationKey;
    static const FName CanSeeTargetKey;
    static const FName LastKnownPositionKey;
    static const FName HeardNoiseKey;
    static const FName NoiseLocationKey;

protected:
    //-- Default Perception Settings (used if no AIConfig) --
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

private:
    //-- Helper Methods --
    void InitializePerception();
    void ConfigureSight();
    void ConfigureHearing();

    //-- Perception Event --
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
};
