// Copyright Bret Bouchard. All Rights Reserved.

#include "Subsystems/GSDVehicleSpawnerSubsystem.h"
#include "Subsystems/GSDVehiclePoolSubsystem.h"
#include "Actors/GSDVehiclePawn.h"
#include "DataAssets/GSDVehicleConfig.h"
#include "GSDVehicleLog.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

bool UGSDVehicleSpawnerSubsystem::ShouldCreateSubsystem(UWorld* World) const
{
    // Only create subsystem for game worlds (not editor preview worlds)
    return World ? World->IsGameWorld() : false;
}

UGSDVehiclePoolSubsystem* UGSDVehicleSpawnerSubsystem::GetPoolSubsystem()
{
    if (!PoolSubsystem && GetWorld())
    {
        PoolSubsystem = GetWorld()->GetSubsystem<UGSDVehiclePoolSubsystem>();
    }
    return PoolSubsystem;
}

AGSDVehiclePawn* UGSDVehicleSpawnerSubsystem::SpawnVehicle(UGSDVehicleConfig* Config, FVector Location, FRotator Rotation)
{
    // Validate config is not null
    if (!Config)
    {
        GSD_VEHICLE_ERROR(TEXT("SpawnVehicle: Config is null"));
        return nullptr;
    }

    // Validate config
    FString ValidationError;
    if (!Config->ValidateConfig(ValidationError))
    {
        GSD_VEHICLE_ERROR(TEXT("SpawnVehicle: Config validation failed for '%s': %s"), *Config->GetName(), *ValidationError);
        return nullptr;
    }

    // Spawn the vehicle
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AGSDVehiclePawn* SpawnedVehicle = GetWorld()->SpawnActor<AGSDVehiclePawn>(AGSDVehiclePawn::StaticClass(), Location, Rotation, SpawnParams);
    if (!SpawnedVehicle)
    {
        GSD_VEHICLE_ERROR(TEXT("SpawnVehicle: Failed to spawn vehicle from config '%s'"), *Config->GetName());
        return nullptr;
    }

    // Apply configuration to the spawned vehicle
    SpawnedVehicle->SpawnFromConfig(Config);

    // Track the spawned vehicle
    SpawnedVehicles.Add(SpawnedVehicle);

    GSD_VEHICLE_LOG(Log, TEXT("SpawnVehicle: Successfully spawned vehicle '%s' from config '%s' at %s"),
        *SpawnedVehicle->GetName(), *Config->GetName(), *Location.ToString());

    return SpawnedVehicle;
}

void UGSDVehicleSpawnerSubsystem::SpawnVehicleAsync(UGSDVehicleConfig* Config, FVector Location, FRotator Rotation, const FOnVehicleSpawnComplete& OnComplete)
{
    // Validate config is not null
    if (!Config)
    {
        GSD_VEHICLE_ERROR(TEXT("SpawnVehicleAsync: Config is null"));
        if (OnComplete.IsBound())
        {
            OnComplete.Execute(nullptr);
        }
        return;
    }

    // Validate config
    FString ValidationError;
    if (!Config->ValidateConfig(ValidationError))
    {
        GSD_VEHICLE_ERROR(TEXT("SpawnVehicleAsync: Config validation failed for '%s': %s"), *Config->GetName(), *ValidationError);
        if (OnComplete.IsBound())
        {
            OnComplete.Execute(nullptr);
        }
        return;
    }

    // Spawn the vehicle
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AGSDVehiclePawn* SpawnedVehicle = GetWorld()->SpawnActor<AGSDVehiclePawn>(AGSDVehiclePawn::StaticClass(), Location, Rotation, SpawnParams);
    if (!SpawnedVehicle)
    {
        GSD_VEHICLE_ERROR(TEXT("SpawnVehicleAsync: Failed to spawn vehicle from config '%s'"), *Config->GetName());
        if (OnComplete.IsBound())
        {
            OnComplete.Execute(nullptr);
        }
        return;
    }

    // Apply configuration to the spawned vehicle
    SpawnedVehicle->SpawnFromConfig(Config);

    // Track the spawned vehicle
    SpawnedVehicles.Add(SpawnedVehicle);

    GSD_VEHICLE_LOG(Log, TEXT("SpawnVehicleAsync: Successfully spawned vehicle '%s' from config '%s' at %s"),
        *SpawnedVehicle->GetName(), *Config->GetName(), *Location.ToString());

    // Execute completion delegate
    if (OnComplete.IsBound())
    {
        OnComplete.Execute(SpawnedVehicle);
    }
}

void UGSDVehicleSpawnerSubsystem::DespawnVehicle(AGSDVehiclePawn* Vehicle)
{
    if (!Vehicle)
    {
        GSD_VEHICLE_WARN(TEXT("DespawnVehicle: Vehicle is null"));
        return;
    }

    // Remove from tracked vehicles array
    SpawnedVehicles.Remove(Vehicle);

    // Call despawn to clean up state
    Vehicle->Despawn();

    // Destroy the actor
    Vehicle->Destroy();

    GSD_VEHICLE_LOG(Log, TEXT("DespawnVehicle: Successfully despawned vehicle '%s'"), *Vehicle->GetName());
}

