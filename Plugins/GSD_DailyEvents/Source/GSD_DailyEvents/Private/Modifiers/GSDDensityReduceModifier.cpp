// Copyright Bret Bouchard. All Rights Reserved.

#include "Modifiers/GSDDensityReduceModifier.h"
#include "Subsystems/GSDCrowdManagerSubsystem.h"
#include "GSDEventLog.h"

UGSDDensityReduceModifier::UGSDDensityReduceModifier()
{
    ModifierTag = FGameplayTag::RequestGameplayTag(FName("Event.Modifier.DensityReduce"));
}

void UGSDDensityReduceModifier::ApplyModifier_Implementation(UObject* WorldContext, FVector Center, float IntensityMultiplier)
{
    if (!WorldContext)
    {
        GSDEVENT_LOG(Warning, TEXT("DensityReduceModifier: Null WorldContext"));
        return;
    }

    UWorld* World = WorldContext->GetWorld();
    if (!World)
    {
        GSDEVENT_LOG(Warning, TEXT("DensityReduceModifier: Null World"));
        return;
    }

    UGSDCrowdManagerSubsystem* CrowdManager = World->GetSubsystem<UGSDCrowdManagerSubsystem>();
    if (!CrowdManager)
    {
        GSDEVENT_LOG(Warning, TEXT("DensityReduceModifier: CrowdManager not found"));
        return;
    }

    // Calculate effective multiplier (reduced by intensity)
    float EffectiveMultiplier = DensityMultiplier * (1.0f / FMath::Max(0.1f, IntensityMultiplier));
    EffectiveMultiplier = FMath::Clamp(EffectiveMultiplier, 0.01f, 1.0f);

    // Apply density reduction
    CrowdManager->AddDensityModifier(ModifierTag, Center, Radius, EffectiveMultiplier);

    GSDEVENT_LOG(Log, TEXT("DensityReduceModifier applied: Center=%s Radius=%.1f Multiplier=%.2f"),
        *Center.ToString(), Radius, EffectiveMultiplier);
}

void UGSDDensityReduceModifier::RemoveModifier_Implementation(UObject* WorldContext)
{
    if (!WorldContext)
    {
        // Still try to remove - modifier might persist
        GSDEVENT_LOG(Log, TEXT("DensityReduceModifier: Removing with null context"));
    }

    UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
    if (!World)
    {
        GSDEVENT_LOG(Warning, TEXT("DensityReduceModifier: Cannot remove - null world"));
        return;
    }

    UGSDCrowdManagerSubsystem* CrowdManager = World->GetSubsystem<UGSDCrowdManagerSubsystem>();
    if (!CrowdManager)
    {
        GSDEVENT_LOG(Warning, TEXT("DensityReduceModifier: CrowdManager not found for removal"));
        return;
    }

    CrowdManager->RemoveDensityModifier(ModifierTag);

    GSDEVENT_LOG(Log, TEXT("DensityReduceModifier removed"));
}
