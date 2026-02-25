# Stack Research

**Domain:** Unreal Engine 5.4+ Open-World Game Infrastructure
**Researched:** 2026-02-24
**Confidence:** HIGH (official Epic documentation verified)

## Recommended Stack

### Core Technologies

| Technology | Version | Purpose | Why Recommended |
|------------|---------|---------|-----------------|
| **Unreal Engine** | 5.4+ (5.5 recommended) | Core engine | World Partition stable, Mass Entity production-ready, Chaos Vehicles mature. 5.5 adds Modular Vehicle System (experimental) for dynamic vehicle construction. |
| **World Partition** | 5.4+ | Large world streaming | Official Epic solution for open-world games. Automatic spatial partitioning, One File Per Actor (OFPA) enables parallel development, distance-based level streaming eliminates manual level management. Used in City Sample, Fortnite. |
| **Mass Entity** | 5.4+ | Crowd simulation (ECS) | UE5's Entity-Component-System implementation. Supports thousands of AI agents with data-oriented calculations. MassCrowd and MassAI plugins provide crowd behaviors out of box. Graduated from experimental in UE5.1. |
| **Chaos Vehicles** | 5.4+ | Vehicle physics | Replaced PhysX vehicles. Physics-based simulation with Chaos physics engine. Required plugin: ChaosVehiclesPlugin. 5.5 adds Modular Vehicle System for dynamic vehicle assembly. |
| **Behavior Trees** | 5.4+ | AI decision-making | Industry standard, mature, well-documented. Blackboard component for state storage. Better tooling support than State Tree for complex hierarchies. Use for zombie AI logic. |
| **Data Tables** | 5.4+ | Configuration data | Row-based tabular data (CSV/Excel import). Struct inherits from FTableRowBase. Ideal for weapon stats, vehicle configs, zombie types, spawn tables. |
| **Data Assets** | 5.4+ | Complex configurations | Blueprint-exposed configuration objects. Use for game settings, difficulty presets, vehicle tuning profiles. Complements Data Tables for nested/hierarchical data. |
| **Nanite** | 5.4+ | Virtualized geometry | Automatic LOD for static meshes. Critical for city-scale environments with millions of polygons. Reduces manual LOD work to zero. |
| **Lumen** | 5.4+ | Global illumination | Dynamic GI and reflections. Essential for day/night cycle in Charlotte city. No pre-baked lighting required. |

### Required Plugins

| Plugin | Required | Purpose | Notes |
|--------|----------|---------|-------|
| **WorldPartition** | Yes | World Partition system | Core plugin for large world management |
| **MassAI** | Yes | Mass Entity AI behaviors | Enables crowd simulation, spawning, navigation |
| **MassCrowd** | Yes | Crowd-specific behaviors | LOD-based animation, avoidance, waypoint following |
| **ChaosVehiclesPlugin** | Yes | Vehicle physics | Required for Chaos Vehicles system |
| **NavigationSystem** | Yes | AI pathfinding | Required for zombie navigation |
| **AIModule** | Yes | Behavior Trees, Blackboard | Core AI functionality |
| **GameplayTasks** | Yes | AI task system | Required by Mass Entity and AI systems |
| **Niagara** | Recommended | VFX system | Blood splatter, explosions, weather effects |
| **AudioModulation** | Recommended | Dynamic audio | Engine sounds, zombie moans, ambient |

### Supporting Libraries

| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| **CommonUI** | 5.4+ | UI framework | For HUD, menus, in-game UI. Lyra-style input handling. |
| **EnhancedInput** | 5.4+ | Input system | Modern input handling replacing legacy input. Action-based, device-agnostic. |
| **GameFeatures** | 5.4+ | Modular content | If using Lyra-style plugin architecture for game modes/content. Advanced - consider post-MVP. |
| **Rivermax** | Optional | nDisplay integration | Only for multi-display setups or virtual production |
| **PythonScriptPlugin** | Optional | Editor automation | For pipeline automation, batch processing city tiles |

### Development Tools

| Tool | Purpose | Notes |
|------|---------|-------|
| **Unreal Editor** | Primary development | Enable World Partition editor, Mass Entity debugger |
| **Visual Studio 2022** | C++ development | Required for gameplay programming. Use 17.8+ for UE5.5. |
| **Rider for Unreal** | Alternative IDE | JetBrains IDE with superior Blueprint integration |
| **Unreal Insights** | Profiling | Critical for open-world performance. Trace Mass Entity, streaming, memory. |
| **PIE (Play In Editor)** | Iteration testing | World Partition supports PIE with spatial loading |
| **Cook On The Fly** | Iteration testing | Faster iteration without full cooks |

