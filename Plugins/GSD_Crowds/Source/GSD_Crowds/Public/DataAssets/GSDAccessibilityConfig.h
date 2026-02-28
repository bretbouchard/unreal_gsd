// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputCoreTypes.h"
#include "GSDAccessibilityConfig.generated.h"

class UTexture2D;
class UMaterialInterface;
class UFont;

/**
 * Color blind mode types supported by the accessibility system.
 * Each mode applies specific color correction matrices for different types of color vision deficiency.
 */
UENUM(BlueprintType, Category = "GSD|Accessibility")
enum class EGSDColorBlindMode : uint8
{
    /** No color blind correction applied */
    None = 0,
    /** Protanopia - Red color blindness (affects ~1% of males) */
    Protanopia = 1,
    /** Deuteranopia - Green color blindness (most common, affects ~6% of males) */
    Deuteranopia = 2,
    /** Tritanopia - Blue color blindness (rare, affects ~0.01% of population) */
    Tritanopia = 3,
    /** Monochromacy - Complete color blindness (very rare) */
    Monochromacy = 4,
    Count UMETA(Hidden)
};

/**
 * Screen reader verbosity levels for accessibility announcements.
 */
UENUM(BlueprintType, Category = "GSD|Accessibility")
enum class EGSDScreenReaderVerbosity : uint8
{
    /** Minimal announcements - critical information only */
    Minimal = 0,
    /** Standard announcements - important state changes and events */
    Standard = 1,
    /** Verbose announcements - detailed information including context */
    Verbose = 2,
    /** Debug mode - all possible announcements for testing */
    Debug = 3,
    Count UMETA(Hidden)
};

/**
 * Input action types for remapping support.
 */
UENUM(BlueprintType, Category = "GSD|Accessibility")
enum class EGSDInputActionType : uint8
{
    /** No action */
    None = 0,
    /** Selection/interact action */
    Select = 1,
    /** Cancel/back action */
    Cancel = 2,
    /** Navigate up */
    NavigateUp = 3,
    /** Navigate down */
    NavigateDown = 4,
    /** Navigate left */
    NavigateLeft = 5,
    /** Navigate right */
    NavigateRight = 6,
    /** Quick action 1 */
    QuickAction1 = 7,
    /** Quick action 2 */
    QuickAction2 = 8,
    /** Menu toggle */
    MenuToggle = 9,
    /** Debug toggle */
    DebugToggle = 10,
    Count UMETA(Hidden)
};

/**
 * Input binding structure for remappable controls.
 */
USTRUCT(BlueprintType, Category = "GSD|Accessibility")
struct GSD_CROWDS_API FGSDInputBinding
{
    GENERATED_BODY()

    /** The type of input action this binding represents */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    EGSDInputActionType ActionType = EGSDInputActionType::None;

    /** Primary key binding */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    FKey PrimaryKey = EKeys::Invalid;

    /** Secondary key binding (for alternate input) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    FKey SecondaryKey = EKeys::Invalid;

    /** Gamepad button binding */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    FKey GamepadKey = EKeys::Invalid;

    /** Human-readable display name for the action */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    FText DisplayName;

    /** Description of what this action does */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    FText Description;

    /** Whether this binding can be remapped by the user */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    bool bCanRemap = true;

    /** Category for organizing bindings in UI */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    FName Category = NAME_None;

    FGSDInputBinding()
        : DisplayName(NSLOCTEXT("GSDAccessibility", "DefaultActionName", "Action"))
        , Description(NSLOCTEXT("GSDAccessibility", "DefaultActionDesc", "Perform action"))
    {
    }

    FGSDInputBinding(EGSDInputActionType InActionType, const FKey& InPrimaryKey, const FText& InDisplayName)
        : ActionType(InActionType)
        , PrimaryKey(InPrimaryKey)
        , DisplayName(InDisplayName)
        , Description(FText::GetEmpty())
        , bCanRemap(true)
    {
    }
};

/**
 * High contrast color scheme configuration.
 */
USTRUCT(BlueprintType, Category = "GSD|Accessibility")
struct GSD_CROWDS_API FGSDHighContrastColors
{
    GENERATED_BODY()

    /** Primary text color (high contrast) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor PrimaryText = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    /** Secondary text color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor SecondaryText = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);

    /** Background color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor Background = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);

    /** Accent/highlight color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor Accent = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);

    /** Selection color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor Selection = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);

    /** Warning color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor Warning = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);

    /** Error color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor Error = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

    /** Success color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor Success = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);

    /** Disabled state color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor Disabled = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

    /** Friendly entity color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor Friendly = FLinearColor(0.0f, 1.0f, 0.5f, 1.0f);

    /** Neutral entity color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor Neutral = FLinearColor(0.5f, 0.5f, 1.0f, 1.0f);

    /** Hostile entity color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor Hostile = FLinearColor(1.0f, 0.25f, 0.25f, 1.0f);

    /** Objective marker color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor Objective = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
};

/**
 * Screen reader configuration settings.
 */
USTRUCT(BlueprintType, Category = "GSD|Accessibility")
struct GSD_CROWDS_API FGSDScreenReaderSettings
{
    GENERATED_BODY()

