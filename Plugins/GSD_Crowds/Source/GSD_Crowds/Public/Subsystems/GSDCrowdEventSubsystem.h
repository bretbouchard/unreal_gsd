// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "GSDCrowdEventSubsystem.generated.h"

class UGSDCrowdManagerSubsystem;

/**
 * Crowd event types for gameplay integration.
 */
UENUM(BlueprintType)
enum class EGSDCrowdEventType : uint8
{
    None            UMETA(DisplayName = "None"),
    Spawn           UMETA(DisplayName = "Spawn"),
    Despawn         UMETA(DisplayName = "Despawn"),
    Alert           UMETA(DisplayName = "Alert"),
    Attack          UMETA(DisplayName = "Attack"),
    Death           UMETA(DisplayName = "Death"),
    TargetAcquired  UMETA(DisplayName = "Target Acquired"),
    TargetLost      UMETA(DisplayName = "Target Lost"),
    StateChange     UMETA(DisplayName = "State Change"),
};

/**
 * Event payload for crowd events.
 */
USTRUCT(BlueprintType)
struct GSD_CROWDS_API FGSDCrowdEvent
{
    GENERATED_BODY()

    /** Type of event */
    UPROPERTY(BlueprintReadOnly, Category = "Event")
    EGSDCrowdEventType EventType = EGSDCrowdEventType::None;

    /** Event tag for filtering */
    UPROPERTY(BlueprintReadOnly, Category = "Event")
    FGameplayTag EventTag;

    /** World location of event */
    UPROPERTY(BlueprintReadOnly, Category = "Event")
    FVector Location = FVector::ZeroVector;

    /** Radius of effect */
    UPROPERTY(BlueprintReadOnly, Category = "Event")
    float Radius = 0.0f;

    /** Entity ID (if applicable) */
    UPROPERTY(BlueprintReadOnly, Category = "Event")
    int32 EntityID = INDEX_NONE;

    /** Target entity ID (for pursuit events) */
    UPROPERTY(BlueprintReadOnly, Category = "Event")
    int32 TargetEntityID = INDEX_NONE;

    /** Custom event data */
    UPROPERTY(BlueprintReadOnly, Category = "Event")
    FString CustomData;
};

/**
 * Delegate for crowd events.
 */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnCrowdEvent, const FGSDCrowdEvent&, Event, const FGameplayTag&, EventTag);

/**
 * World subsystem for crowd event dispatch and handling.
 *
 * Provides centralized event system for crowd gameplay integration:
 * - Broadcast events (alerts, attacks, deaths, etc.)
 * - Event filtering by tags
 * - Listener registration
 *
 * Usage:
 * 1. Get subsystem: GetWorld()->GetSubsystem<UGSDCrowdEventSubsystem>()
 * 2. Broadcast: Subsystem->BroadcastEvent(Event)
 * 3. Listen: Subsystem->RegisterListener(Tag, Delegate)
 */
UCLASS()
class GSD_CROWDS_API UGSDCrowdEventSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    //-- Broadcasting --

    /**
     * Broadcast a crowd event to all registered listeners.
     *
     * @param Event Event payload to broadcast
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Events")
    void BroadcastEvent(const FGSDCrowdEvent& Event);

    /**
     * Broadcast a simple location-based event.
     *
     * @param EventType Type of event
     * @param Location World location
     * @param Radius Effect radius
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Events")
    void BroadcastLocationEvent(EGSDCrowdEventType EventType, FVector Location, float Radius);

    /**
     * Broadcast an entity event (spawn, death, etc.).
     *
     * @param EventType Type of event
     * @param EntityID Affected entity
     * @param Location World location
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Events")
    void BroadcastEntityEvent(EGSDCrowdEventType EventType, int32 EntityID, FVector Location);

    /**
     * Broadcast a pursuit event (target acquired/lost).
     *
     * @param EventType TargetAcquired or TargetLost
     * @param EntityID Chasing entity
     * @param TargetEntityID Target being chased
     * @param Location World location
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Events")
    void BroadcastPursuitEvent(EGSDCrowdEventType EventType, int32 EntityID, int32 TargetEntityID, FVector Location);

    //-- Listener Management --

    /**
     * Register a listener for specific event tags.
     *
     * @param EventTag Tag to listen for (None = all events)
     * @param Delegate Delegate to call when event occurs
     * @return Handle for unregistration
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Events")
    int32 RegisterListener(FGameplayTag EventTag, const FOnCrowdEvent& Delegate);

    /**
     * Unregister a listener by handle.
     *
     * @param Handle Handle returned from RegisterListener
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Events")
    void UnregisterListener(int32 Handle);

    /**
     * Clear all listeners.
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Events")
    void ClearAllListeners();

    //-- Queries --

    /**
     * Get total number of events broadcast this frame.
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds|Events")
    int32 GetEventsThisFrame() const { return EventsThisFrame; }

    /**
     * Get total number of events broadcast.
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds|Events")
    int64 GetTotalEventsBroadcast() const { return TotalEventsBroadcast; }

protected:
    // ~UWorldSubsystem interface
    virtual bool ShouldCreateSubsystem(UWorld* World) const override;
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    // ~End of UWorldSubsystem interface

private:
    //-- Listener Structure --
    struct FEventListener
    {
        int32 Handle;
        FGameplayTag EventTag;
        FOnCrowdEvent Delegate;
    };

    //-- Listeners --
    UPROPERTY()
    TArray<FEventListener> Listeners;

    int32 NextListenerHandle = 1;

    //-- Statistics --
    int32 EventsThisFrame = 0;
    int64 TotalEventsBroadcast = 0;
};
