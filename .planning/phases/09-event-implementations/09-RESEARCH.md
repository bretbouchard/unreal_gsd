# Phase 9: Event Implementations - Research

**Researched:** 2026-02-26
**Domain:** Unreal Engine 5 Event System, Actor Spawning, Navigation Modification, Niagara FX, Audio Components
**Confidence:** HIGH

## Summary

This research covers the implementation of four concrete daily events that modify gameplay in meaningful ways: Construction (lane closures with barricades), Bonfire (local FX and density modifiers), Block Party (safe zones with crowd props), and Zombie Rave (density boost with attractor volumes and audio).

Phase 8 established the event infrastructure (UGSDEventBusSubsystem, UGSDEventSchedulerSubsystem, UGSDDailyEventConfig, UGSDEventModifierConfig). Phase 9 builds concrete event implementations that leverage:
- **GSDCrowdManagerSubsystem** for density modifications (AddDensityModifier/RemoveDensityModifier API)
- **GSDDataLayerManager** for streaming event content via Data Layers
- **Zone Graph** navigation for lane closures (via FZoneGraphTagMask blocking)
- **Niagara** for visual effects (UNiagaraFunctionLibrary::SpawnSystemAtLocation)
- **UAudioComponent** for spatial audio (SpawnSoundAtLocation pattern)

**Primary recommendation:** Each event extends UGSDDailyEventConfig with OnEventStart/OnEventEnd BlueprintNativeEvent implementations. Events spawn actors via GetWorld()->SpawnActor<>, activate Data Layers via GSDDataLayerManager, apply density modifiers via GSDCrowdManagerSubsystem, and manage cleanup by tracking spawned actors for destruction on event end.

## Standard Stack

The established libraries/tools for this domain:

### Core
| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| UGSDDailyEventConfig | Phase 8 | Base class for event definitions | Established in Phase 8, provides OnEventStart/OnEventEnd pattern |
| UGSDEventModifierConfig | Phase 8 | Base class for modifiers | Established in Phase 8, provides Apply/Remove pattern |
| UGSDCrowdManagerSubsystem | Phase 6 | Density modifications | AddDensityModifier/RemoveDensityModifier API ready |
| GSDDataLayerManager | Phase 3 | Data Layer activation | SetDataLayerState for streaming event content |

### Supporting
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| UNiagaraFunctionLibrary | UE5.0+ | Spawn VFX at location | Bonfire, BlockParty, ZombieRave visual effects |
| UAudioComponent | UE5.0+ | Spatial audio playback | ZombieRave audio, ambient sounds |
| UZoneGraphSubsystem | UE5.0+ | Lane-based navigation | Construction lane blocking (via tags) |
| NavModifierVolume | UE5.0+ | Block NavMesh areas | BlockParty safe zones |

### GSD-Specific Integration
| Component | Location | Purpose |
|-----------|----------|---------|
| UGSDEventBusSubsystem | GSD_DailyEvents | Broadcast event state changes |
| UGSDEventSchedulerSubsystem | GSD_DailyEvents | Schedule and manage event lifecycle |
| GSDDensityBoostModifier | GSD_DailyEvents | Concrete density modifier (EVT-09) |
| GSDEventTags | GSD_DailyEvents | FGameplayTag constants for events |

**Installation:**
No additional modules needed. All dependencies already in Build.cs from Phase 8.

### Build.cs Dependencies (Already Configured)
```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core",
    "CoreUObject",
    "Engine",
    "GSD_Core",              // UGSDDataAsset, IGSDSpawnable
    "GSD_CityStreaming",     // GSDDataLayerManager (for Data Layer activation)
    "GSD_Crowds",            // UGSDCrowdManagerSubsystem (for density modifiers)
    "GameplayTags",          // FGameplayTag
    "Niagara",               // UNiagaraFunctionLibrary for VFX
    "AudioMixer",            // UAudioComponent for spatial audio
    "ZoneGraph",             // Zone Graph navigation (optional, for lane blocking)
    "MassEntity"             // Mass Entity (for crowd integration)
});
```

## Architecture Patterns

