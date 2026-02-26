# Phase 8: Event System Core - Research

**Researched:** 2026-02-26
**Domain:** Unreal Engine Event Systems, Gameplay Tags, Deterministic Scheduling
**Confidence:** HIGH

## Summary

This research covers the implementation of a decoupled event system for the GSD Platform with deterministic daily event generation. The system enables time-based event scheduling where the same date+seed always produces the same events, supporting reproducible gameplay runs.

**Key findings:**
- **UGameplayMessageSubsystem** is the recommended UE5 event bus pattern using GameplayTags for routing
- **GSDDeterminismManager** already exists with `EventCategory` predefined for seeded event RNG
- **GameplayTags** provide hierarchical event organization (e.g., `Event.Daily.Construction`)
- **Data Asset pattern** already established via `UGSDDataAsset` base class

**Primary recommendation:** Use UGameplayMessageSubsystem as the event bus backbone, extend GSDDeterminismManager for date-based seed derivation, and define events as UGSDDataAsset subclasses following existing patterns.

## Standard Stack

The established libraries/tools for this domain:

### Core
| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| GameplayMessaging | UE5.0+ | Event bus with tag routing | Official UE5 solution for decoupled messaging |
| GameplayTags | UE5.0+ | Hierarchical event categorization | Built-in tag system, Blueprint-friendly |
| FRandomStream | UE5.0+ | Deterministic seeded RNG | Same seed = same sequence, essential for daily events |

### Supporting
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| UGameInstanceSubsystem | UE5.0+ | Event scheduler persistence | Cross-level event state tracking |
| UWorldSubsystem | UE5.0+ | Active event management | Per-world event instance tracking |
| UPrimaryDataAsset | UE5.0+ | Event definition storage | Editor-configurable event definitions |

### GSD-Specific Integration
| Component | Location | Purpose |
|-----------|----------|---------|
| UGSDDeterminismManager | GSD_Core | Already has `EventCategory` for seeded RNG |
| UGSDDataAsset | GSD_Core | Base class for event definitions |
| LOG_GSD | GSD_Core | Logging pattern with macros |

**Installation:**
No additional modules needed. GameplayMessaging and GameplayTags are engine modules included with UE5.

### Build.cs Dependencies
```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core",
    "CoreUObject",
    "Engine",
    "GSD_Core",              // UGSDDataAsset, UGSDDeterminismManager
    "GameplayTags",          // FGameplayTag, FGameplayTagContainer
    "GameplayMessaging",     // UGameplayMessageSubsystem (optional, or custom)
    "DeveloperSettings"      // For event config settings
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
│       │   │   ├── GSDDailyEventConfig.h       // Base event definition
│       │   │   ├── GSDEventModifierConfig.h    // Density/navigation modifiers
│       │   │   └── GSDEventScheduleConfig.h    // Daily schedule rules
│       │   ├── Subsystems/
│       │   │   ├── GSDEventBusSubsystem.h      // Event bus (world-scoped)
│       │   │   └── GSDEventSchedulerSubsystem.h // Schedule generation (game-instance)
│       │   ├── Tags/
│       │   │   └── GSDEventTags.h              // GameplayTag definitions
│       │   └── GSD_DailyEvents.h
│       └── Private/
│           ├── DataAssets/
│           ├── Subsystems/
│           └── GSD_DailyEvents.cpp
└── Resources/
    └── GSD_DailyEvents.uplugin
```

### Pattern 1: Event Bus via GameplayMessaging

**What:** Use UGameplayMessageSubsystem or custom delegate-based bus with GameplayTag routing
**When to use:** All cross-system event communication

**Option A: UGameplayMessageSubsystem (Native UE5)**
```cpp
// Source: UE5 GameplayMessaging module
// Register listener for specific event tag
FGameplayMessageListenerHandle Handle =
    UGameplayMessageSubsystem::Get(this).RegisterListener(
        EventTag,  // e.g., "Event.Daily.Construction"
        this,
        &UGSDEventBusSubsystem::OnEventMessageReceived
    );

// Broadcast event
FGameplayEventData EventData;
EventData.Location = SpawnLocation;
EventData.IntValue = AffectedLaneCount;

UGameplayMessageSubsystem::Get(this).BroadcastMessage(
    EventTag,
    EventData
);

// Unregister on cleanup
Handle.Unregister();
```

