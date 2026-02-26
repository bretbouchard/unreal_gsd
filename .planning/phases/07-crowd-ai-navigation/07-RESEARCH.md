# Phase 7: Crowd AI & Navigation - Research

**Researched:** 2026-02-26
**Domain:** Unreal Engine 5 MassAI, ZoneGraph, SmartObjects, StateTree, Behavior Trees, AI Perception
**Confidence:** MEDIUM

## Summary

Phase 7 extends the Phase 6 crowd foundation with intelligent navigation and behavior systems. The core challenge is implementing ZoneGraph-based navigation for mass entities while adding Behavior Tree-driven hero NPCs with AI Perception for gameplay-relevant interactions.

Key technologies: **ZoneGraph** provides lightweight lane-based navigation for crowds; **SmartObjects** define interaction points like cover and ambush; **StateTree** controls crowd behavior states; **Behavior Trees + AI Perception** handle hero NPC decision-making. The Velocity Randomizer trait from Phase 6 is critical for preventing synchronized movement.

**Primary recommendation:** Use ZoneGraph with MassMovement for crowd navigation (despite experimental status - it is the intended UE5 solution), StateTree for crowd behavior state management, Behavior Trees + AI Perception for hero NPCs, and SmartObjects for world interaction points. Extend existing GSD_Crowds plugin with MassAI module integration.

## Standard Stack

The established libraries/tools for crowd AI in UE5:

### Core
| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| MassAI | UE5.1+ | AI navigation and behavior | Official Epic solution for Mass Entity AI |
| ZoneGraph | UE5.0+ | Spline-based navigation lanes | Lightweight alternative to NavMesh for crowds |
| StateTree | UE5.0+ | Hierarchical behavior control | Combines BT selectors with state machine transitions |
| SmartObjects | UE5.0+ | World interaction points | Reservation-based activity system |
| AIModule | UE5.0+ | Behavior Trees and AI Perception | Proven AI decision-making framework |

### Supporting
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| NavigationSystem | UE5.0+ | NavMesh for hero NPCs | Hero NPC pathfinding (not crowds) |
| MassMovement | UE5.2+ | Movement processor for Mass Entity | ZoneGraph lane following |
| MassRepresentation | UE5.0+ | LOD representation switching | Already integrated from Phase 6 |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| ZoneGraph | NavMesh for all | NavMesh 10x more expensive per entity, not designed for crowds |
| StateTree | Behavior Trees for all | BT overhead too high for 200+ entities |
| SmartObjects | Custom interaction system | Rebuilding reservation logic, tagging, and claiming |
| Behavior Trees | StateTree for hero NPCs | BT better for complex decision trees, StateTree better for states |

**Build.cs Dependencies (extend Phase 6):**
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
    "MassCommon",
    "MassAI",           // NEW: Core AI module
    "ZoneGraph",        // NEW: Navigation lanes
    "MassMovement",     // NEW: Movement along lanes
    "StateTreeModule",  // NEW: Behavior control
    "SmartObjectsModule", // NEW: Interaction points
    "AIModule",         // NEW: Behavior Trees, AI Perception
    "NavigationSystem"  // NEW: NavMesh for hero NPCs
});

