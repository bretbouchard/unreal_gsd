// Copyright Bret Bouchard. All Rights Reserved.

#include "Modifiers/GSDNavigationBlockModifier.h"
#include "Navigation/NavModifierVolume.h"
#include "GSDEventLog.h"

UGSDNavigationBlockModifier::UGSDNavigationBlockModifier()
{
    ModifierTag = FGameplayTag::RequestGameplayTag(FName("Event.Modifier.NavigationBlock"));
}

void UGSDNavigationBlockModifier::ApplyModifier_Implementation(UObject* WorldContext, FVector Center, float IntensityMultiplier)
{
    if (!WorldContext) return;

    UWorld* World = WorldContext->GetWorld();
    if (!World) return;

    // Calculate effective extent based on intensity
    FVector EffectiveExtent = VolumeExtent * IntensityMultiplier;

    // Spawn NavModifierVolume to block navigation
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ANavModifierVolume* NavVolume = World->SpawnActor<ANavModifierVolume>(Center, FRotator::ZeroRotator, SpawnParams);
    if (NavVolume)
    {
        NavVolume->SetActorScale3D(EffectiveExtent / FVector(100.0f)); // Default volume is 100 units
        SpawnedVolumes.Add(NavVolume);

        GSDEVENT_LOG(Log, TEXT("NavigationBlockModifier applied: Center=%s Extent=%s"),
            *Center.ToString(), *EffectiveExtent.ToString());
    }
}

void UGSDNavigationBlockModifier::RemoveModifier_Implementation(UObject* WorldContext)
{
    // Count before clearing for logging
    int32 NumDestroyed = SpawnedVolumes.Num();

    // Destroy all spawned volumes
    for (ANavModifierVolume* Volume : SpawnedVolumes)
    {
        if (Volume)
        {
            Volume->Destroy();
        }
    }
    SpawnedVolumes.Empty();

    GSDEVENT_LOG(Log, TEXT("NavigationBlockModifier removed: %d volumes destroyed"), NumDestroyed);
}
