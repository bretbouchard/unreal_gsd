# Data Layer Workflow

## Overview

This document explains the workflow for creating, assigning, and managing Data Layers in the GSD City Streaming system. Data Layers enable runtime content toggling for dynamic events like construction zones, block parties, and zombie raves.

## Layer Overview

| Layer Name        | Purpose                                    | Priority  | When Active              |
|-------------------|--------------------------------------------|-----------|--------------------------|
| **DL_BaseCity**   | Always-on base content (buildings, roads, terrain) | Critical  | Always (never deactivated) |
| **DL_Events**     | Dynamic event content (bonfires, rave zones) | Normal    | On-demand during events  |
| **DL_Construction** | Construction barricades, road closures    | High      | During construction events |
| **DL_Parties**    | Block party props, crowd extras           | Normal    | During party events      |

### Content Assignment Guide

| Content Type                | Assign to Layer          | Reasoning                                      |
|-----------------------------|--------------------------|------------------------------------------------|
| Buildings, roads, terrain   | DL_BaseCity              | Core map - always needed                       |
| Bonfire FX, rave lights     | DL_Events                | Event-specific, activate on demand             |
| Construction barricades     | DL_Construction          | Blocks paths, needs high priority activation   |
| Party decorations, crowds   | DL_Parties               | Cosmetic, loaded for celebrations              |
| Zombies, pedestrians        | DL_BaseCity              | Core gameplay, always active                   |
| Vehicles                    | DL_BaseCity              | Core gameplay, always active                   |

---

## Creating New Layers

If you need a new layer for a specific event type:

### Steps

1. **Open Unreal Editor**

2. **Navigate to Content Browser:**
   - Go to `Plugins/GSD_CityStreaming/Content/DataLayers/`

3. **Create the Data Layer:**
   - Right-click in Content Browser
   - Select **Miscellaneous → Data Layer**
   - Name it following convention: `DL_[Category]`
   - Examples: `DL_Holidays`, `DL_Weather`, `DL_NightMode`

4. **Set Layer Type (CRITICAL):**
   - Select the new Data Layer asset
   - In Details panel, find **Data Layer Type**
   - Set to **Runtime** (NOT Editor)
   - Runtime layers can be toggled at runtime; Editor-only layers cannot

5. **Add to Configuration:**
   - Open `DA_GSDDataLayerConfig` in `Plugins/GSD_CityStreaming/Content/Config/`
   - Add the new layer to `AllRuntimeLayers` array
   - If it's a core layer, add to appropriate slot (BaseCityLayer, EventsLayer, etc.)

### Naming Convention

- **DL_** prefix for all Data Layers
- **PascalCase** for category names
- **Examples:** `DL_BaseCity`, `DL_Events`, `DL_Construction`, `DL_Parties`

---

## Assigning Actors to Layers

### Steps

1. **Select Actor:**
   - In viewport or World Outliner, select the actor you want to assign

2. **Open Details Panel:**
   - If not visible, press **Details** button or Window → Details

3. **Find Data Layers Section:**
   - Scroll down to find **Data Layers** section
   - If collapsed, click the arrow to expand

4. **Add Layer:**
   - Click **+** button next to Data Layers array
   - Select the desired layer from dropdown (e.g., `DL_Events`)

5. **Multiple Layers:**
   - Actors can belong to multiple layers
   - Click **+** again to add additional layers
   - Actor activates when ANY of its layers are active

### Batch Assignment

To assign multiple actors to the same layer:

1. **Select multiple actors** in viewport or World Outliner
2. **Right-click → Edit → Add to Data Layer**
3. **Choose layer** from the popup

### Removing Actors from Layers

1. **Select actor**
2. **In Details panel → Data Layers section**
3. **Click X** next to the layer you want to remove

---

## Testing Layer Toggling

### Console Commands

Use these console commands (press **~** to open console):

| Command                                      | Description                                    |
|----------------------------------------------|------------------------------------------------|
| `DataLayer.List`                             | List all Data Layers and their states          |
| `DataLayer.SetState [Name] [State]`          | Toggle layer state                             |
| `wp.Runtime.DrawDataLayers 1`                | Visualize layer bounds in viewport             |
| `stat streaming`                             | Show streaming statistics                      |

#### Layer States

- **Unloaded** - Layer content is not loaded (lowest memory)
- **Loaded** - Layer content is loaded but not visible
- **Activated** - Layer content is loaded and visible (highest memory)

#### Examples

```bash
# Activate Events layer
DataLayer.SetState DL_Events Activated

# Deactivate Construction layer
DataLayer.SetState DL_Construction Unloaded

# List all layers
DataLayer.List
```

