// Copyright Bret Bouchard. All Rights Reserved.

#include "Components/GSDLaunchControlComponent.h"
#include "DataAssets/GSDLaunchControlConfig.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "GSDVehicleLog.h"

UGSDLaunchControlComponent::UGSDLaunchControlComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false; // Only tick when active
}

void UGSDLaunchControlComponent::Initialize(UGSDLaunchControlConfig* InConfig, UChaosWheeledVehicleMovementComponent* InMovement)
{
    LaunchConfig = InConfig;
    VehicleMovement = InMovement;

    if (!VehicleMovement)
    {
        GSD_VEHICLE_ERROR(TEXT("LaunchControlComponent: Initialize called with null movement component"));
        return;
    }

    GSD_VEHICLE_LOG(Log, TEXT("LaunchControlComponent: Initialized with config '%s'"),
        LaunchConfig ? *LaunchConfig->GetName() : TEXT("None"));
}

void UGSDLaunchControlComponent::ActivateLaunchControl()
{
    if (!LaunchConfig)
    {
        GSD_VEHICLE_ERROR(TEXT("LaunchControlComponent: ActivateLaunchControl called with no config"));
        return;
    }

    if (!VehicleMovement)
    {
        GSD_VEHICLE_ERROR(TEXT("LaunchControlComponent: ActivateLaunchControl called with no movement component"));
        return;
    }

    bLaunchControlActive = true;
    RampProgress = 0.0f;
    CurrentThrottleTarget = LaunchConfig->InitialThrottleLimit;

    // Enable ticking
    SetComponentTickEnabled(true);

    GSD_VEHICLE_LOG(Log, TEXT("LaunchControlComponent: Activated with initial throttle %.2f, ramp time %.2fs"),
        CurrentThrottleTarget, LaunchConfig->ThrottleRampTime);
}

void UGSDLaunchControlComponent::DeactivateLaunchControl()
{
    bLaunchControlActive = false;
    CurrentThrottleTarget = 0.0f;

    // Disable ticking
    SetComponentTickEnabled(false);

    GSD_VEHICLE_LOG(Log, TEXT("LaunchControlComponent: Deactivated"));
}

void UGSDLaunchControlComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bLaunchControlActive || !LaunchConfig || !VehicleMovement)
    {
        return;
    }

    ApplyThrottleRamp(DeltaTime);

    if (LaunchConfig->bEnableTractionControl)
    {
        ApplyTractionControl();
    }
}

void UGSDLaunchControlComponent::ApplyThrottleRamp(float DeltaTime)
{
    if (!LaunchConfig || !VehicleMovement)
    {
        return;
    }

    // Increment progress
    RampProgress += DeltaTime / LaunchConfig->ThrottleRampTime;
    RampProgress = FMath::Clamp(RampProgress, 0.0f, 1.0f);

    // Lerp from initial limit to 1.0
    CurrentThrottleTarget = FMath::Lerp(LaunchConfig->InitialThrottleLimit, 1.0f, RampProgress);

    // Apply throttle
    VehicleMovement->SetThrottleInput(CurrentThrottleTarget);

    // Check if complete
    if (RampProgress >= 1.0f)
    {
        GSD_VEHICLE_LOG(Log, TEXT("LaunchControlComponent: Launch sequence complete"));

        // Broadcast completion
        OnLaunchControlComplete.Broadcast();

        // Optionally deactivate after completion
        // DeactivateLaunchControl();
    }
}

void UGSDLaunchControlComponent::ApplyTractionControl()
{
    if (!LaunchConfig || !VehicleMovement)
    {
        return;
    }

    // Get wheel states
    const TArray<FChaosWheelState>& WheelStates = VehicleMovement->GetWheelStates();

    for (int32 i = 0; i < WheelStates.Num(); ++i)
    {
        float Slip = CalculateWheelSlip(i);

        if (Slip > LaunchConfig->WheelSlipThreshold)
        {
            // Calculate reduced torque
            float CurrentTorque = VehicleMovement->GetEngineTorque();
            float ReducedTorque = CurrentTorque * (1.0f - LaunchConfig->TorqueReductionFactor);

            // Apply reduced torque to this wheel
            VehicleMovement->SetDriveTorque(i, ReducedTorque);

            GSD_VEHICLE_LOG(Verbose, TEXT("LaunchControlComponent: Wheel %d slip %.2f exceeds threshold %.2f, reducing torque to %.2f"),
                i, Slip, LaunchConfig->WheelSlipThreshold, ReducedTorque);
        }
    }
}

float UGSDLaunchControlComponent::CalculateWheelSlip(int32 WheelIndex)
{
    if (!VehicleMovement)
    {
        return 0.0f;
    }

    const TArray<FChaosWheelState>& WheelStates = VehicleMovement->GetWheelStates();

    if (WheelIndex < 0 || WheelIndex >= WheelStates.Num())
    {
        return 0.0f;
    }

    const FChaosWheelState& WheelState = WheelStates[WheelIndex];

    // Get vehicle speed in cm/s
    float VehicleSpeed = VehicleMovement->GetForwardSpeed();

    // Get wheel speed (angular velocity * radius)
    float WheelSpeed = WheelState.AngularVelocity * WheelState.Radius;

    // Threshold for meaningful slip calculation (100 cm/s = 1 m/s)
    const float MinSpeedThreshold = 100.0f;

    if (FMath::Abs(VehicleSpeed) < MinSpeedThreshold)
    {
        // Vehicle is stationary or very slow
        // If wheel is spinning, it's full slip
        return FMath::Abs(WheelSpeed) > MinSpeedThreshold ? 1.0f : 0.0f;
    }

    // Calculate slip ratio: |wheel_speed - vehicle_speed| / |vehicle_speed|
    return FMath::Abs((WheelSpeed - VehicleSpeed) / VehicleSpeed);
}
