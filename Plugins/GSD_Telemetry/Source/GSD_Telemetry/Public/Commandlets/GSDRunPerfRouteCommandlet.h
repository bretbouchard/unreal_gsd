// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "Types/GSDValidationTypes.h"
#include "GSDRunPerfRouteCommandlet.generated.h"

/**
 * Performance route waypoint result
 */
USTRUCT(BlueprintType)
struct FGSDWaypointResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    FString WaypointName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float CapturedFrameTimeMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float ExpectedFrameTimeMs = 16.67f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bPassed = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float DeltaMs = 0.0f;  // Actual - Expected
};

/**
 * Performance route commandlet for CI pipelines
 * Implements TEL-06 (GSDRunPerfRoute commandlet)
 *
 * Usage:
 *   UnrealEditor-Cmd.exe MyProject -run=GSDRunPerfRoute
 *
 * Parameters:
 *   - targetfps=60      : Target FPS for validation (default: 60)
 *   - tolerance=0.1     : Tolerance for frame time (default: 0.1 = 10%)
 *   - duration=5.0      : Test duration in seconds (default: 5.0)
 *   - json=true         : Output JSON to stdout (default: true)
 *
 * Exit codes:
 *   0 = Performance within baseline
 *   1 = Performance regression detected
 */
UCLASS()
class GSD_TELEMETRY_API UGSDRunPerfRouteCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    UGSDRunPerfRouteCommandlet();

    virtual int32 Main(const FString& Params) override;

private:
    // Configuration
    UPROPERTY()
    float TargetFPS = 60.0f;

    UPROPERTY()
    float Tolerance = 0.1f;  // 10% tolerance

    UPROPERTY()
    float TestDuration = 5.0f;

    UPROPERTY()
    bool bOutputJSON = true;

    UPROPERTY()
    bool bVerbose = false;

    // Waypoints (simplified - actual implementation would load from config)
    UPROPERTY()
    TArray<FGSDPerfRouteWaypoint> Waypoints;

    // Results
    UPROPERTY()
    TArray<FGSDWaypointResult> WaypointResults;

    // Methods
    void ParseParameters(const FString& Params);
    void InitializeDefaultWaypoints();
    bool RunRoute(UWorld* World);
    FGSDWaypointResult CaptureMetricsAtWaypoint(const FGSDPerfRouteWaypoint& Waypoint, UWorld* World);
    void OutputJSON(const TArray<FGSDWaypointResult>& Results);
    void OutputText(const TArray<FGSDWaypointResult>& Results);
};
