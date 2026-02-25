---
phase: 01-plugin-architecture-foundation
plan: 04
type: execute
wave: 2
depends_on: ["01", "02"]
files_modified:
  - Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDNetworkSpawnValidator.h
  - Plugins/GSD_Core/Source/GSD_Core/Private/Interfaces/IGSDNetworkSpawnValidator.cpp
  - Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDSpawnStateValidator.h
  - Plugins/GSD_Core/Source/GSD_Core/Private/Interfaces/IGSDSpawnStateValidator.cpp
autonomous: true
must_haves:
  truths:
    - "Server can validate spawn requests before execution"
    - "Spawn state can be hashed for determinism verification"
    - "Divergence between expected and actual spawn state is logged"
  artifacts:
    - path: "Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDNetworkSpawnValidator.h"
      provides: "Server-authoritative spawn validation"
      contains: "ValidateSpawnRequest"
    - path: "Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDSpawnStateValidator.h"
      provides: "Determinism state verification"
      contains: "ValidateSpawnState"
  key_links:
    - from: "IGSDNetworkSpawnValidator"
      to: "FGSDSpawnTicket"
      via: "Validation parameter"
      pattern: "const FGSDSpawnTicket&"
---

# 04-PLAN: Network and Determinism Interfaces

**Requirements:** PLUG-08, PLUG-09

**Objective:** Implement network spawn validation and determinism verification interfaces for future multiplayer support and reproducible runs.

**Purpose:** Define the contract for server-authoritative spawn validation (future multiplayer) and spawn state verification for determinism. These interfaces enable the platform to validate spawns and verify reproducible behavior.

**Output:** Two validation interfaces with network and determinism support.

---

## Context

@.planning/PROJECT.md
@.planning/ROADMAP.md
@.planning/phases/01-plugin-architecture-foundation/01-RESEARCH.md
@.planning/phases/01-plugin-architecture-foundation/1-CONTEXT.md

---

## Tasks

