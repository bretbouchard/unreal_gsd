---
phase: 01-plugin-architecture-foundation
plan: 02
type: execute
wave: 1
depends_on: ["01"]
files_modified:
  - Plugins/GSD_Core/Source/GSD_Core/Public/Types/GSDSpawnTypes.h
  - Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDSpawnable.h
  - Plugins/GSD_Core/Source/GSD_Core/Private/Interfaces/IGSDSpawnable.cpp
  - Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDMassSpawnerInterface.h
  - Plugins/GSD_Core/Source/GSD_Core/Private/Interfaces/IGSDMassSpawnerInterface.cpp
  - Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDComponentSpawnable.h
  - Plugins/GSD_Core/Source/GSD_Core/Private/Interfaces/IGSDComponentSpawnable.cpp
autonomous: true
must_haves:
  truths:
    - "Blueprint classes can implement IGSDSpawnable interface"
    - "SpawnAsync method can be called from Blueprint"
    - "Execute_ prefix pattern works for Blueprint implementations"
    - "Mass spawning supports batch operations with single callback"
    - "Component spawning works for audio/particle components"
  artifacts:
    - path: "Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDSpawnable.h"
      provides: "Actor spawning interface"
      contains: "UINTERFACE(Blueprintable)"
    - path: "Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDMassSpawnerInterface.h"
      provides: "Batch spawning for 200+ entities"
      contains: "SpawnBatch"
    - path: "Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDComponentSpawnable.h"
      provides: "Non-actor component spawning"
      contains: "SpawnComponent"
  key_links:
    - from: "IGSDSpawnable"
      to: "FGSDSpawnTicket"
      via: "SpawnAsync parameter"
      pattern: "UGSDDataAsset\\*"
    - from: "IGSDMassSpawnerInterface"
      to: "FOnMassSpawnComplete"
      via: "Delegate callback"
      pattern: "DECLARE_DYNAMIC_MULTICAST_DELEGATE"
---

# 02-PLAN: Spawning Interfaces

**Requirements:** PLUG-08, PLUG-09

**Objective:** Implement the core spawning interfaces (IGSDSpawnable, IGSDMassSpawnerInterface, IGSDComponentSpawnable) with Blueprint support using BlueprintNativeEvent pattern.

**Purpose:** Define the contract for how all entities spawn in the GSD platform. These interfaces enable async spawning, batch spawning for crowds (200+ zombies), and component spawning for audio/particles.

**Output:** Three Blueprint-exposable interfaces with proper Execute_ pattern support.

---

## Context

@.planning/PROJECT.md
@.planning/ROADMAP.md
@.planning/phases/01-plugin-architecture-foundation/01-RESEARCH.md
@.planning/phases/01-plugin-architecture-foundation/1-CONTEXT.md

---

## Tasks

