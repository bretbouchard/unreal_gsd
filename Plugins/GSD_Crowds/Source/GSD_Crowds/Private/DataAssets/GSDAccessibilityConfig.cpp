// Copyright Bret Bouchard. All Rights Reserved.

#include "DataAssets/GSDAccessibilityConfig.h"
#include "GSDCrowdLog.h"
#include "Internationalization/Internationalization.h"

#if WITH_EDITOR
#include "Misc/MessageDialog.h"
#endif

// Use the existing log category from GSDCrowdLog.h
#define LOG_GSDAccessibility LOG_GSDCROWDS

// Color blind correction matrices based on scientific research
// These are RGB-to-RGB transformation matrices for each type of color blindness
// Source: Color blind simulation and Daltonization algorithms
namespace ColorBlindMatrices
{
    // Normal vision (identity matrix)
    static const TArray<float> None = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    // Protanopia correction (red-blind)
    // Increases red-cyan contrast and shifts red towards yellow
    static const TArray<float> Protanopia = {
        0.567f, 0.433f, 0.0f,
        0.558f, 0.442f, 0.0f,
        0.0f, 0.242f, 0.758f
    };

    // Deuteranopia correction (green-blind)
    // Increases green-magenta contrast
    static const TArray<float> Deuteranopia = {
        0.625f, 0.375f, 0.0f,
        0.7f, 0.3f, 0.0f,
        0.0f, 0.3f, 0.7f
    };

    // Tritanopia correction (blue-blind)
    // Increases blue-yellow contrast
    static const TArray<float> Tritanopia = {
        0.95f, 0.05f, 0.0f,
        0.0f, 0.433f, 0.567f,
        0.0f, 0.475f, 0.525f
    };

    // Monochromacy (complete color blindness)
    // Converts to luminance only
    static const TArray<float> Monochromacy = {
        0.299f, 0.587f, 0.114f,
        0.299f, 0.587f, 0.114f,
        0.299f, 0.587f, 0.114f
    };

    static const TArray<float>& GetMatrix(EGSDColorBlindMode Mode)
    {
        switch (Mode)
        {
        case EGSDColorBlindMode::Protanopia:
            return Protanopia;
        case EGSDColorBlindMode::Deuteranopia:
            return Deuteranopia;
        case EGSDColorBlindMode::Tritanopia:
            return Tritanopia;
        case EGSDColorBlindMode::Monochromacy:
            return Monochromacy;
        default:
            return None;
        }
    }
}

// Static member initialization
UGSDAccessibilityConfig* UGSDAccessibilityConfig::CachedDefaultConfig = nullptr;

UGSDAccessibilityConfig::UGSDAccessibilityConfig()
{
    // Initialize default announcement prefix
    CrowdAnnouncementPrefix = FText::FromString(TEXT("Crowd: "));

    // Initialize default high contrast colors
    HighContrastColors = FGSDHighContrastColors();

    // Initialize default screen reader settings
    ScreenReaderSettings = FGSDScreenReaderSettings();

    // Initialize default motion settings
    MotionSettings = FGSDMotionSettings();

    // Initialize default input bindings
    InitializeDefaultBindings();
}

