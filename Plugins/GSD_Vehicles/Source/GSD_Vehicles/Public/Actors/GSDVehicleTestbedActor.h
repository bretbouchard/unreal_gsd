// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GSDVehicleTestbedActor.generated.h"

class UGSDVehicleConfig;
class AGSDVehiclePawn;

/**
 * Vehicle testbed actor for validation and performance testing.
 *
 * Spawns 50 (configurable) randomized vehicles in a grid for testing:
 * - Vehicle spawning and pooling performance
 * - FPS tracking with vehicle count
 * - Deterministic spawning with seeded randomization
 *
 * Usage:
 * 1. Place actor in level
 * 2. Assign VehicleConfigs array
 * 3. Call SpawnTestVehicles() or enable bAutoSpawnOnBeginPlay
 * 4. Monitor FPS and vehicle count
 */
UCLASS(BlueprintType, Blueprintable, Category = "GSD|Vehicles")
class GSD_VEHICLES_API AGSDVehicleTestbedActor : public AActor
{
    GENERATED_BODY()

public:
    AGSDVehicleTestbedActor();

    //-- Configuration --

    /** Vehicle configs to randomly select from */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testbed")
    TArray<TSoftObjectPtr<UGSDVehicleConfig>> VehicleConfigs;

    /** Number of vehicles to spawn */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testbed", meta = (ClampMin = "1", ClampMax = "200"))
    int32 NumVehiclesToSpawn = 50;

    /** Spawn area dimensions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testbed")
    FVector SpawnAreaSize = FVector(10000.0f, 10000.0f, 0.0f);

    /** Whether to use vehicle pooling */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testbed")
    bool bUsePooling = true;

    /** Pool warmup size per config */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testbed", meta = (ClampMin = "1", ClampMax = "100"))
    int32 PoolSizePerConfig = 20;

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

    //-- Actions --

    /** Spawn all test vehicles */
    UFUNCTION(BlueprintCallable, Category = "Testbed")
    void SpawnTestVehicles();

    /** Despawn all test vehicles */
    UFUNCTION(BlueprintCallable, Category = "Testbed")
    void DespawnTestVehicles();

    /** Respawn with new random positions */
    UFUNCTION(BlueprintCallable, Category = "Testbed")
    void RespawnTestVehicles();

    //-- Statistics --

    /** Get current FPS */
    UFUNCTION(BlueprintPure, Category = "Testbed")
    float GetCurrentFPS() const { return CurrentFPS; }

    /** Get spawned vehicle count */
    UFUNCTION(BlueprintPure, Category = "Testbed")
    int32 GetSpawnedVehicleCount() const { return SpawnedVehicles.Num(); }

    /** Get average frame time over last 60 frames */
    UFUNCTION(BlueprintPure, Category = "Testbed")
    float GetAverageFrameTime() const;

    /** Get average FPS (1.0 / GetAverageFrameTime()) */
    UFUNCTION(BlueprintPure, Category = "Testbed")
    float GetAverageFPS() const;

protected:
    // ~AActor interface
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    // ~End of AActor interface

private:
    /** Spawned vehicles */
    UPROPERTY()
    TArray<TObjectPtr<AGSDVehiclePawn>> SpawnedVehicles;

    /** Current FPS calculated from delta time */
    float CurrentFPS = 0.0f;

    /** Frame time history for averaging */
    TArray<float> FrameTimeHistory;

    /** Maximum frames to keep in history */
    int32 MaxFrameTimeHistory = 60;
};
