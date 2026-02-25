# REQUIREMENTS: Unreal GSD Platform

## v1 Requirements

### WP - World Partition & Streaming

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| WP-01 | User can configure World Partition with grid-based cells for city streaming | P0 | Open |
| WP-02 | System generates HLOD for distant content automatically | P0 | Open |
| WP-03 | Streaming source component attached to player and vehicles for predictive loading | P0 | Open |
| WP-04 | One File Per Actor workflow enabled for team collaboration | P1 | Open |
| WP-05 | User can toggle Data Layers at runtime for dynamic events | P0 | Open |
| WP-06 | System supports multiple Data Layers (base city, events, construction, parties) | P0 | Open |
| WP-07 | City tiles import from USD/FBX with correct scale and collision | P0 | Open |
| WP-08 | Streaming cell load times tracked in telemetry | P1 | Open |

### VEH - Vehicles

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| VEH-01 | System spawns vehicles from Data Assets (body, wheels, suspension, tires) | P0 | Open |
| VEH-02 | Player can drive vehicles with Chaos Vehicle physics | P0 | Open |
| VEH-03 | Launch control system provides torque curve ramp and traction control | P0 | Open |
| VEH-04 | Vehicle pooling system reuses instances for performance | P0 | Open |
| VEH-05 | Tuning presets switch between compliance mode and chaos mode | P1 | Open |
| VEH-06 | Wheel bone orientation validated on import (prevents physics bugs) | P0 | Open |
| VEH-07 | Vehicle testbed map spawns 50 randomized cars for validation | P1 | Open |
| VEH-08 | Bumper/plow attachment points defined in vehicle data | P1 | Open |

### CROWD - Crowds (Mass Entity)

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| CROWD-01 | Mass Entity framework configured for crowd simulation | P0 | Open |
| CROWD-02 | System spawns 200+ zombie entities with LOD switching | P0 | Open |
| CROWD-03 | LOD representation switches: Actor - Simplified Actor - ISM - Culled | P0 | Open |
| CROWD-04 | Entities navigate via Zone Graph | P0 | Open |
| CROWD-05 | Smart Objects provide interaction points (cover, ambush, etc.) | P1 | Open |
| CROWD-06 | Hero NPCs use Behavior Trees and AI Perception | P0 | Open |
| CROWD-07 | Crowd testbed map validates 200+ entities at target framerate | P0 | Open |
| CROWD-08 | Velocity randomizer trait prevents synchronized movement | P1 | Open |

### EVT - Events System

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| EVT-01 | Event bus enables decoupled messaging between systems | P0 | Open |
| EVT-02 | Gameplay Tags organize event types and modifiers | P0 | Open |
| EVT-03 | Daily event definitions stored as Data Assets | P0 | Open |
| EVT-04 | Event schedule generates deterministically from date and seed | P0 | Open |
| EVT-05 | Construction event spawns barricades and closes lanes | P0 | Open |
| EVT-06 | Bonfire event spawns FX and modifies zombie density locally | P1 | Open |
| EVT-07 | Block party event creates safe zones and crowd props | P1 | Open |
| EVT-08 | Zombie rave event boosts density with attractor volumes and audio | P1 | Open |
| EVT-09 | Event modifiers affect spawn densities and navigation | P0 | Open |

### PLUG - Plugin Architecture

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| PLUG-01 | GSD_Core plugin provides interfaces and base classes | P0 | Complete |
| PLUG-02 | GSD_CityStreaming plugin handles World Partition and imports | P0 | Open |
| PLUG-03 | GSD_Vehicles plugin provides vehicle abstraction and spawning | P0 | Open |
| PLUG-04 | GSD_Crowds plugin provides Mass Entity and hero NPC systems | P0 | Open |
| PLUG-05 | GSD_DailyEvents plugin provides event scheduling and modifiers | P0 | Open |
| PLUG-06 | GSD_Telemetry plugin provides metrics and logging | P1 | Open |
| PLUG-07 | GSD_ValidationTools plugin provides editor utilities and commandlets | P1 | Open |
| PLUG-08 | Games consume platform via plugin dependencies (game-as-client) | P0 | Complete |
| PLUG-09 | Build.cs enforces correct loading phase order | P0 | Complete |
| PLUG-10 | All cross-plugin communication via interfaces (no hard dependencies) | P0 | Complete |

