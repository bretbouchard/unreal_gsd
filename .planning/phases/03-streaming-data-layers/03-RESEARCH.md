# Phase 3 Research: Streaming & Data Layers

**Phase Goal:** Streaming behavior is predictive and runtime-configurable via Data Layers

**Requirements:** WP-03, WP-05, WP-06, WP-08

**Research Date:** 2026-02-25

---

## Summary

Phase 3 builds on the World Partition foundation from Phase 2 to add runtime Data Layer control and streaming telemetry. The key insight is that **GSDStreamingSourceComponent already exists** with predictive loading support - Phase 3 focuses on:

1. **Runtime Data Layer toggling** - Allow players/GM to toggle event layers
2. **Streaming telemetry** - Track cell load times for performance monitoring
3. **Vehicle streaming integration** - Ensure vehicles use the streaming source correctly

The primary new systems are:
- **UDataLayerSubsystem** - C++ API for runtime Data Layer control
- **UWorldPartitionSubsystem** - Streaming progress and telemetry hooks
- **Data Layer Assets** - Organize content into toggleable layers

---

## Standard Stack

### Core UE5 Systems (Use These)

| System | Purpose | Documentation |
|--------|---------|---------------|
| **UDataLayerSubsystem** | Runtime Data Layer state management | Engine API, well-documented |
| **UWorldPartitionSubsystem** | Streaming progress, source management | Engine API |
| **UWorldPartitionStreamingSourceComponent** | Base class for streaming sources | Already extended in Phase 2 |
| **Data Layer Assets** | Define layers in editor | Editor workflow |
| **OnWorldPartitionStreamingProgressUpdated** | Delegate for streaming telemetry | Engine delegate |

### Data Layer Types

| Type | Purpose | Runtime Control |
|------|---------|-----------------|
| **Editor Data Layer** | Organization only | Not toggleable at runtime |
| **Runtime Data Layer** | Gameplay-controllable | Yes - via UDataLayerSubsystem |

### Runtime Data Layer States

```
Unloaded -> Loaded -> Activated
    ^          |          |
    |----------+----------+
```

- **Unloaded**: Actor not in memory
- **Loaded**: Actor in memory but not ticking
- **Activated**: Actor fully active (default for most gameplay)

---

## Architecture Patterns

### Pattern 1: Data Layer Organization

```
GSDCityLevel (World)
├── DL_BaseCity (Runtime) - Always activated
│   ├── Buildings
│   ├── Roads
│   └── Terrain
├── DL_Events (Runtime) - Toggled by event system
│   ├── Bonfire locations
│   ├── Block party zones
│   └── Construction sites
├── DL_Construction (Runtime) - Toggled by event system
│   └── Barricades, road closures
└── DL_Parties (Runtime) - Toggled by event system
    └── Party props, crowd extras
```

### Pattern 2: Streaming Source on Entities

```
Player Pawn
└── GSDStreamingSourceComponent (always active, predictive loading)

Vehicle Pawn
└── GSDStreamingSourceComponent (active when driven, disabled when parked)
    - SetStreamingEnabled(false) when vehicle parks
    - SetLoadingRangeMultiplier(2.0) for fast vehicles
```

### Pattern 3: Telemetry Hook

```
GameInstance or Subsystem
└── Bind to OnWorldPartitionStreamingProgressUpdated
    └── Log cell load times to GSD telemetry system
```

---

## Don't Hand-Roll

### Use Built-in Data Layers (Not Custom)

| Don't Build | Use Instead | Why |
|-------------|-------------|-----|
| Custom layer toggle system | UDataLayerSubsystem | Engine handles streaming, replication |
| Custom streaming sources | UWorldPartitionStreamingSourceComponent | Already integrated with WP |
| Custom cell tracking | UWorldPartitionSubsystem | Has built-in progress delegates |

### Data Layer Naming Convention

Use the established GSD prefix:
```
DL_BaseCity      // Base city layer
DL_Events        // Dynamic event content
DL_Construction  // Construction/barricades
DL_Parties       // Party/block party content
```

---

## Common Pitfalls

### 1. Loading Too Many Layers Simultaneously

**Problem:** Activating all layers at once causes massive hitch

