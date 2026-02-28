# Crowd Debug Input Setup Guide

This guide explains how to set up EnhancedInput for crowd debug controls in the GSD_Crowds plugin.

## Prerequisites

- **EnhancedInput Plugin**: Must be enabled in your project (Edit > Plugins > Built-In > Input > Enhanced Input)
- **GSD_Crowds Plugin**: Must be enabled in your project

## Overview

The GSD_Crowds plugin provides a configurable input system for debug controls using Unreal Engine's EnhancedInput system. This allows designers to:

- Configure key bindings without code changes
- Customize debug controls per-project
- Enable/disable debug input at runtime

## Setup Steps

### Step 1: Create Input Actions

Create Input Actions for each debug control in the Content Browser:

1. Right-click in Content Browser > Input > Input Action
2. Create the following Input Actions:

| Action Name | Description |
|-------------|-------------|
| IA_ToggleDebugWidget | Toggle crowd debug widget visibility |
| IA_IncreaseDensity | Increase crowd density |
| IA_DecreaseDensity | Decrease crowd density |
| IA_ToggleCrowdEnabled | Toggle crowd spawning on/off |
| IA_ResetCrowd | Reset all crowds to initial state |
| IA_SpawnTestCrowd | Spawn test crowd for debugging |
| IA_DespawnAllCrowds | Despawn all crowd entities |

**Input Action Settings:**
- Value Type: Digital (bool) - these are button presses
- No triggers needed (default Started trigger works)

### Step 2: Create Input Mapping Context

Create an Input Mapping Context for debug controls:

1. Right-click in Content Browser > Input > Input Mapping Context
2. Name it: `IMC_CrowdDebug`
3. Add all 7 Input Actions created in Step 1
4. Configure key bindings (see Default Key Bindings below)

### Step 3: Create Input Config DataAsset

Create the Input Config DataAsset:

1. Right-click in Content Browser > Blueprints > Data Asset
2. Select `GSDCrowdInputConfig`
3. Name it: `DA_CrowdInputConfig`
4. Configure the DataAsset:
   - Assign all 7 Input Actions to their respective slots
   - Assign the Input Mapping Context (`IMC_CrowdDebug`)
   - Set Mapping Context Priority: 100 (higher than gameplay controls)

### Step 4: Add Component to Player Controller

Add the `GSDCrowdInputComponent` to your Player Controller:

**In C++:**
```cpp
// In your PlayerController header
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GSD|Crowds")
class UGSDCrowdInputComponent* CrowdInputComponent;

// In your PlayerController constructor
CrowdInputComponent = CreateDefaultSubobject<UGSDCrowdInputComponent>(TEXT("CrowdInputComponent"));
```

**In Blueprint:**
1. Open your Player Controller Blueprint
2. Add Component > Search for "GSDCrowdInputComponent"
3. Name it "CrowdInputComponent"

### Step 5: Initialize Input at Runtime

Initialize the input component with your config:

**In C++:**
```cpp
// In BeginPlay() or appropriate initialization point
if (CrowdInputComponent)
{
    // Load the input config DataAsset
    UGSDCrowdInputConfig* Config = LoadObject<UGSDCrowdInputConfig>(
        this,
        TEXT("/Game/Path/To/DA_CrowdInputConfig")
    );

    if (Config)
    {
        CrowdInputComponent->SetInputConfig(Config);
        CrowdInputComponent->SetDebugEnabled(true); // Enable debug controls
    }
}
```

**In Blueprint:**
1. Get the CrowdInputComponent
2. Call `SetInputConfig` with your DA_CrowdInputConfig
3. Call `SetDebugEnabled(true)`

### Step 6: Bind to Events

Bind to the input component's delegates to handle debug actions:

