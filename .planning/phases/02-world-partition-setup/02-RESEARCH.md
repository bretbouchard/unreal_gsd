# Phase 2: World Partition Setup - Research

**Researched:** 2026-02-25
**Domain:** Unreal Engine 5 World Partition system for large-scale city environments
**Confidence:** HIGH

## Summary

World Partition is Unreal Engine 5's automatic data management and distance-based level streaming system, designed specifically for large open-world environments. For the Charlotte urban environment in Zombie Taxi, World Partition replaces the legacy Level Streaming system by storing the world in a single persistent level divided into grid cells that are dynamically loaded/unloaded based on streaming sources (typically the player).

This research covers the complete setup workflow: enabling World Partition with grid-based cells optimized for urban density, configuring HLOD (Hierarchical Level of Detail) for distant content visualization, implementing the One File Per Actor (OFPA) workflow for team collaboration, and establishing import pipelines for USD/FBX city tiles with correct scale and collision.

**Primary recommendation:** Use the Open World template as the foundation, configure 25,600 UU grid cells for Charlotte's urban density, enable 3-tier HLOD system (Instancing for foliage, Merged Mesh for buildings, Simplified Mesh for distant terrain), and ensure OFPA is enabled by default for all World Partition levels.

## Standard Stack

The established components for World Partition implementation:

### Core
| Component | Version | Purpose | Why Standard |
|-----------|---------|---------|--------------|
| World Partition System | UE 5.4+ | Automatic grid-based streaming | Built-in UE5 feature, replaces Level Streaming |
| HLOD System | UE 5.4+ | Hierarchical LOD for distant content | Reduces draw calls, essential for large worlds |
| One File Per Actor (OFPA) | UE 5.0+ | Per-actor file storage | Enabled by default with World Partition, enables team collaboration |
| USD Importer | UE 5.4+ | Universal Scene Description import | Industry standard for DCC-to-engine workflows |
| FBX Importer | UE 5.4+ | Traditional mesh import | Most compatible format for city tiles |

### Supporting
| Component | Purpose | When to Use |
|-----------|---------|-------------|
| World Partition Builder Commandlets | Automated HLOD generation, minimap building | CI/CD pipelines, batch processing |
| Data Layers | Runtime-controllable content layers | Dynamic events, day/night variations |
| Location Volumes | Editor-only region loading markers | Designating work areas in large maps |
| World Partition Streaming Source Component | Custom streaming triggers | Vehicles, teleport destinations |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| World Partition | Level Streaming (UE4) | Legacy system, manual sublevel management, no OFPA |
| Grid-based cells | 3D spatial hash | 2D sufficient for city terrain, 3D for underground/multi-level |
| HLOD Mesh Merge | HLOD Simplified Mesh | Mesh Merge preserves detail, Simplified reduces poly count significantly |
| USD Workflow | FBX-only workflow | FBX simpler but less flexible for large scenes, USD supports layering |

**Setup:**
1. Create new level using **Open World** template (auto-enables World Partition + OFPA + HLOD)
2. Configure grid cell size in World Settings > World Partition Setup
3. Create HLOD Layer assets for different content types
4. Run HLOD generation via Build menu or commandlet

## Architecture Patterns

### Recommended Project Structure

```
GSD_CityStreaming/              # Plugin for World Partition management
├── Source/
│   ├── GSD_CityStreaming.Build.cs
│   ├── Public/
│   │   ├── GSDCityStreamingSettings.h      # World Partition configuration
│   │   ├── GSDStreamingSourceComponent.h   # Custom streaming sources
│   │   └── IGSDCityTileImporter.h         # Import interface
│   └── Private/
│       ├── GSDCityStreamingSettings.cpp
│       ├── GSDStreamingSourceComponent.cpp
│       └── GSDCityTileImporter.cpp        # USD/FBX import logic
├── Content/
│   ├── HLOD/                   # HLOD Layer assets
│   │   ├── HLOD_Buildings.uasset
│   │   ├── HLOD_Foliage.uasset
│   │   └── HLOD_Terrain.uasset
│   └── Maps/
│       └── Charlotte_City.umap  # Main World Partition level
└── GSD_CityStreaming.uplugin
```

### Pattern 1: Grid Cell Configuration

**What:** Configure World Partition runtime grid for Charlotte urban density
**When to use:** Initial level setup for open world city

