# GSD Accessibility System

The GSD Accessibility System provides comprehensive accessibility support for visual, text, and interaction accommodations. This system enables players with various accessibility needs to enjoy your game.

## Features Overview

### Visual Accessibility

- **High Contrast Mode**: Increases contrast for better visibility
- **Color Blind Correction**: Supports 4 types of color blindness:
  - Deuteranope (Red-Green)
  - Protanope (Red-Green)
  - Tritanope (Blue-Yellow)
  - Achromatope (Monochromatic)
- **Reduce Motion**: Disables or reduces animations and motion effects
- **Screen Shake Intensity**: Adjustable screen shake (0% to 100%)
- **Visual Audio Cues**: Visual indicators for audio events

### Text Accessibility

- **Text Scaling**: 0.5x to 2.0x text size multiplier
- **Subtitle Options**:
  - Always show subtitles
  - Adjustable subtitle background opacity
  - Separate subtitle text scale

### Interaction Accessibility

- **Auto-Aim Assistance**: 0% to 100% aim assistance strength
- **Hold Time Adjustment**: Configurable button hold duration
- **Single Press Alternative**: Option to use tap instead of hold
- **Input Repeat Settings**: Configurable repeat delay and rate

## Usage Examples

### Blueprint Usage

#### Get the Accessibility Manager

``` blueprint
// Get the accessibility manager subsystem
Get Accessibility Manager -> Manager
```

#### Check if High Contrast Mode is Enabled

``` blueprint
// Check high contrast mode
Is High Contrast Mode Enabled -> IsEnabled (Boolean)
```

#### Correct a Color for Color Blindness

``` blueprint
// Apply color blind correction
Correct Color for Color Blindness (InColor) -> CorrectedColor (Linear Color)
```

#### Scale Text Size

``` blueprint
// Get scaled text size based on settings
Get Scaled Text Size (BaseSize: 24.0) -> ScaledSize (Float)
```

#### Apply Accessibility to Material

``` blueprint
// Create MID with accessibility corrections applied
Apply Accessibility to Material (BaseMaterial, "BaseColor") -> MID
```

#### Get Subtitle Background Color

``` blueprint
// Get properly colored subtitle background
Get Subtitle Background Color -> BackgroundColor (Linear Color)
```

### C++ Usage

#### Include Headers

```cpp
#include "Subsystems/GSDAccessibilityManager.h"
#include "Blueprint/GSDAccessibilityLibrary.h"
```

#### Get the Accessibility Manager

```cpp
UGSDAccessibilityManager* Manager = GEngine->GetEngineSubsystem<UGSDAccessibilityManager>();
```

#### Apply Color Blind Correction

```cpp
FLinearColor OriginalColor = FLinearColor::Red;
FLinearColor CorrectedColor = Manager->ApplyColorBlindCorrection(OriginalColor);
```

#### Scale Text Size

```cpp
float BaseSize = 24.0f;
float ScaledSize = Manager->GetScaledTextSize(BaseSize);
```

#### Check Settings

```cpp
bool bHighContrast = Manager->IsHighContrastMode();
bool bReducedMotion = Manager->IsReducedMotion();
bool bVisualCues = Manager->HasVisualAudioCues();
```

#### Listen for Settings Changes

```cpp
// Bind to settings change delegate
Manager->OnAccessibilitySettingsChanged.AddDynamic(this, &AMyActor::OnAccessibilityChanged);
```

#### Using the Blueprint Library

```cpp
// Static functions for convenience
float ScaledSize = UGSDAccessibilityLibrary::GetScaledTextSize(this, 24.0f);
bool bHighContrast = UGSDAccessibilityLibrary::IsHighContrastModeEnabled(this);
FLinearColor CorrectedColor = UGSDAccessibilityLibrary::CorrectColorForColorBlindness(this, OriginalColor);
```

## Configuration Guide

### Creating an Accessibility Config

1. Right-click in the Content Browser
2. Select **Miscellaneous > Data Asset**
3. Choose **GSDAccessibilityConfig**
4. Name your config (e.g., `DA_Accessibility_Default`)

### Config Properties

#### Visual Settings

