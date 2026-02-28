// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "DataAssets/GSDCrowdConfig.h"
#include "GSDCrowdAudioSubsystem.generated.h"

class UAudioComponent;
class USoundBase;
class UMassEntitySubsystem;

/** Audio LOD levels for crowd entities */
UENUM(BlueprintType)
enum class ECrowdAudioLOD : uint8
{
    Full        UMETA(DisplayName = "LOD0 - Full Volume"),
    Reduced     UMETA(DisplayName = "LOD1 - Reduced Volume"),
    Minimal     UMETA(DisplayName = "LOD2 - Minimal Volume"),
    Culled      UMETA(DisplayName = "LOD3 - Culled (Silent)")
};

/** Audio event types for crowd entities */
UENUM(BlueprintType)
enum class ECrowdAudioEvent : uint8
{
    Idle        UMETA(DisplayName = "Idle/Ambient"),
    Walk        UMETA(DisplayName = "Walking"),
    Run         UMETA(DisplayName = "Running"),
    Attack      UMETA(DisplayName = "Attack"),
    Death       UMETA(DisplayName = "Death"),
    Spawn       UMETA(DisplayName = "Spawn"),
    Impact      UMETA(DisplayName = "Impact"),
    Alert       UMETA(DisplayName = "Alert")
};

/** Audio configuration for a crowd entity type */
USTRUCT(BlueprintType)
struct FCrowdAudioConfig
{
    GENERATED_BODY()

    /** Sound to play for each audio event type */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sounds")
    TMap<ECrowdAudioEvent, USoundBase*> EventSounds;

    /** Idle sound loop interval range (min) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timing")
    float IdleSoundIntervalMin = 5.0f;

    /** Idle sound loop interval range (max) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timing")
    float IdleSoundIntervalMax = 15.0f;

    /** Footstep interval when walking */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timing")
    float WalkFootstepInterval = 0.5f;

    /** Footstep interval when running */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timing")
    float RunFootstepInterval = 0.3f;

    /** Base volume for all sounds */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Volume")
    float BaseVolume = 1.0f;

    /** Volume variation (random) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Volume")
    float VolumeVariation = 0.1f;

    /** Pitch variation (random) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pitch")
    float PitchVariation = 0.05f;

    /** Maximum simultaneous sounds per entity type */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limits")
    int32 MaxConcurrentSounds = 10;
};

/** Active audio instance tracking */
USTRUCT()
struct FCrowdAudioInstance
{
    GENERATED_BODY()

    /** Audio component playing the sound */
    UPROPERTY()
    UAudioComponent* AudioComponent = nullptr;

    /** Entity ID this audio belongs to */
    int32 EntityID = INDEX_NONE;

    /** Current LOD level */
    ECrowdAudioLOD CurrentLOD = ECrowdAudioLOD::Full;

    /** Location of the entity */
    FVector Location = FVector::ZeroVector;

    /** Whether this is a looping sound */
    bool bIsLooping = false;

    /** Time until next sound can play */
    float CooldownRemaining = 0.0f;
};

/** Spatial audio cluster for performance */
USTRUCT()
struct FCrowdAudioCluster
{
    GENERATED_BODY()

    /** Center of the cluster */
    FVector ClusterCenter = FVector::ZeroVector;

    /** Radius of the cluster */
    float ClusterRadius = 0.0f;

    /** Number of entities in cluster */
    int32 EntityCount = 0;

    /** Cluster audio component */
    UPROPERTY()
    UAudioComponent* ClusterAudio = nullptr;
};

/**
 * Crowd Audio Subsystem
 * Manages spatial audio for crowd entities with LOD-based optimization.
 *
 * Features:
 * - Distance-based audio LOD (full, reduced, minimal, culled)
 * - Spatial positioning using 3D audio
 * - Sound pooling for performance
 * - Clustering for distant crowds
 * - Volume/pitch variation for natural sound
 */
UCLASS()
class GSD_CROWDS_API UGSDCrowdAudioSubsystem : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Update audio for a crowd entity.
     * Called by processors each frame.
     * @param EntityID The mass entity ID
     * @param Location World location of the entity
     * @param ListenerLocation Current listener (player) location
     * @param AudioEvent The type of audio event to play
     * @param Config Audio configuration to use
     */
    void UpdateEntityAudio(int32 EntityID, const FVector& Location, const FVector& ListenerLocation,
                          ECrowdAudioEvent AudioEvent, const FCrowdAudioConfig& Config);

    /**
     * Play a one-shot sound for a crowd entity.
     * @param EntityID The mass entity ID
     * @param Location World location for spatial audio
     * @param Sound Sound to play
     * @param Volume Volume multiplier
     * @param Config Audio configuration
     */
    void PlayOneShotSound(int32 EntityID, const FVector& Location, USoundBase* Sound,
                          float Volume, const FCrowdAudioConfig& Config);

    /**
     * Remove audio for an entity (when culled/destroyed).
     * @param EntityID The entity to remove
     */
    void RemoveEntityAudio(int32 EntityID);

    /**
     * Calculate audio LOD based on distance.
     * @param Distance Distance from listener
     * @return The appropriate audio LOD level
     */
    ECrowdAudioLOD CalculateAudioLOD(float Distance) const;

    /**
     * Set the crowd config to use for LOD distances.
     * @param Config The crowd configuration
     */
    void SetCrowdConfig(UGSDCrowdConfig* Config) { CachedCrowdConfig = Config; }

    /**
     * Get active audio instance count for debugging.
     */
    int32 GetActiveAudioCount() const { return ActiveAudioInstances.Num(); }

    /**
     * Get clustered audio count for debugging.
     */
    int32 GetClusteredAudioCount() const { return AudioClusters.Num(); }

protected:
    /** Update audio instances each frame */
    void TickAudio(float DeltaTime);

    /** Update audio clusters for distant entities */
    void UpdateAudioClusters();

    /** Apply LOD-based volume adjustment */
    float ApplyLODVolume(float BaseVolume, ECrowdAudioLOD LOD) const;

    /** Create an audio cluster for distant entities */
    void CreateAudioCluster(const FVector& Center, float Radius, int32 EntityCount);

private:
    /** Cached crowd config for LOD distances */
    UPROPERTY()
    TObjectPtr<UGSDCrowdConfig> CachedCrowdConfig;

    /** Active audio instances */
    UPROPERTY()
    TMap<int32, FCrowdAudioInstance> ActiveAudioInstances;

    /** Audio clusters for distant crowds */
    UPROPERTY()
    TArray<FCrowdAudioCluster> AudioClusters;

    /** Audio component pool */
    UPROPERTY()
    TArray<UAudioComponent*> AudioComponentPool;

    /** Available audio components in pool */
    TArray<UAudioComponent*> AvailableAudioComponents;

    /** Total sounds played this frame */
    int32 SoundsPlayedThisFrame = 0;

    /** Maximum sounds per frame budget */
    static constexpr int32 MaxSoundsPerFrame = 20;

    /** Pool size for audio components */
    static constexpr int32 AudioPoolSize = 50;

    /** Handle for tick delegate */
    FDelegateHandle TickHandle;
};