// Remove MassAI from Private - now public dependency
```

## Architecture Patterns

### Recommended Plugin Structure (Extended from Phase 6)
```
Plugins/GSD_Crowds/
├── Source/
│   └── GSD_Crowds/
│       ├── Public/
│       │   ├── GSD_Crowds.h
│       │   ├── Fragments/
│       │   │   ├── GSDZombieStateFragment.h      # Phase 6
│       │   │   ├── GSDNavigationFragment.h       # NEW: ZoneGraph lane data
│       │   │   └── GSDSmartObjectFragment.h      # NEW: Interaction state
│       │   ├── Processors/
│       │   │   ├── GSDZombieBehaviorProcessor.h  # Phase 6
│       │   │   ├── GSDCrowdLODProcessor.h        # Phase 6
│       │   │   ├── GSDNavigationProcessor.h      # NEW: Lane following
│       │   │   └── GSDSmartObjectProcessor.h     # NEW: Interaction handling
│       │   ├── Traits/
│       │   │   └── GSDCrowdTrait.h               # NEW: Combined trait for spawning
│       │   ├── Subsystems/
│       │   │   ├── GSDCrowdManagerSubsystem.h    # Phase 6 (extend)
│       │   │   └── GSDSmartObjectSubsystem.h     # NEW: Smart Object queries
│       │   ├── Actors/
│       │   │   ├── GSDCrowdTestbedActor.h        # Phase 6
│       │   │   └── GSDHeroNPC.h                  # NEW: Hero NPC pawn
│       │   ├── AI/
│       │   │   ├── GSDHeroAIController.h         # NEW: Hero NPC controller
│       │   │   └── GSDHeroPerceptionComponent.h  # NEW: AI Perception setup
│       │   └── SmartObjects/
│       │       └── GSDSmartObjectDefinition.h    # NEW: Custom SO types
│       └── Private/
│           └── [implementations]
└── Content/
    └── GSD_Crowds/
        ├── EntityConfigs/
        │   └── GSDZombieEntityConfig.uasset      # Phase 6 (extend with nav)
        ├── ZoneGraphs/
        │   └── DG_CrowdLanes.uasset              # NEW: ZoneGraph data layer
        ├── SmartObjects/
        │   ├── SO_Cover.uasset                   # NEW
        │   ├── SO_Ambush.uasset                  # NEW
        │   └── SO_WanderPoint.uasset             # NEW
        ├── StateTrees/
        │   └── ST_ZombieBehavior.uasset          # NEW
        └── BehaviorTrees/
            └── BT_HeroNPC.uasset                 # NEW
```

### Pattern 1: ZoneGraph Navigation Fragment

**What:** Fragment storing ZoneGraph lane reference and progress
**When to use:** All entities that need lane-based navigation

**Example:**
```cpp
// Source: UE5 MassAI ZoneGraph patterns
// https://dev.epicgames.com/documentation/en-us/unreal-engine/zonegraph-in-unreal-engine

USTRUCT()
struct GSD_CROWDS_API FGSDNavigationFragment : public FMassFragment
{
    GENERATED_BODY()

    //-- Lane Reference --
    UPROPERTY()
    FZoneGraphLaneHandle CurrentLane;

    UPROPERTY()
    float LanePosition = 0.0f;  // Distance along lane

    UPROPERTY()
    uint8 bIsOnLane : 1;

    UPROPERTY()
    uint8 bReachedDestination : 1;

    //-- Target --
    UPROPERTY()
    FZoneGraphLaneHandle TargetLane;

    UPROPERTY()
    float TargetLanePosition = -1.0f;

    //-- Movement Config --
    UPROPERTY()
    float DesiredSpeed = 150.0f;

    FGSDNavigationFragment()
        : bIsOnLane(false)
        , bReachedDestination(false)
    {}
};
```

### Pattern 2: Navigation Processor with ZoneGraph

**What:** Processor that moves entities along ZoneGraph lanes
**When to use:** All crowd entities with lane navigation

**Example:**
```cpp
// Source: UE5 MassMovement patterns
// https://dev.epicgames.com/documentation/en-us/unreal-engine/mass-entity-in-unreal-engine

