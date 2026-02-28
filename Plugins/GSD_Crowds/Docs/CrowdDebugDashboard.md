# Crowd Debug Dashboard

Live metrics dashboard for monitoring crowd system performance and behavior.

## Overview

The Crowd Debug Dashboard is a UMG widget that displays real-time crowd system metrics:
- Entity counts (total entities, active crowds)
- LOD distribution (visual bars for each LOD level)
- Performance metrics (frame time, draw calls, memory usage)
- Color-coded performance warnings

## Usage

### 1. Create Widget Blueprint

1. Right-click in Content Browser
2. Select **User Interface > Widget Blueprint**
3. Name it `WBP_CrowdDebugDashboard`
4. Open the Graph view
5. Reparent to `UGSDCrowdDebugWidget` (GSD_Crowds plugin)
   - Class Settings > Parent Class > GSDCrowdDebugWidget

### 2. Add Named Widgets

In the Designer view, add these named widgets (must match exactly):

#### Entity Counts
- **TotalEntitiesText** (Text Block) - Displays total crowd entities
- **ActiveCrowdsText** (Text Block) - Displays number of active crowds

#### LOD Distribution
- **LOD0Bar** (Progress Bar) - Close-up entities (0-100%)
- **LOD1Bar** (Progress Bar) - Medium distance entities (0-100%)
- **LOD2Bar** (Progress Bar) - Far entities (0-100%)
- **LOD3Bar** (Progress Bar) - Culled/distant entities (0-100%)

#### Performance Metrics
- **FrameTimeText** (Text Block) - Last frame time in milliseconds
- **PerformanceBar** (Progress Bar) - Color-coded performance indicator
  - Green: <16.67ms (60+ FPS)
  - Yellow: 16.67-25ms (40-60 FPS)
  - Red: >25ms (<40 FPS)
- **MemoryText** (Text Block) - Memory usage in MB
- **WarningsBox** (Vertical Box) - Shown when performance issues detected

### 3. Spawn Widget in Game

#### Blueprint Example:
```blueprint
Event BeginPlay
  → Create Widget (WBP_CrowdDebugDashboard)
  → Add to Viewport
```

#### C++ Example:
```cpp
// In player controller or HUD
void AMyHUD::BeginPlay()
{
    Super::BeginPlay();

    if (UGSDCrowdDebugWidget* DebugWidget = CreateWidget<UGSDCrowdDebugWidget>(GetWorld(), DebugWidgetClass))
    {
        DebugWidget->AddToViewport();
        // Widget auto-binds to crowd manager in NativeConstruct()
    }
}
```

### 4. Toggle Dashboard (Optional)

To toggle the dashboard at runtime:

```cpp
// Show dashboard
if (DebugWidget)
{
    DebugWidget->AddToViewport();
}

// Hide dashboard
if (DebugWidget)
{
    DebugWidget->RemoveFromParent();
}
```

## Metrics Reference

| Metric | Type | Description | Update Rate |
|--------|------|-------------|-------------|
| TotalEntities | int32 | Total spawned crowd entities | 10 Hz |
| ActiveCrowds | int32 | Number of active crowd groups | 10 Hz |
| LOD0Count | int32 | Entities at LOD 0 (closest) | 10 Hz |
| LOD1Count | int32 | Entities at LOD 1 (medium) | 10 Hz |
| LOD2Count | int32 | Entities at LOD 2 (far) | 10 Hz |
| LOD3Count | int32 | Entities at LOD 3 (culled) | 10 Hz |
| LastFrameTime | float | Last frame time in seconds | 10 Hz |
| AverageFrameTime | float | 60-frame average in seconds | 10 Hz |
| DrawCalls | int32 | Estimated draw calls | 10 Hz |
| MemoryUsedMB | float | Estimated memory in MB | 10 Hz |

## Performance Notes

### Update Frequency
- Metrics broadcast at **10 Hz** (0.1s interval)
- Uses timer-based updates, NOT tick
- Efficient for debugging without performance impact

