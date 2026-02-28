// Copyright Bret Bouchard. All Rights Reserved.

#include "Audio/GSDCrowdAudioSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UGSDCrowdAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Pre-allocate audio component pool
    AudioComponentPool.Reserve(AudioPoolSize);
    AvailableAudioComponents.Reserve(AudioPoolSize);

    // Create initial pool of audio components
    if (UWorld* World = GetWorld())
    {
        for (int32 i = 0; i < AudioPoolSize; ++i)
        {
            UAudioComponent* AudioComp = NewObject<UAudioComponent>(this);
            AudioComp->SetComponentTickEnabled(false);
            AudioCompPool.Add(AudioComp);
            AvailableAudioComponents.Add(AudioComp);
        }
    }
}

void UGSDCrowdAudioSubsystem::Deinitialize()
{
    // Clean up all audio components
    for (auto& Pair : ActiveAudioInstances)
    {
        if (UAudioComponent* AudioComp = Pair.Value.AudioComponent)
        {
            AudioComp->Stop();
            AudioComp->DestroyComponent();
        }
    }
    ActiveAudioInstances.Empty();

    for (UAudioComponent* AudioComp : AudioComponentPool)
    {
        if (AudioComp)
        {
            AudioComp->DestroyComponent();
        }
    }
    AudioComponentPool.Empty();
    AvailableAudioComponents.Empty();

    for (FCrowdAudioCluster& Cluster : AudioClusters)
    {
        if (Cluster.ClusterAudio)
        {
            Cluster.ClusterAudio->Stop();
            Cluster.ClusterAudio->DestroyComponent();
        }
    }
    AudioClusters.Empty();

    Super::Deinitialize();
}

void UGSDCrowdAudioSubsystem::UpdateEntityAudio(int32 EntityID, const FVector& Location,
                                                 const FVector& ListenerLocation,
                                                 ECrowdAudioEvent AudioEvent,
                                                 const FCrowdAudioConfig& Config)
{
    const float Distance = FVector::Dist(Location, ListenerLocation);
    const ECrowdAudioLOD NewLOD = CalculateAudioLOD(Distance);

    // Skip if culled
    if (NewLOD == ECrowdAudioLOD::Culled)
    {
        RemoveEntityAudio(EntityID);
        return;
    }

    // Check if entity already has active audio
    FCrowdAudioInstance* Instance = ActiveAudioInstances.Find(EntityID);
    if (Instance)
    {
        // Update LOD if changed
        if (Instance->CurrentLOD != NewLOD)
        {
            Instance->CurrentLOD = NewLOD;

            // Apply new volume based on LOD
            if (Instance->AudioComponent)
            {
                const float NewVolume = ApplyLODVolume(Config.BaseVolume, NewLOD);
                Instance->AudioComponent->SetVolumeMultiplier(NewVolume);
            }
        }

        // Update location for spatial audio
        Instance->Location = Location;
        if (Instance->AudioComponent)
        {
            Instance->AudioComponent->SetWorldLocation(Location);
        }

        // Update cooldown
        if (Instance->CooldownRemaining > 0.0f)
        {
            return; // Still on cooldown
        }
    }

    // Play sound if we have one for this event
    if (USoundBase* const* SoundPtr = Config.EventSounds.Find(AudioEvent))
    {
        if (USoundBase* Sound = *SoundPtr)
        {
            PlayOneShotSound(EntityID, Location, Sound, Config.BaseVolume, Config);
        }
    }
}

