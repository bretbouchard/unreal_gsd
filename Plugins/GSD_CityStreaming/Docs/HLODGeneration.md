# HLOD Generation for Charlotte City

## Overview

This document describes the three-tier HLOD (Hierarchical Level of Detail) system for efficient distant content rendering in the Charlotte urban environment.

HLOD reduces draw calls and maintains performance when viewing distant city content by creating simplified representations of geometry that are loaded at different distances from the player.

## Tier Configuration

The Charlotte city environment uses three HLOD tiers, each optimized for specific content types:

### Tier 1 - Foliage (Instancing)

- **Layer Type:** Instancing
- **Cell Size:** 51,200 UU (512 meters)
- **Loading Range:** 30,000 UU (300 meters)
- **Purpose:** Trees, bushes, vegetation
- **Strategy:** Uses lowest LOD as imposters for repeated objects
- **Best for:** Large numbers of similar objects like trees and grass

### Tier 2 - Buildings (Merged Mesh)

- **Layer Type:** Merged Mesh
- **Cell Size:** 25,600 UU (256 meters)
- **Loading Range:** 50,000 UU (500 meters)
- **Target Lightmap Resolution:** 256
- **Material Merging:** Enabled
- **Nanite:** Enabled
- **Purpose:** Downtown buildings, structures
- **Strategy:** Merges meshes while preserving geometric detail
- **Best for:** Complex geometry like buildings with windows and details

### Tier 3 - Terrain (Simplified Mesh)

- **Layer Type:** Simplified Mesh
- **Cell Size:** 102,400 UU (1 kilometer)
- **Loading Range:** 100,000 UU (1 kilometer)
- **Screen Size:** 0.1
- **Merge Distance:** 100.0
- **Purpose:** Ground plane, distant terrain, hills
- **Strategy:** Aggressive polygon simplification
- **Best for:** Large flat surfaces and distant geography

## Generation Methods

### In-Editor (Small Areas)

For quick iteration on small areas:

1. Open the level in Unreal Editor
2. Navigate to **Build > Build HLODs**
3. Wait for generation to complete
4. Verify with console command: `wp.Runtime.ToggleDrawRuntimeHash2D`

**Use when:**
- Testing small content changes
- Iterating on specific areas
- Quick verification during development

**Limitations:**
- Slow for large cities (40+ minutes)
- Blocks editor during generation
- Not suitable for CI/CD pipelines

### Commandlet (Large Cities)

For full city HLOD generation:

```bash
# Windows
UnrealEditor.exe "C:/Projects/ZombieTaxi/ZombieTaxi.uproject" "/Game/Maps/Charlotte_City" -run=WorldPartitionBuilderCommandlet -AllowCommandletRendering -Builder=WorldPartitionHLODsBuilder -D3D11

# Mac/Linux
./UnrealEditor "/Projects/ZombieTaxi/ZombieTaxi.uproject" "/Game/Maps/Charlotte_City" -run=WorldPartitionBuilderCommandlet -AllowCommandletRendering -Builder=WorldPartitionHLODsBuilder
```

**Options:**
- `-SetupHLODs`: Create HLOD actors without building geometry (fast setup)
- `-BuildHLODs`: Build geometry for existing HLOD actors
- `-DeleteHLODs`: Remove all HLOD actors

**Use when:**
- Generating HLODs for entire city
- CI/CD pipeline integration
- Overnight batch processing
- Initial level setup

**Benefits:**
- Faster than in-editor (10-40 minutes typical)
- Doesn't block editor
- Can run on build machines
- Suitable for automation

## Verification

After HLOD generation, verify the results:

### Visual Verification

1. Enter PIE (Play In Editor) or Simulate
2. Open console with `~` key
3. Run: `wp.Runtime.ToggleDrawRuntimeHash2D`
4. Observe:
   - **Green cells:** HLOD actors loaded correctly
   - **Empty cells:** HLOD generation may have failed
   - **Red cells:** Streaming issues

### Performance Verification

1. Run: `stat streaming` to see streaming performance
2. Run: `stat gpu` to verify GPU performance
3. Run: `stat unit` to check frame times
4. Look for:
   - Reduced draw calls compared to full-detail geometry
   - Smooth frame rate when viewing distant content
   - No visible pop-in when moving through world

### Console Commands

