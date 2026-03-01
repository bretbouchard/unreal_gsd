// Copyright Bret Bouchard. All Rights Reserved.

#include "Subsystems/GSDCrowdEventSubsystem.h"
#include "GSDCrowdLog.h"

bool UGSDCrowdEventSubsystem::ShouldCreateSubsystem(UWorld* World) const
{
    // Only create in game worlds (not editor preview worlds)
    return World && (World->IsGameWorld() || World->IsPlayInEditor());
}

void UGSDCrowdEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LOG_GSDCROWDS, Log, TEXT("CrowdEventSubsystem initialized"));
}

void UGSDCrowdEventSubsystem::Deinitialize()
{
    ClearAllListeners();

    Super::Deinitialize();
}

void UGSDCrowdEventSubsystem::BroadcastEvent(const FGSDCrowdEvent& Event)
{
    EventsThisFrame++;
    TotalEventsBroadcast++;

    // Broadcast to all matching listeners
    for (const FEventListener& Listener : Listeners)
    {
        // If listener has no tag filter, or tag matches, broadcast
        if (!Listener.EventTag.IsValid() || Listener.EventTag == Event.EventTag)
        {
            if (Listener.Delegate.IsBound())
            {
                Listener.Delegate.Execute(Event, Event.EventTag);
            }
        }
    }

    // Log significant events
    if (Event.EventType != EGSDCrowdEventType::None)
    {
        UE_LOG(LOG_GSDCROWDS, Verbose, TEXT("Broadcast event: %s at %s"),
            *UEnum::GetValueAsString(Event.EventType), *Event.Location.ToString());
    }
}

void UGSDCrowdEventSubsystem::BroadcastLocationEvent(EGSDCrowdEventType EventType, FVector Location, float Radius)
{
    FGSDCrowdEvent Event;
    Event.EventType = EventType;
    Event.Location = Location;
    Event.Radius = Radius;

    BroadcastEvent(Event);
}

void UGSDCrowdEventSubsystem::BroadcastEntityEvent(EGSDCrowdEventType EventType, int32 EntityID, FVector Location)
{
    FGSDCrowdEvent Event;
    Event.EventType = EventType;
    Event.EntityID = EntityID;
    Event.Location = Location;

    BroadcastEvent(Event);
}

void UGSDCrowdEventSubsystem::BroadcastPursuitEvent(EGSDCrowdEventType EventType, int32 EntityID, int32 TargetEntityID, FVector Location)
{
    FGSDCrowdEvent Event;
    Event.EventType = EventType;
    Event.EntityID = EntityID;
    Event.TargetEntityID = TargetEntityID;
    Event.Location = Location;

    BroadcastEvent(Event);
}

int32 UGSDCrowdEventSubsystem::RegisterListener(FGameplayTag EventTag, const FOnCrowdEvent& Delegate)
{
    if (!Delegate.IsBound())
    {
        UE_LOG(LOG_GSDCROWDS, Warning, TEXT("RegisterListener: Delegate is not bound"));
        return INDEX_NONE;
    }

    FEventListener NewListener;
    NewListener.Handle = NextListenerHandle++;
    NewListener.EventTag = EventTag;
    NewListener.Delegate = Delegate;

    Listeners.Add(NewListener);

    UE_LOG(LOG_GSDCROWDS, Verbose, TEXT("Registered event listener (handle=%d, tag=%s)"),
        NewListener.Handle, *EventTag.ToString());

    return NewListener.Handle;
}

void UGSDCrowdEventSubsystem::UnregisterListener(int32 Handle)
{
    int32 RemovedCount = Listeners.RemoveAll([Handle](const FEventListener& Listener)
    {
        return Listener.Handle == Handle;
    });

    if (RemovedCount > 0)
    {
        UE_LOG(LOG_GSDCROWDS, Verbose, TEXT("Unregistered event listener (handle=%d)"), Handle);
    }
    else
    {
        UE_LOG(LOG_GSDCROWDS, Warning, TEXT("UnregisterListener: Handle %d not found"), Handle);
    }
}

void UGSDCrowdEventSubsystem::ClearAllListeners()
{
    Listeners.Empty();
    UE_LOG(LOG_GSDCROWDS, Log, TEXT("Cleared all event listeners"));
}
