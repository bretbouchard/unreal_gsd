// Copyright Bret Bouchard. All Rights Reserved.

#include "DataAssets/Events/GSDEventBonfireConfig.h"
#include "Modifiers/GSDDensityReduceModifier.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "GSDEventLog.h"

UGSDEventBonfireConfig::UGSDEventBonfireConfig()
{
    EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Daily.Bonfire"));
    DurationMinutes = 30.0f;  // Bonfires last 30 minutes by default
}

bool UGSDEventBonfireConfig::ValidateConfig(FString& OutError) const
{
    if (!Super::ValidateConfig(OutError))
    {
        return false;
    }

    // Specific validation for Bonfire event
    if (!BonfireFX)
    {
        OutError = TEXT("Bonfire event requires BonfireFX (NiagaraSystem) to be set");
        return false;
    }

    if (!DensityReducer)
    {
        OutError = TEXT("Bonfire event requires DensityReducer to be set");
        return false;
    }

    return true;
}

void UGSDEventBonfireConfig::OnEventStart_Implementation(UObject* WorldContext, FVector Location, float Intensity)
{
    EventCenter = Location;

    // Spawn Niagara FX at location
    if (BonfireFX)
    {
        FVector EffectiveScale = FXScale * FMath::Max(0.1f, Intensity);

        SpawnedFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            WorldContext,
            BonfireFX,
            Location,
            FRotator::ZeroRotator,
            EffectiveScale,
            false,  // bAutoDestroy - we manage manually
            true,   // bAutoActivate
            ENCPoolMethod::None,
            true    // bPreCullCheck
        );

        if (SpawnedFXComponent)
        {
            // Set intensity parameter for FX
            SpawnedFXComponent->SetNiagaraVariableFloat(FString("User.Intensity"), Intensity);
        }
    }

    // Apply density reduction modifier
    if (DensityReducer)
    {
        DensityReducer->ApplyModifier(WorldContext, Location, Intensity);
    }

    GSDEVENT_LOG(Log, TEXT("Bonfire event started at %s with intensity %.2f"), *Location.ToString(), Intensity);
}

void UGSDEventBonfireConfig::OnEventEnd_Implementation(UObject* WorldContext)
{
    // Deactivate and cleanup FX
    if (SpawnedFXComponent)
    {
        SpawnedFXComponent->Deactivate();
        SpawnedFXComponent->DestroyComponent();
        SpawnedFXComponent = nullptr;
    }

    // Remove density modifier (always, even if context invalid)
    if (DensityReducer)
    {
        DensityReducer->RemoveModifier(WorldContext);
    }

    GSDEVENT_LOG(Log, TEXT("Bonfire event ended"));
}
