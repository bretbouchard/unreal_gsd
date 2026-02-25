// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Commandlet/Commandlet.h"
#include "GSDVehicleTestCommandlet.generated.h"

/**
 * Commandlet for automated CI testing of vehicle systems.
 * Spawns vehicles with deterministic seed and measures performance.
 *
 * Usage: UnrealEditor-Cmd.exe MyProject -run=GSDVehicleTest
 *
 * Parameters:
 *   -numvehicles=N : Number of vehicles to spawn (default: 50)
 *   -seed=N        : Random seed for deterministic testing (default: 42)
 *   -targetfps=F   : Target FPS for validation (default: 60.0)
 */
UCLASS()
class GSD_VEHICLES_API UGSDVehicleTestCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    UGSDVehicleTestCommandlet();

    // ~UCommandlet interface
    virtual int32 Main(const FString& Params) override;
    // ~End of UCommandlet interface

private:
    /** Number of vehicles to spawn for testing */
    int32 NumVehiclesToTest = 50;

    /** Target FPS for validation */
    float TargetFPS = 60.0f;

    /** Random seed for deterministic testing */
    int32 TestSeed = 42;
};
