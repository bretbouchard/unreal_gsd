// Copyright Bret Bouchard. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Subsystems/GSDStreamingTelemetry.h"

#if WITH_DEV_AUTOMATION_TESTS

// Test 1: Subsystem can be created (SMOKE)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDStreamingTelemetryCreationTest,
    "GSD.Streaming.Telemetry.Creation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::SmokeFilter)

bool FGSDStreamingTelemetryCreationTest::RunTest(const FString& Parameters)
{
    UGSDStreamingTelemetry* Telemetry = NewObject<UGSDStreamingTelemetry>();
    TestNotNull(TEXT("Telemetry should be created"), Telemetry);
    return true;
}

// Test 2: LogStreamingEvent adds events correctly
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDStreamingTelemetryLogEventTest,
    "GSD.Streaming.Telemetry.LogEvent",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDStreamingTelemetryLogEventTest::RunTest(const FString& Parameters)
{
    UGSDStreamingTelemetry* Telemetry = NewObject<UGSDStreamingTelemetry>();
    TestNotNull(TEXT("Telemetry should be created"), Telemetry);

    if (Telemetry)
    {
        Telemetry->LogStreamingEvent(TEXT("Cell_01"), 15.5f, FVector::ZeroVector, 0.0f);
        Telemetry->LogStreamingEvent(TEXT("Cell_02"), 22.0f, FVector::ZeroVector, 0.0f);

        const TArray<FGSDStreamingEvent>& Events = Telemetry->GetRecentEvents();
        TestEqual(TEXT("Should have 2 events"), Events.Num(), 2);

        if (Events.Num() >= 2)
        {
            TestEqual(TEXT("First event CellName"), Events[0].CellName, FString(TEXT("Cell_01")));
            TestEqual(TEXT("First event LoadTimeMs"), Events[0].LoadTimeMs, 15.5f);
        }
    }
    return true;
}

// Test 3: GetAverageLoadTimeMs calculates correctly
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDStreamingTelemetryAverageTest,
    "GSD.Streaming.Telemetry.AverageLoadTime",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDStreamingTelemetryAverageTest::RunTest(const FString& Parameters)
{
    UGSDStreamingTelemetry* Telemetry = NewObject<UGSDStreamingTelemetry>();
    TestNotNull(TEXT("Telemetry should be created"), Telemetry);

    if (Telemetry)
    {
        // Empty telemetry returns 0 (edge case - no divide-by-zero)
        float AvgEmpty = Telemetry->GetAverageLoadTimeMs();
        TestEqual(TEXT("Empty average should be 0"), AvgEmpty, 0.0f);

        // Log events
        Telemetry->LogStreamingEvent(TEXT("Cell_01"), 10.0f, FVector::ZeroVector, 0.0f);
        Telemetry->LogStreamingEvent(TEXT("Cell_02"), 20.0f, FVector::ZeroVector, 0.0f);
        Telemetry->LogStreamingEvent(TEXT("Cell_03"), 30.0f, FVector::ZeroVector, 0.0f);

        float AvgLoaded = Telemetry->GetAverageLoadTimeMs();
        TestEqual(TEXT("Average should be 20.0"), AvgLoaded, 20.0f);
    }
    return true;
}

// Test 4: Event buffer respects max size
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDStreamingTelemetryBufferTest,
    "GSD.Streaming.Telemetry.BufferLimit",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDStreamingTelemetryBufferTest::RunTest(const FString& Parameters)
{
    UGSDStreamingTelemetry* Telemetry = NewObject<UGSDStreamingTelemetry>();
    TestNotNull(TEXT("Telemetry should be created"), Telemetry);

    if (Telemetry)
    {
        // Log more than MaxRecentEvents
        for (int32 i = 0; i < 150; ++i)
        {
            Telemetry->LogStreamingEvent(FString::Printf(TEXT("Cell_%d"), i), 10.0f, FVector::ZeroVector, 0.0f);
        }

        const TArray<FGSDStreamingEvent>& Events = Telemetry->GetRecentEvents();
        TestTrue(TEXT("Events should be capped at MaxRecentEvents"), Events.Num() <= 150);
    }
    return true;
}

