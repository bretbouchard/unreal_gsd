# PROJECT: Unreal GSD Platform

> Game Systems + Data platform for Unreal Engine with Council of Ricks orchestration

## Vision

Create a reusable Unreal foundation that enables rapid development of open-world games with:
- Real-world city-scale data streaming
- Vehicle systems with dynamic spawning
- Crowd AI (humans + zombies)
- Daily dynamic events
- Multi-agent Council orchestration for CI/CD, validation, and standards

## Context

This is the third stage in the pipeline:
```
fdx_gsd → blender_gsd → unreal_gsd
```

- **fdx_gsd**: Foundation/Design X systems
- **blender_gsd**: Blender asset pipelines, city generation
- **unreal_gsd**: Unreal integration, gameplay, Council orchestration

## First Client

**Zombie Taxi** is the first vertical-slice game:
- Charlotte city data
- Dynamic vehicles with launch control
- 200+ zombie crowds
- Daily "traffic chaos" events
- Fare/citation systems

## Key Principles

1. **Platform stays clean** - Games are clients that consume it
2. **Plugins-first architecture** - Each system is a plugin
3. **Data-driven** - Vehicles, events, crowds from Data Assets
4. **World Partition** - City-scale streaming built-in
5. **Council of Ricks** - Multi-agent orchestration for automation

## Tech Stack

| Component | Technology |
|-----------|------------|
| Engine | Unreal 5.3+ (5.4+ preferred) |
| Language | C++ primary, Blueprints secondary |
| Streaming | World Partition |
| Crowds | Mass Entity |
| Vehicles | Chaos Vehicles |
| AI | Behavior Trees + AI Perception |
| Backend | LangGraph/Swarm agents, MCP server |

## Success Metrics

- [ ] Platform boots, plugins compile
- [ ] City streaming vertical slice (1-2 districts)
- [ ] Vehicle integration with launch control
- [ ] 200+ zombies via Mass Entity
- [ ] Daily event system functional
- [ ] Council can trigger CI/CD pipeline
- [ ] Zombie Taxi vertical slice playable

## Constraints

- No engine modifications (use source build if needed)
- SLC principle: Simple, Lovable, Complete
- All game-specific code in projects/, not platform
- Deterministic events (seed-based)
