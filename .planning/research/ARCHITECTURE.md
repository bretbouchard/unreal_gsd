# Architecture Research

**Domain:** Unreal Engine 5 Open-World Game Infrastructure
**Researched:** 2026-02-24
**Confidence:** HIGH (Official Epic Documentation + Community Best Practices)

## Standard Architecture

### System Overview

```
+------------------------------------------------------------------+
|                        GAME RUNTIME                               |
|  +------------------------------------------------------------+  |
|  |                    GSD_Game (Project)                       |  |
|  |  - Game-specific logic                                      |  |
|  |  - Level Blueprints                                         |  |
|  |  - Game Instance / Game Mode                                |  |
|  +------------------------------------------------------------+  |
+------------------------------------------------------------------+
                                |
                                | Depends On
                                v
+------------------------------------------------------------------+
|                    GSD PLUGIN LAYER                               |
|  +------------+  +------------+  +------------+  +-------------+  |
|  | GSD_Crowds |  | GSD_Vehicle|  |GSD_DailyEvt|  |GSD_Telemetry|  |
|  +-----+------+  +-----+------+  +-----+------+  +------+------+  |
|        |              |               |                  |        |
|        +------+-------+-------+-------+------------------+        |
|               |               |                                       |
|               v               v                                       |
|  +----------------------------+  +--------------------------------+  |
|  |     GSD_CityStreaming      |  |     GSD_ValidationTools        |  |
|  |  - World Partition config  |  |  - Editor-only validation     |  |
|  |  - Streaming policies      |  |  - Data consistency checks     |  |
|  +-------------+--------------+  +--------------------------------+  |
|                |                                                    |
+----------------+----------------------------------------------------+
                 |
                 | Depends On
                 v
+------------------------------------------------------------------+
|                       GSD_Core                                    |
|  +------------------------------------------------------------+  |
|  |  - Common Interfaces (IGSDStreamable, IGSDSpawnable)       |  |
|  |  - Base Data Assets (GSDAssetDefinition, GSDConfigBase)    |  |
|  |  - Asset Manager Integration                               |  |
|  |  - Subsystem Base Classes (UGSDSubsystem)                  |  |
|  |  - Logging & Debug Utilities                               |  |
|  +------------------------------------------------------------+  |
+------------------------------------------------------------------+
                 |
                 | Depends On
                 v
+------------------------------------------------------------------+
|                    ENGINE LAYER                                   |
|  +------------+  +----------------+  +------------------------+   |
|  | Engine     |  | World Partition |  | Game Features System  |   |
|  | Core       |  | Module          |  | Module                |   |
|  +------------+  +----------------+  +------------------------+   |
+------------------------------------------------------------------+
```

### Component Responsibilities

| Component | Responsibility | Typical Implementation |
|-----------|----------------|------------------------|
| **GSD_Core** | Foundation interfaces, base classes, shared utilities | C++ module with abstract interfaces and UPrimaryDataAsset bases |
| **GSD_CityStreaming** | World Partition setup, streaming policies, tile management | C++ module + Editor widgets + Data Assets for streaming config |
| **GSD_Vehicles** | Vehicle spawning, physics, AI integration | C++ module + Data Assets for vehicle definitions + Blueprint library |
| **GSD_Crowds** | NPC spawning, crowd simulation, LOD management | C++ module + Mass Entity integration or custom system |
| **GSD_DailyEvents** | Time-based event system, scheduling | C++ module + Data Tables for event schedules |
| **GSD_Telemetry** | Performance monitoring, analytics hooks | C++ module with async HTTP/UDP sends |
| **GSD_ValidationTools** | Editor-time data validation, CI integration | Editor-only C++ module with commandlets |
| **GSD_Game** | Project-specific game logic, level orchestration | C++ module + Blueprint game logic |

## Recommended Project Structure

