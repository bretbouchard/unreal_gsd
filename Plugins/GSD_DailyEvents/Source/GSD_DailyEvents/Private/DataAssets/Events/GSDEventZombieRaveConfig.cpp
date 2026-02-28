// Copyright Bret Bouchard. All Rights Reserved.

#include "DataAssets/Events/GSDEventZombieRaveConfig.h"
#include "DataAssets/GSDDensityBoostModifier.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "GSDEventLog.h"

UGSDEventZombieRaveConfig::UGSDEventZombieRaveConfig()
{
    EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Daily.ZombieRave"));
    DurationMinutes = 45.0f;  // Zombie Raves last 45 minutes by default
}

bool UGSDEventZombieRaveConfig::ValidateConfig(FString& OutError) const
{
    if (!Super::ValidateConfig(OutError))
    {
        return false;
    }

    // Specific validation for Zombie Rave event
    if (!RaveMusic)
    {
        OutError = TEXT("Zombie Rave event requires RaveMusic (SoundBase) to be set");
        return false;
    }

    if (!RaveFX)
    {
        OutError = TEXT("Zombie Rave event requires RaveFX (NiagaraSystem) to be set");
        return false;
    }

    if (!DensityBooster)
    {
        OutError = TEXT("Zombie Rave event requires DensityBooster to be set");
        return false;
    }

    return true;
}

void UGSDEventZombieRaveConfig::OnEventStart_Implementation(UObject* WorldContext, FVector Location, float Intensity)
{
    UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
    if (!World)
    {
        GSDEVENT_LOG(Warning, TEXT("Zombie Rave event: Invalid world context"));
        return;
    }

    EventCenter = Location;

    // Spawn spatial audio - use SpawnSoundAtLocation for control
    if (RaveMusic)
    {
        SpawnedAudio = UGameplayStatics::SpawnSoundAtLocation(
            World,
            RaveMusic,
            Location,
            FRotator::ZeroRotator,
            VolumeMultiplier * Intensity,
            1.0f,   // Pitch
            0.0f,   // StartTime
            nullptr, // AttenuationSettings (use defaults)
            nullptr, // ConcurrencySettings
            false,   // bAutoDestroy - we manage manually
            true     // bPersistAcrossLevelTransition
        );

        if (SpawnedAudio)
        {
            // Set radius parameter for audio attenuation
            SpawnedAudio->SetFloatParameter(FName("Radius"), AudioRadius * Intensity);
        }
    }

    // Spawn VFX
    if (RaveFX)
    {
        SpawnedFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            WorldContext,
            RaveFX,
            Location,
            FRotator::ZeroRotator,
            FVector(Intensity),
            false,  // bAutoDestroy - we manage manually
            true,   // bAutoActivate
            ENCPoolMethod::None,
            true    // bPreCullCheck
        );

        if (SpawnedFX)
        {
            // Set intensity parameter
            SpawnedFX->SetNiagaraVariableFloat(FString("User.Intensity"), Intensity);
        }
    }

    // Apply density boost - attracts zombies
    if (DensityBooster)
    {
        DensityBooster->ApplyModifier(WorldContext, Location, Intensity);
    }

    GSDEVENT_LOG(Log, TEXT("Zombie Rave event started at %s with intensity %.2f"), *Location.ToString(), Intensity);
}

void UGSDEventZombieRaveConfig::OnEventEnd_Implementation(UObject* WorldContext)
{
    // Stop and cleanup audio
    if (SpawnedAudio)
    {
        SpawnedAudio->Stop();
        SpawnedAudio->DestroyComponent();
        SpawnedAudio = nullptr;
    }

    // Deactivate FX
    if (SpawnedFX)
    {
        SpawnedFX->Deactivate();
        SpawnedFX->DestroyComponent();
        SpawnedFX = nullptr;
    }

    // Remove density boost
    if (DensityBooster)
    {
        DensityBooster->RemoveModifier(WorldContext);
    }

    GSDEVENT_LOG(Log, TEXT("Zombie Rave event ended"));
}