**Option B: Custom Delegate Bus (More Control)**
```cpp
// Custom event bus following GSD patterns
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnGSDEvent,
    FGameplayTag /*EventTag*/,
    const FVector& /*Location*/,
    float /*Intensity*/);

UCLASS()
class GSDEventBusSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // Subscribe to events matching tag (supports wildcard)
    FDelegateHandle Subscribe(FGameplayTag EventTag,
                              FOnGSDEvent::FDelegate&& Delegate);

    // Broadcast event to all matching subscribers
    void BroadcastEvent(FGameplayTag EventTag,
                        FVector Location,
                        float Intensity);

    // Unsubscribe via handle
    void Unsubscribe(FDelegateHandle Handle);

private:
    TMap<FGameplayTag, FOnGSDEvent> EventDelegates;
};
```

**Recommendation:** Start with Option B (custom bus) for more control over event ordering and debugging. UGameplayMessageSubsystem adds complexity for simple use cases.

### Pattern 2: Gameplay Tags Hierarchy

**What:** Organize events in hierarchical tags for filtering and categorization
**When to use:** All event type definitions

```
Event
├── Daily
│   ├── Construction     // Lane closures, barricades
│   ├── Bonfire          // Local density modifier, FX
│   ├── BlockParty       // Safe zone, crowd props
│   └── ZombieRave       // Density boost, audio
├── Modifier
│   ├── DensityBoost     // Increases spawn density
│   ├── DensityReduce    // Decreases spawn density
│   ├── NavigationBlock  // Closes lanes
│   └── SafeZone         // Disables hostile spawns
└── State
    ├── Active           // Event currently running
    ├── Pending          // Scheduled but not started
    └── Complete         // Event finished
```

**Tag Definition (Native GameplayTags):**
```ini
; In DefaultGameplayTags.ini
[/Script/GameplayTags.GameplayTagsSettings]
+GameplayTagList=(Tag="Event.Daily.Construction",DevComment="Lane closure event")
+GameplayTagList=(Tag="Event.Daily.Bonfire",DevComment="Local density modifier")
+GameplayTagList=(Tag="Event.Modifier.DensityBoost",DevComment="Increases spawn density")
```

**Programmatic Tag Query:**
```cpp
// Check if tag matches hierarchy
bool bIsDailyEvent = EventTag.MatchesTag(FGameplayTag::RequestGameplayTag("Event.Daily"));

// Container for multiple modifiers
FGameplayTagContainer Modifiers;
Modifiers.AddTag(FGameplayTag::RequestGameplayTag("Event.Modifier.DensityBoost"));
Modifiers.AddTag(FGameplayTag::RequestGameplayTag("Event.Modifier.NavigationBlock"));

// Check if any modifier applies
bool bHasDensityMod = EventConfig->GetModifiers().HasAny(Modifiers);
```

### Pattern 3: Deterministic Daily Schedule Generation

**What:** Generate event schedules from date+seed using FRandomStream
**When to use:** Daily event scheduling, reproducible runs

```cpp
// Date-to-seed derivation
int32 DateToSeed(FDateTime Date)
{
    // Convert date to integer: 20250115 for Jan 15, 2025
    return Date.GetYear() * 10000 + Date.GetMonth() * 100 + Date.GetDay();
}

// Schedule generation using GSDDeterminismManager
void UGSDEventSchedulerSubsystem::GenerateDailySchedule(FDateTime Date, int32 WorldSeed)
{
    UGSDDeterminismManager* DeterminismMgr =
        GetGameInstance()->GetSubsystem<UGSDDeterminismManager>();

    // Combine date and world seed for daily seed
    int32 DailySeed = DateToSeed(Date) ^ WorldSeed;
    DeterminismMgr->InitializeWithSeed(DailySeed);

    // Get event-specific stream (already defined in GSDDeterminismManager!)
    FRandomStream& EventStream = DeterminismMgr->GetStream(UGSDDeterminismManager::EventCategory);

    // Select events for today
    TArray<UGSDDailyEventConfig*> AvailableEvents = LoadAvailableEvents();
    int32 NumEventsToday = EventStream.RandRange(2, 5);  // 2-5 events per day

    for (int32 i = 0; i < NumEventsToday && AvailableEvents.Num() > 0; ++i)
    {
        int32 EventIndex = EventStream.RandHelper(AvailableEvents.Num());
        UGSDDailyEventConfig* SelectedEvent = AvailableEvents[EventIndex];

        // Random time slot (8:00 - 22:00)
        int32 Hour = EventStream.RandRange(8, 22);
        FDateTime EventTime = Date + FTimespan(Hour, 0, 0);

        // Random location in valid spawn areas
        FVector EventLocation = GetRandomEventLocation(EventStream);

        ScheduleEvent(SelectedEvent, EventTime, EventLocation);
        AvailableEvents.RemoveAtSwap(EventIndex);  // No duplicates
    }
}
```

