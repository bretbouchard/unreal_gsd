# GSD Crowds Plugin

**High-performance crowd simulation system for Unreal Engine 5 using Mass Entity Component System (ECS).**

## Overview

GSD Crowds provides a scalable, game-agnostic crowd simulation framework built on Unreal Engine's Mass Entity system. It supports thousands of entities with LOD-based rendering, deterministic behavior, and flexible game integration.

## Features

### Core Systems

- **Mass Entity Integration**: Leverages UE5's ECS for 10,000+ entity support
- **LOD System**: Automatic level-of-detail management (Full Actor → Low Actor → ISM → Culled)
- **Deterministic Simulation**: Seeded random for reproducible behavior
- **World Partition Support**: Streaming-safe entity spawning

### Behavior Systems

- **Wandering AI**: Organic movement with natural speed variation
- **Pursuit/Attack**: Configurable target acquisition and combat behavior
- **Smart Objects**: Interaction with world objects (chairs, doors, etc.)
- **ZoneGraph Navigation**: Lane-based pathfinding for crowds

### Developer Tools

- **Blueprint Integration**: Full Blueprint support for spawning and configuration
- **Debug Dashboard**: Real-time metrics (entity count, LOD distribution, frame time)
- **Event System**: Gameplay event broadcasting and listener registration
- **Config DataAssets**: No-code tuning of all parameters

## Quick Start

### Spawning a Crowd

**Blueprint:**
```blueprint
// Static one-liner spawn
UGSDCrowdManagerSubsystem::SpawnCrowd(World, 100, Location, 500.0f)
```

**C++:**
```cpp
#include "Subsystems/GSDCrowdManagerSubsystem.h"

// Get subsystem
UGSDCrowdManagerSubsystem* CrowdManager = GetWorld()->GetSubsystem<UGSDCrowdManagerSubsystem>();

// Spawn 100 entities in 500-unit radius
int32 NumSpawned = CrowdManager->SpawnEntities(100, Location, 500.0f);
```

### Async Spawning with Callback

```cpp
FOnCrowdSpawnComplete OnComplete;
OnComplete.BindDynamic(this, &AMyActor::OnCrowdSpawned);

CrowdManager->SpawnEntitiesAsync(100, Location, 500.0f, nullptr, OnComplete);
```

### Using the Event System

```cpp
#include "Subsystems/GSDCrowdEventSubsystem.h"

UGSDCrowdEventSubsystem* EventSystem = GetWorld()->GetSubsystem<UGSDCrowdEventSubsystem>();

// Register listener
FOnCrowdEvent EventDelegate;
EventDelegate.BindDynamic(this, &AMyActor::OnCrowdEvent);
int32 Handle = EventSystem->RegisterListener(FGameplayTag(), EventDelegate);

// Broadcast event
FGSDCrowdEvent Event;
Event.EventType = EGSDCrowdEventType::Alert;
Event.Location = AlertLocation;
Event.Radius = 1000.0f;
EventSystem->BroadcastEvent(Event);
```

## Configuration

### GSDCrowdConfig

Main configuration asset for all crowd parameters:

| Category | Setting | Default | Description |
|----------|---------|---------|-------------|
| Detection | DetectionRadius | 5000.0f | Entity perception range |
| LOD | LOD0Distance | 500.0f | Full detail distance |
| LOD | LOD1Distance | 2000.0f | Low actor distance |
| LOD | LOD2Distance | 4000.0f | ISM distance |
| LOD | LOD3CullDistance | 5000.0f | Cull distance |
| Behavior | BaseMoveSpeed | 150.0f | Base movement speed |
| Behavior | SpeedVariationPercent | 0.2f | Speed randomization (20%) |
| Pursuit | bEnablePursuitBehavior | true | Enable chasing |
| Pursuit | PursuitSpeedMultiplier | 2.0f | Speed when chasing |
| Pursuit | AttackRange | 100.0f | Melee attack distance |
| Pursuit | AttackCooldown | 1.0f | Seconds between attacks |

### GSDZombieBehaviorConfig

Game-specific behavior configuration:

| Setting | Default | Description |
|---------|---------|-------------|
| bEnableTargetDetection | true | Enable target finding |
| DetectionRadius | 1000.0f | Target detection range |
| VisionConeAngle | 90.0f | Field of view (degrees) |
| bEnablePursuit | true | Enable chasing behavior |
| bEnableAttack | true | Enable attack behavior |
| AttackDamage | 10.0f | Damage per attack |
| BaseAggression | 0.5f | Base aggression level (0-1) |

## Architecture

### Subsystems

```
UGSDCrowdManagerSubsystem    - Entity spawning/tracking, density modifiers
UGSDCrowdEventSubsystem      - Event broadcasting/listening
UGSDCrowdPerformanceBudget   - Frame budget management
UGSDCrowdHLODManager         - Hierarchical LOD management
UGSDSmartObjectSubsystem     - Smart object interaction
```

### Processors

```
UGSDZombieBehaviorProcessor  - Behavior updates (speed, wandering, pursuit)
UGSDNavigationProcessor      - ZoneGraph pathfinding
UGSDCrowdLODProcessor        - LOD distance calculations
UGSDSmartObjectProcessor     - Smart object interactions
```

