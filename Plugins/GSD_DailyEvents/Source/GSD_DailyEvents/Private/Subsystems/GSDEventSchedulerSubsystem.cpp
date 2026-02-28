// Copyright Bret Bouchard. All Rights Reserved.

#include "Subsystems/GSDEventSchedulerSubsystem.h"
#include "Subsystems/GSDEventBusSubsystem.h"
#include "Subsystems/GSDEventSpawnRegistry.h"
#include "DataAssets/GSDDailyEventConfig.h"
#include "Managers/GSDDeterminismManager.h"
#include "Tags/GSDEventTags.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
#include "GSDEventLog.h"

int32 UGSDEventSchedulerSubsystem::DateToSeed(FDateTime Date) const
{
    // Convert date to integer: 20250115 for Jan 15, 2025
    // This ensures same date always produces same seed
    return Date.GetYear() * 10000 + Date.GetMonth() * 100 + Date.GetDay();
}

void UGSDEventSchedulerSubsystem::SortEventsForDeterministicExecution(TArray<FGSDEventInstance>& Events) const
{
    // EVENT ORDERING (Pitfall 1 Mitigation):
    // ========================================
    // Events execute in deterministic order to prevent race conditions.
    //
    // Ordering Rules:
    // 1. PRIMARY: ScheduledTime (earliest first)
    // 2. SECONDARY: EventTag name (alphabetical) for same-timestamp events
    //
    // Example: Two events at 14:00:
    //   - Event.Daily.Bonfire executes first (B < C)
    //   - Event.Daily.Construction executes second
    Events.Sort([](const FGSDEventInstance& A, const FGSDEventInstance& B)
    {
        if (A.ScheduledTime != B.ScheduledTime)
        {
            return A.ScheduledTime < B.ScheduledTime;
        }
        // Same timestamp - use EventTag name for deterministic ordering
        return A.EventTag.ToString() < B.EventTag.ToString();
    });
}

void UGSDEventSchedulerSubsystem::GenerateDailySchedule(FDateTime Date, int32 WorldSeed)
{
    GSDEVENT_LOG(Log, TEXT("Generating daily schedule for %s with world seed %d"),
        *Date.ToString(), WorldSeed);

    // Clear existing schedule
    ScheduledEvents.Empty();

    // Get determinism manager
    UGSDDeterminismManager* DeterminismMgr = GetGameInstance()->GetSubsystem<UGSDDeterminismManager>();
    if (!DeterminismMgr)
    {
        GSDEVENT_LOG(Error, TEXT("GSDDeterminismManager not found!"));
        return;
    }

    // Combine date and world seed for daily seed
    int32 DailySeed = DateToSeed(Date) ^ WorldSeed;
    DeterminismMgr->InitializeWithSeed(DailySeed);

    // Get event-specific stream (uses EventCategory from GSDDeterminismManager!)
    FRandomStream& EventStream = DeterminismMgr->GetStream(UGSDDeterminismManager::EventCategory);

    // Load available events
    TArray<UGSDDailyEventConfig*> AvailableEvents = LoadAvailableEvents();
    if (AvailableEvents.Num() == 0)
    {
        GSDEVENT_LOG(Warning, TEXT("No event configs found"));
        return;
    }

    // Select events for today (2-5 events per day)
    int32 NumEventsToday = EventStream.RandRange(2, 5);
    GSDEVENT_LOG(Log, TEXT("Scheduling %d events for today"), NumEventsToday);

    for (int32 i = 0; i < NumEventsToday && AvailableEvents.Num() > 0; ++i)
    {
        int32 EventIndex = EventStream.RandHelper(AvailableEvents.Num());
        UGSDDailyEventConfig* SelectedEvent = AvailableEvents[EventIndex];

        // Random time slot (8:00 - 22:00)
        int32 Hour = EventStream.RandRange(8, 22);
        FDateTime EventTime = Date + FTimespan(Hour, 0, 0);

        // Get spawn location from spawn registry (deterministic)
        FVector EventLocation = GetRandomEventLocation(SelectedEvent->EventTag, EventStream);

        // Random intensity (0.5 - 1.5)
        float Intensity = EventStream.FRandRange(0.5f, 1.5f);

        // Create event instance
        FGSDEventInstance Instance;
        Instance.EventConfig = SelectedEvent;
        Instance.EventTag = SelectedEvent->EventTag;
        Instance.ScheduledTime = EventTime;
        Instance.Location = EventLocation;
        Instance.Intensity = Intensity;
        Instance.bIsActive = false;

        ScheduledEvents.Add(Instance);

        GSDEVENT_LOG(Log, TEXT("Scheduled event %s at %s"),
            *SelectedEvent->EventTag.ToString(), *EventTime.ToString());

        // Remove to prevent duplicates
        AvailableEvents.RemoveAtSwap(EventIndex);
    }

    // Sort for deterministic execution order (Pitfall 1 mitigation)
    SortEventsForDeterministicExecution(ScheduledEvents);
}

void UGSDEventSchedulerSubsystem::GetEventsForDate(FDateTime Date, TArray<FGSDEventInstance>& OutEvents) const
{
    OutEvents.Empty();
    for (const FGSDEventInstance& Event : ScheduledEvents)
    {
        if (Event.ScheduledTime.GetDate() == Date.GetDate())
        {
            OutEvents.Add(Event);
        }
    }

    // Return already sorted (was sorted on generation)
    // But re-sort to ensure determinism if caller modifies
    const_cast<UGSDEventSchedulerSubsystem*>(this)->SortEventsForDeterministicExecution(OutEvents);
}

