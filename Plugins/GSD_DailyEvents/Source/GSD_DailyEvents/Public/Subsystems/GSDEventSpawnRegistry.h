// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "HAL/CriticalSection.h"
#include "GSDEventSpawnRegistry.generated.h"

class UGSDEventSpawnZone;
class UGSDDailyEventConfig;

/**
 * Performance and reliability configuration for spawn registry.
 * Can be configured via DefaultGame.ini
 */
USTRUCT(BlueprintType)
struct GSD_DAILYEVENTS_API FGSDEventSpawnConfig
{
    GENERATED_BODY()

    //-- Async Loading --
    UPROPERTY(EditDefaultsOnly, Config, Category = "Loading")
    bool bUseAsyncLoading = true;

    UPROPERTY(EditDefaultsOnly, Config, Category = "Loading")
    float AsyncLoadTimeoutSeconds = 5.0f;

    //-- NavMesh Projection --
    UPROPERTY(EditDefaultsOnly, Config, Category = "Navigation")
    int32 MaxNavMeshRetries = 3;

    UPROPERTY(EditDefaultsOnly, Config, Category = "Navigation")
    float NavMeshRetryDelayMs = 10.0f;

    //-- Caching --
    UPROPERTY(EditDefaultsOnly, Config, Category = "Caching")
    int32 MaxCachedZonesPerTag = 50;

    //-- Asset Paths --
    UPROPERTY(EditDefaultsOnly, Config, Category = "Paths")
    TArray<FName> SpawnZoneSearchPaths;

    //-- Defaults --
    FGSDEventSpawnConfig()
    {
        SpawnZoneSearchPaths.Add(FName("/Game/DailyEvents/SpawnZones"));
    }
};

/**
 * Cached zone data for a specific event tag.
 * Pre-calculated to avoid O(n) filtering on hot path.
 */
USTRUCT()
struct FGSDEventTagZoneCache
{
    GENERATED_BODY()

    //-- Cached Zones (sorted by priority) --
    UPROPERTY()
    TArray<TObjectPtr<UGSDEventSpawnZone>> Zones;

    //-- Pre-calculated total weight for weighted selection --
    float TotalWeight = 0.0f;

    //-- Is cache valid (invalidated on zone refresh) --
    bool bIsValid = false;
};

/**
 * Registry for event spawn zones.
 * Loads spawn zones from asset registry and provides deterministic zone selection.
 *
 * Usage:
 * 1. Create UGSDEventSpawnZone data assets in content browser
 * 2. Registry auto-loads zones on initialization (async by default)
 * 3. GetSpawnLocationForEvent() returns deterministic location
 *
 * Determinism:
 * - Zones sorted by Priority (descending), then ZoneName (alphabetical)
 * - Same seed always selects same zone for same event
 *
 * Thread Safety:
 * - Query methods are thread-safe for concurrent reads
 * - Registration/refresh methods must be called from game thread only
 */