### Pattern 4: Event Definition Data Assets

**What:** Define events as UGSDDataAsset subclasses for editor configuration
**When to use:** All event type definitions

```cpp
// Base class for daily events
UCLASS(Abstract, BlueprintType)
class GSDDailyEventConfig : public UGSDDataAsset
{
    GENERATED_BODY()

public:
    //-- Event Identity --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Event")
    FGameplayTag EventTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Event")
    FText DisplayName;

    //-- Timing --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timing")
    float DurationMinutes = 60.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timing")
    TArray<FName> ValidTimeSlots;  // "Morning", "Afternoon", "Evening"

    //-- Modifiers --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifiers")
    TArray<TSoftObjectPtr<UGSDEventModifierConfig>> Modifiers;

    //-- Spawn Requirements --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
    TArray<FName> RequiredDataLayers;  // Data layers to activate

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
    TArray<TSoftObjectPtr<UWorld>> SpawnLevelPackages;

    //-- Validation --
    virtual bool ValidateConfig(FString& OutError) const override;
};

// Concrete event example
UCLASS(BlueprintType)
class UGSDEventConstructionConfig : public UGSDDailyEventConfig
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Construction")
    int32 NumLanesToClose = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Construction")
    TArray<TSoftObjectPtr<AActor>> BarricadePrefabs;
};
```

### Pattern 5: Event Modifier System

**What:** Define reusable modifiers that affect gameplay systems
**When to use:** Events that affect spawn density, navigation, etc.

```cpp
UCLASS(Abstract, BlueprintType)
class UGSDEventModifierConfig : public UGSDDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
    FGameplayTag ModifierTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
    float Radius = 1000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
    float Intensity = 1.0f;

    // Apply modifier to target system
    UFUNCTION(BlueprintNativeEvent, Category = "Modifier")
    void ApplyModifier(UObject* WorldContext, FVector Center, float Intensity);
    virtual void ApplyModifier_Implementation(UObject* WorldContext, FVector Center, float Intensity);

    // Remove modifier
    UFUNCTION(BlueprintNativeEvent, Category = "Modifier")
    void RemoveModifier(UObject* WorldContext);
    virtual void RemoveModifier_Implementation(UObject* WorldContext);
};

// Concrete modifier: Density boost
UCLASS(BlueprintType)
class UGSDEventModifierDensityBoost : public UGSDEventModifierConfig
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Density")
    float DensityMultiplier = 2.0f;

    virtual void ApplyModifier_Implementation(UObject* WorldContext, FVector Center, float Intensity) override
    {
        if (UGSDCrowdManagerSubsystem* CrowdMgr = UGSDCrowdManagerSubsystem::Get(WorldContext))
        {
            CrowdMgr->AddDensityModifier(ModifierTag, Center, Radius, DensityMultiplier * Intensity);
        }
    }

    virtual void RemoveModifier_Implementation(UObject* WorldContext) override
    {
        if (UGSDCrowdManagerSubsystem* CrowdMgr = UGSDCrowdManagerSubsystem::Get(WorldContext))
        {
            CrowdMgr->RemoveDensityModifier(ModifierTag);
        }
    }
};
```

### Anti-Patterns to Avoid

- **Don't use Tick for event checking:** Use timers or scheduled callbacks instead
- **Don't hardcode event types:** Use GameplayTags and Data Assets for extensibility
- **Don't store event state in actors:** Use subsystems for persistence across level loads
- **Don't ignore event ordering:** Document and enforce order when events interact
- **Don't skip validation:** Always validate event configs before scheduling

