# Phase 4: Vehicle Core Systems - Research

**Researched:** 2026-02-25
**Domain:** UE5 Chaos Vehicle Physics, Data Asset Configuration, Plugin Architecture
**Confidence:** MEDIUM (UE5.7 docs limited, UE5.5 patterns verified)

## Summary

Phase 4 implements vehicle spawning from Data Assets with Chaos Vehicle physics integration. The research covers UE5's Chaos Vehicle plugin architecture, Data Asset design patterns for vehicle configuration, wheel bone validation requirements, and GSD_Vehicles plugin structure that integrates with existing GSD interfaces (IGSDSpawnable, UGSDDataAsset).

UE5's Chaos Vehicle system replaces the deprecated PhysX vehicle system and requires a specific setup: WheeledVehiclePawn base class, ChaosVehicleWheel blueprints for wheel configuration, ChaosWheeledVehicleMovementComponent for physics, and proper skeletal mesh with wheel bone orientation. The critical wheel bone orientation requirement (X=Forward, Y=Left, Z=Up) must be validated on import to prevent physics bugs.

**Primary recommendation:** Use UE5's standard Chaos Vehicle classes (WheeledVehiclePawn, ChaosVehicleWheel, ChaosWheeledVehicleMovementComponent) with GSD wrapper classes that implement IGSDSpawnable. Create Data Asset hierarchy (GSDVehicleConfig -> wheel/suspension/tire configs) that mirrors the existing UGSDDataAsset pattern.

## Standard Stack

The established libraries/tools for this domain:

### Core
| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| ChaosVehicles Plugin | UE5.0+ | Vehicle physics simulation | UE5 standard, replaces PhysX |
| WheeledVehiclePawn | UE5.0+ | Base vehicle actor class | Official UE5 vehicle base |
| ChaosVehicleWheel | UE5.0+ | Wheel configuration class | Required for wheel physics |
| ChaosWheeledVehicleMovementComponent | UE5.0+ | Physics movement handling | Standard vehicle physics |

### Supporting
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| UGSDDataAsset | Project | Configuration data base | All GSD configs inherit |
| IGSDSpawnable | Project | Spawn interface | Vehicle spawning abstraction |
| GSDStreamingSourceComponent | Project | Vehicle streaming | Predictive loading for vehicles |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| WheeledVehiclePawn | Custom Pawn with physics | Much more work, no engine support |
| ChaosVehicleWheel | Custom wheel component | Lose engine integration, debugging |
| Data Asset configs | Blueprint-only configs | No type safety, harder to manage |

**Build.cs Dependencies:**
```csharp
// In GSD_Vehicles.Build.cs
PrivateDependencyModuleNames.AddRange(new string[] {
    "ChaosVehicles",
    "ChaosVehiclesCore",
    "GSD_Core"
});
```

## Architecture Patterns

### Recommended Plugin Structure
```
Plugins/GSD_Vehicles/
├── Source/GSD_Vehicles/
│   ├── Public/
│   │   ├── GSDVehicles.h                    # Plugin module
│   │   ├── DataAssets/
│   │   │   ├── GSDVehicleConfig.h           # Main vehicle config
│   │   │   ├── GSDWheelConfig.h             # Wheel settings
│   │   │   ├── GSDSuspensionConfig.h        # Suspension settings
│   │   │   └── GSDTireConfig.h              # Tire friction settings
│   │   ├── Actors/
│   │   │   └── GSDVehiclePawn.h             # Base vehicle pawn
│   │   ├── Components/
│   │   │   └── GSDVehicleMovementComponent.h # Movement wrapper
│   │   └── Subsystems/
│   │       └── GSDVehicleSpawnerSubsystem.h  # Vehicle spawning
│   └── Private/
│       └── [matching .cpp files]
└── Docs/
    └── VehicleSetup.md
```

