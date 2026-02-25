#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Types/GSDHLODTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDHLODTypesEnumTest,
    "GSD.CityStreaming.HLOD.Types.EnumValues",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDHLODTypesEnumTest::RunTest(const FString& Parameters)
{
    // Test enum values exist
    TestEqual("Instancing type should be 0",
        static_cast<int32>(EGSDHLODLayerType::Instancing), 0);
    TestEqual("MergedMesh type should be 1",
        static_cast<int32>(EGSDHLODLayerType::MergedMesh), 1);
    TestEqual("SimplifiedMesh type should be 2",
        static_cast<int32>(EGSDHLODLayerType::SimplifiedMesh), 2);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDHLODDefaultConfigTest,
    "GSD.CityStreaming.HLOD.Config.DefaultTiers",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDHLODDefaultConfigTest::RunTest(const FString& Parameters)
{
    FGSDHLODSystemConfig Config = GSDHLODHelpers::CreateDefaultCityHLODConfig();

    // Should have 3 tiers
    TestEqual("Should have 3 HLOD tiers", Config.Layers.Num(), 3);

    if (Config.Layers.Num() >= 3)
    {
        // Tier 1 - Foliage (Instancing)
        TestEqual("Tier 1 should be Instancing",
            Config.Layers[0].LayerType, EGSDHLODLayerType::Instancing);
        TestEqual("Tier 1 cell size should be 51200",
            Config.Layers[0].CellSize, 51200);
        TestEqual("Tier 1 loading range should be 30000",
            Config.Layers[0].LoadingRange, 30000);

        // Tier 2 - Buildings (MergedMesh)
        TestEqual("Tier 2 should be MergedMesh",
            Config.Layers[1].LayerType, EGSDHLODLayerType::MergedMesh);
        TestEqual("Tier 2 cell size should be 25600",
            Config.Layers[1].CellSize, 25600);
        TestEqual("Tier 2 loading range should be 50000",
            Config.Layers[1].LoadingRange, 50000);
        TestTrue("Tier 2 should have Nanite enabled",
            Config.Layers[1].bGenerateNaniteMesh);

        // Tier 3 - Terrain (SimplifiedMesh)
        TestEqual("Tier 3 should be SimplifiedMesh",
            Config.Layers[2].LayerType, EGSDHLODLayerType::SimplifiedMesh);
        TestEqual("Tier 3 cell size should be 102400",
            Config.Layers[2].CellSize, 102400);
        TestEqual("Tier 3 loading range should be 100000",
            Config.Layers[2].LoadingRange, 100000);
    }

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
