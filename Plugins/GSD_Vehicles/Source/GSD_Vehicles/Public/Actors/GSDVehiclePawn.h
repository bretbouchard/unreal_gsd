// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Interfaces/IGSDSpawnable.h"
#include "Components/GSDStreamingSourceComponent.h"
#include "GSDVehiclePawn.generated.h"

class UGSDVehicleConfig;
class UGSDWheelConfig;
class UGSDLaunchControlComponent;
class UGSDAttachmentComponent;
class UGSDTuningPreset;

/**
 * Base vehicle pawn for GSD vehicles.
 *
 * Extends AWheeledVehiclePawn (Chaos Vehicle system) and implements IGSDSpawnable
 * for integration with GSD spawning system. Supports configuration from GSDVehicleConfig
 * Data Assets and integrates with World Partition streaming.
 *
 * Usage:
 * 1. Create a GSDVehicleConfig Data Asset with mesh, wheels, engine settings
 * 2. Spawn vehicle via GSD spawning system or directly
 * 3. Call SpawnFromConfig() with the config to apply all settings
 */
UCLASS(BlueprintType, Blueprintable, Category = "GSD|Vehicles")
class GSD_VEHICLES_API AGSDVehiclePawn : public AWheeledVehiclePawn, public IGSDSpawnable
{
    GENERATED_BODY()

public:
    AGSDVehiclePawn();

    // ~IGSDSpawnable interface
    virtual void SpawnFromConfig(UGSDDataAsset* Config) override;
    virtual UGSDDataAsset* GetSpawnConfig() override;
    virtual bool IsSpawned() override;
    virtual void Despawn() override;
    virtual void ResetSpawnState() override;
    // ~End of IGSDSpawnable interface

    /**
     * Get the Chaos vehicle movement component.
     * Returns the movement component cast to UChaosWheeledVehicleMovementComponent.
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Vehicles")
    UChaosWheeledVehicleMovementComponent* GetVehicleMovement() const;

    //-- Tuning Presets --

    /**
     * Apply tuning preset at runtime.
     * Modifies steering ratio, drag, mass, max RPM on movement component.
     *
     * @param Preset Tuning preset to apply
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Vehicles")
    void ApplyTuningPreset(UGSDTuningPreset* Preset);

    /**
     * Get currently active tuning preset.
     *
     * @return Active tuning preset, or nullptr if none applied
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Vehicles")
    UGSDTuningPreset* GetActiveTuningPreset() const;

    //-- Launch Control --

    /**
     * Activate launch control with config from VehicleConfig.
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Vehicles")
    void ActivateLaunchControl();

    /**
     * Deactivate launch control.
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Vehicles")
    void DeactivateLaunchControl();

    //-- Components --

    /** Streaming source component for World Partition integration */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GSD|Vehicles")
    TObjectPtr<UGSDStreamingSourceComponent> StreamingSource;

    /** Launch control component for throttle ramping and traction control */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GSD|Vehicles")
    TObjectPtr<UGSDLaunchControlComponent> LaunchControlComponent;

    /** Attachment component for accessories */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GSD|Vehicles")
    TObjectPtr<UGSDAttachmentComponent> AttachmentComponent;

protected:
    /** Vehicle configuration applied during spawn */
    UPROPERTY(BlueprintReadOnly, Category = "GSD|Vehicles")
    TObjectPtr<UGSDVehicleConfig> VehicleConfig;

    /** Currently active tuning preset */
    UPROPERTY()
    TObjectPtr<UGSDTuningPreset> ActiveTuningPreset;

    /** Whether this vehicle has been spawned from a config */
    bool bIsSpawned = false;

    /**
     * Apply vehicle configuration settings.
     * Loads mesh, physics asset, wheel setups, and engine settings from config.
     */
    void ApplyVehicleConfig(UGSDVehicleConfig* Config);
};
