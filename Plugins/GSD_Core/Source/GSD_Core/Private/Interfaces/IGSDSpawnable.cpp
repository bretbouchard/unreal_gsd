#include "Interfaces/IGSDSpawnable.h"

void IGSDSpawnable::SpawnFromConfig(UGSDDataAsset* Config)
{
    // Default implementation - override in C++
}

UGSDDataAsset* IGSDSpawnable::GetSpawnConfig()
{
    return nullptr;
}

bool IGSDSpawnable::IsSpawned()
{
    return false;
}

void IGSDSpawnable::Despawn()
{
    // Default implementation - override in C++
}

void IGSDSpawnable::ResetSpawnState()
{
    // Default implementation - override in C++
}
