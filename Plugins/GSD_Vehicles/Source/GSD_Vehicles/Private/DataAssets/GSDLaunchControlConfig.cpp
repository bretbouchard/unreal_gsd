// Copyright Bret Bouchard. All Rights Reserved.

#include "DataAssets/GSDLaunchControlConfig.h"
#include "GSDVehicleLog.h"

UGSDLaunchControlConfig::UGSDLaunchControlConfig()
    : ThrottleRampTime(1.5f)
    , InitialThrottleLimit(0.7f)
    , bEnableTractionControl(true)
    , WheelSlipThreshold(0.2f)
    , TorqueReductionFactor(0.3f)
    , TargetLaunchRPM(4000.0f)
    , bHoldRPMAtLaunch(false)
{
}

bool UGSDLaunchControlConfig::ValidateConfig(FString& OutError) const
{
    bool bIsValid = true;
    TArray<FString> Errors;

    // Validate throttle ramp time (avoid division by zero)
    if (ThrottleRampTime <= 0.0f)
    {
        Errors.Add(FString::Printf(TEXT("ThrottleRampTime (%.2f) must be greater than 0."), ThrottleRampTime));
        bIsValid = false;
    }

    // Validate initial throttle limit
    if (InitialThrottleLimit < 0.0f || InitialThrottleLimit > 1.0f)
    {
        Errors.Add(FString::Printf(TEXT("InitialThrottleLimit (%.2f) must be between 0 and 1."), InitialThrottleLimit));
        bIsValid = false;
    }

    // Validate wheel slip threshold
    if (WheelSlipThreshold < 0.0f || WheelSlipThreshold > 1.0f)
    {
        Errors.Add(FString::Printf(TEXT("WheelSlipThreshold (%.2f) must be between 0 and 1."), WheelSlipThreshold));
        bIsValid = false;
    }

    // Validate torque reduction factor
    if (TorqueReductionFactor < 0.0f || TorqueReductionFactor > 1.0f)
    {
        Errors.Add(FString::Printf(TEXT("TorqueReductionFactor (%.2f) must be between 0 and 1."), TorqueReductionFactor));
        bIsValid = false;
    }

    // Validate target launch RPM
    if (TargetLaunchRPM <= 0.0f)
    {
        Errors.Add(FString::Printf(TEXT("TargetLaunchRPM (%.2f) must be greater than 0."), TargetLaunchRPM));
        bIsValid = false;
    }

    // Log all validation errors
    if (!bIsValid)
    {
        for (const FString& Error : Errors)
        {
            UE_LOG(LOG_GSDVEHICLES, Error, TEXT("LaunchControlConfig '%s' validation failed: %s"), *GetName(), *Error);
        }
        OutError = FString::Join(Errors, TEXT(" "));
    }

    return bIsValid;
}

FString UGSDLaunchControlConfig::GetDisplayName() const
{
    return TEXT("Launch Control Config");
}
