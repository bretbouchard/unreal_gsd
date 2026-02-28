// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "GSDEventSchedulerSubsystem.generated.h"

class UGSDDailyEventConfig;
class UGSDDeterminismManager;
class UGSDEventSpawnRegistry;

/** Scheduled event instance */
USTRUCT(BlueprintType)
struct GSD_DAILYEVENTS_API FGSDEventInstance
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<UGSDDailyEventConfig> EventConfig;

    UPROPERTY()
    FGameplayTag EventTag;

    UPROPERTY()
    FDateTime ScheduledTime;

    UPROPERTY()
    FVector Location = FVector::ZeroVector;

    UPROPERTY()
    float Intensity = 1.0f;

    UPROPERTY()
    bool bIsActive = false;

    bool IsValid() const { return EventConfig != nullptr; }
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnEventScheduled, const FGSDEventInstance&, Event);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnEventStarted, const FGSDEventInstance&, Event);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnEventEnded, const FGSDEventInstance&, Event);

/**
 * Game instance subsystem for deterministic daily event scheduling.
 * Persists across level loads, generates schedules from date + seed.
 *
 * Determinism: Same date + world seed always produces same events.
 * Uses GSDDeterminismManager::EventCategory for isolated RNG.
 *
 * EVENT ORDERING (Pitfall 1 Mitigation from Research):
 * ========================================
 * Events execute in a strict deterministic order to prevent race conditions
 * when events have dependencies on each other or shared systems.
 *
 * Ordering Rules:
 * 1. PRIMARY SORT: Events with earlier ScheduledTime execute first
 * 2. SECONDARY SORT: Events with the same timestamp execute in alphabetical
 *    order of their EventTag name (e.g., "Event.Daily.Bonfire" before "Event.Daily.Construction")
 *
 * Example: If two events are both scheduled for 14:00:
 *   - Event.Daily.Bonfire executes first (B < C alphabetically)
 *   - Event.Daily.Construction executes second
 *
 * This guarantees that same-timestamp events ALWAYS execute in the same order
 * across all game sessions with the same seed.
 *
 * Usage:
 * 1. GenerateDailySchedule(Date, WorldSeed) - Generate events for a day
 * 2. GetEventsForDate(Date) - Retrieve scheduled events
 * 3. StartEvent(Event) - Begin event execution
 * 4. EndEvent(EventTag) - End active event
 */
UCLASS()
class GSD_DAILYEVENTS_API UGSDEventSchedulerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    //-- Schedule Generation --

    /**
     * Generate deterministic schedule for a specific date.
     * Uses GSDDeterminismManager::EventCategory for reproducible RNG.
     *
     * @param Date The date to generate events for
     * @param WorldSeed World-specific seed for variation
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Events")
    void GenerateDailySchedule(FDateTime Date, int32 WorldSeed);

    /**
     * Get events scheduled for a date.
     * Returns events sorted by ScheduledTime, then by EventTag for determinism.
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Events")
    void GetEventsForDate(FDateTime Date, TArray<FGSDEventInstance>& OutEvents) const;

    //-- Runtime Control --

    /**
     * Start a scheduled event.
     * Calls EventConfig->OnEventStart and broadcasts to EventBus.
     */
    void StartEvent(const FGSDEventInstance& Event);

    /**
     * End an active event.
     * Calls EventConfig->OnEventEnd and broadcasts to EventBus.
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Events")
    void EndEvent(FGameplayTag EventTag);

    //-- Queries --

    UFUNCTION(BlueprintPure, Category = "GSD|Events")
    int32 GetScheduledEventCount() const { return ScheduledEvents.Num(); }

    UFUNCTION(BlueprintPure, Category = "GSD|Events")
    int32 GetActiveEventCount() const { return ActiveEvents.Num(); }

    //-- Delegates --

    FOnEventStarted& GetOnEventStarted() { return OnEventStarted; }
    FOnEventEnded& GetOnEventEnded() { return OnEventEnded; }

protected:
    //-- State --
    UPROPERTY()
    TArray<FGSDEventInstance> ScheduledEvents;

    UPROPERTY()
    TArray<FGSDEventInstance> ActiveEvents;

    //-- Delegates --
    FOnEventStarted OnEventStarted;
    FOnEventEnded OnEventEnded;

    //-- Helpers --
    int32 DateToSeed(FDateTime Date) const;

    /**
     * Get spawn location for an event using the spawn registry.
     * Uses spawn zones defined in UGSDEventSpawnZone assets.
     * Falls back to world center if no zones are defined.
     *
     * @param EventTag The event type to spawn
     * @param Stream Deterministic RNG stream
     * @return Valid spawn location in world space
     */
    FVector GetRandomEventLocation(const FGameplayTag& EventTag, FRandomStream& Stream) const;

    /**
     * Load all available event configs from asset registry.
     * Discovers all UGSDDailyEventConfig derived assets.
     *
     * @return Array of loaded event configs
     */
    TArray<UGSDDailyEventConfig*> LoadAvailableEvents() const;

    /**
     * Sort events for deterministic execution order.
     * Primary: ScheduledTime (earliest first)
     * Secondary: EventTag name (alphabetical) for same-timestamp events
     */
    void SortEventsForDeterministicExecution(TArray<FGSDEventInstance>& Events) const;
};