### Pattern 1: Data Asset Hierarchy for Vehicles
**What:** Vehicle configuration stored in nested Data Assets
**When to use:** All vehicle definitions for consistency and reusability
**Example:**
```cpp
// Source: GSD architecture pattern from Phase 1
// GSDVehicleConfig.h
UCLASS(BlueprintType, Category = "GSD|Vehicles")
class GSD_VEHICLES_API UGSDVehicleConfig : public UGSDDataAsset
{
    GENERATED_BODY()

public:
    // Vehicle mesh
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vehicle")
    TSoftObjectPtr<USkeletalMesh> VehicleMesh;

    // Physics asset
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vehicle")
    TSoftObjectPtr<UPhysicsAsset> PhysicsAsset;

    // Animation blueprint
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vehicle")
    TSubclassOf<UAnimInstance> AnimBlueprintClass;

    // Wheel configurations (front/rear)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wheels")
    TArray<TSoftObjectPtr<UGSDWheelConfig>> WheelConfigs;

    // Engine torque curve
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Engine")
    UCurveFloat* TorqueCurve;

    // Mass in kg
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Physics")
    float Mass = 1500.0f;

    // Center of mass offset
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Physics")
    FVector CenterOfMassOffset = FVector::ZeroVector;
};
```

### Pattern 2: Wheel Configuration Data Asset
**What:** Separate wheel config for front/rear wheel reuse
**When to use:** All vehicles for consistent wheel setup
**Example:**
```cpp
// GSDWheelConfig.h
UCLASS(BlueprintType, Category = "GSD|Vehicles")
class GSD_VEHICLES_API UGSDWheelConfig : public UGSDDataAsset
{
    GENERATED_BODY()

public:
    // Wheel bone name in skeletal mesh
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wheel")
    FName WheelBoneName;

    // Wheel radius in cm
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wheel")
    float WheelRadius = 35.0f;

    // Wheel width in cm
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wheel")
    float WheelWidth = 20.0f;

    // Is this wheel affected by steering?
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wheel")
    bool bAffectedBySteering = false;

    // Is this wheel affected by handbrake?
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wheel")
    bool bAffectedByHandbrake = false;

    // Is this wheel driven by engine?
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wheel")
    bool bAffectedByEngine = true;

    // Max steering angle in degrees
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wheel")
    float MaxSteerAngle = 70.0f;

    // Suspension settings
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Suspension")
    float SuspensionMaxRaise = 10.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Suspension")
    float SuspensionMaxDrop = 10.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Suspension")
    float SuspensionDampingRatio = 0.5f;

    // Tire friction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tire")
    float FrictionForceMultiplier = 3.0f;
};
```

### Pattern 3: Vehicle Pawn with IGSDSpawnable
**What:** Vehicle pawn implements IGSDSpawnable for GSD integration
**When to use:** All GSD vehicles
**Example:**
```cpp
// Source: GSD IGSDSpawnable pattern from Phase 1
// GSDVehiclePawn.h
UCLASS(BlueprintType, Category = "GSD|Vehicles")
class GSD_VEHICLES_API AGSDVehiclePawn : public AWheeledVehiclePawn, public IGSDSpawnable
{
    GENERATED_BODY()

public:
    AGSDVehiclePawn();

    // IGSDSpawnable implementation
    virtual void SpawnAsync(UGSDDataAsset* Config) override;
    virtual UGSDDataAsset* GetSpawnConfig() override { return VehicleConfig; }
    virtual bool IsSpawned() override { return bIsSpawned; }
    virtual void Despawn() override;
    virtual void ResetSpawnState() override;

    // Vehicle movement component accessor
    UFUNCTION(BlueprintPure, Category = "GSD|Vehicles")
    UChaosWheeledVehicleMovementComponent* GetVehicleMovement() const;

    // Streaming source for World Partition integration
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GSD|Vehicles")
    UGSDStreamingSourceComponent* StreamingSource;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "GSD|Vehicles")
    UGSDVehicleConfig* VehicleConfig;

    bool bIsSpawned = false;

    void ApplyVehicleConfig(UGSDVehicleConfig* Config);
};
```

