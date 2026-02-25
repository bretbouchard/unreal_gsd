// Copyright Bret Bouchard. All Rights Reserved.

#include "Components/GSDAttachmentComponent.h"
#include "DataAssets/GSDAttachmentConfig.h"
#include "Actors/GSDVehiclePawn.h"
#include "GSDVehicleLog.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"

UGSDAttachmentComponent::UGSDAttachmentComponent()
{
}

AStaticMeshActor* UGSDAttachmentComponent::AttachAccessory(UGSDAttachmentConfig* Config)
{
    if (!Config)
    {
        GSD_VEHICLE_ERROR(TEXT("AttachAccessory: Config is null"));
        return nullptr;
    }

    // Get vehicle owner
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        GSD_VEHICLE_ERROR(TEXT("AttachAccessory: No owner"));
        return nullptr;
    }

    AGSDVehiclePawn* VehicleOwner = Cast<AGSDVehiclePawn>(Owner);
    if (!VehicleOwner)
    {
        GSD_VEHICLE_ERROR(TEXT("AttachAccessory: Owner is not a GSDVehiclePawn"));
        return nullptr;
    }

    // Get vehicle mesh
    USkeletalMeshComponent* VehicleMesh = VehicleOwner->GetMesh();
    if (!VehicleMesh)
    {
        GSD_VEHICLE_ERROR(TEXT("AttachAccessory: Vehicle has no mesh"));
        return nullptr;
    }

    // Check socket exists
    if (!VehicleMesh->DoesSocketExist(Config->SocketName))
    {
        GSD_VEHICLE_ERROR(TEXT("AttachAccessory: Socket '%s' does not exist on vehicle mesh"), *Config->SocketName.ToString());
        return nullptr;
    }

    // Load attachment mesh
    UStaticMesh* Mesh = Config->AttachmentMesh.LoadSynchronous();
    if (!Mesh)
    {
        GSD_VEHICLE_ERROR(TEXT("AttachAccessory: Failed to load mesh for attachment '%s'"), *Config->GetName());
        return nullptr;
    }

    // Spawn attachment actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = GetOwner();
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AStaticMeshActor* AttachmentActor = GetWorld()->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        SpawnParams);

    if (!AttachmentActor)
    {
        GSD_VEHICLE_ERROR(TEXT("AttachAccessory: Failed to spawn attachment actor"));
        return nullptr;
    }

    // Set mesh on spawned actor
    UStaticMeshComponent* MeshComponent = AttachmentActor->GetStaticMeshComponent();
    if (MeshComponent)
    {
        MeshComponent->SetStaticMesh(Mesh);
    }

    // Attach to socket
    AttachmentActor->AttachToComponent(
        VehicleMesh,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        Config->SocketName);

    // Set collision based on config
    if (MeshComponent)
    {
        if (Config->bHasCollision)
        {
            MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            // Ignore vehicle and pawn channels to prevent self-collision
            MeshComponent->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
            MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
        }
        else
        {
            MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }

    // Apply additional mass to vehicle
    if (Config->AdditionalMass > 0.0f)
    {
        if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(VehicleOwner->GetRootComponent()))
        {
            float CurrentMass = RootPrim->GetMass();
            RootPrim->SetMassOverrideInKg(NAME_None, CurrentMass + Config->AdditionalMass);
            TotalAddedMass += Config->AdditionalMass;

            GSD_VEHICLE_LOG(Verbose, TEXT("AttachAccessory: Added %.2f kg mass to vehicle (total added: %.2f kg)"),
                Config->AdditionalMass, TotalAddedMass);
        }
    }

    // Track attachment
    AttachedAccessories.Add(AttachmentActor);
    AttachmentConfigMap.Add(AttachmentActor, Config);

    GSD_VEHICLE_LOG(Log, TEXT("AttachAccessory: Attached '%s' to socket '%s'"),
        *Config->GetDisplayName(), *Config->SocketName.ToString());

    // Broadcast notification
    OnAttachmentAdded.Broadcast(AttachmentActor, Config);

    return AttachmentActor;
}

