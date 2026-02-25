#pragma once

#include "CoreMinimal.h"
#include "GSDHLODTypes.generated.h"

/**
 * HLOD layer type determines the merging strategy for distant content.
 *
 * Instancing: Uses lowest LOD as imposters (best for foliage)
 * MergedMesh: Merges meshes while preserving detail (best for buildings)
 * SimplifiedMesh: Aggressive simplification (best for distant terrain)
 */
UENUM(BlueprintType)
enum class EGSDHLODLayerType : uint8
{
    /** Uses lowest LOD as imposters - best for foliage and repeated objects */
    Instancing      UMETA(DisplayName = "Instancing"),

    /** Merges meshes while preserving geometric detail - best for buildings */
    MergedMesh      UMETA(DisplayName = "Merged Mesh"),

    /** Aggressive polygon simplification - best for distant terrain */
    SimplifiedMesh  UMETA(DisplayName = "Simplified Mesh")
};

/**
 * Configuration for a single HLOD layer.
 * Each layer represents one tier of the HLOD system with specific settings.
 */
USTRUCT(BlueprintType)
struct FGSDHLODLayerConfig
{
    GENERATED_BODY()

    // === Layer Identification ===

    /** Display name for this HLOD layer (e.g., "Foliage", "Buildings", "Terrain") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HLOD|Identity")
    FString LayerName;

    /** Layer type determines merging strategy */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HLOD|Identity")
    EGSDHLODLayerType LayerType = EGSDHLODLayerType::MergedMesh;

    // === Grid Settings ===

    /** Cell size in Unreal Units (1 UU = 1 cm) - determines HLOD grid resolution */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HLOD|Grid", meta = (ClampMin = 12800, ClampMax = 204800))
    int32 CellSize = 25600;

    /** Loading range in Unreal Units - distance at which HLOD becomes visible */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HLOD|Grid", meta = (ClampMin = 25600))
    int32 LoadingRange = 50000;

    // === Merged Mesh Settings (for Buildings) ===

    /** Target lightmap resolution for merged meshes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HLOD|MergedMesh")
    int32 TargetLightmapResolution = 256;

    /** Whether to merge similar materials to reduce draw calls */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HLOD|MergedMesh")
    bool bMergeMaterials = true;

    /** Whether to generate Nanite-enabled meshes (UE5.0+) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HLOD|MergedMesh")
    bool bGenerateNaniteMesh = true;

    // === Simplified Mesh Settings (for Terrain) ===

    /** Screen size threshold for simplification (0.0-1.0, lower = more aggressive) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HLOD|SimplifiedMesh", meta = (ClampMin = 0.01, ClampMax = 1.0))
    float ScreenSize = 0.1f;

    /** Distance threshold for merging vertices */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HLOD|SimplifiedMesh", meta = (ClampMin = 0.0))
    float MergeDistance = 100.0f;

    /** Default constructor */
    FGSDHLODLayerConfig()
        : LayerName(TEXT("Default"))
        , LayerType(EGSDHLODLayerType::MergedMesh)
        , CellSize(25600)
        , LoadingRange(50000)
        , TargetLightmapResolution(256)
        , bMergeMaterials(true)
        , bGenerateNaniteMesh(true)
        , ScreenSize(0.1f)
        , MergeDistance(100.0f)
    {}

    /** Constructor with basic parameters */
    FGSDHLODLayerConfig(const FString& InLayerName, EGSDHLODLayerType InLayerType, int32 InCellSize, int32 InLoadingRange)
        : LayerName(InLayerName)
        , LayerType(InLayerType)
        , CellSize(InCellSize)
        , LoadingRange(InLoadingRange)
        , TargetLightmapResolution(256)
        , bMergeMaterials(true)
        , bGenerateNaniteMesh(true)
        , ScreenSize(0.1f)
        , MergeDistance(100.0f)
    {}
};

/**
 * Container for the complete HLOD system configuration.
 * Holds all HLOD layers (tiers) for a World Partition level.
 */
USTRUCT(BlueprintType)
struct FGSDHLODSystemConfig
{
    GENERATED_BODY()

    /** Array of HLOD layer configurations (typically 3 tiers: Foliage, Buildings, Terrain) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HLOD")
    TArray<FGSDHLODLayerConfig> Layers;

    /** Default constructor */
    FGSDHLODSystemConfig()
    {
        Layers.Empty();
    }
};

/**
 * Helper functions for HLOD configuration.
 */
namespace GSDHLODHelpers
{
    /**
     * Create default HLOD configuration for Charlotte city environment.
     * Returns 3-tier system: Foliage (Instancing), Buildings (MergedMesh), Terrain (SimplifiedMesh)
     *
     * @return FGSDHLODSystemConfig with 3 pre-configured tiers
     */
    GSD_CITYSTREAMING_API FGSDHLODSystemConfig CreateDefaultCityHLODConfig();
}