## Installation

### Project Setup (C++)

```bash
# Create new UE5.4+ C++ project
# Enable required plugins in Edit > Plugins:

# Core Systems
- World Partition (WorldPartition)
- Mass AI (MassAI)
- Mass Crowd (MassCrowd)
- Chaos Vehicles (ChaosVehiclesPlugin)
- Navigation System (NavigationSystem)
- AI Module (AIModule)

# Recommended
- Niagara
- CommonUI
- EnhancedInput
```

### Build.cs Dependencies

```csharp
// YourProject.Build.cs
PublicDependencyModuleNames.AddRange(new string[] {
    "Core",
    "CoreUObject",
    "Engine",
    "InputCore",

    // World Partition
    "WorldPartition",

    // Mass Entity
    "MassEntity",
    "MassAI",
    "MassCrowd",
    "MassCommon",
    "MassLOD",
    "MassRepresentation",
    "MassSpawner",
    "MassMovement",
    "MassActorSpawner",

    // Vehicles
    "ChaosVehicles",
    "ChaosVehiclesCore",

    // AI
    "AIModule",
    "NavigationSystem",

    // Data
    "GameplayTags",
    "DeveloperSettings",

    // UI
    "CommonUI",
    "EnhancedInput",
});
```

### Engine.ini Configuration

```ini
[/Script/Engine.Engine]
; Enable World Partition by default for new levels
bUseWorldPartition=True

[/Script/WorldPartition.WorldPartitionRuntimeSpatialHash]
; Cell size tuning for Charlotte city streaming
DefaultWorldPartitionRuntimeSpatialHashCellSize=12800
DefaultWorldPartitionRuntimeSpatialHashGridsToPlaceActorsIn=1

[/Script/MassEntity.MassEntitySettings]
; Mass Entity configuration
bDebugMassEntity=False
```

## Alternatives Considered

| Recommended | Alternative | When to Use Alternative |
|-------------|-------------|-------------------------|
| **World Partition** | Level Streaming (Legacy) | Only for existing projects pre-UE5. World Partition is superior for new open-world projects. |
| **World Partition** | Open World (Legacy) | Never. Open World plugin is deprecated. World Partition replaces it. |
| **Mass Entity** | AIController per actor | Only for <20 AI agents. Mass Entity has overhead but scales to thousands. |
| **Mass Entity** | Custom ECS | Only if you need features Mass Entity doesn't provide. Mass Entity is integrated and production-ready. |
| **Behavior Trees** | State Tree | State Tree if you need state-machine-style AI with selector logic combined. Simpler for certain AI patterns. Less tooling maturity. |
| **Chaos Vehicles** | Custom physics | Only for arcade-style vehicles where realism doesn't matter. Chaos provides realistic simulation. |
| **Data Tables** | JSON parsing | Only for runtime modding. Data Tables are editor-friendly, type-safe, and optimized. |
| **Data Tables** | Hardcoded values | Never. Data-driven architecture enables iteration without code changes. |
| **Nanite** | Manual LODs | Only for VR or mobile. Nanite is essential for open-world complexity management. |

## What NOT to Use

| Avoid | Why | Use Instead |
|-------|-----|-------------|
| **PhysX Vehicles** | Deprecated, removed in UE5 | Chaos Vehicles |
| **Legacy Input System** | Deprecated, no longer maintained | Enhanced Input |
| **Level Streaming Volumes** | Replaced by World Partition | World Partition with Loading Range |
| **UMG for 3D UI** | Performance issues with many widgets | CommonUI with Slate optimization |
| **Actor-based Crowds** | Doesn't scale past ~50 actors | Mass Entity |
| **Blueprint-heavy AI** | Performance bottleneck for 200+ agents | C++ for Mass Entity, Blueprint for configuration |
| **Hardcoded Config** | No iteration, requires recompilation | Data Tables / Data Assets |
| **Single Level World** | Memory limits, no streaming | World Partition |
| **Manual LOD Creation** | Time-consuming, error-prone | Nanite (automatic) |

## Stack Patterns by Variant

### Standard Open World (Zombie Taxi Default)

```
World Partition + Mass Entity + Chaos Vehicles + Behavior Trees + Data Tables
```

**Rationale:** Proven combination for city-scale open world with crowds. Used in City Sample. All systems production-ready in UE5.4+.

### VR / Mobile Open World

```
World Partition (tuned cell sizes) + Actor-based AI (<20) + Simplified Vehicles + Manual LODs
```