UCLASS(Config=Game)
class GSD_DAILYEVENTS_API UGSDEventSpawnRegistry : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    //-- Configuration --

    /** Get current spawn configuration */
    UFUNCTION(BlueprintPure, Category = "GSD|Events")
    const FGSDEventSpawnConfig& GetSpawnConfig() const { return SpawnConfig; }

    /** Update spawn configuration (applies immediately) */
    UFUNCTION(BlueprintCallable, Category = "GSD|Events")
    void SetSpawnConfig(const FGSDEventSpawnConfig& NewConfig);

    //-- Zone Queries --

    /**
     * Get all loaded spawn zones.
     * Thread-safe for reads.
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Events")
    const TArray<UGSDEventSpawnZone*>& GetAllSpawnZones() const;

    /**
     * Get spawn zones compatible with a specific event tag.
     * Uses cached zones for O(1) lookup.
     * Thread-safe for reads.
     */
    void GetCompatibleZones(const FGameplayTag& EventTag, TArray<UGSDEventSpawnZone*>& OutZones) const;

    /**
     * Get a deterministic spawn location for an event.
     * Uses the RNG stream to select zone and point within zone.
     *
     * Thread-safe for reads.
     *
     * @param EventTag The event type to spawn
     * @param Stream Deterministic RNG stream (from GSDDeterminismManager)
     * @param World Optional world for NavMesh projection
     * @return Spawn location in world space
     */
    FVector GetSpawnLocationForEvent(const FGameplayTag& EventTag, FRandomStream& Stream, UWorld* World = nullptr) const;

    //-- Registry Management --

    /**
     * Reload spawn zones from asset registry.
     * Uses async loading if configured.
     * Must be called from game thread.
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Events")
    void RefreshZones();

    /**
     * Check if zones have finished loading.
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Events")
    bool IsLoaded() const { return bZonesLoaded; }

    /**
     * Get total zone count.
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Events")
    int32 GetZoneCount() const;

    //-- Events --

    /** Broadcast when zones finish loading */
    DECLARE_MULTICAST_DELEGATE(FOnZonesLoaded);
    FOnZonesLoaded OnZonesLoaded;

    //-- Network Validation (GSDNETWORK-107) --
    // These validation functions can be called from Server RPCs to prevent exploitable inputs.
    // Subsystems cannot have Server RPCs directly, only AActor/UActorComponent can.
    // These validation functions provide the security layer that Server RPCs would call.

    /**
     * Validate event tag for network safety.
     * Call from Server RPCs before spawning events.
     *
     * @param EventTag The event tag to validate
     * @param OutError Error message if validation fails
     * @return True if tag is valid and safe
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Events|Network")
    bool ValidateEventTag(const FGameplayTag& EventTag, FString& OutError) const;

    /**
     * Validate spawn zone for network safety.
     * Call from Server RPCs before using a spawn zone.
     *
     * @param Zone The spawn zone to validate
     * @param OutError Error message if validation fails
     * @return True if zone is valid and safe
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Events|Network")
    bool ValidateSpawnZone(UGSDEventSpawnZone* Zone, FString& OutError) const;

    /**
     * Get maximum allowed zones per tag for validation.
     */
    static constexpr int32 GetMaxZonesPerTag() { return MaxZonesPerTag; }

private:
    //-- Validation Constants (GSDNETWORK-107) --
    // Maximum zones per tag to prevent memory exploits
    static constexpr int32 MaxZonesPerTag = 50;
    // Maximum spawn location extent to prevent extreme value exploits
    static constexpr float MaxSpawnExtent = 1000000.0f; // 1km from origin
    //-- Configuration --
    UPROPERTY(Config)
    FGSDEventSpawnConfig SpawnConfig;

    //-- State --
    UPROPERTY()
    TArray<TObjectPtr<UGSDEventSpawnZone>> LoadedZones;

    //-- Zone Cache by Event Tag (for O(1) lookup) --
    TMap<FGameplayTag, FGSDEventTagZoneCache> ZoneCache;

    //-- Cache for "any event" (empty tag) --
    UPROPERTY()
    TArray<TObjectPtr<UGSDEventSpawnZone>> CachedAllZones;
    float CachedTotalWeight = 0.0f;

    //-- Loading State --
    std::atomic<bool> bZonesLoaded{false};
    std::atomic<bool> bIsLoading{false};

    //-- Thread Safety --
    mutable FRWLock ZoneCacheLock;

    //-- Helpers --

    /** Async load zones from asset registry */
    void LoadZonesAsync();

    /** Synchronous load zones (fallback) */
    void LoadZonesSync();

    /** Process loaded zone data */
    void ProcessLoadedZones(const TArray<UGSDEventSpawnZone*>& Zones);

    /** Sort zones for deterministic order */
    void SortZones();

    /** Rebuild zone cache for all tags */
    void RebuildZoneCache();

    /** Rebuild cache for specific tag */
    void RebuildTagCache(const FGameplayTag& Tag);

    /** Calculate total weight for zone array */
    static float CalculateTotalWeight(const TArray<UGSDEventSpawnZone*>& Zones);

    /** Select zone using weighted random selection */
    UGSDEventSpawnZone* SelectWeightedZone(const TArray<UGSDEventSpawnZone*>& Zones, float TotalWeight, FRandomStream& Stream) const;

    /** Project point to NavMesh with retry logic */
    FVector ProjectToNavMeshWithRetry(UWorld* World, const FVector& Point, float QueryExtent) const;
};

/**
 * Helper struct for asset registry search.
 * Used to find all UGSDEventSpawnZone assets.
 */
USTRUCT()
struct FGSDEventSpawnZoneRegistrySearch
{
    GENERATED_BODY()

    TArray<FAssetData> FoundAssets;
    void Search(const TArray<FName>& SearchPaths);
};