### Pattern 4: Vehicle Spawner Subsystem
**What:** World subsystem manages vehicle spawning from Data Assets
**When to use:** Centralized vehicle spawning with streaming integration
**Example:**
```cpp
// GSDVehicleSpawnerSubsystem.h
UCLASS()
class GSD_VEHICLES_API UGSDVehicleSpawnerSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // Spawn vehicle from config at location
    UFUNCTION(BlueprintCallable, Category = "GSD|Vehicles")
    AGSDVehiclePawn* SpawnVehicle(
        UGSDVehicleConfig* Config,
        FVector Location,
        FRotator Rotation = FRotator::ZeroRotator
    );

    // Async spawn with callback
    UFUNCTION(BlueprintCallable, Category = "GSD|Vehicles")
    void SpawnVehicleAsync(
        UGSDVehicleConfig* Config,
        FVector Location,
        FRotator Rotation,
        const FOnSpawnComplete& OnComplete
    );

    // Despawn all vehicles
    UFUNCTION(BlueprintCallable, Category = "GSD|Vehicles")
    void DespawnAllVehicles();

    // Get all spawned vehicles
    UFUNCTION(BlueprintPure, Category = "GSD|Vehicles")
    const TArray<AGSDVehiclePawn*>& GetSpawnedVehicles() const { return SpawnedVehicles; }

private:
    UPROPERTY()
    TArray<AGSDVehiclePawn*> SpawnedVehicles;
};
```

### Anti-Patterns to Avoid
- **PhysX Vehicle Classes:** UE5 uses Chaos, not PhysX. Never use AWheeledVehicle (PhysX) or UWheeledVehicleMovementComponent (PhysX)
- **Tick-based Velocity Polling:** Use event-driven pattern for vehicle state changes (see Phase 3 vehicle streaming integration)
- **Hardcoded Wheel Bone Names:** Use Data Asset configuration for bone names
- **Wheel Blueprint per Wheel:** Create reusable wheel configs, not unique blueprints for each wheel position

## Don't Hand-Roll

Problems that look simple but have existing solutions:

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Vehicle physics simulation | Custom physics | ChaosWheeledVehicleMovementComponent | Engine integration, debugging, network support |
| Wheel suspension | Custom spring logic | ChaosVehicleWheel properties | Chaos handles all physics edge cases |
| Engine torque curves | Custom RPM logic | UCurveFloat + EngineSetup | Standard UE pattern, visual editing |
| Tire friction | Custom friction math | FrictionForceMultiplier + TireConfig | Engine handles material interactions |
| Vehicle spawning | Custom spawn logic | IGSDSpawnable + UGSDDataAsset | GSD consistency, async support |

**Key insight:** Chaos Vehicle physics is deeply integrated with UE5's physics, networking, and animation systems. Custom implementations would lose debugging tools, performance profiling, and multiplayer support.

## Common Pitfalls

### Pitfall 1: Incorrect Wheel Bone Orientation
**What goes wrong:** Wheels don't rotate properly, vehicle physics is erratic or vehicle falls through ground
**Why it happens:** UE5 expects wheel bones in specific local coordinate orientation
**How to avoid:** Validate bone orientation on import - MUST be: X=Forward, Y=Left, Z=Up (local coordinates)
**Warning signs:**
- Wheels rotate sideways instead of forward
- Vehicle has extreme understeer/oversteer unexpectedly
- Physics debugger shows wheel forces in wrong directions

**Critical Requirements:**
| Axis | Direction | Color in UE |
|------|-----------|-------------|
| X (Red) | Forward | Red |
| Y (Green) | Left | Green |
| Z (Blue) | Up | Blue |

### Pitfall 2: Using PhysX Vehicle Classes
**What goes wrong:** Compile errors or runtime crashes in UE5.3+
**Why it happens:** PhysX vehicles were deprecated in UE5.0, removed in UE5.3+
**How to avoid:** Always use Chaos classes:
- `AWheeledVehiclePawn` (not `AWheeledVehicle`)
- `UChaosWheeledVehicleMovementComponent` (not `UWheeledVehicleMovementComponent`)
- `UChaosVehicleWheel` (not `UVehicleWheel`)
**Warning signs:** Build errors mentioning PhysX, missing vehicle movement component

### Pitfall 3: Missing Animation Blueprint Wheel Controllers
**What goes wrong:** Wheels don't animate (no rotation visual)
**Why it happens:** UE5 uses Wheel Controller nodes, not the deprecated Wheel Handler from PhysX
**How to avoid:** Create Animation Blueprint with Wheel Controller nodes for each wheel bone
**Warning signs:** Wheels visually stationary while vehicle moves