### Recommended Project Structure
```
Plugins/GSD_DailyEvents/
├── Source/
│   └── GSD_DailyEvents/
│       ├── Public/
│       │   ├── DataAssets/
│       │   │   ├── Events/
│       │   │   │   ├── GSDEventConstructionConfig.h    // Construction event
│       │   │   │   ├── GSDEventBonfireConfig.h         // Bonfire event
│       │   │   │   ├── GSDEventBlockPartyConfig.h      // Block Party event
│       │   │   │   └── GSDEventZombieRaveConfig.h      // Zombie Rave event
│       │   │   ├── GSDDailyEventConfig.h               // Base class (Phase 8)
│       │   │   └── GSDEventModifierConfig.h            // Base modifier (Phase 8)
│       │   ├── Modifiers/
│       │   │   ├── GSDDensityBoostModifier.h           // Done in Phase 8
│       │   │   ├── GSDDensityReduceModifier.h          // NEW: Reduce density
│       │   │   ├── GSDNavigationBlockModifier.h        // NEW: Lane blocking
│       │   │   └── GSDSafeZoneModifier.h               // NEW: Safe zone
│       │   └── Actors/
│       │       ├── GSDEventBarricade.h                 // Construction barricade
│       │       └── GSDEventPropBase.h                  // Base for event props
│       └── Private/
│           ├── DataAssets/
│           ├── Modifiers/
│           └── Actors/
```

### Pattern 1: Event Config with Actor Spawning

**What:** Extend UGSDDailyEventConfig to spawn actors on event start and clean up on event end
**When to use:** Events that require physical world objects (barricades, props, FX actors)

**Implementation Pattern:**
```cpp
// GSDEventConstructionConfig.h
UCLASS(BlueprintType, Category = "GSD|Events")
class GSD_DAILYEVENTS_API UGSDEventConstructionConfig : public UGSDDailyEventConfig
{
    GENERATED_BODY()

public:
    //-- Construction Settings --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Construction")
    int32 NumLanesToClose = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Construction")
    TArray<TSubclassOf<AActor>> BarricadeClasses;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Construction")
    float BarricadeSpacing = 300.0f;

    //-- UGSDDailyEventConfig Interface --
    virtual void OnEventStart_Implementation(UObject* WorldContext, FVector Location, float Intensity) override;
    virtual void OnEventEnd_Implementation(UObject* WorldContext) override;

protected:
    // Track spawned barricades for cleanup
    UPROPERTY()
    TArray<TObjectPtr<AActor>> SpawnedBarricades;

    // Track blocked lanes for unblocking
    TArray<FZoneGraphLaneHandle> BlockedLanes;
};
```

```cpp
// GSDEventConstructionConfig.cpp
void UGSDEventConstructionConfig::OnEventStart_Implementation(UObject* WorldContext, FVector Location, float Intensity)
{
    UWorld* World = WorldContext->GetWorld();
    if (!World || BarricadeClasses.Num() == 0) return;

    // Spawn barricades along lane
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    int32 NumBarricades = FMath::CeilToInt(Intensity * 5.0f); // More barricades at higher intensity

    for (int32 i = 0; i < NumBarricades; ++i)
    {
        FVector SpawnLocation = Location + FVector(i * BarricadeSpacing, 0.0f, 0.0f);
        TSubclassOf<AActor> BarricadeClass = BarricadeClasses[i % BarricadeClasses.Num()];

        AActor* Barricade = World->SpawnActor<AActor>(BarricadeClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
        if (Barricade)
        {
            SpawnedBarricades.Add(Barricade);
        }
    }

    // Activate construction Data Layer if specified
    if (RequiredDataLayers.Num() > 0)
    {
        if (UGSDDataLayerManager* LayerManager = World->GetSubsystem<UGSDDataLayerManager>())
        {
            for (const FName& LayerName : RequiredDataLayers)
            {
                LayerManager->ActivateEventLayer(LayerName);
            }
        }
    }

    GSD_EVENT_LOG(Log, TEXT("Construction event started: %d barricades spawned"), SpawnedBarricades.Num());
}

void UGSDEventConstructionConfig::OnEventEnd_Implementation(UObject* WorldContext)
{
    UWorld* World = WorldContext->GetWorld();

    // Destroy all spawned barricades
    for (AActor* Barricade : SpawnedBarricades)
    {
        if (Barricade)
        {
            Barricade->Destroy();
        }
    }
    SpawnedBarricades.Empty();

    // Deactivate construction Data Layer
    if (World && RequiredDataLayers.Num() > 0)
    {
        if (UGSDDataLayerManager* LayerManager = World->GetSubsystem<UGSDDataLayerManager>())
        {
            for (const FName& LayerName : RequiredDataLayers)
            {
                LayerManager->DeactivateEventLayer(LayerName);
            }
        }
    }

    // Unblock lanes
    // (Zone Graph lane unblocking would go here)

    GSD_EVENT_LOG(Log, TEXT("Construction event ended: barricades cleared"));
}
```

