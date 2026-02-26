# Phase 6: Crowd Core Systems - Research

**Researched:** 2026-02-25
**Domain:** Unreal Engine 5 Mass Entity Framework, Crowd Simulation, LOD-based Representation
**Confidence:** MEDIUM

## Summary

Phase 6 implements crowd simulation using UE5's Mass Entity framework to spawn 200+ zombie entities with LOD-based visual representation switching. Mass Entity is a data-oriented Entity Component System (ECS) framework designed for simulating tens of thousands of entities efficiently.

The core challenge is configuring Mass Entity with proper LOD representation that switches between full Actor (close), Simplified Actor (medium), Instanced Static Mesh (far), and culled (very far). This requires understanding Fragments (data), Processors (logic), Traits (combined functionality), and the Mass Representation subsystem.

**Primary recommendation:** Use Mass Entity with Mass Representation Actor for LOD switching, configure 4 LOD levels with distance-based thresholds, and leverage ISM for distant entities. Always add Velocity Randomizer trait to prevent synchronized movement.

## Standard Stack

The established libraries/tools for crowd simulation in UE5:

### Core
| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| MassEntity | UE5.0+ | ECS framework for crowds | Official Epic solution, handles 10k+ entities |
| MassRepresentation | UE5.0+ | Visual LOD switching | Built-in Actor/ISM switching |
| MassSpawner | UE5.0+ | Entity spawning from Data Assets | Integrates with Mass Entity Definition |
| MassLOD | UE5.0+ | Distance-based LOD calculations | Automatic significance calculation |

### Supporting
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| MassAI | UE5.1+ | AI navigation and behavior | Phase 7 (Crowd AI & Navigation) |
| ZoneGraph | UE5.0+ | Spline-based navigation lanes | Phase 7 for entity movement |
| StateTree | UE5.0+ | Behavior control | Phase 7 for complex NPC behavior |
| SmartObjects | UE5.0+ | Interaction points | Phase 7 for cover/ambush points |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Mass Entity | Niagara Crowd | Niagara better for pure visuals, Mass Entity for game logic |
| Mass Entity | Traditional AI Controllers | Controllers 10x more expensive per entity |
| ISM | HISM (Hierarchical ISM) | HISM supports culling per section but more complex setup |

**Build.cs Dependencies:**
```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core",
    "CoreUObject",
    "Engine",
    "GSD_Core",
    "GSD_CityStreaming",
    "MassEntity",
    "MassRepresentation",
    "MassSpawner",
    "MassLOD",
    "MassCommon"
});

PrivateDependencyModuleNames.AddRange(new string[] {
    "MassAI"  // For future navigation support
});
```

## Architecture Patterns

### Recommended Plugin Structure
```
Plugins/GSD_Crowds/
├── Source/
│   └── GSD_Crowds/
│       ├── Public/
│       │   ├── GSD_Crowds.h              # Plugin module
│       │   ├── Fragments/                 # Mass Entity Fragments
│       │   │   ├── GSDCrowdFragment.h    # Custom crowd data
│       │   │   └── GSDZombieStateFragment.h
│       │   ├── Processors/                # Mass Entity Processors
│       │   │   ├── GSDCrowdLODProcessor.h
│       │   │   └── GSDZombieBehaviorProcessor.h
│       │   ├── Subsystems/
│       │   │   └── GSDCrowdManagerSubsystem.h
│       │   └── Actors/
│       │       └── GSDCrowdTestbedActor.h
│       ├── Private/
│       │   └── [implementations]
│       └── GSD_Crowds.Build.cs
└── Content/
    └── GSD_Crowds/
        ├── EntityConfigs/
        │   ├── GSDZombieEntityConfig.uasset
        │   └── GSDZombieLODConfig.uasset
        └── Blueprints/
            └── BP_GSDZombieCrowdManager.uasset
```

### Pattern 1: Mass Entity Definition Setup

**What:** Create a Mass Entity Config Asset that defines what fragments and processors an entity has
**When to use:** Every crowd entity type needs an Entity Definition

