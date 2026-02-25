#include "Interfaces/IGSDSpawnable.h"

void IGSDSpawnable::SpawnAsync_Implementation(UGSDDataAsset* Config, FOnSpawnComplete Callback)
{
    // Default implementation - override in C++ or Blueprint
}

UGSDDataAsset* IGSDSpawnable::GetSpawnConfig_Implementation()
{
    return nullptr;
}

bool IGSDSpawnable::IsSpawned_Implementation()
{
    return false;
}

void IGSDSpawnable::Despawn_Implementation()
{
    // Default implementation - override in C++ or Blueprint
}

void IGSDSpawnable::ResetSpawnState_Implementation()
{
    // Default implementation - override in C++ or Blueprint
}