```
unreal_gsd/
+-- Plugins/
|   +-- GSD_Core/
|   |   +-- GSD_Core.uplugin
|   |   +-- Resources/
|   |   |   +-- Icon128.png
|   |   +-- Source/
|   |   |   +-- GSDCore/
|   |   |   |   +-- GSDCore.Build.cs
|   |   |   |   +-- Private/
|   |   |   |   |   +-- GSDCore.cpp
|   |   |   |   |   +-- DataAssets/
|   |   |   |   |   +-- Subsystems/
|   |   |   |   |   +-- Interfaces/
|   |   |   |   +-- Public/
|   |   |   |       +-- GSDCore.h
|   |   |   |       +-- DataAssets/
|   |   |   |       +-- Subsystems/
|   |   |   |       +-- Interfaces/
|   |   |   +-- GSDCoreEditor/          [Editor-only module]
|   |   |       +-- GSDCoreEditor.Build.cs
|   |   |       +-- Private/
|   |   |       +-- Public/
|   |   +-- Content/
|   |       +-- GSDCore/                 [Plugin content]
|   +-- GSD_CityStreaming/
|   |   +-- GSD_CityStreaming.uplugin
|   |   +-- Source/
|   |   |   +-- GSDCityStreaming/
|   |   |   +-- GSDCityStreamingEditor/
|   |   +-- Content/
|   +-- GSD_Vehicles/
|   +-- GSD_Crowds/
|   +-- GSD_DailyEvents/
|   +-- GSD_Telemetry/
|   +-- GSD_ValidationTools/
+-- Source/
|   +-- GSD_Game/                        [Project module]
|   +-- GSD_GameEditor/                  [Project editor module]
+-- Content/
|   +-- City/                            [Level content]
|   +-- Data/                            [Project data assets]
|   +-- Blueprints/
+-- Config/
    +-- DefaultGSD.ini                   [Custom config]
```

### Structure Rationale

- **Plugins/GSD_Core:** Must be standalone - no dependencies on other GSD plugins. Foundation layer.
- **Plugins/GSD_*/Editor modules:** Editor-only code isolated in separate modules to reduce cooked size.
- **Source/GSD_Game:** Project-specific logic that depends on plugins, never the reverse.
- **Content/Data/:** All data assets live here or in plugin Content/ - data-driven design central location.

## Architectural Patterns

### Pattern 1: Layered Plugin Architecture

**What:** Plugins organized in dependency layers. Lower layers provide interfaces; upper layers provide implementations.

**When to use:** Always for modular game infrastructure. Enables swapping implementations without breaking dependencies.

**Trade-offs:**
- (+) Clear dependency direction
- (+) Can replace entire subsystems
- (-) More upfront design required
- (-) Interface abstraction adds indirection

**Example:**
```cpp
// GSD_Core/Public/Interfaces/IGSDSpawnable.h
UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UGSDSpawnableInterface : public UInterface
{
    GENERATED_BODY()
};

class IGSDSpawnable
{
    GENERATED_BODY()
public:
    virtual FVector GetSpawnLocation() const = 0;
    virtual void OnSpawned(const FGSDSpawnContext& Context) = 0;
    virtual void OnDespawned() = 0;
};

// GSD_Vehicles/Private/GSDVehiclePawn.cpp
class AGSDVehiclePawn : public APawn, public IGSDSpawnable
{
    virtual FVector GetSpawnLocation() const override;
    virtual void OnSpawned(const FGSDSpawnContext& Context) override;
    virtual void OnDespawned() override;
};
```

### Pattern 2: Data-Driven Configuration

**What:** All configuration, definitions, and tuning data stored in UPrimaryDataAsset derivatives. Code reads data; does not hardcode values.

**When to use:** All game systems that need iteration, balancing, or artist/designer control.

**Trade-offs:**
- (+) Non-programmers can tune systems
- (+) Hot-reload during PIE
- (+) Asset references managed automatically
- (-) Asset loading complexity

**Example:**
```cpp
// GSD_Core/Public/DataAssets/GSDVehicleDefinition.h
UCLASS(BlueprintType)
class GSDCORE_API UGSDVehicleDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vehicle")
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vehicle")
    TSoftObjectPtr<UStaticMesh> VehicleMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vehicle")
    float MaxSpeed = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vehicle")
    float Mass = 1500.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vehicle")
    TArray<TSoftObjectPtr<UMaterialInterface>> MaterialVariants;

    // Asset bundle for async loading
    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};

// Usage at runtime
UGSDVehicleDefinition* VehicleDef = AssetManager->LoadAsset<UGSDVehicleDefinition>(AssetId);
```

