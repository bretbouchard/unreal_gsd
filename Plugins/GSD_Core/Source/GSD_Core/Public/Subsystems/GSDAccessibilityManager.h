// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "DataAssets/GSDAccessibilityConfig.h"
#include "GSDAccessibilityManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGSDAccessibility, Log, All);

// Delegate broadcast when accessibility settings change
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAccessibilitySettingsChanged);

/**
 * Accessibility manager subsystem.
 * Manages accessibility settings and provides helper functions for visual accommodations.
 */
UCLASS()
class GSD_CORE_API UGSDAccessibilityManager : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Set the current accessibility configuration
    UFUNCTION(BlueprintCallable, Category = "GSD|Accessibility")
    void SetConfig(UGSDAccessibilityConfig* InConfig);

    // Get the current accessibility configuration
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility")
    UGSDAccessibilityConfig* GetConfig() const { return Config; }

    // Apply all settings from current config
    UFUNCTION(BlueprintCallable, Category = "GSD|Accessibility")
    void ApplySettings();

    // Apply color blind correction to a color
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility", meta = (DisplayName = "Correct Color for Color Blindness"))
    FLinearColor ApplyColorBlindCorrection(const FLinearColor& InColor) const;

    // Get scaled text size based on accessibility settings
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility")
    float GetScaledTextSize(float BaseSize) const;

    // Check if high contrast mode is enabled
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility")
    bool IsHighContrastMode() const;

    // Check if reduced motion is enabled
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility")
    bool IsReducedMotion() const;

    // Check if visual audio cues are enabled
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility")
    bool HasVisualAudioCues() const;

    // Get current color blind type
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility")
    EGSDColorBlindType GetColorBlindType() const;

    // Get screen shake intensity multiplier
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility")
    float GetScreenShakeIntensity() const;

    // Get auto-aim strength
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility")
    float GetAutoAimStrength() const;

    // Get subtitle background color with opacity
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility")
    FLinearColor GetSubtitleBackgroundColor() const;

    // Delegate for settings change notifications
    UPROPERTY(BlueprintAssignable, Category = "GSD|Accessibility")
    FOnAccessibilitySettingsChanged OnAccessibilitySettingsChanged;

protected:
    // Current accessibility configuration
    UPROPERTY()
    TObjectPtr<UGSDAccessibilityConfig> Config = nullptr;

    // Default config created if none provided
    UPROPERTY()
    TObjectPtr<UGSDAccessibilityConfig> DefaultConfig = nullptr;

private:
    // Color blind correction implementations
    FLinearColor ApplyDeuteranopeCorrection(const FLinearColor& InColor, float Strength) const;
    FLinearColor ApplyProtanopeCorrection(const FLinearColor& InColor, float Strength) const;
    FLinearColor ApplyTritanopeCorrection(const FLinearColor& InColor, float Strength) const;
    FLinearColor ApplyAchromatopeCorrection(const FLinearColor& InColor, float Strength) const;

    // Helper for linear interpolation in color correction
    FLinearColor LerpColor(const FLinearColor& A, const FLinearColor& B, float Alpha) const;

    // RGB to LMS color space conversion helpers
    void RGBToLMS(float R, float G, float B, float& L, float& M, float& S) const;
    void LMSToRGB(float L, float M, float S, float& R, float& G, float& B) const;
};
