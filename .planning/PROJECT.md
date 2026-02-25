# PROJECT: Unreal GSD Platform

> Infrastructure for building Unreal Engine games - tools, utilities, automation

## What This Is

**unreal_gsd** is the common infrastructure layer for building Unreal games. It provides:
- Tools and automation scripts
- Shared utilities and patterns
- GSD workflow (planning, tracking, tribe knowledge)

**Games live in `projects/`** as separate private repos. The first game is **Zombie Taxi**.

## Architecture

```
unreal_gsd/                    # THIS REPO - infrastructure
├── tools/                     # Automation, scripts, utilities
├── docs/                      # Shared knowledge, patterns
├── .planning/                 # GSD workflow (tribe knowledge)
└── projects/                  # Individual games (private repos)
    └── zombie_taxi/           # First game - production quality

blender_gsd → unreal_gsd
     │              │
     │              └── Imports city tiles (USD/FBX)
     │
     └── Exports chunked city data
```

## Three Distinct Layers

| Layer | Purpose | Location |
|-------|---------|----------|
| **unreal_gsd** | Infrastructure (tools, utilities, automation) | This repo |
| **projects/** | Actual games (assets, gameplay, content) | Private repos |
| **GSD** | Workflow/methodology (planning, tracking, Council) | .planning/ |

## Pipeline Integration

**blender_gsd → unreal_gsd**
- Blender exports city tiles as USD/FBX
- Unreal imports with World Partition streaming
- Chunked by region (not one giant mesh)

## First Game: Zombie Taxi

**Production quality game to ship** - not a throwaway prototype.

| Feature | Description |
|---------|-------------|
| City | Charlotte data streaming |
| Vehicles | Dynamic spawning with launch control |
| Crowds | 200+ zombies via Mass Entity |
| Events | Daily "traffic chaos" (construction, bonfires, raves) |
| Gameplay | Fare/citation systems, passenger behavior |

## Council of Ricks

**External system** - invoke it, don't build it.

The Council is a slash command with specialized agents for:
- Plan review
- Code review
- Process enforcement
- Quality gates

## Tech Stack

| Component | Technology |
|-----------|------------|
| Engine | Unreal 5.4+ (decide in Phase 1) |
| Language | C++ primary, Blueprints secondary |
| Streaming | World Partition |
| Crowds | Mass Entity |
| Vehicles | Chaos Vehicles |
| AI | Behavior Trees + AI Perception |

## First Milestone: Vertical Slice

**Goal:** Prove the whole stack works together.

- [ ] City district loads and streams
- [ ] Drive a car with launch control
- [ ] Spawn 200+ zombies that move

All three working in a single playable slice.

## Key Principles

1. **Platform stays clean** - Games are clients, not mixed in
2. **Plugins-first** - Each system is a separate plugin
3. **Data-driven** - Vehicles, events, crowds from Data Assets
4. **World Partition** - City-scale streaming from day one
5. **Deterministic** - Events seed-based for reproducibility

## Constraints

- No engine modifications (use source build if needed)
- SLC principle: Simple, Lovable, Complete
- All game-specific code in projects/, not platform
- Internal only for now (symlink/submodule deps)

## Requirements

### Validated

(None yet — ship to validate)

### Active

- [ ] Platform infrastructure bootstraps
- [ ] City streaming works (World Partition)
- [ ] Vehicles spawn and drive (Chaos + launch control)
- [ ] Crowds spawn and move (Mass Entity)
- [ ] Daily events modify gameplay
- [ ] Zombie Taxi vertical slice playable

### Out of Scope

- Marketplace distribution — internal only for now
- Multiplayer/networking — single-player first
- Engine modifications — use source build if needed

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Zombie Taxi = production quality | Not a prototype, real game to ship | — Pending |
| Council of Ricks = external | Slash command system, invoke don't build | — Pending |
| Engine version = flexible | Decide during Phase 1 | — Pending |
| Vertical slice first | Prove all systems work together | — Pending |

---
*Last updated: 2026-02-24 after initialization*
