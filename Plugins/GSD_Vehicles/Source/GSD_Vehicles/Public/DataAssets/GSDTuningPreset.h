// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/GSDDataAsset.h"
#include "GSDTuningPreset.generated.h"

/**
 * Tuning preset configuration data asset.
 *
 * Contains all properties needed to define a vehicle tuning preset,
 * including steering, suspension, tire, engine, and physics multipliers.
 *
 * Designers create GSDTuningPreset assets for different driving modes
 * (compliance mode for normal driving, chaos mode for aggressive driving).
 */
UCLASS(BlueprintType)
class GSD_VEHICLES_API UGSDTuningPreset : public UGSDDataAsset
{
    GENERATED_BODY()

public:
    UGSDTuningPreset();

    // ~UGSDDataAsset interface
    virtual bool ValidateConfig(FString& OutError) const override;
    virtual FString GetDisplayName() const override;
    // ~End of UGSDDataAsset interface

    //-- Steering --

    /** Steering sensitivity multiplier (1.0 = normal, <1 = less sensitive, >1 = more sensitive) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Steering", meta = (ClampMin = "0.1", UIMin = "0.1", UIMax = "5.0"))
    float SteeringRatio;

    /** Maximum steering angle in degrees */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Steering", meta = (ClampMin = "0", ClampMax = "90", UIMin = "0", UIMax = "90"))
    float MaxSteeringAngle;

    //-- Suspension --

    /** Suspension stiffness multiplier (1.0 = normal, >1 = stiffer) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Suspension", meta = (ClampMin = "0.1", UIMin = "0.1", UIMax = "10.0"))
    float SuspensionStiffness;

    /** Suspension damping multiplier (1.0 = normal, >1 = more damping) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Suspension", meta = (ClampMin = "0.1", UIMin = "0.1", UIMax = "10.0"))
    float SuspensionDamping;

    //-- Tire --

    /** Tire friction multiplier (1.0 = normal, >1 = more grip) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tire", meta = (ClampMin = "0.1", UIMin = "0.1", UIMax = "5.0"))
    float FrictionMultiplier;

    /** Lateral stiffness multiplier (1.0 = normal, >1 = stiffer cornering) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tire", meta = (ClampMin = "0.1", UIMin = "0.1", UIMax = "5.0"))
    float LateralStiffness;

    //-- Engine --

    /** Engine torque multiplier (1.0 = normal, >1 = more torque) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Engine", meta = (ClampMin = "0.1", UIMin = "0.1", UIMax = "5.0"))
    float TorqueMultiplier;

    /** Maximum engine RPM */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Engine", meta = (ClampMin = "1000.0", UIMin = "1000.0", UIMax = "15000.0"))
    float MaxRPM;

    //-- Physics --

    /** Mass multiplier (1.0 = normal, >1 = heavier, <1 = lighter) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Physics", meta = (ClampMin = "0.1", UIMin = "0.1", UIMax = "5.0"))
    float MassMultiplier;

    /** Aerodynamic drag coefficient (0 = no drag, typical car ~0.3) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Physics", meta = (ClampMin = "0", UIMin = "0", UIMax = "1.0"))
    float DragCoefficient;

    //-- Interpolation --

    /** Duration to interpolate to this preset (0 = instant, >0 = smooth transition in seconds) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interpolation", meta = (ClampMin = "0", UIMin = "0", UIMax = "5.0"))
    float InterpolationDuration;

    //-- Static Helpers --

    /**
     * Get the default compliance preset.
     * Returns nullptr until default presets are created in editor.
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Tuning")
    static UGSDTuningPreset* GetDefaultCompliancePreset();

    /**
     * Get the default chaos preset.
     * Returns nullptr until default presets are created in editor.
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Tuning")
    static UGSDTuningPreset* GetDefaultChaosPreset();
};
