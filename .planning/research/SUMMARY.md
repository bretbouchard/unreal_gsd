# Project Research Summary

**Project:** unreal_gsd (Zombie Taxi)
**Domain:** Unreal Engine 5.4+ Open-World Game Infrastructure
**Researched:** 2026-02-24
**Confidence:** HIGH

## Executive Summary

Unreal Engine 5.4+ provides a mature, production-ready foundation for open-world game infrastructure through **World Partition** for spatial streaming, **Mass Entity** for crowd simulation, and **Chaos Vehicles** for physics-based driving. Epic's City Sample and The Matrix Awakens demonstrate these systems working at scale (10,000+ entities, 15km worlds). The key architectural insight is that infrastructure must be built as a layered **plugin architecture** with clean separation between platform (reusable systems) and game (Zombie Taxi-specific logic).

The recommended approach follows a **layered plugin architecture** with GSD_Core as the foundation, GSD_CityStreaming for world management, and feature plugins (Vehicles, Crowds, Events, Telemetry) consuming the core interfaces. All configuration should be data-driven via UPrimaryDataAsset derivatives with soft references to prevent memory bloat. World Partition cell sizes must be tuned for Charlotte's urban density (500-750m cells), and HLOD must be generated before any open-world testing.

The critical risks are: (1) World Partition misconfiguration causing streaming stutter during vehicle traversal, (2) Mass Entity zombies without LOD-based representation switching destroying CPU performance, and (3) hard asset references causing memory bloat that prevents console memory budgets. Each of these requires architectural decisions in early phases that are expensive to retrofit later.

## Key Findings

### Recommended Stack

UE5.4+ with World Partition, Mass Entity, Chaos Vehicles, Behavior Trees, and Data Tables forms the proven combination for city-scale open worlds. This stack is used in Epic's City Sample and The Matrix Awakens, with all systems production-ready as of UE5.4. UE5.5 adds Modular Vehicle System (experimental) for dynamic vehicle assembly.

**Core technologies:**
- **World Partition (5.4+):** Large world streaming with automatic spatial partitioning, OFPA for parallel development
- **Mass Entity (5.4+):** ECS-style crowd simulation supporting thousands of AI agents with data-oriented calculations
- **Chaos Vehicles (5.4+):** Physics-based vehicle simulation replacing deprecated PhysX
- **Behavior Trees (5.4+):** Industry-standard AI decision-making with mature tooling
- **Data Tables/Data Assets (5.4+):** Data-driven configuration enabling iteration without recompilation
- **Nanite + Lumen (5.4+):** Virtualized geometry and dynamic GI eliminating manual LOD/lighting work

**Required plugins:** WorldPartition, MassAI, MassCrowd, ChaosVehiclesPlugin, NavigationSystem, AIModule, GameplayTasks

### Expected Features

**Must have (table stakes):**
- World Partition Streaming with HLOD - seamless traversal without loading screens
- Data Layers - runtime content toggling for day/night and events
- Save/Load System - USaveGame with World Partition state serialization
- Basic Vehicle Physics - Chaos Vehicle setup with playable taxi
- Entity LOD System - distance-based optimization for crowds/vehicles
- GameMode/GameState - core UE framework for rules and state sync
- Event System - Gameplay Tags + UGameplayMessageSubsystem for decoupled communication
- Plugin Structure - infrastructure as plugins, game as client

**Should have (competitive):**
- Mass Entity Crowd System - 10,000+ entities with cache-optimized memory layout
- Smart Objects Integration - crowds interact meaningfully with environment
- Zone Graph Navigation - spline-based lanes more efficient than NavMesh for roads
- PCG for Foliage - procedural trees, grass placement
- Runtime Data Layer Transforms - dynamic world changes without loading screens (UE5.6+)
- Unified Streaming Budget - stable 60 FPS during traversal (UE5.6+)

**Defer (v2+):**
- Nanite Foliage - requires UE5.7, significant rendering changes
- Neural Navigation (NNE) - experimental, unclear benefit
- Multiplayer Infrastructure - adds significant complexity, single-player first
- Cloud Save Integration - platform-specific, requires backend
- Full City Sample Integration - large asset import, can start simpler