UCLASS()
class GSD_CROWDS_API UGSDNavigationProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UGSDNavigationProcessor()
    {
        ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
        ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::SyncWorld);
        ProcessingPhase = EMassProcessingPhase::PrePhysics;
    }

    virtual void ConfigureQueries() override
    {
        EntityQuery.AddRequirement<FGSDNavigationFragment>(EMassFragmentAccess::ReadWrite);
        EntityQuery.AddRequirement<FDataFragment_Transform>(EMassFragmentAccess::ReadWrite);
        EntityQuery.AddRequirement<FGSDZombieStateFragment>(EMassFragmentAccess::ReadOnly);
    }

    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override
    {
        const UWorld* World = GetWorld();
        const UZoneGraphSubsystem* ZoneGraphSubsystem = World->GetSubsystem<UZoneGraphSubsystem>();

        EntityQuery.ForEachEntityChunk(EntityManager, Context,
            [&](FMassExecutionContext& Context)
            {
                auto NavFragments = Context.GetMutableFragmentView<FGSDNavigationFragment>();
                auto Transforms = Context.GetMutableFragmentView<FDataFragment_Transform>();
                const auto ZombieStates = Context.GetFragmentView<FGSDZombieStateFragment>();

                for (int32 i = 0; i < Context.GetNumEntities(); ++i)
                {
                    FGSDNavigationFragment& Nav = NavFragments[i];
                    FDataFragment_Transform& Transform = Transforms[i];
                    const FGSDZombieStateFragment& Zombie = ZombieStates[i];

                    if (!Nav.bIsOnLane || !Nav.CurrentLane.IsValid())
                    {
                        // Find nearest lane
                        FindNearestLane(Nav, Transform, ZoneGraphSubsystem);
                        continue;
                    }

                    // Move along lane
                    const float MoveSpeed = Zombie.MovementSpeed;
                    Nav.LanePosition += MoveSpeed * Context.GetDeltaTimeSeconds();

                    // Update transform from lane position
                    UpdateTransformFromLane(Nav, Transform, ZoneGraphSubsystem);

                    // Check if reached end of lane
                    CheckLaneProgress(Nav, ZoneGraphSubsystem);
                }
            });
    }

private:
    FMassEntityQuery EntityQuery;

    void FindNearestLane(FGSDNavigationFragment& Nav, const FDataFragment_Transform& Transform,
                         const UZoneGraphSubsystem* Subsystem);
    void UpdateTransformFromLane(FGSDNavigationFragment& Nav, FDataFragment_Transform& Transform,
                                  const UZoneGraphSubsystem* Subsystem);
    void CheckLaneProgress(FGSDNavigationFragment& Nav, const UZoneGraphSubsystem* Subsystem);
};
```

### Pattern 3: Smart Object Fragment and Processor

**What:** Fragment for Smart Object interaction state, processor for claiming/releasing
**When to use:** Entities that interact with world objects (cover, ambush points)

**Example:**
```cpp
// Source: UE5 Smart Objects documentation
// https://dev.epicgames.com/documentation/en-us/unreal-engine/smart-objects-in-unreal-engine

USTRUCT()
struct GSD_CROWDS_API FGSDSmartObjectFragment : public FMassFragment
{
    GENERATED_BODY()

    //-- Current Interaction --
    UPROPERTY()
    FSmartObjectClaimHandle ClaimedHandle;

    UPROPERTY()
    float InteractionTime = 0.0f;

    UPROPERTY()
    float InteractionDuration = 0.0f;

    //-- State --
    UPROPERTY()
    uint8 bIsInteracting : 1;

    UPROPERTY()
    uint8 bHasClaimedObject : 1;

    FGSDSmartObjectFragment()
        : bIsInteracting(false)
        , bHasClaimedObject(false)
    {}
};

// Processor for Smart Object interactions
UCLASS()
class GSD_CROWDS_API UGSDSmartObjectProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UGSDSmartObjectProcessor()
    {
        // Run after navigation, before behavior
        ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::SyncWorld;
        ProcessingPhase = EMassProcessingPhase::PrePhysics;
    }

    virtual void ConfigureQueries() override
    {
        EntityQuery.AddRequirement<FGSDSmartObjectFragment>(EMassFragmentAccess::ReadWrite);
        EntityQuery.AddRequirement<FDataFragment_Transform>(EMassFragmentAccess::ReadOnly);
        EntityQuery.AddRequirement<FGSDZombieStateFragment>(EMassFragmentAccess::ReadWrite);
    }

    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
    float SearchRadius = 1000.0f;
};
```

### Pattern 4: Hero NPC with Behavior Tree and AI Perception

**What:** Traditional AI setup for gameplay-critical NPCs
**When to use:** Hero NPCs (important characters, bosses, quest-givers)

**Example:**
```cpp
// Hero NPC Controller
// Source: UE5 AI Controller patterns
// https://dev.epicgames.com/documentation/en-us/unreal-engine/ai-controllers-in-unreal-engine

