# Architecture Overview

## High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                   unreal_gsd (Public Repo)                   │
│  Platform core, docs, orchestration, Council of Ricks       │
└────────────────────┬────────────────────────────────────────┘
                     │
          ┌──────────┼──────────┐
          ▼          ▼          ▼
┌─────────────┐ ┌─────────────┐ ┌─────────────┐
│ zombie_taxi │ │ future_game │ │ another_game│
│  (Private)  │ │  (Private)  │ │  (Private)  │
└─────────────┘ └─────────────┘ └─────────────┘
```

## Repos

### A) unreal_gsd (Platform - Public)

- Unreal project (C++ primary)
- Plugins (GSD Core, City Streaming, Vehicles, AI, Events, Telemetry)
- Editor utilities (import pipelines, validators)
- Tests (functional + automation)
- Council of Ricks infrastructure

### B) projects/* (Game Clients - Private)

- Minimal Unreal content + gameplay modules
- Depends on platform plugins
- Only game-specific assets/data/config

**Key Rule:** Platform stays clean; games are "clients" that consume it.

## Unreal Engine Requirements

| Requirement | Version/Feature |
|-------------|-----------------|
| Engine | Unreal 5.3+ (5.4+ preferred) |
| Project Type | C++ (not Blueprint-only) |
| Architecture | Plugins-first |

### Mandatory Engine Features

- World Partition
- Large World Coordinates
- Chaos Vehicles (or custom)
- Mass Entity (crowds)
- NavMesh / AI Perception
- Gameplay Tags
- Data Assets + Data Tables

## C++ Module Structure

| Module | Purpose |
|--------|---------|
| GSDCore | Foundational types, config, logging, gameplay tags |
| GSDWorld | World Partition helpers, streaming volumes, city chunks |
| GSDImport | Editor-only import automation (USD/FBX) |
| GSDVehicles | Vehicle abstraction, launch control, tuning |
| GSDAI | Mass crowd archetypes, perception, behavior trees |
| GSDEvents | Daily event system |
| GSDTelemetry | Events, metrics, replay markers |
| GSDNet | Replication conventions (optional) |

## Plugin Strategy

Each system is a separate plugin so clients can consume cleanly:

1. GSD_Core
2. GSD_CityStreaming
3. GSD_Vehicles
4. GSD_Crowds
5. GSD_DailyEvents
6. GSD_ValidationTools
7. GSD_Telemetry

## Content Structure

```
/Content/GSD/
  Config/
  Maps/
    Testbeds/
      VehicleTestbed.umap
      CrowdTestbed.umap
      CityStreamingTestbed.umap
  Blueprints/
    Core/
    UI/
  Data/
    Tags/
    Curves/
    DT_*
  Materials/
  FX/
  Audio/
```

## Naming Conventions

| Type | Prefix | Example |
|------|--------|---------|
| Plugins | GSD_ | GSD_Vehicles |
| Gameplay Tags | Category.* | City.*, Vehicle.*, Crowd.*, Event.*, Taxi.* |
| Data Assets | DA_ | DA_VehicleConfig |
| Data Tables | DT_ | DT_EventDefinitions |
