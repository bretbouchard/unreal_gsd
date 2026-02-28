// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GSDDataAsset.h"
#include "GSDDailyEventConfig.generated.h"

class UGSDEventModifierConfig;

/**
 * Base class for daily event definitions.
 * Extend this for specific event types (Construction, Bonfire, etc.)
 *
 * Events are scheduled by GSDEventSchedulerSubsystem and executed
 * at their scheduled time with the configured parameters.
 *
 * Usage:
 * 1. Create a blueprint extending this class
 * 2. Set EventTag, DisplayName, DurationMinutes
 * 3. Add modifiers to the Modifiers array
 * 4. Override OnEventStart/OnEventEnd for custom behavior
 *
 * Lifecycle:
 * - OnEventStart: Called when event begins (apply modifiers, spawn actors)
 * - OnEventEnd: Called when event ends (remove modifiers, despawn actors)
 */
UCLASS(Abstract, BlueprintType, Category = "GSD|Events")
class GSD_DAILYEVENTS_API UGSDDailyEventConfig : public UGSDDataAsset
{
    GENERATED_BODY()

public:
    //-- Event Identity --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Event")
    FGameplayTag EventTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Event")
    FText DisplayName;

    //-- Timing --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timing")
    float DurationMinutes = 60.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timing")
    TArray<FName> ValidTimeSlots;  // "Morning", "Afternoon", "Evening"

    //-- Modifiers --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifiers")
    TArray<TSoftObjectPtr<UGSDEventModifierConfig>> Modifiers;

    //-- Spawn Requirements --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
    TArray<FName> RequiredDataLayers;

    //-- Validation --
    virtual bool ValidateConfig(FString& OutError) const override;

    //-- Execution --

    /**
     * Called when event starts. Override in subclasses.
     * Apply modifiers, spawn actors, activate data layers.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Event")
    void OnEventStart(UObject* WorldContext, FVector Location, float Intensity);
    virtual void OnEventStart_Implementation(UObject* WorldContext, FVector Location, float Intensity);

    /**
     * Called when event ends. Override in subclasses.
     * Remove modifiers, despawn actors, deactivate data layers.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Event")
    void OnEventEnd(UObject* WorldContext);
    virtual void OnEventEnd_Implementation(UObject* WorldContext);

protected:
    /** Track applied modifiers for cleanup */
    UPROPERTY()
    TArray<TObjectPtr<UGSDEventModifierConfig>> AppliedModifiers;

    /** Last applied location for cleanup */
    FVector LastAppliedLocation = FVector::ZeroVector;

    /** Last applied intensity for cleanup */
    float LastAppliedIntensity = 1.0f;
};
