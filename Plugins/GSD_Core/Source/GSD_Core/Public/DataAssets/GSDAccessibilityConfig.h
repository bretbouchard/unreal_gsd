// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GSDAccessibilityConfig.generated.h"

/**
 * Types of color blindness supported by the accessibility system.
 */
UENUM(BlueprintType)
enum class EGSDColorBlindType : uint8
{
    None            UMETA(DisplayName = "None (Normal Vision)"),
    Deuteranope     UMETA(DisplayName = "Deuteranope (Red-Green)"),
    Protanope       UMETA(DisplayName = "Protanope (Red-Green)"),
    Tritanope       UMETA(DisplayName = "Tritanope (Blue-Yellow)"),
    Achromatope     UMETA(DisplayName = "Achromatope (Monochromatic)")
};

/**
 * Visual accessibility settings for players with visual impairments.
 */
USTRUCT(BlueprintType)
struct FGSDVisualAccessibilitySettings
{
    GENERATED_BODY()

    // Enable high contrast mode for better visibility
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
    bool bHighContrastMode = false;

    // Type of color blindness correction to apply
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
    EGSDColorBlindType ColorBlindType = EGSDColorBlindType::None;

    // Strength of color blind correction (0.0 to 1.0)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float ColorBlindStrength = 1.0f;

    // Reduce motion and animation effects
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
    bool bReduceMotion = false;

    // Screen shake intensity multiplier (0.0 to 1.0)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float ScreenShakeIntensity = 1.0f;

    // Show visual cues for audio events (subtitles, indicators)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
    bool bVisualAudioCues = false;
};

/**
 * Text accessibility settings for players who need larger or clearer text.
 */
USTRUCT(BlueprintType)
struct FGSDTextAccessibilitySettings
{
    GENERATED_BODY()

    // Base text scale multiplier (0.5 to 2.0)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Text", meta = (ClampMin = "0.5", ClampMax = "2.0", UIMin = "0.5", UIMax = "2.0"))
    float TextScale = 1.0f;

    // Minimum allowed text scale
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Text", meta = (ClampMin = "0.5", ClampMax = "2.0", UIMin = "0.5", UIMax = "2.0"))
    float MinTextScale = 0.5f;

    // Maximum allowed text scale
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Text", meta = (ClampMin = "0.5", ClampMax = "2.0", UIMin = "0.5", UIMax = "2.0"))
    float MaxTextScale = 2.0f;

    // Always show subtitles regardless of audio settings
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Text")
    bool bAlwaysShowSubtitles = false;

    // Opacity of subtitle background (0.0 to 1.0)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Text", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float SubtitleBackgroundOpacity = 0.75f;

    // Scale multiplier for subtitle text
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Text", meta = (ClampMin = "0.5", ClampMax = "2.0", UIMin = "0.5", UIMax = "2.0"))
    float SubtitleTextScale = 1.0f;
};

/**
 * Interaction accessibility settings for players with motor impairments.
 */
USTRUCT(BlueprintType)
struct FGSDInteractionAccessibilitySettings
{
    GENERATED_BODY()

    // Auto-aim assistance strength (0.0 = off, 1.0 = full)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float AutoAimStrength = 0.0f;

    // Time required to hold a button before action triggers (seconds)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
    float HoldTime = 0.5f;

    // Enable single-press alternative for hold actions
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
    bool bSinglePressAlternative = false;

    // Delay before input repeats (seconds)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0.1", ClampMax = "1.0", UIMin = "0.1", UIMax = "1.0"))
    float InputRepeatDelay = 0.5f;

    // Rate of input repetition (repeats per second)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "1.0", ClampMax = "30.0", UIMin = "1.0", UIMax = "30.0"))
    float InputRepeatRate = 10.0f;
};

/**
 * Accessibility configuration data asset.
 * Contains all accessibility settings for visual, text, and interaction accommodations.
 */
UCLASS(BlueprintType, Category = "GSD|Accessibility")
class GSD_CORE_API UGSDAccessibilityConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    UGSDAccessibilityConfig();

    // Visual accessibility settings
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Accessibility")
    FGSDVisualAccessibilitySettings VisualSettings;

    // Text accessibility settings
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Accessibility")
    FGSDTextAccessibilitySettings TextSettings;

    // Interaction accessibility settings
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Accessibility")
    FGSDInteractionAccessibilitySettings InteractionSettings;

    // Save config to player settings (for persistence)
    UFUNCTION(BlueprintCallable, Category = "GSD|Accessibility")
    bool SaveToConfig(const FString& SlotName = TEXT("Accessibility"));

    // Load config from player settings
    UFUNCTION(BlueprintCallable, Category = "GSD|Accessibility")
    bool LoadFromConfig(const FString& SlotName = TEXT("Accessibility"));

    // Validate all settings
    UFUNCTION(BlueprintCallable, Category = "GSD|Accessibility")
    bool ValidateSettings(FString& OutError) const;

    // Get default accessibility config
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility", meta = (WorldContext = "WorldContextObject"))
    static UGSDAccessibilityConfig* GetDefaultConfig(UObject* WorldContextObject);

protected:
    // Clamp all values to valid ranges
    void ClampValues();

    // Config version for migration support
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
    int32 ConfigVersion = 1;
};