void UGSDVehicleSpawnerSubsystem::DespawnAllVehicles()
{
    const int32 NumVehicles = SpawnedVehicles.Num();

    // Despawn and destroy each tracked vehicle
    for (AGSDVehiclePawn* Vehicle : SpawnedVehicles)
    {
        if (Vehicle)
        {
            Vehicle->Despawn();
            Vehicle->Destroy();
        }
    }

    // Clear the tracked vehicles array
    SpawnedVehicles.Empty();

    // Broadcast notification that all vehicles have been despawned
    AllVehiclesDespawnedDelegate.Broadcast();

    GSD_VEHICLE_LOG(Log, TEXT("DespawnAllVehicles: Successfully despawned %d vehicles"), NumVehicles);
}

AGSDVehiclePawn* UGSDVehicleSpawnerSubsystem::SpawnVehicleFromPool(UGSDVehicleConfig* Config, FVector Location, FRotator Rotation)
{
    UGSDVehiclePoolSubsystem* Pool = GetPoolSubsystem();

    if (Pool)
    {
        AGSDVehiclePawn* Vehicle = Pool->AcquireVehicle(Config, Location, Rotation);
        if (Vehicle)
        {
            SpawnedVehicles.Add(Vehicle);
            return Vehicle;
        }
    }

    // Fallback to regular spawn if pool not available or failed
    return SpawnVehicle(Config, Location, Rotation);
}

void UGSDVehicleSpawnerSubsystem::ReturnVehicleToPool(AGSDVehiclePawn* Vehicle)
{
    if (!Vehicle)
    {
        return;
    }

    // Remove from tracked vehicles
    SpawnedVehicles.Remove(Vehicle);

    UGSDVehiclePoolSubsystem* Pool = GetPoolSubsystem();

    if (Pool)
    {
        Pool->ReleaseVehicle(Vehicle);
    }
    else
    {
        // Fallback to despawn if pool not available
        DespawnVehicle(Vehicle);
    }
}

//-- Network Validation (GSDNETWORK-107) --

bool UGSDVehicleSpawnerSubsystem::ValidateSpawnParameters(UGSDVehicleConfig* Config, FVector Location, FString& OutError) const
{
    OutError.Empty();

    // Validate config is not null
    if (!Config)
    {
        OutError = TEXT("Vehicle config is null");
        GSD_VEHICLE_WARN(TEXT("ValidateSpawnParameters: %s"), *OutError);
        return false;
    }

    // Validate location is finite (not NaN or Inf)
    if (!FMath::IsFinite(Location.X) || !FMath::IsFinite(Location.Y) || !FMath::IsFinite(Location.Z))
    {
        OutError = FString::Printf(TEXT("Invalid spawn location: %s (contains NaN or Inf)"), *Location.ToString());
        GSD_VEHICLE_WARN(TEXT("ValidateSpawnParameters: %s"), *OutError);
        return false;
    }

    // Validate location magnitude (prevent extreme values)
    if (FMath::Abs(Location.X) > MaxWorldExtent || FMath::Abs(Location.Y) > MaxWorldExtent || FMath::Abs(Location.Z) > MaxWorldExtent)
    {
        OutError = FString::Printf(TEXT("Spawn location %s exceeds world bounds (max %.0f)"), *Location.ToString(), MaxWorldExtent);
        GSD_VEHICLE_WARN(TEXT("ValidateSpawnParameters: %s"), *OutError);
        return false;
    }

    // Validate pool capacity if using pool
    if (SpawnedVehicles.Num() >= MaxPoolSize)
    {
        OutError = FString::Printf(TEXT("Vehicle pool at capacity (%d/%d)"), SpawnedVehicles.Num(), MaxPoolSize);
        GSD_VEHICLE_WARN(TEXT("ValidateSpawnParameters: %s"), *OutError);
        return false;
    }

    return true;
}

bool UGSDVehicleSpawnerSubsystem::ValidateReturnToPool(AGSDVehiclePawn* Vehicle, FString& OutError) const
{
    OutError.Empty();

    // Validate vehicle is not null
    if (!Vehicle)
    {
        OutError = TEXT("Vehicle is null");
        GSD_VEHICLE_WARN(TEXT("ValidateReturnToPool: %s"), *OutError);
        return false;
    }

    // Validate vehicle is in our tracked list (prevent returning arbitrary vehicles)
    if (!SpawnedVehicles.Contains(Vehicle))
    {
        OutError = FString::Printf(TEXT("Vehicle '%s' is not tracked by this spawner"), *Vehicle->GetName());
        GSD_VEHICLE_WARN(TEXT("ValidateReturnToPool: %s"), *OutError);
        return false;
    }

    return true;
}