### Pitfall 4: Not Enabling Chaos Plugin
**What goes wrong:** Vehicle classes not found, movement component is null
**Why it happens:** ChaosVehiclesPlugin must be explicitly enabled in project settings
**How to avoid:**
1. Edit > Plugins > Physics > ChaosVehiclesPlugin > Enable
2. Restart editor
**Warning signs:** Compile errors about missing ChaosVehicle classes

### Pitfall 5: Streaming Source on Vehicles Without Event-Driven Pattern
**What goes wrong:** Performance degradation with many parked vehicles (0.5-1.0ms per frame)
**Why it happens:** Polling velocity in Tick instead of event-driven state changes
**How to avoid:** Use GSDStreamingSourceComponent with OnVehicleStateChanged delegate pattern (already implemented in Phase 3)
**Warning signs:** Frame rate drops in areas with many parked vehicles

## Code Examples

Verified patterns from official sources and project context:

### Vehicle Pawn Initialization
```cpp
// Source: UE5 Chaos Vehicle pattern + GSD IGSDSpawnable
// GSDVehiclePawn.cpp
AGSDVehiclePawn::AGSDVehiclePawn()
{
    // Create vehicle mesh component (inherited from WheeledVehiclePawn)
    // Mesh is already created by parent class

    // Create our movement component wrapper
    VehicleMovement = CreateDefaultSubobject<UChaosWheeledVehicleMovementComponent>(
        TEXT("VehicleMovement"));

    // Create streaming source for World Partition integration
    StreamingSource = CreateDefaultSubobject<UGSDStreamingSourceComponent>(
        TEXT("StreamingSource"));
}

void AGSDVehiclePawn::SpawnAsync(UGSDDataAsset* Config)
{
    UGSDVehicleConfig* VehicleConfig = Cast<UGSDVehicleConfig>(Config);
    if (!VehicleConfig)
    {
        UE_LOG(LogGSD, Error, TEXT("Invalid vehicle config"));
        return;
    }

    this->VehicleConfig = VehicleConfig;
    ApplyVehicleConfig(VehicleConfig);
    bIsSpawned = true;

    // Configure streaming source for vehicle
    if (StreamingSource)
    {
        bool bIsFastVehicle = VehicleConfig->bIsFastVehicle;
        StreamingSource->ConfigureForVehicle(bIsFastVehicle);
    }
}

void AGSDVehiclePawn::ApplyVehicleConfig(UGSDVehicleConfig* Config)
{
    // Load skeletal mesh
    if (USkeletalMesh* Mesh = Config->VehicleMesh.LoadSynchronous())
    {
        GetMesh()->SetSkeletalMesh(Mesh);
    }

    // Set physics asset
    if (UPhysicsAsset* PhysAsset = Config->PhysicsAsset.LoadSynchronous())
    {
        GetMesh()->SetPhysicsAsset(PhysAsset);
    }

    // Set animation blueprint
    if (Config->AnimBlueprintClass)
    {
        GetMesh()->SetAnimInstanceClass(Config->AnimBlueprintClass);
    }

    // Configure wheel setups
    UChaosWheeledVehicleMovementComponent* Movement =
        Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());

    if (Movement && Config->WheelConfigs.Num() > 0)
    {
        Movement->WheelSetups.SetNum(Config->WheelConfigs.Num());

        for (int32 i = 0; i < Config->WheelConfigs.Num(); ++i)
        {
            if (UGSDWheelConfig* WheelConfig = Config->WheelConfigs[i].LoadSynchronous())
            {
                Movement->WheelSetups[i].WheelClass = UChaosVehicleWheel::StaticClass();
                Movement->WheelSetups[i].BoneName = WheelConfig->WheelBoneName;
            }
        }
    }

    // Set engine torque curve
    if (Config->TorqueCurve && Movement)
    {
        Movement->EngineSetup.TorqueCurve.ExternalCurve = Config->TorqueCurve;
    }

    // Set mass
    if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(GetRootComponent()))
    {
        RootPrim->SetMassOverrideInKg(NAME_None, Config->Mass);
    }
}
```