**In C++:**
```cpp
// Bind to delegates
CrowdInputComponent->OnToggleDebugWidget.AddDynamic(this, &AMyPlayerController::OnToggleDebugWidget);
CrowdInputComponent->OnDensityChanged.AddDynamic(this, &AMyPlayerController::OnDensityChanged);
CrowdInputComponent->OnToggleCrowdEnabled.AddDynamic(this, &AMyPlayerController::OnToggleCrowdEnabled);
CrowdInputComponent->OnResetCrowd.AddDynamic(this, &AMyPlayerController::OnResetCrowd);
CrowdInputComponent->OnSpawnTestCrowd.AddDynamic(this, &AMyPlayerController::OnSpawnTestCrowd);
CrowdInputComponent->OnDespawnAllCrowds.AddDynamic(this, &AMyPlayerController::OnDespawnAllCrowds);
```

**In Blueprint:**
1. Select the CrowdInputComponent
2. In Details panel > Events, click the + button for each event
3. Implement the event logic in the resulting Blueprint graph

## Default Key Bindings

Recommended default key bindings for `IMC_CrowdDebug`:

| Action | Key | Description |
|--------|-----|-------------|
| IA_ToggleDebugWidget | F1 | Toggle debug widget visibility |
| IA_IncreaseDensity | Equals (+) | Increase crowd density |
| IA_DecreaseDensity | Minus (-) | Decrease crowd density |
| IA_ToggleCrowdEnabled | F2 | Toggle crowd spawning |
| IA_ResetCrowd | F3 | Reset all crowds |
| IA_SpawnTestCrowd | F4 | Spawn test crowd |
| IA_DespawnAllCrowds | F5 | Despawn all crowds |

**Note:** These are suggestions. Designers can customize these bindings in the Input Mapping Context.

## Runtime Control

### Enable/Disable Debug Input

```cpp
// Enable debug controls
CrowdInputComponent->SetDebugEnabled(true);

// Disable debug controls (removes mapping context)
CrowdInputComponent->SetDebugEnabled(false);
```

### Change Input Config at Runtime

```cpp
// Switch to different config (removes old bindings, applies new)
UGSDCrowdInputConfig* NewConfig = LoadObject<UGSDCrowdInputConfig>(...);
CrowdInputComponent->SetInputConfig(NewConfig);
```

## Troubleshooting

### Input Not Working

1. **Check EnhancedInput Plugin**: Ensure the plugin is enabled
2. **Check Component Attachment**: Verify component is on PlayerController
3. **Check Config Assignment**: Ensure SetInputConfig() was called with valid asset
4. **Check Debug Enabled**: Ensure SetDebugEnabled(true) was called
5. **Check Mapping Context**: Verify IMC_CrowdDebug has key bindings

### Key Conflicts

If debug keys conflict with gameplay:
- Increase `MappingContextPriority` in DA_CrowdInputConfig (default: 100)
- Or change key bindings in IMC_CrowdDebug to avoid conflicts

### No Events Firing

1. Check that delegates are bound (Blueprint events connected or C++ AddDynamic called)
2. Verify input actions are triggered (check EnhancedInput debug visualization)
3. Ensure owning PlayerController is possessing a pawn

## Architecture Notes

### Component Lifecycle

1. **BeginPlay**: Caches PlayerController and EnhancedInputComponent
2. **SetDebugEnabled(true)**: Binds actions and adds mapping context
3. **SetDebugEnabled(false)**: Removes mapping context and unbinds actions
4. **EndPlay**: Cleanup - removes mapping context and unbinds actions

### Thread Safety

- Input component operates on game thread only
- No thread safety concerns (all EnhancedInput operations are game thread)

### Performance

- Component has no tick (bCanEverTick = false)
- Only processes input when debug enabled
- Mapping context add/remove is lightweight

## See Also

- [GSD_Crowds Plugin Documentation](../README.md)
- [Crowd Manager Subsystem](../Source/GSD_Crowds/Public/Subsystems/GSDCrowdManagerSubsystem.h)
- [Crowd Debug Widget](../Source/GSD_Crowds/Public/Widgets/GSDCrowdDebugWidget.h) (if applicable)
