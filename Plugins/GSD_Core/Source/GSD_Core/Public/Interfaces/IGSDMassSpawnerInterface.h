#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Types/GSDSpawnTypes.h"
#include "IGSDMassSpawnerInterface.generated.h"

class UGSDDataAsset;

UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
class UGSDMassSpawnerInterface : public UInterface
{
    GENERATED_BODY()
};

class GSD_CORE_API IGSDMassSpawnerInterface
{
    GENERATED_BODY()

public:
    // Batch spawn - single delegate for entire batch
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Mass")
    void SpawnBatch(
        const TArray<FTransform>& Transforms,
        UGSDDataAsset* Config,
        FOnMassSpawnComplete& OnComplete
    );
    virtual void SpawnBatch_Implementation(
        const TArray<FTransform>& Transforms,
        UGSDDataAsset* Config,
        FOnMassSpawnComplete& OnComplete
    );

    // Chunked spawning to avoid callback storms
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Mass")
    void SpawnBatchChunked(
        const TArray<FTransform>& Transforms,
        UGSDDataAsset* Config,
        int32 ChunkSize,
        FOnMassSpawnChunkComplete& OnChunkComplete
    );
    virtual void SpawnBatchChunked_Implementation(
        const TArray<FTransform>& Transforms,
        UGSDDataAsset* Config,
        int32 ChunkSize,
        FOnMassSpawnChunkComplete& OnChunkComplete
    );

    // Get current spawn queue status
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Mass")
    int32 GetPendingSpawnCount() const;
    virtual int32 GetPendingSpawnCount_Implementation() const;

    // Cancel pending spawns
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Mass")
    void CancelPendingSpawns();
    virtual void CancelPendingSpawns_Implementation();

    // Check if spawning is in progress
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Mass")
    bool IsSpawning() const;
    virtual bool IsSpawning_Implementation() const;
};