**Rationale:** Mass Entity and Nanite have VR/mobile limitations. Reduce complexity, tune streaming for platform.

### Lyra-Style Modular

```
All Standard + GameFeatures Plugin + CommonUI + EnhancedInput
```

**Rationale:** If you need modular content loading, game modes as plugins. Adds complexity but enables live updates. Consider post-MVP.

### Maximum Performance

```
World Partition + Mass Entity (C++ heavy) + State Tree + Custom Vehicle Physics
```

**Rationale:** If you need every CPU cycle. State Tree has lower overhead than Behavior Trees. Custom physics tuned for your needs. Advanced only.

## C++ vs Blueprint Patterns

### C++ (Required)

| System | Why C++ |
|--------|---------|
| **Mass Entity Processors** | Performance. Fragment processors must be C++. |
| **Vehicle Physics Tuning** | Complex calculations, performance-critical. |
| **Data Table Parsing** | Bulk operations, type safety. |
| **AI Tasks (complex)** | Performance for 200+ zombies. |
| **Custom World Partition handlers** | Low-level streaming control. |
| **Performance-critical calculations** | Pathfinding, spatial queries, crowd avoidance. |

### Blueprint (Recommended)

| System | Why Blueprint |
|--------|---------------|
| **Data Asset Configuration** | Designer-friendly, no compilation. |
| **Behavior Tree Logic** | Visual debugging, rapid iteration. |
| **UI/HUD** | CommonUI + Blueprint for rapid prototyping. |
| **Game Mode Setup** | Quick iteration on rules. |
| **Animation Blueprints** | Visual state machines. |
| **Level-specific Events** | No C++ recompilation needed. |

### Hybrid Pattern (Best Practice)

```cpp
// C++: Expose data-driven configuration to Blueprint
UCLASS(BlueprintType)
class UZombieConfig : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float MovementSpeed = 150.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float DetectionRange = 1000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    class UBehaviorTree* BehaviorTree;
};

// Blueprint: Configure via Data Asset
// Designer adjusts MovementSpeed, DetectionRange in editor
// No code changes required
```

## Version Compatibility

| UE Version | World Partition | Mass Entity | Chaos Vehicles | Nanite | Lumen |
|------------|-----------------|-------------|----------------|--------|-------|
| 5.4 | Stable | Production | Stable | Stable | Stable |
| 5.5 | Stable | Production | Stable + Modular (Exp) | Stable | Stable |
| 5.6 | Stable | Production | Stable | Stable | Stable |

**Recommendation:** Use UE 5.5 if available for Modular Vehicle System. Otherwise, 5.4 is fully production-ready.

**Breaking Changes Note:**
- UE5.3+ changed Mass Entity API slightly (fragment registration)
- UE5.4+ removed PhysX completely (must use Chaos)
- UE5.5+ Modular Vehicle System is experimental (API may change)

## Data Pipeline Integration

### blender_gsd to unreal_gsd

```
Blender (City Tiles) → USD/FBX → Unreal World Partition Cells

Recommended: USD for complex scenes (materials, instancing, hierarchy)
Alternative: FBX for simple geometry

World Partition Setup:
1. Create World Partition level
2. Import each city tile as separate actor
3. Actors auto-partition into cells
4. Configure Loading Range per actor type
```

### Charlotte City Streaming Strategy

```cpp
// World Partition cell configuration for Charlotte
// Based on City Sample best practices

// Ground-level detail (zombie spawn areas)
LoadingRange = 10000.0f; // 100m radius

// Buildings (exterior only)
LoadingRange = 20000.0f; // 200m radius

// Distant buildings (HLOD)
LoadingRange = 50000.0f; // 500m radius

// Skybox/environment
LoadingRange = 100000.0f; // 1km radius
```

## Sources

- **Epic Games Documentation** — World Partition, Mass Entity, State Tree, Data-Driven Gameplay Elements (HIGH confidence)
- **City Sample Project** — Epic's reference implementation for open world (HIGH confidence - official sample)
- **Lyra Sample Game** — GameFeatures plugin architecture (HIGH confidence - official sample)
- **Unreal Engine 5.5 Release Notes** — Feature availability, version compatibility (HIGH confidence)
- **Community Research (2024-2025)** — Best practices, performance patterns (MEDIUM confidence - verified with official docs)

---
*Stack research for: Unreal Engine 5.4+ Open-World Infrastructure*
*Project: unreal_gsd (Zombie Taxi)*
*Researched: 2026-02-24*
