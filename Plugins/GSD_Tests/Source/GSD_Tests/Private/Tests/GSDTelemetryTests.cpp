// Copyright Bret Bouchard. All Rights Reserved.

#include "GSD_Tests.h"
#include "Misc/AutomationTest.h"
#include "Types/GSDTelemetryTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// Test 1: Frame Time Tracking - Test circular buffer implementation
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDTelemetryFrameTimeTest,
    "GSD.Telemetry.FrameTime.Tracking",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDTelemetryFrameTimeTest::RunTest(const FString& Parameters)
{
    // Test FGSDFrameTimeHistory circular buffer
    FGSDFrameTimeHistory History;

    // Verify initial state
    TestEqual(TEXT("Initial sample count is 0"), History.GetSampleCount(), 0);
    TestEqual(TEXT("Initial average is 0.0"), History.GetAverageMs(), 0.0f);

    // Add frames to fill buffer (60 frames for 1-second average at 60fps)
    for (int32 i = 0; i < 60; i++)
    {
        History.AddFrameTime(16.67f); // 60fps
    }

    // Verify buffer filled correctly
    TestEqual(TEXT("Buffer has 60 samples"), History.GetSampleCount(), 60);

    // Verify average calculation (O(1) operation)
    float Average = History.GetAverageMs();
    TestEqual(TEXT("Average frame time is 16.67ms"), Average, 16.67f);

    // Test circular buffer wraparound
    History.AddFrameTime(33.33f); // This should replace the oldest sample
    TestEqual(TEXT("Buffer still has 60 samples after wrap"), History.GetSampleCount(), 60);

    // Average should change slightly (one 33.33ms sample replaced a 16.67ms sample)
    float NewAverage = History.GetAverageMs();
    float ExpectedNewAverage = (16.67f * 59 + 33.33f) / 60.0f;
    TestTrue(TEXT("Average changed after wraparound"),
        FMath::Abs(NewAverage - ExpectedNewAverage) < 0.01f);

    // Test Reset
    History.Reset();
    TestEqual(TEXT("Sample count is 0 after reset"), History.GetSampleCount(), 0);
    TestEqual(TEXT("Average is 0.0 after reset"), History.GetAverageMs(), 0.0f);

    return true;
}

// Test 2: Hitch Detection - Test hitch event structure
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDTelemetryHitchTest,
    "GSD.Telemetry.Hitch.Detection",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDTelemetryHitchTest::RunTest(const FString& Parameters)
{
    // Test FGSDHitchEvent structure
    FGSDHitchEvent HitchEvent;

    // Verify defaults
    TestEqual(TEXT("Default hitch time is 0.0"), HitchEvent.HitchTimeMs, 0.0f);
    TestEqual(TEXT("Default district name is empty"), HitchEvent.DistrictName, FName());
    TestEqual(TEXT("Default timestamp is 0.0"), HitchEvent.Timestamp, 0.0);

    // Test setting values
    HitchEvent.HitchTimeMs = 33.33f;
    HitchEvent.DistrictName = FName(TEXT("Downtown"));
    HitchEvent.Timestamp = 12345.6789;

    TestEqual(TEXT("Hitch time set correctly"), HitchEvent.HitchTimeMs, 33.33f);
    TestEqual(TEXT("District name set correctly"), HitchEvent.DistrictName, FName(TEXT("Downtown")));
    TestEqual(TEXT("Timestamp set correctly"), HitchEvent.Timestamp, 12345.6789);

    // Test threshold detection logic (hitch > 16.67ms threshold)
    const float HitchThreshold = 16.67f;
    TestTrue(TEXT("Hitch exceeds threshold"), HitchEvent.HitchTimeMs > HitchThreshold);

    // Test normal frame (no hitch)
    FGSDHitchEvent NormalFrame;
    NormalFrame.HitchTimeMs = 16.0f;
    TestFalse(TEXT("Normal frame does not exceed threshold"), NormalFrame.HitchTimeMs > HitchThreshold);

    return true;
}

// Test 3: Streaming Telemetry - Test cell load time record
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDTelemetryStreamingTest,
    "GSD.Telemetry.Streaming.CellLoad",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDTelemetryStreamingTest::RunTest(const FString& Parameters)
{
    // Test FGSDCellLoadTimeRecord structure
    FGSDCellLoadTimeRecord CellRecord;

    // Verify defaults
    TestEqual(TEXT("Default cell name is empty"), CellRecord.CellName, FName());
    TestEqual(TEXT("Default load time is 0.0"), CellRecord.LoadTimeMs, 0.0f);
    TestEqual(TEXT("Default district name is empty"), CellRecord.DistrictName, FName());
    TestEqual(TEXT("Default timestamp is 0.0"), CellRecord.Timestamp, 0.0);

    // Test setting values
    CellRecord.CellName = FName(TEXT("Cell_1_1"));
    CellRecord.LoadTimeMs = 45.5f;
    CellRecord.DistrictName = FName(TEXT("Industrial"));
    CellRecord.Timestamp = 100.0;

    TestEqual(TEXT("Cell name set correctly"), CellRecord.CellName, FName(TEXT("Cell_1_1")));
    TestEqual(TEXT("Load time set correctly"), CellRecord.LoadTimeMs, 45.5f);
    TestEqual(TEXT("District name set correctly"), CellRecord.DistrictName, FName(TEXT("Industrial")));
    TestEqual(TEXT("Timestamp set correctly"), CellRecord.Timestamp, 100.0);

    // Test slow load detection (threshold: 100ms)
    const float SlowLoadThreshold = 100.0f;
    TestFalse(TEXT("Normal load does not exceed slow threshold"), CellRecord.LoadTimeMs > SlowLoadThreshold);

    // Test slow load
    FGSDCellLoadTimeRecord SlowLoad;
    SlowLoad.LoadTimeMs = 150.0f;
    TestTrue(TEXT("Slow load exceeds threshold"), SlowLoad.LoadTimeMs > SlowLoadThreshold);

    // Test FGSDActorCountSnapshot structure
    FGSDActorCountSnapshot Snapshot;
    TestEqual(TEXT("Default vehicle count is 0"), Snapshot.VehicleCount, 0);
    TestEqual(TEXT("Default zombie count is 0"), Snapshot.ZombieCount, 0);
    TestEqual(TEXT("Default human count is 0"), Snapshot.HumanCount, 0);
    TestEqual(TEXT("Default timestamp is 0.0"), Snapshot.Timestamp, 0.0);

    // Set snapshot values
    Snapshot.VehicleCount = 50;
    Snapshot.ZombieCount = 200;
    Snapshot.HumanCount = 10;
    Snapshot.Timestamp = 60.0;

    TestEqual(TEXT("Vehicle count set correctly"), Snapshot.VehicleCount, 50);
    TestEqual(TEXT("Zombie count set correctly"), Snapshot.ZombieCount, 200);
    TestEqual(TEXT("Human count set correctly"), Snapshot.HumanCount, 10);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
