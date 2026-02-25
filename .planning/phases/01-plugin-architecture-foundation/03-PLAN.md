---
phase: 01-plugin-architecture-foundation
plan: 03
type: execute
wave: 2
depends_on: ["01", "02"]
files_modified:
  - Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDStreamable.h
  - Plugins/GSD_Core/Source/GSD_Core/Private/Interfaces/IGSDStreamable.cpp
  - Plugins/GSD_Core/Source/GSD_Core/Public/Types/GSDSpatialAudioTypes.h
  - Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDSpatialAudioStreamable.h
  - Plugins/GSD_Core/Source/GSD_Core/Private/Interfaces/IGSDSpatialAudioStreamable.cpp
  - Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDMetaSoundInterface.h
  - Plugins/GSD_Core/Source/GSD_Core/Private/Interfaces/IGSDMetaSoundInterface.cpp
autonomous: true
must_haves:
  truths:
    - "Actors can implement IGSDStreamable to receive stream in/out events"
    - "Spatial audio state can be preserved across stream boundaries"
    - "MetaSound integration provides sound source routing"
    - "Audio LOD distance thresholds are configurable"
  artifacts:
    - path: "Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDStreamable.h"
      provides: "Streaming lifecycle interface"
      contains: "OnStreamIn"
    - path: "Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDSpatialAudioStreamable.h"
      provides: "Audio-specific streaming with state preservation"
      contains: "FGSDSpatialAudioState"
    - path: "Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDMetaSoundInterface.h"
      provides: "MetaSound source routing"
      contains: "GetMetaSoundSource"
  key_links:
    - from: "IGSDSpatialAudioStreamable"
      to: "IGSDStreamable"
      via: "Inheritance"
      pattern: "class.*: public IGSDStreamable"
---

# 03-PLAN: Streaming and Audio Interfaces

**Requirements:** PLUG-08, PLUG-09

**Objective:** Implement streaming interfaces (IGSDStreamable, IGSDSpatialAudioStreamable, IGSDMetaSoundInterface) for World Partition integration and audio state management.

**Purpose:** Define the contract for streaming behavior and audio integration. IGSDStreamable provides lifecycle callbacks for World Partition cell loading. IGSDSpatialAudioStreamable preserves audio state across stream boundaries. IGSDMetaSoundInterface provides MetaSound routing.

**Output:** Three streaming-related interfaces with audio state preservation support.

---

## Context

@.planning/PROJECT.md
@.planning/ROADMAP.md
@.planning/phases/01-plugin-architecture-foundation/01-RESEARCH.md
@.planning/phases/01-plugin-architecture-foundation/1-CONTEXT.md

---

## Tasks

