#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "GSDCityStreamingSettings.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDCityStreamingSettingsTest,
    "GSD.CityStreaming.Settings.DefaultValues",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDCityStreamingSettingsTest::RunTest(const FString& Parameters)
{
    // Test default grid cell size
    const UGSDCityStreamingSettings* Settings = UGSDCityStreamingSettings::Get();
    TestNotNull("Settings should exist", Settings);

    if (Settings)
    {
        // Verify default grid cell size is 25600 (256m)
        TestEqual(TEXT("Default grid cell size should be 25600"),
            Settings->DefaultGridCellSize, 25600);

        // Verify default loading range is 76800 (768m)
        TestEqual(TEXT("Default loading range should be 76800"),
            Settings->DefaultLoadingRange, 76800);

        // Verify HLOD tier count
        TestEqual(TEXT("HLOD tier count should be 3"),
            Settings->HLODTierCount, 3);

        // Verify auto-generate collision
        TestTrue(TEXT("Auto-generate collision should be true"),
            Settings->bAutoGenerateCollision);

        // Verify import scale
        TestEqual(TEXT("Import scale should be 1.0"),
            Settings->ImportScale, 1.0f);
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDCityStreamingSettingsValidationTest,
    "GSD.CityStreaming.Settings.Validation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDCityStreamingSettingsValidationTest::RunTest(const FString& Parameters)
{
    const UGSDCityStreamingSettings* Settings = UGSDCityStreamingSettings::Get();
    TestNotNull("Settings should exist", Settings);

    if (Settings)
    {
        // Test validation: grid cell size within bounds
        TestTrue("Grid cell size should be >= 12800",
            Settings->DefaultGridCellSize >= 12800);
        TestTrue("Grid cell size should be <= 102400",
            Settings->DefaultGridCellSize <= 102400);

        // Test validation: loading range >= grid cell size * 2
        TestTrue("Loading range should be >= 51200",
            Settings->DefaultLoadingRange >= 51200);

        // Test validation: HLOD tier count within bounds
        TestTrue("HLOD tier count should be >= 1",
            Settings->HLODTierCount >= 1);
        TestTrue("HLOD tier count should be <= 5",
            Settings->HLODTierCount <= 5);
    }

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
