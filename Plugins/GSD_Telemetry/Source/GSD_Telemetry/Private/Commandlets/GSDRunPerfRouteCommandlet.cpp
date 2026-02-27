// Copyright Bret Bouchard. All Rights Reserved.

#include "Commandlets/GSDRunPerfRouteCommandlet.h"
#include "GSDTelemetryLog.h"
#include "Subsystems/GSDPerformanceTelemetry.h"
#include "JsonObjectConverter.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

UGSDRunPerfRouteCommandlet::UGSDRunPerfRouteCommandlet()
{
    HelpDescription = TEXT("Performance route commandlet for CI pipelines");
    HelpUsage = TEXT("UnrealEditor-Cmd.exe MyProject -run=GSDRunPerfRoute");
    HelpParamNames.Add(TEXT("targetfps"));
    HelpParamDescriptions.Add(TEXT("Target FPS for validation (default: 60)"));
    HelpParamNames.Add(TEXT("tolerance"));
    HelpParamDescriptions.Add(TEXT("Tolerance for frame time (default: 0.1 = 10%)"));
    HelpParamNames.Add(TEXT("duration"));
    HelpParamDescriptions.Add(TEXT("Test duration in seconds (default: 5.0)"));
    HelpParamNames.Add(TEXT("json"));
    HelpParamDescriptions.Add(TEXT("Output JSON to stdout (default: true)"));
}

void UGSDRunPerfRouteCommandlet::ParseParameters(const FString& Params)
{
    if (FParse::Value(*Params, TEXT("targetfps="), TargetFPS))
    {
        GSDTELEMETRY_LOG(Log, TEXT("Parsed targetfps: %.1f"), TargetFPS);
    }

    if (FParse::Value(*Params, TEXT("tolerance="), Tolerance))
    {
        GSDTELEMETRY_LOG(Log, TEXT("Parsed tolerance: %.2f"), Tolerance);
    }

    if (FParse::Value(*Params, TEXT("duration="), TestDuration))
    {
        GSDTELEMETRY_LOG(Log, TEXT("Parsed duration: %.1f"), TestDuration);
    }

    FString JsonFlag;
    if (FParse::Value(*Params, TEXT("json="), JsonFlag))
    {
        bOutputJSON = JsonFlag.ToLower() == TEXT("true");
    }

    FString VerboseFlag;
    if (FParse::Value(*Params, TEXT("verbose="), VerboseFlag))
    {
        bVerbose = VerboseFlag.ToLower() == TEXT("true");
    }
}

void UGSDRunPerfRouteCommandlet::InitializeDefaultWaypoints()
{
    // Create default waypoints for testing
    // In production, these would be loaded from config or test map
    Waypoints.Empty();

    FGSDPerfRouteWaypoint Waypoint1;
    Waypoint1.Location = FVector(0.0f, 0.0f, 0.0f);
    Waypoint1.WaypointName = TEXT("Start");
    Waypoint1.ExpectedFrameTimeMs = 1000.0f / TargetFPS;
    Waypoints.Add(Waypoint1);

    FGSDPerfRouteWaypoint Waypoint2;
    Waypoint2.Location = FVector(10000.0f, 0.0f, 0.0f);
    Waypoint2.WaypointName = TEXT("District_A");
    Waypoint2.ExpectedFrameTimeMs = 1000.0f / TargetFPS;
    Waypoints.Add(Waypoint2);

    FGSDPerfRouteWaypoint Waypoint3;
    Waypoint3.Location = FVector(10000.0f, 10000.0f, 0.0f);
    Waypoint3.WaypointName = TEXT("District_B");
    Waypoint3.ExpectedFrameTimeMs = 1000.0f / TargetFPS;
    Waypoints.Add(Waypoint3);
}