    /** Enable screen reader support */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreenReader")
    bool bEnabled = false;

    /** Verbosity level for announcements */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreenReader")
    EGSDScreenReaderVerbosity Verbosity = EGSDScreenReaderVerbosity::Standard;

    /** Enable announcements for crowd state changes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreenReader")
    bool bAnnounceCrowdStates = true;

    /** Enable announcements for entity interactions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreenReader")
    bool bAnnounceInteractions = true;

    /** Enable announcements for navigation events */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreenReader")
    bool bAnnounceNavigation = true;

    /** Enable announcements for performance warnings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreenReader")
    bool bAnnouncePerformanceWarnings = true;

    /** Minimum time between similar announcements (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreenReader")
    float AnnouncementCooldown = 2.0f;

    /** Enable detailed entity descriptions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreenReader")
    bool bDetailedEntityDescriptions = false;

    /** Maximum length of announcement text */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreenReader")
    int32 MaxAnnouncementLength = 200;
};

/**
 * Motion sensitivity settings for accessibility.
 */
USTRUCT(BlueprintType, Category = "GSD|Accessibility")
struct GSD_CROWDS_API FGSDMotionSettings
{
    GENERATED_BODY()

    /** Reduce motion effects (camera shake, screen effects) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bReduceMotion = false;

    /** Motion reduction percentage (0.0 = no reduction, 1.0 = full reduction) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MotionReductionAmount = 0.5f;

    /** Disable particle effects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bDisableParticles = false;

    /** Disable camera shake */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bDisableCameraShake = false;

    /** Reduce crowd animation speed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bReduceCrowdAnimation = false;

    /** Crowd animation speed multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float CrowdAnimationSpeed = 1.0f;
};

/**
 * Accessibility configuration data asset for the GSD Crowds plugin.
 *
 * This config provides comprehensive accessibility settings including:
 * - Color blind mode support with correction matrices
 * - Text scale and font size adjustments
 * - High contrast mode with customizable colors
 * - Screen reader support with verbosity levels
 * - Input remapping for keyboard and gamepad
 * - Motion sensitivity settings
 *
 * Create instances of this DataAsset in /GSD_Crowds/Config/ to customize
 * accessibility features without code changes.
 *
 * Usage:
 * 1. Create a DataAsset derived from UGSDAccessibilityConfig
 * 2. Configure settings for your target audience
 * 3. Access via GetDefaultConfig() or through subsystem
 * 4. Apply settings to UI and gameplay systems
 */
UCLASS(BlueprintType, Category = "GSD|Crowds")
class GSD_CROWDS_API UGSDAccessibilityConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    UGSDAccessibilityConfig();

    // === Color Blind Support ===

    /** Current color blind mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ColorBlind")
    EGSDColorBlindMode ColorBlindMode = EGSDColorBlindMode::None;

    /** Color blind correction strength (0.0 = none, 1.0 = full) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ColorBlind", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ColorBlindCorrectionStrength = 1.0f;

    /** Enable color blind mode for crowd visualization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ColorBlind")
    bool bApplyToCrowdVisualization = true;

    /** Enable color blind mode for UI elements */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ColorBlind")
    bool bApplyToUI = true;

    // === Text Scale ===

    /** Text scale multiplier (1.0 = normal, 1.5 = 50% larger) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text", meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float TextScaleMultiplier = 1.0f;

    /** Minimum font size for readability */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text", meta = (ClampMin = "8", ClampMax = "32"))
    int32 MinimumFontSize = 12;

    /** Enable large text mode (alternative to scale multiplier) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text")
    bool bLargeTextMode = false;

    /** Large text scale when bLargeTextMode is enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text", meta = (ClampMin = "1.0", ClampMax = "2.0"))
    float LargeTextScale = 1.3f;

    // === High Contrast Mode ===

    /** Enable high contrast mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HighContrast")
    bool bHighContrastMode = false;

    /** High contrast color scheme */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HighContrast")
    FGSDHighContrastColors HighContrastColors;

    /** Increase outline thickness in high contrast mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HighContrast")
    float HighContrastOutlineThickness = 3.0f;

    /** Enable high contrast for crowd entity indicators */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HighContrast")
    bool bHighContrastCrowdIndicators = true;

    // === Screen Reader ===

    /** Screen reader configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreenReader")
    FGSDScreenReaderSettings ScreenReaderSettings;

    /** Screen reader announcement prefix for crowd events */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreenReader")
    FText CrowdAnnouncementPrefix;

    // === Input Remapping ===

    /** Enable input remapping */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    bool bEnableInputRemapping = true;

    /** Default input bindings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    TArray<FGSDInputBinding> InputBindings;

    /** Allow gamepad remapping */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    bool bAllowGamepadRemapping = true;

    /** Allow keyboard remapping */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    bool bAllowKeyboardRemapping = true;

