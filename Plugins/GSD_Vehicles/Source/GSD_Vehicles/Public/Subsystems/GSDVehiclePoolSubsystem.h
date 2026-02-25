// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DataAssets/GSDVehicleConfig.h"
#include "Actors/GSDVehiclePawn.h"
#include "GSDVehiclePoolSubsystem.generated.h"

class AGSDVehiclePawn;
class UGSDVehicleConfig;

/**
 * Delegate for pool warmup completion.
 * Called when WarmUpPool completes creating the requested number of vehicles.
 */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnPoolWarmupComplete, UGSDVehicleConfig*, Config, int32, PoolSize);

/**
 * World subsystem for vehicle pooling that manages vehicle reuse with proper physics reset.
 *
 * Improves performance for spawn-heavy scenarios (traffic, AI vehicles) by reusing
 * vehicle instances instead of spawning/destroying. Pooled vehicles are hidden
 * and have collision disabled until acquired.
 *
 * Usage:
 * 1. Get subsystem from world: GetWorld()->GetSubsystem<UGSDVehiclePoolSubsystem>()
 * 2. Warm up pool: Subsystem->WarmUpPool(Config, 20)
 * 3. Acquire vehicle: Subsystem->AcquireVehicle(Config, Location, Rotation)
 * 4. Release vehicle: Subsystem->ReleaseVehicle(Vehicle)
 * 5. Clear pools: Subsystem->ClearAllPools()
 */
UCLASS()
class GSD_VEHICLES_API UGSDVehiclePoolSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    /**
     * Pre-spawn vehicles into pool for the specified config.
     * Creates hidden, collision-disabled vehicles ready for acquisition.
     *
     * @param Config Vehicle configuration Data Asset
     * @param PoolSize Number of vehicles to have in pool (adds more if needed)
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Vehicles")
    void WarmUpPool(UGSDVehicleConfig* Config, int32 PoolSize);

    /**
     * Get a vehicle from pool (creates new if pool empty).
     * Activates vehicle at specified location with physics enabled.
     *
     * @param Config Vehicle configuration Data Asset
     * @param Location World location to place vehicle
     * @param Rotation World rotation
     * @return Activated vehicle pawn, or nullptr on failure
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Vehicles")
    AGSDVehiclePawn* AcquireVehicle(UGSDVehicleConfig* Config, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    /**
     * Return a vehicle to pool with physics reset.
     * Vehicle becomes hidden and collision-disabled.
     *
     * @param Vehicle Vehicle to return to pool
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Vehicles")
    void ReleaseVehicle(AGSDVehiclePawn* Vehicle);

    /**
     * Get count of available pooled vehicles for a config.
     *
     * @param Config Vehicle configuration to check
     * @return Number of vehicles available in pool
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Vehicles")
    int32 GetAvailableCount(UGSDVehicleConfig* Config) const;

    /**
     * Clear all pooled vehicles (both available and active).
     * Destroys all vehicles managed by this subsystem.
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Vehicles")
    void ClearAllPools();

    /**
     * Get pool statistics for debugging.
     *
     * @param OutTotalPooled Total vehicles in available pools
     * @param OutTotalActive Total vehicles currently in use
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Vehicles")
    void GetPoolStatistics(int32& OutTotalPooled, int32& OutTotalActive) const;

    /**
     * Get the delegate for pool warmup completion notification.
     * Bind to receive notification when WarmUpPool completes.
     *
     * @return Reference to the delegate
     */
    FOnPoolWarmupComplete& GetOnPoolWarmupComplete() { return PoolWarmupCompleteDelegate; }

protected:
    /** Map of config to available pooled vehicles */
    UPROPERTY()
    TMap<TObjectPtr<UGSDVehicleConfig>, TArray<TObjectPtr<AGSDVehiclePawn>>> AvailablePools;

    /** Array of currently in-use vehicles */
    UPROPERTY()
    TArray<TObjectPtr<AGSDVehiclePawn>> ActiveVehicles;

    /** Delegate broadcast when pool warmup completes */
    FOnPoolWarmupComplete PoolWarmupCompleteDelegate;

    // ~UWorldSubsystem interface
    virtual bool ShouldCreateSubsystem(UWorld* World) const override;
    // ~End of UWorldSubsystem interface

private:
    /**
     * Reset physics state for pooling.
     * Clears velocities, resets body states, recreates physics.
     *
     * @param Vehicle Vehicle to reset
     */
    void ResetVehicleForPool(AGSDVehiclePawn* Vehicle);

    /**
     * Create a new vehicle for pool (hidden, collision-disabled).
     *
     * @param Config Vehicle configuration
     * @return New vehicle ready for pooling, or nullptr on failure
     */
    AGSDVehiclePawn* CreateNewPooledVehicle(UGSDVehicleConfig* Config);
};
