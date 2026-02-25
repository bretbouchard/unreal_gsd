#include "Interfaces/IGSDComponentSpawnable.h"

UActorComponent* IGSDComponentSpawnable::SpawnComponent_Implementation(AActor* Owner, const FTransform& Transform)
{
    return nullptr;
}

FName IGSDComponentSpawnable::GetComponentType_Implementation() const
{
    return NAME_None;
}

bool IGSDComponentSpawnable::IsComponentActive_Implementation() const
{
    return false;
}

void IGSDComponentSpawnable::DeactivateComponent_Implementation()
{
    // Default implementation - override in C++ or Blueprint
}

void IGSDComponentSpawnable::DestroyComponent_Implementation()
{
    // Default implementation - override in C++ or Blueprint
}
