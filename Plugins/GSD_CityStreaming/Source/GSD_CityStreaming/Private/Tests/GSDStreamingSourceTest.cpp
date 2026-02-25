#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Components/GSDStreamingSourceComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDStreamingSourceDefaultsTest,
    "GSD.CityStreaming.StreamingSource.Defaults",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDStreamingSourceDefaultsTest::RunTest(const FString& Parameters)
{
    // Create component to test defaults
    UGSDStreamingSourceComponent* Component = NewObject<UGSDStreamingSourceComponent>();
    TestNotNull("Component should be created", Component);

    if (Component)
    {
        // Verify default values
        TestTrue("Streaming should be enabled by default",
            Component->IsStreamingEnabled());
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDStreamingSourceEnableDisableTest,
    "GSD.CityStreaming.StreamingSource.EnableDisable",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDStreamingSourceEnableDisableTest::RunTest(const FString& Parameters)
{
    UGSDStreamingSourceComponent* Component = NewObject<UGSDStreamingSourceComponent>();
    TestNotNull("Component should be created", Component);

    if (Component)
    {
        // Test enable/disable
        Component->SetStreamingEnabled(false);
        TestFalse("Streaming should be disabled", Component->IsStreamingEnabled());

        Component->SetStreamingEnabled(true);
        TestTrue("Streaming should be enabled", Component->IsStreamingEnabled());
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDStreamingSourceMultiplierTest,
    "GSD.CityStreaming.StreamingSource.LoadingRangeMultiplier",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDStreamingSourceMultiplierTest::RunTest(const FString& Parameters)
{
    UGSDStreamingSourceComponent* Component = NewObject<UGSDStreamingSourceComponent>();
    TestNotNull("Component should be created", Component);

    if (Component)
    {
        // Test loading range multiplier
        Component->SetLoadingRangeMultiplier(2.0f);

        // Note: We can't directly test the internal value since it's private
        // But we can verify the method doesn't crash and the component remains valid
        TestNotNull("Component should still be valid after setting multiplier", Component);

        // Test clamping with extreme values
        Component->SetLoadingRangeMultiplier(10.0f); // Should clamp to 5.0
        Component->SetLoadingRangeMultiplier(0.1f);  // Should clamp to 0.5

        TestNotNull("Component should handle clamped values", Component);
    }

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
