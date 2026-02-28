// Copyright Bret Bouchard. All Rights Reserved.

#include "DataAssets/Events/GSDEventBlockPartyConfig.h"
#include "Modifiers/GSDSafeZoneModifier.h"
#include "Kismet/KismetMathLibrary.h"
#include "GSDEventLog.h"

UGSDEventBlockPartyConfig::UGSDEventBlockPartyConfig()
{
    EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Daily.BlockParty"));
    DurationMinutes = 45.0f;  // Block parties last 45 minutes by default
}

bool UGSDEventBlockPartyConfig::ValidateConfig(FString& OutError) const
{
    if (!Super::ValidateConfig(OutError))
    {
        return false;
    }

    // Specific validation for Block Party event
    if (CrowdPropClasses.Num() == 0)
    {
        OutError = TEXT("Block Party event requires at least one CrowdPropClass");
        return false;
    }

    if (!SafeZoneModifier)
    {
        OutError = TEXT("Block Party event requires SafeZoneModifier to be set");
        return false;
    }

    // Validate each prop class is valid
    for (int32 i = 0; i < CrowdPropClasses.Num(); ++i)
    {
        if (!CrowdPropClasses[i])
        {
            OutError = FString::Printf(TEXT("CrowdPropClasses[%d] is null"), i);
            return false;
        }
    }

    // Validate decorative FX if specified
    for (int32 i = 0; i < DecorativeFXClasses.Num(); ++i)
    {
        if (!DecorativeFXClasses[i])
        {
            OutError = FString::Printf(TEXT("DecorativeFXClasses[%d] is null"), i);
            return false;
        }
    }

    return true;
}

void UGSDEventBlockPartyConfig::OnEventStart_Implementation(UObject* WorldContext, FVector Location, float Intensity)
{
    UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
    if (!World)
    {
        GSDEVENT_LOG(Warning, TEXT("Block Party event: Invalid world"));
        return;
    }

    EventCenter = Location;

    // Apply safe zone modifier
    if (SafeZoneModifier)
    {
        SafeZoneModifier->ApplyModifier(WorldContext, Location, Intensity);
    }

    // Calculate number of props based on intensity
    int32 NumProps = FMath::RoundToInt(FMath::Lerp(float(MinProps), float(MaxProps), Intensity));
    NumProps = FMath::Clamp(NumProps, MinProps, MaxProps);

    // Spawn crowd props
    SpawnCrowdProps(World, Location, NumProps);

    // Spawn decorative FX (string lights, speakers, etc.)
    if (DecorativeFXClasses.Num() > 0)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        int32 NumFX = FMath::CeilToInt(Intensity * 3.0f);
        for (int32 i = 0; i < NumFX; ++i)
        {
            FVector RandomOffset = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(200.0f, PropSpawnRadius * 0.8f);
            FVector SpawnLoc = Location + RandomOffset;
            FRotator RandomRotation = UKismetMathLibrary::RandomRotator();

            TSubclassOf<AActor> FXClass = DecorativeFXClasses[FMath::RandRange(0, DecorativeFXClasses.Num() - 1)];
            AActor* FX = World->SpawnActor<AActor>(FXClass, SpawnLoc, RandomRotation, SpawnParams);
            if (FX)
            {
                SpawnedFX.Add(FX);
            }
        }
    }

    GSDEVENT_LOG(Log, TEXT("Block Party event started at %s: %d props, %d FX, intensity %.2f"),
        *Location.ToString(), SpawnedProps.Num(), SpawnedFX.Num(), Intensity);
}

void UGSDEventBlockPartyConfig::SpawnCrowdProps(UWorld* World, const FVector& Center, int32 Count)
{
    if (!World || CrowdPropClasses.Num() == 0) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    for (int32 i = 0; i < Count; ++i)
    {
        // Random position within spawn radius
        FVector RandomOffset = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(100.0f, PropSpawnRadius);
        FVector SpawnLocation = Center + RandomOffset;

        // Random rotation (mostly upright, some variation)
        FRotator SpawnRotation = FRotator(
            FMath::FRandRange(-5.0f, 5.0f),   // Slight pitch variation
            FMath::FRandRange(0.0f, 360.0f),  // Random yaw
            FMath::FRandRange(-5.0f, 5.0f)    // Slight roll variation
        );

        // Select random prop class
        TSubclassOf<AActor> PropClass = CrowdPropClasses[FMath::RandRange(0, CrowdPropClasses.Num() - 1)];

        AActor* Prop = World->SpawnActor<AActor>(PropClass, SpawnLocation, SpawnRotation, SpawnParams);
        if (Prop)
        {
            SpawnedProps.Add(Prop);
        }
    }
}

void UGSDEventBlockPartyConfig::OnEventEnd_Implementation(UObject* WorldContext)
{
    // Count before clearing for logging
    int32 NumProps = SpawnedProps.Num();
    int32 NumFX = SpawnedFX.Num();

    // Destroy all spawned props
    for (AActor* Prop : SpawnedProps)
    {
        if (Prop)
        {
            Prop->Destroy();
        }
    }
    SpawnedProps.Empty();

    // Destroy all spawned FX
    for (AActor* FX : SpawnedFX)
    {
        if (FX)
        {
            FX->Destroy();
        }
    }
    SpawnedFX.Empty();

    // Remove safe zone modifier
    if (SafeZoneModifier)
    {
        SafeZoneModifier->RemoveModifier(WorldContext);
    }

    GSDEVENT_LOG(Log, TEXT("Block Party event ended: %d props, %d FX destroyed"),
        NumProps, NumFX);
}
