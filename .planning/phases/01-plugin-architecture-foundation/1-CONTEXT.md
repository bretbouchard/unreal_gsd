# Phase 1 Context: Plugin Architecture Foundation

**Phase Goal:** Platform provides clean plugin structure with core interfaces that all feature plugins depend on

**Requirements:** PLUG-01, PLUG-08, PLUG-09, PLUG-10

---

## Philosophy: Opinionated & Rigid

The platform has a way. You follow it. You ship faster.

- Optimized for speed, not flexibility
- One way to do things, not multiple options
- Game developers push pixels and make games, platform handles the rest

---

## Interface Design

### IGSDSpawnable

**Scope:** Spawning + initialization

| Method | Purpose |
|--------|---------|
| `SpawnAsync(Config, Callback)` | Create instance with config, callback when complete |
| `GetSpawnConfig()` | Retrieve the config this was spawned with |
| `IsSpawned()` | Check if currently active |

**Key decisions:**
- **Async only** — consistent behavior, no hitches on large spawns (200+ zombies)
- **Blueprint-friendly** — expose to visual scripting from day one
- **Config-driven** — all spawn data passed in, no hardcoded values

### IGSDStreamable

**Scope:** Full streaming control

| Method | Purpose |
|--------|---------|
| `GetStreamingBounds()` | Return bounds for distance-based loading |
| `OnStreamIn()` | Callback when actor loads into memory |
| `OnStreamOut()` | Callback when actor unloads from memory |
| `GetStreamingPriority()` | Priority for load order |
| `ShouldPersist()` | Should survive across level loads |

**Key decisions:**
- **Blueprint-friendly** — expose to visual scripting from day one
- **Full control** — distance, callbacks, persistence, priority

---

## Plugin Granularity

**GSD_Core contains:**

| Component | In Core? | Notes |
|-----------|----------|-------|
| Interfaces | ✓ | `IGSDSpawnable`, `IGSDStreamable` |
| Base classes | ✓ | `AGSDActor`, `UGSDComponent` — all GSD types inherit from these |
| Logging macros | ✓ | `GSD_LOG()`, `GSD_WARN()`, `GSD_ERROR()` — consistent format |
| Data Asset base | ✓ | `UGSDDataAsset` — all config goes through this |
| Utility functions | ✓ | Common helpers — validation, math, etc. |

**Why opinionated:**
- All GSD actors inherit from `AGSDActor`
- All GSD components inherit from `UGSDComponent`
- All logging uses `GSD_LOG()` macros
- All config uses `UGSDDataAsset`
- All spawning uses `SpawnAsync()`

---

## Dependency Model

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Plugin location | Engine plugins | Install to Unreal, all projects see them |
| Build.cs dependency | Explicit | List each plugin individually, only pull what you need |
| Versioning | Git submodules | Each game pins to specific commit of unreal_gsd |
| Discovery | Both | README index for humans, `GSD_PluginInfo` metadata for tools |

**Game setup pattern:**
```bash
# In projects/my_game/
git submodule add https://github.com/bretbouchard/unreal_gsd.git unreal_gsd
# Run install script to copy plugins to Engine
```

**Build.cs pattern:**
```cpp
PublicDependencyModuleNames.AddRange(new string[] {
    "GSD_Core",
    "GSD_Vehicles",
    "GSD_Crowds"
    // Only what you need
});
```

---

## Naming Conventions

| Category | Convention | Example |
|----------|------------|---------|
| Class prefix | GSD | `AGSDActor`, `UGSDDataAsset` |
| Plugin folders | Underscore | `GSD_Core`, `GSD_Vehicles`, `GSD_DailyEvents` |
| Interface files | I prefix | `IGSDSpawnable.h`, `IGSDStreamable.h` |
| File organization | By type | `Interfaces/`, `Classes/`, `DataAssets/`, `Macros/` |

**Folder structure:**
```
GSD_Core/
├── Source/
│   ├── GSD_Core/
│   │   ├── Private/
│   │   │   ├── Classes/
│   │   │   │   ├── GSDActor.cpp
│   │   │   │   └── GSDComponent.cpp
│   │   │   ├── DataAssets/
│   │   │   │   └── GSDDataAsset.cpp
│   │   │   └── Macros/
│   │   │       └── GSDLog.cpp
│   │   └── Public/
│   │       ├── Interfaces/
│   │       │   ├── IGSDSpawnable.h
│   │       │   └── IGSDStreamable.h
│   │       ├── Classes/
│   │       │   ├── GSDActor.h
│   │       │   └── GSDComponent.h
│   │       ├── DataAssets/
│   │       │   └── GSDDataAsset.h
│   │       └── Macros/
│   │           └── GSDLog.h
│   └── GSD_Core.Build.cs
├── GSD_Core.uplugin
└── Resources/
```

---

## Success Criteria (from ROADMAP.md)

1. GSD_Core plugin compiles and loads in Unreal Editor
2. IGSDSpawnable and IGSDStreamable interfaces are defined and accessible to other plugins
3. Build.cs enforces correct module loading phase order
4. Games can reference platform plugins without circular dependencies
5. All cross-plugin communication uses interfaces (no hard dependencies between feature plugins)

---

## Deferred Ideas

(Things that came up but belong in future phases or separate discussions)

- Plugin installer script — automate copying to Engine folder
- `GSD_PluginInfo` data asset structure — define in Phase 1 or later
- Validation tools for plugin structure — belongs in Phase 10 (Telemetry & Validation)

---

## Summary

**Phase 1 delivers:**

- `GSD_Core` plugin with opinionated architecture
- `IGSDSpawnable` — async, Blueprint-friendly, config-driven spawning
- `IGSDStreamable` — full streaming control with callbacks
- `AGSDActor` / `UGSDComponent` — base classes for all GSD types
- `UGSDDataAsset` — base class for all configuration
- `GSD_LOG()` macros — consistent logging
- Build.cs structure for correct loading phases

**At game level, developers:**
- Inherit from `AGSDActor` / `UGSDComponent`
- Use `SpawnAsync()` to create things
- Store config in `UGSDDataAsset` derivatives
- Log with `GSD_LOG()`
- Push pixels and make games
