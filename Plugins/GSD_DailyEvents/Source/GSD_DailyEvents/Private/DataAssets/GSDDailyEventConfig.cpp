// Copyright Bret Bouchard. All Rights Reserved.

#include "DataAssets/GSDDailyEventConfig.h"
#include "DataAssets/GSDEventModifierConfig.h"
#include "GSDEventLog.h"

bool UGSDDailyEventConfig::ValidateConfig(FString& OutError) const
{
    // Call parent validation first
    if (!Super::ValidateConfig(OutError))
    {
        return false;
    }

    // Check EventTag is valid
    if (!EventTag.IsValid())
    {
        OutError = TEXT("EventTag is not set or invalid");
        return false;
    }

    // Check DurationMinutes is positive
    if (DurationMinutes <= 0.0f)
    {
        OutError = FString::Printf(TEXT("DurationMinutes must be positive, got %.2f"), DurationMinutes);
        return false;
    }

    // Check DisplayName is not empty
    if (DisplayName.IsEmpty())
    {
        OutError = TEXT("DisplayName is empty");
        return false;
    }

    return true;
}

void UGSDDailyEventConfig::OnEventStart_Implementation(UObject* WorldContext, FVector Location, float Intensity)
{
    GSDEVENT_LOG(Log, TEXT("Event %s starting at %s (intensity=%.2f)"),
        *EventTag.ToString(), *Location.ToString(), Intensity);

    // Store for cleanup
    LastAppliedLocation = Location;
    LastAppliedIntensity = Intensity;
    AppliedModifiers.Empty();

    // Load and apply all modifiers
    for (const TSoftObjectPtr<UGSDEventModifierConfig>& ModifierPtr : Modifiers)
    {
        if (UGSDEventModifierConfig* Modifier = ModifierPtr.LoadSynchronous())
        {
            FString ValidateError;
            if (!Modifier->ValidateConfig(ValidateError))
            {
                GSDEVENT_WARN(TEXT("Modifier %s failed validation: %s"), *Modifier->GetName(), *ValidateError);
                continue;
            }

            Modifier->ApplyModifier(WorldContext, Location, Intensity);
            AppliedModifiers.Add(Modifier);

            GSDEVENT_LOG(Log, TEXT("Applied modifier: %s"), *Modifier->GetName());
        }
        else
        {
            GSDEVENT_WARN(TEXT("Failed to load modifier: %s"), *ModifierPtr.ToString());
        }
    }
}

void UGSDDailyEventConfig::OnEventEnd_Implementation(UObject* WorldContext)
{
    GSDEVENT_LOG(Log, TEXT("Event %s ending"), *EventTag.ToString());

    // Remove all applied modifiers in reverse order
    for (int32 i = AppliedModifiers.Num() - 1; i >= 0; --i)
    {
        if (UGSDEventModifierConfig* Modifier = AppliedModifiers[i])
        {
            Modifier->RemoveModifier(WorldContext);
            GSDEVENT_LOG(Log, TEXT("Removed modifier: %s"), *Modifier->GetName());
        }
    }

    AppliedModifiers.Empty();
}
