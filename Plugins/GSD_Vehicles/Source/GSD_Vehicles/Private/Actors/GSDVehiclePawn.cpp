// Copyright Bret Bouchard. All Rights Reserved.

#include "Actors/GSDVehiclePawn.h"
#include "DataAssets/GSDVehicleConfig.h"
#include "DataAssets/GSDWheelConfig.h"
#include "GSDVehicleLog.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "GameFramework/Controller.h"

AGSDVehiclePawn::AGSDVehiclePawn()
{
    // Create streaming source component for World Partition integration
    StreamingSource = CreateDefaultSubobject<UGSDStreamingSourceComponent>(TEXT("StreamingSource"));

    bIsSpawned = false;
}

void AGSDVehiclePawn::SpawnFromConfig(UGSDDataAsset* Config)
{
    UGSDVehicleConfig* VehicleConfigPtr = Cast<UGSDVehicleConfig>(Config);
    if (!VehicleConfigPtr)
    {
        GSD_VEHICLE_ERROR(TEXT("SpawnFromConfig: Invalid vehicle config - expected UGSDVehicleConfig"));
        return;
    }

    // Store the config
    VehicleConfig = VehicleConfigPtr;

    // Apply all vehicle settings from config
    ApplyVehicleConfig(VehicleConfigPtr);

    // Mark as spawned
    bIsSpawned = true;

    // Configure streaming source for vehicle
    if (StreamingSource)
    {
        StreamingSource->ConfigureForVehicle(VehicleConfigPtr->bIsFastVehicle);
    }
}

UGSDDataAsset* AGSDVehiclePawn::GetSpawnConfig()
{
    return VehicleConfig;
}

bool AGSDVehiclePawn::IsSpawned()
{
    return bIsSpawned;
}

void AGSDVehiclePawn::Despawn()
{
    bIsSpawned = false;
    VehicleConfig = nullptr;

    // Cancel any pending streaming hibernation
    if (StreamingSource)
    {
        StreamingSource->CancelHibernation();
    }
}

void AGSDVehiclePawn::ResetSpawnState()
{
    Despawn();
}

UChaosWheeledVehicleMovementComponent* AGSDVehiclePawn::GetVehicleMovement() const
{
    return Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());
}

void AGSDVehiclePawn::ApplyVehicleConfig(UGSDVehicleConfig* Config)
{
    if (!Config)
    {
        GSD_VEHICLE_ERROR(TEXT("ApplyVehicleConfig: Config is null"));
        return;
    }

    USkeletalMeshComponent* MeshComponent = GetMesh();
    if (!MeshComponent)
    {
        GSD_VEHICLE_ERROR(TEXT("ApplyVehicleConfig: GetMesh() returned null"));
        return;
    }

    // Load and apply skeletal mesh
    if (USkeletalMesh* Mesh = Config->VehicleMesh.LoadSynchronous())
    {
        MeshComponent->SetSkeletalMesh(Mesh);
    }
    else
    {
        GSD_VEHICLE_WARN(TEXT("ApplyVehicleConfig: Failed to load VehicleMesh for %s"), *Config->GetName());
    }

    // Load and apply physics asset
    if (UPhysicsAsset* PhysAsset = Config->PhysicsAsset.LoadSynchronous())
    {
        MeshComponent->SetPhysicsAsset(PhysAsset);
    }
    else
    {
        GSD_VEHICLE_WARN(TEXT("ApplyVehicleConfig: Failed to load PhysicsAsset for %s"), *Config->GetName());
    }

    // Set animation blueprint if specified
    if (Config->AnimBlueprintClass)
    {
        MeshComponent->SetAnimInstanceClass(Config->AnimBlueprintClass);
    }

    // Configure wheel setups
    UChaosWheeledVehicleMovementComponent* Movement = GetVehicleMovement();
    if (!Movement)
    {
        GSD_VEHICLE_ERROR(TEXT("ApplyVehicleConfig: GetVehicleMovement() returned null"));
        return;
    }

    const int32 NumWheels = Config->WheelConfigs.Num();
    if (NumWheels > 0)
    {
        Movement->WheelSetups.SetNum(NumWheels);

        for (int32 i = 0; i < NumWheels; ++i)
        {
            UGSDWheelConfig* WheelConfig = Config->WheelConfigs[i].LoadSynchronous();
            if (!WheelConfig)
            {
                GSD_VEHICLE_WARN(TEXT("ApplyVehicleConfig: Failed to load WheelConfig[%d] for %s"), i, *Config->GetName());
                continue;
            }

            // Set basic wheel setup
            Movement->WheelSetups[i].WheelClass = UChaosVehicleWheel::StaticClass();
            Movement->WheelSetups[i].BoneName = WheelConfig->WheelBoneName;
        }
    }

    // Set engine torque curve if specified
    if (Config->TorqueCurve && Movement)
    {
        Movement->EngineSetup.TorqueCurve.ExternalCurve = Config->TorqueCurve;
    }

    // Set mass on root component
    if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(GetRootComponent()))
    {
        RootPrim->SetMassOverrideInKg(NAME_None, Config->Mass);
    }

    // Note: Wheel physics properties (radius, suspension, friction, etc.) are applied
    // when the wheel blueprints are created from UChaosVehicleWheel. The WheelSetups
    // array configures bone names and wheel classes. Detailed physics properties
    // should be set on wheel blueprint subclasses or via the movement component's
    // wheel instances after initialization.

    GSD_VEHICLE_LOG(Log, TEXT("ApplyVehicleConfig: Applied config '%s' to vehicle"), *Config->GetName());
}
