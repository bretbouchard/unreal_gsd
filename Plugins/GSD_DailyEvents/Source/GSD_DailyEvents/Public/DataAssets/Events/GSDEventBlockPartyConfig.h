// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/GSDDailyEventConfig.h"
#include "GSDEventBlockPartyConfig.generated.h"

class UGSDSafeZoneModifier;

/**
 * Block Party event configuration.
 * Creates safe zones and spawns crowd props.
 *
 * EVT-07: Block Party event creates safe zones and crowd props
 */
UCLASS(BlueprintType, Category = "GSD|Events")
class GSD_DAILYEVENTS_API UGSDEventBlockPartyConfig : public UGSDDailyEventConfig
{
    GENERATED_BODY()

public:
    UGSDEventBlockPartyConfig();

    //-- Safe Zone Settings --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SafeZone")
    TObjectPtr<UGSDSafeZoneModifier> SafeZoneModifier;

    //-- Crowd Props --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Props")
    TArray<TSubclassOf<AActor>> CrowdPropClasses;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Props")
    int32 MinProps = 5;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Props")
    int32 MaxProps = 15;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Props")
    float PropSpawnRadius = 1000.0f;

    //-- Decorative FX --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
    TArray<TSubclassOf<AActor>> DecorativeFXClasses;

    //-- UGSDDailyEventConfig Interface --
    virtual bool ValidateConfig(FString& OutError) const override;
    virtual void OnEventStart_Implementation(UObject* WorldContext, FVector Location, float Intensity) override;
    virtual void OnEventEnd_Implementation(UObject* WorldContext) override;

protected:
    /** Track spawned props for cleanup */
    UPROPERTY()
    TArray<TObjectPtr<AActor>> SpawnedProps;

    /** Track spawned FX for cleanup */
    UPROPERTY()
    TArray<TObjectPtr<AActor>> SpawnedFX;

    /** Event center for cleanup */
    FVector EventCenter;

    /** Helper: Spawn props in random positions around center */
    void SpawnCrowdProps(UWorld* World, const FVector& Center, int32 Count);
};
