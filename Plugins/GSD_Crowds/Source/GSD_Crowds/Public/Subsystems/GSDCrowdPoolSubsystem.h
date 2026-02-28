// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GSDCrowdPoolSubsystem.generated.h"

/**
 * Struct for tracking pool statistics.
 */
USTRUCT(BlueprintType)
struct GSD_CROWDS_API FGSDPoolStats
{
    GENERATED_BODY()

    //-- Pool Configuration --
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config")
    int32 InitialPoolSize = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config")
    int32 MaxPoolSize = 0;

    //-- Current State --
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    int32 ActiveCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    int32 AvailableCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    int32 TotalCreated = 0;

    //-- Performance Metrics --
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 TotalAcquired = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 TotalReturned = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 CacheHits = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 CacheMisses = 0;

    //-- Utility Methods --

    /** Get the utilization percentage (0-100) */
    float GetUtilizationPercent() const
    {
        const int32 Total = ActiveCount + AvailableCount;
        return Total > 0 ? (static_cast<float>(ActiveCount) / Total) * 100.0f : 0.0f;
    }

    /** Get the cache hit ratio (0-100) */
    float GetCacheHitRatio() const
    {
        const int32 TotalRequests = CacheHits + CacheMisses;
        return TotalRequests > 0 ? (static_cast<float>(CacheHits) / TotalRequests) * 100.0f : 0.0f;
    }
};

/**
 * Delegate for pool expansion notification.
 */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnPoolExpanded, TSubclassOf<AActor>, ActorClass, int32, NewSize);

/**
 * Delegate for pool exhaustion warning.
 */
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnPoolExhausted, TSubclassOf<AActor>, ActorClass, int32, ActiveCount, int32, MaxSize);

/**
 * World subsystem for pooling crowd entity actors.
 *
 * Provides efficient object pooling to avoid constant spawn/destroy overhead.
 * Pre-allocates actors and recycles them for reuse.
 *
 * Usage:
 * 1. Get subsystem: GetWorld()->GetSubsystem<UGSDCrowdPoolSubsystem>()
 * 2. Prewarm pool: Subsystem->PrewarmPool(ActorClass, 100)
 * 3. Acquire actor: Subsystem->AcquireFromPool(ActorClass, Transform)
 * 4. Return actor: Subsystem->ReturnToPool(Actor)
 * 5. Get stats: Subsystem->GetPoolStats(ActorClass)
 */
UCLASS()
class GSD_CROWDS_API UGSDCrowdPoolSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    //-- Pool Management --

    /**
     * Pre-allocate entities into the pool for a specific actor class.
     * Call this during level initialization or before spawning crowds.
     *
     * @param ActorClass The actor class to pool (must be valid)
     * @param Count Number of entities to pre-allocate
     * @return Number of entities successfully pre-allocated
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Pool")
    int32 PrewarmPool(TSubclassOf<AActor> ActorClass, int32 Count);

    /**
     * Acquire an entity from the pool.
     * If pool is empty, creates a new actor (up to MaxPoolSize).
     *
     * @param ActorClass The actor class to acquire
     * @param SpawnTransform The world transform for the actor
     * @return The acquired actor, or nullptr if pool exhausted
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Pool")
    AActor* AcquireFromPool(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform);

    /**
     * Return an entity to the pool for reuse.
     * Deactivates the actor and makes it available for future acquisitions.
     *
     * @param Actor The actor to return to the pool
     * @return True if successfully returned to pool
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Pool")
    bool ReturnToPool(AActor* Actor);

    /**
     * Clear all pools and destroy all pooled actors.
     * Call during level transition or cleanup.
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Pool")
    void ClearAllPools();

    /**
     * Clear pool for a specific actor class.
     *
     * @param ActorClass The actor class pool to clear
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Pool")
    void ClearPool(TSubclassOf<AActor> ActorClass);

    //-- Configuration --

    /**
     * Set the maximum pool size for a specific actor class.
     * Existing actors beyond this limit will be destroyed when returned.
     *
     * @param ActorClass The actor class to configure
     * @param MaxSize Maximum number of actors in pool (0 = unlimited)
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Pool|Config")
    void SetMaxPoolSize(TSubclassOf<AActor> ActorClass, int32 MaxSize);

    /**
     * Set the initial pool size for a specific actor class.
     * Used when auto-prewarming on first acquisition.
     *
     * @param ActorClass The actor class to configure
     * @param InitialSize Initial number of actors to pre-allocate
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Pool|Config")
    void SetInitialPoolSize(TSubclassOf<AActor> ActorClass, int32 InitialSize);

    /**
     * Enable or disable auto-expansion for a pool.
     * When disabled, AcquireFromPool returns nullptr if pool is empty.
     *
     * @param ActorClass The actor class to configure
     * @param bEnable Whether to allow auto-expansion
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Pool|Config")
    void SetAutoExpand(TSubclassOf<AActor> ActorClass, bool bEnable);

    //-- Queries --

    /**
     * Get pool statistics for a specific actor class.
     *
     * @param ActorClass The actor class to query
     * @return Pool statistics structure
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds|Pool")
    FGSDPoolStats GetPoolStats(TSubclassOf<AActor> ActorClass) const;

    /**
     * Get the number of active (acquired) actors for a class.
     *
     * @param ActorClass The actor class to query
     * @return Number of active actors
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds|Pool")
    int32 GetActiveCount(TSubclassOf<AActor> ActorClass) const;

    /**
     * Get the number of available actors in the pool for a class.
     *
     * @param ActorClass The actor class to query
     * @return Number of available actors
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds|Pool")
    int32 GetAvailableCount(TSubclassOf<AActor> ActorClass) const;

    /**
     * Check if pool has available actors for a class.
     *
     * @param ActorClass The actor class to check
     * @return True if pool has available actors
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds|Pool")
    bool HasAvailableActors(TSubclassOf<AActor> ActorClass) const;

    /**
     * Get total number of pooled actor classes.
     *
     * @return Number of unique actor classes in pools
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds|Pool")
    int32 GetPoolCount() const { return Pools.Num(); }

    //-- Delegates --

    /**
     * Get delegate for pool expansion events.
     */
    FOnPoolExpanded& GetOnPoolExpanded() { return PoolExpandedDelegate; }

    /**
     * Get delegate for pool exhaustion warnings.
     */
    FOnPoolExhausted& GetOnPoolExhausted() { return PoolExhaustedDelegate; }