### Memory Overhead
- ~2KB per entity for transforms and fragments
- Dashboard widget has minimal overhead (<1KB)
- Frame time history: 60 frames (~240 bytes)

### Performance Warnings
The dashboard automatically shows warnings when:
- Frame time exceeds 25ms (<40 FPS)
- Memory usage is abnormally high
- LOD distribution is imbalanced

### Best Practices
1. **Only spawn in development builds** - Use `#if !UE_BUILD_SHIPPING`
2. **Remove when not debugging** - Call `RemoveFromParent()` to stop updates
3. **Position carefully** - Use viewport anchoring for corner placement
4. **Avoid overlap** - Keep dashboard away from gameplay UI

## Troubleshooting

### Widget Not Updating
- Check crowd manager exists: `GetWorld()->GetSubsystem<UGSDCrowdManagerSubsystem>()`
- Verify binding: `BindToCrowdManager()` called in `NativeConstruct()`
- Check metrics broadcasting: `StartMetricsUpdates()` called

### Metrics All Zero
- Ensure crowds are spawned: `SpawnEntities()`
- Check entity tracking: `GetActiveEntityCount()`
- Verify Mass Entity subsystem initialized

### Performance Impact
- Dashboard updates at 10 Hz (configurable in subsystem)
- Reduce update frequency if needed: Change `MetricsUpdateInterval` in header
- Consider disabling LOD bars if not needed

## Blueprint Setup Guide

### Step-by-Step Blueprint Creation

1. **Create Widget Blueprint**
   - Content Browser > Add > User Interface > Widget Blueprint
   - Name: `WBP_CrowdDebugDashboard`

2. **Set Parent Class**
   - Open WBP_CrowdDebugDashboard
   - Graph View > Class Settings
   - Parent Class > Search "GSDCrowdDebugWidget"
   - Select and compile

3. **Add Layout (Designer View)**
   - Vertical Box (root)
     - Horizontal Box (Entity Counts)
       - Text Block "Entities:"
       - Text Block (Name: TotalEntitiesText)
       - Text Block "Crowds:"
       - Text Block (Name: ActiveCrowdsText)
     - Vertical Box (LOD Distribution)
       - Text Block "LOD Distribution:"
       - Progress Bar (Name: LOD0Bar)
       - Progress Bar (Name: LOD1Bar)
       - Progress Bar (Name: LOD2Bar)
       - Progress Bar (Name: LOD3Bar)
     - Vertical Box (Performance)
       - Text Block "Frame Time:"
       - Text Block (Name: FrameTimeText)
       - Progress Bar (Name: PerformanceBar)
       - Text Block (Name: MemoryText)
     - Vertical Box (Name: WarningsBox, Visibility: Collapsed)
       - Text Block "Performance Warning!"

4. **Style Progress Bars**
   - LOD Bars: Set Fill Color to blue/cyan
   - Performance Bar: Set Fill Color to green (will be overridden by code)
   - Set Percent to 0.0 in designer (will be updated at runtime)

5. **Compile and Save**

## Named Widgets Reference

| Widget Name | Type | Required | Description |
|------------|------|----------|-------------|
| TotalEntitiesText | UTextBlock | Yes | Displays formatted entity count |
| ActiveCrowdsText | UTextBlock | Yes | Displays active crowd count |
| LOD0Bar | UProgressBar | Yes | LOD 0 distribution (0-100%) |
| LOD1Bar | UProgressBar | Yes | LOD 1 distribution (0-100%) |
| LOD2Bar | UProgressBar | Yes | LOD 2 distribution (0-100%) |
| LOD3Bar | UProgressBar | Yes | LOD 3 distribution (0-100%) |
| FrameTimeText | UTextBlock | Yes | Frame time in milliseconds |
| PerformanceBar | UProgressBar | Yes | Color-coded performance indicator |
| MemoryText | UTextBlock | Yes | Memory usage in MB |
| WarningsBox | UVerticalBox | Yes | Container for performance warnings |

**Note:** Widget names must match exactly (case-sensitive).