void UGSDAccessibilityConfig::InitializeDefaultBindings()
{
    InputBindings.Empty();

    // Navigation bindings
    InputBindings.Add(FGSDInputBinding(
        EGSDInputActionType::NavigateUp,
        EKeys::Up,
        FText::FromString(TEXT("Navigate Up"))
    ));
    InputBindings.Last().SecondaryKey = EKeys::W;
    InputBindings.Last().GamepadKey = EKeys::Gamepad_DPad_Up;
    InputBindings.Last().Category = FName(TEXT("Navigation"));
    InputBindings.Last().Description = FText::FromString(TEXT("Move selection up or navigate upward"));

    InputBindings.Add(FGSDInputBinding(
        EGSDInputActionType::NavigateDown,
        EKeys::Down,
        FText::FromString(TEXT("Navigate Down"))
    ));
    InputBindings.Last().SecondaryKey = EKeys::S;
    InputBindings.Last().GamepadKey = EKeys::Gamepad_DPad_Down;
    InputBindings.Last().Category = FName(TEXT("Navigation"));
    InputBindings.Last().Description = FText::FromString(TEXT("Move selection down or navigate downward"));

    InputBindings.Add(FGSDInputBinding(
        EGSDInputActionType::NavigateLeft,
        EKeys::Left,
        FText::FromString(TEXT("Navigate Left"))
    ));
    InputBindings.Last().SecondaryKey = EKeys::A;
    InputBindings.Last().GamepadKey = EKeys::Gamepad_DPad_Left;
    InputBindings.Last().Category = FName(TEXT("Navigation"));
    InputBindings.Last().Description = FText::FromString(TEXT("Move selection left or navigate leftward"));

    InputBindings.Add(FGSDInputBinding(
        EGSDInputActionType::NavigateRight,
        EKeys::Right,
        FText::FromString(TEXT("Navigate Right"))
    ));
    InputBindings.Last().SecondaryKey = EKeys::D;
    InputBindings.Last().GamepadKey = EKeys::Gamepad_DPad_Right;
    InputBindings.Last().Category = FName(TEXT("Navigation"));
    InputBindings.Last().Description = FText::FromString(TEXT("Move selection right or navigate rightward"));

    // Action bindings
    InputBindings.Add(FGSDInputBinding(
        EGSDInputActionType::Select,
        EKeys::Enter,
        FText::FromString(TEXT("Select/Interact"))
    ));
    InputBindings.Last().SecondaryKey = EKeys::E;
    InputBindings.Last().GamepadKey = EKeys::Gamepad_FaceButton_Bottom;
    InputBindings.Last().Category = FName(TEXT("Actions"));
    InputBindings.Last().Description = FText::FromString(TEXT("Confirm selection or interact with object"));

    InputBindings.Add(FGSDInputBinding(
        EGSDInputActionType::Cancel,
        EKeys::Escape,
        FText::FromString(TEXT("Cancel/Back"))
    ));
    InputBindings.Last().SecondaryKey = EKeys::Q;
    InputBindings.Last().GamepadKey = EKeys::Gamepad_FaceButton_Right;
    InputBindings.Last().Category = FName(TEXT("Actions"));
    InputBindings.Last().Description = FText::FromString(TEXT("Cancel current action or go back"));

    // Quick actions
    InputBindings.Add(FGSDInputBinding(
        EGSDInputActionType::QuickAction1,
        EKeys::One,
        FText::FromString(TEXT("Quick Action 1"))
    ));
    InputBindings.Last().GamepadKey = EKeys::Gamepad_FaceButton_Left;
    InputBindings.Last().Category = FName(TEXT("QuickActions"));
    InputBindings.Last().Description = FText::FromString(TEXT("Perform quick action 1"));

    InputBindings.Add(FGSDInputBinding(
        EGSDInputActionType::QuickAction2,
        EKeys::Two,
        FText::FromString(TEXT("Quick Action 2"))
    ));
    InputBindings.Last().GamepadKey = EKeys::Gamepad_FaceButton_Top;
    InputBindings.Last().Category = FName(TEXT("QuickActions"));
    InputBindings.Last().Description = FText::FromString(TEXT("Perform quick action 2"));

    // System bindings
    InputBindings.Add(FGSDInputBinding(
        EGSDInputActionType::MenuToggle,
        EKeys::Tab,
        FText::FromString(TEXT("Toggle Menu"))
    ));
    InputBindings.Last().GamepadKey = EKeys::Gamepad_Special_Left;
    InputBindings.Last().Category = FName(TEXT("System"));
    InputBindings.Last().Description = FText::FromString(TEXT("Open or close the main menu"));

    InputBindings.Add(FGSDInputBinding(
        EGSDInputActionType::DebugToggle,
        EKeys::BackTick,
        FText::FromString(TEXT("Toggle Debug"))
    ));
    InputBindings.Last().Category = FName(TEXT("System"));
    InputBindings.Last().Description = FText::FromString(TEXT("Toggle debug display"));
    InputBindings.Last().bCanRemap = false; // Debug toggle usually shouldn't be remapped
}

UGSDAccessibilityConfig* UGSDAccessibilityConfig::GetDefaultConfig()
{
    if (CachedDefaultConfig)
    {
        return CachedDefaultConfig;
    }

    // Try to load from default path
    static const FString DefaultPath = TEXT("/GSD_Crowds/Config/DA_AccessibilityConfig.DA_AccessibilityConfig");
    CachedDefaultConfig = LoadObject<UGSDAccessibilityConfig>(nullptr, *DefaultPath);

    if (!CachedDefaultConfig)
    {
        // Try alternate paths
        static const FString AlternatePath = TEXT("/Game/GSD_Crowds/Config/DA_AccessibilityConfig.DA_AccessibilityConfig");
        CachedDefaultConfig = LoadObject<UGSDAccessibilityConfig>(nullptr, *AlternatePath);
    }

    if (CachedDefaultConfig)
    {
        GSD_CROWD_LOG(Log, TEXT("Loaded default accessibility config: %s"), *CachedDefaultConfig->GetPathName());
    }
    else
    {
        GSD_CROWD_WARN(TEXT("Could not find default accessibility config. Create one at /GSD_Crowds/Config/DA_AccessibilityConfig"));
    }

    return CachedDefaultConfig;
}

