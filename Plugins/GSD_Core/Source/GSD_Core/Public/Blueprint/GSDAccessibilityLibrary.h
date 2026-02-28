// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DataAssets/GSDAccessibilityConfig.h"
#include "GSDAccessibilityLibrary.generated.h"

class UGSDAccessibilityManager;
class UMaterialInterface;
class UMaterialInstanceDynamic;

/**
 * Blueprint function library for accessibility features.
 * Provides easy access to accessibility settings and helper functions.
 */
UCLASS()
class GSD_CORE_API UGSDAccessibilityLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // Get the accessibility manager subsystem
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility", meta = (WorldContext = "WorldContextObject"))
    static UGSDAccessibilityManager* GetAccessibilityManager(UObject* WorldContextObject);

    // Correct a color for the current color blindness setting
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility", meta = (WorldContext = "WorldContextObject", DisplayName = "Correct Color for Color Blindness"))
    static FLinearColor CorrectColorForColorBlindness(UObject* WorldContextObject, const FLinearColor& InColor);

    // Get scaled text size based on accessibility settings
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility", meta = (WorldContext = "WorldContextObject"))
    static float GetScaledTextSize(UObject* WorldContextObject, float BaseSize);

    // Check if high contrast mode is enabled
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility", meta = (WorldContext = "WorldContextObject"))
    static bool IsHighContrastModeEnabled(UObject* WorldContextObject);

    // Check if reduced motion is enabled
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility", meta = (WorldContext = "WorldContextObject"))
    static bool IsReducedMotionEnabled(UObject* WorldContextObject);

    // Check if visual audio cues are enabled
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility", meta = (WorldContext = "WorldContextObject"))
    static bool AreVisualAudioCuesEnabled(UObject* WorldContextObject);

    // Apply accessibility corrections to a material
    UFUNCTION(BlueprintCallable, Category = "GSD|Accessibility", meta = (WorldContext = "WorldContextObject"))
    static UMaterialInstanceDynamic* ApplyAccessibilityToMaterial(UObject* WorldContextObject, UMaterialInterface* BaseMaterial, FName ColorParameterName = NAME_None);

    // Get subtitle background color with current opacity setting
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility", meta = (WorldContext = "WorldContextObject"))
    static FLinearColor GetSubtitleBackgroundColor(UObject* WorldContextObject);

    // Get current color blind type
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility", meta = (WorldContext = "WorldContextObject"))
    static EGSDColorBlindType GetCurrentColorBlindType(UObject* WorldContextObject);

    // Get screen shake intensity multiplier
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility", meta = (WorldContext = "WorldContextObject"))
    static float GetScreenShakeIntensity(UObject* WorldContextObject);

    // Get auto-aim strength
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility", meta = (WorldContext = "WorldContextObject"))
    static float GetAutoAimStrength(UObject* WorldContextObject);

    // Get text scale setting
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility", meta = (WorldContext = "WorldContextObject"))
    static float GetTextScale(UObject* WorldContextObject);

    // Check if subtitles should always be shown
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility", meta = (WorldContext = "WorldContextObject"))
    static bool ShouldAlwaysShowSubtitles(UObject* WorldContextObject);

    // Get subtitle text scale
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility", meta = (WorldContext = "WorldContextObject"))
    static float GetSubtitleTextScale(UObject* WorldContextObject);

    // Get input hold time setting
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility", meta = (WorldContext = "WorldContextObject"))
    static float GetInputHoldTime(UObject* WorldContextObject);

    // Check if single press alternative is enabled
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility", meta = (WorldContext = "WorldContextObject"))
    static bool IsSinglePressAlternativeEnabled(UObject* WorldContextObject);
};
