// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GSDCrowdTestbedActor.generated.h"

class UGSDCrowdEntityConfig;
class UGSDCrowdManagerSubsystem;
class AGSDHeroNPC;

/**
 * Crowd testbed actor for validation and performance testing.
 *
 * Spawns 200+ (configurable) crowd entities in a defined area for testing:
 * - Mass Entity spawning performance
 * - FPS tracking with entity count
 * - LOD representation switching validation
 * - Deterministic spawning with seeded randomization
 *
 * Usage:
 * 1. Place actor in level
 * 2. Configure NumEntitiesToSpawn (default: 200)
 * 3. Call SpawnTestEntities() or enable bAutoSpawnOnBeginPlay
 * 4. Monitor FPS and entity count
 * 5. Verify LOD switching at different camera distances
 */
UCLASS(BlueprintType, Blueprintable, Category = "GSD|Crowds")
class GSD_CROWDS_API AGSDCrowdTestbedActor : public AActor
{
    GENERATED_BODY()

public:
    AGSDCrowdTestbedActor();

    //-- Configuration --

    /** Entity config to use for spawning (uses default if null) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testbed")
    TObjectPtr<UGSDCrowdEntityConfig> EntityConfig;

    /** Number of entities to spawn */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testbed", meta = (ClampMin = "1", ClampMax = "1000"))
    int32 NumEntitiesToSpawn = 200;

    /** Spawn area radius (entities placed randomly within circle) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testbed", meta = (ClampMin = "100.0", ClampMax = "50000.0"))
    float SpawnRadius = 10000.0f;

    /** Auto-spawn on BeginPlay */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testbed")
    bool bAutoSpawnOnBeginPlay = false;

    /** Seed for reproducible spawns */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testbed", meta = (EditCondition = "bUseDeterministicSeed"))
    int32 RandomSeed = 42;

    /** Whether to use deterministic seed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testbed")
    bool bUseDeterministicSeed = true;

    /** Target FPS for performance validation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testbed")
    float TargetFPS = 60.0f;

    /** Log warning if FPS drops below target */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testbed")
    bool bLogPerformanceWarnings = true;

    /** Performance warning threshold (percentage below target FPS) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testbed", meta = (ClampMin = "0.0", ClampMax = "50.0"))
    float PerformanceWarningThreshold = 10.0f;

    //-- Phase 7: AI Testing Configuration --
    /** Enable navigation testing */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GSD|AI")
    bool bEnableNavigation = true;

    /** Enable Smart Object testing */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GSD|AI")
    bool bEnableSmartObjects = true;

    /** Number of Hero NPCs to spawn for AI testing */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GSD|AI", meta = (ClampMin = "0", ClampMax = "50"))
    int32 HeroNPCCount = 3;

    /** Hero NPC class to spawn */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GSD|AI")
    TSubclassOf<AGSDHeroNPC> HeroNPCClass;

    //-- AI Statistics (Read-only) --
    /** Number of entities currently on navigation lanes */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GSD|AI|Stats")
    int32 EntitiesOnLanes = 0;

    /** Number of entities currently interacting with Smart Objects */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GSD|AI|Stats")
    int32 EntitiesInteracting = 0;

    /** Number of active Hero NPCs */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GSD|AI|Stats")
    int32 ActiveHeroNPCs = 0;

    //-- Actions --

    /** Spawn all test entities */
    UFUNCTION(BlueprintCallable, Category = "Testbed")
    void SpawnTestEntities();

    /** Despawn all test entities */
    UFUNCTION(BlueprintCallable, Category = "Testbed")
    void DespawnTestEntities();

    /** Respawn with new random positions */
    UFUNCTION(BlueprintCallable, Category = "Testbed")
    void RespawnTestEntities();

    //-- Statistics --

    /** Get current FPS */
    UFUNCTION(BlueprintPure, Category = "Testbed")
    float GetCurrentFPS() const { return CurrentFPS; }

    /** Get spawned entity count */
    UFUNCTION(BlueprintPure, Category = "Testbed")
    int32 GetSpawnedEntityCount() const;

    /** Get average frame time over last 60 frames */
    UFUNCTION(BlueprintPure, Category = "Testbed")
    float GetAverageFrameTime() const;

    /** Get average FPS (1.0 / GetAverageFrameTime()) */
    UFUNCTION(BlueprintPure, Category = "Testbed")
    float GetAverageFPS() const;

    /** Check if performance meets target */
    UFUNCTION(BlueprintPure, Category = "Testbed")
    bool IsPerformanceAcceptable() const;

protected:
    // ~AActor interface
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    // ~End of AActor interface

    //-- Hero NPC Management --
    /** Spawned Hero NPCs for AI testing */
    UPROPERTY()
    TArray<TObjectPtr<AGSDHeroNPC>> SpawnedHeroNPCs;

    //-- AI Debug Methods --
    /** Update AI statistics from Mass Entity system */
    void UpdateAIStatistics();

    /** Draw navigation debug visualization */
    void DrawNavigationDebug();

    /** Spawn Hero NPCs for AI testing */
    void SpawnHeroNPCs();

    /** Despawn all Hero NPCs */
    void DespawnHeroNPCs();

private:
    /** Cached crowd manager subsystem */
    UPROPERTY()
    TObjectPtr<UGSDCrowdManagerSubsystem> CrowdManager;

    /** Current FPS calculated from delta time */
    float CurrentFPS = 0.0f;

    /** Frame time history for circular buffer averaging */
    TArray<float> FrameTimeHistory;

    /** Current write index in circular buffer */
    int32 FrameHistoryIndex = 0;

    /** Running sum for O(1) average calculation */
    float FrameTimeSum = 0.0f;

    /** Maximum frames to keep in history */
    static constexpr int32 MaxFrameTimeHistory = 60;

    /** Last performance warning time (for throttling) */
    float LastPerformanceWarningTime = 0.0f;

    /** Performance warning cooldown (seconds) */
    float PerformanceWarningCooldown = 5.0f;

    /**
     * Get crowd manager subsystem (lazy initialization).
     *
     * @return Crowd manager, or nullptr if not available
     */
    UGSDCrowdManagerSubsystem* GetCrowdManager();

    /**
     * Update FPS metrics from delta time.
     *
     * @param DeltaTime Frame delta time
     */
    void UpdateFPSMetrics(float DeltaTime);

    /**
     * Check and log performance warnings.
     */
    void CheckPerformanceWarnings();
};
