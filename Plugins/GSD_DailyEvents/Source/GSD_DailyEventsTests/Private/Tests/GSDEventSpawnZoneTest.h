// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTGORITH_ONLY
namespace GSD_DailyEventsTests
{

    // Forward declarations
    class UGSDEventSpawnZone;

    /**
     * Test suite for UGSDEventSpawnZone validation
     */
    DECLARE_SPEC(FGSDEventSpawnZoneValidationTest, GSD_DailyEventsTests, EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter);

    bool RunTest(const FString& Parameters) override;

    /**
     * Test suite for UGSDEventSpawnZone determinism
     */
    DECLARE_SPEC(FGSDEventSpawnZoneDeterminismTest, GSD_DailyEventsTests, EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter);

    bool RunTest(const FString& Parameters) override;

    /**
     * Test suite for UGSDEventSpawnZone event tag support
     */
    DECLARE_SPEC(FGSDEventSpawnZoneEventTagTest, GSD_DailyEventsTests, EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter);

    bool RunTest(const FString& Parameters) override;

#endif
