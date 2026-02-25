# Unreal GSD Platform

> Game Systems + Data platform for Unreal Engine projects with Council of Ricks orchestration

## Overview

Unreal GSD is a reusable foundation for Unreal Engine projects that provides:

- **City-scale data streaming** (World Partition, LODs, collision budgets)
- **Vehicle systems** (dynamic spawning, launch control, tuning data)
- **Crowd AI** (Mass Entity for crowds, Behavior Trees for hero NPCs)
- **Daily event system** (construction, bonfires, raves, block parties)
- **Council of Ricks backend** (multi-agent orchestration, CI/CD, validation)

## Architecture

```
unreal_gsd/
├── projects/              # Individual game projects (private repos, gitignored)
│   └── zombie_taxi/       # First vertical-slice client
├── docs/                  # Platform documentation
│   ├── architecture/      # System design docs
│   ├── guides/            # Dev guides
│   └── council/           # Council of Ricks specs
├── .planning/             # GSD project management
└── tools/                 # Shared tooling
```

## Repos

| Repo | Purpose | Visibility |
|------|---------|------------|
| `unreal_gsd` (this) | Platform core, docs, orchestration | Public |
| `projects/*` | Individual game clients | Private |

## Quick Start

```bash
# Create a new game project
./tools/new-project.sh my_game

# Initialize GSD planning
/gsd:new-project

# Sync with beads tracking
/bret:sync
```

## Plugins

| Plugin | Purpose |
|--------|---------|
| GSD_Core | Foundational types, config, logging, gameplay tags |
| GSD_CityStreaming | World Partition helpers, streaming volumes |
| GSD_Vehicles | Vehicle abstraction, launch control, tuning |
| GSD_Crowds | Mass crowd archetypes, perception, behavior |
| GSD_DailyEvents | Daily event system (construction, parties, chaos) |
| GSD_Telemetry | Events, metrics, replay markers |
| GSD_ValidationTools | Editor utilities, headless commandlets |

## Workflow

This project follows the **fdx_gsd → blender_gsd → unreal_gsd** pipeline:

1. **fdx_gsd** - Foundation/Design X systems
2. **blender_gsd** - Blender asset pipelines, city generation
3. **unreal_gsd** - Unreal integration, gameplay, Council orchestration

## Council of Ricks

The Council provides:

- Project scaffolding
- Validation & QA automation
- Asset ingestion (Blender → UE)
- CI build/test/release
- Design-rule enforcement

See [docs/council/](docs/council/) for details.

## First Client: Zombie Taxi

Zombie Taxi is the first vertical-slice game built on this platform:

- Charlotte city data streaming
- Dynamic vehicle spawning with launch control
- 200+ zombie crowds (Mass Entity)
- Daily "traffic chaos" events
- Fare/citation/ratings systems

## Documentation

- [Architecture Overview](docs/architecture/README.md)
- [City Data Pipeline](docs/guides/city-pipeline.md)
- [Vehicle Integration](docs/guides/vehicles.md)
- [Crowd System](docs/guides/crowds.md)
- [Daily Events](docs/guides/events.md)
- [Council Infrastructure](docs/council/README.md)

## Milestones

| Milestone | Goal |
|-----------|------|
| M0 | Platform boots, plugins compile |
| M1 | City streaming vertical slice |
| M2 | Vehicle integration complete |
| M3 | Crowds (200+ zombies, hero NPCs) |
| M4 | Daily event system |
| M5 | Council integration |
| M6 | Zombie Taxi vertical slice |

## License

MIT
