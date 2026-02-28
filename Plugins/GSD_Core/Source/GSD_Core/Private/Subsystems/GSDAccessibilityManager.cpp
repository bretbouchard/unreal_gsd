// Copyright Bret Bouchard. All Rights Reserved.

#include "Subsystems/GSDAccessibilityManager.h"

DEFINE_LOG_CATEGORY(LogGSDAccessibility);

void UGSDAccessibilityManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Create default config if none exists
    if (!Config)
    {
        DefaultConfig = NewObject<UGSDAccessibilityConfig>(this);
        Config = DefaultConfig;
    }

    UE_LOG(LogGSDAccessibility, Log, TEXT("Accessibility Manager initialized"));
}

void UGSDAccessibilityManager::Deinitialize()
{
    Config = nullptr;
    DefaultConfig = nullptr;
    Super::Deinitialize();
}

void UGSDAccessibilityManager::SetConfig(UGSDAccessibilityConfig* InConfig)
{
    if (InConfig)
    {
        Config = InConfig;
    }
    else
    {
        // Fall back to default config
        Config = DefaultConfig;
    }

    ApplySettings();
}

void UGSDAccessibilityManager::ApplySettings()
{
    if (!Config)
    {
        UE_LOG(LogGSDAccessibility, Warning, TEXT("Cannot apply settings - no config set"));
        return;
    }

    // Validate settings before applying
    FString ValidationError;
    if (!Config->ValidateSettings(ValidationError))
    {
        UE_LOG(LogGSDAccessibility, Warning, TEXT("Settings validation failed: %s"), *ValidationError);
        return;
    }

    // Broadcast settings change
    OnAccessibilitySettingsChanged.Broadcast();

    UE_LOG(LogGSDAccessibility, Log, TEXT("Accessibility settings applied - HighContrast: %d, ReduceMotion: %d, ColorBlindType: %d"),
        Config->VisualSettings.bHighContrastMode,
        Config->VisualSettings.bReduceMotion,
        (int32)Config->VisualSettings.ColorBlindType);
}

FLinearColor UGSDAccessibilityManager::ApplyColorBlindCorrection(const FLinearColor& InColor) const
{
    if (!Config || Config->VisualSettings.ColorBlindType == EGSDColorBlindType::None)
    {
        return InColor;
    }

    const float Strength = Config->VisualSettings.ColorBlindStrength;

    switch (Config->VisualSettings.ColorBlindType)
    {
    case EGSDColorBlindType::Deuteranope:
        return ApplyDeuteranopeCorrection(InColor, Strength);
    case EGSDColorBlindType::Protanope:
        return ApplyProtanopeCorrection(InColor, Strength);
    case EGSDColorBlindType::Tritanope:
        return ApplyTritanopeCorrection(InColor, Strength);
    case EGSDColorBlindType::Achromatope:
        return ApplyAchromatopeCorrection(InColor, Strength);
    default:
        return InColor;
    }
}

float UGSDAccessibilityManager::GetScaledTextSize(float BaseSize) const
{
    if (!Config)
    {
        return BaseSize;
    }

    return BaseSize * Config->TextSettings.TextScale;
}

bool UGSDAccessibilityManager::IsHighContrastMode() const
{
    return Config ? Config->VisualSettings.bHighContrastMode : false;
}

bool UGSDAccessibilityManager::IsReducedMotion() const
{
    return Config ? Config->VisualSettings.bReduceMotion : false;
}

bool UGSDAccessibilityManager::HasVisualAudioCues() const
{
    return Config ? Config->VisualSettings.bVisualAudioCues : false;
}

EGSDColorBlindType UGSDAccessibilityManager::GetColorBlindType() const
{
    return Config ? Config->VisualSettings.ColorBlindType : EGSDColorBlindType::None;
}

float UGSDAccessibilityManager::GetScreenShakeIntensity() const
{
    return Config ? Config->VisualSettings.ScreenShakeIntensity : 1.0f;
}

float UGSDAccessibilityManager::GetAutoAimStrength() const
{
    return Config ? Config->InteractionSettings.AutoAimStrength : 0.0f;
}

FLinearColor UGSDAccessibilityManager::GetSubtitleBackgroundColor() const
{
    if (!Config)
    {
        return FLinearColor(0.0f, 0.0f, 0.0f, 0.75f);
    }

    return FLinearColor(0.0f, 0.0f, 0.0f, Config->TextSettings.SubtitleBackgroundOpacity);
}

