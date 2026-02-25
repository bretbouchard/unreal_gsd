# Pitfalls Research

**Domain:** Unreal Engine 5 Open-World Game Infrastructure
**Researched:** 2026-02-24
**Confidence:** MEDIUM (WebSearch + Official Documentation verified)

---

## Critical Pitfalls

Mistakes that cause rewrites, major performance issues, or architectural failures.

### Pitfall 1: World Partition Grid Cell Size Misconfiguration

**What goes wrong:**
Using default 1km grid cells without adjusting for Charlotte city scale causes either excessive I/O operations (cells too small) or memory pressure/stuttering (cells too large). The default settings are for general open worlds, not dense urban environments.

**Why it happens:**
Developers assume World Partition "just works" with defaults. The default 12800x12800 UU (approx 1km) cell size is a reasonable starting point but Charlotte's density requires tuning.

**Consequences:**
- Too small cells: Constant streaming I/O, disk thrashing, CPU bottleneck
- Too large cells: High memory peaks, visible loading when crossing cell boundaries
- Player perceives "pop-in" or frame drops when driving through city

**How to avoid:**
1. Profile with `stat streaming` early in development
2. For city-scale: Consider 500m-750m cells (6400-9600 UU)
3. Match loading range to expected vehicle speeds
4. Use `wp.Runtime.UpdateStreaming.LocationQuantization` (default 400 units) to control streaming trigger sensitivity
5. Test with worst-case scenarios (fast vehicle traversal)

**Warning signs:**
- Frame time spikes correlate with cell boundary crossings
- Memory usage fluctuates wildly during traversal
- `stat streaming` shows high operation counts

**Phase to address:** Phase 1 (World Partition Foundation)

---

### Pitfall 2: Neglecting HLOD for Unloaded Cells

**What goes wrong:**
Without HLOD (Hierarchical Level of Detail), distant unloaded cells render nothing or cause massive draw calls when trying to show far distances. Players see empty voids beyond loaded cells.

**Why it happens:**
HLOD requires explicit setup and generation. It's not automatic. Developers skip it thinking World Partition handles everything.

**Consequences:**
- Distant buildings/terrain vanish completely
- Or: Attempting to load distant cells destroys performance
- Draw call explosion when viewing large areas
- Broken immersion in open-world context

**How to avoid:**
1. Enable HLOD in World Settings immediately after WP setup
2. Generate HLOD via Window > HLOD Outliner > Generate HLOD
3. Use 3 LOD levels for city-scale projects
4. Set appropriate HLOD transition distances
5. Regenerate HLOD after major world changes

**Warning signs:**
- Distant areas appear empty when player is elevated
- High draw call counts in profiler
- Players report "the city disappears" when flying/viewing from distance

**Phase to address:** Phase 1 (World Partition Foundation)

---

### Pitfall 3: Mass Entity Without LOD-Based Representation Switching

**What goes wrong:**
Rendering 200+ zombies as full skeletal mesh actors without LOD-based visualization switching destroys CPU performance. UE renders skeletal meshes on CPU, not GPU, and there's no instanced skeletal mesh support like static meshes have.

**Why it happens:**
Developers prototype with simple actor spawning, which works for 10-20 entities. Scaling to 200+ without architectural changes causes cascading performance failure.

**Consequences:**
- Frame rate drops to single digits with 200 zombies
- CPU-bound performance that GPU upgrades cannot fix
- Complete architectural rewrite required to fix

**How to avoid:**
Use 4-tier LOD representation system:
1. **0-50m:** Full Blueprint actor with skeletal mesh, full AI
2. **50-200m:** Simplified actor with reduced animation/AI update rate
3. **200-500m:** Instanced Static Mesh (ISM) with vertex animation texture
4. **500m+:** Culled or single-point representation

**Warning signs:**
- Performance tanks as zombie count increases
- Profiler shows skeletal mesh rendering dominating CPU time
- Animation update costs are disproportionate to visual benefit