FLinearColor UGSDAccessibilityConfig::ApplyColorBlindCorrection(
    const FLinearColor& InColor,
    EGSDColorBlindMode Mode,
    float Strength)
{
    if (Mode == EGSDColorBlindMode::None || Strength <= 0.0f)
    {
        return InColor;
    }

    const TArray<float>& Matrix = ColorBlindMatrices::GetMatrix(Mode);

    // Apply 3x3 color transformation matrix
    const float R = InColor.R;
    const float G = InColor.G;
    const float B = InColor.B;

    FLinearColor CorrectedColor;
    CorrectedColor.R = FMath::Clamp(Matrix[0] * R + Matrix[1] * G + Matrix[2] * B, 0.0f, 1.0f);
    CorrectedColor.G = FMath::Clamp(Matrix[3] * R + Matrix[4] * G + Matrix[5] * B, 0.0f, 1.0f);
    CorrectedColor.B = FMath::Clamp(Matrix[6] * R + Matrix[7] * G + Matrix[8] * B, 0.0f, 1.0f);
    CorrectedColor.A = InColor.A;

    // Interpolate between original and corrected based on strength
    return FMath::Lerp(InColor, CorrectedColor, FMath::Clamp(Strength, 0.0f, 1.0f));
}

FLinearColor UGSDAccessibilityConfig::ApplyCurrentColorBlindCorrection(const FLinearColor& InColor) const
{
    return ApplyColorBlindCorrection(InColor, ColorBlindMode, ColorBlindCorrectionStrength);
}

float UGSDAccessibilityConfig::GetScaledFontSize(float BaseFontSize) const
{
    const float EffectiveScale = GetEffectiveTextScale();
    const float ScaledSize = BaseFontSize * EffectiveScale;
    return FMath::Max(ScaledSize, static_cast<float>(MinimumFontSize));
}

float UGSDAccessibilityConfig::GetEffectiveTextScale() const
{
    if (bLargeTextMode)
    {
        return FMath::Max(TextScaleMultiplier, LargeTextScale);
    }
    return TextScaleMultiplier;
}

FLinearColor UGSDAccessibilityConfig::GetHighContrastColor(bool bIsSelected) const
{
    if (!bHighContrastMode)
    {
        return FLinearColor::White;
    }

    return bIsSelected ? HighContrastColors.Selection : HighContrastColors.PrimaryText;
}

bool UGSDAccessibilityConfig::GetInputBinding(EGSDInputActionType ActionType, FGSDInputBinding& OutBinding) const
{
    for (const FGSDInputBinding& Binding : InputBindings)
    {
        if (Binding.ActionType == ActionType)
        {
            OutBinding = Binding;
            return true;
        }
    }
    return false;
}

bool UGSDAccessibilityConfig::SetInputBinding(const FGSDInputBinding& Binding)
{
    if (!bEnableInputRemapping)
    {
        GSD_CROWD_WARN(TEXT("Input remapping is disabled"));
        return false;
    }

    if (!Binding.bCanRemap)
    {
        GSD_CROWD_WARN(TEXT("Input binding %d cannot be remapped"), static_cast<int32>(Binding.ActionType));
        return false;
    }

    // Find and update existing binding
    for (int32 i = 0; i < InputBindings.Num(); ++i)
    {
        if (InputBindings[i].ActionType == Binding.ActionType)
        {
            InputBindings[i] = Binding;
            GSD_CROWD_LOG(Log, TEXT("Updated input binding for action %d"), static_cast<int32>(Binding.ActionType));
            return true;
        }
    }

    // Add new binding if not found
    InputBindings.Add(Binding);
    GSD_CROWD_LOG(Log, TEXT("Added new input binding for action %d"), static_cast<int32>(Binding.ActionType));
    return true;
}

bool UGSDAccessibilityConfig::ResetInputBinding(EGSDInputActionType ActionType)
{
    // Store current bindings
    TArray<FGSDInputBinding> CurrentBindings = InputBindings;

    // Reinitialize defaults
    InitializeDefaultBindings();

    // Find the default binding
    FGSDInputBinding DefaultBinding;
    if (GetInputBinding(ActionType, DefaultBinding))
    {
        // Restore to current array
        InputBindings = CurrentBindings;

        // Update with default
        return SetInputBinding(DefaultBinding);
    }

    // Restore current bindings if default not found
    InputBindings = CurrentBindings;
    return false;
}

