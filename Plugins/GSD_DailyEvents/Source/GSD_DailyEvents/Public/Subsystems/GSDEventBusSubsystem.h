// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "GSDEventBusSubsystem.generated.h"

/**
 * Delegate for event notifications.
 * Called when an event matching the subscribed tag is broadcast.
 *
 * @param EventTag The tag of the broadcast event
 * @param Location World location of the event
 * @param Intensity Event intensity (0.0 - 1.0+)
 */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnGSDEvent, FGameplayTag /*EventTag*/, const FVector& /*Location*/, float /*Intensity*/);

/**
 * Handle for an event subscription.
 * Used to unsubscribe from events when no longer needed.
 *
 * CRITICAL: Store this handle and call Unsubscribe when done to prevent memory leaks.
 */
USTRUCT(BlueprintType)
struct GSD_DAILYEVENTS_API FGSDEventHandle
{
    GENERATED_BODY()

    /** The delegate handle for internal use */
    FDelegateHandle DelegateHandle;

    /** The tag that was subscribed to */
    UPROPERTY(BlueprintReadOnly, Category = "Event")
    FGameplayTag SubscribedTag;

    /** Check if this handle is valid */
    bool IsValid() const { return DelegateHandle.IsValid() && SubscribedTag.IsValid(); }

    /** Reset this handle */
    void Reset()
    {
        DelegateHandle.Reset();
        SubscribedTag = FGameplayTag();
    }
};

/**
 * World subsystem for decoupled event messaging with GameplayTag routing.
 *
 * Purpose: Enable systems to communicate without direct dependencies.
 * Events are routed by GameplayTag, supporting hierarchical matching.
 *
 * Hierarchical Matching:
 * - Subscribing to "Event.Daily" receives ALL Event.Daily.* broadcasts
 * - Subscribing to "Event.Daily.Construction" receives ONLY that specific event
 *
 * Usage:
 * 1. Subscribe to events using Subscribe()
 * 2. Store the returned FGSDEventHandle for later unsubscription
 * 3. Broadcast events using BroadcastEvent()
 * 4. Unsubscribe when done using Unsubscribe()
 *
 * Example:
 * // Subscribe to all daily events
 * FGSDEventHandle Handle = EventBus->Subscribe(
 *     FGameplayTag::RequestGameplayTag("Event.Daily"),
 *     FOnGSDEvent::FDelegate::CreateLambda([](FGameplayTag Tag, FVector Loc, float Intensity) {
 *         UE_LOG(LogTemp, Log, TEXT("Event %s at %s"), *Tag.ToString(), *Loc.ToString());
 *     })
 * );
 *
 * // Later, unsubscribe
 * EventBus->Unsubscribe(Handle);
 */
UCLASS()
class GSD_DAILYEVENTS_API UGSDEventBusSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    //-- Subscription API --

    /**
     * Subscribe to events matching a specific GameplayTag.
     *
     * Hierarchical matching is supported:
     * - Subscribing to "Event.Daily" receives "Event.Daily.Construction", "Event.Daily.Bonfire", etc.
     *
     * @param EventTag The tag to subscribe to (hierarchical matching enabled)
     * @param Delegate The delegate to call when matching events are broadcast
     * @return Handle for later unsubscription. CRITICAL: Store this for cleanup.
     */
    FGSDEventHandle Subscribe(FGameplayTag EventTag, FOnGSDEvent::FDelegate&& Delegate);

    /**
     * Broadcast an event to all matching subscribers.
     *
     * Hierarchical routing:
     * - Broadcasting "Event.Daily.Construction" notifies:
     *   - "Event.Daily.Construction" subscribers (exact match)
     *   - "Event.Daily" subscribers (parent match)
     *   - "Event" subscribers (grandparent match)
     *
     * @param EventTag The tag of the event to broadcast
     * @param Location World location of the event
     * @param Intensity Event intensity (default 1.0)
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Events")
    void BroadcastEvent(FGameplayTag EventTag, FVector Location, float Intensity = 1.0f);

    /**
     * Unsubscribe from events using a previously returned handle.
     *
     * CRITICAL: Always call this when done with a subscription to prevent memory leaks.
     *
     * @param Handle The handle returned from Subscribe()
     */
    void Unsubscribe(FGSDEventHandle& Handle);

    //-- Queries --

    /**
     * Get the count of currently active events.
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Events")
    int32 GetActiveEventCount() const { return ActiveEvents.Num(); }

protected:
    //-- UWorldSubsystem Interface --
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    //-- State --

    /** Map of event tags to their delegate broadcasts */
    TMap<FGameplayTag, FOnGSDEvent> EventDelegates;

    /** Currently active events (for queries) */
    UPROPERTY()
    TArray<FGameplayTag> ActiveEvents;
};