void UGSDCrowdAudioSubsystem::PlayOneShotSound(int32 EntityID, const FVector& Location,
                                                USoundBase* Sound, float Volume,
                                                const FCrowdAudioConfig& Config)
{
    if (!Sound || SoundsPlayedThisFrame >= MaxSoundsPerFrame)
    {
        return;
    }

    // Calculate distance-based LOD
    FVector ListenerLocation;
    if (GEngine && GEngine->GetFirstLocalPlayerController(GetWorld()))
    {
        ListenerLocation = GEngine->GetFirstLocalPlayerController(GetWorld())->GetPawnOrSpectator()->GetActorLocation();
    }
    else
    {
        ListenerLocation = FVector::ZeroVector;
    }

    const float Distance = FVector::Dist(Location, ListenerLocation);
    const ECrowdAudioLOD LOD = CalculateAudioLOD(Distance);

    if (LOD == ECrowdAudioLOD::Culled)
    {
        return;
    }

    // Apply LOD volume
    float FinalVolume = ApplyLODVolume(Volume, LOD);

    // Add variation (use seeded random for determinism)
    if (Config.VolumeVariation > 0.0f)
    {
        // Use seeded random - fallback to static seed if no manager
        static FRandomStream AudioVolumeStream(44455);
        FinalVolume *= 1.0f + (AudioVolumeStream.FRand() - 0.5f) * 2.0f * Config.VolumeVariation;
    }

    // Get or create audio instance
    FCrowdAudioInstance& Instance = ActiveAudioInstances.FindOrAdd(EntityID);
    Instance.EntityID = EntityID;
    Instance.Location = Location;
    Instance.CurrentLOD = LOD;

    // Get pooled audio component
    UAudioComponent* AudioComp = nullptr;
    if (AvailableAudioComponents.Num() > 0)
    {
        AudioComp = AvailableAudioComponents.Pop();
    }
    else
    {
        // Create new if pool exhausted
        AudioComp = NewObject<UAudioComponent>(this);
        AudioComponentPool.Add(AudioComp);
    }

    if (AudioComp)
    {
        Instance.AudioComponent = AudioComp;

        // Configure spatial audio
        AudioComp->SetSound(Sound);
        AudioComp->SetWorldLocation(Location);
        AudioComp->SetVolumeMultiplier(FinalVolume);

        // Apply pitch variation (use seeded random for determinism)
        if (Config.PitchVariation > 0.0f)
        {
            // Use seeded random - fallback to static seed if no manager
            static FRandomStream AudioPitchStream(55544);
            const float PitchMod = 1.0f + (AudioPitchStream.FRand() - 0.5f) * 2.0f * Config.PitchVariation;
            AudioComp->SetPitchMultiplier(PitchMod);
        }

        // Enable 3D spatialization
        AudioComp->SetIsSpatialized(true);
        AudioComp->SetAttenuationSettings(nullptr); // Use default attenuation

        AudioComp->Play();
        SoundsPlayedThisFrame++;

        // Set cooldown based on sound type
        Instance.CooldownRemaining = 0.5f; // Minimum cooldown between sounds
    }
}

void UGSDCrowdAudioSubsystem::RemoveEntityAudio(int32 EntityID)
{
    if (FCrowdAudioInstance* Instance = ActiveAudioInstances.Find(EntityID))
    {
        if (UAudioComponent* AudioComp = Instance->AudioComponent)
        {
            AudioComp->Stop();
            AvailableAudioComponents.Add(AudioComp);
        }
        ActiveAudioInstances.Remove(EntityID);
    }
}

ECrowdAudioLOD UGSDCrowdAudioSubsystem::CalculateAudioLOD(float Distance) const
{
    // Use config if available, otherwise defaults
    float LOD0Dist = 500.0f;
    float LOD1Dist = 2000.0f;
    float LOD2Dist = 4000.0f;
    float CullDist = 5000.0f;

    if (CachedCrowdConfig)
    {
        LOD0Dist = CachedCrowdConfig->AudioLOD0Distance;
        LOD1Dist = CachedCrowdConfig->AudioLOD1Distance;
        LOD2Dist = CachedCrowdConfig->AudioLOD2Distance;
        CullDist = CachedCrowdConfig->AudioCullDistance;
    }

    if (Distance < LOD0Dist)
    {
        return ECrowdAudioLOD::Full;
    }
    else if (Distance < LOD1Dist)
    {
        return ECrowdAudioLOD::Reduced;
    }
    else if (Distance < LOD2Dist)
    {
        return ECrowdAudioLOD::Minimal;
    }
    else
    {
        return ECrowdAudioLOD::Culled;
    }
}

