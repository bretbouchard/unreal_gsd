// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GSDLaunchControlComponent.generated.h"

class UGSDLaunchControlConfig;
class UChaosWheeledVehicleMovementComponent;

/**
 * Delegate for launch control completion.
 * Broadcast when the launch control sequence has fully completed (throttle at 100%).
 */
DECLARE_DYNAMIC_DELEGATE(FOnLaunchControlComplete);

/**
 * Launch control component that provides throttle ramping and traction control for vehicle launches.
 *
 * Provides consistent, controlled vehicle launches for performance scenarios with:
 * - Throttle ramping from initial limit to full throttle over configured time
 * - Traction control that reduces torque when wheel slip exceeds threshold
 *
 * Usage:
 * 1. Add component to vehicle pawn
 * 2. Call Initialize() with config and movement component
 * 3. Call ActivateLaunchControl() to begin launch sequence
 * 4. Call DeactivateLaunchControl() to return to normal driving
 */
UCLASS(ClassGroup = (GSD), meta = (BlueprintSpawnableComponent))
class GSD_VEHICLES_API UGSDLaunchControlComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGSDLaunchControlComponent();

    /**
     * Initialize launch control with config and movement component.
     * Must be called before ActivateLaunchControl().
     *
     * @param InConfig Launch control configuration
     * @param InMovement Vehicle movement component for physics control
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Launch Control")
    void Initialize(UGSDLaunchControlConfig* InConfig, UChaosWheeledVehicleMovementComponent* InMovement);

    /**
     * Begin launch control sequence.
     * Starts throttle ramping and optional traction control.
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Launch Control")
    void ActivateLaunchControl();

    /**
     * End launch control, return to normal driving.
     * Stops throttle ramping and traction control.
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Launch Control")
    void DeactivateLaunchControl();

    /**
     * Check if launch control is currently active.
     *
     * @return True if launch control is active
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Launch Control")
    bool IsLaunchControlActive() const { return bLaunchControlActive; }

    /**
     * Get the delegate for launch control completion notification.
     * Bind to receive notification when throttle reaches 100%.
     *
     * @return Reference to the delegate
     */
    FOnLaunchControlComplete& GetOnLaunchControlComplete() { return OnLaunchControlComplete; }

    /** Delegate broadcast when launch control completes */
    UPROPERTY(BlueprintAssignable, Category = "GSD|Launch Control")
    FOnLaunchControlComplete OnLaunchControlComplete;

protected:
    // ~UActorComponent interface
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    // ~End of UActorComponent interface

    /**
     * Interpolate throttle from initial limit to full.
     *
     * @param DeltaTime Frame delta time
     */
    void ApplyThrottleRamp(float DeltaTime);

    /**
     * Check wheel slip and reduce torque if needed.
     */
    void ApplyTractionControl();

    /**
     * Calculate slip ratio for a wheel.
     *
     * @param WheelIndex Index of the wheel to check
     * @return Slip ratio (0 = no slip, 1+ = significant slip)
     */
    float CalculateWheelSlip(int32 WheelIndex);

private:
    /** Launch control configuration */
    UPROPERTY()
    TObjectPtr<UGSDLaunchControlConfig> LaunchConfig;

    /** Vehicle movement component for physics control */
    UPROPERTY()
    TObjectPtr<UChaosWheeledVehicleMovementComponent> VehicleMovement;

    /** Whether launch control is currently active */
    bool bLaunchControlActive = false;

    /** Current throttle target (interpolated) */
    float CurrentThrottleTarget = 0.0f;

    /** Progress through the throttle ramp (0-1) */
    float RampProgress = 0.0f;
};
