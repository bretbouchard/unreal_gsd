#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Types/GSDSpawnTypes.h"
#include "IGSDSpawnable.generated.h"

class UGSDDataAsset;

UINTERFACE(MinimalAPI, NotBlueprintable, Category = "GSD")
class UGSDSpawnable : public UInterface
{
    GENERATED_BODY()
};

class GSD_CORE_API IGSDSpawnable
{
    GENERATED_BODY()

public:
    // Initialize spawnable from config (synchronous - rename from SpawnAsync for clarity)
    UFUNCTION(BlueprintCallable, Category = "GSD|Spawning")
    virtual void SpawnFromConfig(UGSDDataAsset* Config);

    UFUNCTION(BlueprintCallable, Category = "GSD|Spawning")
    virtual UGSDDataAsset* GetSpawnConfig();

    UFUNCTION(BlueprintCallable, Category = "GSD|Spawning")
    virtual bool IsSpawned();

    UFUNCTION(BlueprintCallable, Category = "GSD|Spawning")
    virtual void Despawn();

    UFUNCTION(BlueprintCallable, Category = "GSD|Spawning")
    virtual void ResetSpawnState();
};