**World Settings Configuration:**
```
World Partition Setup > Runtime Grids:
  - Grid Name: "MainGrid"
  - Cell Size: 25600 (256m x 256m for urban density)
  - Loading Range: 76800 (768m radius - loads 3x3 cell grid)
  - Block on Slow Streaming: Enabled (prevents pop-in)
```

**Source:** [Epic Games Documentation - World Partition](https://dev.epicgames.com/documentation/en-us/unreal-engine/world-partition-in-unreal-engine)

**Rationale:**
- 256m cells balance memory usage with streaming frequency
- Charlotte's downtown blocks are ~100m, so 2.5 blocks per cell
- 768m loading range ensures smooth driving experience (loads ahead before arrival)

### Pattern 2: Three-Tier HLOD System

**What:** Configure multiple HLOD layers for different content types
**When to use:** Large open worlds with varied content (buildings, foliage, terrain)

**HLOD Layer Setup:**

```
Tier 1 - Foliage (Instancing):
  - Layer Type: Instancing
  - Cell Size: 51200 (512m)
  - Loading Range: 30000 (300m)
  - Uses lowest LOD for tree imposters

Tier 2 - Buildings (Mesh Merge):
  - Layer Type: Merged Mesh
  - Cell Size: 25600 (256m)
  - Loading Range: 50000 (500m)
  - Target Lightmap Resolution: 256
  - Merge Materials: Enabled
  - Generate Nanite Enabled Mesh: Yes

Tier 3 - Distant Terrain (Simplified Mesh):
  - Layer Type: Simplified Mesh
  - Cell Size: 102400 (1km)
  - Loading Range: 100000 (1km)
  - Screen Size: 0.1 (aggressive simplification)
  - Merge Distance: 100 (closes small gaps)
```

**Source:** [Epic Games Documentation - World Partition HLOD](https://dev.epicgames.com/documentation/en-us/unreal-engine/world-partition---hierarchical-level-of-detail-in-unreal-engine)

### Pattern 3: One File Per Actor Workflow

**What:** Enable OFPA for team collaboration on large maps
**When to use:** All World Partition levels (enabled by default)

**Enabling OFPA:**
```
Window > World Settings > World > Use External Actors: Checked
```

**Key behaviors:**
- Each Actor saved as separate `.uasset` file
- Main level file no longer needs checkout for actor modifications
- Multiple team members can edit different actors simultaneously
- Actors embedded back into level during cooking (editor-only feature)

**Source:** [Epic Games Documentation - One File Per Actor](https://dev.epicgames.com/documentation/en-us/unreal-engine/one-file-per-actor-in-unreal-engine)

### Pattern 4: USD/FBX Import Pipeline

**What:** Import city tiles from blender_gsd with correct scale and collision
**When to use:** Initial city data import from external DCC tools

**Import Workflow:**
```
1. USD Import (Preferred):
   File > Import into Level > Select USD file
   - Import Type: Static Mesh
   - Auto Generate Collision: Enabled
   - Scale: 1.0 (USD uses centimeters, matches UE)

2. FBX Import (Fallback):
   File > Import into Level > Select FBX file
   - Auto Generate Collision: Convex Hull
   - Scale: 1.0 (FBX should be exported in cm from Blender)
   - Combine Meshes: Disabled (keep separate for HLOD)
```

**Best Practices:**
- USD preferred for large scenes (supports layering, better material handling)
- Use relative texture paths in USD: `./textures/` not absolute paths
- Convert USD materials to UsdPreviewSurface standard for best compatibility
- Verify collision generation in Editor before runtime testing

**Source:** [Epic Games Documentation - FBX Import Options](https://dev.epicgames.com/documentation/zh-cn/unreal-engine/fbx-import-options-reference-in-unreal-engine)

### Anti-Patterns to Avoid

- **Anti-pattern:** Disabling OFPA for World Partition levels
  **Why it's bad:** Loses team collaboration benefits, requires level file checkout for any actor changes
  **What to do instead:** Always use OFPA with World Partition (enabled by default)

- **Anti-pattern:** Using single HLOD layer for all content types
  **Why it's bad:** Different content needs different LOD strategies (trees need imposters, buildings need merged meshes)
  **What to do instead:** Create separate HLOD layers per content type (foliage, buildings, terrain)

- **Anti-pattern:** Setting grid cell size too small (<12800 UU)
  **Why it's bad:** Excessive I/O thrashing, constant cell loading/unloading, poor performance
  **What to do instead:** Use 25600 UU minimum for urban environments, 51200 UU for suburban

- **Anti-pattern:** Setting loading range too small (<51200 UU)
  **Why it's bad:** Visible pop-in as player moves, especially at driving speeds
  **What to do instead:** Use 76800 UU minimum for vehicle gameplay, higher for faster speeds

- **Anti-pattern:** Building HLODs in-editor for large cities
  **Why it's bad:** Extremely slow (40+ minutes for large city), blocks editor
  **What to do instead:** Use commandlet for HLOD generation: `UnrealEditor.exe Project -run=WorldPartitionBuilderCommandlet -Builder=WorldPartitionHLODsBuilder`

## Don't Hand-Roll

Problems that look simple but have existing solutions:

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Streaming source management | Custom streaming component | World Partition Streaming Source Component | Built-in component handles priority, shapes, grid targeting |
| HLOD generation | Custom LOD merging system | World Partition HLOD Builder commandlet | Handles mesh merging, material baking, Nanite generation |
| Grid cell management | Manual spatial partitioning | World Partition Runtime Spatial Hash | Optimized spatial hashing, handles actor assignment automatically |
| Actor file management | Custom actor serialization | One File Per Actor system | Built-in OFPA handles checkout, save, source control integration |
| City tile import | Custom USD/FBX parser | USD Stage Actor / FBX Importer | Handles scale, materials, collision, animation automatically |
| Minimap generation | Manual screenshot stitching | World Partition Minimap Builder commandlet | Automated minimap with correct cell boundaries |
| Level conversion | Manual actor migration | WorldPartitionConvertCommandlet | Converts existing levels with HLOD layer setup |

**Key insight:** World Partition is a mature system with comprehensive tooling. Custom solutions for streaming, LOD, or import will be inferior to built-in features and require significant maintenance. Use commandlets for batch operations (HLOD generation, minimap building) rather than in-editor tools for large worlds.

## Common Pitfalls

### Pitfall 1: Grid Cell Size Too Small

**What goes wrong:** Excessive I/O operations, constant cell loading/unloading, frame rate stuttering
**Why it happens:** Developers assume smaller cells = better memory management, but streaming overhead dominates
**How to avoid:**
- Use 25,600 UU minimum for urban environments
- Use 51,200 UU for suburban/rural areas
- Monitor `wp.Runtime.MaxLoadingLevelStreamingCells` console command during testing
**Warning signs:**
- Frame time spikes when moving through world
- "Streaming cell load" messages in log at high frequency
- HDD/SSD activity light constantly blinking

### Pitfall 2: HLOD Not Generated Before Testing

**What goes wrong:** Distant areas appear empty during PIE/Simulate, only nearby content loads
**Why it happens:** HLODs must be explicitly built, they don't auto-generate
**How to avoid:**
- Always run Build > Build HLODs after content changes
- Use commandlet for large worlds: `UnrealEditor.exe Project Map.umap -run=WorldPartitionBuilderCommandlet -Builder=WorldPartitionHLODsBuilder -AllowCommandletRendering`
- Verify HLODs with `wp.Runtime.ToggleDrawRuntimeHash2D` console command
**Warning signs:**
- Empty horizon in PIE despite world partition editor showing loaded cells
- "No HLOD actor found" warnings in log

### Pitfall 3: OFPA Disabled After Level Conversion

**What goes wrong:** Team collaboration breaks, source control conflicts increase
**Why it happens:** WorldPartitionConvertCommandlet doesn't force-enable OFPA by default
**How to avoid:**
- Verify `Use External Actors` checkbox in World Settings after conversion
- Use conversion INI file with explicit OFPA settings
- Check for `.uasset` files in `__ExternalActors__` folder
**Warning signs:**
- Large `.umap` file size (should be small with OFPA)
- Source control conflicts on level file when multiple people editing

### Pitfall 4: Incorrect Import Scale

**What goes wrong:** City tiles import at wrong size (too large/small), collision misaligned
**Why it happens:** Blender uses meters, Unreal uses centimeters, export settings incorrect
**How to avoid:**
- Export from Blender with "Scene Scale: 1.0" (already in cm)
- Verify import scale preview before confirming import
- Check collision visualization in Editor (Show > Collision)
- Test with player character (196 cm tall reference)
**Warning signs:**
- Doorways too small/large for player character
- Collision doesn't match visual mesh
- Grid cells appear empty despite having geometry

### Pitfall 5: Missing Collision on Imported Meshes

**What goes wrong:** Player/vehicles fall through geometry, no physics interaction
**Why it happens:** FBX import "Auto Generate Collision" disabled, or USD materials don't include collision
**How to avoid:**
- Enable "Auto Generate Collision: Convex Hull" during FBX import
- For USD, verify collision primitives in `usdview` before import
- Use "Simple Collision" view in Editor to verify (Show > Collision)
- Test collision in PIE before proceeding with content work
**Warning signs:**
- Player falls through floor on spawn
- Vehicles clip through buildings
- Line traces return no hits on visible geometry

### Pitfall 6: Loading Range Too Short for Vehicle Speeds

**What goes wrong:** Pop-in visible while driving, cells load too late causing stuttering
**Why it happens:** Loading range calculated for walking speed, not driving speed
**How to avoid:**
- Calculate loading range based on max vehicle speed: `LoadingRange = MaxSpeed * LoadTime`
- For 30 m/s driving speed (108 km/h) with 2s load time: `30 * 2 = 60m minimum`
- Use 76,800 UU (768m) minimum for urban driving gameplay
- Test streaming with `stat streaming` console command
**Warning signs:**
- Visible pop-in when driving at speed
- Frame time spikes correlated with movement
- "Streaming cell not loaded" warnings in log

### Pitfall 7: VRAM Exhaustion with Large Cities

**What goes wrong:** GPU memory errors, crash to desktop, texture streaming issues
**Why it happens:** Loading too many high-res assets simultaneously, HLODs not working correctly
**How to avoid:**
- Use HLODs to reduce memory for distant content
- Monitor VRAM with `stat gpu` and `stat streaming`
- Adjust `r.Streaming.PoolSize` console command (default 1000 MB)
- Reduce texture LOD bias in scalability settings if needed
**Warning signs:**
- "Out of video memory" errors
- Textures appearing blurry/low-res
- Frame rate drops over time (memory leak)

## Code Examples

### C++: GSDCityStreamingSettings Configuration

```cpp
// Source: Based on Epic Games World Partition architecture
// File: GSD_CityStreaming/Public/GSDCityStreamingSettings.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GSDCityStreamingSettings.generated.h"

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="GSD City Streaming"))
class GSD_CITYSTREAMING_API UGSDCityStreamingSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    // Grid cell size in Unreal Units (1 UU = 1 cm)
    // 25600 UU = 256 meters - optimized for urban density
    UPROPERTY(Config, EditAnywhere, Category="World Partition|Grid", meta=(ClampMin=12800, ClampMax=102400))
    int32 DefaultGridCellSize = 25600;

    // Loading range in Unreal Units
    // 76800 UU = 768 meters - loads 3x3 grid around player
    UPROPERTY(Config, EditAnywhere, Category="World Partition|Grid", meta=(ClampMin=25600))
    int32 DefaultLoadingRange = 76800;

    // HLOD generation settings
    UPROPERTY(Config, EditAnywhere, Category="HLOD")
    bool bAutoGenerateHLODs = true;

    // Number of HLOD tiers (typically 3: foliage, buildings, terrain)
    UPROPERTY(Config, EditAnywhere, Category="HLOD", meta=(ClampMin=1, ClampMax=5))
    int32 HLODTierCount = 3;

    // Import settings
    UPROPERTY(Config, EditAnywhere, Category="Import")
    bool bAutoGenerateCollision = true;

    UPROPERTY(Config, EditAnywhere, Category="Import")
    float ImportScale = 1.0f;

    // Get settings singleton
    static UGSDCityStreamingSettings* Get();
};
```

### C++: Custom Streaming Source Component

```cpp
// Source: Based on Epic Games World Partition Streaming Source
// File: GSD_CityStreaming/Public/GSDStreamingSourceComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WorldPartition/WorldPartitionStreamingSourceComponent.h"
#include "GSDStreamingSourceComponent.generated.h"

/**
 * Custom streaming source for vehicles and predictive loading.
 * Extends base World Partition streaming source with GSD-specific features.
 */
UCLASS(ClassGroup=(GSD), meta=(BlueprintSpawnableComponent))
class GSD_CITYSTREAMING_API UGSDStreamingSourceComponent : public UWorldPartitionStreamingSourceComponent
{
    GENERATED_BODY()

public:
    UGSDStreamingSourceComponent();

    // Enable/disable streaming (useful for parked vehicles)
    UFUNCTION(BlueprintCallable, Category="GSD|Streaming")
    void SetStreamingEnabled(bool bEnabled);

    // Set custom loading range multiplier (for fast vehicles)
    UFUNCTION(BlueprintCallable, Category="GSD|Streaming")
    void SetLoadingRangeMultiplier(float Multiplier);

protected:
    virtual void BeginPlay() override;

    // Override to apply custom settings
    virtual void UpdateStreamingSourceState();

private:
    // Multiplier for loading range (1.0 = default, 2.0 = double range for fast vehicles)
    UPROPERTY(EditAnywhere, Category="GSD|Streaming")
    float LoadingRangeMultiplier = 1.0f;

    // Should this source load cells ahead of movement direction?
    UPROPERTY(EditAnywhere, Category="GSD|Streaming")
    bool bPredictiveLoading = false;

    // Velocity threshold for predictive loading (in cm/s)
    UPROPERTY(EditAnywhere, Category="GSD|Streaming")
    float PredictiveLoadingVelocityThreshold = 1000.0f;
};
```

### Blueprint: World Partition Grid Configuration

```cpp
// World Settings Configuration (set in Editor)
// This is configured in World Settings panel, not code

World Partition Setup:
  Enable Streaming: true
  Runtime Grids:
    [0]:
      Grid Name: "MainGrid"
      Cell Size: 25600
      Loading Range: 76800
      Block on Slow Streaming: true
      Priority: 0
      Debug Color: (R=1, G=0, B=0, A=1)

  Editor Hash:
    Cell Size: 51200  // Larger cells for editor performance

  HLOD Settings:
    HLOD Layer Assets Path: "/Game/HLOD"
    Default HLOD Layer Name: "HLODLayer_Default"
```

### Commandlet: HLOD Generation

```bash
# Source: Epic Games World Partition Builder Commandlet Reference
# Generate HLODs for large city using commandlet (faster than in-editor)

# Windows
UnrealEditor.exe "C:/Projects/ZombieTaxi/ZombieTaxi.uproject" "/Game/Maps/Charlotte_City" -run=WorldPartitionBuilderCommandlet -AllowCommandletRendering -Builder=WorldPartitionHLODsBuilder -D3D11

# Mac/Linux
./UnrealEditor "/Projects/ZombieTaxi/ZombieTaxi.uproject" "/Game/Maps/Charlotte_City" -run=WorldPartitionBuilderCommandlet -AllowCommandletRendering -Builder=WorldPartitionHLODsBuilder

# Options:
# -SetupHLODs: Create HLOD actors without building geometry (fast)
# -BuildHLODs: Build geometry for existing HLOD actors
# -DeleteHLODs: Remove all HLOD actors
```

### Commandlet: World Partition Conversion

```bash
# Convert existing level to World Partition with OFPA enabled
UnrealEditor.exe ZombieTaxi -run=WorldPartitionConvertCommandlet OldCity.umap -AllowCommandletRendering -Verbose

# Optional arguments:
# -ConversionSuffix: Adds "_WP" suffix to converted map
# -DeleteSourceLevels: Removes original after conversion
# -SCCProvider=None: Run without source control
```

### Console Commands: Runtime Debugging

```cpp
// World Partition runtime debugging commands

// Toggle 2D debug display of runtime hash
wp.Runtime.ToggleDrawRuntimeHash2D

// Override loading range for testing
wp.Runtime.OverrideRuntimeSpatialHashLoadingRange -grid=0 -range=100000

// Limit concurrent streaming cells (performance tuning)
wp.Runtime.MaxLoadingLevelStreamingCells 10

// Show HLOD status (green = loaded)
wp.Runtime.ToggleDrawRuntimeHash2D

// Streaming performance stats
stat streaming
stat gpu
stat memory
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Level Streaming (UE4) | World Partition (UE5) | UE 5.0 (2022) | Automatic grid-based streaming, no manual sublevel management |
| Single level file | One File Per Actor | UE 5.0 (2022) | Team collaboration on large maps without conflicts |
| Manual LOD setup | HLOD Layers | UE 5.0 (2022) | Automatic hierarchical LOD generation with multiple strategies |
| FBX-only import | USD + FBX import | UE 4.27 (2021) | Industry-standard USD workflow for DCC-to-engine |
| Per-platform cooking | World Partition cooking | UE 5.0 (2022) | Chunked cooking for streaming worlds |

**Deprecated/outdated:**
- **Level Streaming (UE4):** Replaced by World Partition in UE5. Manual sublevel management, no automatic grid-based streaming.
- **Always-loaded actors:** In World Partition, use "Is Spatially Loaded" flag to control loading behavior. Only use "Always Loaded" for global managers and skyboxes.

## Open Questions

### 1. Grid Cell Size for Multi-Level City Areas

**What we know:**
- Default 2D grid works well for ground-level content
- Charlotte has parking decks, bridges, overpasses that span multiple Z levels

**What's unclear:**
- Should we use 3D spatial hash for areas with significant verticality?
- Performance impact of 3D hash vs. 2D hash for urban environments

**Recommendation:**
- Start with 2D grid (simpler, better performance)
- Use Data Layers for underground areas (parking decks, subways)
- Only consider 3D hash if testing shows 2D is insufficient

### 2. HLOD Generation Frequency

**What we know:**
- HLODs must be rebuilt when source content changes
- Commandlet generation takes 10-40 minutes for large cities

**What's unclear:**
- How often will city tiles change during development?
- Should HLOD generation be part of CI/CD pipeline?

**Recommendation:**
- Generate HLODs on-demand during active development
- Automate HLOD generation in CI/CD for main branch builds
- Cache HLOD assets separately from source geometry

### 3. Streaming Source Attachment for Vehicles

**What we know:**
- Player Controller has streaming source by default
- Vehicles need streaming sources for predictive loading

**What's unclear:**
- Should all vehicles have streaming sources, or only player-driven?
- How to handle streaming for AI vehicles that may be far from player?

**Recommendation:**
- Only player-driven vehicles need streaming sources
- AI vehicles load/unload with player proximity
- Disable streaming source for parked vehicles to save performance

## Sources

### Primary (HIGH confidence)
- [Epic Games Documentation - World Partition in Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/world-partition-in-unreal-engine) - Official UE 5.7 documentation covering complete World Partition system
- [Epic Games Documentation - One File Per Actor](https://dev.epicgames.com/documentation/en-us/unreal-engine/one-file-per-actor-in-unreal-engine) - Official UE 5.6 documentation for OFPA workflow
- [Epic Games Documentation - World Partition HLOD](https://dev.epicgames.com/documentation/en-us/unreal-engine/world-partition---hierarchical-level-of-detail-in-unreal-engine) - Official documentation for HLOD generation and configuration
- [Epic Games Documentation - City Sample Quick Start](https://dev.epicgames.com/documentation/en-us/unreal-engine/city-sample-quick-start-for-generating-a-city-and-freeway-in-unreal-engine-5) - Real-world example of city import and HLOD generation

### Secondary (MEDIUM confidence)
- [Epic Games Documentation - FBX Import Options](https://dev.epicgames.com/documentation/zh-cn/unreal-engine/fbx-import-options-reference-in-unreal-engine) - FBX import settings including collision generation
- WebSearch: "Unreal Engine World Partition setup best practices 2026" - Community best practices verified against official docs
- WebSearch: "Unreal Engine World Partition grid cell size configuration 2026" - Grid sizing recommendations from community and official docs
- WebSearch: "Unreal Engine USD import workflow best practices 2026" - USD workflow patterns from community sources

### Tertiary (LOW confidence)
- WebSearch: "Unreal Engine World Partition common pitfalls mistakes 2026" - Community-reported issues, not all verified with official sources
- CSDN blog posts on World Partition architecture - Secondary sources, Chinese community documentation

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH - All components are official Unreal Engine features with comprehensive documentation
- Architecture: HIGH - Patterns based on official documentation and City Sample reference project
- Pitfalls: MEDIUM - Most pitfalls verified with official docs, some from community reports

**Research date:** 2026-02-25
**Valid until:** 2026-05-25 (3 months - World Partition is stable in UE5.x, but best practices may evolve)