## Don't Hand-Roll

Problems that look simple but have existing solutions:

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Random event selection | Custom RNG | GSDDeterminismManager::EventCategory | Already exists, category-isolated |
| Event config storage | Custom serialization | UGSDDataAsset | Migration support, editor integration |
| Event type identification | String/enum | FGameplayTag | Hierarchical, Blueprint-friendly |
| Cross-system messaging | Interface calls | Event bus with delegates | Decoupled, extensible |
| Date-to-seed conversion | Custom hash | DateToSeed pattern | Simple, reproducible |

**Key insight:** GSDDeterminismManager already has `EventCategory` defined, specifically for event-related seeded RNG. Do not create a separate determinism system.

## Common Pitfalls

### Pitfall 1: Event Ordering Dependencies
**What goes wrong:** Events that depend on each other fire in wrong order, causing missing data or null references
**Why it happens:** Event broadcast order is not guaranteed, especially with multiple listeners
**How to avoid:**
- Document event dependencies explicitly
- Use priority system for event execution
- Consider event phases (Setup -> Execute -> Cleanup)
**Warning signs:** Events referencing other events' data, intermittent null crashes

### Pitfall 2: Delegate Binding Lifetime Issues
**What goes wrong:** Listeners don't receive events or crash when handling them
**Why it happens:** Delegate handles not stored, or listeners destroyed without unregistering
**How to avoid:**
- Store FDelegateHandle as member variable
- Always unregister in destructor or EndPlay
- Use weak pointers for delegate binding
```cpp
// BAD: Handle not stored
EventBus->Subscribe(EventTag, FOnGSDEvent::FDelegate::CreateLambda([](...){}));

// GOOD: Handle stored for cleanup
EventHandle = EventBus->Subscribe(EventTag,
    FOnGSDEvent::FDelegate::CreateUObject(this, &ThisClass::OnEvent));
```
**Warning signs:** Events not received, crashes during event handling

### Pitfall 3: Non-Deterministic Date Handling
**What goes wrong:** Same date produces different events across runs
**Why it happens:** Using system time, uninitialized RNG, or multiple seed sources
**How to avoid:**
- Always use GSDDeterminismManager for event RNG
- Derive daily seed consistently (DateToSeed pattern)
- Never use FMath::Rand* for event generation
**Warning signs:** Events differ between runs with same seed

### Pitfall 4: Event State Not Persisting
**What goes wrong:** Events reset on level transition or save/load
**Why it happens:** Event state stored in world-scoped subsystems that get recreated
**How to avoid:**
- Use UGameInstanceSubsystem for schedule persistence
- Save/restore active event state via save game system
- Separate schedule (instance) from active events (world)
**Warning signs:** Events disappear on level change, daily schedule resets

### Pitfall 5: Gameplay Tag String Literals
**What goes wrong:** Typos in tag names cause silent failures
**Why it happens:** FGameplayTag::RequestGameplayTag(FName) accepts any string
**How to avoid:**
- Define tags in DefaultGameplayTags.ini
- Use const/static FGameplayTag variables for common tags
- Validate tags at startup
```cpp
// BAD: String literal, typo-prone
FGameplayTag Tag = FGameplayTag::RequestGameplayTag("Event.Daily.Constructin");

// GOOD: Defined constant
namespace GSDEventTags
{
    const FGameplayTag DailyConstruction = FGameplayTag::RequestGameplayTag("Event.Daily.Construction");
}
```
**Warning signs:** Events not matching, tags returning invalid

### Pitfall 6: Deep Callstacks from Event Chains
**What goes wrong:** Events triggering other events cause stack overflows or performance issues
**Why it happens:** No depth limiting, circular event dependencies
**How to avoid:**
- Limit event recursion depth (max 3-4 levels)
- Log warning on deep chains
- Consider queued event processing for chains
**Warning signs:** Stack overflow, frame hitches during event storms

## Code Examples

### Complete Event Bus Subsystem