**Solution:** Layer activation should be staged or event-driven
```cpp
// Bad: Load everything
for (auto& Layer : AllLayers) {
    Subsystem->SetDataLayerInstanceRuntimeState(Layer, EDataLayerRuntimeState::Activated);
}

// Good: Load based on gameplay context
if (CurrentEvent == EEventType::Construction) {
    Subsystem->SetDataLayerInstanceRuntimeState(ConstructionLayer, EDataLayerRuntimeState::Activated);
}
```

### 2. Forgetting to Disable Streaming for Parked Vehicles

**Problem:** 100+ parked vehicles all acting as streaming sources = performance hit

**Solution:** Use SetStreamingEnabled(false) when vehicle parks
```cpp
void AVehicle::OnPark() {
    if (UGSDStreamingSourceComponent* Source = FindComponentByClass<UGSDStreamingSourceComponent>()) {
        Source->SetStreamingEnabled(false);
    }
}
```

### 3. Not Using Predictive Loading for Fast Vehicles

**Problem:** Pop-in during high-speed driving

**Solution:** Enable predictive loading and adjust range multiplier
```cpp
void AVehicle::OnStartDriving() {
    if (UGSDStreamingSourceComponent* Source = FindComponentByClass<UGSDStreamingSourceComponent>()) {
        Source->SetStreamingEnabled(true);
        if (IsFastVehicle()) {
            Source->SetLoadingRangeMultiplier(2.0f);
        }
    }
}
```

### 4. Telemetry Without Context

**Problem:** Raw load times don't help debug issues

**Solution:** Tag telemetry with cell location, player position, active layers
```cpp
struct FStreamingTelemetryEvent {
    FString CellName;
    float LoadTimeMs;
    FVector PlayerPosition;
    TArray<FString> ActiveLayers;
    float PlayerSpeed;
};
```

---

## Code Examples

### C++ Data Layer Toggle

```cpp
// GSDDataLayerManager.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataLayer.h"
#include "GSDDataLayerManager.generated.h"

UCLASS()
class GSD_CITYSTREAMING_API UGSDDataLayerManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // Toggle a runtime data layer by name
    UFUNCTION(BlueprintCallable, Category = "GSD|DataLayers")
    void SetDataLayerState(FName LayerName, bool bActivated);

    // Check if a layer is currently activated
    UFUNCTION(BlueprintPure, Category = "GSD|DataLayers")
    bool IsDataLayerActivated(FName LayerName) const;

    // Get all runtime data layer names
    UFUNCTION(BlueprintPure, Category = "GSD|DataLayers")
    TArray<FName> GetRuntimeDataLayerNames() const;

private:
    UDataLayerSubsystem* GetDataLayerSubsystem() const;
};
```

```cpp
// GSDDataLayerManager.cpp
#include "Subsystems/DataLayerSubsystem.h"
#include "Engine/World.h"

void UGSDDataLayerManager::SetDataLayerState(FName LayerName, bool bActivated)
{
    if (UDataLayerSubsystem* DLSubsystem = GetDataLayerSubsystem())
    {
        if (UDataLayer* DataLayer = DLSubsystem->GetDataLayerInstanceFromName(LayerName))
        {
            EDataLayerRuntimeState NewState = bActivated
                ? EDataLayerRuntimeState::Activated
                : EDataLayerRuntimeState::Unloaded;

            DLSubsystem->SetDataLayerInstanceRuntimeState(DataLayer, NewState);
        }
    }
}

bool UGSDDataLayerManager::IsDataLayerActivated(FName LayerName) const
{
    if (UDataLayerSubsystem* DLSubsystem = GetDataLayerSubsystem())
    {
        if (UDataLayer* DataLayer = DLSubsystem->GetDataLayerInstanceFromName(LayerName))
        {
            return DLSubsystem->GetDataLayerInstanceRuntimeState(DataLayer) == EDataLayerRuntimeState::Activated;
        }
    }
    return false;
}

TArray<FName> UGSDDataLayerManager::GetRuntimeDataLayerNames() const
{
    TArray<FName> Names;
    if (UDataLayerSubsystem* DLSubsystem = GetDataLayerSubsystem())
    {
        for (const UDataLayer* Layer : DLSubsystem->GetDataLayerInstances())
        {
            if (Layer->IsRuntime())
            {
                Names.Add(Layer->GetFName());
            }
        }
    }
    return Names;
}

UDataLayerSubsystem* UGSDDataLayerManager::GetDataLayerSubsystem() const
{
    return GetWorld()->GetSubsystem<UDataLayerSubsystem>();
}
```

