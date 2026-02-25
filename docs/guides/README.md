# Development Guides

## City Data Pipeline

**Blender → Unreal import workflow**

### Principles

- Never import whole city as one mesh
- Chunk by region (grid tiles)
- Each tile: buildings, props, road meshes, landmarks
- Terrain: prefer Unreal Landscape

### Formats

| Format | Use Case |
|--------|----------|
| USD | Preferred for large scenes (clean hierarchy) |
| FBX | Acceptable (more manual discipline) |

### Streaming Model (World Partition)

- Use World Partition as baseline
- Chunk assets into cells
- Use Data Layers for:
  - Base city (always)
  - Night events
  - Construction
  - Bonfire sites
  - Block party overlays
  - Zombie rave overlays

### Collision Rules

| Asset Type | Collision |
|------------|-----------|
| Buildings | Simplified (UCX or auto convex) |
| Roads | Simple collision, avoid complex mesh |
| Props | Per-asset collision budgets enforced |

### LOD Rules

- Auto-generate LODs for background buildings
- Hero landmarks: custom LODs
- Validation tool fails builds if LOD/collision missing

---

## Vehicle Integration

### Platform Abstraction

Create `IGSDVehicleInterface` and `UGSDVehicleConfigDataAsset`

### Requirements

Vehicle must be spawnable from data:
- Body mesh
- Wheel set
- Suspension tuning
- Tire type (monster, run-flat, etc.)
- Bumper/plow attachment points

### Launch Control

- Torque curve ramp
- Traction control rules
- Tire grip modifiers
- "Compliance mode" vs "Chaos mode" tuning presets

### Recommendations

- Keep car generator logic in C++
- Expose tuning as Data Assets
- Create testbed map for spawning 50 randomized cars

---

## Crowd System (Humans + Zombies)

### Two-Tier NPC Approach

**Tier A — Hero NPCs (few)**
- Passengers
- Special enemies
- Quest NPCs

Use: Character BP + Behavior Tree + AI Perception

**Tier B — Mass NPCs (many)**
- Zombie crowds
- Ambient humans

Use: Mass Entity framework, simplified movement/animation

### Animation Strategy

- Simple rigs + few animations (walk, run, attack, stumble)
- Animation retargeting (Mixamo or marketplace)

---

## Daily Event System

### Data-Driven Events

Create:
- `UGSDDailyEventDefinition` (Data Asset)
- `UGSDEventModifier` components
- `UGSDEventSchedule` (seed-based generation)

### Event Types

| Event | Modifiers |
|-------|-----------|
| Construction | Barricades, closed lanes, reroute nav |
| Corpse bonfire | FX volumes, reduced zombie density, visibility |
| Block party | Crowd props, human "safe zone" rules |
| Zombie rave | Zombie cluster attractors, audio, lighting, density boost |

### Determinism

Events reproducible from:
- Date
- Seed
- District set

---

## Telemetry & Replay

### Minimum Telemetry

- Frame time / hitch metrics by district
- Actor counts: vehicles, zombies, humans
- Navmesh rebuild time
- Streaming cell load times
- "Citation events" (speeding/late/etc)

Store locally in dev; stream to backend later.