### Architecture Approach

A **layered plugin architecture** with GSD_Core as the foundation providing interfaces (IGSDSpawnable, IGSDStreamable) and base classes. Feature plugins (GSD_Vehicles, GSD_Crowds, GSD_DailyEvents, GSD_Telemetry) depend only on Core and communicate via event delegates. GSD_Game project module consumes all plugins, never the reverse. All configuration stored in UPrimaryDataAsset derivatives with TSoftObjectPtr for lazy loading.

**Major components:**
1. **GSD_Core** - Foundation interfaces, base data assets, subsystem base classes, logging utilities
2. **GSD_CityStreaming** - World Partition configuration, streaming policies, HLOD management, cell callbacks
3. **GSD_Vehicles** - Vehicle spawning pool, Chaos integration, AI vehicle behavior
4. **GSD_Crowds** - Mass Entity integration, LOD representation switching, Smart Objects
5. **GSD_DailyEvents** - Time-based event scheduling, Data Tables for event definitions
6. **GSD_Telemetry** - Async analytics, performance monitoring, fire-and-forget HTTP

### Critical Pitfalls

1. **World Partition Grid Cell Size Misconfiguration** - Default 1km cells wrong for dense urban; use 500-750m cells for Charlotte, profile with `stat streaming` during fast vehicle traversal
2. **Missing HLOD for Unloaded Cells** - Distant content vanishes or causes draw call explosion; generate HLOD immediately after WP setup, use 3 LOD levels
3. **Mass Entity Without LOD Representation Switching** - 200+ full skeletal mesh zombies destroys CPU; implement 4-tier LOD: actor (0-50m), simplified (50-200m), ISM (200-500m), culled (500m+)
4. **Hard References Instead of Soft References** - Memory bloat from Data Assets; use TSoftObjectPtr everywhere, load via Asset Manager
5. **Not Testing in Packaged Builds** - PIE streaming differs from cooked builds; establish regular packaged build testing cadence from Phase 1

## Implications for Roadmap

Based on research, suggested phase structure:

### Phase 1: World Partition Foundation
**Rationale:** World Partition is the foundation all other systems depend on. Must be configured correctly before any content or gameplay systems are added. Establishes streaming patterns that affect everything downstream.
**Delivers:** Functional streaming world with HLOD, cell loading callbacks, packaged build testing
**Addresses:** World Partition Streaming, Data Layers, Basic Vehicle Physics (drivable test vehicle)
**Avoids:** Pitfalls 1 (cell size), 2 (HLOD), 6 (packaged build testing)

### Phase 2: Mass Entity Crowds
**Rationale:** Requires World Partition cell callbacks for spawn triggers. Crowd system is the most complex subsystem with the highest performance risk. Must establish LOD representation patterns early.
**Delivers:** Functional zombie crowd system with 200+ entities, LOD switching, basic wandering AI
**Uses:** Mass Entity, MassAI, MassCrowd, Behavior Trees, Zone Graph
**Implements:** GSD_Crowds plugin with 4-tier LOD representation
**Avoids:** Pitfalls 3 (LOD representation), 4 (velocity randomizer), 8 (RVO/Detour conflict)

### Phase 3: Chaos Vehicle Integration
**Rationale:** Vehicle physics requires stable world to test traversal. Vehicle pool needs cell callbacks from Phase 1. Wheel bone orientation must be correct before vehicle spawning.
**Delivers:** Playable taxi with Chaos physics, vehicle spawning pool, basic AI traffic
**Uses:** ChaosVehicles, ChaosVehiclesCore, PhysicsCore
**Implements:** GSD_Vehicles plugin with spawn pool subsystem
**Avoids:** Pitfall 5 (wheel bone orientation), 9 (synchronous loading via pooling)

### Phase 4: Event System & Data Assets
**Rationale:** Event system enables decoupled communication between plugins. Data Assets provide configuration foundation for all systems. Soft reference patterns must be established here.
**Delivers:** Gameplay Tag event system, Data Asset definitions for all entity types, soft reference loading
**Uses:** GameplayTags, DeveloperSettings, Asset Manager
**Implements:** GSD_Core data asset base classes, GSD_DailyEvents scheduling
**Avoids:** Pitfall 7 (hard references)

