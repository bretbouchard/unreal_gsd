// Copyright Bret Bouchard. All Rights Reserved.

#include "Modifiers/GSDSafeZoneModifier.h"
#include "Subsystems/GSDCrowdManagerSubsystem.h"
#include "GSDEventLog.h"

UGSDSafeZoneModifier::UGSDSafeZoneModifier()
{
    ModifierTag = FGameplayTag::RequestGameplayTag(FName("Event.Modifier.SafeZone"));
}

void UGSDSafeZoneModifier::ApplyModifier_Implementation(UObject* WorldContext, FVector Center, float IntensityMultiplier)
{
    if (!WorldContext) return;

    UWorld* World = WorldContext->GetWorld();
    if (!World) return;

    UGSDCrowdManagerSubsystem* CrowdManager = World->GetSubsystem<UGSDCrowdManagerSubsystem>();
    if (!CrowdManager) return;

    // Safe zone uses near-zero multiplier regardless of intensity
    // Intensity only affects radius (handled by base class)
    CrowdManager->AddDensityModifier(ModifierTag, Center, Radius * IntensityMultiplier, DensityMultiplier);

    GSDEVENT_LOG(Log, TEXT("SafeZoneModifier applied: Center=%s Radius=%.1f"),
        *Center.ToString(), Radius * IntensityMultiplier);
}

void UGSDSafeZoneModifier::RemoveModifier_Implementation(UObject* WorldContext)
{
    UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
    if (!World) return;

    UGSDCrowdManagerSubsystem* CrowdManager = World->GetSubsystem<UGSDCrowdManagerSubsystem>();
    if (!CrowdManager) return;

    CrowdManager->RemoveDensityModifier(ModifierTag);

    GSDEVENT_LOG(Log, TEXT("SafeZoneModifier removed"));
}