protected:
    //-- Pool Entry Structure --
    struct FPoolEntry
    {
        //-- Available Actors (inactive, ready for use) --
        TArray<TWeakObjectPtr<AActor>> AvailableActors;

        //-- Active Actors (currently in use) --
        TArray<TWeakObjectPtr<AActor>> ActiveActors;

        //-- Configuration --
        int32 InitialPoolSize = 50;
        int32 MaxPoolSize = 500;
        bool bAutoExpand = true;

        //-- Statistics --
        int32 TotalCreated = 0;
        int32 TotalAcquired = 0;
        int32 TotalReturned = 0;
        int32 CacheHits = 0;
        int32 CacheMisses = 0;
    };

    //-- Pool Storage --
    UPROPERTY()
    TMap<TSubclassOf<AActor>, FPoolEntry> Pools;

    //-- Delegates --
    FOnPoolExpanded PoolExpandedDelegate;
    FOnPoolExhausted PoolExhaustedDelegate;

    //-- Default Configuration --
    static constexpr int32 DefaultInitialPoolSize = 50;
    static constexpr int32 DefaultMaxPoolSize = 500;
    static constexpr bool DefaultAutoExpand = true;

    // ~UWorldSubsystem interface
    virtual bool ShouldCreateSubsystem(UWorld* World) const override;
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    // ~End of UWorldSubsystem interface

private:
    /**
     * Create a new actor for the pool.
     *
     * @param ActorClass The actor class to create
     * @param SpawnTransform The initial transform
     * @return The created actor, or nullptr on failure
     */
    AActor* CreatePooledActor(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform);

    /**
     * Activate a pooled actor for use.
     *
     * @param Actor The actor to activate
     * @param SpawnTransform The transform to set
     */
    void ActivateActor(AActor* Actor, const FTransform& SpawnTransform);

    /**
     * Deactivate an actor for return to pool.
     *
     * @param Actor The actor to deactivate
     */
    void DeactivateActor(AActor* Actor);

    /**
     * Get or create pool entry for an actor class.
     *
     * @param ActorClass The actor class
     * @return Reference to the pool entry
     */
    FPoolEntry& GetOrCreatePoolEntry(TSubclassOf<AActor> ActorClass);

    /**
     * Clean up weak pointers in a pool entry.
     * Removes null/destroyed actors from arrays.
     *
     * @param PoolEntry The pool entry to clean
     */
    void CleanPoolEntry(FPoolEntry& PoolEntry);

    /**
     * Check if actor class is valid for pooling.
     *
     * @param ActorClass The actor class to validate
     * @return True if valid for pooling
     */
    bool IsValidPoolClass(TSubclassOf<AActor> ActorClass) const;
};
