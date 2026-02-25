---
phase: 01-plugin-architecture-foundation
plan: 05
type: execute
wave: 3
depends_on: ["01", "02", "03", "04"]
files_modified:
  - Plugins/GSD_Core/Source/GSD_Core/Public/Classes/GSDActor.h
  - Plugins/GSD_Core/Source/GSD_Core/Private/Classes/GSDActor.cpp
  - Plugins/GSD_Core/Source/GSD_Core/Public/Classes/GSDComponent.h
  - Plugins/GSD_Core/Source/GSD_Core/Private/Classes/GSDComponent.cpp
  - Plugins/GSD_Core/Source/GSD_Core/Public/DataAssets/GSDDataAsset.h
  - Plugins/GSD_Core/Source/GSD_Core/Private/DataAssets/GSDDataAsset.cpp
autonomous: true
must_haves:
  truths:
    - "All GSD actors inherit from AGSDActor"
    - "All GSD components inherit from UGSDComponent"
    - "All GSD config uses UGSDDataAsset derivatives"
    - "Base classes implement core interfaces"
  artifacts:
    - path: "Plugins/GSD_Core/Source/GSD_Core/Public/Classes/GSDActor.h"
      provides: "Base class for all GSD actors"
      contains: "class.*AGSDActor : public AActor"
    - path: "Plugins/GSD_Core/Source/GSD_Core/Public/Classes/GSDComponent.h"
      provides: "Base class for all GSD components"
      contains: "class.*UGSDComponent : public UActorComponent"
    - path: "Plugins/GSD_Core/Source/GSD_Core/Public/DataAssets/GSDDataAsset.h"
      provides: "Base class for all GSD config"
      contains: "class.*UGSDDataAsset : public UPrimaryDataAsset"
  key_links:
    - from: "AGSDActor"
      to: "IGSDSpawnable"
      via: "Interface implementation"
      pattern: "public IGSDSpawnable"
    - from: "UGSDDataAsset"
      to: "SpawnAsync"
      via: "Config parameter"
      pattern: "UGSDDataAsset\\*"
---

# 05-PLAN: Base Classes

**Requirements:** PLUG-01, PLUG-08

**Objective:** Implement the base classes (AGSDActor, UGSDComponent, UGSDDataAsset) that all GSD types inherit from, implementing core interfaces.

**Purpose:** Provide opinionated base classes that enforce the GSD way of doing things. All GSD actors inherit from AGSDActor, all components from UGSDComponent, all config from UGSDDataAsset.

**Output:** Three base classes implementing core interfaces with common functionality.

---

## Context

@.planning/PROJECT.md
@.planning/ROADMAP.md
@.planning/phases/01-plugin-architecture-foundation/01-RESEARCH.md
@.planning/phases/01-plugin-architecture-foundation/1-CONTEXT.md

---

## Tasks