```cpp
// GSDEventBusSubsystem.h
// Source: GSD pattern following GSDVehicleSpawnerSubsystem/GSDCrowdManagerSubsystem

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "GSDEventBusSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnGSDEvent,
    FGameplayTag /*EventTag*/,
    const FVector& /*Location*/,
    float /*Intensity*/);

/** Handle for event listener cleanup */
USTRUCT(BlueprintType)
struct FGSD_EVENT_HANDLE
{
    GENERATED_BODY()

    FDelegateHandle DelegateHandle;
    FGameplayTag SubscribedTag;
    bool IsValid() const { return DelegateHandle.IsValid(); }
};

/**
 * World subsystem for event broadcasting and listening.
 * Uses GameplayTags for event routing with wildcard support.
 */
UCLASS()
class GSD_DAILYEVENTS_API UGSDEventBusSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    /**
     * Subscribe to events matching a tag.
     * Supports hierarchical matching (Event.Daily matches Event.Daily.Construction).
     */
    FGSD_EVENT_HANDLE Subscribe(FGameplayTag EventTag, FOnGSDEvent::FDelegate&& Delegate);

    /**
     * Broadcast event to all matching subscribers.
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Events")
    void BroadcastEvent(FGameplayTag EventTag, FVector Location, float Intensity = 1.0f);

    /**
     * Unsubscribe via handle.
     */
    void Unsubscribe(FGSD_EVENT_HANDLE& Handle);

    //-- Queries --

    /** Get active event count */
    UFUNCTION(BlueprintPure, Category = "GSD|Events")
    int32 GetActiveEventCount() const { return ActiveEvents.Num(); }

protected:
    // Map of tag -> delegate multicast
    TMap<FGameplayTag, FOnGSDEvent> EventDelegates;

    // Currently active events
    UPROPERTY()
    TArray<FGameplayTag> ActiveEvents;

    // ~UWorldSubsystem interface
    virtual bool ShouldCreateSubsystem(UWorld* World) const override;
};
```

```cpp
// GSDEventBusSubsystem.cpp

#include "Subsystems/GSDEventBusSubsystem.h"
#include "GSDLog.h"

FGSD_EVENT_HANDLE UGSDEventBusSubsystem::Subscribe(FGameplayTag EventTag, FOnGSDEvent::FDelegate&& Delegate)
{
    FGSD_EVENT_HANDLE Handle;
    Handle.SubscribedTag = EventTag;
    Handle.DelegateHandle = EventDelegates.FindOrAdd(EventTag).Add(MoveTemp(Delegate));

    GSD_TRACE(TEXT("Subscribed to event tag: %s"), *EventTag.ToString());
    return Handle;
}

void UGSDEventBusSubsystem::BroadcastEvent(FGameplayTag EventTag, FVector Location, float Intensity)
{
    GSD_LOG(Log, TEXT("Broadcasting event: %s at %s with intensity %.2f"),
        *EventTag.ToString(), *Location.ToString(), Intensity);

    // Find exact match
    if (FOnGSDEvent* Delegate = EventDelegates.Find(EventTag))
    {
        Delegate->Broadcast(EventTag, Location, Intensity);
    }

    // Also broadcast to parent tags (hierarchical matching)
    // Event.Daily.Construction also triggers Event.Daily listeners
    for (auto& Pair : EventDelegates)
    {
        if (EventTag.MatchesTag(Pair.Key) && Pair.Key != EventTag)
        {
            Pair.Value.Broadcast(EventTag, Location, Intensity);
        }
    }

    ActiveEvents.AddUnique(EventTag);
}

void UGSDEventBusSubsystem::Unsubscribe(FGSD_EVENT_HANDLE& Handle)
{
    if (!Handle.IsValid())
    {
        return;
    }

    if (FOnGSDEvent* Delegate = EventDelegates.Find(Handle.SubscribedTag))
    {
        Delegate->Remove(Handle.DelegateHandle);
    }

    Handle.DelegateHandle.Reset();
}

bool UGSDEventBusSubsystem::ShouldCreateSubsystem(UWorld* World) const
{
    // Only create in game worlds, not editor preview
    return World->IsGameWorld();
}
```

### Event Scheduler with Determinism