**Phase to address:** Phase 2 (Mass Entity Crowds)

---

### Pitfall 4: Missing Velocity Randomizer in Mass Entity Config

**What goes wrong:**
Mass Entity zombies spawn but never move. The entity exists but has no velocity data, so movement processors skip it.

**Why it happens:**
Mass Entity requires explicit fragment composition. Forgetting to add `Velocity Randomizer` trait means entities lack velocity fragments.

**Consequences:**
- Zombies spawn correctly but remain stationary
- Debugging leads down wrong paths (AI, pathfinding)
- Wasted time before discovering missing trait

**How to avoid:**
1. Use Mass Entity Config assets with complete trait sets
2. Always include: Transform + Velocity + LOD + Visualization traits
3. Create template configs and derive from them
4. Test movement immediately after creating new entity types

**Warning signs:**
- Entities spawn but don't respond to movement logic
- Processors execute but entities remain at spawn location
- Query logs show entities exist but lack expected fragments

**Phase to address:** Phase 2 (Mass Entity Crowds)

---

### Pitfall 5: Chaos Vehicle Wheel Bone Orientation

**What goes wrong:**
Wheels have no collision, fall through ground, or collide at wrong angles. Vehicle is uncontrollable or physics explodes.

**Why it happens:**
Chaos Vehicles require specific bone orientation in the skeletal mesh:
- X-axis (red) pointing forward
- Y-axis (green) pointing left
- Z-axis (blue) pointing up

If bones are oriented differently in the 3D modeling software, Chaos interprets wheel collision incorrectly.

**Consequences:**
- Vehicles fall through terrain
- Wheels collide with invisible walls
- Erratic suspension behavior
- Hours of debugging physics settings when the problem is in asset import

**How to avoid:**
1. Verify bone orientation in 3D software BEFORE import
2. Use `Show Collision` console command to visualize wheel collision shapes
3. Set wheel collision to "No Collision" in skeletal mesh (collision handled by wheel blueprint)
4. Test with simple flat ground first before complex terrain

**Warning signs:**
- Vehicle immediately falls through floor on spawn
- Wheels appear perpendicular to expected orientation
- Collision debug shows wheel shapes at wrong angles

**Phase to address:** Phase 3 (Chaos Vehicle Integration)

---

### Pitfall 6: Not Testing World Partition in Packaged Builds

**What goes wrong:**
Streaming works perfectly in PIE (Play In Editor) but fails or behaves differently in packaged builds. Cell loading, memory usage, and performance all differ.

**Why it happens:**
PIE uses editor's asset loading mechanisms. Packaged builds use cooked assets with different streaming paths. This is documented but often forgotten.

**Consequences:**
- Last-minute discovery of streaming failures
- Performance targets met in editor but not in build
- Shipping build has different memory characteristics
- Long iteration cycles (cook, test, fix, re-cook)

**How to avoid:**
1. Establish regular packaged build testing cadence
2. Create automated performance test that runs on packaged builds
3. Use `stat streaming` and `wp.Runtime.ToggleDebugDraw` in builds
4. Test on target platforms, not just development machines
5. Set up CI pipeline that validates streaming in cooked builds

**Warning signs:**
- "It works in editor" becomes common phrase
- Performance only validated in PIE
- No regular packaged build testing

**Phase to address:** Phase 1 (World Partition Foundation) - establish testing discipline early

---

### Pitfall 7: Hard References Instead of Soft References in Data Assets

**What goes wrong:**
Primary Data Assets with hard references to large assets (meshes, textures, sounds) cause those assets to load into memory immediately, even if never used. Memory bloat accumulates as project grows.

**Why it happens:**
Hard references (`UObject*`) are the default and easiest to use. Soft references (`TSoftObjectPtr`) require explicit loading code. Convenience wins over correctness.

