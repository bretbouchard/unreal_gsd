#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Types/GSDSpawnTypes.h"
#include "IGSDMassSpawnerInterface.generated.h"

class UGSDDataAsset;

UINTERFACE(MinimalAPI, NotBlueprintable, Category = "GSD")
class UGSDMassSpawnerInterface : public UInterface
{
    GENERATED_BODY()
};

class GSD_CORE_API IGSDMassSpawnerInterface
{
    GENERATED_BODY()

public:
    // Batch spawn - returns spawned actors
    UFUNCTION(BlueprintCallable, Category = "GSD|Mass")
    virtual TArray<AActor*> SpawnBatch(
        const TArray<FTransform>& Transforms,
        UGSDDataAsset* Config
    );

    // Chunked spawning - returns spawned actors for chunk
    UFUNCTION(BlueprintCallable, Category = "GSD|Mass")
    virtual TArray<AActor*> SpawnBatchChunked(
        const TArray<FTransform>& Transforms,
        UGSDDataAsset* Config,
        int32 ChunkSize,
        int32 ChunkIndex
    );

    // Get current spawn queue status
    UFUNCTION(BlueprintCallable, Category = "GSD|Mass")
    virtual int32 GetPendingSpawnCount() const;

    // Cancel pending spawns
    UFUNCTION(BlueprintCallable, Category = "GSD|Mass")
    virtual void CancelPendingSpawns();

    // Check if spawning is in progress
    UFUNCTION(BlueprintCallable, Category = "GSD|Mass")
    virtual bool IsSpawning() const;
};
