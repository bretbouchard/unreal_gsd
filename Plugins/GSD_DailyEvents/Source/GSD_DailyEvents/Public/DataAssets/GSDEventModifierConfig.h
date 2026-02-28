// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GSDDataAsset.h"
#include "GSDEventModifierConfig.generated.h"

/**
 * Base class for event modifiers.
 * Modifiers are reusable effects that can be applied by events.
 *
 * Examples:
 * - DensityBoost: Increases spawn density in area
 * - NavigationBlock: Closes navigation lanes
 * - SafeZone: Disables hostile spawns in area
 *
 * CRITICAL: Always pair Apply with Remove to prevent state leaks.
 * The Apply/Remove pattern ensures events can be cleanly reversed.
 *
 * Usage:
 * 1. Create a blueprint extending this class
 * 2. Set ModifierTag, Radius, Intensity
 * 3. Override ApplyModifier_Implementation and RemoveModifier_Implementation
 * 4. Add to UGSDDailyEventConfig Modifiers array
 */
UCLASS(Abstract, BlueprintType, Category = "GSD|Events")
class GSD_DAILYEVENTS_API UGSDEventModifierConfig : public UGSDDataAsset
{
    GENERATED_BODY()

public:
    //-- Modifier Identity --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
    FGameplayTag ModifierTag;

    //-- Area of Effect --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
    float Radius = 1000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
    float Intensity = 1.0f;

    //-- Validation --
    virtual bool ValidateConfig(FString& OutError) const override;

    //-- Apply/Remove Pattern --

    /**
     * Apply this modifier to the target system.
     * Override in subclasses to affect CrowdManager, VehicleSpawner, etc.
     *
     * CRITICAL: Store state for removal. Must be reversible.
     *
     * @param WorldContext Object with world context (for subsystem access)
     * @param Center World location of modifier center
     * @param IntensityMultiplier Multiplier applied to base Intensity
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Modifier")
    void ApplyModifier(UObject* WorldContext, FVector Center, float IntensityMultiplier = 1.0f);
    virtual void ApplyModifier_Implementation(UObject* WorldContext, FVector Center, float IntensityMultiplier = 1.0f);

    /**
     * Remove this modifier from the target system.
     * CRITICAL: Must undo everything done in ApplyModifier.
     *
     * @param WorldContext Object with world context (for subsystem access)
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Modifier")
    void RemoveModifier(UObject* WorldContext);
    virtual void RemoveModifier_Implementation(UObject* WorldContext);

protected:
    /** Last applied center for cleanup */
    FVector LastAppliedCenter = FVector::ZeroVector;

    /** Last applied intensity multiplier for cleanup */
    float LastIntensityMultiplier = 1.0f;
};