### Pattern 3: World Partition Integration

**What:** Leverage UE5's World Partition for automatic streaming. Content organized spatially; engine handles loading/unloading.

**When to use:** All open-world content. Mandatory for large streaming worlds.

**Trade-offs:**
- (+) Automatic streaming based on distance
- (+) Memory efficient - only loaded what's needed
- (+) Built-in HLOD support
- (-) Requires World Partition setup
- (-) One File Per Actor workflow change

**Example:**
```cpp
// GSD_CityStreaming configuration via Data Asset
UCLASS(BlueprintType)
class GSDCITYSTREAMING_API UGSDStreamingConfig : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    // Cell size in world units (default 12800 = 128m)
    UPROPERTY(EditDefaultsOnly, Category = "World Partition")
    int32 CellSize = 12800;

    // Loading range around streaming sources
    UPROPERTY(EditDefaultsOnly, Category = "World Partition")
    float LoadingRange = 25600.0f;

    // Block on load for critical areas
    UPROPERTY(EditDefaultsOnly, Category = "World Partition")
    TArray<FBox> BlockingLoadVolumes;

    // HLOD layers configuration
    UPROPERTY(EditDefaultsOnly, Category = "HLOD")
    TArray<FGSDHLODLayerConfig> HLODLayers;
};
```

### Pattern 4: Subsystem-Based Systems

**What:** Game systems implemented as UGameInstanceSubsystem or UWorldSubsystem. Auto-lifecycle, global access.

**When to use:** Singleton-like systems (vehicle pool, crowd manager, telemetry).

**Trade-offs:**
- (+) Automatic lifecycle management
- (+) Clean access via GetSubsystem<T>()
- (+) Supports PIE properly
- (-) Must handle level transitions carefully

**Example:**
```cpp
// GSD_Vehicles/Public/Subsystems/GSDVehicleSubsystem.h
UCLASS()
class GSDVEHICLES_API UGSDVehicleSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Spawn vehicle from definition
    AGSDVehiclePawn* SpawnVehicle(const FPrimaryAssetId& VehicleId, const FTransform& Transform);

    // Return vehicle to pool
    void ReleaseVehicle(AGSDVehiclePawn* Vehicle);

    // Get all active vehicles in streaming range
    TArray<AGSDVehiclePawn*> GetActiveVehicles() const;

private:
    UPROPERTY()
    TArray<TObjectPtr<AGSDVehiclePawn>> ActiveVehicles;

    UPROPERTY()
    TArray<TObjectPtr<AGSDVehiclePawn>> VehiclePool;
};

// Usage
UGSDVehicleSubsystem* VehicleSys = GetGameInstance()->GetSubsystem<UGSDVehicleSubsystem>();
AGSDVehiclePawn* Car = VehicleSys->SpawnVehicle(VehicleDefId, SpawnTransform);
```

## Data Flow

### Asset Pipeline Flow

```
[Blender]                            [Unreal Editor]                    [Runtime]
+-----------+                        +------------------+              +----------+
| City Tile |  --(USD/FBX)-->        | GSD_Validation   |              | Game    |
| Modeling  |                        | Tools Import     |              | Instance|
+-----------+                        +------------------+              +----------+
     |                                       |                              |
     v                                       v                              v
+-----------+                        +------------------+              +----------+
| Material  |  --(USD/FBX)-->        | Content Browser  |              | World    |
| Export    |                        | Data Assets      |              | Partition|
+-----------+                        +------------------+              +----------+
                                            |                              |
                                            v                              v
                                     +------------------+              +----------+
                                     | GSD Data Asset   |              | Streaming|
                                     | Definitions      |              | Sources  |
                                     +------------------+              +----------+
                                            |                              |
                                            +-------------+----------------+
                                                          |
                                                          v
                                                   +----------+
                                                   | Loaded   |
                                                   | Assets   |
                                                   +----------+
```

### Request Flow

