// Copyright Bret Bouchard. All Rights Reserved.

#include "Components/GSDStreamingSourceComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Actor.h"
#include "GSDLog.h"

UGSDStreamingSourceComponent::UGSDStreamingSourceComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(false);
}

void UGSDStreamingSourceComponent::BeginPlay()
{
    Super::BeginPlay();

    GSD_LOG(TEXT("GSDStreamingSourceComponent: Initialized on %s"), *GetOwner()->GetName());

    // Initial streaming state update
    UpdateStreamingSourceState();
}

void UGSDStreamingSourceComponent::SetStreamingEnabled(bool bEnabled)
{
    if (bStreamingEnabled != bEnabled)
    {
        bStreamingEnabled = bEnabled;
        GSD_LOG(TEXT("GSDStreamingSourceComponent: Streaming %s for %s"),
            bEnabled ? TEXT("enabled") : TEXT("disabled"),
            *GetOwner()->GetName());

        UpdateStreamingSourceState();
    }
}

void UGSDStreamingSourceComponent::SetLoadingRangeMultiplier(float Multiplier)
{
    LoadingRangeMultiplier = FMath::Clamp(Multiplier, 0.5f, 5.0f);
    GSD_LOG(TEXT("GSDStreamingSourceComponent: Loading range multiplier set to %.1f"), LoadingRangeMultiplier);

    UpdateStreamingSourceState();
}

void UGSDStreamingSourceComponent::UpdateStreamingSourceState()
{
    // The base class handles actual streaming source registration
    // We just need to update our custom properties

    if (!bStreamingEnabled)
    {
        // Disable streaming by setting this source as inactive
        // The base class will handle unregistering
        SetComponentTickEnabled(false);
        return;
    }

    SetComponentTickEnabled(true);

    // Get owner velocity for predictive loading calculations
    if (AActor* Owner = GetOwner())
    {
        CachedVelocity = Owner->GetVelocity();

        // Check if predictive loading should activate based on velocity
        if (bPredictiveLoading)
        {
            float Speed = CachedVelocity.Size();
            if (Speed > PredictiveLoadingVelocityThreshold)
            {
                // Predictive loading is active - World Partition will
                // load cells in movement direction based on our position
                GSD_VERY_TRACE(TEXT("GSDStreamingSourceComponent: Predictive loading active, speed=%.0f"), Speed);
            }
        }
    }
}
