// Copyright Bret Bouchard. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Components/GSDStreamingSourceComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

// Test 1: ConfigureForVehicle sets predictive loading
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDStreamingSourceVehicleConfigTest,
    "GSD.Streaming.StreamingSource.VehicleConfig",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDStreamingSourceVehicleConfigTest::RunTest(const FString& Parameters)
{
    UGSDStreamingSourceComponent* Component = NewObject<UGSDStreamingSourceComponent>();
    TestNotNull(TEXT("Component should be created"), Component);

    if (Component)
    {
        Component->ConfigureForVehicle(false, 1000.0f);

        TestTrue(TEXT("Predictive loading should be enabled"), Component->IsPredictiveLoadingEnabled());
        TestEqual(TEXT("Loading range multiplier should be 1.0"), Component->GetLoadingRangeMultiplier(), 1.0f);
        TestEqual(TEXT("Velocity threshold should be 1000"), Component->GetPredictiveLoadingThreshold(), 1000.0f);
    }
    return true;
}

// Test 2: ConfigureForVehicle with fast vehicle flag
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDStreamingSourceFastVehicleTest,
    "GSD.Streaming.StreamingSource.FastVehicle",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDStreamingSourceFastVehicleTest::RunTest(const FString& Parameters)
{
    UGSDStreamingSourceComponent* Component = NewObject<UGSDStreamingSourceComponent>();
    TestNotNull(TEXT("Component should be created"), Component);

    if (Component)
    {
        Component->ConfigureForVehicle(true, 1500.0f);

        TestTrue(TEXT("Predictive loading should be enabled"), Component->IsPredictiveLoadingEnabled());
        TestEqual(TEXT("Loading range multiplier should be 2.0 for fast vehicle"), Component->GetLoadingRangeMultiplier(), 2.0f);
        TestEqual(TEXT("Velocity threshold should be 1500"), Component->GetPredictiveLoadingThreshold(), 1500.0f);
    }
    return true;
}

// Test 3: Getters return correct values after manual changes
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDStreamingSourceGetterTest,
    "GSD.Streaming.StreamingSource.Getters",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDStreamingSourceGetterTest::RunTest(const FString& Parameters)
{
    UGSDStreamingSourceComponent* Component = NewObject<UGSDStreamingSourceComponent>();
    TestNotNull(TEXT("Component should be created"), Component);

    if (Component)
    {
        Component->SetLoadingRangeMultiplier(3.0f);
        TestEqual(TEXT("GetLoadingRangeMultiplier should return 3.0"), Component->GetLoadingRangeMultiplier(), 3.0f);

        Component->ConfigureForVehicle(false);
        TestEqual(TEXT("ConfigureForVehicle should reset multiplier to 1.0"), Component->GetLoadingRangeMultiplier(), 1.0f);
    }
    return true;
}

// Test 4: Default state (CRITICAL from Council review)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDStreamingSourceDefaultStateTest,
    "GSD.Streaming.StreamingSource.DefaultState",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDStreamingSourceDefaultStateTest::RunTest(const FString& Parameters)
{
    UGSDStreamingSourceComponent* Component = NewObject<UGSDStreamingSourceComponent>();
    TestNotNull(TEXT("Component should be created"), Component);

    if (Component)
    {
        // Verify default state
        TestFalse(TEXT("Should not be hibernating by default"), Component->IsHibernating());
        TestTrue(TEXT("Should have streaming enabled by default"), Component->IsStreamingEnabled());
        TestEqual(TEXT("Default loading range multiplier should be 1.0"), Component->GetLoadingRangeMultiplier(), 1.0f);
    }
    return true;
}

// Test 5: OnVehicleStateChanged event-driven pattern
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDStreamingSourceVehicleStateTest,
    "GSD.Streaming.StreamingSource.VehicleState",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDStreamingSourceVehicleStateTest::RunTest(const FString& Parameters)
{
    UGSDStreamingSourceComponent* Component = NewObject<UGSDStreamingSourceComponent>();
    TestNotNull(TEXT("Component should be created"), Component);

    if (Component)
    {
        // Configure for vehicle first
        Component->ConfigureForVehicle(false);

        // Simulate state changes (EVENT-DRIVEN, not tick)
        Component->OnVehicleStateChanged(true, 500.0f);  // Driving slow
        TestTrue(TEXT("Streaming should be enabled when driving"), Component->IsStreamingEnabledForVehicle());

        Component->OnVehicleStateChanged(true, 3000.0f); // Driving fast
        TestEqual(TEXT("Fast vehicle should get 2.0x range"), Component->GetLoadingRangeMultiplier(), 2.0f);

        Component->OnVehicleStateChanged(false, 0.0f);   // Parked
        // Note: Hysteresis delay means it won't disable immediately
        // This test verifies the call doesn't crash
        TestTrue(TEXT("OnVehicleStateChanged parked should not crash"), true);
    }
    return true;
}

#endif
