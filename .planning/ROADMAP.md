# Roadmap: Unreal GSD Platform

## Overview

This roadmap delivers the Unreal GSD Platform infrastructure for building open-world games. Starting with plugin architecture foundation, we build World Partition streaming, vehicle physics, crowd simulation, event systems, and telemetry in dependency order. Each phase delivers a complete, verifiable capability that enables subsequent phases.

## Phases

- [x] **Phase 1: Plugin Architecture Foundation** - Core interfaces and plugin structure (2026-02-25)
- [x] **Phase 2: World Partition Setup** - Streaming configuration and HLOD (2026-02-25)
- [x] **Phase 2.5: Charlotte Map Data Acquisition** - Automated map tile and terrain data retrieval (2026-02-25)
- [x] **Phase 3: Streaming & Data Layers** - Runtime streaming behavior and cell callbacks (2026-02-25)
- [x] **Phase 4: Vehicle Core Systems** - Basic vehicle spawning and Chaos physics (2026-02-25)
- [x] **Phase 5: Vehicle Advanced Features** - Launch control, pooling, and testbed (2026-02-25)
- [ ] **Phase 6: Crowd Core Systems** - Mass Entity setup and LOD switching
- [ ] **Phase 7: Crowd AI & Navigation** - Zone Graph and Behavior Trees
- [ ] **Phase 8: Event System Core** - Event bus, tags, and scheduling
- [ ] **Phase 9: Event Implementations** - Concrete daily events
- [ ] **Phase 10: Telemetry & Validation** - Performance monitoring and commandlets

## Phase Details

### Phase 1: Plugin Architecture Foundation

**Goal**: Platform provides clean plugin structure with core interfaces that all feature plugins depend on

**Depends on**: Nothing (first phase)

**Requirements**: PLUG-01, PLUG-08, PLUG-09, PLUG-10

**Success Criteria** (what must be TRUE):
1. GSD_Core plugin compiles and loads in Unreal Editor
2. IGSDSpawnable and IGSDStreamable interfaces are defined and accessible to other plugins
3. Build.cs enforces correct module loading phase order
4. Games can reference platform plugins without circular dependencies
5. All cross-plugin communication uses interfaces (no hard dependencies between feature plugins)

**Plans**: 7 plans in 5 waves

Plans:
- [x] 01-PLAN.md - GSD_Core Plugin Structure
- [x] 02-PLAN.md - Spawning Interfaces
- [x] 03-PLAN.md - Streaming and Audio Interfaces
- [x] 04-PLAN.md - Network and Determinism Interfaces
- [x] 05-PLAN.md - Base Classes
- [x] 06-PLAN.md - Performance, Determinism, and SaveGame
- [x] 07-PLAN.md - Unit Tests and Verification

---

### Phase 2: World Partition Setup

**Goal**: City environment streams correctly with World Partition grid-based cells and HLOD for distant content

**Depends on**: Phase 1

**Requirements**: WP-01, WP-02, WP-04, WP-07, PLUG-02

**Success Criteria** (what must be TRUE):
1. World Partition is configured with grid-based cells sized for Charlotte urban density
2. HLOD generates automatically for distant content (3 LOD levels)
3. One File Per Actor workflow is enabled for team collaboration
4. City tiles import from USD/FBX with correct scale and collision
5. GSD_CityStreaming plugin provides World Partition configuration

**Plans**: 6 plans in 4 waves

Plans:
- [x] 02-01-PLAN.md - GSD_CityStreaming Plugin Foundation
- [x] 02-02-PLAN.md - HLOD Layer Configuration
- [x] 02-03-PLAN.md - Streaming Source Component
- [x] 02-04-PLAN.md - City Tile Import Pipeline
- [x] 02-05-PLAN.md - Charlotte City Level Setup
- [x] 02-06-PLAN.md - Verification and Editor Tests

---

### Phase 2.5: Charlotte Map Data Acquisition

**Goal**: Automated pipeline retrieves Charlotte map imagery and terrain data for I-485 corridor

**Depends on**: Phase 2

**Requirements**: MAP-01, MAP-02, MAP-03, MAP-04

**Success Criteria** (what must be TRUE):
1. System retrieves map tiles at configurable zoom levels for I-485 corridor
2. Coordinate system defines capture grid covering entire highway loop
3. DEM (Digital Elevation Model) height data acquired at required fidelity
4. Highway boundary geometry extracted from OpenStreetMap
5. Pipeline is scriptable/automatable for refresh and updates

**Tech Stack**: Python + GDAL + PyProj + OSMnx + Rasterio

**Plans**: 5 plans in 5 waves

Plans:
- [x] 02.5-01-PLAN.md - Package Foundation and Coordinate System
- [x] 02.5-02-PLAN.md - Data Acquisition Scripts (Tiles, DEM, Highway)
- [x] 02.5-03-PLAN.md - Data Transformation Scripts (Heightmap, Vectors)
- [x] 02.5-04-PLAN.md - Pipeline Orchestration
- [x] 02.5-05-PLAN.md - Unit Tests and Verification

---

