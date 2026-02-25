// Copyright Bret Bouchard. All Rights Reserved.

#include "DataAssets/GSDVehicleConfig.h"
#include "GSDVehicleLog.h"

UGSDVehicleConfig::UGSDVehicleConfig()
    : Mass(1500.0f)
    , CenterOfMassOffset(FVector::ZeroVector)
    , DragCoefficient(0.3f)
    , bIsFastVehicle(false)
{
}

bool UGSDVehicleConfig::ValidateConfig(FString& OutError) const
{
    bool bIsValid = true;
    TArray<FString> Errors;

    // Check VehicleMesh is set
    if (!VehicleMesh.IsValid() && VehicleMesh.IsNull())
    {
        Errors.Add(TEXT("VehicleMesh is not set. A skeletal mesh is required for the vehicle."));
        bIsValid = false;
    }

    // Check WheelConfigs has at least 2 entries
    if (WheelConfigs.Num() < 2)
    {
        Errors.Add(FString::Printf(TEXT("WheelConfigs must have at least 2 entries (found %d)."), WheelConfigs.Num()));
        bIsValid = false;
    }
    else
    {
        // Check each wheel config is valid
        int32 InvalidWheelCount = 0;
        for (int32 i = 0; i < WheelConfigs.Num(); ++i)
        {
            if (!WheelConfigs[i].IsValid() && WheelConfigs[i].IsNull())
            {
                InvalidWheelCount++;
            }
        }
        if (InvalidWheelCount > 0)
        {
            Errors.Add(FString::Printf(TEXT("%d wheel config(s) are not set."), InvalidWheelCount));
            bIsValid = false;
        }
    }

    // Check TorqueCurve is set
    if (!TorqueCurve)
    {
        Errors.Add(TEXT("TorqueCurve is not set. An engine torque curve is required."));
        bIsValid = false;
    }

    // Validate physics settings
    if (Mass <= 0.0f)
    {
        Errors.Add(FString::Printf(TEXT("Mass (%.2f) must be greater than 0."), Mass));
        bIsValid = false;
    }

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
            UE_LOG(LOG_GSDVEHICLES, Error, TEXT("VehicleConfig '%s' validation failed: %s"), *GetName(), *Error);
        }
        OutError = FString::Join(Errors, TEXT(" "));
    }

    return bIsValid;
}

FString UGSDVehicleConfig::GetDisplayName() const
{
    // Return DisplayName from base class if set, otherwise return a default
    if (!DisplayName.IsEmpty())
    {
        return DisplayName;
    }
    return TEXT("Unnamed Vehicle");
}