### Pattern 2: Niagara FX Spawning for Events

**What:** Spawn Niagara systems at runtime for visual effects
**When to use:** Bonfire, BlockParty, ZombieRave visual effects

```cpp
// GSDEventBonfireConfig.h
UCLASS(BlueprintType, Category = "GSD|Events")
class GSD_DAILYEVENTS_API UGSDEventBonfireConfig : public UGSDDailyEventConfig
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
    UNiagaraSystem* BonfireFX;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
    FVector FXScale = FVector(1.0f);

    //-- Modifiers --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifiers")
    UGSDDensityReduceModifier* DensityReducer;

    virtual void OnEventStart_Implementation(UObject* WorldContext, FVector Location, float Intensity) override;
    virtual void OnEventEnd_Implementation(UObject* WorldContext) override;

protected:
    UPROPERTY()
    UNiagaraComponent* SpawnedFXComponent;
};

// GSDEventBonfireConfig.cpp
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

void UGSDEventBonfireConfig::OnEventStart_Implementation(UObject* WorldContext, FVector Location, float Intensity)
{
    // Spawn Niagara FX at location
    if (BonfireFX)
    {
        SpawnedFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            WorldContext,
            BonfireFX,
            Location,
            FRotator::ZeroRotator,
            FXScale * Intensity,
            true,  // bAutoDestroy
            true,  // bAutoActivate
            ENCPoolMethod::None,
            true   // bPreCullCheck
        );

        if (SpawnedFXComponent)
        {
            // Set FX parameters based on intensity
            SpawnedFXComponent->SetNiagaraVariableFloat(FString("Intensity"), Intensity);
        }
    }

    // Apply density reducer modifier
    if (DensityReducer)
    {
        DensityReducer->ApplyModifier(WorldContext, Location, Intensity);
    }

    GSD_EVENT_LOG(Log, TEXT("Bonfire event started at %s"), *Location.ToString());
}

void UGSDEventBonfireConfig::OnEventEnd_Implementation(UObject* WorldContext)
{
    // Deactivate FX (will auto-destroy)
    if (SpawnedFXComponent)
    {
        SpawnedFXComponent->Deactivate();
        SpawnedFXComponent = nullptr;
    }

    // Remove density modifier
    if (DensityReducer)
    {
        DensityReducer->RemoveModifier(WorldContext);
    }

    GSD_EVENT_LOG(Log, TEXT("Bonfire event ended"));
}
```

### Pattern 3: Safe Zone via NavModifierVolume

**What:** Create AI-free safe zones for events
**When to use:** BlockParty event creates safe zone for survivors

**Approach Options:**
1. **NavModifierVolume** - Blocks NavMesh generation in volume
2. **Density Modifier** - Reduces spawn rate to 0 in area
3. **Smart Object exclusion** - AI won't use objects in safe zone