### TEL - Telemetry & Tooling

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| TEL-01 | Frame time and hitch metrics tracked by district | P0 | Open |
| TEL-02 | Actor counts (vehicles, zombies, humans) logged periodically | P1 | Open |
| TEL-03 | Streaming cell load times captured in telemetry | P1 | Open |
| TEL-04 | GSDValidateAssets commandlet runs asset validation headlessly | P0 | Open |
| TEL-05 | GSDValidateWorldPartition commandlet validates streaming config | P0 | Open |
| TEL-06 | GSDRunPerfRoute commandlet captures performance baseline | P1 | Open |
| TEL-07 | Editor validation UI provides one-click checks | P1 | Open |
| TEL-08 | Asset budget enforcement fails builds when exceeded | P0 | Open |

---

## v2 Requirements (Deferred)

| ID | Requirement | Reason |
|----|-------------|--------|
| NET-01 | Multiplayer replication | Single-player first |
| NET-02 | Dedicated server build | Single-player first |
| PCG-01 | Procedural Content Graph for prop placement | Manual placement sufficient for v1 |
| SAVE-01 | World Partition state serialization | Complex, defer after core systems |

---

## Out of Scope

| Exclusion | Reason |
|-----------|--------|
| Engine modifications | Use source build if needed, no code changes |
| Marketplace distribution | Internal only for now |
| Mobile/console targets | PC first |
| VR/AR support | Not relevant to Zombie Taxi |
| Networking/multiplayer | Single-player first |
| Council of Ricks (build) | External system - invoke, don't build |

---

## Traceability

| Requirement | Phase | Plan |
|-------------|-------|------|
| PLUG-01 | 1 | 01, 05 |
| PLUG-02 | 2 | - |
| PLUG-03 | 4 | - |
| PLUG-04 | 6 | - |
| PLUG-05 | 8 | - |
| PLUG-06 | 10 | - |
| PLUG-07 | 10 | - |
| PLUG-08 | 1 | 01 |
| PLUG-09 | 1 | 01 |
| PLUG-10 | 1 | 02, 03, 04 |
| WP-01 | 2 | - |
| WP-02 | 2 | - |
| WP-03 | 3 | - |
| WP-04 | 2 | - |
| WP-05 | 3 | - |
| WP-06 | 3 | - |
| WP-07 | 2 | - |
| WP-08 | 3 | - |
| VEH-01 | 4 | - |
| VEH-02 | 4 | - |
| VEH-03 | 5 | - |
| VEH-04 | 5 | - |
| VEH-05 | 5 | - |
| VEH-06 | 4 | - |
| VEH-07 | 5 | - |
| VEH-08 | 5 | - |
| CROWD-01 | 6 | - |
| CROWD-02 | 6 | - |
| CROWD-03 | 6 | - |
| CROWD-04 | 7 | - |
| CROWD-05 | 7 | - |
| CROWD-06 | 7 | - |
| CROWD-07 | 6 | - |
| CROWD-08 | 7 | - |
| EVT-01 | 8 | - |
| EVT-02 | 8 | - |
| EVT-03 | 8 | - |
| EVT-04 | 8 | - |
| EVT-05 | 9 | - |
| EVT-06 | 9 | - |
| EVT-07 | 9 | - |
| EVT-08 | 9 | - |
| EVT-09 | 8 | - |
| TEL-01 | 10 | - |
| TEL-02 | 10 | - |
| TEL-03 | 10 | - |
| TEL-04 | 10 | - |
| TEL-05 | 10 | - |
| TEL-06 | 10 | - |
| TEL-07 | 10 | - |
| TEL-08 | 10 | - |

---

## Quality Criteria

Each requirement is:
- **Specific and testable:** "User can X" or "System does Y with Z"
- **User-centric:** Describes observable behavior
- **Atomic:** One capability per requirement
- **Independent:** Minimal dependencies on other requirements

---

## Summary

| Category | v1 Count | P0 | P1 |
|----------|----------|----|----|
| World Partition | 8 | 6 | 2 |
| Vehicles | 8 | 5 | 3 |
| Crowds | 8 | 5 | 3 |
| Events | 9 | 6 | 3 |
| Plugins | 10 | 8 | 2 |
| Telemetry | 8 | 4 | 4 |
| **Total** | **51** | **34** | **17** |
