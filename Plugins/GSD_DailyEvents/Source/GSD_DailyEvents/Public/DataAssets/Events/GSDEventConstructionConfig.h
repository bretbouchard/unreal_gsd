// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/GSDDailyEventConfig.h"
#include "GSDEventConstructionConfig.generated.h"

class UGSDNavigationBlockModifier;

/**
 * Construction event configuration.
 * Spawns barricades and closes navigation lanes.
 *
 * EVT-05: Construction event spawns barricades and closes lanes
 */
UCLASS(BlueprintType, Category = "GSD|Events")
class GSD_DAILYEVENTS_API UGSDEventConstructionConfig : public UGSDDailyEventConfig
{
    GENERATED_BODY()

public:
    UGSDEventConstructionConfig();

    //-- Construction Settings --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Construction")
    TArray<TSubclassOf<AActor>> BarricadeClasses;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Construction")
    float BarricadeSpacing = 300.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Construction")
    int32 MinBarricades = 3;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Construction")
    int32 MaxBarricades = 8;

    //-- Warning Settings --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Construction")
    TArray<TSubclassOf<AActor>> WarningSignClasses;

    //-- Navigation Blocking --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Construction")
    TObjectPtr<UGSDNavigationBlockModifier> NavigationBlocker;

    //-- UGSDDailyEventConfig Interface --
    virtual bool ValidateConfig(FString& OutError) const override;
    virtual void OnEventStart_Implementation(UObject* WorldContext, FVector Location, float Intensity) override;
    virtual void OnEventEnd_Implementation(UObject* WorldContext) override;

protected:
    /** Track spawned barricades for cleanup */
    UPROPERTY()
    TArray<TObjectPtr<AActor>> SpawnedBarricades;

    /** Track spawned warning signs for cleanup */
    UPROPERTY()
    TArray<TObjectPtr<AActor>> SpawnedWarnings;

    /** Event center location for navigation blocking */
    FVector EventCenter;

    /** Helper: Spawn barricade line at location */
    void SpawnBarricadeLine(UWorld* World, const FVector& Center, int32 Count);
};
