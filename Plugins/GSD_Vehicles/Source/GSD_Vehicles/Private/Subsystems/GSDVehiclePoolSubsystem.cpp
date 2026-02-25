// Copyright Bret Bouchard. All Rights Reserved.

#include "Subsystems/GSDVehiclePoolSubsystem.h"
#include "Actors/GSDVehiclePawn.h"
#include "DataAssets/GSDVehicleConfig.h"
#include "GSDVehicleLog.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "ChaosWheeledVehicleMovementComponent.h"

bool UGSDVehiclePoolSubsystem::ShouldCreateSubsystem(UWorld* World) const
{
    // Only create subsystem for game worlds (not editor preview worlds)
    return World ? World->IsGameWorld() : false;
}

void UGSDVehiclePoolSubsystem::WarmUpPool(UGSDVehicleConfig* Config, int32 PoolSize)
{
    if (!Config)
    {
        GSD_VEHICLE_ERROR(TEXT("WarmUpPool: Config is null"));
        return;
    }

    if (PoolSize <= 0)
    {
        GSD_VEHICLE_WARN(TEXT("WarmUpPool: Invalid pool size %d for config '%s'"), PoolSize, *Config->GetName());
        return;
    }

    // Get or create pool array
    TArray<TObjectPtr<AGSDVehiclePawn>>& Pool = AvailablePools.FindOrAdd(Config);

    // Calculate how many more vehicles needed
    int32 Needed = PoolSize - Pool.Num();
    if (Needed <= 0)
    {
        GSD_VEHICLE_LOG(Log, TEXT("WarmUpPool: Pool already has %d vehicles for config '%s' (requested %d)"),
            Pool.Num(), *Config->GetName(), PoolSize);
        return;
    }

    GSD_VEHICLE_LOG(Log, TEXT("WarmUpPool: Creating %d vehicles for config '%s'"), Needed, *Config->GetName());

    // Create vehicles for pool
    int32 Created = 0;
    for (int32 i = 0; i < Needed; ++i)
    {
        AGSDVehiclePawn* NewVehicle = CreateNewPooledVehicle(Config);
        if (NewVehicle)
        {
            Pool.Add(NewVehicle);
            ++Created;
        }
    }

    GSD_VEHICLE_LOG(Log, TEXT("WarmUpPool: Created %d/%d vehicles for config '%s' (pool size now %d)"),
        Created, Needed, *Config->GetName(), Pool.Num());

    // Broadcast completion
    PoolWarmupCompleteDelegate.Broadcast(Config, Pool.Num());
}

AGSDVehiclePawn* UGSDVehiclePoolSubsystem::AcquireVehicle(UGSDVehicleConfig* Config, FVector Location, FRotator Rotation)
{
    if (!Config)
    {
        GSD_VEHICLE_ERROR(TEXT("AcquireVehicle: Config is null"));
        return nullptr;
    }

    AGSDVehiclePawn* Vehicle = nullptr;

    // Try to get from available pool
    TArray<TObjectPtr<AGSDVehiclePawn>>* Pool = AvailablePools.Find(Config);
    if (Pool && Pool->Num() > 0)
    {
        // Pop from end of array
        Vehicle = Pool->Pop();
        GSD_VEHICLE_LOG(Log, TEXT("AcquireVehicle: Retrieved vehicle from pool for config '%s' (remaining: %d)"),
            *Config->GetName(), Pool->Num());
    }
    else
    {
        // Pool empty, create new vehicle
        Vehicle = CreateNewPooledVehicle(Config);
        if (!Vehicle)
        {
            GSD_VEHICLE_ERROR(TEXT("AcquireVehicle: Failed to create new vehicle for config '%s'"), *Config->GetName());
            return nullptr;
        }
        GSD_VEHICLE_LOG(Log, TEXT("AcquireVehicle: Created new vehicle for config '%s' (pool was empty)"), *Config->GetName());
    }

    // Activate vehicle
    Vehicle->SetActorLocationAndRotation(Location, Rotation, false, nullptr, ETeleportType::ResetPhysics);
    Vehicle->SetActorHiddenInGame(false);
    Vehicle->SetActorEnableCollision(true);

    // Re-enable physics on mesh
    if (USkeletalMeshComponent* Mesh = Vehicle->GetMesh())
    {
        Mesh->SetSimulatePhysics(true);
        Mesh->WakeAllRigidBodies();
    }

    // Add to active vehicles
    ActiveVehicles.Add(Vehicle);

    GSD_VEHICLE_LOG(Log, TEXT("AcquireVehicle: Activated vehicle '%s' at %s"),
        *Vehicle->GetName(), *Location.ToString());

    return Vehicle;
}

void UGSDVehiclePoolSubsystem::ReleaseVehicle(AGSDVehiclePawn* Vehicle)
{
    if (!Vehicle)
    {
        GSD_VEHICLE_WARN(TEXT("ReleaseVehicle: Vehicle is null"));
        return;
    }

    // Remove from active vehicles
    ActiveVehicles.Remove(Vehicle);

    // Reset vehicle for pooling
    ResetVehicleForPool(Vehicle);

    // Get config from vehicle
    UGSDVehicleConfig* Config = Cast<UGSDVehicleConfig>(Vehicle->GetSpawnConfig());
    if (Config)
    {
        // Add to available pool
        TArray<TObjectPtr<AGSDVehiclePawn>>& Pool = AvailablePools.FindOrAdd(Config);
        Pool.Add(Vehicle);

        GSD_VEHICLE_LOG(Log, TEXT("ReleaseVehicle: Returned vehicle '%s' to pool for config '%s' (pool size: %d)"),
            *Vehicle->GetName(), *Config->GetName(), Pool.Num());
    }
    else
    {
        // No config - destroy the vehicle
        GSD_VEHICLE_WARN(TEXT("ReleaseVehicle: Vehicle '%s' has no config, destroying"), *Vehicle->GetName());
        Vehicle->Destroy();
    }
}