### Using GSDDataLayerManager in Blueprint

For runtime control, use the GSDDataLayerManager subsystem:

#### Toggle Single Layer

```blueprint
Get GSDDataLayerManager (Get World Subsystem node)
→ SetDataLayerState
   - LayerName: "DL_Events"
   - bActivated: true
```

#### Activate Multiple Layers (Staged)

```blueprint
Get GSDDataLayerManager
→ ActivateLayersStaged
   - LayerNames: ["DL_Events", "DL_Parties"]
   - MaxTimePerFrameMs: 5.0
```

#### Check Layer State

```blueprint
Get GSDDataLayerManager
→ IsDataLayerActivated
   - LayerName: "DL_Events"
→ Returns: true/false
```

#### Get All Runtime Layers

```blueprint
Get GSDDataLayerManager
→ GetRuntimeDataLayerNames
→ Returns: Array of Names
```

---

## Performance Best Practices

### CRITICAL: Avoid Activating All Layers at Once

**Problem:** Activating multiple layers simultaneously can cause 200-500ms hitches, exceeding the 33.33ms frame budget (30 FPS).

**Solution:** Use `ActivateLayersStaged()` for multiple layers.

#### Example: WRONG (Causes Hitches)

```blueprint
# DON'T DO THIS - causes massive hitch
SetDataLayerState "DL_Events", true
SetDataLayerState "DL_Construction", true
SetDataLayerState "DL_Parties", true
```

#### Example: CORRECT (Staged Activation)

```blueprint
# DO THIS - spreads activation across frames
ActivateLayersStaged ["DL_Events", "DL_Construction", "DL_Parties"]
```

### Frame Budget Settings

Default frame budget is **5ms per frame**. Adjust based on platform:

| Platform      | MaxActivationTimePerFrameMs | Notes                              |
|---------------|-----------------------------|------------------------------------|
| PC High-End   | 5.0                         | Default                            |
| PC Mid-Range  | 3.0                         | Reduce for smoother frame times    |
| Console       | 4.0                         | Stable hardware, predictable       |
| Mobile        | 2.0                         | Limited CPU, spread more           |

### Staged Activation Delay

Delay between layer activations (default **0.1 seconds**):

- **Higher delay** = smoother frame times, slower overall activation
- **Lower delay** = faster activation, risk of minor hitches
- **0.0** = activate immediately (only if single layer or powerful hardware)

### Layer Organization Tips

1. **Keep base content in DL_BaseCity** - Always loaded, never toggled
2. **Event layers activate on-demand** - Only load when event is active
3. **Use priority for critical layers** - High-priority layers activate first
4. **Minimize overlapping content** - Actors in multiple layers activate multiple times

### Memory Management

| Scenario                      | Recommended Approach                              |
|-------------------------------|---------------------------------------------------|
| Event not happening           | Keep layer **Unloaded** (not just deactivated)    |
| Event imminent (5-10 sec)     | Pre-load layer (Loaded state)                     |
| Event starting                | Activate layer (Activated state)                  |
| Event ended                   | Unload layer (Unloaded state)                     |

---

## Console Commands Reference

### Data Layer Commands

| Command                                      | Description                                    |
|----------------------------------------------|------------------------------------------------|
| `DataLayer.List`                             | List all Data Layers and their current states  |
| `DataLayer.SetState [Name] [State]`          | Set layer state (Unloaded/Loaded/Activated)    |
| `DataLayer.Toggle [Name]`                    | Toggle layer between Activated/Unloaded        |

### World Partition Commands

| Command                                      | Description                                    |
|----------------------------------------------|------------------------------------------------|
| `wp.Runtime.DrawDataLayers 1`                | Draw Data Layer bounds in viewport             |
| `wp.Runtime.DrawCells 1`                     | Draw streaming cells in viewport               |
| `stat streaming`                             | Show streaming statistics (FPS, memory, etc.)  |
| `stat unit`                                  | Show game, draw, and GPU times                 |

### Performance Commands

| Command                                      | Description                                    |
|----------------------------------------------|------------------------------------------------|
| `stat fps`                                   | Show FPS counter                               |
| `stat unitgraph`                             | Graph frame times over time                    |
| `memreport -full`                            | Generate full memory report                    |

---

## Scalability Settings

Different platforms may require different Data Layer settings:

### Low-End Platforms (Mobile, Low-Spec PC)

In `DA_GSDDataLayerConfig`:

```
MaxActivationTimePerFrameMs: 2.0
StagedActivationDelay: 0.2
bUseStagedActivation: true
```

