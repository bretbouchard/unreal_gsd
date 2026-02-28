// Copyright Bret Bouchard. All Rights Reserved.

#include "Subsystems/GSDEventBusSubsystem.h"
#include "GSDEventLog.h"

bool UGSDEventBusSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not editor preview worlds
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

FGSDEventHandle UGSDEventBusSubsystem::Subscribe(FGameplayTag EventTag, FOnGSDEvent::FDelegate&& Delegate)
{
    if (!EventTag.IsValid())
    {
        GSDEVENT_WARN(TEXT("Subscribe called with invalid tag"));
        return FGSDEventHandle();
    }

    // Find or add delegate for this tag
    FOnGSDEvent& EventDelegate = EventDelegates.FindOrAdd(EventTag);

    // Add the delegate and store the handle
    FDelegateHandle Handle = EventDelegate.Add(MoveTemp(Delegate));

    // Create and return the subscription handle
    FGSDEventHandle EventHandle;
    EventHandle.DelegateHandle = Handle;
    EventHandle.SubscribedTag = EventTag;

    GSDEVENT_TRACE(TEXT("Subscribed to event tag: %s"), *EventTag.ToString());

    return EventHandle;
}

void UGSDEventBusSubsystem::BroadcastEvent(FGameplayTag EventTag, FVector Location, float Intensity)
{
    if (!EventTag.IsValid())
    {
        GSDEVENT_WARN(TEXT("BroadcastEvent called with invalid tag"));
        return;
    }

    GSDEVENT_LOG(Log, TEXT("Broadcasting event: %s at %s (intensity=%.2f)"),
        *EventTag.ToString(), *Location.ToString(), Intensity);

    // Track as active event
    ActiveEvents.AddUnique(EventTag);

    // Broadcast to exact match subscribers
    if (FOnGSDEvent* ExactDelegate = EventDelegates.Find(EventTag))
    {
        ExactDelegate->Broadcast(EventTag, Location, Intensity);
    }

    // CRITICAL: Broadcast to parent tags for hierarchical matching
    // This allows Event.Daily listener to receive Event.Daily.Construction events
    for (auto& Pair : EventDelegates)
    {
        // Skip exact match (already handled above)
        if (Pair.Key == EventTag)
        {
            continue;
        }

        // Check if the broadcast event matches this subscriber's tag
        // EventTag.MatchesTag(Pair.Key) means EventTag is a child of Pair.Key
        // e.g., Event.Daily.Construction.MatchesTag(Event.Daily) returns true
        if (EventTag.MatchesTag(Pair.Key))
        {
            GSDEVENT_TRACE(TEXT("Hierarchical match: %s -> %s"), *EventTag.ToString(), *Pair.Key.ToString());
            Pair.Value.Broadcast(EventTag, Location, Intensity);
        }
    }
}

void UGSDEventBusSubsystem::Unsubscribe(FGSDEventHandle& Handle)
{
    if (!Handle.IsValid())
    {
        GSDEVENT_TRACE(TEXT("Unsubscribe called with invalid handle"));
        return;
    }

    // Find the delegate for this tag
    if (FOnGSDEvent* EventDelegate = EventDelegates.Find(Handle.SubscribedTag))
    {
        // Remove the specific delegate
        EventDelegate->Remove(Handle.DelegateHandle);

        GSDEVENT_TRACE(TEXT("Unsubscribed from event tag: %s"), *Handle.SubscribedTag.ToString());

        // Clean up empty delegates
        if (!EventDelegate->IsBound())
        {
            EventDelegates.Remove(Handle.SubscribedTag);
        }
    }

    // Reset the handle
    Handle.Reset();
}