**Consequences:**
- Multi-gigabyte memory footprint at startup
- Long initial load times
- Cannot fit within console memory budgets
- Requires systematic refactoring to fix

**How to avoid:**
1. Use `TSoftObjectPtr` for all large asset references
2. Load via `UAssetManager::LoadPrimaryAsset` or async methods
3. Never use synchronous loading in constructors
4. Establish code review rule: hard references need justification
5. Profile memory early and often

**Warning signs:**
- Memory usage grows linearly with Data Asset count
- Startup time increases as assets are added
- Profiler shows assets loaded that aren't currently needed

**Phase to address:** Phase 4 (Event System & Data Assets)

---

### Pitfall 8: Running Both RVO and Detour Crowd Simultaneously

**What goes wrong:**
NPCs exhibit erratic behavior, collide with each other, or have inconsistent avoidance. Performance degrades unexpectedly.

**Why it happens:**
Unreal has two built-in crowd avoidance systems:
- RVO (via CharacterMovementComponent)
- Detour Crowd Manager

Both active simultaneously = conflict. This is documented but easy to miss.

**Consequences:**
- NPCs jitter or collide
- Unpredictable avoidance behavior
- Wasted CPU on duplicate calculations
- Difficult to debug because both systems are "working"

**How to avoid:**
1. Choose ONE system at project start
2. RVO: Better for small-scale, high-quality avoidance
3. Detour Crowd: Better for large-scale crowds
4. Disable the unused system in project settings
5. For Mass Entity crowds, use Mass Avoidance (separate system)

**Warning signs:**
- NPCs seem to fight against avoidance
- Inconsistent behavior between different NPCs
- Avoidance quality varies unpredictably

**Phase to address:** Phase 2 (Mass Entity Crowds)

---

### Pitfall 9: Synchronous Loading During Gameplay

**What goes wrong:**
Frame hitches when spawning vehicles, loading events, or transitioning areas. The game freezes momentarily, breaking immersion.

**Why it happens:**
`LoadClass` or `StaticLoadObject` in gameplay code causes synchronous disk I/O. It "works" during development but hitches in production.

**Consequences:**
- Unpredictable frame drops
- Players perceive game as "janky"
- Hitches correlate with gameplay events (bad UX)
- Cannot achieve stable frame rate

**How to avoid:**
1. Pre-load all needed assets during loading screens
2. Use object pooling for frequently spawned objects
3. Use async loading (`RequestAsyncLoad`) for runtime needs
4. Profile with `stat streaming` and `stat loadtime`
5. Establish zero-tolerance policy for sync loading in gameplay

**Warning signs:**
- Hitches occur when specific events trigger
- Profiler shows `WaitForAsyncLoad` or similar in call stack
- Frame time spikes correlate with asset loading

**Phase to address:** Phase 5 (Vehicle Spawning Pool)

---

### Pitfall 10: Plugin Module Dependency Declaration Errors

**What goes wrong:**
Plugin compiles in development but fails in shipping build, or crashes on load with cryptic linker errors.

**Why it happens:**
Build.cs files don't declare all required dependencies. Development builds link against editor modules that shipping builds exclude. Missing `#if WITH_EDITOR` guards.

**Consequences:**
- Late-stage build failures
- Shipping build crashes
- Confusing LNK2019 errors
- Delayed release while hunting missing dependencies

**How to avoid:**
1. Declare ALL dependencies in Build.cs:
   ```cpp
   PublicDependencyModuleNames.AddRange(new string[] {
       "Core", "CoreUObject", "Engine", "InputCore"
   });
   ```
2. Editor-only code requires:
   ```cpp
   if (Target.bBuildEditor) {
       PrivateDependencyModuleNames.Add("UnrealEd");
   }
   ```
3. Guard editor code with `#if WITH_EDITOR`
4. Test shipping builds regularly, not just development

**Warning signs:**
- "Works in editor, crashes in build"
- Development builds pass CI, shipping builds fail
- Missing module errors only in certain configurations