    /** Input hold time for accessibility (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float InputHoldTime = 0.0f;

    /** Input repeat delay (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float InputRepeatDelay = 0.5f;

    // === Motion Sensitivity ===

    /** Motion sensitivity settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FGSDMotionSettings MotionSettings;

    // === Audio Accessibility ===

    /** Enable audio cues for visual events */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bEnableAudioCues = false;

    /** Audio cue volume multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AudioCueVolume = 0.8f;

    /** Enable visual representation of audio */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bEnableVisualAudioIndicators = false;

    // === Debug ===

    /** Enable accessibility debug logging */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bEnableDebugLogging = false;

    /** Simulate color blindness for testing (editor only) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bSimulateColorBlindness = false;

    // === Helper Functions ===

    /**
     * Get the default accessibility config.
     * @return Default config asset, or nullptr if not found
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility")
    static UGSDAccessibilityConfig* GetDefaultConfig();

    /**
     * Apply color blind correction to a color.
     * @param InColor The original color to correct
     * @param Mode The color blind mode to apply
     * @param Strength The correction strength (0-1)
     * @return The corrected color
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility")
    static FLinearColor ApplyColorBlindCorrection(
        const FLinearColor& InColor,
        EGSDColorBlindMode Mode,
        float Strength = 1.0f);

    /**
     * Apply current color blind settings to a color.
     * @param InColor The original color to correct
     * @return The corrected color based on current settings
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility")
    FLinearColor ApplyCurrentColorBlindCorrection(const FLinearColor& InColor) const;

    /**
     * Get scaled font size based on current text settings.
     * @param BaseFontSize The base font size to scale
     * @return The scaled font size
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility")
    float GetScaledFontSize(float BaseFontSize) const;

    /**
     * Get the effective text scale (considering large text mode).
     * @return The effective text scale multiplier
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility")
    float GetEffectiveTextScale() const;

    /**
     * Check if high contrast color should be used.
     * @param bIsSelected Whether the element is selected
     * @return The appropriate color for the state
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility")
    FLinearColor GetHighContrastColor(bool bIsSelected = false) const;

    /**
     * Get input binding for a specific action.
     * @param ActionType The action type to find
     * @param OutBinding The found binding (if any)
     * @return True if binding was found
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility")
    bool GetInputBinding(EGSDInputActionType ActionType, FGSDInputBinding& OutBinding) const;

    /**
     * Set input binding for a specific action.
     * @param Binding The new binding to set
     * @return True if binding was set successfully
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Accessibility")
    bool SetInputBinding(const FGSDInputBinding& Binding);

    /**
     * Reset input binding to default.
     * @param ActionType The action type to reset
     * @return True if binding was reset successfully
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Accessibility")
    bool ResetInputBinding(EGSDInputActionType ActionType);

    /**
     * Get all input bindings for a category.
     * @param Category The category to filter by
     * @return Array of bindings in the category
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility")
    TArray<FGSDInputBinding> GetInputBindingsByCategory(FName Category) const;

    /**
     * Check if screen reader should announce based on settings.
     * @param Verbosity The verbosity level of the announcement
     * @return True if announcement should be made
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility")
    bool ShouldAnnounce(EGSDScreenReaderVerbosity Verbosity) const;

    /**
     * Format text for screen reader announcement.
     * @param Text The text to format
     * @param bIsCrowdEvent Whether this is a crowd-related event
     * @return Formatted text ready for announcement
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility")
    FText FormatForScreenReader(const FText& Text, bool bIsCrowdEvent = false) const;

    /**
     * Get effective motion multiplier based on settings.
     * @return Motion multiplier (0 = disabled, 1 = normal)
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility")
    float GetMotionMultiplier() const;

    /**
     * Check if motion effect should be applied.
     * @param EffectType Type of motion effect (0=any, 1=camera, 2=particles, 3=animation)
     * @return True if effect should be applied
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility")
    bool ShouldApplyMotionEffect(int32 EffectType = 0) const;

    /**
     * Get color blind correction matrix for a specific mode.
     * Returns a 3x3 color transformation matrix.
     * @param Mode The color blind mode
     * @return Color correction matrix as array of 9 floats
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Accessibility")
    static void GetColorBlindCorrectionMatrix(EGSDColorBlindMode Mode, TArray<float>& OutMatrix);

    /**
     * Validate accessibility settings and return any issues.
     * @param OutIssues Array of issue descriptions
     * @return True if all settings are valid
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Accessibility")
    bool ValidateSettings(TArray<FText>& OutIssues) const;

    /**
     * Apply recommended settings for a specific accessibility profile.
     * @param ProfileType Profile type (0=default, 1=colorBlind, 2=lowVision, 3=motorImpairment, 4=cognitive)
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Accessibility")
    void ApplyAccessibilityProfile(int32 ProfileType);

protected:
    /** Initialize default input bindings */
    void InitializeDefaultBindings();

    /** Color blind correction matrices (RGB transformation) */
    static const TMap<EGSDColorBlindMode, TArray<float>> ColorBlindMatrices;

private:
    /** Cached default config reference */
    static UGSDAccessibilityConfig* CachedDefaultConfig;
};