**Example:**
```cpp
// Source: UE5 Mass Entity Documentation
// https://dev.epicgames.com/documentation/en-us/unreal-engine/mass-entity-in-unreal-engine

// Fragment - Atomic data unit
USTRUCT()
struct GSD_CROWDS_API FGSDZombieStateFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY()
    float Health = 100.0f;

    UPROPERTY()
    float MovementSpeed = 150.0f;

    UPROPERTY()
    uint8 bIsAggressive : 1;
};

// Processor - Stateless logic
UCLASS()
class GSD_CROWDS_API UGSDZombieBehaviorProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UGSDZombieBehaviorProcessor()
    {
        // Required fragments for processing
        ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::SyncWorld;
        ProcessingPhase = EMassProcessingPhase::PrePhysics;
    }

    virtual void ConfigureQueries() override
    {
        EntityQuery.AddRequirement<FGSDZombieStateFragment>(EMassFragmentAccess::ReadWrite);
        EntityQuery.AddRequirement<FDataFragment_Transform>(EMassFragmentAccess::ReadOnly);
        EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadOnly);
    }

    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override
    {
        EntityQuery.ForEachEntityChunk(EntityManager, Context,
            [&](FMassExecutionContext& Context)
            {
                const int32 NumEntities = Context.GetNumEntities();
                auto ZombieStates = Context.GetMutableFragmentView<FGSDZombieStateFragment>();
                const auto& Transforms = Context.GetFragmentView<FDataFragment_Transform>();

                for (int32 i = 0; i < NumEntities; ++i)
                {
                    // Process each zombie
                    ZombieStates[i].Health -= 0.1f; // Example: decay
                }
            });
    }

private:
    FMassEntityQuery EntityQuery;
};
```

### Pattern 2: Mass Representation LOD Switching

**What:** Configure visual representation that switches based on distance
**When to use:** All crowd entities that need LOD optimization

**Example:**
```cpp
// Source: UE5 Mass Representation Documentation
// https://dev.epicgames.com/documentation/en-us/unreal-engine/mass-representation-in-unreal-engine

// LOD significance values (0.0 = High/Close, 3.0 = Off/Culled)
// High Actor:    0.0 - 0.5
// Low Actor:     0.5 - 1.5
// ISM:           1.5 - 2.5
// Culled:        2.5 - 3.0

// In Mass Entity Config Asset:
// - Add MassRepresentationFragment
// - Add MassRepresentationLODFragment
// - Set HighRepresentation (full actor)
// - Set LowRepresentation (simplified actor)
// - Set ISMRepresentation (instanced mesh)

// Processor to calculate LOD significance
UCLASS()
class UGSDCrowdLODProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UGSDCrowdLODProcessor()
    {
        ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::SyncWorld);
        ProcessingPhase = EMassProcessingPhase::PrePhysics;
    }

    virtual void ConfigureQueries() override
    {
        EntityQuery.AddRequirement<FMassRepresentationLODFragment>(EMassFragmentAccess::ReadWrite);
        EntityQuery.AddRequirement<FDataFragment_Transform>(EMassFragmentAccess::ReadOnly);
    }

    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override
    {
        // Get viewer location (player camera)
        const FVector ViewerLocation = GetViewerLocation();

        EntityQuery.ForEachEntityChunk(EntityManager, Context,
            [&](FMassExecutionContext& Context)
            {
                auto LODFragments = Context.GetMutableFragmentView<FMassRepresentationLODFragment>();
                const auto& Transforms = Context.GetFragmentView<FDataFragment_Transform>();

                for (int32 i = 0; i < Context.GetNumEntities(); ++i)
                {
                    const float Distance = FVector::Dist(Transforms[i].GetTransform().GetLocation(), ViewerLocation);
                    // Calculate LOD significance based on distance
                    LODFragments[i].LODSignificance = CalculateLODSignificance(Distance);
                }
            });
    }

private:
    float CalculateLODSignificance(float Distance) const
    {
        // 2000 units = High Actor
        // 5000 units = Low Actor
        // 10000 units = ISM
        // 20000 units = Culled
        if (Distance < 2000.0f) return 0.0f;
        if (Distance < 5000.0f) return 0.75f;
        if (Distance < 10000.0f) return 1.75f;
        if (Distance < 20000.0f) return 2.5f;
        return 3.0f;
    }

    FMassEntityQuery EntityQuery;
};
```

