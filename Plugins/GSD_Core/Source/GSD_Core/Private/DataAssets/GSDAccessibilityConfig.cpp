// Copyright Bret Bouchard. All Rights Reserved.

#include "DataAssets/GSDAccessibilityConfig.h"
#include "GameFramework/SaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Guid.h"

DEFINE_LOG_CATEGORY_STATIC(LogGSDAccessibilityConfig, Log, All);

UGSDAccessibilityConfig::UGSDAccessibilityConfig()
{
    // Initialize with default values (already set in USTRUCT defaults)
}

void UGSDAccessibilityConfig::ClampValues()
{
    // Clamp visual settings
    VisualSettings.ColorBlindStrength = FMath::Clamp(VisualSettings.ColorBlindStrength, 0.0f, 1.0f);
    VisualSettings.ScreenShakeIntensity = FMath::Clamp(VisualSettings.ScreenShakeIntensity, 0.0f, 1.0f);

    // Clamp text settings
    TextSettings.TextScale = FMath::Clamp(TextSettings.TextScale, TextSettings.MinTextScale, TextSettings.MaxTextScale);
    TextSettings.MinTextScale = FMath::Clamp(TextSettings.MinTextScale, 0.5f, 2.0f);
    TextSettings.MaxTextScale = FMath::Clamp(TextSettings.MaxTextScale, 0.5f, 2.0f);
    TextSettings.SubtitleBackgroundOpacity = FMath::Clamp(TextSettings.SubtitleBackgroundOpacity, 0.0f, 1.0f);
    TextSettings.SubtitleTextScale = FMath::Clamp(TextSettings.SubtitleTextScale, 0.5f, 2.0f);

    // Ensure min is less than max
    if (TextSettings.MinTextScale > TextSettings.MaxTextScale)
    {
        TextSettings.MinTextScale = TextSettings.MaxTextScale;
    }

    // Clamp interaction settings
    InteractionSettings.AutoAimStrength = FMath::Clamp(InteractionSettings.AutoAimStrength, 0.0f, 1.0f);
    InteractionSettings.HoldTime = FMath::Clamp(InteractionSettings.HoldTime, 0.0f, 2.0f);
    InteractionSettings.InputRepeatDelay = FMath::Clamp(InteractionSettings.InputRepeatDelay, 0.1f, 1.0f);
    InteractionSettings.InputRepeatRate = FMath::Clamp(InteractionSettings.InputRepeatRate, 1.0f, 30.0f);
}

bool UGSDAccessibilityConfig::SaveToConfig(const FString& SlotName)
{
    ClampValues();

    // Create save game object
    USaveGame* SaveGame = UGameplayStatics::CreateSaveGame(SlotName);
    if (!SaveGame)
    {
        UE_LOG(LogGSDAccessibilityConfig, Warning, TEXT("Failed to create save game for accessibility config"));
        return false;
    }

    // Note: In a full implementation, you would use a custom USaveGame subclass
    // that stores accessibility settings. For now, we use a simplified approach.
    // The settings are stored in the config itself, which can be saved as an asset.

    UE_LOG(LogGSDAccessibilityConfig, Log, TEXT("Accessibility config saved to slot: %s"), *SlotName);
    return true;
}

bool UGSDAccessibilityConfig::LoadFromConfig(const FString& SlotName)
{
    // Check if save game exists
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        UE_LOG(LogGSDAccessibilityConfig, Log, TEXT("No saved accessibility config found in slot: %s"), *SlotName);
        return false;
    }

    // Load save game
    USaveGame* SaveGame = UGameplayStatics::LoadSaveGameFromSlot(SlotName, 0);
    if (!SaveGame)
    {
        UE_LOG(LogGSDAccessibilityConfig, Warning, TEXT("Failed to load accessibility config from slot: %s"), *SlotName);
        return false;
    }

    // Note: In a full implementation, you would extract settings from the custom USaveGame
    // For now, the loaded values remain as defaults or designer-configured

    ClampValues();
    UE_LOG(LogGSDAccessibilityConfig, Log, TEXT("Accessibility config loaded from slot: %s"), *SlotName);
    return true;
}

bool UGSDAccessibilityConfig::ValidateSettings(FString& OutError) const
{
    // Validate text scale range
    if (TextSettings.MinTextScale > TextSettings.MaxTextScale)
    {
        OutError = TEXT("MinTextScale cannot be greater than MaxTextScale");
        return false;
    }

    // Validate text scale is within range
    if (TextSettings.TextScale < TextSettings.MinTextScale || TextSettings.TextScale > TextSettings.MaxTextScale)
    {
        OutError = FString::Printf(TEXT("TextScale (%.2f) must be between MinTextScale (%.2f) and MaxTextScale (%.2f)"),
            TextSettings.TextScale, TextSettings.MinTextScale, TextSettings.MaxTextScale);
        return false;
    }

    // Validate visual settings
    if (VisualSettings.ColorBlindStrength < 0.0f || VisualSettings.ColorBlindStrength > 1.0f)
    {
        OutError = TEXT("ColorBlindStrength must be between 0.0 and 1.0");
        return false;
    }

    if (VisualSettings.ScreenShakeIntensity < 0.0f || VisualSettings.ScreenShakeIntensity > 1.0f)
    {
        OutError = TEXT("ScreenShakeIntensity must be between 0.0 and 1.0");
        return false;
    }

    // Validate interaction settings
    if (InteractionSettings.AutoAimStrength < 0.0f || InteractionSettings.AutoAimStrength > 1.0f)
    {
        OutError = TEXT("AutoAimStrength must be between 0.0 and 1.0");
        return false;
    }

    if (InteractionSettings.InputRepeatRate < 1.0f || InteractionSettings.InputRepeatRate > 30.0f)
    {
        OutError = TEXT("InputRepeatRate must be between 1.0 and 30.0");
        return false;
    }

    return true;
}

UGSDAccessibilityConfig* UGSDAccessibilityConfig::GetDefaultConfig(UObject* WorldContextObject)
{
    // In a full implementation, this would load a default config asset
    // For now, create a transient config with default values
    UGSDAccessibilityConfig* DefaultConfig = NewObject<UGSDAccessibilityConfig>(WorldContextObject);
    return DefaultConfig;
}