```cpp
// GSDEventBlockPartyConfig.h
UCLASS(BlueprintType, Category = "GSD|Events")
class GSD_DAILYEVENTS_API UGSDEventBlockPartyConfig : public UGSDDailyEventConfig
{
    GENERATED_BODY()

public:
    //-- Safe Zone Settings --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SafeZone")
    float SafeZoneRadius = 1500.0f;

    //-- Crowd Props --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Props")
    TArray<TSubclassOf<AActor>> CrowdProps;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Props")
    int32 MinProps = 5;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Props")
    int32 MaxProps = 15;

    //-- Modifiers --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifiers")
    UGSDSafeZoneModifier* SafeZoneModifier;

    virtual void OnEventStart_Implementation(UObject* WorldContext, FVector Location, float Intensity) override;
    virtual void OnEventEnd_Implementation(UObject* WorldContext) override;

protected:
    UPROPERTY()
    TArray<TObjectPtr<AActor>> SpawnedProps;

    // Store original NavModifierVolume states if dynamically created
    UPROPERTY()
    TArray<TObjectPtr<ANavModifierVolume>> SafeZoneVolumes;
};
```

### Pattern 4: Spatial Audio with Attractor Volume

**What:** Spawn spatial audio that attracts zombies via density boost
**When to use:** ZombieRave event - zombies attracted to loud music

```cpp
// GSDEventZombieRaveConfig.h
UCLASS(BlueprintType, Category = "GSD|Events")
class GSD_DAILYEVENTS_API UGSDEventZombieRaveConfig : public UGSDDailyEventConfig
{
    GENERATED_BODY()

public:
    //-- Audio Settings --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    USoundBase* RaveMusic;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    float AudioRadius = 3000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    float VolumeMultiplier = 1.0f;

    //-- VFX --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
    UNiagaraSystem* RaveFX;

    //-- Modifiers --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifiers")
    UGSDDensityBoostModifier* DensityBooster;

    virtual void OnEventStart_Implementation(UObject* WorldContext, FVector Location, float Intensity) override;
    virtual void OnEventEnd_Implementation(UObject* WorldContext) override;

protected:
    UPROPERTY()
    UAudioComponent* SpawnedAudio;

    UPROPERTY()
    UNiagaraComponent* SpawnedFX;
};

// GSDEventZombieRaveConfig.cpp
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

void UGSDEventZombieRaveConfig::OnEventStart_Implementation(UObject* WorldContext, FVector Location, float Intensity)
{
    UWorld* World = WorldContext->GetWorld();
    if (!World) return;

    // Spawn spatial audio - use SpawnSoundAtLocation for control
    if (RaveMusic)
    {
        SpawnedAudio = UGameplayStatics::SpawnSoundAtLocation(
            World,
            RaveMusic,
            Location,
            FRotator::ZeroRotator,
            VolumeMultiplier * Intensity,
            1.0f,   // Pitch
            0.0f,   // StartTime
            nullptr, // AttenuationSettings (use defaults)
            nullptr, // ConcurrencySettings
            true,    // bAutoDestroy (we'll manage manually)
            true     // bPersistAcrossLevelTransition
        );

        if (SpawnedAudio)
        {
            SpawnedAudio->SetFloatParameter(FName("Radius"), AudioRadius * Intensity);
        }
    }

    // Spawn VFX
    if (RaveFX)
    {
        SpawnedFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            WorldContext,
            RaveFX,
            Location,
            FRotator::ZeroRotator,
            FVector(Intensity),
            true,
            true
        );
    }

    // Apply density boost - attracts zombies
    if (DensityBooster)
    {
        DensityBooster->ApplyModifier(WorldContext, Location, Intensity);
    }

    GSD_EVENT_LOG(Log, TEXT("Zombie Rave event started at %s"), *Location.ToString());
}

void UGSDEventZombieRaveConfig::OnEventEnd_Implementation(UObject* WorldContext)
{
    // Stop and cleanup audio
    if (SpawnedAudio)
    {
        SpawnedAudio->Stop();
        SpawnedAudio->DestroyComponent();
        SpawnedAudio = nullptr;
    }

    // Deactivate FX
    if (SpawnedFX)
    {
        SpawnedFX->Deactivate();
        SpawnedFX = nullptr;
    }

    // Remove density boost
    if (DensityBooster)
    {
        DensityBooster->RemoveModifier(WorldContext);
    }

    GSD_EVENT_LOG(Log, TEXT("Zombie Rave event ended"));
}
```

