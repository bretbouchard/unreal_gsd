// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/GSDDataAsset.h"
#include "GSDAttachmentConfig.generated.h"

class UStaticMesh;

/**
 * Types of vehicle attachments for categorization and gameplay logic.
 */
UENUM(BlueprintType)
enum class EGSDAttachmentType : uint8
{
    FrontBumper    UMETA(DisplayName = "Front Bumper"),
    RearBumper     UMETA(DisplayName = "Rear Bumper"),
    Plow           UMETA(DisplayName = "Plow"),
    RoofRack       UMETA(DisplayName = "Roof Rack"),
    SideStep       UMETA(DisplayName = "Side Step"),
    Custom         UMETA(DisplayName = "Custom")
};

/**
 * Attachment configuration data asset.
 *
 * Contains all properties needed to configure a vehicle attachment,
 * including socket information, mesh reference, and physics settings.
 *
 * Designers create GSDAttachmentConfig assets for different vehicle parts
 * (bumpers, plows, roof racks, etc.) that can be attached at runtime.
 */
UCLASS(BlueprintType)
class GSD_VEHICLES_API UGSDAttachmentConfig : public UGSDDataAsset
{
    GENERATED_BODY()

public:
    UGSDAttachmentConfig();

    // ~UGSDDataAsset interface
    virtual bool ValidateConfig(FString& OutError) const override;
    virtual FString GetDisplayName() const override;
    // ~End of UGSDDataAsset interface

    //-- Socket Information --

    /** Socket name on skeletal mesh for attachment point */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Socket")
    FName SocketName;

    /** Type of attachment for categorization */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Socket")
    EGSDAttachmentType AttachmentType;

    /** Display name for UI */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Socket")
    FString DisplayName;

    //-- Mesh --

    /** Static mesh to attach (soft reference for async loading) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
    TSoftObjectPtr<UStaticMesh> AttachmentMesh;

    //-- Physics --

    /** Whether attachment has collision */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Physics")
    bool bHasCollision;

    /** Additional mass in kg */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Physics", meta = (ClampMin = "0.0"))
    float AdditionalMass;

    //-- Gameplay --

    /** Whether attachment affects vehicle physics behavior */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
    bool bAffectsPhysics;

    /** Drag coefficient modifier (-1 to 1, positive = more drag, negative = less drag) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay", meta = (ClampMin = "-1.0", UIMin = "-1.0", UIMax = "1.0"))
    float DragModifier;
};
