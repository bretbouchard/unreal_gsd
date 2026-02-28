// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "Commandlets/Commandlet.h"
#include "GSDEventTestCommandlet.generated.h"

/**
 * Commandlet for automated event system validation.
 *
 * Tests event bus, scheduler determinism, and data asset validation
 * without requiring editor interaction.
 *
 * Usage:
 *   UE-Editor.exe UnrealGSD.exe -run=GSDEventTest -seed=12345
 *
 * Exit codes:
 *   0 = All tests passed
 *   1 = Test failed
 *
 * JSON Output:
 * {
 *   "success": true,
 *   "determinism_verified": true,
 *   "event_bus_working": true,
 *   "scheduling_working": true,
 *   "test_duration_seconds": 1.5,
 *   "seed": 12345
 * }
 */
UCLASS()
class GSD_DAILYEVENTS_API UGSDEventTestCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    UGSDEventTestCommandlet();

    // ~UCommandlet interface
    virtual int32 Main(const FString& Params) override;
    // ~End of UCommandlet interface

private:
    //-- Test Parameters --
    int32 TestSeed = 12345;
    bool bOutputJSON = true;

    /**
     * Parse command line parameters.
     */
    void ParseParameters(const FString& Params);

    /**
     * Test scheduler determinism.
     * Generate schedule twice with same seed, verify identical results.
     *
     * @return True if determinism verified
     */
    UFUNCTION()
    bool TestDeterminism();

    /**
     * Test event bus subscribe/broadcast.
     * Subscribe to tag, broadcast, verify receipt.
     *
     * @return True if event bus works
     */
    UFUNCTION()
    bool TestEventBus();

    /**
     * Test event scheduling.
     * Generate schedule, verify events created.
     *
     * @return True if scheduling works
     */
    UFUNCTION()
    bool TestScheduling();

    /**
     * Output test results as JSON to stdout.
     */
    void OutputJSON(bool bSuccess, bool bDeterminismVerified, bool bEventBusWorking, bool bSchedulingWorking, float Duration);

    /**
     * Output test results as human-readable text.
     */
    void OutputText(bool bSuccess, bool bDeterminismVerified, bool bEventBusWorking, bool bSchedulingWorking);
};
