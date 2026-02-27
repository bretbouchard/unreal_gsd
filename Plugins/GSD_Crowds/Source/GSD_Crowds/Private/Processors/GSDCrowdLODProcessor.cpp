// Copyright Bret Bouchard. All Rights Reserved.

#include "Processors/GSDCrowdLODProcessor.h"
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
    // Map distance to LOD significance (0.0 = close, 3.0 = far)
    if (Distance < HighActorDistance) return 0.0f;      // High Actor
    if (Distance < LowActorDistance) return 0.75f;      // Low Actor
    if (Distance < ISMDistance) return 1.75f;           // ISM
    if (Distance < CullDistance) return 2.5f;           // Culled
    return 3.0f;                                        // Far culled
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
    if (!bEnableAudioLOD)
    {
        return 1.0f;  // No LOD, full volume
    }

    if (DistanceToListener < AudioLOD0Distance)
    {
        // LOD 0: Full audio
        return 1.0f;
    }
    else if (DistanceToListener < AudioLOD1Distance)
    {
        // LOD 1: Reduced audio (0.5x volume)
        return 0.5f;
    }
    else if (DistanceToListener < AudioLOD2Distance)
    {
        // LOD 2: Minimal audio (0.25x volume)
        return 0.25f;
    }

    // Beyond LOD 2: Culled
    return 0.0f;
}

bool UGSDCrowdLODProcessor::ShouldCullAudio(float DistanceToListener) const
{
    if (!bEnableAudioLOD)
    {
        return false;  // Never cull if LOD disabled
    }

    return DistanceToListener >= AudioCullDistance;
}