### Streaming Telemetry Hook

```cpp
// GSDStreamingTelemetry.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GSDStreamingTelemetry.generated.h"

USTRUCT(BlueprintType)
struct FGSDStreamingEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString CellName;

    UPROPERTY(BlueprintReadOnly)
    float LoadTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    FVector PlayerPosition = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    float Timestamp = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStreamingEventLogged, const FGSDStreamingEvent&, Event);

UCLASS()
class GSD_CITYSTREAMING_API UGSDStreamingTelemetry : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Broadcast when a streaming event is logged
    UPROPERTY(BlueprintAssignable, Category = "GSD|Telemetry")
    FOnStreamingEventLogged OnStreamingEventLogged;

    // Get recent streaming events
    UFUNCTION(BlueprintPure, Category = "GSD|Telemetry")
    const TArray<FGSDStreamingEvent>& GetRecentEvents() const { return RecentEvents; }

    // Get average load time
    UFUNCTION(BlueprintPure, Category = "GSD|Telemetry")
    float GetAverageLoadTimeMs() const;

private:
    void OnStreamingProgressUpdated(UWorld* World);

    FDelegateHandle ProgressDelegateHandle;
    TArray<FGSDStreamingEvent> RecentEvents;
    static constexpr int32 MaxRecentEvents = 100;
};
```

```cpp
// GSDStreamingTelemetry.cpp
#include "GSDStreamingTelemetry.h"
#include "Engine/World.h"
#include "WorldPartition/WorldPartitionSubsystem.h"

void UGSDStreamingTelemetry::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Bind to world partition streaming progress
    if (UWorld* World = GetWorld())
    {
        if (UWorldPartitionSubsystem* WPSubsystem = World->GetSubsystem<UWorldPartitionSubsystem>())
        {
            // Note: Actual delegate binding depends on UE5 version
            // This is the conceptual pattern
            ProgressDelegateHandle = WPSubsystem->OnWorldPartitionStreamingProgressUpdated.AddUObject(
                this, &UGSDStreamingTelemetry::OnStreamingProgressUpdated);
        }
    }
}

void UGSDStreamingTelemetry::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        if (UWorldPartitionSubsystem* WPSubsystem = World->GetSubsystem<UWorldPartitionSubsystem>())
        {
            WPSubsystem->OnWorldPartitionStreamingProgressUpdated.Remove(ProgressDelegateHandle);
        }
    }

    Super::Deinitialize();
}

void UGSDStreamingTelemetry::OnStreamingProgressUpdated(UWorld* World)
{
    // Log streaming event with context
    FGSDStreamingEvent Event;
    Event.Timestamp = GetWorld()->GetTimeSeconds();

    // Get player position for context
    if (APlayerController* PC = World->GetFirstPlayerController())
    {
        if (APawn* Pawn = PC->GetPawn())
        {
            Event.PlayerPosition = Pawn->GetActorLocation();
        }
    }

    RecentEvents.Add(Event);
    if (RecentEvents.Num() > MaxRecentEvents)
    {
        RecentEvents.RemoveAt(0);
    }

    OnStreamingEventLogged.Broadcast(Event);
}

float UGSDStreamingTelemetry::GetAverageLoadTimeMs() const
{
    if (RecentEvents.Num() == 0) return 0.0f;

    float Total = 0.0f;
    for (const auto& Event : RecentEvents)
    {
        Total += Event.LoadTimeMs;
    }
    return Total / RecentEvents.Num();
}
```

### Vehicle Streaming Source Setup