void UGSDEventSchedulerSubsystem::StartEvent(const FGSDEventInstance& Event)
{
    if (!Event.IsValid())
    {
        return;
    }

    GSDEVENT_LOG(Log, TEXT("Starting event: %s"), *Event.EventTag.ToString());

    // Call event start handler
    if (Event.EventConfig)
    {
        Event.EventConfig->OnEventStart(this, Event.Location, Event.Intensity);
    }

    // Broadcast to event bus
    if (UWorld* World = GetWorld())
    {
        if (UGSDEventBusSubsystem* EventBus = World->GetSubsystem<UGSDEventBusSubsystem>())
        {
            EventBus->BroadcastEvent(Event.EventTag, Event.Location, Event.Intensity);
        }
    }

    // Track as active
    FGSDEventInstance ActiveEvent = Event;
    ActiveEvent.bIsActive = true;
    ActiveEvents.Add(ActiveEvent);

    // Broadcast delegate
    OnEventStarted.Broadcast(Event);
}

void UGSDEventSchedulerSubsystem::EndEvent(FGameplayTag EventTag)
{
    GSDEVENT_LOG(Log, TEXT("Ending event: %s"), *EventTag.ToString());

    for (int32 i = ActiveEvents.Num() - 1; i >= 0; --i)
    {
        if (ActiveEvents[i].EventTag == EventTag)
        {
            // Call event end handler
            if (ActiveEvents[i].EventConfig)
            {
                ActiveEvents[i].EventConfig->OnEventEnd(this);
            }

            // Broadcast delegate
            OnEventEnded.Broadcast(ActiveEvents[i]);

            // Remove from active
            ActiveEvents.RemoveAtSwap(i);
        }
    }
}

FVector UGSDEventSchedulerSubsystem::GetRandomEventLocation(const FGameplayTag& EventTag, FRandomStream& Stream) const
{
    // Get spawn registry subsystem
    UGSDEventSpawnRegistry* SpawnRegistry = GetGameInstance()->GetSubsystem<UGSDEventSpawnRegistry>();
    if (SpawnRegistry)
    {
        // Use spawn registry for zone-based location selection
        return SpawnRegistry->GetSpawnLocationForEvent(EventTag, Stream, GetWorld());
    }

    // Fallback: No registry available - use world center with random offset
    GSDEVENT_LOG(Warning, TEXT("GSDEventSpawnRegistry not available, using fallback location for event %s"),
        *EventTag.ToString());

    return FVector(
        Stream.FRandRange(-2000.0f, 2000.0f),
        Stream.FRandRange(-2000.0f, 2000.0f),
        0.0f
    );
}

TArray<UGSDDailyEventConfig*> UGSDEventSchedulerSubsystem::LoadAvailableEvents() const
{
    TArray<UGSDDailyEventConfig*> Events;

    // Get asset registry
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    // Wait for registry to finish loading (important for cooked builds)
    AssetRegistry.SearchAllAssets(true);

    // Find all UGSDDailyEventConfig assets (including derived classes)
    FARFilter Filter;
    Filter.ClassPaths.Add(UGSDDailyEventConfig::StaticClass()->GetClassPathName());
    Filter.bRecursiveClasses = true;  // Include derived classes
    Filter.bIncludeOnlyOnDiskAssets = false;  // Include development assets

    TArray<FAssetData> AssetDataList;
    AssetRegistry.GetAssets(Filter, AssetDataList);

    GSDEVENT_LOG(Log, TEXT("Found %d event config assets in registry"), AssetDataList.Num());

    // Load and validate each event
    for (const FAssetData& AssetData : AssetDataList)
    {
        // Skip abstract base classes - only load concrete event implementations
        if (AssetData.IsAssetLoaded())
        {
            UGSDDailyEventConfig* EventConfig = Cast<UGSDDailyEventConfig>(AssetData.GetAsset());
            if (EventConfig && !EventConfig->GetClass()->HasAnyClassFlags(CLASS_Abstract))
            {
                // Validate event config
                FString ValidationError;
                if (EventConfig->ValidateConfig(ValidationError))
                {
                    Events.Add(EventConfig);
                    GSDEVENT_LOG(Log, TEXT("Loaded event: %s (Tag: %s)"),
                        *AssetData.AssetName.ToString(), *EventConfig->EventTag.ToString());
                }
                else
                {
                    GSDEVENT_LOG(Warning, TEXT("Event '%s' failed validation: %s"),
                        *AssetData.AssetName.ToString(), *ValidationError);
                }
            }
        }
        else
        {
            // Synchronous load for initialization phase
            UGSDDailyEventConfig* EventConfig = Cast<UGSDDailyEventConfig>(AssetData.GetAsset());
            if (EventConfig && !EventConfig->GetClass()->HasAnyClassFlags(CLASS_Abstract))
            {
                FString ValidationError;
                if (EventConfig->ValidateConfig(ValidationError))
                {
                    Events.Add(EventConfig);
                    GSDEVENT_LOG(Log, TEXT("Loaded event: %s (Tag: %s)"),
                        *AssetData.AssetName.ToString(), *EventConfig->EventTag.ToString());
                }
                else
                {
                    GSDEVENT_LOG(Warning, TEXT("Event '%s' failed validation: %s"),
                        *AssetData.AssetName.ToString(), *ValidationError);
                }
            }
        }
    }

    GSDEVENT_LOG(Log, TEXT("LoadAvailableEvents: %d valid events loaded"), Events.Num());
    return Events;
}
