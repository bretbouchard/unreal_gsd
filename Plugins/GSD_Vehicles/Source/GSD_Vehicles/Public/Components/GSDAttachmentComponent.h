// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GSDAttachmentComponent.generated.h"

class UGSDAttachmentConfig;
class AStaticMeshActor;
class AGSDVehiclePawn;

/**
 * Delegate for attachment added notification.
 */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnAttachmentAdded, AStaticMeshActor*, Attachment, UGSDAttachmentConfig*, Config);

/**
 * Delegate for attachment removed notification.
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnAttachmentRemoved, AStaticMeshActor*, Attachment);

/**
 * Attachment component that manages vehicle accessories via socket-based attachment.
 *
 * Enables vehicles to have attachable accessories (bumpers, plows, racks) that:
 * - Attach to skeletal mesh sockets
 * - Apply collision settings from config
 * - Add mass to vehicle physics
 *
 * Usage:
 * 1. Add component to vehicle pawn
 * 2. Call AttachAccessory() with attachment config
 * 3. Call RemoveAccessory() to detach specific accessory
 * 4. Call RemoveAllAttachments() to clear all attachments
 */
UCLASS(ClassGroup = (GSD), meta = (BlueprintSpawnableComponent))
class GSD_VEHICLES_API UGSDAttachmentComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGSDAttachmentComponent();

    /**
     * Attach an accessory to the vehicle.
     * Spawns actor, attaches to socket, applies collision and mass.
     *
     * @param Config Attachment configuration
     * @return Spawned attachment actor, or nullptr on failure
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Attachments")
    AStaticMeshActor* AttachAccessory(UGSDAttachmentConfig* Config);

    /**
     * Remove a specific attachment by config.
     * Removes mass and destroys actor.
     *
     * @param Config Attachment configuration to remove
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Attachments")
    void RemoveAccessory(UGSDAttachmentConfig* Config);

    /**
     * Get all attached accessories.
     *
     * @return Const reference to attached accessories array
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Attachments")
    const TArray<TObjectPtr<AStaticMeshActor>>& GetAttachedAccessories() const { return AttachedAccessories; }

    /**
     * Remove all attachments from vehicle.
     * Removes all mass and destroys all actors.
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Attachments")
    void RemoveAllAttachments();

    /** Delegate broadcast when attachment is added */
    UPROPERTY(BlueprintAssignable, Category = "GSD|Attachments")
    FOnAttachmentAdded OnAttachmentAdded;

    /** Delegate broadcast when attachment is removed */
    UPROPERTY(BlueprintAssignable, Category = "GSD|Attachments")
    FOnAttachmentRemoved OnAttachmentRemoved;

private:
    /** Array of spawned attachment actors */
    UPROPERTY()
    TArray<TObjectPtr<AStaticMeshActor>> AttachedAccessories;

    /** Map from actor to config for removal lookup */
    UPROPERTY()
    TMap<TObjectPtr<AStaticMeshActor>, TObjectPtr<UGSDAttachmentConfig>> AttachmentConfigMap;

    /** Total mass added by attachments */
    float TotalAddedMass = 0.0f;

    /**
     * Find attachment actor by config.
     *
     * @param Config Attachment configuration to find
     * @return Actor if found, nullptr otherwise
     */
    AStaticMeshActor* FindAccessoryByConfig(UGSDAttachmentConfig* Config);
};