UCLASS()
class GSD_CROWDS_API AGSDHeroAIController : public AAIController
{
    GENERATED_BODY()

public:
    AGSDHeroAIController();

    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

    //-- Behavior Tree --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    TObjectPtr<UBehaviorTree> BehaviorTree;

    //-- AI Perception --
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    TObjectPtr<UAIPerceptionComponent> PerceptionComponent;

    //-- Sight Config --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    //-- Hearing Config --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

    //-- Delegates --
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
    //-- Blackboard Keys --
    FName TargetActorKey = FName("TargetActor");
    FName TargetLocationKey = FName("TargetLocation");
    FName CanSeeTargetKey = FName("CanSeeTarget");
    FName LastKnownPositionKey = FName("LastKnownPosition");
};

// Hero NPC Pawn
UCLASS()
class GSD_CROWDS_API AGSDHeroNPC : public APawn, public IGSDSpawnable
{
    GENERATED_BODY()

public:
    AGSDHeroNPC();

    //-- IGSDSpawnable Interface --
    virtual void ApplySpawnConfig_Implementation(UGSDSpawnConfig* Config) override;
    virtual FTransform GetSpawnTransform_Implementation() override;

    //-- Components --
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCapsuleComponent> CapsuleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> SkeletalMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCharacterMovementComponent> MovementComponent;

    //-- AI Configuration --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    TObjectPtr<UBehaviorTree> BehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    TObjectPtr<UAIPerceptionComponent> PerceptionComponent;
};
```

### Pattern 5: StateTree for Crowd Behavior

**What:** StateTree asset for crowd entity behavior states
**When to use:** Crowd entities that need state-based behavior (idle, wander, chase)

**Example:**
```cpp
// StateTree is configured via Editor, but we can define the context
// Source: UE5 StateTree documentation
// https://dev.epicgames.com/documentation/en-us/unreal-engine/state-tree-in-unreal-engine

USTRUCT()
struct GSD_CROWDS_API FGSDCrowdStateTreeFragment : public FMassFragment
{
    GENERATED_BODY()

    //-- Current State --
    UPROPERTY()
    uint8 StateIndex = 0;  // 0=Idle, 1=Wander, 2=Chase, 3=Flee

    //-- State Timer --
    UPROPERTY()
    float TimeInState = 0.0f;

    //-- Transition Conditions --
    UPROPERTY()
    float StateDuration = 0.0f;

    UPROPERTY()
    uint8 bTransitionRequested : 1;
};