**Phase to address:** Phase 6 (Plugin Architecture)

---

## Technical Debt Patterns

Shortcuts that seem reasonable but create long-term problems.

| Shortcut | Immediate Benefit | Long-term Cost | When Acceptable |
|----------|-------------------|----------------|-----------------|
| Skip HLOD generation | Saves setup time | Draw call explosion, cannot show distant terrain | Never for open world |
| Use hard refs everywhere | Faster coding | Memory bloat, long load times | Prototype only, remove before production |
| Spawn/destroy vehicles | Simpler logic | Frame hitches, GC pressure | Never - use pooling from start |
| Single streaming source | Simpler setup | Pop-in when player moves fast | Single-player, slow movement only |
| Skip packaged build testing | Faster iteration | Late discovery of streaming issues | Never - establish regular testing |
| All zombies as full actors | Easier implementation | CPU-bound, 20 entity limit | Prototype only |
| Monolithic Data Assets | Fewer files | Memory bloat, version control conflicts | Never - split by entity type |
| Direct Chaos setup without templates | Faster initial setup | No reuse, inconsistent behavior | Prototype only |

---

## Integration Gotchas

Common mistakes when connecting systems together.

| Integration | Common Mistake | Correct Approach |
|-------------|----------------|------------------|
| Mass Entity + World Partition | Spawn entities in unloaded cells | Check cell loaded state before spawning; use streaming source proximity |
| Chaos Vehicles + Streaming | Vehicles don't have streaming source component | Add `WorldPartitionStreamingSourceComponent` to vehicle blueprint |
| Data Assets + Event System | Hard-code event timing in code | Define event schedules in Data Assets for designer control |
| HLOD + Data Layers | HLOD generated for wrong layer combination | Generate HLOD per relevant Data Layer configuration |
| Mass Entity + Smart Objects | Both systems trying to control same NPCs | Use one system for crowd logic, other for interaction points |
| Vehicle Pool + Physics | Returning vehicle to pool without resetting physics | Clear velocity, reset transform, disable collision before returning |

---

## Performance Traps

Patterns that work at small scale but fail as usage grows.

| Trap | Symptoms | Prevention | When It Breaks |
|------|----------|------------|----------------|
| Full skeletal mesh zombies | FPS drops as zombie count increases | Use LOD-based representation switching | >20 full skeletal zombies |
| Default grid cell size | Streaming hitches on fast travel | Tune cell size for expected traversal speeds | City-scale, vehicles |
| No object pooling for vehicles | Frame hitch on every vehicle spawn | Pre-spawn pool during loading | Any spawn rate > 0.1/sec |
| Single streaming source | Pop-in when driving fast | Add streaming source to vehicle | Speed > streaming range / load time |
| Actor-based crowd AI | CPU saturation, unresponsive game | Use Mass Entity with simple AI | >50 AI agents |
| No memory budget | Gradual performance degradation | Set per-cell memory limits, monitor usage | Varies by platform |
| Synchronous asset loading | Frame hitches on event triggers | Pre-load or async load all runtime assets | First occurrence |

---

## Security/Stability Mistakes

Domain-specific issues beyond general game development.

| Mistake | Risk | Prevention |
|---------|------|------------|
| Infinite loop in Mass Processor | Game freeze, requires kill | Use execution time limits; profile processors |
| Unbounded entity spawning | Memory exhaustion | Implement spawn limits based on LOD budget |
| Streaming source race condition | Cells load/unload rapidly | Debounce streaming source updates |
| Asset load during GC | Crash or corruption | Defer asset loading until GC completes |
| Physics state mismatch after pooling | Vehicle behaves erratically | Full physics state reset on pool return |

---

## "Looks Done But Isn't" Checklist

Things that appear complete but are missing critical pieces.