// Test 5: Empty buffer edge case (CRITICAL from Council review)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDStreamingTelemetryEmptyBufferTest,
    "GSD.Streaming.Telemetry.EmptyBuffer",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDStreamingTelemetryEmptyBufferTest::RunTest(const FString& Parameters)
{
    UGSDStreamingTelemetry* Telemetry = NewObject<UGSDStreamingTelemetry>();
    TestNotNull(TEXT("Telemetry should be created"), Telemetry);

    if (Telemetry)
    {
        // Operations on empty telemetry should not crash
        float Avg = Telemetry->GetAverageLoadTimeMs();
        TestEqual(TEXT("Empty buffer average should be 0"), Avg, 0.0f);

        FGSDStreamingTelemetryData Data = Telemetry->GetAggregatedData();
        TestEqual(TEXT("Empty aggregated data should have 0 cells"), Data.LoadedCells, 0);

        const TArray<FGSDStreamingEvent>& Events = Telemetry->GetRecentEvents();
        TestEqual(TEXT("Empty buffer should have 0 events"), Events.Num(), 0);
    }
    return true;
}

// Test 6: Peak load time tracking
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDStreamingTelemetryPeakTest,
    "GSD.Streaming.Telemetry.PeakLoadTime",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDStreamingTelemetryPeakTest::RunTest(const FString& Parameters)
{
    UGSDStreamingTelemetry* Telemetry = NewObject<UGSDStreamingTelemetry>();
    TestNotNull(TEXT("Telemetry should be created"), Telemetry);

    if (Telemetry)
    {
        Telemetry->LogStreamingEvent(TEXT("Cell_01"), 10.0f, FVector::ZeroVector, 0.0f);
        Telemetry->LogStreamingEvent(TEXT("Cell_02"), 50.0f, FVector::ZeroVector, 0.0f); // Peak
        Telemetry->LogStreamingEvent(TEXT("Cell_03"), 20.0f, FVector::ZeroVector, 0.0f);

        TestEqual(TEXT("Peak should be 50.0"), Telemetry->GetPeakLoadTimeMs(), 50.0f);
        TestEqual(TEXT("Bottleneck cell should be Cell_02"), Telemetry->GetBottleneckCell(), FString(TEXT("Cell_02")));
    }
    return true;
}

// Test 7: Reset telemetry
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDStreamingTelemetryResetTest,
    "GSD.Streaming.Telemetry.Reset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDStreamingTelemetryResetTest::RunTest(const FString& Parameters)
{
    UGSDStreamingTelemetry* Telemetry = NewObject<UGSDStreamingTelemetry>();
    TestNotNull(TEXT("Telemetry should be created"), Telemetry);

    if (Telemetry)
    {
        Telemetry->LogStreamingEvent(TEXT("Cell_01"), 10.0f, FVector::ZeroVector, 0.0f);
        TestEqual(TEXT("Should have 1 event"), Telemetry->GetRecentEvents().Num(), 1);

        Telemetry->ResetTelemetry();
        TestEqual(TEXT("Should have 0 events after reset"), Telemetry->GetRecentEvents().Num(), 0);
        TestEqual(TEXT("Peak should be reset"), Telemetry->GetPeakLoadTimeMs(), 0.0f);
    }
    return true;
}

// Test 8: Aggregated data structure
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDStreamingTelemetryAggregatedTest,
    "GSD.Streaming.Telemetry.AggregatedData",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDStreamingTelemetryAggregatedTest::RunTest(const FString& Parameters)
{
    UGSDStreamingTelemetry* Telemetry = NewObject<UGSDStreamingTelemetry>();
    TestNotNull(TEXT("Telemetry should be created"), Telemetry);

    if (Telemetry)
    {
        Telemetry->LogStreamingEvent(TEXT("Cell_01"), 10.0f, FVector::ZeroVector, 0.0f);
        Telemetry->LogStreamingEvent(TEXT("Cell_02"), 30.0f, FVector::ZeroVector, 0.0f);

        FGSDStreamingTelemetryData Data = Telemetry->GetAggregatedData();
        TestEqual(TEXT("LoadedCells should be 2"), Data.LoadedCells, 2);
        TestEqual(TEXT("AverageLoadTimeMs should be 20.0"), Data.AverageLoadTimeMs, 20.0f);
        TestEqual(TEXT("PeakLoadTimeMs should be 30.0"), Data.PeakLoadTimeMs, 30.0f);
    }
    return true;
}

#endif
