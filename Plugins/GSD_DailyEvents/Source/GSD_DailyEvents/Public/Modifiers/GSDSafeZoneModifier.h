// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/GSDEventModifierConfig.h"
#include "GSDSafeZoneModifier.generated.h"

/**
 * Concrete modifier that creates a safe zone by reducing density to near-zero.
 * Used by BlockParty event to create survivor-safe areas.
 *
 * Uses extremely low multiplier (0.01) to effectively prevent spawns.
 */
UCLASS(BlueprintType, Category = "GSD|Events|Modifiers")
class GSD_DAILYEVENTS_API UGSDSafeZoneModifier : public UGSDEventModifierConfig
{
    GENERATED_BODY()

public:
    UGSDSafeZoneModifier();

    //-- Safe Zone Settings --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SafeZone")
    float DensityMultiplier = 0.01f;  // 1% spawn rate = effectively blocked

    //-- UGSDEventModifierConfig Interface --
    virtual void ApplyModifier_Implementation(UObject* WorldContext, FVector Center, float IntensityMultiplier) override;
    virtual void RemoveModifier_Implementation(UObject* WorldContext) override;
};