// StateTree states would be:
// 1. Idle - Stand still, look around
// 2. Wander - Move along ZoneGraph lanes randomly
// 3. Chase - Move toward player when visible
// 4. Interact - Use Smart Object (cover, ambush)
```

### Anti-Patterns to Avoid

- **Using NavMesh for 200+ crowd entities:** NavMesh pathfinding is too expensive; use ZoneGraph lanes
- **Behavior Trees for all entities:** BT overhead scales poorly; use StateTree for crowds, BT for hero NPCs
- **Smart Objects without reservation system:** Multiple entities will try to use same object
- **Skipping Velocity Randomizer:** All entities move in perfect synchronization (Phase 6 lesson)
- **Polling Smart Objects every frame:** Use spatial queries with cooldowns
- **Storing AActor pointers in Mass Fragments:** Use indices or handles; fragments are not UObjects
- **Ignoring ZoneGraph maintenance mode status:** Have fallback to direct movement if ZoneGraph unavailable

## Don't Hand-Roll

Problems that look simple but have existing solutions:

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Lane navigation | Custom spline following | ZoneGraph + MassMovement | Built-in lane connections, metadata |
| Smart Object claiming | Custom reservation system | SmartObjectsModule | Thread-safe claiming, automatic release |
| Behavior state machine | Custom state machine | StateTree | Visual editor, transitions, conditions |
| Hero NPC decision making | Custom decision logic | Behavior Trees + Blackboard | Proven, debugger support, visual |
| AI sensory input | Custom raycasts | AI Perception System | Multiple sense types, stimulus handling |
| Path following | Custom path following | AIController MoveTo | Built-in path following, callbacks |
| Crowd de-synchronization | Random velocity in Tick | Velocity Randomizer trait | Proper interpolation, distribution |

**Key insight:** UE5 provides complete AI stack for crowds (MassAI, ZoneGraph, StateTree, SmartObjects) and hero NPCs (BT, AI Perception). Building custom solutions is unnecessary and will be slower/buggier.

## Common Pitfalls

### Pitfall 1: ZoneGraph Experimental Status

**What goes wrong:** ZoneGraph features change or break between UE versions
**Why it happens:** ZoneGraph is marked experimental/maintenance mode in UE5
**How to avoid:** Implement fallback to direct movement, test on target UE version, watch release notes
**Warning signs:** ZoneGraph compilation errors, missing lane data at runtime

```cpp
// Always check ZoneGraph availability
if (UZoneGraphSubsystem* ZoneGraph = World->GetSubsystem<UZoneGraphSubsystem>())
{
    if (ZoneGraph->GetNumLanes() > 0)
    {
        // Use ZoneGraph navigation
    }
    else
    {
        // Fallback to direct movement
    }
}
else
{
    // Fallback to direct movement
}
```

### Pitfall 2: Smart Object Claiming Race Conditions

**What goes wrong:** Multiple entities claim same Smart Object simultaneously
**Why it happens:** Claiming is async, race conditions without proper handles
**How to avoid:** Always use FSmartObjectClaimHandle, check IsValid() before use
**Warning signs:** Entities stacked on same Smart Object location, animation glitches

```cpp
// WRONG:
if (SmartObject->IsAvailable())
{
    // Race condition! Another entity could claim between check and claim
    UseSmartObject(SmartObject);
}

// CORRECT:
FSmartObjectClaimHandle Handle = SmartObjectSubsystem->Claim(SmartObjectHandle);
if (Handle.IsValid())
{
    // Claimed successfully, safe to use
    UseSmartObject(Handle);
}
```

### Pitfall 3: AI Perception Not Detecting Player

**What goes wrong:** Hero NPCs don't react to player despite correct setup
**Why it happens:** Player pawn missing AI Perception Stimuli Source, or sense config wrong
**How to avoid:** Ensure player has UAIPerceptionStimuliSourceComponent, configure sight sense correctly
**Warning signs:** AI Perception debug shows no stimuli, OnTargetPerceptionUpdated never fires

```cpp
// On player pawn:
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
TObjectPtr<UAIPerceptionStimuliSourceComponent> StimuliSource;

// In BeginPlay:
StimuliSource->RegisterForSense(UAISense_Sight::StaticClass());
StimuliSource->RegisterWithPerceptionSystem();
```

### Pitfall 4: StateTree Not Transitioning

**What goes wrong:** Entities stuck in one state, StateTree transitions never fire
**Why it happens:** Transition conditions never met, or StateTree not ticking
**How to avoid:** Debug StateTree with visualizer, ensure proper tick settings, test conditions
**Warning signs:** Entities repeat same behavior indefinitely, no state changes

```cpp
// Ensure StateTree component ticks
UStateTreeComponent* StateTreeComp = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTree"));
StateTreeComp->SetIsReplicated(false);  // Single-player
StateTreeComp->SetTickEnabled(true);
```

### Pitfall 5: Behavior Tree Not Running

**What goes wrong:** Blackboard values not updating, BT not making decisions
**Why it happens:** BT not started, Blackboard not initialized, or AIController not possessing
**How to avoid:** Call RunBehaviorTree() in OnPossess(), verify Blackboard asset is set
**Warning signs:** BT debugger shows no active nodes, Blackboard values all default

```cpp
void AGSDHeroAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BehaviorTree)
    {
        // CRITICAL: Must use the BT's Blackboard asset
        UseBlackboard(BehaviorTree->BlackboardAsset, BlackboardComponent);
        RunBehaviorTree(BehaviorTree);
    }
}
```

## Code Examples

### ZoneGraph Data Layer Setup

```cpp
// ZoneGraph is authored in Editor using Zone Shape actors
// Place AZoneShape actors to define navigation lanes
// Lanes can have metadata (type, speed, tags)