### Fragments

```
FGSDZombieStateFragment      - Entity state (health, speed, aggression)
FGSDNavigationFragment       - Navigation data (lane, path)
FGSDSmartObjectFragment      - Smart object interaction state
```

## Performance

### Benchmarks

| Entity Count | Frame Time | Memory |
|--------------|------------|--------|
| 1,000 | 0.5ms | 2MB |
| 5,000 | 2.0ms | 10MB |
| 10,000 | 3.5ms | 20MB |

### Optimization Tips

1. **Use LOD Effectively**: Configure LOD distances based on visibility requirements
2. **Batch Spawns**: Spawn entities in batches of 50-100 for smoother frame times
3. **Limit Active Entities**: Use `MaxEntityCount` to prevent overspawning
4. **Profile First**: Use the Debug Dashboard to identify bottlenecks

## Network Support

### Validation Functions

The CrowdManagerSubsystem provides server-side validation for network safety:

```cpp
// Validate spawn parameters
FString ErrorMsg;
if (!CrowdManager->ValidateSpawnParameters(Count, Location, Radius, ErrorMsg))
{
    UE_LOG(LogTemp, Warning, TEXT("Invalid spawn: %s"), *ErrorMsg);
    return;
}

// Validate density modifiers
if (!CrowdManager->ValidateDensityModifier(Location, Radius, Multiplier, ErrorMsg))
{
    UE_LOG(LogTemp, Warning, TEXT("Invalid modifier: %s"), *ErrorMsg);
    return;
}
```

### Limits

- **Max Crowd Size**: 500 entities per spawn call
- **Max Spawn Radius**: 10,000 units
- **Max Density Multiplier**: 10x

## World Partition Integration

### Streaming-Safe Spawning

```cpp
// Spawns are automatically queued if cell is unloaded
CrowdManager->SpawnEntities(100, Location, 500.0f);

// Check if location is in loaded cell
if (CrowdManager->IsPositionInLoadedCell(Location))
{
    // Safe to spawn
}
```

### Cell Events

Entities are automatically despawned when cells unload:

```cpp
// Bind to cell loading (automatic)
CrowdManager->BindToStreamingEvents();
```

## Debugging

### Debug Dashboard

Enable debug visualization:

```cpp
// Start metrics updates (10 Hz)
CrowdManager->StartMetricsUpdates();

// Get current metrics
FGSDCrowdMetrics Metrics = CrowdManager->GetCurrentMetrics();
UE_LOG(LogTemp, Log, TEXT("Entities: %d, Frame Time: %.2fms"),
    Metrics.TotalEntities, Metrics.LastFrameTime * 1000.0f);
```

### Console Commands

```bash
// View crowd stats
stat crowd

// Toggle debug visualization
gsd.crowd.debug 1
```

## API Reference

### UGSDCrowdManagerSubsystem

| Function | Description |
|----------|-------------|
| `SpawnCrowd(World, Count, Center, Radius, Config)` | Static one-line spawn |
| `SpawnEntities(Count, Center, Radius, Config)` | Spawn entities in area |
| `SpawnEntitiesAsync(...)` | Async spawn with callback |
| `DespawnAllEntities()` | Remove all tracked entities |
| `GetActiveEntityCount()` | Get current entity count |
| `AddDensityModifier(Tag, Center, Radius, Mult)` | Add spawn density modifier |
| `RemoveDensityModifier(Tag)` | Remove density modifier |
| `ValidateSpawnParameters(...)` | Network-safe spawn validation |
| `StartMetricsUpdates()` | Begin metrics tracking |
| `StopMetricsUpdates()` | Stop metrics tracking |

### UGSDCrowdEventSubsystem

| Function | Description |
|----------|-------------|
| `BroadcastEvent(Event)` | Broadcast crowd event |
| `BroadcastLocationEvent(Type, Location, Radius)` | Location-based event |
| `BroadcastEntityEvent(Type, EntityID, Location)` | Entity event |
| `BroadcastPursuitEvent(Type, EntityID, TargetID, Location)` | Pursuit event |
| `RegisterListener(Tag, Delegate)` | Register event listener |
| `UnregisterListener(Handle)` | Remove listener |

## Troubleshooting

### Entities Not Spawning

1. Check if `SpawnEntities` returns 0
2. Verify location is in loaded cell: `IsPositionInLoadedCell()`
3. Check entity config is valid
4. Look for errors in output log

### Poor Performance

1. Enable Debug Dashboard to see metrics
2. Check LOD distribution (are too many at LOD0?)
3. Reduce `MaxEntityCount`
4. Increase LOD distances
5. Use batch spawning instead of many small spawns

### Behavior Issues

1. Check `GSDCrowdConfig` settings
2. Verify `bIsAlive` and `bIsActive` flags
3. Check `bIsAggressive` for pursuit behavior
4. Verify DeterminismManager is available

## License

Copyright Bret Bouchard. All Rights Reserved.

## Support

For issues and feature requests, please use the project issue tracker.