**Rationale:** Slower hardware needs more time per frame, longer delays between activations.

### High-End Platforms (PC, Current-Gen Console)

```
MaxActivationTimePerFrameMs: 5.0
StagedActivationDelay: 0.1
bUseStagedActivation: true
```

**Rationale:** Default settings work well for capable hardware.

### Disable Telemetry (Optional)

For platforms where performance telemetry is not needed:

- Set `MaxRecentEvents` to 0 in GSDDataLayerConfig
- This disables layer state change event broadcasting
- Reduces overhead on low-end devices

---

## DataAsset Configuration

### DA_GSDDataLayerConfig

The `DA_GSDDataLayerConfig` DataAsset (located in `Plugins/GSD_CityStreaming/Content/Config/`) links Data Layers to the GSDDataLayerManager.

#### Properties

| Property                     | Description                                      | Default |
|------------------------------|--------------------------------------------------|---------|
| **BaseCityLayer**            | Always-on base content layer                     | DL_BaseCity |
| **EventsLayer**              | Dynamic event content layer                      | DL_Events |
| **ConstructionLayer**        | Construction zones layer                         | DL_Construction |
| **PartiesLayer**             | Block party content layer                        | DL_Parties |
| **AllRuntimeLayers**         | Array of all runtime layers                      | [All 4 layers] |
| **MaxActivationTimePerFrameMs** | Frame budget for staged activation (ms)       | 5.0     |
| **StagedActivationDelay**    | Delay between layer activations (seconds)        | 0.1     |
| **bUseStagedActivation**     | Enable staged activation (recommended)           | true    |

### Per-Map Configurations

You can create multiple DataAsset configs:

1. **Duplicate** `DA_GSDDataLayerConfig`
2. **Rename** to match map/game mode (e.g., `DA_GSDDataLayerConfig_NightMode`)
3. **Adjust** layer assignments and performance settings
4. **Assign** to GSDDataLayerManager at runtime

#### Switching Config at Runtime

```blueprint
Get GSDDataLayerManager
→ SetLayerConfig
   - InConfig: DA_GSDDataLayerConfig_NightMode
```

### Integration with GSDDataLayerManager

The GSDDataLayerManager subsystem uses the config for:

1. **Layer references** - No hardcoded layer names in C++
2. **Performance tuning** - Frame budget and delay settings
3. **Event system integration** - Phase 8/9 will use config for event layer activation

---

## Troubleshooting

### Layer Not Toggling

**Symptom:** `DataLayer.SetState` command doesn't change layer state

**Solutions:**
1. Check layer is **Runtime** type (not Editor-only)
2. Verify actor is assigned to the layer (Details panel → Data Layers)
3. Check console for error messages
4. Ensure World Partition is enabled in level settings

### Massive Frame Rate Drop

**Symptom:** FPS drops significantly when activating layers

**Solutions:**
1. Use `ActivateLayersStaged()` instead of individual `SetDataLayerState()` calls
2. Reduce `MaxActivationTimePerFrameMs` (e.g., from 5.0 to 2.0)
3. Increase `StagedActivationDelay` (e.g., from 0.1 to 0.2)
4. Pre-load layers (Loaded state) before activating

### Layer Content Not Visible

**Symptom:** Layer is activated but content not visible

**Solutions:**
1. Check streaming cells are loaded (`wp.Runtime.DrawCells 1`)
2. Verify actor's **Bounds** are correct
3. Check actor's **Min/Max LOD** settings
4. Ensure actor is not **culled** by distance

### Memory Not Released

**Symptom:** Memory stays high after deactivating layers

**Solutions:**
1. Set layer to **Unloaded** state (not just Deactivated)
2. Wait for garbage collection (automatic, may take 10-30 seconds)
3. Use `FlushLevelStreaming` console command to force unload
4. Check for actors that are **Always Loaded** (override Data Layer setting)

---

## Summary

**Key Takeaways:**

1. **Always use Runtime Data Layers** - Editor-only layers cannot be toggled at runtime
2. **Use staged activation** - `ActivateLayersStaged()` prevents hitches
3. **Respect frame budget** - Keep activation under 5ms per frame
4. **Assign content correctly** - Base content in DL_BaseCity, events in event layers
5. **Test with console commands** - `DataLayer.List`, `DataLayer.SetState`, `stat streaming`
6. **Configure per platform** - Adjust settings for low-end devices

**Next Steps:**

- Phase 3 Plan 03: Streaming Telemetry System (track layer activation metrics)
- Phase 8: Event System Core (automated layer activation for daily events)
- Phase 9: Event Implementations (concrete event layer activation)