FLinearColor UGSDAccessibilityManager::ApplyDeuteranopeCorrection(const FLinearColor& InColor, float Strength) const
{
    // Deuteranope simulation (green-blind) - then shift toward visible colors
    // Simulation matrix approximates how a deuteranope sees the color
    float SimR = InColor.R * 0.625f + InColor.G * 0.375f;
    float SimG = InColor.R * 0.7f + InColor.G * 0.3f;
    float SimB = InColor.B;

    // Daltonization: enhance difference between red and green channels
    // Shift reds toward yellow-orange, greens toward cyan
    float ErrR = InColor.R - SimR;
    float ErrG = InColor.G - SimG;

    float CorrR = InColor.R + ErrR * 0.0f + ErrG * 0.0f;
    float CorrG = InColor.G + ErrR * 0.5f + ErrG * 0.0f;
    float CorrB = InColor.B + ErrR * 0.5f + ErrG * 0.5f;

    FLinearColor Corrected(FMath::Clamp(CorrR, 0.0f, 1.0f),
                           FMath::Clamp(CorrG, 0.0f, 1.0f),
                           FMath::Clamp(CorrB, 0.0f, 1.0f),
                           InColor.A);

    return LerpColor(InColor, Corrected, Strength);
}

FLinearColor UGSDAccessibilityManager::ApplyProtanopeCorrection(const FLinearColor& InColor, float Strength) const
{
    // Protanope simulation (red-blind)
    float SimR = InColor.R * 0.567f + InColor.G * 0.433f;
    float SimG = InColor.R * 0.558f + InColor.G * 0.442f;
    float SimB = InColor.B;

    // Daltonization: enhance red channel visibility
    float ErrR = InColor.R - SimR;
    float ErrG = InColor.G - SimG;

    float CorrR = InColor.R + ErrR * 0.0f + ErrG * 0.0f;
    float CorrG = InColor.G + ErrR * 0.7f + ErrG * 0.0f;
    float CorrB = InColor.B + ErrR * 0.7f + ErrG * 0.7f;

    FLinearColor Corrected(FMath::Clamp(CorrR, 0.0f, 1.0f),
                           FMath::Clamp(CorrG, 0.0f, 1.0f),
                           FMath::Clamp(CorrB, 0.0f, 1.0f),
                           InColor.A);

    return LerpColor(InColor, Corrected, Strength);
}

FLinearColor UGSDAccessibilityManager::ApplyTritanopeCorrection(const FLinearColor& InColor, float Strength) const
{
    // Tritanope simulation (blue-blind)
    float SimR = InColor.R * 0.95f + InColor.G * 0.05f;
    float SimG = InColor.R * 0.433f + InColor.G * 0.567f;
    float SimB = InColor.B * 0.475f + InColor.G * 0.525f;

    // Daltonization: enhance blue channel visibility
    float ErrR = InColor.R - SimR;
    float ErrG = InColor.G - SimG;
    float ErrB = InColor.B - SimB;

    float CorrR = InColor.R + ErrB * 0.7f;
    float CorrG = InColor.G + ErrB * 0.7f;
    float CorrB = InColor.B + ErrB * 0.0f;

    FLinearColor Corrected(FMath::Clamp(CorrR, 0.0f, 1.0f),
                           FMath::Clamp(CorrG, 0.0f, 1.0f),
                           FMath::Clamp(CorrB, 0.0f, 1.0f),
                           InColor.A);

    return LerpColor(InColor, Corrected, Strength);
}

FLinearColor UGSDAccessibilityManager::ApplyAchromatopeCorrection(const FLinearColor& InColor, float Strength) const
{
    // Achromatope (monochromatic) - convert to grayscale with enhanced contrast
    // Use luminance formula for better perceptual grayscale
    float Luminance = InColor.R * 0.299f + InColor.G * 0.587f + InColor.B * 0.114f;

    // Apply high contrast enhancement
    float EnhancedLuminance = FMath::Pow(Luminance, 0.8f); // Slight gamma boost for visibility

    FLinearColor Grayscale(EnhancedLuminance, EnhancedLuminance, EnhancedLuminance, InColor.A);

    return LerpColor(InColor, Grayscale, Strength);
}

FLinearColor UGSDAccessibilityManager::LerpColor(const FLinearColor& A, const FLinearColor& B, float Alpha) const
{
    return FLinearColor(
        FMath::Lerp(A.R, B.R, Alpha),
        FMath::Lerp(A.G, B.G, Alpha),
        FMath::Lerp(A.B, B.B, Alpha),
        FMath::Lerp(A.A, B.A, Alpha)
    );
}

void UGSDAccessibilityManager::RGBToLMS(float R, float G, float B, float& L, float& M, float& S) const
{
    // Standard RGB to LMS color space transformation
    L = R * 0.31399022f + G * 0.63951294f + B * 0.04649755f;
    M = R * 0.15537241f + G * 0.75789446f + B * 0.08670142f;
    S = R * 0.01775239f + G * 0.10944209f + B * 0.87256922f;
}

void UGSDAccessibilityManager::LMSToRGB(float L, float M, float S, float& R, float& G, float& B) const
{
    // LMS to RGB inverse transformation
    R = L * 5.47221206f + M * -4.6419601f + S * 0.16963708f;
    G = L * -1.1252419f + M * 2.29317094f + S * -0.1678952f;
    B = L * 0.02980165f + M * -0.19318073f + S * 1.16364789f;
}
