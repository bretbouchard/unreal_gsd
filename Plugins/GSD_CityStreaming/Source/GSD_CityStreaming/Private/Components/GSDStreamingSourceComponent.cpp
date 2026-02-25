// Copyright Bret Bouchard. All Rights Reserved.

#include "Components/GSDStreamingSourceComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "GSDLog.h"

UGSDStreamingSourceComponent::UGSDStreamingSourceComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(false);
}

void UGSDStreamingSourceComponent::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogGSD, Log, TEXT("GSDStreamingSourceComponent: Initialized on %s"), *GetOwner()->GetName());

    // Initial streaming state update
    UpdateStreamingSourceState();
}

void UGSDStreamingSourceComponent::SetStreamingEnabled(bool bEnabled)
{
    if (bStreamingEnabled != bEnabled)
    {
        bStreamingEnabled = bEnabled;
        UE_LOG(LogGSD, Log, TEXT("GSDStreamingSourceComponent: Streaming %s for %s"),
            bEnabled ? TEXT("enabled") : TEXT("disabled"),
            *GetOwner()->GetName());

        UpdateStreamingSourceState();
    }
}

void UGSDStreamingSourceComponent::SetLoadingRangeMultiplier(float Multiplier)
{
    LoadingRangeMultiplier = FMath::Clamp(Multiplier, 0.5f, 5.0f);
    UE_LOG(LogGSD, Log, TEXT("GSDStreamingSourceComponent: Loading range multiplier set to %.1f"), LoadingRangeMultiplier);

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
                GSD_LOG(VeryVerbose, TEXT("GSDStreamingSourceComponent: Predictive loading active, speed=%.0f"), Speed);
            }
        }
    }
}

// === Vehicle Configuration ===

void UGSDStreamingSourceComponent::ConfigureForVehicle(bool bIsFastVehicle, float VelocityThreshold)
{
    bPredictiveLoading = true;
    PredictiveLoadingVelocityThreshold = VelocityThreshold;

    if (bIsFastVehicle)
    {
        LoadingRangeMultiplier = FastVehicleRangeMultiplier;
    }
    else
    {
        LoadingRangeMultiplier = 1.0f;
    }

    UpdateStreamingSourceState();

    GSD_LOG(Log, TEXT("GSDStreamingSourceComponent: Configured for vehicle (Fast=%d, Threshold=%.0f, RangeMult=%.1f)"),
        bIsFastVehicle, VelocityThreshold, LoadingRangeMultiplier);
}

// CRITICAL: Event-driven state change - call from vehicle delegate, NOT from Tick
void UGSDStreamingSourceComponent::OnVehicleStateChanged(bool bIsDriving, float CurrentVelocity)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Cancel any pending hysteresis or hibernation timers
    World->GetTimerManager().ClearTimer(ParkingHysteresisTimer);
    World->GetTimerManager().ClearTimer(HibernationTimer);

    if (bIsDriving)
    {
        // Wake from hibernation if needed
        if (bIsHibernating)
        {
            bIsHibernating = false;
            SetStreamingEnabled(true);
            GSD_LOG(Log, TEXT("GSDStreamingSourceComponent: Woke from hibernation for %s"), *GetOwner()->GetName());
        }

        // Adjust range based on velocity
        if (CurrentVelocity > FastVehicleThreshold)
        {
            LoadingRangeMultiplier = FastVehicleRangeMultiplier;
        }
        else
        {
            LoadingRangeMultiplier = 1.0f;
        }

        SetStreamingEnabled(true);
        bPendingDisable = false;
    }
    else
    {
        // Parked - use hysteresis before disabling
        bPendingDisable = true;

        if (ParkingHysteresisDelay > 0.0f)
        {
            // CRITICAL: Delay before disabling to prevent rapid toggling
            World->GetTimerManager().SetTimer(
                ParkingHysteresisTimer,
                [this]()
                {
                    if (bPendingDisable)
                    {
                        SetStreamingEnabled(false);
                        bPendingDisable = false;

                        // Start hibernation timer for long-term parking
                        EnableHibernationMode(HibernationDelay);
                    }
                },
                ParkingHysteresisDelay,
                false
            );

            GSD_LOG(Verbose, TEXT("GSDStreamingSourceComponent: Parking hysteresis started (%.1fs) for %s"),
                ParkingHysteresisDelay, *GetOwner()->GetName());
        }
        else
        {
            // No hysteresis - disable immediately
            SetStreamingEnabled(false);
        }
    }

    UpdateStreamingSourceState();
}

void UGSDStreamingSourceComponent::EnableHibernationMode(float DelaySeconds)
{
    if (DelaySeconds <= 0.0f)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    World->GetTimerManager().SetTimer(
        HibernationTimer,
        [this]()
        {
            bIsHibernating = true;
            SetStreamingEnabled(false);
            GSD_LOG(Log, TEXT("GSDStreamingSourceComponent: Entered hibernation for %s"), *GetOwner()->GetName());
        },
        DelaySeconds,
        false
    );

    GSD_LOG(Verbose, TEXT("GSDStreamingSourceComponent: Hibernation scheduled (%.1fs) for %s"),
        DelaySeconds, *GetOwner()->GetName());
}

void UGSDStreamingSourceComponent::CancelHibernation()
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(HibernationTimer);
    }
    bIsHibernating = false;

    GSD_LOG(Verbose, TEXT("GSDStreamingSourceComponent: Hibernation cancelled for %s"), *GetOwner()->GetName());
}