void UGSDAttachmentComponent::RemoveAccessory(UGSDAttachmentConfig* Config)
{
    if (!Config)
    {
        GSD_VEHICLE_WARN(TEXT("RemoveAccessory: Config is null"));
        return;
    }

    AStaticMeshActor* AttachmentActor = FindAccessoryByConfig(Config);
    if (!AttachmentActor)
    {
        GSD_VEHICLE_WARN(TEXT("RemoveAccessory: No attachment found for config '%s'"), *Config->GetName());
        return;
    }

    // Get vehicle owner for mass removal
    AActor* Owner = GetOwner();
    AGSDVehiclePawn* VehicleOwner = Cast<AGSDVehiclePawn>(Owner);

    // Remove mass from vehicle
    if (Config->AdditionalMass > 0.0f && TotalAddedMass >= Config->AdditionalMass && VehicleOwner)
    {
        if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(VehicleOwner->GetRootComponent()))
        {
            float CurrentMass = RootPrim->GetMass();
            RootPrim->SetMassOverrideInKg(NAME_None, CurrentMass - Config->AdditionalMass);
            TotalAddedMass -= Config->AdditionalMass;

            GSD_VEHICLE_LOG(Verbose, TEXT("RemoveAccessory: Removed %.2f kg mass from vehicle (total added: %.2f kg)"),
                Config->AdditionalMass, TotalAddedMass);
        }
    }

    // Remove from tracking
    AttachmentConfigMap.Remove(AttachmentActor);
    AttachedAccessories.Remove(AttachmentActor);

    // Broadcast before destruction
    OnAttachmentRemoved.Broadcast(AttachmentActor);

    // Destroy actor
    AttachmentActor->Destroy();

    GSD_VEHICLE_LOG(Log, TEXT("RemoveAccessory: Removed '%s' from vehicle"), *Config->GetDisplayName());
}

void UGSDAttachmentComponent::RemoveAllAttachments()
{
    // Get vehicle owner for mass removal
    AActor* Owner = GetOwner();
    AGSDVehiclePawn* VehicleOwner = Cast<AGSDVehiclePawn>(Owner);

    int32 RemovedCount = 0;

    // Remove each attachment
    for (AStaticMeshActor* AttachmentActor : AttachedAccessories)
    {
        if (!AttachmentActor)
        {
            continue;
        }

        // Get config for mass removal
        UGSDAttachmentConfig** ConfigPtr = AttachmentConfigMap.Find(AttachmentActor);
        if (ConfigPtr && *ConfigPtr)
        {
            UGSDAttachmentConfig* Config = *ConfigPtr;
            if (Config->AdditionalMass > 0.0f && TotalAddedMass >= Config->AdditionalMass && VehicleOwner)
            {
                if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(VehicleOwner->GetRootComponent()))
                {
                    float CurrentMass = RootPrim->GetMass();
                    RootPrim->SetMassOverrideInKg(NAME_None, CurrentMass - Config->AdditionalMass);
                    TotalAddedMass -= Config->AdditionalMass;
                }
            }
        }

        // Broadcast before destruction
        OnAttachmentRemoved.Broadcast(AttachmentActor);

        // Destroy actor
        AttachmentActor->Destroy();
        ++RemovedCount;
    }

    // Clear tracking
    AttachedAccessories.Empty();
    AttachmentConfigMap.Empty();
    TotalAddedMass = 0.0f;

    GSD_VEHICLE_LOG(Log, TEXT("RemoveAllAttachments: Removed %d attachments from vehicle"), RemovedCount);
}

AStaticMeshActor* UGSDAttachmentComponent::FindAccessoryByConfig(UGSDAttachmentConfig* Config)
{
    if (!Config)
    {
        return nullptr;
    }

    for (const auto& Pair : AttachmentConfigMap)
    {
        if (Pair.Value == Config)
        {
            return Pair.Key;
        }
    }

    return nullptr;
}