<task type="auto">
  <name>T1: Create Spawn Types and Delegates</name>
  <files>
    Plugins/GSD_Core/Source/GSD_Core/Public/Types/GSDSpawnTypes.h
  </files>
  <action>
    Create the foundational spawn types, structs, and delegates that all spawning interfaces will use.

    GSDSpawnTypes.h:
    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "GSDSpawnTypes.generated.h"

    class UGSDDataAsset;

    // Forward declarations
    class AActor;
    class UActorComponent;

    // Network spawn parameters (for future multiplayer support)
    USTRUCT(BlueprintType)
    struct GSD_CORE_API FGSDNetworkSpawnParams
    {
        GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bReplicates = true;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        ENetRole RemoteRole = ROLE_SimulatedProxy;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float NetUpdateFrequency = 100.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float NetCullDistanceSquared = 10000.0f * 10000.0f; // 100m

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        AActor* Owner = nullptr;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bServerAuthoritative = true;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        APlayerController* RequestingController = nullptr;
    };

    // Spawn ticket containing all spawn parameters
    USTRUCT(BlueprintType)
    struct GSD_CORE_API FGSDSpawnTicket
    {
        GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FVector Location = FVector::ZeroVector;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FRotator Rotation = FRotator::ZeroRotator;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TSubclassOf<AActor> ActorClass;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        UGSDDataAsset* Config = nullptr;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FGSDNetworkSpawnParams NetworkParams;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 Priority = 0;
    };

    // Seeded spawn ticket for determinism
    USTRUCT(BlueprintType)
    struct GSD_CORE_API FGSDSeededSpawnTicket : public FGSDSpawnTicket
    {
        GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 SpawnSeed = 0;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 SpawnOrder = -1;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        uint32 ParameterHash = 0;
    };

    // Spawn comparator for deterministic ordering
    struct FGSDSpawnComparator
    {
        bool operator()(const FGSDSeededSpawnTicket& A, const FGSDSeededSpawnTicket& B) const
        {
            if (A.Priority != B.Priority)
                return A.Priority > B.Priority;
            return A.SpawnOrder < B.SpawnOrder;
        }
    };

    // Delegates for spawn callbacks
    DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSpawnComplete, AActor*, SpawnedActor);
    DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnSpawnFailed, const FString&, Reason, const FGSDSpawnTicket&, Ticket);

    // Mass spawn delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMassSpawnComplete, const TArray<AActor*>&, SpawnedActors);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMassSpawnChunkComplete, int32, ChunkIndex, int32, TotalChunks);

    // Tick context with budget tracking
    USTRUCT(BlueprintType)
    struct GSD_CORE_API FGSDTickContext
    {
        GENERATED_BODY()

        UPROPERTY(BlueprintReadWrite)
        float DeltaTime = 0.0f;

        UPROPERTY(BlueprintReadWrite)
        float GameTime = 0.0f;

        UPROPERTY(BlueprintReadWrite)
        float AudioBudgetRemainingMs = 2.0f;

        UFUNCTION(BlueprintPure, Category = "GSD")
        bool HasAudioBudget() const { return AudioBudgetRemainingMs > 0.1f; }
    };
    ```
  </action>
  <verify>
    - GSDSpawnTypes.h compiles without errors
    - All structs have GENERATED_BODY() macro
    - Delegates compile with correct signatures
    - FGSDSpawnComparator struct compiles
  </verify>
  <done>
    Spawn types header exists with FGSDSpawnTicket, FGSDSeededSpawnTicket, FGSDNetworkSpawnParams, and all spawn delegates defined.
  </done>
</task>

<task type="auto">
  <name>T2: Implement IGSDSpawnable Interface</name>
  <files>
    Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDSpawnable.h
    Plugins/GSD_Core/Source/GSD_Core/Private/Interfaces/IGSDSpawnable.cpp
  </files>
  <action>
    Create the primary actor spawning interface with BlueprintNativeEvent support.

    IGSDSpawnable.h:
    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "UObject/Interface.h"
    #include "Types/GSDSpawnTypes.h"
    #include "IGSDSpawnable.generated.h"

    class UGSDDataAsset;

    UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
    class UGSDSpawnable : public UInterface
    {
        GENERATED_BODY()
    };

    class GSD_CORE_API IGSDSpawnable
    {
        GENERATED_BODY()

    public:
        // Async spawn with config and callback
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Spawning")
        void SpawnAsync(UGSDDataAsset* Config, FOnSpawnComplete Callback);
        virtual void SpawnAsync_Implementation(UGSDDataAsset* Config, FOnSpawnComplete Callback);

        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Spawning")
        UGSDDataAsset* GetSpawnConfig();
        virtual UGSDDataAsset* GetSpawnConfig_Implementation();

        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Spawning")
        bool IsSpawned();
        virtual bool IsSpawned_Implementation();

        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Spawning")
        void Despawn();
        virtual void Despawn_Implementation();

        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Spawning")
        void ResetSpawnState();
        virtual void ResetSpawnState_Implementation();
    };
    ```

    IGSDSpawnable.cpp:
    ```cpp
    #include "Interfaces/IGSDSpawnable.h"

    void IGSDSpawnable::SpawnAsync_Implementation(UGSDDataAsset* Config, FOnSpawnComplete Callback)
    {
        // Default implementation - override in C++ or Blueprint
    }

    UGSDDataAsset* IGSDSpawnable::GetSpawnConfig_Implementation()
    {
        return nullptr;
    }

    bool IGSDSpawnable::IsSpawned_Implementation()
    {
        return false;
    }

    void IGSDSpawnable::Despawn_Implementation()
    {
        // Default implementation - override in C++ or Blueprint
    }

    void IGSDSpawnable::ResetSpawnState_Implementation()
    {
        // Default implementation - override in C++ or Blueprint
    }
    ```

    IMPORTANT: Include generated.h as LAST include. Use Execute_ prefix when calling from C++.
  </action>
  <verify>
    - IGSDSpawnable.h compiles without errors
    - IGSDSpawnable.cpp compiles without errors
    - Blueprint can implement the interface (create test BP)
    - Execute_SpawnAsync works from C++ code
  </verify>
  <done>
    IGSDSpawnable interface exists with SpawnAsync, GetSpawnConfig, IsSpawned, Despawn, ResetSpawnState methods. BlueprintNativeEvent pattern works for Blueprint overrides.
  </done>
