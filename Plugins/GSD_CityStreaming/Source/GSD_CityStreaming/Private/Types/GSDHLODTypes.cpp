#include "Types/GSDHLODTypes.h"

namespace GSDHLODHelpers
{
    FGSDHLODSystemConfig CreateDefaultCityHLODConfig()
    {
        FGSDHLODSystemConfig Config;

        // Tier 1: Foliage (Instancing)
        // - Uses lowest LOD as imposters for trees and vegetation
        // - 512m cells with 300m loading range
        // - Best for repeated objects like trees, bushes, grass
        FGSDHLODLayerConfig FoliageLayer(
            TEXT("Foliage"),
            EGSDHLODLayerType::Instancing,
            51200,  // 512m cell size
            30000   // 300m loading range
        );
        Config.Layers.Add(FoliageLayer);

        // Tier 2: Buildings (Merged Mesh)
        // - Merges building meshes while preserving geometric detail
        // - 256m cells with 500m loading range
        // - Nanite enabled for optimal rendering performance
        // - Material merging enabled to reduce draw calls
        FGSDHLODLayerConfig BuildingsLayer(
            TEXT("Buildings"),
            EGSDHLODLayerType::MergedMesh,
            25600,  // 256m cell size
            50000   // 500m loading range
        );
        BuildingsLayer.TargetLightmapResolution = 256;
        BuildingsLayer.bMergeMaterials = true;
        BuildingsLayer.bGenerateNaniteMesh = true;
        Config.Layers.Add(BuildingsLayer);

        // Tier 3: Terrain (Simplified Mesh)
        // - Aggressive polygon simplification for distant terrain
        // - 1km cells with 1km loading range
        // - Best for ground plane, hills, distant geography
        FGSDHLODLayerConfig TerrainLayer(
            TEXT("Terrain"),
            EGSDHLODLayerType::SimplifiedMesh,
            102400, // 1km cell size
            100000  // 1km loading range
        );
        TerrainLayer.ScreenSize = 0.1f;
        TerrainLayer.MergeDistance = 100.0f;
        Config.Layers.Add(TerrainLayer);

        return Config;
    }
}
