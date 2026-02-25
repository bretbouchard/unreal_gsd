// Copyright Bret Bouchard. All Rights Reserved.

#include "DataAssets/GSDWheelConfig.h"
#include "GSDVehicleLog.h"

UGSDWheelConfig::UGSDWheelConfig()
    : WheelBoneName(NAME_None)
    , WheelRadius(35.0f)
    , WheelWidth(20.0f)
    , bAffectedBySteering(false)
    , bAffectedByHandbrake(false)
    , bAffectedByEngine(true)
    , MaxSteerAngle(70.0f)
    , SuspensionMaxRaise(10.0f)
    , SuspensionMaxDrop(10.0f)
    , SuspensionDampingRatio(0.5f)
    , FrictionForceMultiplier(3.0f)
{
}

bool UGSDWheelConfig::ValidateConfig(FString& OutError) const
{
    bool bIsValid = true;
    TArray<FString> Errors;

    // Check that the wheel bone name is set
    if (WheelBoneName == NAME_None || WheelBoneName.IsNone())
    {
        Errors.Add(TEXT("WheelBoneName is not set. Each wheel must reference a bone in the skeletal mesh."));
        bIsValid = false;
    }

    // Validate wheel dimensions
    if (WheelRadius <= 0.0f)
    {
        Errors.Add(FString::Printf(TEXT("WheelRadius (%.2f) must be greater than 0."), WheelRadius));
        bIsValid = false;
    }

    if (WheelWidth <= 0.0f)
    {
        Errors.Add(FString::Printf(TEXT("WheelWidth (%.2f) must be greater than 0."), WheelWidth));
        bIsValid = false;
    }

    // Validate suspension settings
    if (SuspensionMaxRaise < 0.0f)
    {
        Errors.Add(FString::Printf(TEXT("SuspensionMaxRaise (%.2f) cannot be negative."), SuspensionMaxRaise));
        bIsValid = false;
    }

    if (SuspensionMaxDrop < 0.0f)
    {
        Errors.Add(FString::Printf(TEXT("SuspensionMaxDrop (%.2f) cannot be negative."), SuspensionMaxDrop));
        bIsValid = false;
    }

    // Validate friction
    if (FrictionForceMultiplier <= 0.0f)
    {
        Errors.Add(FString::Printf(TEXT("FrictionForceMultiplier (%.2f) must be greater than 0."), FrictionForceMultiplier));
        bIsValid = false;
    }

    // Log all validation errors
    if (!bIsValid)
    {
        for (const FString& Error : Errors)
        {
            UE_LOG(LOG_GSDVEHICLES, Error, TEXT("WheelConfig '%s' validation failed: %s"), *GetName(), *Error);
        }
        OutError = FString::Join(Errors, TEXT(" "));
    }

    return bIsValid;
}