TArray<FGSDInputBinding> UGSDAccessibilityConfig::GetInputBindingsByCategory(FName Category) const
{
    TArray<FGSDInputBinding> Result;

    for (const FGSDInputBinding& Binding : InputBindings)
    {
        if (Binding.Category == Category)
        {
            Result.Add(Binding);
        }
    }

    return Result;
}

bool UGSDAccessibilityConfig::ShouldAnnounce(EGSDScreenReaderVerbosity Verbosity) const
{
    if (!ScreenReaderSettings.bEnabled)
    {
        return false;
    }

    return static_cast<int32>(Verbosity) <= static_cast<int32>(ScreenReaderSettings.Verbosity);
}

FText UGSDAccessibilityConfig::FormatForScreenReader(const FText& Text, bool bIsCrowdEvent) const
{
    if (!ScreenReaderSettings.bEnabled)
    {
        return FText::GetEmpty();
    }

    FString ResultString = Text.ToString();

    // Add prefix for crowd events
    if (bIsCrowdEvent && !CrowdAnnouncementPrefix.IsEmpty())
    {
        ResultString = CrowdAnnouncementPrefix.ToString() + ResultString;
    }

    // Truncate if necessary
    if (ResultString.Len() > ScreenReaderSettings.MaxAnnouncementLength)
    {
        ResultString = ResultString.Left(ScreenReaderSettings.MaxAnnouncementLength - 3) + TEXT("...");
    }

    return FText::FromString(ResultString);
}

float UGSDAccessibilityConfig::GetMotionMultiplier() const
{
    if (!MotionSettings.bReduceMotion)
    {
        return 1.0f;
    }

    return 1.0f - MotionSettings.MotionReductionAmount;
}

bool UGSDAccessibilityConfig::ShouldApplyMotionEffect(int32 EffectType) const
{
    if (!MotionSettings.bReduceMotion)
    {
        return true;
    }

    switch (EffectType)
    {
    case 1: // Camera
        return !MotionSettings.bDisableCameraShake;
    case 2: // Particles
        return !MotionSettings.bDisableParticles;
    case 3: // Animation
        return !MotionSettings.bReduceCrowdAnimation;
    default: // Any
        return GetMotionMultiplier() > 0.0f;
    }
}

void UGSDAccessibilityConfig::GetColorBlindCorrectionMatrix(EGSDColorBlindMode Mode, TArray<float>& OutMatrix)
{
    OutMatrix = ColorBlindMatrices::GetMatrix(Mode);
}

bool UGSDAccessibilityConfig::ValidateSettings(TArray<FText>& OutIssues) const
{
    OutIssues.Empty();
    bool bIsValid = true;

    // Validate text scale
    if (TextScaleMultiplier < 0.5f || TextScaleMultiplier > 2.0f)
    {
        OutIssues.Add(FText::FromString(TEXT("Text scale multiplier is outside recommended range (0.5 - 2.0)")));
        bIsValid = false;
    }

    // Validate color blind strength
    if (ColorBlindCorrectionStrength < 0.0f || ColorBlindCorrectionStrength > 1.0f)
    {
        OutIssues.Add(FText::FromString(TEXT("Color blind correction strength must be between 0 and 1")));
        bIsValid = false;
    }

    // Validate screen reader settings
    if (ScreenReaderSettings.bEnabled && ScreenReaderSettings.MaxAnnouncementLength < 10)
    {
        OutIssues.Add(FText::FromString(TEXT("Screen reader max announcement length is too short")));
        bIsValid = false;
    }

    // Check for conflicting settings
    if (bLargeTextMode && TextScaleMultiplier > 1.5f)
    {
        OutIssues.Add(FText::FromString(TEXT("Large text mode is enabled with high text scale - consider reducing one")));
    }

    // Validate input bindings
    TSet<EGSDInputActionType> SeenActions;
    TSet<FKey> SeenPrimaryKeys;
    TSet<FKey> SeenSecondaryKeys;

    for (const FGSDInputBinding& Binding : InputBindings)
    {
        // Check for duplicate actions
        if (SeenActions.Contains(Binding.ActionType))
        {
            OutIssues.Add(FText::FromString(FString::Printf(
                TEXT("Duplicate input binding for action type %d"),
                static_cast<int32>(Binding.ActionType))));
            bIsValid = false;
        }
        SeenActions.Add(Binding.ActionType);

        // Check for duplicate primary keys
        if (Binding.PrimaryKey.IsValid() && SeenPrimaryKeys.Contains(Binding.PrimaryKey))
        {
            OutIssues.Add(FText::FromString(FString::Printf(
                TEXT("Primary key %s is bound to multiple actions"),
                *Binding.PrimaryKey.ToString())));
        }
        SeenPrimaryKeys.Add(Binding.PrimaryKey);
    }

    // Validate high contrast colors have sufficient contrast
    if (bHighContrastMode)
    {
        // Calculate relative luminance difference between text and background
        auto CalcLuminance = [](const FLinearColor& Color) -> float
        {
            return 0.299f * Color.R + 0.587f * Color.G + 0.114f * Color.B;
        };

        const float TextLum = CalcLuminance(HighContrastColors.PrimaryText);
        const float BgLum = CalcLuminance(HighContrastColors.Background);
        const float Contrast = FMath::Abs(TextLum - BgLum);

        if (Contrast < 0.7f)
        {
            OutIssues.Add(FText::FromString(TEXT("High contrast text/background may not have sufficient contrast ratio")));
        }
    }

    // Validate motion settings
    if (MotionSettings.bReduceMotion && MotionSettings.MotionReductionAmount <= 0.0f)
    {
        OutIssues.Add(FText::FromString(TEXT("Reduce motion is enabled but reduction amount is 0")));
    }

    if (bEnableDebugLogging)
    {
        GSD_CROWD_LOG(Log, TEXT("Validation completed with %d issues"), OutIssues.Num());
    }

    return bIsValid;
}

