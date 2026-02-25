#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Types/GSDSpawnTypes.h"
#include "IGSDSpawnable.generated.h"

class UGSDDataAsset;

UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
class UGSDSpawnable : public UInterface
{
    GENERATED_BODY()
};

class GSD_CORE_API IGSDSpawnable
{
    GENERATED_BODY()

public:
    // Async spawn with config and callback
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Spawning")
    void SpawnAsync(UGSDDataAsset* Config, FOnSpawnComplete Callback);
    virtual void SpawnAsync_Implementation(UGSDDataAsset* Config, FOnSpawnComplete Callback);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Spawning")
    UGSDDataAsset* GetSpawnConfig();
    virtual UGSDDataAsset* GetSpawnConfig_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Spawning")
    bool IsSpawned();
    virtual bool IsSpawned_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Spawning")
    void Despawn();
    virtual void Despawn_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Spawning")
    void ResetSpawnState();
    virtual void ResetSpawnState_Implementation();
};