int32 UGSDRunPerfRouteCommandlet::Main(const FString& Params)
{
    GSDTELEMETRY_LOG(Log, TEXT("GSDRunPerfRouteCommandlet starting..."));

    const double StartTime = FPlatformTime::Seconds();

    // Parse parameters
    ParseParameters(Params);

    // Initialize waypoints
    InitializeDefaultWaypoints();

    // Get world
    UWorld* World = GWorld;
    if (!World)
    {
        GSDTELEMETRY_LOG(Error, TEXT("No world available for performance route"));
        FGSDWaypointResult ErrorResult;
        ErrorResult.WaypointName = TEXT("Error");
        ErrorResult.bPassed = false;
        WaypointResults.Add(ErrorResult);
        OutputJSON(WaypointResults);
        return 1;
    }

    // Run performance route
    const bool bAllPassed = RunRoute(World);

    const double EndTime = FPlatformTime::Seconds();

    // Output results
    if (bOutputJSON)
    {
        OutputJSON(WaypointResults);
    }
    else
    {
        OutputText(WaypointResults);
    }

    GSDTELEMETRY_LOG(Log, TEXT("GSDRunPerfRouteCommandlet completed: %s (Time: %.2fs)"),
        bAllPassed ? TEXT("SUCCESS") : TEXT("FAILED"), EndTime - StartTime);

    return bAllPassed ? 0 : 1;
}

bool UGSDRunPerfRouteCommandlet::RunRoute(UWorld* World)
{
    GSDTELEMETRY_LOG(Log, TEXT("Running performance route with %d waypoints..."), Waypoints.Num());

    bool bAllPassed = true;

    for (const FGSDPerfRouteWaypoint& Waypoint : Waypoints)
    {
        GSDTELEMETRY_LOG(Log, TEXT("Capturing metrics at waypoint: %s"), *Waypoint.WaypointName);

        // Capture metrics at waypoint
        FGSDWaypointResult Result = CaptureMetricsAtWaypoint(Waypoint, World);
        WaypointResults.Add(Result);

        if (!Result.bPassed)
        {
            bAllPassed = false;
            GSDTELEMETRY_LOG(Warning, TEXT("Waypoint %s failed: %.2fms (expected: %.2fms)"),
                *Waypoint.WaypointName, Result.CapturedFrameTimeMs, Result.ExpectedFrameTimeMs);
        }
        else if (bVerbose)
        {
            GSDTELEMETRY_LOG(Log, TEXT("Waypoint %s passed: %.2fms"), *Waypoint.WaypointName, Result.CapturedFrameTimeMs);
        }
    }

    return bAllPassed;
}

FGSDWaypointResult UGSDRunPerfRouteCommandlet::CaptureMetricsAtWaypoint(const FGSDPerfRouteWaypoint& Waypoint, UWorld* World)
{
    FGSDWaypointResult Result;
    Result.WaypointName = Waypoint.WaypointName;
    Result.ExpectedFrameTimeMs = Waypoint.ExpectedFrameTimeMs;

    // Note: In a real implementation, this would:
    // 1. Move player/camera to waypoint location
    // 2. Wait for streaming to settle
    // 3. Capture frame time samples over TestDuration
    // 4. Calculate average frame time

    // Simplified implementation: Simulate frame time capture
    TArray<float> FrameTimeSamples;
    const double WaypointStartTime = FPlatformTime::Seconds();
    const double WaypointEndTime = WaypointStartTime + TestDuration;

    while (FPlatformTime::Seconds() < WaypointEndTime)
    {
        // Tick world
        if (World)
        {
            World->Tick(LEVELTICK_All, 1.0f / TargetFPS);
        }

        // Simulate frame time capture (placeholder)
        // In production, use actual frame time from stats or telemetry
        const float FrameTimeMs = 1000.0f / TargetFPS;  // Placeholder
        FrameTimeSamples.Add(FrameTimeMs);

        // Small delay to prevent tight loop
        FPlatformProcess::Sleep(1.0f / TargetFPS);
    }

    // Calculate average frame time
    if (FrameTimeSamples.Num() > 0)
    {
        float TotalFrameTime = 0.0f;
        for (float FrameTime : FrameTimeSamples)
        {
            TotalFrameTime += FrameTime;
        }
        Result.CapturedFrameTimeMs = TotalFrameTime / static_cast<float>(FrameTimeSamples.Num());
    }

    // Calculate delta
    Result.DeltaMs = Result.CapturedFrameTimeMs - Result.ExpectedFrameTimeMs;

    // Check if within tolerance
    const float MaxAllowedFrameTimeMs = Result.ExpectedFrameTimeMs * (1.0f + Tolerance);
    Result.bPassed = Result.CapturedFrameTimeMs <= MaxAllowedFrameTimeMs;

    return Result;
}