- [ ] **World Partition:** Often missing HLOD - verify with elevated view and `stat streaming`
- [ ] **Mass Entity Crowds:** Often missing LOD representation switching - verify 200+ entities at 60 FPS
- [ ] **Vehicle Physics:** Often missing proper wheel bone orientation - verify with `Show Collision`
- [ ] **Data Assets:** Often using hard references - verify memory usage vs. active content
- [ ] **Vehicle Pooling:** Often missing physics reset - verify returned vehicles don't retain velocity
- [ ] **Streaming:** Often only tested in PIE - verify in packaged build on target platform
- [ ] **Event System:** Often not deterministic - verify same seed produces same results
- [ ] **Plugin:** Often missing shipping build dependencies - verify development AND shipping builds compile

---

## Recovery Strategies

When pitfalls occur despite prevention, how to recover.

| Pitfall | Recovery Cost | Recovery Steps |
|---------|---------------|----------------|
| Wrong grid cell size | MEDIUM | Adjust settings, regenerate HLOD, retest traversal |
| Missing HLOD | LOW | Generate HLOD, verify distant rendering |
| No LOD representation switching | HIGH | Requires architectural change; implement 4-tier system |
| Wrong wheel bone orientation | MEDIUM | Re-export from 3D software with correct orientation |
| Hard references everywhere | HIGH | Systematic refactoring to soft references + async loading |
| No vehicle pooling | MEDIUM | Implement pool, update spawn points to use pool |
| Missing module dependencies | LOW | Add to Build.cs, verify all build configurations |
| RVO + Detour both active | LOW | Disable one system in project settings |

---

## Pitfall-to-Phase Mapping

How roadmap phases should address these pitfalls.

| Pitfall | Prevention Phase | Verification |
|---------|------------------|--------------|
| Grid cell size misconfiguration | Phase 1 | Profile traversal at target speeds |
| Missing HLOD | Phase 1 | Verify distant rendering from elevated position |
| No LOD representation switching | Phase 2 | Benchmark 200+ entities at 60 FPS |
| Missing velocity randomizer | Phase 2 | Test entity movement immediately |
| Wrong wheel bone orientation | Phase 3 | Visualize collision with `Show Collision` |
| Not testing in packaged builds | Phase 1 | Establish CI with packaged build tests |
| Hard references | Phase 4 | Profile memory vs. active content |
| RVO + Detour conflict | Phase 2 | Verify single system active |
| Synchronous loading | Phase 5 | Profile frame time during spawns |
| Module dependency errors | Phase 6 | Compile and run shipping build |

---

## Sources

**Official Documentation (HIGH Confidence):**
- Epic Developer Community: World Partition Documentation (UE 5.5/5.6)
- Epic Developer Community: Mass Entity Overview
- Epic Developer Community: Asset Management in Unreal Engine
- Epic Developer Community: City Sample Project Documentation

**Community Resources (MEDIUM Confidence):**
- Unreal Engine Forums: Mass Entity discussions (2024-2025)
- CSDN Technical Articles: UE5 World Partition optimization
- LinkedIn Posts: UE5 Best Practices (2025)
- Bilibili Technical Articles: UE5 Anti-Patterns

**Verified Patterns (MEDIUM Confidence):**
- Days Gone crowd AI patterns (Bend Studios post-mortem)
- Witcher 4 Tech Demo: 800 NPCs at 60 FPS (UE 5.6)
- City Sample performance requirements and optimization strategies

**Console Commands for Verification:**
```
stat streaming              - View streaming statistics
wp.Runtime.ToggleDebugDraw  - Visualize loaded/unloaded cells (green/red)
Show Collision              - Visualize collision shapes
stat unit                   - Check game thread performance
stat fps                    - Monitor framerate
stat memory                 - Memory usage breakdown
```

---

*Pitfalls research for: Unreal Engine 5 Open-World Game Infrastructure*
*Researched: 2026-02-24*
*Confidence: MEDIUM (WebSearch + Official Documentation verified)*
