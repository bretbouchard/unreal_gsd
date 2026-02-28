// Copyright Bret Bouchard. All Rights Reserved.

#include "DataAssets/GSDDensityBoostModifier.h"
#include "Subsystems/GSDCrowdManagerSubsystem.h"
#include "Tags/GSDEventTags.h"
#include "GSDEventLog.h"

void UGSDDensityBoostModifier::ApplyModifier_Implementation(UObject* WorldContext, FVector Center, float IntensityMultiplier)
{
    if (!WorldContext)
    {
        GSDEVENT_LOG(Warning, TEXT("UGSDDensityBoostModifier::ApplyModifier - WorldContext is null"));
        return;
    }

    UWorld* World = WorldContext->GetWorld();
    if (!World)
    {
        GSDEVENT_LOG(Warning, TEXT("UGSDDensityBoostModifier::ApplyModifier - World is null"));
        return;
    }

    UGSDCrowdManagerSubsystem* CrowdManager = World->GetSubsystem<UGSDCrowdManagerSubsystem>();
    if (!CrowdManager)
    {
        GSDEVENT_LOG(Warning, TEXT("UGSDDensityBoostModifier::ApplyModifier - GSDCrowdManagerSubsystem not found"));
        return;
    }

    // Calculate final multiplier with intensity
    float FinalMultiplier = DensityMultiplier * IntensityMultiplier;

    // Apply to crowd manager
    CrowdManager->AddDensityModifier(ModifierTag, Center, Radius, FinalMultiplier);

    GSDEVENT_LOG(Log, TEXT("Density boost applied: tag=%s, center=%s, radius=%.0f, multiplier=%.2f"),
        *ModifierTag.ToString(), *Center.ToString(), Radius, FinalMultiplier);
}

void UGSDDensityBoostModifier::RemoveModifier_Implementation(UObject* WorldContext)
{
    if (!WorldContext)
    {
        GSDEVENT_LOG(Warning, TEXT("UGSDDensityBoostModifier::RemoveModifier - WorldContext is null"));
        return;
    }

    UWorld* World = WorldContext->GetWorld();
    if (!World)
    {
        GSDEVENT_LOG(Warning, TEXT("UGSDDensityBoostModifier::RemoveModifier - World is null"));
        return;
    }

    UGSDCrowdManagerSubsystem* CrowdManager = World->GetSubsystem<UGSDCrowdManagerSubsystem>();
    if (!CrowdManager)
    {
        GSDEVENT_LOG(Warning, TEXT("UGSDDensityBoostModifier::RemoveModifier - GSDCrowdManagerSubsystem not found"));
        return;
    }

    // Remove from crowd manager
    CrowdManager->RemoveDensityModifier(ModifierTag);

    GSDEVENT_LOG(Log, TEXT("Density boost removed: tag=%s"), *ModifierTag.ToString());
}