void UGSDCrowdAudioSubsystem::TickAudio(float DeltaTime)
{
    // Reset frame counter
    SoundsPlayedThisFrame = 0;

    // Update cooldowns and cleanup finished sounds
    TArray<int32> ToRemove;
    for (auto& Pair : ActiveAudioInstances)
    {
        FCrowdAudioInstance& Instance = Pair.Value;

        // Update cooldown
        if (Instance.CooldownRemaining > 0.0f)
        {
            Instance.CooldownRemaining -= DeltaTime;
        }

        // Check if sound finished playing
        if (Instance.AudioComponent && !Instance.AudioComponent->IsPlaying() && !Instance.bIsLooping)
        {
            ToRemove.Add(Pair.Key);
        }
    }

    // Remove finished instances
    for (int32 EntityID : ToRemove)
    {
        RemoveEntityAudio(EntityID);
    }

    // Update clusters periodically
    UpdateAudioClusters();
}

void UGSDCrowdAudioSubsystem::UpdateAudioClusters()
{
    // Group distant entities into clusters for performance
    // Entities at LOD2 (Minimal) or beyond are clustered

    TMap<int32, TArray<FCrowdAudioInstance*>> GridClusters;
    const float ClusterGridSize = 1000.0f;

    for (auto& Pair : ActiveAudioInstances)
    {
        FCrowdAudioInstance& Instance = Pair.Value;

        if (Instance.CurrentLOD >= ECrowdAudioLOD::Minimal)
        {
            // Grid-based clustering
            const int32 GridX = FMath::FloorToInt(Instance.Location.X / ClusterGridSize);
            const int32 GridY = FMath::FloorToInt(Instance.Location.Y / ClusterGridSize);
            const int32 GridKey = (GridX << 16) | GridY;

            GridClusters.FindOrAdd(GridKey).Add(&Instance);
        }
    }

    // Create/update cluster audio
    AudioClusters.SetNum(GridClusters.Num());
    int32 ClusterIndex = 0;

    for (auto& Pair : GridClusters)
    {
        TArray<FCrowdAudioInstance*>& ClusterEntities = Pair.Value;
        if (ClusterEntities.Num() == 0)
        {
            continue;
        }

        // Calculate cluster center
        FVector Center = FVector::ZeroVector;
        for (const FCrowdAudioInstance* Instance : ClusterEntities)
        {
            Center += Instance->Location;
        }
        Center /= ClusterEntities.Num();

        // Update or create cluster
        if (ClusterIndex < AudioClusters.Num())
        {
            FCrowdAudioCluster& Cluster = AudioClusters[ClusterIndex];
            Cluster.ClusterCenter = Center;
            Cluster.EntityCount = ClusterEntities.Num();

            // Cluster radius encompasses all entities
            float MaxDist = 0.0f;
            for (const FCrowdAudioInstance* Instance : ClusterEntities)
            {
                MaxDist = FMath::Max(MaxDist, FVector::Dist(Center, Instance->Location));
            }
            Cluster.ClusterRadius = MaxDist;
        }

        ClusterIndex++;
    }
}

float UGSDCrowdAudioSubsystem::ApplyLODVolume(float BaseVolume, ECrowdAudioLOD LOD) const
{
    float Multiplier = 1.0f;

    if (CachedCrowdConfig)
    {
        switch (LOD)
        {
        case ECrowdAudioLOD::Reduced:
            Multiplier = CachedCrowdConfig->AudioLOD1VolumeMultiplier;
            break;
        case ECrowdAudioLOD::Minimal:
            Multiplier = CachedCrowdConfig->AudioLOD2VolumeMultiplier;
            break;
        case ECrowdAudioLOD::Culled:
            Multiplier = 0.0f;
            break;
        default:
            break;
        }
    }
    else
    {
        // Default multipliers
        switch (LOD)
        {
        case ECrowdAudioLOD::Reduced:
            Multiplier = 0.5f;
            break;
        case ECrowdAudioLOD::Minimal:
            Multiplier = 0.25f;
            break;
        case ECrowdAudioLOD::Culled:
            Multiplier = 0.0f;
            break;
        default:
            break;
        }
    }

    return BaseVolume * Multiplier;
}

void UGSDCrowdAudioSubsystem::CreateAudioCluster(const FVector& Center, float Radius, int32 EntityCount)
{
    FCrowdAudioCluster NewCluster;
    NewCluster.ClusterCenter = Center;
    NewCluster.ClusterRadius = Radius;
    NewCluster.EntityCount = EntityCount;

    // Create cluster audio component (plays ambient crowd sound)
    // This would typically play a looping ambient crowd sound at lower volume
    AudioClusters.Add(NewCluster);
}