```cpp
// Toggle HLOD debug visualization
wp.Runtime.ToggleDrawRuntimeHash2D

// Override loading range for testing
wp.Runtime.OverrideRuntimeSpatialHashLoadingRange -grid=0 -range=100000

// Limit concurrent streaming cells (performance tuning)
wp.Runtime.MaxLoadingLevelStreamingCells 10

// Streaming performance stats
stat streaming
stat gpu
stat memory
```

## Performance Targets

Based on the RESEARCH.md analysis:

### Generation Performance

- **Small area (< 1 sq km):** 5-10 minutes in-editor
- **Medium area (1-5 sq km):** 10-20 minutes commandlet
- **Full city (Charlotte downtown):** 10-40 minutes commandlet

### Runtime Performance

- **Draw call reduction:** 70-90% for distant content
- **Memory savings:** Significant for cells beyond loading range
- **Frame time impact:** Minimal when HLOD system configured correctly

### Optimization Tips

1. **Grid Cell Size:** Use 25,600 UU minimum for urban environments
2. **Loading Range:** Use 76,800 UU minimum for vehicle gameplay
3. **HLOD Layers:** Use separate layers per content type (don't mix foliage and buildings)
4. **Nanite:** Enable for building HLODs in UE 5.0+
5. **Material Merging:** Enable to reduce draw calls

## Troubleshooting

### Issue: Distant areas appear empty

**Cause:** HLODs not generated or loading range too short

**Solution:**
1. Verify HLOD generation completed: `wp.Runtime.ToggleDrawRuntimeHash2D`
2. Check loading range in World Settings
3. Re-run HLOD generation if needed

### Issue: Pop-in while driving

**Cause:** Loading range insufficient for vehicle speed

**Solution:**
1. Increase loading range to 76,800 UU minimum
2. For fast vehicles (108 km/h+), use 100,000 UU loading range
3. Consider predictive loading for vehicles

### Issue: Slow HLOD generation

**Cause:** Large city with many actors

**Solution:**
1. Use commandlet instead of in-editor
2. Run on build machine overnight
3. Break into smaller areas if needed

### Issue: HLOD looks incorrect

**Cause:** Wrong layer type for content

**Solution:**
1. Verify layer type matches content:
   - Foliage → Instancing
   - Buildings → MergedMesh
   - Terrain → SimplifiedMesh
2. Adjust layer-specific settings (ScreenSize, MergeDistance, etc.)
3. Re-generate HLODs after changes

## Best Practices

1. **Always generate HLODs** before testing World Partition levels
2. **Use commandlet** for large worlds (faster and doesn't block editor)
3. **Verify with console commands** after generation
4. **Use separate layers** for different content types
5. **Test with vehicle speeds** to ensure loading range is sufficient
6. **Monitor performance** with `stat streaming` and `stat gpu`
7. **Commit HLOD assets** to source control (they're binary, but necessary)

## CI/CD Integration

For automated HLOD generation in CI/CD pipelines:

```yaml
# Example GitHub Actions workflow
- name: Generate HLODs
  run: |
    ./UnrealEditor "$PROJECT_FILE" "$MAP_PATH" \
      -run=WorldPartitionBuilderCommandlet \
      -AllowCommandletRendering \
      -Builder=WorldPartitionHLODsBuilder \
      -unattended \
      -nopause \
      -NullRHI \
      -log
```

**Recommendations:**
- Run HLOD generation on main branch builds
- Cache HLOD assets separately from source geometry
- Use build machines with sufficient RAM (32GB+ recommended)
- Monitor generation time and alert if exceeds threshold

## Related Documentation

- [Phase 2 RESEARCH.md](/.planning/phases/02-world-partition-setup/02-RESEARCH.md) - Complete World Partition research
- [Epic Games Documentation - World Partition HLOD](https://dev.epicgames.com/documentation/en-us/unreal-engine/world-partition---hierarchical-level-of-detail-in-unreal-engine)
- [Epic Games Documentation - World Partition](https://dev.epicgames.com/documentation/en-us/unreal-engine/world-partition-in-unreal-engine)

## Version History

- **2026-02-25:** Initial documentation for Charlotte city HLOD system
- **Phase:** 02 - World Partition Setup
- **Plan:** 02-02 - HLOD System Configuration
