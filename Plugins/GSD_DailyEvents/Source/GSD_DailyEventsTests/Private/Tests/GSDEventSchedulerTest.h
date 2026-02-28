// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION
namespace GSD_DailyEventsTests
{

    // Forward declarations
    class UGSDEventSchedulerSubsystem;

    /**
     * Test suite for UGSDEventSchedulerSubsystem determinism
     */
    DECLARE_SPEC(FGSDEventSchedulerDeterminismTest, GSD_DailyEventsTests, EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter);

    bool RunTest(const FString& Parameters) override;

    /**
     * Test suite for UGSDEventSchedulerSubsystem scheduling
     */
    DECLARE_SPEC(FGSDEventSchedulerSchedulingTest, GSD_DailyEventsTests, EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter);

    bool RunTest(const FString& Parameters) override;

    /**
     * Test suite for UGSDEventSchedulerSubsystem event lifecycle
     */
    DECLARE_SPEC(FGSDEventSchedulerLifecycleTest, GSD_DailyEventsTests, EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter);

    bool RunTest(const FString& Parameters) override;

    /**
     * Test suite for UGSDEventSchedulerSubsystem event ordering
     */
    DECLARE_SPEC(FGSDEventSchedulerOrderingTest, GSD_DailyEventsTests, EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter);

    bool RunTest(const FString& Parameters) override;

#endif