<task type="auto">
  <name>T1: Implement AGSDActor Base Class</name>
  <files>
    Plugins/GSD_Core/Source/GSD_Core/Public/Classes/GSDActor.h
    Plugins/GSD_Core/Source/GSD_Core/Private/Classes/GSDActor.cpp
  </files>
  <action>
    Create the base actor class that implements IGSDSpawnable and IGSDStreamable.

    GSDActor.h:
    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "GameFramework/Actor.h"
    #include "Interfaces/IGSDSpawnable.h"
    #include "Interfaces/IGSDStreamable.h"
    #include "GSDActor.generated.h"

    class UGSDDataAsset;
    class UGSDComponent;

    UCLASS(Abstract, Blueprintable, Category = "GSD")
    class GSD_CORE_API AGSDActor : public AActor, public IGSDSpawnable, public IGSDStreamable
    {
        GENERATED_BODY()

    public:
        AGSDActor();

        // ~ IGSDSpawnable interface
        virtual void SpawnAsync_Implementation(UGSDDataAsset* Config, FOnSpawnComplete Callback) override;
        virtual UGSDDataAsset* GetSpawnConfig_Implementation() override;
        virtual bool IsSpawned_Implementation() override;
        virtual void Despawn_Implementation() override;
        virtual void ResetSpawnState_Implementation() override;

        // ~ IGSDStreamable interface
        virtual FBoxSphereBounds GetStreamingBounds_Implementation() override;
        virtual void OnStreamIn_Implementation() override;
        virtual void OnStreamOut_Implementation() override;
        virtual int32 GetStreamingPriority_Implementation() override;
        virtual bool ShouldPersist_Implementation() override;

        // Common GSD actor functionality
        UFUNCTION(BlueprintCallable, Category = "GSD|Actor")
        UGSDDataAsset* GetGSDConfig() const { return GSDConfig; }

        UFUNCTION(BlueprintCallable, Category = "GSD|Actor")
        void SetGSDConfig(UGSDDataAsset* InConfig) { GSDConfig = InConfig; }

        UFUNCTION(BlueprintCallable, Category = "GSD|Actor")
        bool GetIsSpawned() const { return bIsSpawned; }

        UFUNCTION(BlueprintCallable, Category = "GSD|Actor")
        bool GetIsStreamedIn() const { return bIsStreamedIn; }

    protected:
        // The GSD config asset for this actor
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GSD")
        UGSDDataAsset* GSDConfig = nullptr;

        // Spawn state
        UPROPERTY(BlueprintReadOnly, Category = "GSD")
        bool bIsSpawned = false;

        // Streaming state
        UPROPERTY(BlueprintReadOnly, Category = "GSD")
        bool bIsStreamedIn = true;

        // Called when config is applied
        UFUNCTION(BlueprintNativeEvent, Category = "GSD|Actor")
        void OnConfigApplied(UGSDDataAsset* Config);
        virtual void OnConfigApplied_Implementation(UGSDDataAsset* Config);

        // Called when spawn completes
        UFUNCTION(BlueprintNativeEvent, Category = "GSD|Actor")
        void OnSpawnComplete();
        virtual void OnSpawnComplete_Implementation();

        // Called when despawn starts
        UFUNCTION(BlueprintNativeEvent, Category = "GSD|Actor")
        void OnDespawnStart();
        virtual void OnDespawnStart_Implementation();
    };
    ```

    GSDActor.cpp:
    ```cpp
    #include "Classes/GSDActor.h"
    #include "DataAssets/GSDDataAsset.h"
    #include "GSDLog.h"

    AGSDActor::AGSDActor()
    {
        PrimaryActorTick.bCanEverTick = false;
    }

    void AGSDActor::SpawnAsync_Implementation(UGSDDataAsset* Config, FOnSpawnComplete Callback)
    {
        GSDConfig = Config;
        bIsSpawned = true;

        OnConfigApplied(Config);
        OnSpawnComplete();

        GSD_LOG(Log, "AGSDActor %s spawned with config %s",
            *GetName(), Config ? *Config->GetName() : TEXT("nullptr"));

        if (Callback.IsBound())
        {
            Callback.Execute(this);
        }
    }

    UGSDDataAsset* AGSDActor::GetSpawnConfig_Implementation()
    {
        return GSDConfig;
    }

    bool AGSDActor::IsSpawned_Implementation()
    {
        return bIsSpawned;
    }

    void AGSDActor::Despawn_Implementation()
    {
        OnDespawnStart();
        bIsSpawned = false;
        GSD_LOG(Log, "AGSDActor %s despawned", *GetName());
    }

    void AGSDActor::ResetSpawnState_Implementation()
    {
        bIsSpawned = false;
        GSDConfig = nullptr;
    }

    FBoxSphereBounds AGSDActor::GetStreamingBounds_Implementation()
    {
        return FBoxSphereBounds(GetActorBounds(true));
    }

    void AGSDActor::OnStreamIn_Implementation()
    {
        bIsStreamedIn = true;
        GSD_LOG(Verbose, "AGSDActor %s streamed in", *GetName());
    }

    void AGSDActor::OnStreamOut_Implementation()
    {
        bIsStreamedIn = false;
        GSD_LOG(Verbose, "AGSDActor %s streamed out", *GetName());
    }

    int32 AGSDActor::GetStreamingPriority_Implementation()
    {
        return 0;
    }

    bool AGSDActor::ShouldPersist_Implementation()
    {
        return false;
    }

    void AGSDActor::OnConfigApplied_Implementation(UGSDDataAsset* Config)
    {
        // Override in derived classes
    }

    void AGSDActor::OnSpawnComplete_Implementation()
    {
        // Override in derived classes
    }

    void AGSDActor::OnDespawnStart_Implementation()
    {
        // Override in derived classes
    }
    ```
  </action>
  <verify>
    - AGSDActor compiles without errors
    - IGSDSpawnable methods are implemented
    - IGSDStreamable methods are implemented
    - Blueprint can inherit from AGSDActor
    - OnSpawnComplete delegate executes
  </verify>
  <done>
    AGSDActor base class exists implementing IGSDSpawnable and IGSDStreamable with config management, spawn state, and streaming state tracking.
  </done>
</task>

<task type="auto">
  <name>T2: Implement UGSDComponent Base Class</name>
  <files>
    Plugins/GSD_Core/Source/GSD_Core/Public/Classes/GSDComponent.h
    Plugins/GSD_Core/Source/GSD_Core/Private/Classes/GSDComponent.cpp
  </files>
  <action>
    Create the base component class with common GSD functionality.

    GSDComponent.h:
    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "Components/ActorComponent.h"
    #include "Interfaces/IGSDComponentSpawnable.h"
    #include "GSDComponent.generated.h"

    class UGSDDataAsset;

    UCLASS(Abstract, Blueprintable, ClassGroup = "GSD", meta = (BlueprintSpawnableComponent))
    class GSD_CORE_API UGSDComponent : public UActorComponent, public IGSDComponentSpawnable
    {
        GENERATED_BODY()

    public:
        UGSDComponent();

        // ~ IGSDComponentSpawnable interface
        virtual UActorComponent* SpawnComponent_Implementation(AActor* Owner, const FTransform& Transform) override;
        virtual FName GetComponentType_Implementation() const override;
        virtual bool IsComponentActive_Implementation() const override;
        virtual void DeactivateComponent_Implementation() override;
        virtual void DestroyComponent_Implementation() override;

        // Common GSD component functionality
        UFUNCTION(BlueprintCallable, Category = "GSD|Component")
        UGSDDataAsset* GetGSDConfig() const { return GSDConfig; }

        UFUNCTION(BlueprintCallable, Category = "GSD|Component")
        void SetGSDConfig(UGSDDataAsset* InConfig) { GSDConfig = InConfig; }

        UFUNCTION(BlueprintPure, Category = "GSD|Component")
        bool IsGSDActive() const { return bGSDActive; }

    protected:
        // The GSD config asset for this component
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GSD")
        UGSDDataAsset* GSDConfig = nullptr;

        // GSD-specific active state
        UPROPERTY(BlueprintReadOnly, Category = "GSD")
        bool bGSDActive = false;

        // Called when config is applied
        UFUNCTION(BlueprintNativeEvent, Category = "GSD|Component")
        void OnConfigApplied(UGSDDataAsset* Config);
        virtual void OnConfigApplied_Implementation(UGSDDataAsset* Config);

        // Called when component activates
        UFUNCTION(BlueprintNativeEvent, Category = "GSD|Component")
        void OnGSDActivate();
        virtual void OnGSDActivate_Implementation();

        // Called when component deactivates
        UFUNCTION(BlueprintNativeEvent, Category = "GSD|Component")
        void OnGSDDeactivate();
        virtual void OnGSDDeactivate_Implementation();
    };
    ```

    GSDComponent.cpp:
    ```cpp
    #include "Classes/GSDComponent.h"
    #include "DataAssets/GSDDataAsset.h"
    #include "GSDLog.h"

    UGSDComponent::UGSDComponent()
    {
        PrimaryComponentTick.bCanEverTick = false;
    }

    UActorComponent* UGSDComponent::SpawnComponent_Implementation(AActor* Owner, const FTransform& Transform)
    {
        if (!Owner)
        {
            GSD_WARN("UGSDComponent::SpawnComponent called with null Owner");
            return nullptr;
        }

        // Components are created via NewObject and registered
        // This is a default implementation - override for specific behavior
        bGSDActive = true;
        OnGSDActivate();

        GSD_LOG(Log, "UGSDComponent %s spawned on actor %s", *GetName(), *Owner->GetName());
        return this;
    }

    FName UGSDComponent::GetComponentType_Implementation() const
    {
        return GetClass()->GetFName();
    }

    bool UGSDComponent::IsComponentActive_Implementation() const
    {
        return bGSDActive && IsActive();
    }

    void UGSDComponent::DeactivateComponent_Implementation()
    {
        OnGSDDeactivate();
        bGSDActive = false;
        GSD_LOG(Log, "UGSDComponent %s deactivated", *GetName());
    }

    void UGSDComponent::DestroyComponent_Implementation()
    {
        DeactivateComponent_Implementation();
        DestroyComponent();
    }

    void UGSDComponent::OnConfigApplied_Implementation(UGSDDataAsset* Config)
    {
        // Override in derived classes
    }

    void UGSDComponent::OnGSDActivate_Implementation()
    {
        // Override in derived classes
    }

    void UGSDComponent::OnGSDDeactivate_Implementation()
    {
        // Override in derived classes
    }
    ```
  </action>
  <verify>
    - UGSDComponent compiles without errors
    - IGSDComponentSpawnable methods are implemented
    - Blueprint can inherit from UGSDComponent
    - BlueprintSpawnableComponent meta works
  </verify>
  <done>
    UGSDComponent base class exists implementing IGSDComponentSpawnable with config management and activation state tracking.
  </done>
