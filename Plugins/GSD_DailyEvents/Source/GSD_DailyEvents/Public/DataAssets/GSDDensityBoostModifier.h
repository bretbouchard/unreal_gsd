// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GSDEventModifierConfig.h"
#include "GSDDensityBoostModifier.generated.h"

/**
 * Concrete modifier that increases spawn density in an area.
 * Integrates with GSDCrowdManagerSubsystem via AddDensityModifier/RemoveDensityModifier.
 *
 * Usage:
 * 1. Create Data Asset of this class
 * 2. Set DensityMultiplier (e.g., 2.0 = double spawn rate)
 * 3. Reference from UGSDDailyEventConfig Modifiers array
 *
 * When event starts: ApplyModifier -> CrowdManager->AddDensityModifier()
 * When event ends: RemoveModifier -> CrowdManager->RemoveDensityModifier()
 *
 * This fulfills EVT-09: Event modifiers affect spawn densities and navigation.
 */
UCLASS(BlueprintType, Category = "GSD|Events|Modifiers")
class GSD_DAILYEVENTS_API UGSDDensityBoostModifier : public UGSDEventModifierConfig
{
    GENERATED_BODY()

public:
    UGSDDensityBoostModifier()
    {
        // Default to density boost tag
        ModifierTag = FGameplayTag::RequestGameplayTag(FName("Event.Modifier.DensityBoost"));
    }

    //-- Density Settings --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Density")
    float DensityMultiplier = 2.0f;

    //-- UGSDEventModifierConfig Interface --
    virtual void ApplyModifier_Implementation(UObject* WorldContext, FVector Center, float IntensityMultiplier) override;
    virtual void RemoveModifier_Implementation(UObject* WorldContext) override;
};
