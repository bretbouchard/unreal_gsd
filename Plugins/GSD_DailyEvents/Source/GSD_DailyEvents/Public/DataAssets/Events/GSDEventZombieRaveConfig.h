// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/GSDDailyEventConfig.h"
#include "GSDEventZombieRaveConfig.generated.h"

class USoundBase;
class UNiagaraSystem;
class UNiagaraComponent;
class UAudioComponent;
class UGSDDensityBoostModifier;

/**
 * Zombie Rave event configuration.
 * Boosts density with attractor volumes, audio, and VFX.
 *
 * EVT-08: Zombie Rave event boosts density with attractor volumes and audio
 */
UCLASS(BlueprintType, Category = "GSD|Events")
class GSD_DAILYEVENTS_API UGSDEventZombieRaveConfig : public UGSDDailyEventConfig
{
    GENERATED_BODY()

public:
    UGSDEventZombieRaveConfig();

    //-- Audio Settings --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> RaveMusic;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    float AudioRadius = 3000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    float VolumeMultiplier = 1.0f;

    //-- VFX Settings --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
    TObjectPtr<UNiagaraSystem> RaveFX;

    //-- Density Boost --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Density")
    TObjectPtr<UGSDDensityBoostModifier> DensityBooster;

    //-- UGSDDailyEventConfig Interface --
    virtual bool ValidateConfig(FString& OutError) const override;
    virtual void OnEventStart_Implementation(UObject* WorldContext, FVector Location, float Intensity) override;
    virtual void OnEventEnd_Implementation(UObject* WorldContext) override;

protected:
    /** Spawned audio component for cleanup */
    UPROPERTY()
    TObjectPtr<UAudioComponent> SpawnedAudio;

    /** Spawned FX component for cleanup */
    UPROPERTY()
    TObjectPtr<UNiagaraComponent> SpawnedFX;

    /** Event center for cleanup */
    FVector EventCenter;
};