```cpp
// In vehicle Blueprint or C++ constructor
void AVehicleBase::SetupStreamingSource()
{
    // GSDStreamingSourceComponent should be added in Blueprint
    // or created here in C++

    StreamingSourceComponent = CreateDefaultSubobject<UGSDStreamingSourceComponent>(TEXT("StreamingSource"));

    // Configure for vehicle
    StreamingSourceComponent->bPredictiveLoading = true;
    StreamingSourceComponent->PredictiveLoadingVelocityThreshold = 1000.0f; // 10 m/s

    // Fast vehicles get extended range
    if (bIsFastVehicle)
    {
        StreamingSourceComponent->SetLoadingRangeMultiplier(2.0f);
    }
}

void AVehicleBase::OnVehicleStateChanged(EVehicleState NewState)
{
    if (!StreamingSourceComponent) return;

    switch (NewState)
    {
    case EVehicleState::Driving:
        StreamingSourceComponent->SetStreamingEnabled(true);
        break;
    case EVehicleState::Parked:
        // Disable streaming source for parked vehicles to save performance
        StreamingSourceComponent->SetStreamingEnabled(false);
        break;
    case EVehicleState::Destroyed:
        StreamingSourceComponent->SetStreamingEnabled(false);
        break;
    }
}
```

---

## Console Commands for Testing

| Command | Purpose |
|---------|---------|
| `stat streaming` | Show streaming statistics |
| `wp.Runtime.ToggleDebugDraw` | Visualize streaming sources and cells |
| `wp.Runtime.DrawDataLayers 1` | Visualize Data Layer bounds |
| `DataLayer.List` | List all Data Layers |
| `DataLayer.SetState [Name] [State]` | Manually set layer state |

---

## Recommended Plan Structure

Based on research, Phase 3 should have **4-5 plans**:

### Plan 1: GSDDataLayerManager Subsystem
- Create UGSDDataLayerManager world subsystem
- Implement SetDataLayerState, IsDataLayerActivated
- Blueprint-callable API for runtime layer control
- Unit tests for layer state management

### Plan 2: Data Layer Asset Setup
- Create Data Layer assets (DL_BaseCity, DL_Events, DL_Construction, DL_Parties)
- Document editor workflow for assigning actors to layers
- Verification checklist for layer configuration

### Plan 3: Streaming Telemetry System
- Create UGSDStreamingTelemetry game instance subsystem
- Bind to OnWorldPartitionStreamingProgressUpdated
- Log cell load times with context (player position, active layers)
- Expose telemetry to GSD_Telemetry plugin (Phase 10)

### Plan 4: Vehicle Streaming Integration
- Ensure GSDStreamingSourceComponent is added to vehicle base class
- Implement state-based streaming enable/disable
- Configure predictive loading thresholds for different vehicle types
- Test with 50+ vehicles (spawned/parked/driving)

### Plan 5: Verification and Editor Tests
- Unit tests for DataLayerManager
- Unit tests for StreamingTelemetry
- Editor test for layer toggle functionality
- Performance test with multiple layers active

---

## Dependencies

### From Phase 2 (Already Complete)
- **GSDStreamingSourceComponent** - Predictive loading foundation exists
- **GSD_CityStreaming plugin** - Module structure ready
- **World Partition configuration** - Grid cells, HLOD setup

### Enables Future Phases
- **Phase 4 (Vehicles)** - Streaming source integration pattern
- **Phase 8 (Events)** - Data Layer toggle for event content
- **Phase 9 (Event Implementations)** - Layer activation per event type
- **Phase 10 (Telemetry)** - Streaming telemetry feeds into performance monitoring

---

## Sources

| Topic | Source | Confidence |
|-------|--------|------------|
| Data Layer C++ API | Epic Games Official Documentation | HIGH |
| UDataLayerSubsystem methods | UE5 Source Code (Engine/Source/Runtime/Engine) | HIGH |
| Runtime Data Layer states | Epic Games Documentation | HIGH |
| Streaming telemetry hooks | UWorldPartitionSubsystem API | MEDIUM |
| Predictive loading patterns | UE5 World Partition Documentation | HIGH |
| Console commands | UE5 Console Commands Reference | HIGH |

---

## Next Steps

1. Review this research with the team
2. Create PLAN.md files for each recommended plan
3. Begin execution with Plan 1 (GSDDataLayerManager)
