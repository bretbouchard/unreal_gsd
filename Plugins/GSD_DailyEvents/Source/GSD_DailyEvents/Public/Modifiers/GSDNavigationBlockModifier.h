// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/GSDEventModifierConfig.h"
#include "Navigation/NavModifierVolume.h"
#include "GSDNavigationBlockModifier.generated.h"

/**
 * Concrete modifier that blocks AI navigation in an area.
 * Used by Construction event to close lanes.
 *
 * Uses NavModifierVolume to block NavMesh generation.
 * Falls back from ZoneGraph due to unclear runtime blocking API.
 */
UCLASS(BlueprintType, Category = "GSD|Events|Modifiers")
class GSD_DAILYEVENTS_API UGSDNavigationBlockModifier : public UGSDEventModifierConfig
{
    GENERATED_BODY()

public:
    UGSDNavigationBlockModifier();

    //-- Navigation Block Settings --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
    FVector VolumeExtent = FVector(500.0f, 500.0f, 200.0f);

    //-- UGSDEventModifierConfig Interface --
    virtual void ApplyModifier_Implementation(UObject* WorldContext, FVector Center, float IntensityMultiplier) override;
    virtual void RemoveModifier_Implementation(UObject* WorldContext) override;

protected:
    /** Track spawned volumes for cleanup */
    UPROPERTY()
    TArray<TObjectPtr<ANavModifierVolume>> SpawnedVolumes;
};
