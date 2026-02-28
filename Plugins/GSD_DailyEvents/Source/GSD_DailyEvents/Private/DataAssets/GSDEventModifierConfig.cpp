// Copyright Bret Bouchard. All Rights Reserved.

#include "DataAssets/GSDEventModifierConfig.h"
#include "GSDEventLog.h"

bool UGSDEventModifierConfig::ValidateConfig(FString& OutError) const
{
    // Call parent validation first
    if (!Super::ValidateConfig(OutError))
    {
        return false;
    }

    // Check ModifierTag is valid
    if (!ModifierTag.IsValid())
    {
        OutError = TEXT("ModifierTag is not set or invalid");
        return false;
    }

    // Check Radius is positive
    if (Radius <= 0.0f)
    {
        OutError = FString::Printf(TEXT("Radius must be positive, got %.2f"), Radius);
        return false;
    }

    // Check Intensity is positive
    if (Intensity <= 0.0f)
    {
        OutError = FString::Printf(TEXT("Intensity must be positive, got %.2f"), Intensity);
        return false;
    }

    return true;
}

void UGSDEventModifierConfig::ApplyModifier_Implementation(UObject* WorldContext, FVector Center, float IntensityMultiplier)
{
    // Store for cleanup
    LastAppliedCenter = Center;
    LastIntensityMultiplier = IntensityMultiplier;

    // Base class just logs - override in subclasses
    GSDEVENT_LOG(Warning, TEXT("Base class ApplyModifier called for %s - override in subclass"),
        *ModifierTag.ToString());
}

void UGSDEventModifierConfig::RemoveModifier_Implementation(UObject* WorldContext)
{
    // Base class just logs - override in subclasses
    GSDEVENT_LOG(Warning, TEXT("Base class RemoveModifier called for %s - override in subclass"),
        *ModifierTag.ToString());
}