int32 UGSDVehiclePoolSubsystem::GetAvailableCount(UGSDVehicleConfig* Config) const
{
    if (!Config)
    {
        return 0;
    }

    const TArray<TObjectPtr<AGSDVehiclePawn>>* Pool = AvailablePools.Find(Config);
    return Pool ? Pool->Num() : 0;
}

void UGSDVehiclePoolSubsystem::ClearAllPools()
{
    int32 TotalDestroyed = 0;

    // Destroy all available pooled vehicles
    for (auto& Pair : AvailablePools)
    {
        for (AGSDVehiclePawn* Vehicle : Pair.Value)
        {
            if (Vehicle)
            {
                Vehicle->Destroy();
                ++TotalDestroyed;
            }
        }
    }
    AvailablePools.Empty();

    // Destroy all active vehicles
    for (AGSDVehiclePawn* Vehicle : ActiveVehicles)
    {
        if (Vehicle)
        {
            Vehicle->Destroy();
            ++TotalDestroyed;
        }
    }
    ActiveVehicles.Empty();

    GSD_VEHICLE_LOG(Log, TEXT("ClearAllPools: Destroyed %d vehicles"), TotalDestroyed);
}

void UGSDVehiclePoolSubsystem::GetPoolStatistics(int32& OutTotalPooled, int32& OutTotalActive) const
{
    OutTotalPooled = 0;
    OutTotalActive = ActiveVehicles.Num();

    for (const auto& Pair : AvailablePools)
    {
        OutTotalPooled += Pair.Value.Num();
    }
}

void UGSDVehiclePoolSubsystem::ResetVehicleForPool(AGSDVehiclePawn* Vehicle)
{
    if (!Vehicle)
    {
        return;
    }

    USkeletalMeshComponent* Mesh = Vehicle->GetMesh();
    if (!Mesh)
    {
        GSD_VEHICLE_WARN(TEXT("ResetVehicleForPool: Vehicle '%s' has no mesh"), *Vehicle->GetName());
        return;
    }

    // Disable physics first
    Mesh->SetSimulatePhysics(false);

    // Clear all velocities
    Mesh->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
    Mesh->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

    // Reset vehicle movement component inputs
    if (UChaosWheeledVehicleMovementComponent* Movement = Vehicle->GetVehicleMovement())
    {
        Movement->SetThrottleInput(0.0f);
        Movement->SetSteeringInput(0.0f);
        Movement->SetBrakeInput(1.0f);
        Movement->SetHandbrakeInput(true);
    }

    // Reset mesh relative transform (standard vehicle mesh offset)
    // AWheeledVehiclePawn typically has mesh at (0, 0, 0) with rotation (0, -90, 0)
    Mesh->SetRelativeLocation(FVector::ZeroVector);
    Mesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

    // Reset all bodies and recreate physics state
    Mesh->ResetAllBodiesSimulatePhysics();
    Mesh->RecreatePhysicsState();

    // Hide and disable collision
    Vehicle->SetActorHiddenInGame(true);
    Vehicle->SetActorEnableCollision(false);

    // Reset spawn state (clears config reference, etc.)
    Vehicle->ResetSpawnState();

    GSD_VEHICLE_LOG(Verbose, TEXT("ResetVehicleForPool: Reset vehicle '%s' for pooling"), *Vehicle->GetName());
}

AGSDVehiclePawn* UGSDVehiclePoolSubsystem::CreateNewPooledVehicle(UGSDVehicleConfig* Config)
{
    if (!Config)
    {
        return nullptr;
    }

    // Validate config
    FString ValidationError;
    if (!Config->ValidateConfig(ValidationError))
    {
        GSD_VEHICLE_ERROR(TEXT("CreateNewPooledVehicle: Config validation failed for '%s': %s"),
            *Config->GetName(), *ValidationError);
        return nullptr;
    }

    // Spawn vehicle at zero location
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AGSDVehiclePawn* NewVehicle = GetWorld()->SpawnActor<AGSDVehiclePawn>(
        AGSDVehiclePawn::StaticClass(),
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        SpawnParams);

    if (!NewVehicle)
    {
        GSD_VEHICLE_ERROR(TEXT("CreateNewPooledVehicle: Failed to spawn vehicle for config '%s'"), *Config->GetName());
        return nullptr;
    }

    // Apply configuration
    NewVehicle->SpawnFromConfig(Config);

    // Immediately reset for pooling (hides and disables collision)
    ResetVehicleForPool(NewVehicle);

    GSD_VEHICLE_LOG(Verbose, TEXT("CreateNewPooledVehicle: Created pooled vehicle '%s' for config '%s'"),
        *NewVehicle->GetName(), *Config->GetName());

    return NewVehicle;
}
