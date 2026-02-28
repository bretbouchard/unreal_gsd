// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/GSDDailyEventConfig.h"
#include "GSDEventBonfireConfig.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class UGSDDensityReduceModifier;

/**
 * Bonfire event configuration.
 * Spawns visual FX and reduces zombie density locally.
 *
 * EVT-06: Bonfire event spawns FX and modifies zombie density locally
 */
UCLASS(BlueprintType, Category = "GSD|Events")
class GSD_DAILYEVENTS_API UGSDEventBonfireConfig : public UGSDDailyEventConfig
{
    GENERATED_BODY()

public:
    UGSDEventBonfireConfig();

    //-- VFX Settings --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
    TObjectPtr<UNiagaraSystem> BonfireFX;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
    FVector FXScale = FVector(1.0f);

    //-- Density Reduction --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Density")
    TObjectPtr<UGSDDensityReduceModifier> DensityReducer;

    //-- UGSDDailyEventConfig Interface --
    virtual bool ValidateConfig(FString& OutError) const override;
    virtual void OnEventStart_Implementation(UObject* WorldContext, FVector Location, float Intensity) override;
    virtual void OnEventEnd_Implementation(UObject* WorldContext) override;

protected:
    /** Spawned FX component for cleanup */
    UPROPERTY()
    TObjectPtr<UNiagaraComponent> SpawnedFXComponent;

    /** Event center for modifier removal */
    FVector EventCenter;
};