### Phase 5: Telemetry & Validation Tools
**Rationale:** Performance monitoring needed once all systems are integrated. Validation tools for CI/CD pipeline. Can run in parallel with gameplay development.
**Delivers:** Async telemetry, performance dashboard, editor validation commandlets
**Uses:** HTTP, Json, UnrealEd (editor-only)
**Implements:** GSD_Telemetry, GSD_ValidationTools

### Phase 6: Plugin Architecture Finalization
**Rationale:** All plugins exist, now formalize dependency structure, ensure clean interfaces, verify shipping builds compile.
**Delivers:** Final plugin structure, Build.cs verification, shipping build validation
**Avoids:** Pitfall 10 (module dependency errors)

### Phase Ordering Rationale

- **Phase 1 first:** World Partition is foundational - all other systems depend on streaming callbacks
- **Phase 2 second:** Crowd LOD patterns are hardest to retrofit, must establish before significant content
- **Phase 3 third:** Vehicle physics needs stable world, pool needs cell callbacks from Phase 1
- **Phase 4 fourth:** Event system and Data Assets enable iteration, but can be added incrementally
- **Phase 5 fifth:** Telemetry observes all systems, adds no dependencies to them
- **Phase 6 last:** Architecture finalization requires all plugins to exist

### Research Flags

Phases likely needing deeper research during planning:
- **Phase 2:** Mass Entity integration is complex; StateTree vs Behavior Tree decision needs evaluation; LOD representation switching patterns need API research
- **Phase 3:** Chaos Vehicle wheel bone orientation requires asset pipeline coordination with blender_gsd; Modular Vehicle System (UE5.5 experimental) evaluation

Phases with standard patterns (skip research-phase):
- **Phase 1:** World Partition well-documented, City Sample provides reference implementation
- **Phase 4:** Gameplay Tags and Data Assets are standard UE patterns with excellent documentation
- **Phase 5:** Telemetry is straightforward HTTP/JSON, no UE-specific complexity
- **Phase 6:** Plugin architecture patterns are well-established in Lyra sample

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | HIGH | Official Epic documentation verified, City Sample reference implementation exists |
| Features | MEDIUM | WebSearch-based with official doc verification where possible; GTA VI comparison speculative |
| Architecture | HIGH | Official Epic documentation + Lyra sample patterns + community best practices consensus |
| Pitfalls | MEDIUM | WebSearch + official doc verified; some patterns inferred from community discussions |

**Overall confidence:** HIGH

### Gaps to Address

- **Chaos Vehicle detailed physics tuning:** Official docs exist but detailed physics behavior needs hands-on testing. Address during Phase 3 planning with test-driven approach.
- **Mass Entity LOD representation switching:** Well-documented concept but implementation details need API research. Schedule `/gsd:research-phase 2` before planning.
- **blender_gsd integration pipeline:** USD/FBX export patterns need coordination. Establish asset contract during Phase 1.
- **UE5.6 Unified Streaming Budget:** New feature, production status claimed but limited real-world validation. Evaluate during Phase 1 if targeting UE5.6+.

## Sources

### Primary (HIGH confidence)
- Epic Games Documentation - World Partition, Mass Entity, Data Layers, Data-Driven Gameplay Elements
- City Sample Project - Epic's reference implementation for open world (4.5km x 4.5km, 10,000+ entities)
- Lyra Sample Game - GameFeatures plugin architecture patterns
- Unreal Engine 5.5 Release Notes - Feature availability, version compatibility

### Secondary (MEDIUM confidence)
- Community Research (2024-2025) - Best practices, performance patterns verified with official docs
- Unreal Engine Forums - Mass Entity discussions, crowd optimization patterns
- CSDN Technical Articles - UE5 World Partition optimization, anti-patterns
- Witcher 4 Tech Demo - 800 NPCs at 60 FPS with UE 5.6

### Tertiary (LOW confidence)
- GTA VI Feature Analysis - Speculative based on rumors, used only for context
- Bilibili Technical Articles - UE5 anti-patterns, needs validation

---
*Research completed: 2026-02-24*
*Ready for roadmap: yes*
