// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/GSDDataAsset.h"
#include "GSDLaunchControlConfig.generated.h"

/**
 * Launch control configuration data asset.
 *
 * Contains all properties needed to configure vehicle launch control behavior,
 * including throttle ramp settings, traction control parameters, and RPM control.
 *
 * Designers create one GSDLaunchControlConfig per vehicle type to customize
 * launch behavior for different vehicle classes (sports cars, trucks, etc.).
 */
UCLASS(BlueprintType)
class GSD_VEHICLES_API UGSDLaunchControlConfig : public UGSDDataAsset
{
    GENERATED_BODY()

public:
    UGSDLaunchControlConfig();

    // ~UGSDDataAsset interface
    virtual bool ValidateConfig(FString& OutError) const override;
    virtual FString GetDisplayName() const override;
    // ~End of UGSDDataAsset interface

    //-- Throttle Ramp --

    /** Time to ramp throttle from 0 to 1 (seconds) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throttle Ramp", meta = (ClampMin = "0.1", UIMin = "0.1", UIMax = "10.0"))
    float ThrottleRampTime;

    /** Maximum initial throttle (0-1, limits how much throttle can be applied at launch) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throttle Ramp", meta = (ClampMin = "0", ClampMax = "1", UIMin = "0", UIMax = "1"))
    float InitialThrottleLimit;

    //-- Traction Control --

    /** Whether traction control is enabled during launch */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traction Control")
    bool bEnableTractionControl;

    /** Wheel slip threshold before reducing torque (0-1, 0 = no slip allowed, 1 = full slip) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traction Control", meta = (ClampMin = "0", ClampMax = "1", UIMin = "0", UIMax = "1"))
    float WheelSlipThreshold;

    /** Amount to reduce torque when slip detected (0-1, 0 = no reduction, 1 = full reduction) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traction Control", meta = (ClampMin = "0", ClampMax = "1", UIMin = "0", UIMax = "1"))
    float TorqueReductionFactor;

    //-- RPM Control --

    /** Target launch RPM for clutch-style launch */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RPM Control", meta = (ClampMin = "500.0", UIMin = "1000.0", UIMax = "15000.0"))
    float TargetLaunchRPM;

    /** Whether to hold RPM at target before launch (clutch-style launch) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RPM Control")
    bool bHoldRPMAtLaunch;
};
