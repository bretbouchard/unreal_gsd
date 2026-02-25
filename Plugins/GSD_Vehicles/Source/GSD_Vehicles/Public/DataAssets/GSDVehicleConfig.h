// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/GSDDataAsset.h"
#include "GSDVehicleConfig.generated.h"

class USkeletalMesh;
class UPhysicsAsset;
class UAnimInstance;
class UCurveFloat;
class UGSDWheelConfig;
class UGSDLaunchControlConfig;
class UGSDTuningPreset;
class UGSDAttachmentConfig;

/**
 * Main vehicle configuration data asset.
 *
 * Contains all properties needed to configure a complete vehicle,
 * including mesh references, wheel configurations, engine settings,
 * and physics parameters.
 *
 * Designers create one GSDVehicleConfig per vehicle type and reference
 * it from the vehicle spawning system.
 */
UCLASS(BlueprintType)
class GSD_VEHICLES_API UGSDVehicleConfig : public UGSDDataAsset
{
    GENERATED_BODY()

public:
    UGSDVehicleConfig();

    // ~UGSDDataAsset interface
    virtual bool ValidateConfig(FString& OutError) const override;
    virtual FString GetDisplayName() const override;
    // ~End of UGSDDataAsset interface

    //-- Mesh and Animation --

    /** Vehicle skeletal mesh (soft reference for async loading) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
    TSoftObjectPtr<USkeletalMesh> VehicleMesh;

    /** Physics asset for the vehicle (soft reference for async loading) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
    TSoftObjectPtr<UPhysicsAsset> PhysicsAsset;

    /** Animation blueprint class for vehicle animations */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
    TSubclassOf<UAnimInstance> AnimBlueprintClass;

    //-- Wheel Configuration --

    /**
     * Wheel configurations for this vehicle.
     * Order typically: Front-Left, Front-Right, Rear-Left, Rear-Right, etc.
     * Soft references for async loading support.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wheels")
    TArray<TSoftObjectPtr<UGSDWheelConfig>> WheelConfigs;

    //-- Engine --

    /**
     * Engine torque curve (RPM vs Torque).
     * X-axis: Engine RPM (typically 0-8000)
     * Y-axis: Torque multiplier (0-1)
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Engine")
    TObjectPtr<UCurveFloat> TorqueCurve;

    //-- Physics --

    /** Vehicle mass in kilograms */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Physics", meta = (ClampMin = "100.0", UIMin = "500.0", UIMax = "10000.0"))
    float Mass;

    /** Offset from mesh origin to center of mass */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Physics")
    FVector CenterOfMassOffset;

    /** Aerodynamic drag coefficient (0.0 = no drag, typical car ~0.3) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Physics", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "0.5"))
    float DragCoefficient;

    //-- Streaming --

    /**
     * Whether this is a "fast" vehicle for streaming priority.
     * Fast vehicles (player-driven) get higher streaming priority
     * than ambient traffic vehicles.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Streaming")
    bool bIsFastVehicle;

    //-- Advanced Features (Phase 5) --

    /** Launch control configuration (optional) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Advanced Features")
    TObjectPtr<UGSDLaunchControlConfig> LaunchControlConfig;

    /** Available tuning presets for this vehicle */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Advanced Features")
    TArray<TObjectPtr<UGSDTuningPreset>> TuningPresets;

    /** Available attachment points for this vehicle */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Advanced Features")
    TArray<TSoftObjectPtr<UGSDAttachmentConfig>> AvailableAttachments;

    /** Default attachments to spawn with vehicle */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Advanced Features")
    TArray<TSoftObjectPtr<UGSDAttachmentConfig>> DefaultAttachments;
};