### Pattern 3: Mass Spawner with Data Assets

**What:** Spawn entities from configuration data assets
**When to use:** Spawning initial crowd population or runtime spawning

**Example:**
```cpp
// Source: UE5 Mass Spawner Documentation
// https://dev.epicgames.com/documentation/en-us/unreal-engine/mass-entity-in-unreal-engine

// Spawn entities programmatically
void AGSDCrowdManagerSubsystem::SpawnCrowdEntities(int32 Count, const FVector& Center, float Radius)
{
    UWorld* World = GetWorld();
    if (!World) return;

    UMassEntitySubsystem* MassSubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    if (!MassSubsystem) return;

    // Get the entity config
    UMassEntityConfigAsset* ZombieConfig = LoadObject<UMassEntityConfigAsset>(
        nullptr, TEXT("/GSD_Crowds/EntityConfigs/GSDZombieEntityConfig.GSDZombieEntityConfig"));

    if (!ZombieConfig) return;

    // Create spawn locations
    TArray<FTransform> SpawnTransforms;
    for (int32 i = 0; i < Count; ++i)
    {
        FTransform SpawnTransform;
        SpawnTransform.SetLocation(Center + FMath::VRand() * FMath::RandRange(0.0f, Radius));
        SpawnTransform.SetRotation(FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f).Quaternion());
        SpawnTransforms.Add(SpawnTransform);
    }

    // Spawn using Mass Spawner or directly through subsystem
    FMassSpawnContext SpawnContext;
    SpawnContext.EntityConfig = ZombieConfig;
    SpawnContext.SpawnTransforms = SpawnTransforms;

    // MassSubsystem handles the spawning
    MassSubsystem->SpawnEntities(SpawnContext);
}
```

### Anti-Patterns to Avoid

- **Using raw AActor for 200+ entities:** Each Actor has significant overhead; Mass Entity uses lightweight fragments
- **Processing entities in Tick():** Use Mass Processors with proper execution groups instead
- **Ignoring Velocity Randomizer:** Entities will move in perfect synchronization without it
- **Destroying entities during processor execution:** Must use Defer() to safely destroy entities
- **Storing pointers to Fragments:** Fragments can move in memory; use FMassEntityHandle instead

## Don't Hand-Roll

Problems that look simple but have existing solutions:

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Entity pooling | Custom actor pooling | Mass Entity Archetype pooling | Built-in memory efficiency |
| LOD switching | Distance checks in Tick | Mass Representation subsystem | Multithreaded, optimized |
| Spawn distribution | Random point generation | Mass Spawner with distribution types | Supports multiple distribution algorithms |
| Velocity variation | Random velocity per entity | Velocity Randomizer trait | Handles interpolation, de-synchronization |
| Thread-safe entity access | Mutex/locks | FMassExecutionContext with proper access flags | Lock-free by design |

**Key insight:** Mass Entity is designed from the ground up for high-performance crowd simulation. Building custom solutions defeats the purpose and will be slower.

## Common Pitfalls

### Pitfall 1: Entities Not Moving (Velocity Randomizer Missing)

**What goes wrong:** All crowd entities move in perfect synchronization, looking unnatural
**Why it happens:** Without Velocity Randomizer trait, entities use identical movement parameters
**How to avoid:** Always add Velocity Randomizer to Mass Entity Config Asset
**Warning signs:** All zombies walking at exactly the same speed, turning simultaneously

```cpp
// CRITICAL: Add to Mass Entity Config Asset
// Trait: MassVelocityRandomizer
// - BaseVelocity: 150.0
// - VelocityRandomRange: 0.2 (20% variation)
```

### Pitfall 2: Entity Destruction During Processing

**What goes wrong:** Crash when destroying entities during processor execution
**Why it happens:** Entity array can be modified during iteration, invalidating indices
**How to avoid:** Use Defer() to queue destruction, process after iteration completes
**Warning signs:** Random crashes in FMassEntityManager