### Anti-Patterns to Avoid

- **Don't forget to track spawned actors:** Store references in TArray for cleanup on event end
- **Don't skip modifier cleanup:** Always pair Apply with Remove to prevent state leaks
- **Don't spawn infinite actors:** Use Intensity to cap spawns, validate spawn counts
- **Don't block main thread with heavy spawns:** Consider staged spawning for large events
- **Don't ignore Data Layer activation timing:** Layers may take frames to load, handle async

## Don't Hand-Roll

Problems that look simple but have existing solutions:

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Density modifications | Custom spawn rate logic | GSDCrowdManagerSubsystem::AddDensityModifier | Already implemented, tested in Phase 6/8 |
| Data Layer activation | Direct UDataLayerSubsystem calls | GSDDataLayerManager::ActivateEventLayer | Frame-budget aware, staged loading |
| FX spawning | Manual Niagara component creation | UNiagaraFunctionLibrary::SpawnSystemAtLocation | Standard UE5 pattern, handles pooling |
| Spatial audio | Raw USoundWave playback | UGameplayStatics::SpawnSoundAtLocation | Attenuation, concurrency built-in |
| Event lifecycle | Custom timer management | UGSDEventSchedulerSubsystem | Deterministic scheduling already done |

**Key insight:** Phase 8 built the event infrastructure. Phase 9 is about implementing concrete behaviors using existing APIs, not creating new systems.

## Common Pitfalls

### Pitfall 1: Actor Cleanup Memory Leak
**What goes wrong:** Spawned actors not destroyed when event ends, causing memory leaks and visual artifacts
**Why it happens:** Forgetting to track spawned actors, or early returns before cleanup
**How to avoid:**
- Always store spawned actors in TArray member variable
- Use RAII pattern or ensure cleanup in all code paths
- Call Destroy() on actors in OnEventEnd
**Warning signs:** Actors persist after event ends, memory growth over time

### Pitfall 2: Modifier State Leak
**What goes wrong:** Density modifiers not removed, causing permanent spawn rate changes
**Why it happens:** Early return in OnEventEnd, exception during cleanup, missing RemoveModifier call
**How to avoid:**
- Always pair Apply with Remove
- Store applied modifiers in AppliedModifiers array (already in base class)
- Use finally-style cleanup patterns
```cpp
// BAD: Early return without cleanup
void OnEventEnd_Implementation(UObject* WorldContext) override
{
    if (!WorldContext) return;  // Modifier not removed!
    // ... cleanup
}

// GOOD: Always cleanup
void OnEventEnd_Implementation(UObject* WorldContext) override
{
    // Always remove modifiers regardless of context validity
    if (DensityBooster)
    {
        DensityBooster->RemoveModifier(WorldContext);  // Safe even if WorldContext null
    }
}
```
**Warning signs:** Spawn rates permanently altered, density never returns to normal

### Pitfall 3: Data Layer Loading Hitches
**What goes wrong:** Activating Data Layer causes frame hitch (50-200ms)
**Why it happens:** Loading all content synchronously in single frame
**How to avoid:**
- Use GSDDataLayerManager::ActivateLayersStaged for multiple layers
- Pre-load event layers during less critical moments
- Use async activation API when possible
**Warning signs:** Frame drops when event starts, hitch at event spawn

### Pitfall 4: Niagara Component Lifetime Issues
**What goes wrong:** FX persists after event ends, or crashes when accessing destroyed component
**Why it happens:** bAutoDestroy conflicts with manual management, or component pointer not cleared
**How to avoid:**
- Choose one: bAutoDestroy=true (no manual cleanup) OR bAutoDestroy=false (manual Deactivate/Destroy)
- Set pointer to nullptr after cleanup
- Check pointer validity before use
**Warning signs:** FX flickers, crashes in FX-related code

### Pitfall 5: Audio Component Not Stopping
**What goes wrong:** Music/sounds continue playing after event ends
**Why it happens:** UAudioComponent not properly stopped, or wrong component reference
**How to avoid:**
- Always call Stop() before DestroyComponent()
- Store reference in member variable
- Handle case where audio already finished naturally
**Warning signs:** Sound persists, overlapping audio