<task type="auto">
  <name>T1: Implement IGSDNetworkSpawnValidator Interface</name>
  <files>
    Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDNetworkSpawnValidator.h
    Plugins/GSD_Core/Source/GSD_Core/Private/Interfaces/IGSDNetworkSpawnValidator.cpp
  </files>
  <action>
    Create the network spawn validation interface for server-authoritative spawn control.

    IGSDNetworkSpawnValidator.h:
    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "UObject/Interface.h"
    #include "Types/GSDSpawnTypes.h"
    #include "IGSDNetworkSpawnValidator.generated.h"

    class AActor;

    UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
    class UGSDNetworkSpawnValidator : public UInterface
    {
        GENERATED_BODY()
    };

    class GSD_CORE_API IGSDNetworkSpawnValidator
    {
        GENERATED_BODY()

    public:
        // Called on server to validate spawn request
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Network")
        bool ValidateSpawnRequest(const FGSDSpawnTicket& Request, FString& OutRejectionReason);
        virtual bool ValidateSpawnRequest_Implementation(const FGSDSpawnTicket& Request, FString& OutRejectionReason);

        // Called when spawn completes to notify clients
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Network")
        void NotifySpawnComplete(AActor* SpawnedActor);
        virtual void NotifySpawnComplete_Implementation(AActor* SpawnedActor);

        // Called when spawn is rejected
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Network")
        void NotifySpawnRejected(const FGSDSpawnTicket& Request, const FString& Reason);
        virtual void NotifySpawnRejected_Implementation(const FGSDSpawnTicket& Request, const FString& Reason);

        // Check if this is a server-authoritative spawn
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Network")
        bool IsServerAuthoritative() const;
        virtual bool IsServerAuthoritative_Implementation() const;

        // Get the requesting player controller
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Network")
        APlayerController* GetRequestingPlayer() const;
        virtual APlayerController* GetRequestingPlayer_Implementation() const;

        // Set network spawn parameters
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Network")
        void SetNetworkParams(const FGSDNetworkSpawnParams& Params);
        virtual void SetNetworkParams_Implementation(const FGSDNetworkSpawnParams& Params);
    };
    ```

    IGSDNetworkSpawnValidator.cpp:
    ```cpp
    #include "Interfaces/IGSDNetworkSpawnValidator.h"
    #include "GameFramework/PlayerController.h"

    bool IGSDNetworkSpawnValidator::ValidateSpawnRequest_Implementation(const FGSDSpawnTicket& Request, FString& OutRejectionReason)
    {
        // Default: Allow all spawns
        return true;
    }

    void IGSDNetworkSpawnValidator::NotifySpawnComplete_Implementation(AActor* SpawnedActor)
    {
        // Default implementation - override in C++ or Blueprint
    }

    void IGSDNetworkSpawnValidator::NotifySpawnRejected_Implementation(const FGSDSpawnTicket& Request, const FString& Reason)
    {
        // Default implementation - override in C++ or Blueprint
    }

    bool IGSDNetworkSpawnValidator::IsServerAuthoritative_Implementation() const
    {
        return true; // Default to server-authoritative
    }

    APlayerController* IGSDNetworkSpawnValidator::GetRequestingPlayer_Implementation() const
    {
        return nullptr;
    }

    void IGSDNetworkSpawnValidator::SetNetworkParams_Implementation(const FGSDNetworkSpawnParams& Params)
    {
        // Default implementation - override in C++ or Blueprint
    }
    ```

    Note: Single-player games ignore these interfaces. They are hooks for future multiplayer.
  </action>
  <verify>
    - IGSDNetworkSpawnValidator.h compiles without errors
    - IGSDNetworkSpawnValidator.cpp compiles without errors
    - FGSDSpawnTicket reference parameter works
    - FString& out parameter works with Blueprint
  </verify>
  <done>
    IGSDNetworkSpawnValidator interface exists with ValidateSpawnRequest, NotifySpawnComplete, NotifySpawnRejected, IsServerAuthoritative, GetRequestingPlayer, SetNetworkParams methods.
  </done>
</task>

<task type="auto">
  <name>T2: Implement IGSDSpawnStateValidator Interface</name>
  <files>
    Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDSpawnStateValidator.h
    Plugins/GSD_Core/Source/GSD_Core/Private/Interfaces/IGSDSpawnStateValidator.cpp
  </files>
  <action>
    Create the spawn state validation interface for determinism verification.

    IGSDSpawnStateValidator.h:
    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "UObject/Interface.h"
    #include "Types/GSDSpawnTypes.h"
    #include "IGSDSpawnStateValidator.generated.h"

    UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
    class UGSDSpawnStateValidator : public UInterface
    {
        GENERATED_BODY()
    };

    class GSD_CORE_API IGSDSpawnStateValidator
    {
        GENERATED_BODY()

    public:
        // Validate current spawn state matches recorded state
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Determinism")
        bool ValidateSpawnState(const TArray<FGSDSeededSpawnTicket>& Expected);
        virtual bool ValidateSpawnState_Implementation(const TArray<FGSDSeededSpawnTicket>& Expected);

        // Compute hash of current spawn state
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Determinism")
        int32 ComputeSpawnHash() const;
        virtual int32 ComputeSpawnHash_Implementation() const;

        // Log divergence for debugging
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Determinism")
        void LogDivergence(const FGSDSeededSpawnTicket& Expected, const FGSDSeededSpawnTicket& Actual);
        virtual void LogDivergence_Implementation(const FGSDSeededSpawnTicket& Expected, const FGSDSeededSpawnTicket& Actual);

        // Get current spawn state as tickets
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Determinism")
        void GetCurrentSpawnState(TArray<FGSDSeededSpawnTicket>& OutState) const;
        virtual void GetCurrentSpawnState_Implementation(TArray<FGSDSeededSpawnTicket>& OutState) const;

        // Record spawn for replay
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Determinism")
        void RecordSpawn(const FGSDSeededSpawnTicket& Ticket);
        virtual void RecordSpawn_Implementation(const FGSDSeededSpawnTicket& Ticket);

        // Clear recorded spawns
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Determinism")
        void ClearSpawnHistory();
        virtual void ClearSpawnHistory_Implementation();

        // Get spawn count for this session
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Determinism")
        int32 GetSpawnCount() const;
        virtual int32 GetSpawnCount_Implementation() const;
    };
    ```

    IGSDSpawnStateValidator.cpp:
    ```cpp
    #include "Interfaces/IGSDSpawnStateValidator.h"
    #include "GSDLog.h"

    bool IGSDSpawnStateValidator::ValidateSpawnState_Implementation(const TArray<FGSDSeededSpawnTicket>& Expected)
    {
        // Default: Always passes
        return true;
    }

    int32 IGSDSpawnStateValidator::ComputeSpawnHash_Implementation() const
    {
        return 0;
    }

    void IGSDSpawnStateValidator::LogDivergence_Implementation(const FGSDSeededSpawnTicket& Expected, const FGSDSeededSpawnTicket& Actual)
    {
        GSD_WARN("Spawn divergence detected - Expected Order: %d, Actual Order: %d",
            Expected.SpawnOrder, Actual.SpawnOrder);
    }

    void IGSDSpawnStateValidator::GetCurrentSpawnState_Implementation(TArray<FGSDSeededSpawnTicket>& OutState) const
    {
        OutState.Empty();
    }

    void IGSDSpawnStateValidator::RecordSpawn_Implementation(const FGSDSeededSpawnTicket& Ticket)
    {
        // Default implementation - override in C++ or Blueprint
    }

    void IGSDSpawnStateValidator::ClearSpawnHistory_Implementation()
    {
        // Default implementation - override in C++ or Blueprint
    }

    int32 IGSDSpawnStateValidator::GetSpawnCount_Implementation() const
    {
        return 0;
    }
    ```
  </action>
  <verify>
    - IGSDSpawnStateValidator.h compiles without errors
    - IGSDSpawnStateValidator.cpp compiles without errors
    - TArray<FGSDSeededSpawnTicket> works with Blueprint
    - LogDivergence uses GSD_WARN macro
  </verify>
  <done>
    IGSDSpawnStateValidator interface exists with ValidateSpawnState, ComputeSpawnHash, LogDivergence, GetCurrentSpawnState, RecordSpawn, ClearSpawnHistory, GetSpawnCount methods.
  </done>
</task>

---

## Verification Checklist

- [ ] IGSDNetworkSpawnValidator compiles with all methods
- [ ] IGSDSpawnStateValidator compiles with all methods
- [ ] FGSDSpawnTicket reference parameters work
- [ ] FGSDSeededSpawnTicket array parameters work
- [ ] FString& out parameter compiles correctly
- [ ] GSD_WARN used for divergence logging
- [ ] All generated.h includes are last

---

## Success Criteria

| Criterion | Target |
|-----------|--------|
| Interfaces compile | 100% |
| Network hooks | Defined |
| Determinism hooks | Defined |
| Logging integration | Works |

---

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| Future API changes | Medium | Low | Interfaces are contracts, can extend |
| Complex validation | Low | Low | Default implementations allow all |

---

## Estimated Complexity

**Low** - Interface definitions with default implementations

---

## Output

After completion, create `.planning/phases/01-plugin-architecture-foundation/04-SUMMARY.md`
