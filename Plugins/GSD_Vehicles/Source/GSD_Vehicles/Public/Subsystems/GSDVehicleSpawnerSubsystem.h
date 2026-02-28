// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DataAssets/GSDVehicleConfig.h"
#include "Actors/GSDVehiclePawn.h"
#include "GSDVehicleSpawnerSubsystem.generated.h"

class AGSDVehiclePawn;
class UGSDVehicleConfig;
class UGSDVehiclePoolSubsystem;

/**
 * Delegate for async vehicle spawn completion.
 * Called when a vehicle has been successfully spawned from a config.
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnVehicleSpawnComplete, AGSDVehiclePawn*, SpawnedVehicle);

/**
 * Delegate for all vehicles despawned notification.
 * Called when DespawnAllVehicles completes clearing all tracked vehicles.
 */
DECLARE_DYNAMIC_DELEGATE(FOnAllVehiclesDespawned);

/**
 * World subsystem for spawning and managing GSD vehicles.
 *
 * Provides centralized vehicle spawning from GSDVehicleConfig Data Assets
 * with tracking and cleanup. Integrates with GSD spawning system and
 * World Partition streaming.
 *
 * Usage:
 * 1. Get subsystem from world: GetWorld()->GetSubsystem<UGSDVehicleSpawnerSubsystem>()
 * 2. Spawn vehicle: Subsystem->SpawnVehicle(Config, Location, Rotation)
 * 3. Track spawned vehicles: Subsystem->GetSpawnedVehicles()
 * 4. Cleanup: Subsystem->DespawnAllVehicles()
 */
UCLASS()
class GSD_VEHICLES_API UGSDVehicleSpawnerSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    /**
     * Spawn a vehicle from a config at the specified location (synchronous).
     *
     * @param Config Vehicle configuration Data Asset
     * @param Location World location to spawn at
     * @param Rotation World rotation (default: zero)
     * @return Spawned vehicle pawn, or nullptr on failure
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Vehicles")
    AGSDVehiclePawn* SpawnVehicle(UGSDVehicleConfig* Config, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    /**
     * Spawn a vehicle from a config at the specified location (asynchronous).
     *
     * @param Config Vehicle configuration Data Asset
     * @param Location World location to spawn at
     * @param Rotation World rotation
     * @param OnComplete Delegate called when spawn completes
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Vehicles")
    void SpawnVehicleAsync(UGSDVehicleConfig* Config, FVector Location, FRotator Rotation, const FOnVehicleSpawnComplete& OnComplete);

    /**
     * Despawn a specific vehicle.
     *
     * @param Vehicle Vehicle to despawn
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Vehicles")
    void DespawnVehicle(AGSDVehiclePawn* Vehicle);

    /**
     * Despawn all tracked vehicles.
     * Broadcasts OnAllVehiclesDespawned delegate when complete.
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Vehicles")
    void DespawnAllVehicles();

    //-- Pooling Support --

    /**
     * Spawn vehicle using pool if available, falls back to regular spawn.
     *
     * @param Config Vehicle configuration Data Asset
     * @param Location World location to spawn at
     * @param Rotation World rotation
     * @return Spawned vehicle pawn, or nullptr on failure
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Vehicles")
    AGSDVehiclePawn* SpawnVehicleFromPool(UGSDVehicleConfig* Config, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    /**
     * Return vehicle to pool instead of destroying.
     *
     * @param Vehicle Vehicle to return to pool
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Vehicles")
    void ReturnVehicleToPool(AGSDVehiclePawn* Vehicle);

    /**
     * Get all currently spawned vehicles.
     *
     * @return Array of spawned vehicle pawns
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Vehicles")
    const TArray<TObjectPtr<AGSDVehiclePawn>>& GetSpawnedVehicles() const { return SpawnedVehicles; }

    /**
     * Get the delegate for all vehicles despawned notification.
     * Bind to receive notification when DespawnAllVehicles completes.
     *
     * @return Reference to the delegate
     */
    FOnAllVehiclesDespawned& GetOnAllVehiclesDespawned() { return AllVehiclesDespawnedDelegate; }

    //-- Network Validation (GSDNETWORK-107) --
    // These validation functions can be called from Server RPCs to prevent exploitable inputs.
    // Subsystems cannot have Server RPCs directly, only AActor/UActorComponent can.
    // These validation functions provide the security layer that Server RPCs would call.

    /**
     * Validate spawn parameters for network safety.
     * Call from Server RPCs before calling SpawnVehicle/SpawnVehicleFromPool.
     *
     * @param Config Vehicle configuration Data Asset
     * @param Location World location to spawn at
     * @param OutError Error message if validation fails
     * @return True if parameters are valid and safe
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Vehicles|Network")
    bool ValidateSpawnParameters(UGSDVehicleConfig* Config, FVector Location, FString& OutError) const;

    /**
     * Validate return to pool parameters for network safety.
     * Call from Server RPCs before calling ReturnVehicleToPool.
     *
     * @param Vehicle Vehicle to return to pool
     * @param OutError Error message if validation fails
     * @return True if parameters are valid and safe
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Vehicles|Network")
    bool ValidateReturnToPool(AGSDVehiclePawn* Vehicle, FString& OutError) const;

    /**
     * Get maximum allowed pool size for validation.
     */
    static constexpr int32 GetMaxPoolSize() { return MaxPoolSize; }

protected:
    //-- Validation Constants (GSDNETWORK-107) --
    // Maximum pool size to prevent server overload exploits
    static constexpr int32 MaxPoolSize = 50;
    // Maximum world extent to prevent extreme location exploits
    static constexpr float MaxWorldExtent = 1000000.0f; // 1km from origin

    /** Array of all spawned vehicles being tracked */
    UPROPERTY()
    TArray<TObjectPtr<AGSDVehiclePawn>> SpawnedVehicles;

    /** Delegate broadcast when all vehicles are despawned */
    FOnAllVehiclesDespawned AllVehiclesDespawnedDelegate;

    // ~UWorldSubsystem interface
    virtual bool ShouldCreateSubsystem(UWorld* World) const override;
    // ~End of UWorldSubsystem interface

private:
    /** Cached pool subsystem reference (lazy initialization) */
    UPROPERTY()
    TObjectPtr<UGSDVehiclePoolSubsystem> PoolSubsystem;

    /**
     * Get pool subsystem lazily (handles initialization order).
     *
     * @return Pool subsystem, or nullptr if not available
     */
    UGSDVehiclePoolSubsystem* GetPoolSubsystem();
};
