// Copyright Bret Bouchard. All Rights Reserved.

#include "Blueprint/GSDAccessibilityLibrary.h"
#include "Subsystems/GSDAccessibilityManager.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"

UGSDAccessibilityManager* UGSDAccessibilityLibrary::GetAccessibilityManager(UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        return nullptr;
    }

    UEngine* Engine = GEngine;
    if (!Engine)
    {
        return nullptr;
    }

    return Engine->GetEngineSubsystem<UGSDAccessibilityManager>();
}

FLinearColor UGSDAccessibilityLibrary::CorrectColorForColorBlindness(UObject* WorldContextObject, const FLinearColor& InColor)
{
    UGSDAccessibilityManager* Manager = GetAccessibilityManager(WorldContextObject);
    if (!Manager)
    {
        return InColor;
    }

    return Manager->ApplyColorBlindCorrection(InColor);
}

float UGSDAccessibilityLibrary::GetScaledTextSize(UObject* WorldContextObject, float BaseSize)
{
    UGSDAccessibilityManager* Manager = GetAccessibilityManager(WorldContextObject);
    if (!Manager)
    {
        return BaseSize;
    }

    return Manager->GetScaledTextSize(BaseSize);
}

bool UGSDAccessibilityLibrary::IsHighContrastModeEnabled(UObject* WorldContextObject)
{
    UGSDAccessibilityManager* Manager = GetAccessibilityManager(WorldContextObject);
    if (!Manager)
    {
        return false;
    }

    return Manager->IsHighContrastMode();
}

bool UGSDAccessibilityLibrary::IsReducedMotionEnabled(UObject* WorldContextObject)
{
    UGSDAccessibilityManager* Manager = GetAccessibilityManager(WorldContextObject);
    if (!Manager)
    {
        return false;
    }

    return Manager->IsReducedMotion();
}

bool UGSDAccessibilityLibrary::AreVisualAudioCuesEnabled(UObject* WorldContextObject)
{
    UGSDAccessibilityManager* Manager = GetAccessibilityManager(WorldContextObject);
    if (!Manager)
    {
        return false;
    }

    return Manager->HasVisualAudioCues();
}

UMaterialInstanceDynamic* UGSDAccessibilityLibrary::ApplyAccessibilityToMaterial(UObject* WorldContextObject, UMaterialInterface* BaseMaterial, FName ColorParameterName)
{
    if (!BaseMaterial)
    {
        return nullptr;
    }

    UGSDAccessibilityManager* Manager = GetAccessibilityManager(WorldContextObject);
    if (!Manager)
    {
        // Return basic MID without corrections if manager unavailable
        return UMaterialInstanceDynamic::Create(BaseMaterial, WorldContextObject);
    }

    // Create dynamic material instance
    UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, WorldContextObject);
    if (!MID)
    {
        return nullptr;
    }

    // Apply color blind correction if a color parameter is specified
    if (ColorParameterName != NAME_None)
    {
        FLinearColor BaseColor;
        if (MID->GetVectorParameterValue(ColorParameterName, BaseColor))
        {
            FLinearColor CorrectedColor = Manager->ApplyColorBlindCorrection(BaseColor);
            MID->SetVectorParameterValue(ColorParameterName, CorrectedColor);
        }
    }

    return MID;
}

FLinearColor UGSDAccessibilityLibrary::GetSubtitleBackgroundColor(UObject* WorldContextObject)
{
    UGSDAccessibilityManager* Manager = GetAccessibilityManager(WorldContextObject);
    if (!Manager)
    {
        return FLinearColor(0.0f, 0.0f, 0.0f, 0.75f);
    }

    return Manager->GetSubtitleBackgroundColor();
}

EGSDColorBlindType UGSDAccessibilityLibrary::GetCurrentColorBlindType(UObject* WorldContextObject)
{
    UGSDAccessibilityManager* Manager = GetAccessibilityManager(WorldContextObject);
    if (!Manager)
    {
        return EGSDColorBlindType::None;
    }

    return Manager->GetColorBlindType();
}

float UGSDAccessibilityLibrary::GetScreenShakeIntensity(UObject* WorldContextObject)
{
    UGSDAccessibilityManager* Manager = GetAccessibilityManager(WorldContextObject);
    if (!Manager)
    {
        return 1.0f;
    }

    return Manager->GetScreenShakeIntensity();
}

float UGSDAccessibilityLibrary::GetAutoAimStrength(UObject* WorldContextObject)
{
    UGSDAccessibilityManager* Manager = GetAccessibilityManager(WorldContextObject);
    if (!Manager)
    {
        return 0.0f;
    }

    return Manager->GetAutoAimStrength();
}

float UGSDAccessibilityLibrary::GetTextScale(UObject* WorldContextObject)
{
    UGSDAccessibilityManager* Manager = GetAccessibilityManager(WorldContextObject);
    if (!Manager || !Manager->GetConfig())
    {
        return 1.0f;
    }

    return Manager->GetConfig()->TextSettings.TextScale;
}

bool UGSDAccessibilityLibrary::ShouldAlwaysShowSubtitles(UObject* WorldContextObject)
{
    UGSDAccessibilityManager* Manager = GetAccessibilityManager(WorldContextObject);
    if (!Manager || !Manager->GetConfig())
    {
        return false;
    }

    return Manager->GetConfig()->TextSettings.bAlwaysShowSubtitles;
}

float UGSDAccessibilityLibrary::GetSubtitleTextScale(UObject* WorldContextObject)
{
    UGSDAccessibilityManager* Manager = GetAccessibilityManager(WorldContextObject);
    if (!Manager || !Manager->GetConfig())
    {
        return 1.0f;
    }

    return Manager->GetConfig()->TextSettings.SubtitleTextScale;
}

float UGSDAccessibilityLibrary::GetInputHoldTime(UObject* WorldContextObject)
{
    UGSDAccessibilityManager* Manager = GetAccessibilityManager(WorldContextObject);
    if (!Manager || !Manager->GetConfig())
    {
        return 0.5f;
    }

    return Manager->GetConfig()->InteractionSettings.HoldTime;
}

bool UGSDAccessibilityLibrary::IsSinglePressAlternativeEnabled(UObject* WorldContextObject)
{
    UGSDAccessibilityManager* Manager = GetAccessibilityManager(WorldContextObject);
    if (!Manager || !Manager->GetConfig())
    {
        return false;
    }

    return Manager->GetConfig()->InteractionSettings.bSinglePressAlternative;
}