### Pitfall 6: Zone Graph Lane Blocking Not Reverting
**What goes wrong:** Lanes remain blocked after construction event ends
**Why it happens:** Not storing blocked lanes for unblocking, or tag mask not reverted
**How to avoid:**
- Store BlockedLanes array when blocking
- Restore original tags on OnEventEnd
- Test with AI navigation to verify unblock
**Warning signs:** AI avoids former construction areas, pathfinding issues

## Code Examples

### Complete Construction Event Config

```cpp
// GSDEventConstructionConfig.h
// Source: Phase 9 implementation following Phase 8 patterns

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/GSDDailyEventConfig.h"
#include "GSDEventConstructionConfig.generated.h"

class AActor;

/**
 * Construction event configuration.
 * Spawns barricades and closes navigation lanes.
 *
 * EVT-05: Construction event spawns barricades and closes lanes
 */
UCLASS(BlueprintType, Category = "GSD|Events")
class GSD_DAILYEVENTS_API UGSDEventConstructionConfig : public UGSDDailyEventConfig
{
    GENERATED_BODY()

public:
    UGSDEventConstructionConfig();

    //-- Construction Settings --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Construction")
    int32 NumLanesToClose = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Construction")
    TArray<TSubclassOf<AActor>> BarricadeClasses;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Construction")
    float BarricadeSpacing = 300.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Construction")
    float BarricadeOffsetFromCenter = 500.0f;

    //-- Warning Settings --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warning")
    bool bSpawnWarningSigns = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warning")
    TSubclassOf<AActor> WarningSignClass;

    //-- UGSDDailyEventConfig Interface --
    virtual bool ValidateConfig(FString& OutError) const override;
    virtual void OnEventStart_Implementation(UObject* WorldContext, FVector Location, float Intensity) override;
    virtual void OnEventEnd_Implementation(UObject* WorldContext) override;

protected:
    /** Track spawned barricades for cleanup */
    UPROPERTY()
    TArray<TObjectPtr<AActor>> SpawnedBarricades;

    /** Track spawned warning signs for cleanup */
    UPROPERTY()
    TArray<TObjectPtr<AActor>> SpawnedWarnings;

    /** Store blocked lane handles for unblocking */
    TArray<FZoneGraphLaneHandle> BlockedLanes;

    /** Helper: Spawn barricade line at location */
    void SpawnBarricadeLine(UWorld* World, const FVector& Center, const FVector& Direction, int32 Count);

    /** Helper: Find lanes to block near location */
    void FindAndBlockLanes(UWorld* World, const FVector& Location, int32 NumLanes);
};
```

### Density Reduce Modifier (for Bonfire)

```cpp
// GSDDensityReduceModifier.h
// Source: Phase 9, follows GSDDensityBoostModifier pattern from Phase 8

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/GSDEventModifierConfig.h"
#include "GSDDensityReduceModifier.generated.h"

/**
 * Concrete modifier that decreases spawn density in an area.
 * Used by Bonfire event to create low-density zones.
 *
 * Inverse of GSDDensityBoostModifier - reduces instead of boosts.
 */
UCLASS(BlueprintType, Category = "GSD|Events|Modifiers")
class GSD_DAILYEVENTS_API UGSDDensityReduceModifier : public UGSDEventModifierConfig
{
    GENERATED_BODY()

public:
    UGSDDensityReduceModifier()
    {
        ModifierTag = FGameplayTag::RequestGameplayTag(FName("Event.Modifier.DensityReduce"));
    }

    //-- Density Settings --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Density")
    float DensityMultiplier = 0.25f;  // 25% of normal density

    //-- UGSDEventModifierConfig Interface --
    virtual void ApplyModifier_Implementation(UObject* WorldContext, FVector Center, float IntensityMultiplier) override;
    virtual void RemoveModifier_Implementation(UObject* WorldContext) override;
};
```

### Safe Zone Modifier (for Block Party)