### Wheel Bone Validation Utility
```cpp
// GSDVehicleValidationLibrary.h
UCLASS()
class GSD_VEHICLES_API UGSDVehicleValidationLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // Validate wheel bone orientation
    UFUNCTION(BlueprintCallable, Category = "GSD|Vehicles|Validation")
    static bool ValidateWheelBoneOrientation(
        USkeletalMesh* SkeletalMesh,
        FName BoneName,
        FString& OutError
    );

    // Validate all wheel bones in skeletal mesh
    UFUNCTION(BlueprintCallable, Category = "GSD|Vehicles|Validation")
    static bool ValidateAllWheelBones(
        USkeletalMesh* SkeletalMesh,
        const TArray<FName>& WheelBoneNames,
        TArray<FString>& OutErrors
    );
};

// GSDVehicleValidationLibrary.cpp
bool UGSDVehicleValidationLibrary::ValidateWheelBoneOrientation(
    USkeletalMesh* SkeletalMesh,
    FName BoneName,
    FString& OutError)
{
    if (!SkeletalMesh)
    {
        OutError = TEXT("Skeletal mesh is null");
        return false;
    }

    const FReferenceSkeleton& RefSkeleton = SkeletalMesh->GetRefSkeleton();
    int32 BoneIndex = RefSkeleton.FindBoneIndex(BoneName);

    if (BoneIndex == INDEX_NONE)
    {
        OutError = FString::Printf(TEXT("Bone '%s' not found in skeletal mesh"), *BoneName.ToString());
        return false;
    }

    // Get bone transform in local space
    FTransform BoneTransform = RefSkeleton.GetRefBonePose()[BoneIndex];
    FQuat BoneRotation = BoneTransform.GetRotation();

    // UE5 expects wheel bones with:
    // X-axis pointing forward (vehicle forward direction)
    // Y-axis pointing left
    // Z-axis pointing up (wheel spin axis)

    // Get the bone's local axes
    FVector ForwardAxis = BoneRotation.GetAxisX();
    FVector UpAxis = BoneRotation.GetAxisZ();

    // Check if X is forward (should align with parent's X)
    // Check if Z is up (wheel spin axis)
    // Allow small tolerance for minor misalignment
    float Tolerance = 0.1f;

    bool bForwardValid = FMath::Abs(ForwardAxis.X - 1.0f) < Tolerance ||
                         FMath::Abs(ForwardAxis.X + 1.0f) < Tolerance;
    bool bUpValid = FMath::Abs(UpAxis.Z - 1.0f) < Tolerance ||
                    FMath::Abs(UpAxis.Z + 1.0f) < Tolerance;

    if (!bForwardValid)
    {
        OutError = FString::Printf(
            TEXT("Bone '%s': X-axis (%.2f, %.2f, %.2f) should point forward (1, 0, 0) or (-1, 0, 0)"),
            *BoneName.ToString(), ForwardAxis.X, ForwardAxis.Y, ForwardAxis.Z);
        return false;
    }

    if (!bUpValid)
    {
        OutError = FString::Printf(
            TEXT("Bone '%s': Z-axis (%.2f, %.2f, %.2f) should point up (0, 0, 1) or (0, 0, -1)"),
            *BoneName.ToString(), UpAxis.X, UpAxis.Y, UpAxis.Z);
        return false;
    }

    return true;
}
```

