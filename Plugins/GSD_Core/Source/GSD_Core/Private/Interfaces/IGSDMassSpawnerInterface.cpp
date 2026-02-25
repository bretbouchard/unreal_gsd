#include "Interfaces/IGSDMassSpawnerInterface.h"

void IGSDMassSpawnerInterface::SpawnBatch_Implementation(
    const TArray<FTransform>& Transforms,
    UGSDDataAsset* Config,
    FOnMassSpawnComplete& OnComplete)
{
    // Default implementation - override in C++ or Blueprint
}

void IGSDMassSpawnerInterface::SpawnBatchChunked_Implementation(
    const TArray<FTransform>& Transforms,
    UGSDDataAsset* Config,
    int32 ChunkSize,
    FOnMassSpawnChunkComplete& OnChunkComplete)
{
    // Default implementation - override in C++ or Blueprint
}

int32 IGSDMassSpawnerInterface::GetPendingSpawnCount_Implementation() const
{
    return 0;
}

void IGSDMassSpawnerInterface::CancelPendingSpawns_Implementation()
{
    // Default implementation - override in C++ or Blueprint
}

bool IGSDMassSpawnerInterface::IsSpawning_Implementation() const
{
    return false;
}