```cpp
// GSDSafeZoneModifier.h
// Source: Phase 9, creates AI-free zones

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/GSDEventModifierConfig.h"
#include "GSDSafeZoneModifier.generated.h"

/**
 * Concrete modifier that creates a safe zone by reducing density to near-zero.
 * Used by BlockParty event to create survivor-safe areas.
 *
 * Uses extremely low multiplier (0.01) to effectively prevent spawns.
 */
UCLASS(BlueprintType, Category = "GSD|Events|Modifiers")
class GSD_DAILYEVENTS_API UGSDSafeZoneModifier : public UGSDEventModifierConfig
{
    GENERATED_BODY()

public:
    UGSDSafeZoneModifier()
    {
        ModifierTag = FGameplayTag::RequestGameplayTag(FName("Event.Modifier.SafeZone"));
        DensityMultiplier = 0.01f;  // Near-zero for effective safe zone
    }

    //-- Safe Zone Settings --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SafeZone")
    float DensityMultiplier = 0.01f;  // 1% spawn rate = effectively blocked

    //-- UGSDEventModifierConfig Interface --
    virtual void ApplyModifier_Implementation(UObject* WorldContext, FVector Center, float IntensityMultiplier) override;
    virtual void RemoveModifier_Implementation(UObject* WorldContext) override;
};
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Hardcoded event types | UGSDDailyEventConfig DataAssets | Phase 8 | Data-driven, extensible |
| Direct subsystem calls | Modifier Apply/Remove pattern | Phase 8 | Clean lifecycle, reversible |
| Spawning in Tick | OnEventStart/End lifecycle | Phase 8 | Deterministic, testable |
| FMath::Rand* for events | FRandomStream via DeterminismManager | Phase 8 | Reproducible runs |

**Deprecated/outdated:**
- Direct UDataLayerSubsystem calls: Use GSDDataLayerManager wrapper
- Custom spawn rate logic: Use GSDCrowdManagerSubsystem::AddDensityModifier
- String-based event types: Use FGameplayTag

## Open Questions

Things that couldn't be fully resolved:

1. **Zone Graph Lane Blocking Implementation**
   - What we know: Zone Graph uses FZoneGraphTagMask for lane properties
   - What's unclear: Exact API to dynamically block/unblock lanes at runtime
   - Recommendation: Research Zone Graph subsystem in Phase 9 implementation, may need custom blocking solution or NavModifierVolume fallback

2. **Data Layer Preloading Strategy**
   - What we know: Data Layer activation can cause hitches
   - What's unclear: Best time to preload event content layers
   - Recommendation: Consider preloading during menu/transition screens, or use async activation

3. **Event Prop Persistence**
   - What we know: Spawned actors need cleanup on event end
   - What's unclear: Should props be saved in save game system
   - Recommendation: For v1, don't persist event props across saves (events regenerate on load)

## Sources

### Primary (HIGH confidence)
- GSD_DailyEvents Plugin Source - Phase 8 implementation (UGSDDailyEventConfig, UGSDEventModifierConfig)
- GSD_Crowds Plugin Source - Phase 6/7 implementation (UGSDCrowdManagerSubsystem)
- GSD_CityStreaming Plugin Source - Phase 3 implementation (GSDDataLayerManager)
- UE5 Niagara Documentation - UNiagaraFunctionLibrary patterns

### Secondary (MEDIUM confidence)
- UE5 GameplayStatics Audio Patterns - SpawnSoundAtLocation
- UE5 NavModifierVolume Documentation - NavMesh blocking
- UE5 Zone Graph Overview - Lane-based navigation concepts

### Tertiary (LOW confidence)
- WebSearch results for Zone Graph lane blocking - Community discussions, unverified APIs
- WebSearch results for Mass Entity attractor volumes - Conceptual patterns

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH - All infrastructure from Phase 8 exists and tested
- Architecture: HIGH - Patterns established in prior phases (DataAsset, Subsystem, Modifier)
- Pitfalls: HIGH - Common patterns from UE5 development and prior GSD phases
- Zone Graph integration: MEDIUM - API unclear, may need fallback approach

**Research date:** 2026-02-26
**Valid until:** 30 days - UE5 patterns stable, GSD-specific patterns may evolve during Phase 9 execution