```cpp
// WRONG:
for (int32 i = 0; i < NumEntities; ++i)
{
    if (ShouldDestroy(Entities[i]))
    {
        EntityManager->DestroyEntity(Entities[i]); // CRASH RISK
    }
}

// CORRECT:
TArray<FMassEntityHandle> EntitiesToDestroy;
for (int32 i = 0; i < NumEntities; ++i)
{
    if (ShouldDestroy(Entities[i]))
    {
        EntitiesToDestroy.Add(Entities[i]);
    }
}
// Defer destruction to safe point
EntityManager->Defer().DestroyEntities(EntitiesToDestroy);
```

### Pitfall 3: Incorrect Fragment Access Flags

**What goes wrong:** Race conditions or incorrect data when multiple processors access same fragments
**Why it happens:** Using ReadOnly when you need ReadWrite, or vice versa
**How to avoid:** Carefully specify access requirements in ConfigureQueries()
**Warning signs:** Intermittent data corruption, non-deterministic behavior

```cpp
virtual void ConfigureQueries() override
{
    // Be explicit about access requirements
    EntityQuery.AddRequirement<FGSDZombieStateFragment>(EMassFragmentAccess::ReadWrite); // Modifying
    EntityQuery.AddRequirement<FDataFragment_Transform>(EMassFragmentAccess::ReadOnly);  // Only reading
    EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::None);  // Existence check only
}
```

### Pitfall 4: Ignoring Execution Order

**What goes wrong:** Processors run in undefined order, causing inconsistent behavior
**Why it happens:** Not specifying execution groups and dependencies
**How to avoid:** Always set ExecutionOrder with proper groups
**Warning signs:** Behavior changes between runs, physics glitches

```cpp
UGSDZombieBehaviorProcessor()
{
    // Define when this processor runs
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::SyncWorld;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
    ProcessingPhase = EMassProcessingPhase::PrePhysics;
}
```

### Pitfall 5: GC Issues with UOBJECT in Fragments

**What goes wrong:** Garbage collection crashes or memory leaks with UOBJECT pointers in fragments
**Why it happens:** Fragments are not UOBJECTs, cannot hold strong references
**How to avoid:** Use TObjectPtr with weak references, or store indices/references instead
**Warning signs:** Random GC-related crashes during gameplay

```cpp
// WRONG:
USTRUCT()
struct FGSDZombieFragment : public FMassFragment
{
    UPROPERTY() // Fragments don't support UPROPERTY
    TObjectPtr<UObject> Data; // Will not be tracked properly
};

// CORRECT:
USTRUCT()
struct FGSDZombieFragment : public FMassFragment
{
    GENERATED_BODY()

    // Store raw data or indices, not UObject pointers
    float Health = 100.0f;
    int32 TargetIndex = INDEX_NONE;
};
```

## Code Examples

### Complete Testbed Actor Pattern

```cpp
// Based on GSDVehicleTestbedActor pattern
// Source: /Plugins/GSD_Vehicles/Source/GSD_Vehicles/Public/Actors/GSDVehicleTestbedActor.h

UCLASS(BlueprintType, Blueprintable, Category = "GSD|Crowds")
class GSD_CROWDS_API AGSDCrowdTestbedActor : public AActor
{
    GENERATED_BODY()

public:
    AGSDCrowdTestbedActor();

    //-- Configuration --
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testbed")
    TObjectPtr<UMassEntityConfigAsset> ZombieEntityConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testbed", meta = (ClampMin = "1", ClampMax = "500"))
    int32 NumEntitiesToSpawn = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testbed")
    FVector SpawnAreaSize = FVector(10000.0f, 10000.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testbed")
    float TargetFPS = 60.0f;

    //-- Actions --
    UFUNCTION(BlueprintCallable, Category = "Testbed")
    void SpawnTestEntities();

    UFUNCTION(BlueprintCallable, Category = "Testbed")
    void DespawnAllEntities();

    //-- Statistics --
    UFUNCTION(BlueprintPure, Category = "Testbed")
    float GetCurrentFPS() const { return CurrentFPS; }

    UFUNCTION(BlueprintPure, Category = "Testbed")
    int32 GetSpawnedEntityCount() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    float CurrentFPS = 0.0f;
    TArray<float> FrameTimeHistory;
    int32 MaxFrameTimeHistory = 60;
};
```

