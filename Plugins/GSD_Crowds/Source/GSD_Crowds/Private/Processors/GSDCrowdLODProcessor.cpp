// Copyright Bret Bouchard. All Rights Reserved.

#include "Processors/GSDCrowdLODProcessor.h"
#include "DataAssets/GSDCrowdConfig.h"
#include "MassRepresentationFragments.h"
#include "MassCommonFragments.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "GSDCrowdLog.h"

UGSDCrowdLODProcessor::UGSDCrowdLODProcessor()
{
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::SyncWorld);
    ProcessingPhase = EMassProcessingPhase::PrePhysics;
}

void UGSDCrowdLODProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FMassRepresentationLODFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FDataFragment_Transform>(EMassFragmentAccess::ReadOnly);
}

void UGSDCrowdLODProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Load config (cached for frame)
    if (!CachedConfig)
    {
        CachedConfig = UGSDCrowdConfig::GetDefaultConfig();
    }

    const FVector ViewerLocation = GetViewerLocation(Context);

    EntityQuery.ForEachEntityChunk(EntityManager, Context,
        [&](FMassExecutionContext& Context)
        {
            auto LODFragments = Context.GetMutableFragmentView<FMassRepresentationLODFragment>();
            const auto& Transforms = Context.GetFragmentView<FDataFragment_Transform>();

            for (int32 i = 0; i < Context.GetNumEntities(); ++i)
            {
                const FVector EntityLocation = Transforms[i].GetTransform().GetLocation();
                const float Distance = FVector::Dist(EntityLocation, ViewerLocation);
                LODFragments[i].LODSignificance = CalculateLODSignificance(Distance);
            }
        });
}

float UGSDCrowdLODProcessor::CalculateLODSignificance(float Distance) const
{
    // Get config values with fallbacks
    const float HighActorDist = CachedConfig ? CachedConfig->HighActorDistance : DefaultHighActorDistance;
    const float LowActorDist = CachedConfig ? CachedConfig->LowActorDistance : DefaultLowActorDistance;
    const float ISMDist = CachedConfig ? CachedConfig->ISMDistance : DefaultISMDistance;
    const float CullDist = CachedConfig ? CachedConfig->CullDistance : DefaultCullDistance;

    // Map distance to LOD significance (0.0 = close, 3.0 = far)
    // Uses Config->HighActorDistance, etc. instead of hardcoded values
    if (Distance < HighActorDist) return 0.0f;      // High Actor
    if (Distance < LowActorDist) return 0.75f;      // Low Actor
    if (Distance < ISMDist) return 1.75f;           // ISM
    if (Distance < CullDist) return 2.5f;           // Culled
    return 3.0f;                                    // Far culled
}

FVector UGSDCrowdLODProcessor::GetViewerLocation(FMassExecutionContext& Context) const
{
    if (UWorld* World = Context.GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APlayerCameraManager* CameraManager = PC->PlayerCameraManager)
            {
                return CameraManager->GetCameraLocation();
            }
        }
    }
    return FVector::ZeroVector;
}

float UGSDCrowdLODProcessor::CalculateAudioLODVolume(float DistanceToListener) const
{
    // Check if audio LOD is enabled
    const bool bEnableAudioLOD = CachedConfig ? CachedConfig->bEnableAudioLOD : true;

    if (!bEnableAudioLOD)
    {
        return 1.0f;  // No LOD, full volume
    }

    // Get config values with fallbacks
    const float AudioLOD0Dist = CachedConfig ? CachedConfig->AudioLOD0Distance : DefaultAudioLOD0Distance;
    const float AudioLOD1Dist = CachedConfig ? CachedConfig->AudioLOD1Distance : DefaultAudioLOD1Distance;
    const float AudioLOD2Dist = CachedConfig ? CachedConfig->AudioLOD2Distance : DefaultAudioLOD2Distance;
    const float AudioLOD1Vol = CachedConfig ? CachedConfig->AudioLOD1VolumeMultiplier : DefaultAudioLOD1Volume;
    const float AudioLOD2Vol = CachedConfig ? CachedConfig->AudioLOD2VolumeMultiplier : DefaultAudioLOD2Volume;

    // Uses Config->AudioLOD*Distance instead of hardcoded values
    if (DistanceToListener < AudioLOD0Dist)
    {
        // LOD 0: Full audio
        return 1.0f;
    }
    else if (DistanceToListener < AudioLOD1Dist)
    {
        // LOD 1: Reduced audio (Config->AudioLOD1VolumeMultiplier, default 0.5x volume)
        return AudioLOD1Vol;
    }
    else if (DistanceToListener < AudioLOD2Dist)
    {
        // LOD 2: Minimal audio (Config->AudioLOD2VolumeMultiplier, default 0.25x volume)
        return AudioLOD2Vol;
    }

    // Beyond LOD 2: Culled
    return 0.0f;
}

bool UGSDCrowdLODProcessor::ShouldCullAudio(float DistanceToListener) const
{
    // Check if audio LOD is enabled
    const bool bEnableAudioLOD = CachedConfig ? CachedConfig->bEnableAudioLOD : true;

    if (!bEnableAudioLOD)
    {
        return false;  // Never cull if LOD disabled
    }

    // Get config value with fallback
    const float AudioCullDist = CachedConfig ? CachedConfig->AudioCullDistance : DefaultAudioCullDistance;

    // Uses Config->AudioCullDistance instead of hardcoded value
    return DistanceToListener >= AudioCullDist;
}

//-- Accessor implementations --

float UGSDCrowdLODProcessor::GetHighActorDistance() const
{
    return CachedConfig ? CachedConfig->HighActorDistance : DefaultHighActorDistance;
}

float UGSDCrowdLODProcessor::GetLowActorDistance() const
{
    return CachedConfig ? CachedConfig->LowActorDistance : DefaultLowActorDistance;
}

float UGSDCrowdLODProcessor::GetISMDistance() const
{
    return CachedConfig ? CachedConfig->ISMDistance : DefaultISMDistance;
}

float UGSDCrowdLODProcessor::GetCullDistance() const
{
    return CachedConfig ? CachedConfig->CullDistance : DefaultCullDistance;
}

bool UGSDCrowdLODProcessor::IsAudioLODEnabled() const
{
    return CachedConfig ? CachedConfig->bEnableAudioLOD : true;
}