</task>

<task type="auto">
  <name>T3: Implement UGSDDataAsset Base Class</name>
  <files>
    Plugins/GSD_Core/Source/GSD_Core/Public/DataAssets/GSDDataAsset.h
    Plugins/GSD_Core/Source/GSD_Core/Private/DataAssets/GSDDataAsset.cpp
  </files>
  <action>
    Create the base data asset class for all GSD configuration.

    GSDDataAsset.h:
    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "Engine/DataAsset.h"
    #include "GSDDataAsset.generated.h"

    /**
     * Base class for all GSD configuration data assets.
     * All spawn configs, vehicle configs, crowd configs, etc. inherit from this.
     */
    UCLASS(Abstract, BlueprintType, Category = "GSD")
    class GSD_CORE_API UGSDDataAsset : public UDataAsset
    {
        GENERATED_BODY()

    public:
        UGSDDataAsset();

        // Get the display name for this config
        UFUNCTION(BlueprintPure, Category = "GSD")
        FString GetDisplayName() const;

        // Get the config version for migration support
        UFUNCTION(BlueprintPure, Category = "GSD")
        int32 GetConfigVersion() const { return ConfigVersion; }

        // Validate the config data
        UFUNCTION(BlueprintCallable, Category = "GSD")
        virtual bool ValidateConfig(FString& OutError) const;

        // Called after loading to apply any migrations
        UFUNCTION(BlueprintCallable, Category = "GSD")
        virtual void ApplyMigrations();

        // Get tags associated with this config
        UFUNCTION(BlueprintPure, Category = "GSD")
        const TArray<FName>& GetConfigTags() const { return ConfigTags; }

        // Check if config has a specific tag
        UFUNCTION(BlueprintPure, Category = "GSD")
        bool HasTag(FName Tag) const;

        // Get priority for spawning (higher = more important)
        UFUNCTION(BlueprintPure, Category = "GSD")
        int32 GetSpawnPriority() const { return SpawnPriority; }

    protected:
        // Human-readable name for this config
        UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GSD")
        FString DisplayName;

        // Version number for config migration
        UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GSD")
        int32 ConfigVersion = 1;

        // Tags for categorization and filtering
        UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GSD")
        TArray<FName> ConfigTags;

        // Priority for spawn ordering
        UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GSD")
        int32 SpawnPriority = 0;

        // Description for documentation
        UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GSD", meta = (MultiLine = true))
        FString Description;
    };
    ```

    GSDDataAsset.cpp:
    ```cpp
    #include "DataAssets/GSDDataAsset.h"
    #include "GSDLog.h"

    UGSDDataAsset::UGSDDataAsset()
    {
        DisplayName = TEXT("GSD Config");
    }

    FString UGSDDataAsset::GetDisplayName() const
    {
        if (!DisplayName.IsEmpty())
        {
            return DisplayName;
        }
        return GetName();
    }

    bool UGSDDataAsset::ValidateConfig(FString& OutError) const
    {
        // Base validation - override in derived classes
        OutError.Empty();
        return true;
    }

    void UGSDDataAsset::ApplyMigrations()
    {
        // Override in derived classes to handle version migrations
        GSD_LOG(Verbose, "UGSDDataAsset::ApplyMigrations for %s (version %d)",
            *GetName(), ConfigVersion);
    }

    bool UGSDDataAsset::HasTag(FName Tag) const
    {
        return ConfigTags.Contains(Tag);
    }
    ```
  </action>
  <verify>
    - UGSDDataAsset compiles without errors
    - Blueprint can create data assets inheriting from UGSDDataAsset
    - ValidateConfig returns true by default
    - HasTag works with tag array
  </verify>
  <done>
    UGSDDataAsset base class exists with display name, version, tags, validation, and migration support.
  </done>
</task>

---

## Verification Checklist

- [ ] AGSDActor compiles and implements IGSDSpawnable
- [ ] AGSDActor compiles and implements IGSDStreamable
- [ ] UGSDComponent compiles and implements IGSDComponentSpawnable
- [ ] UGSDDataAsset compiles with validation support
- [ ] Blueprint can inherit from all base classes
- [ ] Config management works across all classes
- [ ] GSD_LOG used for logging
- [ ] All generated.h includes are last

---

## Success Criteria

| Criterion | Target |
|-----------|--------|
| Base classes compile | 100% |
| Interface implementations | Complete |
| Blueprint support | Verified |
| Config management | Works |

---

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| Multiple inheritance issues | Low | Medium | Use interface inheritance |
| Blueprint compatibility | Low | Medium | Test with BP creation |

---

## Estimated Complexity

**Medium** - Multiple base classes with interface implementations

---

## Output

After completion, create `.planning/phases/01-plugin-architecture-foundation/05-SUMMARY.md`
