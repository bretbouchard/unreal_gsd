// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/GSDDataAsset.h"
#include "GSDWheelConfig.generated.h"

/**
 * Wheel configuration data asset for individual wheel setup.
 *
 * Contains all properties needed to configure a single wheel on a vehicle,
 * including dimensions, steering, suspension, and tire friction settings.
 *
 * Designers create one GSDWheelConfig per wheel type (front-left, front-right, etc.)
 * and reference them from GSDVehicleConfig.
 */
UCLASS(BlueprintType)
class GSD_VEHICLES_API UGSDWheelConfig : public UGSDDataAsset
{
    GENERATED_BODY()

public:
    UGSDWheelConfig();

    // ~UGSDDataAsset interface
    virtual bool ValidateConfig(FString& OutError) const override;
    // ~End of UGSDDataAsset interface

    //-- Wheel Settings --

    /** Bone name in the vehicle's skeletal mesh for this wheel */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wheel")
    FName WheelBoneName;

    /** Wheel radius in centimeters */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wheel", meta = (ClampMin = "1.0", UIMin = "10.0", UIMax = "100.0"))
    float WheelRadius;

    /** Wheel width in centimeters */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wheel", meta = (ClampMin = "1.0", UIMin = "5.0", UIMax = "50.0"))
    float WheelWidth;

    //-- Steering/Brake/Drive --

    /** Whether this wheel is affected by steering input */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Steering")
    bool bAffectedBySteering;

    /** Whether this wheel is affected by the handbrake */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Braking")
    bool bAffectedByHandbrake;

    /** Whether this wheel receives power from the engine */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drive")
    bool bAffectedByEngine;

    /** Maximum steering angle in degrees (only used if bAffectedBySteering is true) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Steering", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "90.0", EditCondition = "bAffectedBySteering"))
    float MaxSteerAngle;

    //-- Suspension --

    /** Maximum distance the wheel can move up from its rest position (cm) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Suspension", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "50.0"))
    float SuspensionMaxRaise;

    /** Maximum distance the wheel can move down from its rest position (cm) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Suspension", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "50.0"))
    float SuspensionMaxDrop;

    /** Suspension damping ratio (0.0 = no damping, 1.0 = critical damping) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Suspension", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "1.0"))
    float SuspensionDampingRatio;

    //-- Tire --

    /** Friction force multiplier for tire grip */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tire", meta = (ClampMin = "0.1", UIMin = "0.5", UIMax = "10.0"))
    float FrictionForceMultiplier;
};