### Phase 3: Streaming & Data Layers

**Goal**: Streaming behavior is predictive and runtime-configurable via Data layers

**Depends on**: Phase 2

**Requirements**: WP-03, WP-05, WP-06, WP-08

**Success Criteria** (what must be TRUE):
1. Streaming source component attached to player enables predictive loading
2. Streaming source component attached to vehicles enables predictive loading during traversal
3. User can toggle Data Layers at runtime for dynamic events
4. Multiple Data Layers exist (base city, events, construction, parties)
5. Streaming cell load times are tracked in telemetry

**Plans**: 5 plans in 3 waves

Plans:
- [x] 03-01-PLAN.md - GSDDataLayerManager Subsystem
- [x] 03-02-PLAN.md - Data Layer Asset Setup
- [x] 03-03-PLAN.md - Streaming Telemetry System
- [x] 03-04-PLAN.md - Vehicle Streaming Integration
- [x] 03-05-PLAN.md - Verification and Editor Tests

---

### Phase 4: Vehicle Core Systems

**Goal**: Vehicles spawn from Data Assets and are driveable with Chaos Vehicle physics

**Depends on**: Phase 3

**Requirements**: VEH-01, VEH-02, VEH-06, PLUG-03

**Success Criteria** (what must be TRUE):
1. System spawns vehicles from Data Assets defining body, wheels, suspension, tires
2. Player can drive vehicles with Chaos Vehicle physics (steering, acceleration, braking)
3. Wheel bone orientation is validated on import (prevents physics bugs)
4. GSD_Vehicles plugin provides vehicle abstraction and spawning

**Plans**: 6 plans in 5 waves

Plans:
- [x] 04-01-PLAN.md - GSD_Vehicles Plugin Foundation
- [x] 04-02-PLAN.md - Vehicle Data Assets
- [x] 04-03-PLAN.md - Vehicle Pawn and Movement
- [x] 04-04-PLAN.md - Wheel Bone Validation Utility
- [x] 04-05-PLAN.md - Vehicle Spawner Subsystem
- [x] 04-06-PLAN.md - Verification and Tests

---

### Phase 5: Vehicle Advanced Features

**Goal**: Vehicles support launch control, pooling for performance, and have validation tooling

**Depends on**: Phase 4

**Requirements**: VEH-03, VEH-04, VEH-05, VEH-07, VEH-08

**Success Criteria** (what must be TRUE):
1. Launch control system provides torque curve ramp and traction control
2. Vehicle pooling system reuses instances for performance
3. Tuning presets switch between compliance mode and chaos mode
4. Vehicle testbed map spawns 50 randomized cars for validation
5. Bumper/plow attachment points are defined in vehicle data

**Plans**: 7 plans in 7 waves

Plans:
- [x] 05-01-PLAN.md - Advanced Feature Data Assets
- [x] 05-02-PLAN.md - Vehicle Pool Subsystem
- [x] 05-03-PLAN.md - Launch Control Component
- [x] 05-04-PLAN.md - Attachment Component
- [x] 05-05-PLAN.md - Vehicle Pawn Extensions
- [x] 05-06-PLAN.md - Vehicle Testbed Actor
- [x] 05-07-PLAN.md - Verification and Editor Tests

---

### Phase 6: Crowd Core Systems

**Goal**: 200+ zombie entities spawn with LOD-based representation switching for performance

**Depends on**: Phase 3

**Requirements**: CROWD-01, CROWD-02, CROWD-03, CROWD-07, PLUG-04

**Success Criteria** (what must be TRUE):
1. Mass Entity framework is configured for crowd simulation
2. System spawns 200+ zombie entities
3. LOD representation switches: Actor -> Simplified Actor -> ISM -> Culled based on distance
4. Crowd testbed map validates 200+ entities at target framerate
5. GSD_Crowds plugin provides Mass Entity systems

**Plans**: 6 plans in 6 waves

Plans:
- [ ] 06-01-PLAN.md - GSD_Crowds Plugin Foundation
- [ ] 06-02-PLAN.md - Mass Entity Fragments and Processors
- [ ] 06-03-PLAN.md - Entity Config Data Asset
- [ ] 06-04-PLAN.md - Crowd Manager Subsystem
- [ ] 06-05-PLAN.md - Crowd Testbed Actor
- [ ] 06-06-PLAN.md - Verification and Commandlet

---

### Phase 7: Crowd AI & Navigation

**Goal**: Entities navigate intelligently via Zone Graph and hero NPCs use Behavior Trees

**Depends on**: Phase 6

**Requirements**: CROWD-04, CROWD-05, CROWD-06, CROWD-08

**Success Criteria** (what must be TRUE):
1. Entities navigate via Zone Graph (spline-based lanes)
2. Smart Objects provide interaction points (cover, ambush, etc.)
3. Hero NPCs use Behavior Trees and AI Perception
4. Velocity randomizer trait prevents synchronized movement

**Plans**: TBD

---

### Phase 8: Event System Core

**Goal**: Decoupled event system enables time-based scheduling with deterministic generation

**Depends on**: Phase 1