```cpp
// GSDEventSchedulerSubsystem.h
// Uses GSDDeterminismManager::EventCategory for seeded RNG

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "GSDEventSchedulerSubsystem.generated.h"

class UGSDDailyEventConfig;
class UGSDDeterminismManager;

/** Scheduled event instance */
USTRUCT()
struct FGSDEventInstance
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<UGSDDailyEventConfig> EventConfig;

    FGameplayTag EventTag;
    FDateTime ScheduledTime;
    FVector Location;
    float Intensity;
    bool bIsActive = false;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnEventScheduled, const FGSDEventInstance&, Event);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnEventStarted, const FGSDEventInstance&, Event);

/**
 * Game instance subsystem for deterministic daily event scheduling.
 * Persists across level loads.
 */
UCLASS()
class GSD_DAILYEVENTS_API UGSDEventSchedulerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    //-- Schedule Generation --

    /** Generate deterministic schedule for a specific date */
    UFUNCTION(BlueprintCallable, Category = "GSD|Events")
    void GenerateDailySchedule(FDateTime Date, int32 WorldSeed);

    /** Get events scheduled for a date */
    UFUNCTION(BlueprintPure, Category = "GSD|Events")
    void GetEventsForDate(FDateTime Date, TArray<FGSDEventInstance>& OutEvents) const;

    //-- Runtime Control --

    /** Start scheduled event */
    void StartEvent(const FGSDEventInstance& Event);

    /** End active event */
    void EndEvent(FGameplayTag EventTag);

    //-- Delegates --

    FOnEventStarted& GetOnEventStarted() { return OnEventStarted; }

protected:
    //-- State --
    UPROPERTY()
    TArray<FGSDEventInstance> ScheduledEvents;

    UPROPERTY()
    TArray<FGSDEventInstance> ActiveEvents;

    //-- Delegates --
    FOnEventStarted OnEventStarted;

    //-- Helpers --
    int32 DateToSeed(FDateTime Date) const;
    FVector GetRandomEventLocation(FRandomStream& Stream) const;
    TArray<UGSDDailyEventConfig*> LoadAvailableEvents() const;
};
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| String-based event types | GameplayTags | UE4.22+ | Type-safe, hierarchical, Blueprint-friendly |
| Custom event systems | UGameplayMessageSubsystem | UE5.0 | Official solution, standardized |
| FMath::Rand* | FRandomStream with categories | GSD Phase 1 | Deterministic, reproducible |
| Actor-based event state | Subsystem-based | UE5.0+ | Persists across levels, cleaner lifecycle |

**Deprecated/outdated:**
- Interface-based event systems: Use delegate buses with GameplayTags instead
- Actor components for event state: Use world/game-instance subsystems
- Enum-based event types: Use FGameplayTag for extensibility

## Open Questions

Things that couldn't be fully resolved:

1. **UGameplayMessageSubsystem vs Custom Bus**
   - What we know: UGameplayMessageSubsystem is official UE5 solution
   - What's unclear: Performance overhead for high-frequency events
   - Recommendation: Start with custom bus (Pattern 1B), evaluate UGameplayMessageSubsystem if needed

2. **Event Save/Restore Strategy**
   - What we know: Phase 10 (Telemetry) will handle save system
   - What's unclear: Exact serialization format for active events
   - Recommendation: Use FGSDEventInstance struct serialization, defer complex save logic to Phase 10

3. **Cross-Plugin Event Dependencies**
   - What we know: Events need to affect CrowdManager, VehicleSpawner, DataLayers
   - What's unclear: Registration order for event listeners
   - Recommendation: Use module loading phases (PostEngineInit) to ensure GSD_Core loads first

## Sources

### Primary (HIGH confidence)
- GSD Plugin Source Code - Established patterns from GSD_Core, GSD_Vehicles, GSD_Crowds
- GSDDeterminismManager.h - Existing seeded RNG with EventCategory
- UGSDDataAsset.h - Base class for event definitions
- UE5 GameplayTags Module - Official documentation

### Secondary (MEDIUM confidence)
- UE5 Community Forums - GameplayMessaging best practices
- Unreal Source Code - UGameplayMessageSubsystem implementation reference

### Tertiary (LOW confidence)
- WebSearch results for UE5 event patterns - Community blog posts, unverified

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH - GSDDeterminismManager exists, GameplayTags is official
- Architecture: HIGH - Patterns established in prior phases (DataAsset, Subsystem)
- Pitfalls: MEDIUM - Common patterns documented, project-specific issues may emerge

**Research date:** 2026-02-26
**Valid until:** 30 days - UE5 patterns stable, GSD-specific patterns may evolve during Phase 8-10 execution
