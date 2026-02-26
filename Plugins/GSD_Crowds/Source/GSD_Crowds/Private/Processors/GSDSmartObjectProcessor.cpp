// Copyright Bret Bouchard. All Rights Reserved.

#include "Processors/GSDSmartObjectProcessor.h"
#include "Fragments/GSDSmartObjectFragment.h"
#include "Fragments/GSDNavigationFragment.h"
#include "MassEntity/DataFragmentTypes.h"
#include "Subsystems/GSDSmartObjectSubsystem.h"

UGSDSmartObjectProcessor::UGSDSmartObjectProcessor()
{
    // Run after navigation, before behavior
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::SyncWorld;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::SyncWorld);
    ProcessingPhase = EMassProcessingPhase::PrePhysics;
}

void UGSDSmartObjectProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FGSDSmartObjectFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FGSDNavigationFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FDataFragment_Transform>(EMassFragmentAccess::ReadOnly);
}

void UGSDSmartObjectProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    UGSDSmartObjectSubsystem* SOSubsystem = World->GetSubsystem<UGSDSmartObjectSubsystem>();
    if (!SOSubsystem)
    {
        return;
    }

    const float DeltaTime = Context.GetDeltaTimeSeconds();

    EntityQuery.ForEachEntityChunk(EntityManager, Context,
        [this, SOSubsystem, DeltaTime](FMassExecutionContext& Context)
        {
            auto SOFragments = Context.GetMutableFragmentView<FGSDSmartObjectFragment>();
            auto NavFragments = Context.GetMutableFragmentView<FGSDNavigationFragment>();
            const auto Transforms = Context.GetFragmentView<FDataFragment_Transform>();

            for (int32 i = 0; i < Context.GetNumEntities(); ++i)
            {
                FGSDSmartObjectFragment& SO = SOFragments[i];
                FGSDNavigationFragment& Nav = NavFragments[i];
                const FDataFragment_Transform& Transform = Transforms[i];

                // State machine for Smart Object interaction
                if (SO.bIsInteracting)
                {
                    // Currently interacting - process progress
                    ProcessInteraction(SO, Nav, DeltaTime);

                    // KEY LINK: Release Smart Object when interaction completes
                    if (SO.bInteractionComplete)
                    {
                        ReleaseSmartObject(SO, SOSubsystem);
                    }
                }
                else if (SO.HasValidClaim())
                {
                    // Has claim but not interacting - start interaction
                    SO.bIsInteracting = true;
                    SO.InteractionTime = 0.0f;
                    SO.InteractionDuration = DefaultInteractionDuration;

                    // Pause navigation during interaction
                    Nav.DesiredSpeed = 0.0f;
                }
                else
                {
                    // No claim - search for Smart Object
                    SearchForSmartObject(SO, Transform, SOSubsystem, DeltaTime);

                    // If found one, try to claim
                    if (!SO.HasValidClaim())
                    {
                        TryClaimSmartObject(SO, Transform, SOSubsystem);
                    }
                }
            }
        });
}

void UGSDSmartObjectProcessor::SearchForSmartObject(
    FGSDSmartObjectFragment& SOFragment,
    const FDataFragment_Transform& Transform,
    UGSDSmartObjectSubsystem* SOSubsystem,
    float DeltaTime)
{
    // Update search cooldown
    SOFragment.TimeSinceLastSearch += DeltaTime;

    if (SOFragment.TimeSinceLastSearch < SOFragment.SearchCooldown)
    {
        return;  // Still on cooldown
    }

    SOFragment.TimeSinceLastSearch = 0.0f;

    const FVector Location = Transform.GetTransform().GetLocation();
    const float SearchRadius = SOFragment.SearchRadius > 0.0f ?
        SOFragment.SearchRadius : DefaultSearchRadius;

    // Find nearest available Smart Object
    FSmartObjectHandle NearestSO = SOSubsystem->FindNearestAvailableSmartObject(
        Location,
        SearchRadius
    );

    if (NearestSO.IsValid())
    {
        // Store for claiming next frame
        SOFragment.ClaimedHandle = FSmartObjectClaimHandle(NearestSO, {});
        SOFragment.bHasClaimedObject = false;  // Will be set true after actual claim
    }
}

void UGSDSmartObjectProcessor::TryClaimSmartObject(
    FGSDSmartObjectFragment& SOFragment,
    const FDataFragment_Transform& Transform,
    UGSDSmartObjectSubsystem* SOSubsystem)
{
    if (!SOFragment.ClaimedHandle.SmartObjectHandle.IsValid())
    {
        return;  // No candidate to claim
    }

    // Actually claim the Smart Object
    FSmartObjectClaimHandle NewHandle = SOSubsystem->ClaimSmartObject(
        SOFragment.ClaimedHandle.SmartObjectHandle
    );

    if (NewHandle.IsValid())
    {
        SOFragment.ClaimedHandle = NewHandle;
        SOFragment.bHasClaimedObject = true;
    }
    else
    {
        // Claim failed - clear candidate
        SOFragment.ClaimedHandle = FSmartObjectClaimHandle();
    }
}

void UGSDSmartObjectProcessor::ProcessInteraction(
    FGSDSmartObjectFragment& SOFragment,
    FGSDNavigationFragment& NavFragment,
    float DeltaTime)
{
    SOFragment.InteractionTime += DeltaTime;

    if (SOFragment.InteractionTime >= SOFragment.InteractionDuration)
    {
        // Interaction complete
        SOFragment.bInteractionComplete = true;
        SOFragment.bIsInteracting = false;
    }
}

void UGSDSmartObjectProcessor::ReleaseSmartObject(
    FGSDSmartObjectFragment& SOFragment,
    UGSDSmartObjectSubsystem* SOSubsystem)
{
    if (SOFragment.HasValidClaim())
    {
        SOSubsystem->ReleaseSmartObject(SOFragment.ClaimedHandle);
        SOFragment.bHasClaimedObject = false;
        SOFragment.bIsInteracting = false;
        SOFragment.bInteractionComplete = false;
    }
}