<task type="auto">
  <name>T1: Implement IGSDStreamable Interface</name>
  <files>
    Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDStreamable.h
    Plugins/GSD_Core/Source/GSD_Core/Private/Interfaces/IGSDStreamable.cpp
  </files>
  <action>
    Create the streaming lifecycle interface for World Partition integration.

    IGSDStreamable.h:
    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "UObject/Interface.h"
    #include "IGSDStreamable.generated.h"

    UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
    class UGSDStreamable : public UInterface
    {
        GENERATED_BODY()
    };

    class GSD_CORE_API IGSDStreamable
    {
        GENERATED_BODY()

    public:
        // Get streaming bounds for World Partition
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Streaming")
        FBoxSphereBounds GetStreamingBounds();
        virtual FBoxSphereBounds GetStreamingBounds_Implementation();

        // Called when actor streams in (cell loaded)
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Streaming")
        void OnStreamIn();
        virtual void OnStreamIn_Implementation();

        // Called when actor streams out (cell unloaded)
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Streaming")
        void OnStreamOut();
        virtual void OnStreamOut_Implementation();

        // Get streaming priority (higher = more important)
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Streaming")
        int32 GetStreamingPriority();
        virtual int32 GetStreamingPriority_Implementation();

        // Should this actor persist across level changes?
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Streaming")
        bool ShouldPersist();
        virtual bool ShouldPersist_Implementation();

        // Get distance override for streaming (0 = use default)
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Streaming")
        float GetStreamingDistanceOverride();
        virtual float GetStreamingDistanceOverride_Implementation();

        // Is this actor currently streamed in?
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Streaming")
        bool IsCurrentlyStreamedIn() const;
        virtual bool IsCurrentlyStreamedIn_Implementation() const;
    };
    ```

    IGSDStreamable.cpp:
    ```cpp
    #include "Interfaces/IGSDStreamable.h"

    FBoxSphereBounds IGSDStreamable::GetStreamingBounds_Implementation()
    {
        return FBoxSphereBounds(FVector::ZeroVector, FVector(100.0f), 100.0f);
    }

    void IGSDStreamable::OnStreamIn_Implementation()
    {
        // Default implementation - override in C++ or Blueprint
    }

    void IGSDStreamable::OnStreamOut_Implementation()
    {
        // Default implementation - override in C++ or Blueprint
    }

    int32 IGSDStreamable::GetStreamingPriority_Implementation()
    {
        return 0;
    }

    bool IGSDStreamable::ShouldPersist_Implementation()
    {
        return false;
    }

    float IGSDStreamable::GetStreamingDistanceOverride_Implementation()
    {
        return 0.0f;
    }

    bool IGSDStreamable::IsCurrentlyStreamedIn_Implementation() const
    {
        return true;
    }
    ```
  </action>
  <verify>
    - IGSDStreamable.h compiles without errors
    - IGSDStreamable.cpp compiles without errors
    - FBoxSphereBounds return type works with Blueprint
    - OnStreamIn/OnStreamOut events can be overridden in BP
  </verify>
  <done>
    IGSDStreamable interface exists with GetStreamingBounds, OnStreamIn, OnStreamOut, GetStreamingPriority, ShouldPersist, GetStreamingDistanceOverride, IsCurrentlyStreamedIn methods.
  </done>
</task>

<task type="auto">
  <name>T2: Create Spatial Audio Types</name>
  <files>
    Plugins/GSD_Core/Source/GSD_Core/Public/Types/GSDSpatialAudioTypes.h
  </files>
  <action>
    Create the spatial audio state struct for audio state preservation.

    GSDSpatialAudioTypes.h:
    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "GSDSpatialAudioTypes.generated.h"

    // Spatial audio state for preservation across stream boundaries
    USTRUCT(BlueprintType)
    struct GSD_CORE_API FGSDSpatialAudioState
    {
        GENERATED_BODY()

        UPROPERTY(BlueprintReadWrite)
        FVector WorldLocation = FVector::ZeroVector;

        UPROPERTY(BlueprintReadWrite)
        float DistanceToListener = 0.0f;

        UPROPERTY(BlueprintReadWrite)
        float OcclusionFactor = 0.0f;

        UPROPERTY(BlueprintReadWrite)
        float ReverbSendLevel = 0.0f;

        UPROPERTY(BlueprintReadWrite)
        bool bIsOccluded = false;

        UPROPERTY(BlueprintReadWrite)
        float VolumeMultiplier = 1.0f;

        UPROPERTY(BlueprintReadWrite)
        float PitchMultiplier = 1.0f;

        UPROPERTY(BlueprintReadWrite)
        float HighFrequencyGain = 1.0f;

        // Audio LOD level (0 = full quality, 2 = lowest)
        UPROPERTY(BlueprintReadWrite)
        int32 AudioLODLevel = 0;

        void Reset()
        {
            WorldLocation = FVector::ZeroVector;
            DistanceToListener = 0.0f;
            OcclusionFactor = 0.0f;
            ReverbSendLevel = 0.0f;
            bIsOccluded = false;
            VolumeMultiplier = 1.0f;
            PitchMultiplier = 1.0f;
            HighFrequencyGain = 1.0f;
            AudioLODLevel = 0;
        }
    };

    // Audio LOD configuration
    USTRUCT(BlueprintType)
    struct GSD_CORE_API FGSDAudioLODConfig
    {
        GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float LOD0Distance = 500.0f; // 0-500m: Full spatialization

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float LOD1Distance = 2000.0f; // 500-2000m: Reduced spatialization

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float LOD2Distance = 5000.0f; // 2000-5000m: Mono/attenuation only

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 MaxConcurrentLOD0 = 32;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 MaxConcurrentLOD1 = 16;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 MaxConcurrentLOD2 = 8;

        int32 GetLODLevelForDistance(float Distance) const
        {
            if (Distance < LOD0Distance) return 0;
            if (Distance < LOD1Distance) return 1;
            if (Distance < LOD2Distance) return 2;
            return -1; // Virtual (stopped)
        }
    };
    ```
  </action>
  <verify>
    - GSDSpatialAudioTypes.h compiles without errors
    - FGSDSpatialAudioState has all required fields
    - FGSDAudioLODConfig::GetLODLevelForDistance works correctly
    - Structs are BlueprintType
  </verify>
  <done>
    Spatial audio types exist with FGSDSpatialAudioState for state preservation and FGSDAudioLODConfig for distance-based LOD.
  </done>