### Animation Blueprint Wheel Controller Pattern
```cpp
// In Vehicle Animation Blueprint (Event Graph)
// For each wheel, use Wheel Controller node:
// - Wheel Index: 0-3 (or more for multi-wheel vehicles)
// - Input: Get Vehicle Movement -> Get Wheel State (Wheel Index)
// - Output: Set Bone Rotation (pitch = wheel rotation)

// This replaces the deprecated Wheel Handler from PhysX
// The Wheel Controller node reads from ChaosVehicleMovementComponent
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| PhysX Vehicles | Chaos Vehicles | UE5.0 | Better physics, async support, networking |
| AWheeledVehicle | AWheeledVehiclePawn | UE5.0 | Pawn-based, better input handling |
| Wheel Handler (Anim BP) | Wheel Controller | UE5.0 | Direct Chaos integration |
| Polling velocity in Tick | Event-driven state | Phase 3 | Performance improvement |

**Deprecated/outdated:**
- `AWheeledVehicle`: Use `AWheeledVehiclePawn`
- `UVehicleWheel`: Use `UChaosVehicleWheel`
- `UWheeledVehicleMovementComponent`: Use `UChaosWheeledVehicleMovementComponent`
- Wheel Handler nodes in Animation Blueprints: Use Wheel Controller nodes

## UE5.7 API Considerations

Based on available documentation (MEDIUM confidence):

1. **Chaos Vehicles Plugin:** Remains the standard for UE5.x
2. **Async Physics:** Physics runs on separate thread - use callbacks, not direct queries
3. **Interface Patterns:** Continue using simple virtual functions (Phase 1 pattern updated for UE5.7)
4. **Build.cs:** Module loading phases enforced (Phase 1 pattern)

**Note:** UE5.7 specific documentation is limited. The patterns from UE5.5 should apply. Flag any API differences during implementation.

## Integration with Existing GSD Systems

### IGSDSpawnable Integration
- GSDVehiclePawn implements IGSDSpawnable
- SpawnAsync loads vehicle config and applies to vehicle
- GetSpawnConfig returns UGSDVehicleConfig
- Despawn cleans up streaming source and physics

### UGSDDataAsset Hierarchy
```
UGSDDataAsset (Phase 1)
└── UGSDVehicleConfig
    ├── WheelConfigs: TArray<UGSDWheelConfig*>
    ├── SuspensionConfig: UGSDSuspensionConfig*
    └── TireConfig: UGSDTireConfig*
```

### GSDStreamingSourceComponent Integration
- Add to vehicle pawn in constructor
- Call ConfigureForVehicle() after config application
- Call OnVehicleStateChanged() from vehicle state delegate (EVENT-DRIVEN)

## Open Questions

Things that couldn't be fully resolved:

1. **UE5.7 Specific API Changes**
   - What we know: UE5 uses Chaos, interface patterns stable
   - What's unclear: Any UE5.7 specific vehicle API changes
   - Recommendation: Implement with UE5.5 patterns, test in UE5.7, flag any API differences

2. **Wheel Blueprint Class Requirements**
   - What we know: Wheel configs can be data-only or require blueprint subclasses
   - What's unclear: Minimum blueprint setup for wheels in UE5.5+
   - Recommendation: Start with data-only approach using UChaosVehicleWheel properties, create blueprints if needed

3. **Animation Blueprint Automation**
   - What we know: Wheel Controller nodes required per wheel
   - What's unclear: Can this be automated or must be manual per vehicle
   - Recommendation: Create reusable AnimBP base class with standard wheel controller setup

## Sources

### Primary (HIGH confidence)
- Epic Games Documentation - How to Set Up Vehicles in Unreal Engine (5.3+): https://dev.epicgames.com/documentation/en-us/unreal-engine/how-to-set-up-vehicles-in-unreal-engine
- UE5 Chaos Vehicle Migration Guide: https://dev.epicgames.com/documentation/zh-cn/unreal-engine/how-to-convert-physx-vehicles-to-chaos-in-unreal-engine
- GSD Core Plugin interfaces (IGSDSpawnable, UGSDDataAsset) - Verified in codebase

### Secondary (MEDIUM confidence)
- CSDN UE5 Chaos Vehicle implementation guide (verified against official docs)
- UE5 C++ vehicle implementation patterns (verified against official docs)
- City Sample project documentation for vehicle reference

### Tertiary (LOW confidence)
- UE5.7 specific vehicle API changes - documentation limited, flagged for validation
- Community forum discussions on wheel bone orientation - verified against known requirements

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH - Chaos Vehicles is the official UE5 standard, well documented
- Architecture: HIGH - Based on GSD patterns from Phases 1-3, verified interfaces
- Pitfalls: HIGH - Well-documented common issues with Chaos Vehicle setup
- UE5.7 specifics: LOW - Limited documentation, UE5.5 patterns should apply

**Research date:** 2026-02-25
**Valid until:** 2026-03-25 (30 days - stable engine APIs)
