// Copyright Bret Bouchard. All Rights Reserved.

#include "DataAssets/GSDTuningPreset.h"
#include "GSDVehicleLog.h"

UGSDTuningPreset::UGSDTuningPreset()
    : SteeringRatio(1.0f)
    , MaxSteeringAngle(70.0f)
    , SuspensionStiffness(1.0f)
    , SuspensionDamping(1.0f)
    , FrictionMultiplier(1.0f)
    , LateralStiffness(1.0f)
    , TorqueMultiplier(1.0f)
    , MaxRPM(7000.0f)
    , MassMultiplier(1.0f)
    , DragCoefficient(0.3f)
    , InterpolationDuration(0.5f)
{
}

bool UGSDTuningPreset::ValidateConfig(FString& OutError) const
{
    bool bIsValid = true;
    TArray<FString> Errors;

    // Validate steering ratio (avoid division by zero)
    if (SteeringRatio <= 0.0f)
    {
        Errors.Add(FString::Printf(TEXT("SteeringRatio (%.2f) must be greater than 0."), SteeringRatio));
        bIsValid = false;
    }

    // Validate max steering angle
    if (MaxSteeringAngle < 0.0f || MaxSteeringAngle > 90.0f)
    {
        Errors.Add(FString::Printf(TEXT("MaxSteeringAngle (%.2f) must be between 0 and 90 degrees."), MaxSteeringAngle));
        bIsValid = false;
    }

    // Validate suspension stiffness
    if (SuspensionStiffness <= 0.0f)
    {
        Errors.Add(FString::Printf(TEXT("SuspensionStiffness (%.2f) must be greater than 0."), SuspensionStiffness));
        bIsValid = false;
    }

    // Validate suspension damping
    if (SuspensionDamping <= 0.0f)
    {
        Errors.Add(FString::Printf(TEXT("SuspensionDamping (%.2f) must be greater than 0."), SuspensionDamping));
        bIsValid = false;
    }

    // Validate friction multiplier
    if (FrictionMultiplier <= 0.0f)
    {
        Errors.Add(FString::Printf(TEXT("FrictionMultiplier (%.2f) must be greater than 0."), FrictionMultiplier));
        bIsValid = false;
    }

    // Validate lateral stiffness
    if (LateralStiffness <= 0.0f)
    {
        Errors.Add(FString::Printf(TEXT("LateralStiffness (%.2f) must be greater than 0."), LateralStiffness));
        bIsValid = false;
    }

    // Validate torque multiplier
    if (TorqueMultiplier <= 0.0f)
    {
        Errors.Add(FString::Printf(TEXT("TorqueMultiplier (%.2f) must be greater than 0."), TorqueMultiplier));
        bIsValid = false;
    }

    // Validate max RPM
    if (MaxRPM <= 0.0f)
    {
        Errors.Add(FString::Printf(TEXT("MaxRPM (%.2f) must be greater than 0."), MaxRPM));
        bIsValid = false;
    }

    // Validate mass multiplier
    if (MassMultiplier <= 0.0f)
    {
        Errors.Add(FString::Printf(TEXT("MassMultiplier (%.2f) must be greater than 0."), MassMultiplier));
        bIsValid = false;
    }

    // Validate drag coefficient
    if (DragCoefficient < 0.0f)
    {
        Errors.Add(FString::Printf(TEXT("DragCoefficient (%.2f) cannot be negative."), DragCoefficient));
        bIsValid = false;
    }

    // Log all validation errors
    if (!bIsValid)
    {
        for (const FString& Error : Errors)
        {
            UE_LOG(LOG_GSDVEHICLES, Error, TEXT("TuningPreset '%s' validation failed: %s"), *GetName(), *Error);
        }
        OutError = FString::Join(Errors, TEXT(" "));
    }

    return bIsValid;
}

FString UGSDTuningPreset::GetDisplayName() const
{
    return TEXT("Tuning Preset");
}

UGSDTuningPreset* UGSDTuningPreset::GetDefaultCompliancePreset()
{
    // TODO(GSDVEHICLES-106): Implement when default presets are created in editor
    // These presets will provide baseline tuning for compliance-style driving
    return nullptr;
}

UGSDTuningPreset* UGSDTuningPreset::GetDefaultChaosPreset()
{
    // TODO(GSDVEHICLES-106): Implement when default presets are created in editor
    // These presets will provide baseline tuning for chaos-style driving
    return nullptr;
}