</task>

<task type="auto">
  <name>T3: Implement IGSDSpatialAudioStreamable Interface</name>
  <files>
    Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDSpatialAudioStreamable.h
    Plugins/GSD_Core/Source/GSD_Core/Private/Interfaces/IGSDSpatialAudioStreamable.cpp
  </files>
  <action>
    Create the audio-specific streaming interface that inherits from IGSDStreamable.

    IGSDSpatialAudioStreamable.h:
    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "UObject/Interface.h"
    #include "Interfaces/IGSDStreamable.h"
    #include "Types/GSDSpatialAudioTypes.h"
    #include "IGSDSpatialAudioStreamable.generated.h"

    UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
    class UGSDSpatialAudioStreamable : public UGSDStreamable
    {
        GENERATED_BODY()
    };

    class GSD_CORE_API IGSDSpatialAudioStreamable : public IGSDStreamable
    {
        GENERATED_BODY()

    public:
        // Get spatial state before stream out (for preservation)
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
        FGSDSpatialAudioState GetSpatialState() const;
        virtual FGSDSpatialAudioState GetSpatialState_Implementation() const;

        // Restore spatial state on stream in
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
        void RestoreSpatialState(const FGSDSpatialAudioState& State);
        virtual void RestoreSpatialState_Implementation(const FGSDSpatialAudioState& State);

        // Distance threshold for audio LOD
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
        float GetAudioLODDistance() const;
        virtual float GetAudioLODDistance_Implementation() const;

        // Get current audio LOD level
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
        int32 GetCurrentAudioLOD() const;
        virtual int32 GetCurrentAudioLOD_Implementation() const;

        // Set audio LOD level
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
        void SetAudioLOD(int32 LODLevel);
        virtual void SetAudioLOD_Implementation(int32 LODLevel);

        // Is audio currently playing?
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
        bool IsAudioPlaying() const;
        virtual bool IsAudioPlaying_Implementation() const;

        // Stop audio (for virtualization)
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
        void VirtualizeAudio();
        virtual void VirtualizeAudio_Implementation();

        // Resume audio from virtualized state
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
        void DevirtualizeAudio();
        virtual void DevirtualizeAudio_Implementation();
    };
    ```

    IGSDSpatialAudioStreamable.cpp:
    ```cpp
    #include "Interfaces/IGSDSpatialAudioStreamable.h"

    FGSDSpatialAudioState IGSDSpatialAudioStreamable::GetSpatialState_Implementation() const
    {
        return FGSDSpatialAudioState();
    }

    void IGSDSpatialAudioStreamable::RestoreSpatialState_Implementation(const FGSDSpatialAudioState& State)
    {
        // Default implementation - override in C++ or Blueprint
    }

    float IGSDSpatialAudioStreamable::GetAudioLODDistance_Implementation() const
    {
        return 500.0f; // Default to LOD0 distance
    }

    int32 IGSDSpatialAudioStreamable::GetCurrentAudioLOD_Implementation() const
    {
        return 0;
    }

    void IGSDSpatialAudioStreamable::SetAudioLOD_Implementation(int32 LODLevel)
    {
        // Default implementation - override in C++ or Blueprint
    }

    bool IGSDSpatialAudioStreamable::IsAudioPlaying_Implementation() const
    {
        return false;
    }

    void IGSDSpatialAudioStreamable::VirtualizeAudio_Implementation()
    {
        // Default implementation - override in C++ or Blueprint
    }

    void IGSDSpatialAudioStreamable::DevirtualizeAudio_Implementation()
    {
        // Default implementation - override in C++ or Blueprint
    }
    ```
  </action>
  <verify>
    - IGSDSpatialAudioStreamable.h compiles without errors
    - IGSDSpatialAudioStreamable.cpp compiles without errors
    - Interface inherits from IGSDStreamable
    - FGSDSpatialAudioState parameter works with Blueprint
  </verify>
  <done>
    IGSDSpatialAudioStreamable interface exists with GetSpatialState, RestoreSpatialState, GetAudioLODDistance, GetCurrentAudioLOD, SetAudioLOD, IsAudioPlaying, VirtualizeAudio, DevirtualizeAudio methods. Inherits from IGSDStreamable.
  </done>
</task>

<task type="auto">
  <name>T4: Implement IGSDMetaSoundInterface</name>
  <files>
    Plugins/GSD_Core/Source/GSD_Core/Public/Interfaces/IGSDMetaSoundInterface.h
    Plugins/GSD_Core/Source/GSD_Core/Private/Interfaces/IGSDMetaSoundInterface.cpp
  </files>
  <action>
    Create the MetaSound integration interface for sound source routing.

    IGSDMetaSoundInterface.h:
    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "UObject/Interface.h"
    #include "IGSDMetaSoundInterface.generated.h"

    class USoundSubmix;
    class USoundClass;

    UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
    class UGSDMetaSoundInterface : public UInterface
    {
        GENERATED_BODY()
    };

    class GSD_CORE_API IGSDMetaSoundInterface
    {
        GENERATED_BODY()

    public:
        // Get the MetaSound source for spawning
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
        UObject* GetMetaSoundSource() const;
        virtual UObject* GetMetaSoundSource_Implementation() const;

        // Default parameters to pass on spawn
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
        TMap<FName, float> GetDefaultParameters() const;
        virtual TMap<FName, float> GetDefaultParameters_Implementation() const;

        // Output submix routing
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
        USoundSubmix* GetTargetSubmix() const;
        virtual USoundSubmix* GetTargetSubmix_Implementation() const;

        // Sound class for volume/priority
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
        USoundClass* GetSoundClass() const;
        virtual USoundClass* GetSoundClass_Implementation() const;

        // Set a runtime parameter on the MetaSound
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
        void SetFloatParameter(FName ParameterName, float Value);
        virtual void SetFloatParameter_Implementation(FName ParameterName, float Value);

        // Get a runtime parameter from the MetaSound
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
        bool GetFloatParameter(FName ParameterName, float& OutValue) const;
        virtual bool GetFloatParameter_Implementation(FName ParameterName, float& OutValue) const;
    };
    ```

    IGSDMetaSoundInterface.cpp:
    ```cpp
    #include "Interfaces/IGSDMetaSoundInterface.h"

    UObject* IGSDMetaSoundInterface::GetMetaSoundSource_Implementation() const
    {
        return nullptr;
    }

    TMap<FName, float> IGSDMetaSoundInterface::GetDefaultParameters_Implementation() const
    {
        return TMap<FName, float>();
    }

    USoundSubmix* IGSDMetaSoundInterface::GetTargetSubmix_Implementation() const
    {
        return nullptr;
    }

    USoundClass* IGSDMetaSoundInterface::GetSoundClass_Implementation() const
    {
        return nullptr;
    }

    void IGSDMetaSoundInterface::SetFloatParameter_Implementation(FName ParameterName, float Value)
    {
        // Default implementation - override in C++ or Blueprint
    }

    bool IGSDMetaSoundInterface::GetFloatParameter_Implementation(FName ParameterName, float& OutValue) const
    {
        OutValue = 0.0f;
        return false;
    }
    ```

    Note: This interface will be fully implemented when MetaSound integration is needed. For now, it provides the contract.
  </action>
  <verify>
    - IGSDMetaSoundInterface.h compiles without errors
    - IGSDMetaSoundInterface.cpp compiles without errors
    - TMap<FName, float> works with Blueprint
    - Forward declarations for USoundSubmix/USoundClass correct
  </verify>
  <done>
    IGSDMetaSoundInterface exists with GetMetaSoundSource, GetDefaultParameters, GetTargetSubmix, GetSoundClass, SetFloatParameter, GetFloatParameter methods. Provides contract for MetaSound integration.
  </done>
</task>

---

## Verification Checklist

- [ ] IGSDStreamable compiles with all lifecycle methods
- [ ] FGSDSpatialAudioState has all required fields
- [ ] FGSDAudioLODConfig::GetLODLevelForDistance returns correct levels
- [ ] IGSDSpatialAudioStreamable inherits from IGSDStreamable
- [ ] IGSDMetaSoundInterface compiles with TMap<FName, float>
- [ ] All generated.h includes are last
- [ ] No circular dependencies

---

## Success Criteria

| Criterion | Target |
|-----------|--------|
| Interfaces compile | 100% |
| Inheritance works | Verified |
| Audio state preservation | Struct defined |
| MetaSound contract | Defined |

---

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| Interface inheritance | Low | Medium | Test with Blueprint |
| Forward declarations | Low | Low | Verify compilation |
| MetaSound types | Low | Low | Forward declare, implement later |

---

## Estimated Complexity

**Medium** - Interface inheritance and audio state management

---

## Output

After completion, create `.planning/phases/01-plugin-architecture-foundation/03-SUMMARY.md`