```
[Player Enters Area]
       |
       v
[World Partition] --> [Cell Loaded Event]
       |
       v
[GSD_CityStreaming] --> [OnCellLoaded Delegate]
       |
       +---> [GSD_Vehicles] --> Spawn vehicles from pool based on cell data
       |
       +---> [GSD_Crowds] --> Spawn NPCs based on crowd density data
       |
       +---> [GSD_DailyEvents] --> Activate events for current time
       |
       v
[Game Runtime] --> [Player interacts with content]
```

### State Management

```
[GameInstance]
    |
    +-- [UGSDVehicleSubsystem]      <-- Manages vehicle pool
    |        |
    |        +-- ActiveVehicles[]
    |        +-- VehiclePool[]
    |
    +-- [UGSDTelemetrySubsystem]    <-- Async telemetry queue
    |        |
    |        +-- EventQueue
    |        +-- HTTPClient
    |
    +-- [UGSDEventSubsystem]        <-- Daily event scheduling
             |
             +-- ActiveEvents[]
             +-- ScheduledEvents[]

[World]
    |
    +-- [AGSDWorldSettings]         <-- Custom world settings
             |
             +-- StreamingConfig (Data Asset ref)
             +-- CrowdDensityConfig (Data Asset ref)
```

### Key Data Flows

1. **Import Flow:** Blender exports USD/FBX tiles to watched folder -> ValidationTools imports and validates -> Creates Data Assets for tile metadata -> World Partition references tile actors.

2. **Spawn Flow:** Player enters streaming cell -> World Partition loads cell -> GSD_CityStreaming receives callback -> Queries Data Assets for spawn data -> Requests spawns from Vehicle/Crowd subsystems -> Actors spawned from pools.

3. **Telemetry Flow:** Game events occur -> GSD_Telemetry queues event -> Background thread batches events -> Periodic HTTP send to analytics endpoint.

4. **Event Flow:** Game time advances -> GSD_DailyEvents checks Data Tables for scheduled events -> Activates matching events -> Notifies relevant subsystems.

## Plugin Dependency Graph

```
                    BUILD ORDER (Bottom to Top)
                    ===========================

Layer 0 (Foundation):
+--------------------------------------------------+
|                   GSD_Core                       |
|  - No GSD dependencies                           |
|  - Depends: Core, CoreUObject, Engine            |
+--------------------------------------------------+
                        |
                        | All plugins depend on Core
                        v
Layer 1 (Infrastructure):
+--------------------------------------------------+
|              GSD_CityStreaming                   |
|  - Depends: GSD_Core                             |
|  - Engine: WorldPartition, Landscape             |
+--------------------------------------------------+
+--------------------------------------------------+
|              GSD_ValidationTools                 |
|  - Depends: GSD_Core                             |
|  - Editor-only, no runtime dependencies          |
+--------------------------------------------------+
                        |
                        | Systems depend on streaming
                        v
Layer 2 (Systems):
+--------------------------------------------------+
|              GSD_Vehicles                        |
|  - Depends: GSD_Core, GSD_CityStreaming          |
|  - Engine: PhysicsCore, ChaosVehicles            |
+--------------------------------------------------+
+--------------------------------------------------+
|              GSD_Crowds                          |
|  - Depends: GSD_Core, GSD_CityStreaming          |
|  - Engine: MassEntity (optional), Navigation     |
+--------------------------------------------------+
+--------------------------------------------------+
|              GSD_DailyEvents                     |
|  - Depends: GSD_Core                             |
|  - Can trigger events in Vehicles/Crowds         |
+--------------------------------------------------+
+--------------------------------------------------+
|              GSD_Telemetry                       |
|  - Depends: GSD_Core                             |
|  - Observes all systems, no dependencies up      |
+--------------------------------------------------+
                        |
                        | Game depends on systems
                        v
Layer 3 (Project):
+--------------------------------------------------+
|                  GSD_Game                        |
|  - Depends: All GSD plugins                      |
|  - Project-specific implementation               |
+--------------------------------------------------+
```

### Build.cs Configuration

