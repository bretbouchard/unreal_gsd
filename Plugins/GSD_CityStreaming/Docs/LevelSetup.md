# World Partition Level Setup

## Creating a New City Level

1. Create new level using **Open World** template
   - File > New Level > Open World
   - This auto-enables World Partition + OFPA + HLOD

2. Configure World Settings
   - Window > World Settings
   - World Partition Setup > Runtime Grids
   - Add grid: "MainGrid"
     - Cell Size: 25600 (256m)
     - Loading Range: 76800 (768m)
     - Block on Slow Streaming: Enabled

3. Place GSDCityLevelActor
   - Drag from Content Browser or use Place Actors
   - Configure in Details panel
   - Actor logs configuration on BeginPlay

4. Verify OFPA Enabled
   - World Settings > World > Use External Actors: Checked
   - Check for __ExternalActors__ folder in level directory

## Grid Configuration for Charlotte

| Setting | Value | Reason |
|---------|-------|--------|
| Cell Size | 25600 UU (256m) | 2.5 city blocks per cell |
| Loading Range | 76800 UU (768m) | 3x3 grid for smooth driving |
| Block on Slow Streaming | Yes | Prevents pop-in |

## Editor Hash Configuration

For editor performance with large levels:
- Cell Size: 51200 UU (512m)
- Larger cells reduce editor overhead

## Console Commands for Verification

- `wp.Runtime.ToggleDrawRuntimeHash2D` - Show grid overlay
- `stat streaming` - Streaming performance stats
- `wp.Runtime.OverrideRuntimeSpatialHashLoadingRange -grid=0 -range=100000` - Test different ranges

## Common Issues

- **OFPA not working:** Check Use External Actors in World Settings
- **Cells not loading:** Verify streaming source is active
- **HLOD not visible:** Run Build > Build HLODs
