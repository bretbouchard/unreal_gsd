// Copyright Bret Bouchard. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Subsystems/GSDPerformanceTelemetry.h"
#include "Subsystems/GSDStreamingTelemetry.h"
#include "Types/GSDTelemetryTypes.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#if WITH_DEV_AUTOMATION_TESTS

// Test: Performance telemetry subsystem initialization
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGSDPerformanceTelemetryInitTest,
    "GSD.Telemetry.PerformanceTelemetry.Initialization",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDPerformanceTelemetryInitTest::RunTest(const FString& Parameters)
{
    // Create test world
    UWorld* TestWorld = NewObject<UWorld>();
    TestNotNull(TEXT("Test world created"), TestWorld);

    // Create game instance
    UGameInstance* GameInstance = NewObject<UGameInstance>();
    TestNotNull(TEXT("Game instance created"), GameInstance);

    // Get performance telemetry subsystem
    UGSDPerformanceTelemetry* PerfTelemetry = GameInstance->GetSubsystem<UGSDPerformanceTelemetry>();
    TestNotNull(TEXT("Performance telemetry subsystem exists"), PerfTelemetry);

    // Verify default configuration
    TestEqual(TEXT("Default hitch threshold is 16.67ms"),
        PerfTelemetry->HitchThresholdMs, 16.67f);

    TestEqual(TEXT("Default actor count interval is 5.0s"),
        PerfTelemetry->ActorCountInterval, 5.0f);

    return true;
}

// Test: Frame time recording and averaging
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGSDFrameTimeTest,
    "GSD.Telemetry.PerformanceTelemetry.FrameTime",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDFrameTimeTest::RunTest(const FString& Parameters)
{
    UGameInstance* GameInstance = NewObject<UGameInstance>();
    UGSDPerformanceTelemetry* PerfTelemetry = GameInstance->GetSubsystem<UGSDPerformanceTelemetry>();

    const FName DistrictName = TEXT("TestDistrict");

    // Record frame times
    PerfTelemetry->RecordFrameTime(16.0f, DistrictName);
    PerfTelemetry->RecordFrameTime(17.0f, DistrictName);
    PerfTelemetry->RecordFrameTime(16.5f, DistrictName);

    // Get average
    const float AverageFrameTime = PerfTelemetry->GetAverageFrameTimeMs(DistrictName);

    // Verify average (should be approximately 16.5ms)
    TestEqual(TEXT("Average frame time is correct"),
        AverageFrameTime, 16.5f);

    return true;
}

// Test: Hitch detection
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGSDHitchDetectionTest,
    "GSD.Telemetry.PerformanceTelemetry.HitchDetection",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDHitchDetectionTest::RunTest(const FString& Parameters)
{
    UGameInstance* GameInstance = NewObject<UGameInstance>();
    UGSDPerformanceTelemetry* PerfTelemetry = GameInstance->GetSubsystem<UGSDPerformanceTelemetry>();

    const FName DistrictName = TEXT("TestDistrict");

    // Record normal frame (no hitch)
    PerfTelemetry->RecordFrameTime(16.0f, DistrictName);
    TestEqual(TEXT("No hitches detected yet"),
        PerfTelemetry->GetHitchCount(DistrictName), 0);

    // Record hitch frame (exceeds 16.67ms threshold)
    PerfTelemetry->RecordFrameTime(33.0f, DistrictName);
    TestEqual(TEXT("Hitch detected"),
        PerfTelemetry->GetHitchCount(DistrictName), 1);

    // Record another hitch
    PerfTelemetry->RecordFrameTime(25.0f, DistrictName);
    TestEqual(TEXT("Second hitch detected"),
        PerfTelemetry->GetHitchCount(DistrictName), 2);

    return true;
}

// Test: Circular buffer frame time history
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGSDFrameTimeHistoryTest,
    "GSD.Telemetry.Types.FrameTimeHistory",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDFrameTimeHistoryTest::RunTest(const FString& Parameters)
{
    FGSDFrameTimeHistory History;

    // Add samples up to max
    for (int32 i = 0; i < 60; i++)
    {
        History.AddFrameTime(16.0f);
    }

    TestEqual(TEXT("Buffer has 60 samples"),
        History.GetSampleCount(), 60);

    TestEqual(TEXT("Average is 16.0ms"),
        History.GetAverageMs(), 16.0f);

    // Add one more sample (should wrap around)
    History.AddFrameTime(32.0f);

    TestEqual(TEXT("Buffer still has 60 samples after wrap"),
        History.GetSampleCount(), 60);

    // Average should change slightly (one 32.0ms sample replaced a 16.0ms sample)
    const float NewAverage = History.GetAverageMs();
    TestTrue(TEXT("Average changed after wrap"),
        NewAverage > 16.0f && NewAverage < 17.0f);

    return true;
}

// Test: Streaming telemetry cell load tracking
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGSDStreamingTelemetryTest,
    "GSD.Telemetry.StreamingTelemetry.CellLoadTimes",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDStreamingTelemetryTest::RunTest(const FString& Parameters)
{
    UGameInstance* GameInstance = NewObject<UGameInstance>();
    UGSDStreamingTelemetry* StreamingTelemetry = GameInstance->GetSubsystem<UGSDStreamingTelemetry>();

    const FName DistrictName = TEXT("District_A");
    const FName CellName1 = TEXT("Cell_1_1");
    const FName CellName2 = TEXT("Cell_1_2");

    // Record cell load times
    StreamingTelemetry->RecordCellLoadTime(CellName1, 50.0f, DistrictName);
    StreamingTelemetry->RecordCellLoadTime(CellName2, 75.0f, DistrictName);

    // Verify average
    const float AverageLoadTime = StreamingTelemetry->GetAverageCellLoadTimeMs(DistrictName);
    TestEqual(TEXT("Average cell load time is correct"),
        AverageLoadTime, 62.5f);

    // Verify total cells loaded
    TestEqual(TEXT("Total cells loaded is 2"),
        StreamingTelemetry->GetTotalCellsLoaded(), 2);

    // Verify max cell load time
    TestEqual(TEXT("Max cell load time is 75.0ms"),
        StreamingTelemetry->GetMaxCellLoadTimeMs(), 75.0f);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