```cpp
// GSD_Core.Build.cs
PublicDependencyModuleNames.AddRange(new string[] {
    "Core",
    "CoreUObject",
    "Engine"
});

// GSD_CityStreaming.Build.cs
PublicDependencyModuleNames.AddRange(new string[] {
    "Core",
    "CoreUObject",
    "Engine",
    "WorldPartition",      // World Partition support
    "Landscape"            // Terrain support
});
PrivateDependencyModuleNames.AddRange(new string[] {
    "GSDCore"              // Core plugin dependency
});

// GSD_Vehicles.Build.cs
PublicDependencyModuleNames.AddRange(new string[] {
    "Core",
    "CoreUObject",
    "Engine",
    "PhysicsCore",
    "ChaosVehicles"        // Vehicle physics
});
PrivateDependencyModuleNames.AddRange(new string[] {
    "GSDCore",
    "GSDCityStreaming"     // For spawning integration
});

// GSD_Telemetry.Build.cs
PublicDependencyModuleNames.AddRange(new string[] {
    "Core",
    "CoreUObject",
    "Engine",
    "HTTP",                // HTTP client
    "Json"                 // JSON serialization
});
PrivateDependencyModuleNames.AddRange(new string[] {
    "GSDCore"
});
// Note: Telemetry should NOT depend on Vehicles/Crowds - use interfaces

// GSD_Game.Build.cs
PublicDependencyModuleNames.AddRange(new string[] {
    "Core",
    "CoreUObject",
    "Engine"
});
PrivateDependencyModuleNames.AddRange(new string[] {
    "GSDCore",
    "GSDCityStreaming",
    "GSDVehicles",
    "GSDCrowds",
    "GSDDailyEvents",
    "GSDTelemetry"
});
```

### Module Loading Phase Configuration

```cpp
// In .uplugin files, control loading order:

// GSD_Core.uplugin
{
    "Modules": [
        {
            "Name": "GSDCore",
            "Type": "Runtime",
            "LoadingPhase": "Default"  // Standard loading
        },
        {
            "Name": "GSDCoreEditor",
            "Type": "Editor",
            "LoadingPhase": "Default"
        }
    ]
}

// GSD_CityStreaming.uplugin
{
    "Modules": [
        {
            "Name": "GSDCityStreaming",
            "Type": "Runtime",
            "LoadingPhase": "Default"
        }
    ]
}

// GSD_ValidationTools.uplugin
{
    "Modules": [
        {
            "Name": "GSDValidationTools",
            "Type": "Editor",
            "LoadingPhase": "PostEngineInit"  // After engine ready
        }
    ]
}
```

## Scaling Considerations

| Scale | Architecture Adjustments |
|-------|--------------------------|
| 0-1k users (Development) | Single map testing, all plugins loaded, full validation in editor |
| 1k-10k users (Alpha) | World Partition enabled, HLOD generated, telemetry sampling enabled |
| 10k-100k users (Beta) | Streaming tuned, pool sizes optimized, crowd LOD configured |
| 100k+ users (Live) | Full production: aggressive streaming ranges, HLOD tier 2+, telemetry sampling rate reduced |

### Scaling Priorities

1. **First bottleneck:** Memory from loaded content
   - Fix: World Partition cell sizes, streaming ranges, One File Per Actor
2. **Second bottleneck:** Spawn/despawn performance
   - Fix: Object pooling in subsystems, async asset loading
3. **Third bottleneck:** Draw calls from distant content
   - Fix: HLOD generation, imposters, crowd LOD

## Anti-Patterns

### Anti-Pattern 1: Circular Plugin Dependencies

**What people do:** Plugin A depends on B, B depends on A. Or both depend on a shared project module.

**Why it's wrong:** UE5 plugin loader cannot resolve circular dependencies. Build fails or runtime crashes.

**Do this instead:**
- Extract shared interfaces to GSD_Core
- Use event delegates for cross-plugin communication
- Use soft references for optional dependencies

```
WRONG:
GSD_Vehicles <--depends-- GSD_Crowds
GSD_Crowds <--depends-- GSD_Vehicles

CORRECT:
GSD_Core (interfaces)
    ^
    +-- GSD_Vehicles (implements interfaces)
    +-- GSD_Crowds (implements interfaces)
    +-- Events for cross-plugin communication
```

### Anti-Pattern 2: Hardcoded Spawn Data in Code

