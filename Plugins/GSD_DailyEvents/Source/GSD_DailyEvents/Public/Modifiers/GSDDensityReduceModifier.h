// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/GSDEventModifierConfig.h"
#include "GSDDensityReduceModifier.generated.h"

/**
 * Concrete modifier that decreases spawn density in an area.
 * Used by Bonfire event to create low-density zones.
 *
 * Inverse of GSDDensityBoostModifier - reduces instead of boosts.
 */
UCLASS(BlueprintType, Category = "GSD|Events|Modifiers")
class GSD_DAILYEVENTS_API UGSDDensityReduceModifier : public UGSDEventModifierConfig
{
    GENERATED_BODY()

public:
    UGSDDensityReduceModifier();

    //-- Density Settings --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Density")
    float DensityMultiplier = 0.25f;  // 25% of normal density

    //-- UGSDEventModifierConfig Interface --
    virtual void ApplyModifier_Implementation(UObject* WorldContext, FVector Center, float IntensityMultiplier) override;
    virtual void RemoveModifier_Implementation(UObject* WorldContext) override;
};
