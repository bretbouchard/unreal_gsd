# City Tile Import Workflow

## Overview

Import city tiles from `blender_gsd` into Unreal Engine with correct scale and collision. This document describes the import pipeline and configuration options for city geometry.

## Supported Formats

### USD (Preferred)

- **File extensions:** `.usd`, `.usda`, `.usdc`
- **Benefits:**
  - Layering support for modular content
  - Better material handling with UsdPreviewSurface
  - Industry standard format
  - Better compression for large scenes
- **Best practices:**
  - Use relative texture paths: `./textures/`
  - Convert materials to UsdPreviewSurface standard
  - Export with scene scale 1.0 (centimeters)

### FBX (Fallback)

- **File extension:** `.fbx`
- **Benefits:**
  - Most compatible format
  - Well-supported across all DCC tools
  - Mature import pipeline
- **Best practices:**
  - Export from Blender with Scene Scale: 1.0 (centimeters)
  - Use binary FBX for smaller file sizes
  - Include smoothing groups for proper normals

## Import Configuration

| Setting | Default | Description |
|---------|---------|-------------|
| `SourceFilePath` | *required* | Path to USD or FBX file |
| `Format` | USD | Import format (auto-detected from extension) |
| `ImportScale` | 1.0 | Scale multiplier (should be 1.0 for cm export) |
| `CollisionType` | ConvexHull | Auto-generated collision type |
| `bAutoGenerateCollision` | true | Generate collision on import |
| `bCombineMeshes` | false | Keep meshes separate for HLOD |
| `DestinationPath` | `/Game/CityTiles` | Path for imported assets |
| `TileName` | *required* | Identifier for naming |

## Collision Types

| Type | Use Case | Performance |
|------|----------|-------------|
| **None** | Decorative-only geometry (distant props) | N/A |
| **ConvexHull** | Simple collision, fast queries (buildings, props) | Fast |
| **Simple** | Auto-generated, balanced (default for most) | Balanced |
| **Complex** | Per-triangle, exact collision (terrain) | Expensive |

**Recommendation:** Use ConvexHull for most city geometry. Only use Complex for terrain where precise collision is required.

## Workflow

### 1. Export from blender_gsd

```python
# Example export configuration in Blender
# Ensure scene scale is 1.0 (centimeters)
bpy.context.scene.unit_settings.scale_length = 0.01
bpy.context.scene.unit_settings.length_unit = 'CENTIMETERS'

# Export USD
bpy.ops.wm.usd_export(
    filepath="city_tile_001.usd",
    use_instancing=True,
    export_materials=True
)
```

### 2. Configure Import

```cpp
// C++ configuration
FGSDImportConfig Config;
Config.SourceFilePath = TEXT("D:/Exports/City/city_tile_001.usd");
Config.TileName = TEXT("CityTile_001");
Config.DestinationPath = TEXT("/Game/CityTiles/Downtown");
Config.CollisionType = EGSDCollisionType::ConvexHull;
Config.bAutoGenerateCollision = true;
```

### 3. Import Tile

```cpp
// Single tile import
FGSDImportResult Result = ImportTile(Config);
if (Result.bSuccess)
{
    UE_LOG(LogTemp, Log, TEXT("Imported: %s"), *Result.ImportedStaticMeshPath);
}

// Batch import
TArray<FGSDImportConfig> Configs;
// ... populate configs
TArray<FGSDImportResult> Results = ImportTilesBatch(Configs);
```

### 4. Verify in Editor

1. Open the imported asset in Static Mesh Editor
2. Check **Show > Collision** to verify collision hulls
3. Verify scale against player reference (196 cm character)
4. Test with gameplay (physics, navigation)

## Troubleshooting

### Wrong Scale

**Symptoms:** Geometry appears too large or too small.

**Solution:**
1. Check Blender export settings (should be 1.0 cm scale)
2. Verify `ImportScale` is 1.0
3. USD files should be in centimeters by default
4. FBX may require manual scale adjustment

### Missing Collision

**Symptoms:** Objects have no collision in-game.

**Solution:**
1. Enable `bAutoGenerateCollision = true`
2. Set `CollisionType` to ConvexHull or Simple
3. Check collision complexity in Static Mesh Editor
4. Verify collision is not set to "No Collision" in editor

### Large File Size

**Symptoms:** Imported assets are very large on disk.

**Solution:**
1. Use USD format (better compression)
2. Use `.usdc` (binary) instead of `.usda` (ASCII)
3. Enable mesh reduction in HLOD generation
4. Consider splitting into smaller tiles

### Material Issues

**Symptoms:** Materials are missing or incorrect.

**Solution:**
1. For USD: Use UsdPreviewSurface materials
2. Check texture paths are relative (`./textures/`)
3. Import textures before geometry
4. Verify material assignments in Blender

## Performance Guidelines

| Tile Size | Recommended Format | Collision Type |
|-----------|-------------------|----------------|
| Small (< 1000 triangles) | FBX | Simple |
| Medium (1000-10000 triangles) | USD | ConvexHull |
| Large (> 10000 triangles) | USD | ConvexHull |

**Best Practice:** Split large geometry into multiple tiles matching World Partition cell boundaries (256m x 256m cells).

## API Reference

### IGSDCityTileImporter

```cpp
// Import single tile
FGSDImportResult ImportTile(const FGSDImportConfig& Config);

// Import multiple tiles
TArray<FGSDImportResult> ImportTilesBatch(const TArray<FGSDImportConfig>& Configs);

// Validate configuration
bool ValidateConfig(const FGSDImportConfig& Config, FString& OutErrorMessage);

// Get supported formats
TArray<EGSDImportFormat> GetSupportedFormats() const;

// Async import (future)
void ImportTileAsync(const FGSDImportConfig& Config, const FGSDOnImportComplete& OnComplete);
```

### FGSDImportConfig

```cpp
USTRUCT(BlueprintType)
struct FGSDImportConfig
{
    FString SourceFilePath;          // Source file path
    EGSDImportFormat Format;         // USD or FBX
    float ImportScale;               // Scale multiplier (default: 1.0)
    EGSDCollisionType CollisionType; // Collision type (default: ConvexHull)
    bool bAutoGenerateCollision;     // Auto-generate (default: true)
    bool bCombineMeshes;             // Combine meshes (default: false)
    FString DestinationPath;         // Asset path (default: /Game/CityTiles)
    FString TileName;                // Tile identifier
};
```

### FGSDImportResult

```cpp
USTRUCT(BlueprintType)
struct FGSDImportResult
{
    bool bSuccess;                   // Import succeeded
    FString ErrorMessage;            // Error if failed
    TArray<FString> ImportedAssets;  // All imported assets
    FString ImportedStaticMeshPath;  // Primary static mesh path
};
```

## See Also

- [HLOD Generation](./HLODGeneration.md) - HLOD configuration for imported tiles
- [GSD_CityStreaming Settings](../Source/GSD_CityStreaming/Public/GSDCityStreamingSettings.h)
- [Import Types](../Source/GSD_CityStreaming/Public/Types/GSDImportTypes.h)
