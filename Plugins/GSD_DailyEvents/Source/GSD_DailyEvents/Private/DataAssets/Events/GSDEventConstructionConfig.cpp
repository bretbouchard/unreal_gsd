// Copyright Bret Bouchard. All Rights Reserved.

#include "DataAssets/Events/GSDEventConstructionConfig.h"
#include "Modifiers/GSDNavigationBlockModifier.h"
#include "GSDEventLog.h"

UGSDEventConstructionConfig::UGSDEventConstructionConfig()
{
    EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Daily.Construction"));
    DurationMinutes = 60.0f;  // Construction lasts 1 hour by default
}

bool UGSDEventConstructionConfig::ValidateConfig(FString& OutError) const
{
    if (!Super::ValidateConfig(OutError))
    {
        return false;
    }

    // Specific validation for Construction event
    if (BarricadeClasses.Num() == 0)
    {
        OutError = TEXT("Construction event requires at least one BarricadeClass");
        return false;
    }

    if (!NavigationBlocker)
    {
        OutError = TEXT("Construction event requires NavigationBlocker to be set");
        return false;
    }

    // Validate each barricade class is valid
    for (int32 i = 0; i < BarricadeClasses.Num(); ++i)
    {
        if (!BarricadeClasses[i])
        {
            OutError = FString::Printf(TEXT("BarricadeClasses[%d] is null"), i);
            return false;
        }
    }

    // Validate warning signs if specified
    for (int32 i = 0; i < WarningSignClasses.Num(); ++i)
    {
        if (!WarningSignClasses[i])
        {
            OutError = FString::Printf(TEXT("WarningSignClasses[%d] is null"), i);
            return false;
        }
    }

    return true;
}

void UGSDEventConstructionConfig::OnEventStart_Implementation(UObject* WorldContext, FVector Location, float Intensity)
{
    UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
    if (!World || BarricadeClasses.Num() == 0)
    {
        GSDEVENT_LOG(Warning, TEXT("Construction event: Invalid world or no barricade classes"));
        return;
    }

    EventCenter = Location;

    // Calculate number of barricades based on intensity
    int32 NumBarricades = FMath::RoundToInt(FMath::Lerp(float(MinBarricades), float(MaxBarricades), Intensity));
    NumBarricades = FMath::Clamp(NumBarricades, MinBarricades, MaxBarricades);

    // Spawn barricades
    SpawnBarricadeLine(World, Location, NumBarricades);

    // Spawn warning signs at each end
    if (WarningSignClasses.Num() > 0)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        FVector StartWarningLoc = Location + FVector(-BarricadeSpacing * (NumBarricades / 2 + 1), 0.0f, 0.0f);
        FVector EndWarningLoc = Location + FVector(BarricadeSpacing * (NumBarricades / 2 + 1), 0.0f, 0.0f);

        TSubclassOf<AActor> WarningClass = WarningSignClasses[0];
        AActor* StartWarning = World->SpawnActor<AActor>(WarningClass, StartWarningLoc, FRotator::ZeroRotator, SpawnParams);
        AActor* EndWarning = World->SpawnActor<AActor>(WarningClass, EndWarningLoc, FRotator(0.0f, 180.0f, 0.0f), SpawnParams);

        if (StartWarning) SpawnedWarnings.Add(StartWarning);
        if (EndWarning) SpawnedWarnings.Add(EndWarning);
    }

    // Apply navigation blocker
    if (NavigationBlocker)
    {
        NavigationBlocker->ApplyModifier(WorldContext, Location, Intensity);
    }

    GSDEVENT_LOG(Log, TEXT("Construction event started: %d barricades, %d warnings at %s"),
        SpawnedBarricades.Num(), SpawnedWarnings.Num(), *Location.ToString());
}

void UGSDEventConstructionConfig::SpawnBarricadeLine(UWorld* World, const FVector& Center, int32 Count)
{
    if (!World || BarricadeClasses.Num() == 0) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    float StartOffset = -BarricadeSpacing * (Count - 1) / 2.0f;

    for (int32 i = 0; i < Count; ++i)
    {
        FVector SpawnLocation = Center + FVector(StartOffset + i * BarricadeSpacing, 0.0f, 0.0f);
        TSubclassOf<AActor> BarricadeClass = BarricadeClasses[i % BarricadeClasses.Num()];

        AActor* Barricade = World->SpawnActor<AActor>(BarricadeClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
        if (Barricade)
        {
            SpawnedBarricades.Add(Barricade);
        }
    }
}

void UGSDEventConstructionConfig::OnEventEnd_Implementation(UObject* WorldContext)
{
    // Count before clearing for logging
    int32 NumBarricades = SpawnedBarricades.Num();
    int32 NumWarnings = SpawnedWarnings.Num();

    // Destroy all spawned barricades
    for (AActor* Barricade : SpawnedBarricades)
    {
        if (Barricade)
        {
            Barricade->Destroy();
        }
    }
    SpawnedBarricades.Empty();

    // Destroy all warning signs
    for (AActor* Warning : SpawnedWarnings)
    {
        if (Warning)
        {
            Warning->Destroy();
        }
    }
    SpawnedWarnings.Empty();

    // Remove navigation blocker
    if (NavigationBlocker)
    {
        NavigationBlocker->RemoveModifier(WorldContext);
    }

    GSDEVENT_LOG(Log, TEXT("Construction event ended: %d barricades, %d warnings destroyed"),
        NumBarricades, NumWarnings);
}