// Accessing ZoneGraph data at runtime:
const UZoneGraphSubsystem* ZoneGraphSubsystem = World->GetSubsystem<UZoneGraphSubsystem>();

// Query lanes near a location
TArray<FZoneGraphLaneHandle> NearbyLanes;
const FVector QueryLocation = EntityTransform.GetLocation();
const float QueryRadius = 1000.0f;

ZoneGraphSubsystem->FindLanesInBounds(
    FBoxCenterAndExtent(QueryLocation, FVector(QueryRadius)),
    NearbyLanes
);

// Get lane point data
for (const FZoneGraphLaneHandle& Lane : NearbyLanes)
{
    FZoneGraphLaneLocation LaneLocation;
    if (ZoneGraphSubsystem->GetLaneLocation(Lane, 0.0f, LaneLocation))
    {
        // LaneLocation.Position is world position
        // LaneLocation.Direction is lane direction
        // LaneLocation.Normal is lane normal
    }
}
```

### Smart Object Definition

```cpp
// Smart Object definition (Data Asset)
// Created in Editor: Right-click -> Miscellaneous -> Smart Object Definition

// Querying Smart Objects at runtime:
UCLASS()
class UGSDSmartObjectSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    TArray<FSmartObjectHandle> FindNearbySmartObjects(const FVector& Location, float Radius)
    {
        USmartObjectSubsystem* SOSubsystem = GetWorld()->GetSubsystem<USmartObjectSubsystem>();
        if (!SOSubsystem)
        {
            return {};
        }

        FSmartObjectRequest Request;
        Request.QueryBox = FBoxCenterAndExtent(Location, FVector(Radius));
        Request.Filter = FSmartObjectRequestFilter(); // Add tags/requirements

        TArray<FSmartObjectHandle> Results;
        SOSubsystem->FindSmartObjects(Request, Results);
        return Results;
    }

    FSmartObjectClaimHandle ClaimSmartObject(FSmartObjectHandle Handle)
    {
        USmartObjectSubsystem* SOSubsystem = GetWorld()->GetSubsystem<USmartObjectSubsystem>();
        if (!SOSubsystem)
        {
            return FSmartObjectClaimHandle::Invalid;
        }

        return SOSubsystem->Claim(Handle);
    }

    void ReleaseSmartObject(FSmartObjectClaimHandle Handle)
    {
        USmartObjectSubsystem* SOSubsystem = GetWorld()->GetSubsystem<USmartObjectSubsystem>();
        if (SOSubsystem && Handle.IsValid())
        {
            SOSubsystem->Release(Handle);
        }
    }
};
```

### AI Perception Setup

```cpp
// Hero NPC Controller with AI Perception
// Source: UE5 AI Perception documentation

AGSDHeroAIController::AGSDHeroAIController()
{
    // Create perception component
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

    // Create sight config
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2500.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);  // Remember sightings for 5 seconds
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

    // Create hearing config
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->SetMaxAge(3.0f);

    // Apply configs
    PerceptionComponent->ConfigureSense(*SightConfig);
    PerceptionComponent->ConfigureSense(*HearingConfig);
    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind delegate
    PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AGSDHeroAIController::OnTargetPerceptionUpdated);
}

void AGSDHeroAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (Stimulus.WasSuccessfullySensed())
    {
        // Actor detected
        if (UBlackboardComponent* BB = GetBlackboardComponent())
        {
            BB->SetValueAsObject(TargetActorKey, Actor);
            BB->SetValueAsVector(TargetLocationKey, Stimulus.StimulusLocation);
            BB->SetValueAsBool(CanSeeTargetKey, true);
        }
    }
    else
    {
        // Lost sight of actor
        if (UBlackboardComponent* BB = GetBlackboardComponent())
        {
            BB->SetValueAsBool(CanSeeTargetKey, false);
            BB->SetValueAsVector(LastKnownPositionKey, Stimulus.StimulusLocation);
        }
    }
}
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| NavMesh for all AI | ZoneGraph for crowds, NavMesh for heroes | UE5.0 | 10x more crowd entities |
| Behavior Trees for all | StateTree for crowds, BT for heroes | UE5.0 | Better crowd performance |
| Custom interaction points | Smart Objects with reservation | UE5.0 | Thread-safe interactions |
| Manual perception | AI Perception System | UE4.x | Standardized sensory input |
| Individual actor navigation | Mass Entity navigation | UE5.0 | Data-oriented processing |

**Deprecated/outdated:**
- EQS for crowd queries: Too expensive for 200+ entities
- Custom AI sensing: Use AI Perception System instead
- Blueprint-heavy AI logic: Move to C++ for performance
- NavMesh for lane following: ZoneGraph designed for this purpose

## Open Questions

Things that couldn't be fully resolved:

1. **ZoneGraph Stability**
   - What we know: ZoneGraph is marked experimental/maintenance mode
   - What's unclear: Epic's long-term commitment, roadmap for stabilization
   - Recommendation: Implement with fallback, test thoroughly on target UE version

2. **StateTree vs Behavior Tree for Crowds**
   - What we know: StateTree recommended for crowds, BT for heroes
   - What's unclear: Exact complexity threshold for switching
   - Recommendation: Use StateTree for simple state-based behavior, BT if complex decision trees needed

3. **Smart Object Performance at Scale**
   - What we know: Smart Objects use reservation system
   - What's unclear: Performance impact with 100+ Smart Objects
   - Recommendation: Spatial partitioning, limit queries per frame

4. **MassAI Integration Complexity**
   - What we know: MassAI provides Mass Entity AI capabilities
   - What's unclear: How much custom processor work needed vs built-in traits
   - Recommendation: Start with built-in traits, add custom processors for game-specific behavior

5. **World Partition + ZoneGraph Integration**
   - What we know: ZoneGraph data can be streamed
   - What's unclear: How lane references behave when cells unload
   - Recommendation: Test streaming scenarios, handle invalid lane references gracefully

## Sources

### Primary (HIGH confidence)
- Epic Games Documentation: StateTree in Unreal Engine - https://dev.epicgames.com/documentation/en-us/unreal-engine/state-tree-in-unreal-engine
- Epic Games Documentation: Smart Objects in Unreal Engine - https://dev.epicgames.com/documentation/en-us/unreal-engine/smart-objects-in-unreal-engine
- Epic Games Documentation: AI Perception in Unreal Engine - https://dev.epicgames.com/documentation/en-us/unreal-engine/ai-perception-in-unreal-engine
- Epic Games Documentation: ZoneGraph in Unreal Engine - https://dev.epicgames.com/documentation/en-us/unreal-engine/zonegraph-in-unreal-engine
- Phase 6 RESEARCH.md - Mass Entity patterns and existing GSD_Crowds implementation

### Secondary (MEDIUM confidence)
- UE5 Source Code: MassAI module (API patterns verified)
- UE5 Source Code: ZoneGraph module (navigation patterns verified)
- Community tutorials: AI setup guides (cross-referenced with official docs)

### Tertiary (LOW confidence)
- Web search results for ZoneGraph best practices (limited availability, flagged for validation)
- Forum discussions on MassAI performance (anecdotal, needs validation)

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH - Based on official UE5 documentation and Phase 6 foundation
- Architecture: MEDIUM - ZoneGraph experimental status adds uncertainty
- Pitfalls: HIGH - Well-documented common issues with clear solutions
- Hero NPC setup: HIGH - Traditional AI patterns well-established
- Crowd navigation: MEDIUM - ZoneGraph maintenance mode is concern

**Research date:** 2026-02-26
**Valid until:** 30 days (UE5 API stable, but ZoneGraph experimental status warrants monitoring)