**What people do:** Vehicle spawn locations, crowd densities, event times hardcoded in C++ or Blueprints.

**Why it's wrong:** Designers cannot iterate. Requires recompile for tuning. Cannot vary by level/region.

**Do this instead:**
- All configuration in Data Assets
- Spawn locations derived from actors placed in level or procedural rules
- Densities/tuning in Data Tables for region-based variation

### Anti-Pattern 3: Monolithic World Partition Cells

**What people do:** Single large cell or very large cell sizes for "simplicity."

**Why it's wrong:** Defeats streaming purpose. Loads content player cannot see. Memory waste.

**Do this instead:**
- Cell size matched to gameplay visibility (128m-256m typical)
- Use Data Layers for content that loads conditionally
- HLOD for distant content visibility

### Anti-Pattern 4: Asset References in Blueprints

**What people do:** Hard asset references in Blueprint variables (MyVehicleMesh, MyMaterial).

**Why it's wrong:** All referenced assets load with the Blueprint. No async loading. Memory spike.

**Do this instead:**
- Use TSoftObjectPtr for lazy loading
- Use Data Assets with soft references
- Load via Asset Manager for explicit control

### Anti-Pattern 5: Cross-Plugin Direct Access

**What people do:** GSD_Vehicles directly calls GSD_Crowds functions, tight coupling.

**Why it's wrong:** Cannot use plugins independently. Testing harder. Circular dependency risk.

**Do this instead:**
- Use interfaces from GSD_Core
- Use event delegates for loose coupling
- Telemetry observes via interfaces, does not depend on implementations

## Integration Points

### External Services

| Service | Integration Pattern | Notes |
|---------|---------------------|-------|
| Blender (blender_gsd) | USD/FBX file export to watched folder | ValidationTools auto-imports on change |
| Analytics | GSD_Telemetry HTTP POST | Async, batched, fire-and-forget |
| CI/CD | GSD_ValidationTools commandlets | Exit codes for pass/fail |

### Internal Boundaries

| Boundary | Communication | Notes |
|----------|---------------|-------|
| GSD_Core <-> GSD_CityStreaming | Direct dependency | Streaming uses Core interfaces |
| GSD_CityStreaming <-> GSD_Vehicles | Event delegates | Cell loaded -> spawn vehicles |
| GSD_CityStreaming <-> GSD_Crowds | Event delegates | Cell loaded -> spawn crowds |
| GSD_Vehicles <-> GSD_Crowds | None (decoupled) | Communicate via Core events if needed |
| All Plugins <-> GSD_Telemetry | Interface + events | Telemetry observes, does not control |
| All Plugins <-> GSD_Game | Downward dependency only | Game consumes plugins, never reverse |

### Streaming Source Integration

```cpp
// Player controller registers as streaming source
UCLASS()
class AGSDPlayerController : public APlayerController
{
    // Automatically tracked by World Partition
    // No explicit registration needed for player
};

// Vehicles/crowds can also be streaming sources
UCLASS()
class AGSDVehiclePawn : public APawn, public IWorldPartitionStreamingSourceProvider
{
    // Implement to make vehicle a streaming source
    // Useful for AI-driven vehicles that need to load content ahead
};
```

## Sources

- Epic Games Official Documentation: Plugins (https://dev.epicgames.com/documentation/en-us/unreal-engine/plugins-in-unreal-engine)
- Epic Games Official Documentation: World Partition (https://dev.epicgames.com/documentation/en-us/unreal-engine/world-partition-in-unreal-engine)
- Epic Games Official Documentation: Data Assets (https://dev.epicgames.com/documentation/en-us/unreal-engine/data-assets-in-unreal-engine)
- Epic Games Official Documentation: Game Features (https://dev.epicgames.com/documentation/en-us/unreal-engine/game-features-in-unreal-engine)
- Epic Games Official Documentation: Module Loading Phases (https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-modules)
- Unreal Engine Community Wiki: Plugin Architecture Best Practices
- Unreal Slackers Discord: Accumulated community knowledge on modular architecture

---
*Architecture research for: Unreal Engine 5 Open-World Game Infrastructure*
*Researched: 2026-02-24*
