# ROADMAP: Unreal GSD Platform

## Milestone Overview

```
M0: Platform Boots     ████░░░░░░░░░░░░░░░░  20%
M1: City Streaming     ░░░░░░░░░░░░░░░░░░░░   0%
M2: Vehicles           ░░░░░░░░░░░░░░░░░░░░   0%
M3: Crowds             ░░░░░░░░░░░░░░░░░░░░   0%
M4: Daily Events       ░░░░░░░░░░░░░░░░░░░░   0%
M5: Council Integration░░░░░░░░░░░░░░░░░░░░   0%
M6: Zombie Taxi V1     ░░░░░░░░░░░░░░░░░░░░   0%
```

---

## Phase 0: Project Initialization

**Goal:** Repository structure, GSD setup, documentation

**Requirements:** None (foundation)

**Deliverables:**
- [x] Public git repo initialized
- [x] projects/ folder with gitignore
- [x] Documentation structure
- [x] GSD planning structure
- [ ] Initial commit pushed to remote

---

## Phase 1: Platform Boots (M0)

**Goal:** UE project compiles, plugins compile, testbed maps load

**Requirements:** CORE-01, CORE-02, CORE-03, CORE-04

**Deliverables:**
- [ ] UE 5.3+ C++ project created
- [ ] Plugin skeletons (GSD_Core, GSD_CityStreaming, GSD_Vehicles, GSD_Crowds, GSD_DailyEvents, GSD_Telemetry, GSD_ValidationTools)
- [ ] Gameplay Tags registry
- [ ] Logging system
- [ ] Testbed maps load without crashes

---

## Phase 2: City Streaming Vertical Slice (M1)

**Goal:** Import 1-2 districts, World Partition streaming works

**Requirements:** CITY-01 through CITY-07

**Deliverables:**
- [ ] World Partition enabled
- [ ] Chunk-based import pipeline
- [ ] USD/FBX import working
- [ ] Collision validation passing
- [ ] LOD auto-generation
- [ ] Data layers for events
- [ ] CityStreamingTestbed.umap functional

---

## Phase 3: Vehicle Integration (M2)

**Goal:** Dynamic vehicle spawner works, launch control ported

**Requirements:** VEH-01 through VEH-06

**Deliverables:**
- [ ] IGSDVehicleInterface defined
- [ ] UGSDVehicleConfigDataAsset created
- [ ] Dynamic spawner functional
- [ ] Launch control integrated
- [ ] Tuning presets working
- [ ] VehicleTestbed.umap with 50 randomized cars

---

## Phase 4: Crowds (M3)

**Goal:** 200+ zombies via Mass, 10 hero NPCs with BT

**Requirements:** CROWD-01 through CROWD-06

**Deliverables:**
- [ ] Mass Entity integration
- [ ] 200+ zombie spawning
- [ ] Hero NPC Behavior Trees
- [ ] AI Perception working
- [ ] Animation retargeting
- [ ] CrowdTestbed.umap functional

---

## Phase 5: Daily Event System (M4)

**Goal:** Events modify spawn densities, deterministic schedule

**Requirements:** EVENT-01 through EVENT-08

**Deliverables:**
- [ ] UGSDDailyEventDefinition
- [ ] UGSDEventModifier components
- [ ] UGSDEventSchedule (seed-based)
- [ ] Construction event
- [ ] Bonfire event
- [ ] Block party event
- [ ] Zombie rave event
- [ ] "Today's chaos" summary output

---

## Phase 6: Council Integration (M5)

**Goal:** Council can trigger import, validate, test, build, report

**Requirements:** COUNCIL-01 through COUNCIL-07, VALID-01 through VALID-05

**Deliverables:**
- [ ] MCP server operational
- [ ] Project Shepherd API
- [ ] Asset Ingestion Service
- [ ] Validation/QA Agent
- [ ] Performance Agent
- [ ] CI/CD pipeline
- [ ] Headless commandlets
- [ ] Single report output

---

## Phase 7: Zombie Taxi Vertical Slice (M6)

**Goal:** 1 district, pick-up → drive → drop-off loop, citations

**Requirements:** All above + Zombie Taxi specific

**Deliverables:**
- [ ] ZombieTaxiCore plugin
- [ ] Fare/timing/citation systems
- [ ] Passenger behavior definitions
- [ ] Zombie variants (as data)
- [ ] UI for taxi game
- [ ] Playable vertical slice

---

## Dependency Graph

```
Phase 0 (Init)
    │
    ▼
Phase 1 (Platform Boots)
    │
    ├──────────────┬──────────────┐
    ▼              ▼              ▼
Phase 2       Phase 3        Phase 4
(City)        (Vehicles)     (Crowds)
    │              │              │
    └──────────────┼──────────────┘
                   ▼
              Phase 5 (Events)
                   │
                   ▼
              Phase 6 (Council)
                   │
                   ▼
              Phase 7 (Zombie Taxi)
```

**Note:** Phases 2, 3, 4 can run in parallel after Phase 1.