**Requirements**: EVT-01, EVT-02, EVT-03, EVT-04, EVT-09, PLUG-05

**Success Criteria** (what must be TRUE):
1. Event bus enables decoupled messaging between systems
2. Gameplay Tags organize event types and modifiers
3. Daily event definitions are stored as Data Assets
4. Event schedule generates deterministically from date and seed
5. Event modifiers affect spawn densities and navigation
6. GSD_DailyEvents plugin provides event scheduling

**Plans**: TBD

---

### Phase 9: Event Implementations

**Goal**: Concrete daily events modify gameplay in meaningful ways

**Depends on**: Phase 8, Phase 3, Phase 6

**Requirements**: EVT-05, EVT-06, EVT-07, EVT-08

**Success Criteria** (what must be TRUE):
1. Construction event spawns barricades and closes lanes
2. Bonfire event spawns FX and modifies zombie density locally
3. Block party event creates safe zones and crowd props
4. Zombie rave event boosts density with attractor volumes and audio

**Plans**: TBD

---

### Phase 10: Telemetry & Validation

**Goal**: Performance is observable and validation tools prevent regressions

**Depends on**: Phase 5, Phase 7, Phase 9

**Requirements**: TEL-01, TEL-02, TEL-03, TEL-04, TEL-05, TEL-06, TEL-07, TEL-08, PLUG-06, PLUG-07

**Success Criteria** (what must be TRUE):
1. Frame time and hitch metrics are tracked by district
2. Actor counts (vehicles, zombies, humans) are logged periodically
3. Streaming cell load times are captured in telemetry
4. GSDValidateAssets commandlet runs asset validation headlessly
5. GSDValidateWorldPartition commandlet validates streaming config
6. GSDRunPerfRoute commandlet captures performance baseline
7. Editor validation UI provides one-click checks
8. Asset budget enforcement fails builds when exceeded
9. GSD_Telemetry plugin provides metrics and logging
10. GSD_ValidationTools plugin provides editor utilities and commandlets

**Plans**: TBD

---

## Progress

**Execution Order:**
Phases execute in numeric order: 1 -> 2 -> 2.5 -> 3 -> 4 -> 5 -> 6 -> 7 -> 8 -> 9 -> 10

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. Plugin Architecture Foundation | 7/7 | Complete | 2026-02-25 |
| 2. World Partition Setup | 6/6 | Complete | 2026-02-25 |
| 2.5. Charlotte Map Data Acquisition | 5/5 | Complete | 2026-02-25 |
| 3. Streaming & Data Layers | 5/5 | Complete | 2026-02-25 |
| 4. Vehicle Core Systems | 6/6 | Complete | 2026-02-25 |
| 5. Vehicle Advanced Features | 7/7 | Complete | 2026-02-25 |
| 6. Crowd Core Systems | 0/6 | Not started | - |
| 7. Crowd AI & Navigation | 0/TBD | Not started | - |
| 8. Event System Core | 0/TBD | Not started | - |
| 9. Event Implementations | 0/TBD | Not started | - |
| 10. Telemetry & Validation | 0/TBD | Not started | - |

---

## Coverage Map

| Requirement | Phase |
|-------------|-------|
| PLUG-01 | 1 |
| PLUG-02 | 2 |
| PLUG-03 | 4 |
| PLUG-04 | 6 |
| PLUG-05 | 8 |
| PLUG-06 | 10 |
| PLUG-07 | 10 |
| PLUG-08 | 1 |
| PLUG-09 | 1 |
| PLUG-10 | 1 |
| WP-01 | 2 |
| WP-02 | 2 |
| WP-03 | 3 |
| WP-04 | 2 |
| WP-05 | 3 |
| WP-06 | 3 |
| WP-07 | 2 |
| WP-08 | 3 |
| MAP-01 | 2.5 |
| MAP-02 | 2.5 |
| MAP-03 | 2.5 |
| MAP-04 | 2.5 |
| VEH-01 | 4 |
| VEH-02 | 4 |
| VEH-03 | 5 |
| VEH-04 | 5 |
| VEH-05 | 5 |
| VEH-06 | 4 |
| VEH-07 | 5 |
| VEH-08 | 5 |
| CROWD-01 | 6 |
| CROWD-02 | 6 |
| CROWD-03 | 6 |
| CROWD-04 | 7 |
| CROWD-05 | 7 |
| CROWD-06 | 7 |
| CROWD-07 | 6 |
| CROWD-08 | 7 |
| EVT-01 | 8 |
| EVT-02 | 8 |
| EVT-03 | 8 |
| EVT-04 | 8 |
| EVT-05 | 9 |
| EVT-06 | 9 |
| EVT-07 | 9 |
| EVT-08 | 9 |
| EVT-09 | 8 |
| TEL-01 | 10 |
| TEL-02 | 10 |
| TEL-03 | 10 |
| TEL-04 | 10 |
| TEL-05 | 10 |
| TEL-06 | 10 |
| TEL-07 | 10 |
| TEL-08 | 10 |

**Total mapped: 55/55 (100%)**