</task>

<task type="auto">
  <name>T3: Implement IGSDMassSpawnerInterface</name>
  <files>
    Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDMassSpawnerInterface.h
    Plugins/GSD_Core/Source/GSD_Core/Private/Interfaces/IGSDMassSpawnerInterface.cpp
  </files>
  <action>
    Create the batch spawning interface for 200+ entities (zombies, crowds).

    IGSDMassSpawnerInterface.h:
    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "UObject/Interface.h"
    #include "Types/GSDSpawnTypes.h"
    #include "IGSDMassSpawnerInterface.generated.h"

    class UGSDDataAsset;

    UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
    class UGSDMassSpawnerInterface : public UInterface
    {
        GENERATED_BODY()
    };

    class GSD_CORE_API IGSDMassSpawnerInterface
    {
        GENERATED_BODY()

    public:
        // Batch spawn - single delegate for entire batch
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Mass")
        void SpawnBatch(
            const TArray<FTransform>& Transforms,
            UGSDDataAsset* Config,
            FOnMassSpawnComplete& OnComplete
        );
        virtual void SpawnBatch_Implementation(
            const TArray<FTransform>& Transforms,
            UGSDDataAsset* Config,
            FOnMassSpawnComplete& OnComplete
        );

        // Chunked spawning to avoid callback storms
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Mass")
        void SpawnBatchChunked(
            const TArray<FTransform>& Transforms,
            UGSDDataAsset* Config,
            int32 ChunkSize,
            FOnMassSpawnChunkComplete& OnChunkComplete
        );
        virtual void SpawnBatchChunked_Implementation(
            const TArray<FTransform>& Transforms,
            UGSDDataAsset* Config,
            int32 ChunkSize,
            FOnMassSpawnChunkComplete& OnChunkComplete
        );

        // Get current spawn queue status
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Mass")
        int32 GetPendingSpawnCount() const;
        virtual int32 GetPendingSpawnCount_Implementation() const;

        // Cancel pending spawns
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Mass")
        void CancelPendingSpawns();
        virtual void CancelPendingSpawns_Implementation();

        // Check if spawning is in progress
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Mass")
        bool IsSpawning() const;
        virtual bool IsSpawning_Implementation() const;
    };
    ```

    IGSDMassSpawnerInterface.cpp:
    ```cpp
    #include "Interfaces/IGSDMassSpawnerInterface.h"

    void IGSDMassSpawnerInterface::SpawnBatch_Implementation(
        const TArray<FTransform>& Transforms,
        UGSDDataAsset* Config,
        FOnMassSpawnComplete& OnComplete)
    {
        // Default implementation - override in C++ or Blueprint
    }

    void IGSDMassSpawnerInterface::SpawnBatchChunked_Implementation(
        const TArray<FTransform>& Transforms,
        UGSDDataAsset* Config,
        int32 ChunkSize,
        FOnMassSpawnChunkComplete& OnChunkComplete)
    {
        // Default implementation - override in C++ or Blueprint
    }

    int32 IGSDMassSpawnerInterface::GetPendingSpawnCount_Implementation() const
    {
        return 0;
    }

    void IGSDMassSpawnerInterface::CancelPendingSpawns_Implementation()
    {
        // Default implementation - override in C++ or Blueprint
    }

    bool IGSDMassSpawnerInterface::IsSpawning_Implementation() const
    {
        return false;
    }
    ```
  </action>
  <verify>
    - IGSDMassSpawnerInterface.h compiles without errors
    - IGSDMassSpawnerInterface.cpp compiles without errors
    - TArray<FTransform> parameter works with Blueprint
    - Delegate reference compiles correctly
  </verify>
  <done>
    IGSDMassSpawnerInterface exists with SpawnBatch, SpawnBatchChunked, GetPendingSpawnCount, CancelPendingSpawns, IsSpawning methods. Supports batch spawning for 200+ entities.
  </done>
</task>

<task type="auto">
  <name>T4: Implement IGSDComponentSpawnable Interface</name>
  <files>
    Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDComponentSpawnable.h
    Plugins/GSD_Core/Source/GSD_Core/Private/Interfaces/IGSDComponentSpawnable.cpp
  </files>
  <action>
    Create the component spawning interface for non-actor components (audio, particles, etc.).

    IGSDComponentSpawnable.h:
    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "UObject/Interface.h"
    #include "IGSDComponentSpawnable.generated.h"

    class UActorComponent;
    class AActor;

    UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
    class UGSDComponentSpawnable : public UInterface
    {
        GENERATED_BODY()
    };

    class GSD_CORE_API IGSDComponentSpawnable
    {
        GENERATED_BODY()

    public:
        // Spawn component on owner actor
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Components")
        UActorComponent* SpawnComponent(AActor* Owner, const FTransform& Transform);
        virtual UActorComponent* SpawnComponent_Implementation(AActor* Owner, const FTransform& Transform);

        // Get component type identifier
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Components")
        FName GetComponentType() const;
        virtual FName GetComponentType_Implementation() const;

        // Check if component is active
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Components")
        bool IsComponentActive() const;
        virtual bool IsComponentActive_Implementation() const;

        // Deactivate component
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Components")
        void DeactivateComponent();
        virtual void DeactivateComponent_Implementation();

        // Destroy component
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Components")
        void DestroyComponent();
        virtual void DestroyComponent_Implementation();
    };
    ```

    IGSDComponentSpawnable.cpp:
    ```cpp
    #include "Interfaces/IGSDComponentSpawnable.h"

    UActorComponent* IGSDComponentSpawnable::SpawnComponent_Implementation(AActor* Owner, const FTransform& Transform)
    {
        return nullptr;
    }

    FName IGSDComponentSpawnable::GetComponentType_Implementation() const
    {
        return NAME_None;
    }

    bool IGSDComponentSpawnable::IsComponentActive_Implementation() const
    {
        return false;
    }

    void IGSDComponentSpawnable::DeactivateComponent_Implementation()
    {
        // Default implementation - override in C++ or Blueprint
    }

    void IGSDComponentSpawnable::DestroyComponent_Implementation()
    {
        // Default implementation - override in C++ or Blueprint
    }
    ```
  </action>
  <verify>
    - IGSDComponentSpawnable.h compiles without errors
    - IGSDComponentSpawnable.cpp compiles without errors
    - UActorComponent* return type works with Blueprint
    - AActor* parameter compiles correctly
  </verify>
  <done>
    IGSDComponentSpawnable interface exists with SpawnComponent, GetComponentType, IsComponentActive, DeactivateComponent, DestroyComponent methods.
  </done>
</task>

---

## Verification Checklist

- [ ] GSDSpawnTypes.h defines all structs and delegates
- [ ] FGSDSpawnTicket has all required fields
- [ ] FGSDSeededSpawnTicket extends FGSDSpawnTicket correctly
- [ ] IGSDSpawnable compiles with BlueprintNativeEvent
- [ ] IGSDMassSpawnerInterface compiles with TArray<FTransform>
- [ ] IGSDComponentSpawnable compiles with UActorComponent* return
- [ ] All _Implementation functions have empty defaults
- [ ] generated.h is last include in all headers
- [ ] No circular dependencies in includes

---

## Success Criteria

| Criterion | Target |
|-----------|--------|
| Interfaces compile | 100% |
| Blueprint support | Verified |
| Execute_ pattern | Works |
| Batch spawning API | Defined |

---

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| generated.h include order | Medium | High | Always last include |
| Delegate compilation | Low | Medium | Use DECLARE_DYNAMIC patterns |
| TArray in Blueprint | Low | Medium | Test with BP compilation |

---

## Estimated Complexity

**Medium** - Multiple interfaces with Blueprint integration

---

## Output

After completion, create `.planning/phases/01-plugin-architecture-foundation/02-SUMMARY.md`