| Property | Type | Range | Description |
|----------|------|-------|-------------|
| bHighContrastMode | bool | - | Enable high contrast visuals |
| ColorBlindType | enum | None, Deuteranope, Protanope, Tritanope, Achromatope | Type of color blindness |
| ColorBlindStrength | float | 0.0 - 1.0 | Correction strength |
| bReduceMotion | bool | - | Reduce animations/motion |
| ScreenShakeIntensity | float | 0.0 - 1.0 | Screen shake multiplier |
| bVisualAudioCues | bool | - | Show visual audio indicators |

#### Text Settings

| Property | Type | Range | Description |
|----------|------|-------|-------------|
| TextScale | float | 0.5 - 2.0 | Base text scale multiplier |
| MinTextScale | float | 0.5 - 2.0 | Minimum allowed text scale |
| MaxTextScale | float | 0.5 - 2.0 | Maximum allowed text scale |
| bAlwaysShowSubtitles | bool | - | Force subtitles on |
| SubtitleBackgroundOpacity | float | 0.0 - 1.0 | Subtitle background alpha |
| SubtitleTextScale | float | 0.5 - 2.0 | Subtitle text scale |

#### Interaction Settings

| Property | Type | Range | Description |
|----------|------|-------|-------------|
| AutoAimStrength | float | 0.0 - 1.0 | Aim assistance strength |
| HoldTime | float | 0.0 - 2.0 | Button hold duration (seconds) |
| bSinglePressAlternative | bool | - | Enable tap alternative for hold |
| InputRepeatDelay | float | 0.1 - 1.0 | Delay before repeat (seconds) |
| InputRepeatRate | float | 1.0 - 30.0 | Repeats per second |

### Setting the Config at Runtime

```cpp
// In C++
UGSDAccessibilityManager* Manager = GEngine->GetEngineSubsystem<UGSDAccessibilityManager>();
Manager->SetConfig(MyAccessibilityConfig);
```

```blueprint
// In Blueprint
Get Accessibility Manager -> Set Config (NewConfig)
```

## Best Practices

### UI Design

1. **Always use GetScaledTextSize()** for text sizing in UMG widgets
2. **Check high contrast mode** and adjust colors accordingly
3. **Provide visual feedback** for all audio events when visual cues are enabled
4. **Avoid relying solely on color** to convey information

### Gameplay

1. **Respect reduced motion** by disabling or reducing camera shake, screen shake, and particle effects
2. **Scale screen shake** using GetScreenShakeIntensity()
3. **Support auto-aim** by scaling aim assist with GetAutoAimStrength()
4. **Provide alternatives** to quick-time events when reduced motion is enabled

### Testing

1. Test with all color blind modes enabled
2. Test with text scaling at min (0.5x) and max (2.0x)
3. Test with reduced motion enabled
4. Test with high contrast mode enabled
5. Ensure visual audio cues cover all important audio events

## Architecture

```
GSD_Core Plugin
|-- DataAssets/
|   |-- GSDAccessibilityConfig.h/cpp - Configuration data asset
|-- Subsystems/
|   |-- GSDAccessibilityManager.h/cpp - Engine subsystem manager
|-- Blueprint/
    |-- GSDAccessibilityLibrary.h/cpp - Blueprint function library
```

### Component Responsibilities

- **GSDAccessibilityConfig**: Stores all accessibility settings, handles save/load
- **GSDAccessibilityManager**: Applies settings, provides accessor functions, broadcasts changes
- **GSDAccessibilityLibrary**: Blueprint-friendly static functions for easy access

## Delegate Events

The accessibility manager broadcasts `OnAccessibilitySettingsChanged` when settings are updated. Bind to this delegate to react to setting changes:

```cpp
void AMyActor::BeginPlay()
{
    Super::BeginPlay();

    UGSDAccessibilityManager* Manager = GEngine->GetEngineSubsystem<UGSDAccessibilityManager>();
    if (Manager)
    {
        Manager->OnAccessibilitySettingsChanged.AddDynamic(this, &AMyActor::OnAccessibilityChanged);
    }
}

void AMyActor::OnAccessibilityChanged()
{
    // Refresh UI, update materials, etc.
}
```

## Color Blind Correction Details

The color blind correction system uses Daltonization algorithms:

1. **Simulation**: Calculate how the color appears to someone with color blindness
2. **Error Detection**: Calculate the difference between original and simulated color
3. **Correction**: Shift the color toward a more distinguishable hue
4. **Blending**: Interpolate between original and corrected based on strength setting

The correction is applied per-pixel and can be used on any color value, including material parameters.