### World Subsystem Pattern

```cpp
// Based on GSDVehicleSpawnerSubsystem pattern
// Source: /Plugins/GSD_Vehicles/Source/GSD_Vehicles/Public/Subsystems/GSDVehicleSpawnerSubsystem.h

UCLASS()
class GSD_CROWDS_API UGSDCrowdManagerSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    //-- Spawning --
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds")
    void SpawnEntities(int32 Count, FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds")
    void DespawnAllEntities();

    //-- Queries --
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds")
    int32 GetActiveEntityCount() const;

protected:
    virtual bool ShouldCreateSubsystem(UWorld* World) const override;

private:
    UPROPERTY()
    TArray<FMassEntityHandle> SpawnedEntityHandles;
};
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| AIController per NPC | Mass Entity Processors | UE5.0 | 10x more entities possible |
| Individual Actor spawning | Mass Spawner with archetypes | UE5.0 | Batched spawning, better memory |
| Manual LOD distance checks | Mass Representation LOD | UE5.0 | Multithreaded LOD calculation |
| Behavior Trees for all | StateTree + Mass AI | UE5.1 | Better performance for simple AI |
| NavMesh for crowds | ZoneGraph for lane following | UE5.0 | Lighter weight navigation |

**Deprecated/outdated:**
- AITypes based crowd controllers: Use Mass AI Processors instead
- Blueprint-heavy crowd logic: Move to C++ Processors for performance
- Niagara for game-logic crowds: Only use for pure visual crowds without gameplay interaction

## Open Questions

Things that couldn't be fully resolved:

1. **ISM Vertex Animation Support**
   - What we know: ISM supports vertex animation for simple animations
   - What's unclear: Exact workflow for zombie walk cycles in ISM representation
   - Recommendation: Test with simple 4-bone skeleton first, verify vertex animation baking works

2. **World Partition Integration**
   - What we know: Mass Entity can work with World Partition
   - What's unclear: How entity spawning/despawning interacts with streaming cells
   - Recommendation: Spawn entities only in loaded cells, use cell callbacks to manage entity lifecycle

3. **Network Replication (v2)**
   - What we know: Mass Entity has limited network support
   - What's unclear: Replication strategy for single-player focused design
   - Recommendation: Defer to Phase 10+, not relevant for v1 single-player

4. **Debug Visualization**
   - What we know: Mass Entity has debug draw capabilities
   - What's unclear: Best practices for in-editor crowd debugging
   - Recommendation: Implement console commands for entity count, LOD distribution, FPS metrics

## Sources

### Primary (HIGH confidence)
- Epic Games Documentation: Mass Entity in Unreal Engine - https://dev.epicgames.com/documentation/en-us/unreal-engine/mass-entity-in-unreal-engine
- Epic Games Documentation: Mass Representation in Unreal Engine - https://dev.epicgames.com/documentation/en-us/unreal-engine/mass-representation-in-unreal-engine
- UE5 Source Code: MassEntity module (verified API patterns)

### Secondary (MEDIUM confidence)
- Unreal Engine Forums: Mass Entity discussions (verified against documentation)
- Community tutorials: Mass Entity setup guides (cross-referenced with official docs)

### Tertiary (LOW confidence)
- Web search results for Mass Entity best practices (unverified, flagged for validation)

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH - Based on official UE5 documentation and existing plugin patterns
- Architecture: MEDIUM - Based on documentation, needs validation with actual implementation
- Pitfalls: HIGH - Well-documented common issues with clear solutions
- LOD switching: MEDIUM - Concept clear, exact threshold tuning requires testing
- Performance: MEDIUM - Expected 200+ entities achievable, exact numbers need validation

**Research date:** 2026-02-25
**Valid until:** 30 days (UE5 API stable, but check for 5.4+ updates)
