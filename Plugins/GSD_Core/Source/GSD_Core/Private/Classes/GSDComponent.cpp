#include "Classes/GSDComponent.h"
#include "DataAssets/GSDDataAsset.h"
#include "GSDLog.h"

UGSDComponent::UGSDComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

UActorComponent* UGSDComponent::SpawnComponent_Implementation(AActor* Owner, const FTransform& Transform)
{
    if (!Owner)
    {
        GSD_WARN("UGSDComponent::SpawnComponent called with null Owner");
        return nullptr;
    }

    // Components are created via NewObject and registered
    // This is a default implementation - override for specific behavior
    bGSDActive = true;
    OnGSDActivate();

    GSD_LOG(Log, "UGSDComponent %s spawned on actor %s", *GetName(), *Owner->GetName());
    return this;
}

FName UGSDComponent::GetComponentType_Implementation() const
{
    return GetClass()->GetFName();
}

bool UGSDComponent::IsComponentActive_Implementation() const
{
    return bGSDActive && IsActive();
}

void UGSDComponent::DeactivateComponent_Implementation()
{
    OnGSDDeactivate();
    bGSDActive = false;
    GSD_LOG(Log, "UGSDComponent %s deactivated", *GetName());
}

void UGSDComponent::DestroyComponent_Implementation()
{
    DeactivateComponent_Implementation();
    DestroyComponent();
}

void UGSDComponent::OnConfigApplied_Implementation(UGSDDataAsset* Config)
{
    // Override in derived classes
}

void UGSDComponent::OnGSDActivate_Implementation()
{
    // Override in derived classes
}

void UGSDComponent::OnGSDDeactivate_Implementation()
{
    // Override in derived classes
}
