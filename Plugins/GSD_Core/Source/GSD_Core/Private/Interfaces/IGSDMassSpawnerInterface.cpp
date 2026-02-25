#include "Interfaces/IGSDMassSpawnerInterface.h"

TArray<AActor*> IGSDMassSpawnerInterface::SpawnBatch(
    const TArray<FTransform>& Transforms,
    UGSDDataAsset* Config)
{
    // Default implementation - override in C++
    return TArray<AActor*>();
}

TArray<AActor*> IGSDMassSpawnerInterface::SpawnBatchChunked(
    const TArray<FTransform>& Transforms,
    UGSDDataAsset* Config,
    int32 ChunkSize,
    int32 ChunkIndex)
{
    // Default implementation - override in C++
    return TArray<AActor*>();
}

int32 IGSDMassSpawnerInterface::GetPendingSpawnCount() const
{
    return 0;
}

void IGSDMassSpawnerInterface::CancelPendingSpawns()
{
    // Default implementation - override in C++
}

bool IGSDMassSpawnerInterface::IsSpawning() const
{
    return false;
}