void UGSDAccessibilityConfig::ApplyAccessibilityProfile(int32 ProfileType)
{
    switch (ProfileType)
    {
    case 1: // Color blind profile
        ColorBlindMode = EGSDColorBlindMode::Deuteranopia; // Most common
        ColorBlindCorrectionStrength = 1.0f;
        bApplyToCrowdVisualization = true;
        bApplyToUI = true;
        bHighContrastMode = true;
        TextScaleMultiplier = 1.1f;
        GSD_CROWD_LOG(Log, TEXT("Applied color blind accessibility profile"));
        break;

    case 2: // Low vision profile
        TextScaleMultiplier = 1.5f;
        bLargeTextMode = true;
        MinimumFontSize = 16;
        bHighContrastMode = true;
        HighContrastOutlineThickness = 4.0f;
        ScreenReaderSettings.bEnabled = true;
        ScreenReaderSettings.Verbosity = EGSDScreenReaderVerbosity::Verbose;
        GSD_CROWD_LOG(Log, TEXT("Applied low vision accessibility profile"));
        break;

    case 3: // Motor impairment profile
        InputHoldTime = 0.2f;
        InputRepeatDelay = 0.8f;
        bEnableInputRemapping = true;
        bAllowGamepadRemapping = true;
        MotionSettings.bReduceMotion = true;
        MotionSettings.MotionReductionAmount = 0.5f;
        GSD_CROWD_LOG(Log, TEXT("Applied motor impairment accessibility profile"));
        break;

    case 4: // Cognitive profile
        TextScaleMultiplier = 1.2f;
        ScreenReaderSettings.bEnabled = true;
        ScreenReaderSettings.Verbosity = EGSDScreenReaderVerbosity::Standard;
        MotionSettings.bReduceMotion = true;
        MotionSettings.bDisableParticles = true;
        MotionSettings.MotionReductionAmount = 0.7f;
        GSD_CROWD_LOG(Log, TEXT("Applied cognitive accessibility profile"));
        break;

    default: // Default profile (reset to defaults)
        ColorBlindMode = EGSDColorBlindMode::None;
        ColorBlindCorrectionStrength = 1.0f;
        TextScaleMultiplier = 1.0f;
        bLargeTextMode = false;
        MinimumFontSize = 12;
        bHighContrastMode = false;
        ScreenReaderSettings = FGSDScreenReaderSettings();
        MotionSettings = FGSDMotionSettings();
        InitializeDefaultBindings();
        GSD_CROWD_LOG(Log, TEXT("Applied default accessibility profile"));
        break;
    }

    // Validate after applying profile
    TArray<FText> Issues;
    ValidateSettings(Issues);

#if WITH_EDITOR
    if (Issues.Num() > 0)
    {
        FString IssueString;
        for (const FText& Issue : Issues)
        {
            IssueString += Issue.ToString() + TEXT("\n");
        }
        FMessageDialog::Open(EAppMsgType::Warning,
            FText::FromString(FString::Printf(TEXT("Accessibility profile applied with warnings:\n%s"), *IssueString)));
    }
#endif
}