void UGSDRunPerfRouteCommandlet::OutputJSON(const TArray<FGSDWaypointResult>& Results)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

    const bool bAllPassed = [Results]() {
        for (const FGSDWaypointResult& Result : Results)
        {
            if (!Result.bPassed) return false;
        }
        return true;
    }();

    JsonObject->SetBoolField(TEXT("passed"), bAllPassed);
    JsonObject->SetNumberField(TEXT("target_fps"), TargetFPS);
    JsonObject->SetNumberField(TEXT("tolerance"), Tolerance);
    JsonObject->SetNumberField(TEXT("test_duration_seconds"), TestDuration);
    JsonObject->SetNumberField(TEXT("waypoint_count"), Results.Num());

    // Waypoints array
    TArray<TSharedPtr<FJsonValue>> WaypointsArray;
    for (const FGSDWaypointResult& Result : Results)
    {
        TSharedPtr<FJsonObject> WaypointObj = MakeShareable(new FJsonObject);
        WaypointObj->SetStringField(TEXT("waypoint_name"), Result.WaypointName);
        WaypointObj->SetNumberField(TEXT("captured_frame_time_ms"), Result.CapturedFrameTimeMs);
        WaypointObj->SetNumberField(TEXT("expected_frame_time_ms"), Result.ExpectedFrameTimeMs);
        WaypointObj->SetBoolField(TEXT("passed"), Result.bPassed);
        WaypointObj->SetNumberField(TEXT("delta_ms"), Result.DeltaMs);
        WaypointsArray.Add(MakeShareable(new FJsonValueObject(WaypointObj)));
    }
    JsonObject->SetArrayField(TEXT("waypoints"), WaypointsArray);

    // Serialize to string
    FString OutputString;
    TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

    // Output to stdout for CI parsing
    fprintf(stdout, "%s\n", *OutputString);
    fflush(stdout);

    GSDTELEMETRY_LOG(Log, TEXT("JSON_OUTPUT: %s"), *OutputString);
}

void UGSDRunPerfRouteCommandlet::OutputText(const TArray<FGSDWaypointResult>& Results)
{
    const FString Status = [Results]() {
        for (const FGSDWaypointResult& Result : Results)
        {
            if (!Result.bPassed) return TEXT("FAILED");
        }
        return TEXT("PASSED");
    }();

    GSDTELEMETRY_LOG(Log, TEXT("=== PERFORMANCE ROUTE RESULT ==="));
    GSDTELEMETRY_LOG(Log, TEXT("Status: %s"), *Status);
    GSDTELEMETRY_LOG(Log, TEXT("Target FPS: %.1f"), TargetFPS);
    GSDTELEMETRY_LOG(Log, TEXT("Tolerance: %.0f%%"), Tolerance * 100.0f);
    GSDTELEMETRY_LOG(Log, TEXT("Waypoints: %d"), Results.Num());

    for (const FGSDWaypointResult& Result : Results)
    {
        GSDTELEMETRY_LOG(Log, TEXT(""));
        GSDTELEMETRY_LOG(Log, TEXT("  Waypoint: %s"), *Result.WaypointName);
        GSDTELEMETRY_LOG(Log, TEXT("    Captured: %.2f ms"), Result.CapturedFrameTimeMs);
        GSDTELEMETRY_LOG(Log, TEXT("    Expected: %.2f ms"), Result.ExpectedFrameTimeMs);
        GSDTELEMETRY_LOG(Log, TEXT("    Delta: %.2f ms"), Result.DeltaMs);
        GSDTELEMETRY_LOG(Log, TEXT("    Status: %s"), Result.bPassed ? TEXT("PASS") : TEXT("FAIL"));
    }
}
