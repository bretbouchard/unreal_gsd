// Copyright Bret Bouchard. All Rights Reserved.

#include "Processors/GSDNavigationProcessor.h"
#include "Fragments/GSDNavigationFragment.h"
#include "Fragments/GSDZombieStateFragment.h"
#include "MassEntity/DataFragmentTypes.h"
#include "MassRepresentation/RepresentationFragment.h"
#include "ZoneGraph/ZoneGraphSubsystem.h"
#include "ZoneGraph/ZoneGraphTypes.h"
#include "Managers/GSDDeterminismManager.h"
#include "Engine/GameInstance.h"

UGSDNavigationProcessor::UGSDNavigationProcessor()
{
    // Execute EARLY - Navigation must run before behavior processing
    // This ensures entities have valid navigation data before behavior updates
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::SyncWorld);
    ProcessingPhase = EMassProcessingPhase::PrePhysics;
}

void UGSDNavigationProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FGSDNavigationFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FDataFragment_Transform>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FGSDZombieStateFragment>(EMassFragmentAccess::ReadOnly);
}

void UGSDNavigationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    const UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    const UZoneGraphSubsystem* ZoneGraphSubsystem = World->GetSubsystem<UZoneGraphSubsystem>();
    const bool bZoneGraphAvailable = ZoneGraphSubsystem && ZoneGraphSubsystem->GetNumLanes() > 0;
    const float DeltaTime = Context.GetDeltaTimeSeconds();

    // Get DeterminismManager for seeded random
    UGSDDeterminismManager* DeterminismManager = nullptr;
    if (UGameInstance* GameInstance = World->GetGameInstance())
    {
        DeterminismManager = GameInstance->GetSubsystem<UGSDDeterminismManager>();
    }

    EntityQuery.ForEachEntityChunk(EntityManager, Context,
        [this, ZoneGraphSubsystem, bZoneGraphAvailable, DeltaTime, DeterminismManager](FMassExecutionContext& Context)
        {
            auto NavFragments = Context.GetMutableFragmentView<FGSDNavigationFragment>();
            auto Transforms = Context.GetMutableFragmentView<FDataFragment_Transform>();
            const auto ZombieStates = Context.GetFragmentView<FGSDZombieStateFragment>();

            for (int32 i = 0; i < Context.GetNumEntities(); ++i)
            {
                FGSDNavigationFragment& Nav = NavFragments[i];
                FDataFragment_Transform& Transform = Transforms[i];
                const FGSDZombieStateFragment& Zombie = ZombieStates[i];

                // Check if ZoneGraph is available
                if (!bZoneGraphAvailable)
                {
                    Nav.bUseFallbackMovement = true;
                    ExecuteFallbackMovement(Nav, Transform, Zombie, DeltaTime, DeterminismManager);
                    continue;
                }

                // Not on a lane yet - find one
                if (!Nav.bIsOnLane || !Nav.CurrentLane.IsValid())
                {
                    FindNearestLane(Nav, Transform, ZoneGraphSubsystem, DeterminismManager);
                    if (!Nav.bIsOnLane)
                    {
                        // Still no lane, use fallback
                        Nav.bUseFallbackMovement = true;
                        ExecuteFallbackMovement(Nav, Transform, Zombie, DeltaTime, DeterminismManager);
                        continue;
                    }
                }

                // Move along lane with randomized velocity (CROWD-08)
                Nav.bUseFallbackMovement = false;
                const float RandomizedSpeed = ApplyVelocityRandomization(Zombie.MovementSpeed, VelocityRandomizationPercent, DeterminismManager);
                Nav.LanePosition += RandomizedSpeed * DeltaTime;

                // Update transform from lane position
                UpdateTransformFromLane(Nav, Transform, ZoneGraphSubsystem);

                // Check if reached end of lane
                CheckLaneProgress(Nav, ZoneGraphSubsystem, DeterminismManager);
            }
        });
}

float UGSDNavigationProcessor::ApplyVelocityRandomization(float BaseSpeed, float RandomizationPercent, UGSDDeterminismManager* DeterminismManager) const
{
    // CROWD-08: Apply velocity randomization to prevent synchronized movement
    // Randomization is +/- RandomizationPercent of base speed
    // Use seeded random from DeterminismManager for determinism
    float RandomFactor = 1.0f;
    if (DeterminismManager)
    {
        FRandomStream& NavStream = DeterminismManager->GetCategoryStream(UGSDDeterminismManager::NavigationCategory);
        RandomFactor = 1.0f + NavStream.FRandRange(-RandomizationPercent, RandomizationPercent) / 100.0f;
        DeterminismManager->RecordRandomCall(UGSDDeterminismManager::NavigationCategory, RandomFactor);
    }
    else
    {
        // Fallback to seeded random for determinism even without manager
        static FRandomStream FallbackNavStream(98765);
        RandomFactor = 1.0f + FallbackNavStream.FRandRange(-RandomizationPercent, RandomizationPercent) / 100.0f;
    }
    return BaseSpeed * RandomFactor;
}

void UGSDNavigationProcessor::FindNearestLane(
    FGSDNavigationFragment& Nav,
    const FDataFragment_Transform& Transform,
    const UZoneGraphSubsystem* ZoneGraphSubsystem,
    UGSDDeterminismManager* DeterminismManager) const
{
    if (!ZoneGraphSubsystem)
    {
        return;
    }

    const FVector Location = Transform.GetTransform().GetLocation();

    // Query lanes in bounds
    TArray<FZoneGraphLaneHandle> NearbyLanes;
    ZoneGraphSubsystem->FindLanesInBounds(
        FBoxCenterAndExtent(Location, FVector(LaneSearchRadius)),
        NearbyLanes
    );

    if (NearbyLanes.IsEmpty())
    {
        Nav.bIsOnLane = false;
        return;
    }

    // Pick lane using seeded random for determinism
    int32 LaneIndex = 0;
    if (DeterminismManager)
    {
        FRandomStream& NavStream = DeterminismManager->GetCategoryStream(UGSDDeterminismManager::NavigationCategory);
        LaneIndex = NavStream.RandHelper(NearbyLanes.Num());
        DeterminismManager->RecordRandomCall(UGSDDeterminismManager::NavigationCategory, static_cast<float>(LaneIndex));
    }
    else
    {
        static FRandomStream FallbackLaneStream(11111);
        LaneIndex = FallbackLaneStream.RandHelper(NearbyLanes.Num());
    }

    Nav.CurrentLane = NearbyLanes[LaneIndex];
    Nav.LanePosition = 0.0f;
    Nav.bIsOnLane = true;
    Nav.bReachedDestination = false;
}

void UGSDNavigationProcessor::UpdateTransformFromLane(
    FGSDNavigationFragment& Nav,
    FDataFragment_Transform& Transform,
    const UZoneGraphSubsystem* ZoneGraphSubsystem) const
{
    if (!Nav.CurrentLane.IsValid() || !ZoneGraphSubsystem)
    {
        return;
    }

    FZoneGraphLaneLocation LaneLocation;
    if (ZoneGraphSubsystem->GetLaneLocation(Nav.CurrentLane, Nav.LanePosition, LaneLocation))
    {
        FTransform NewTransform = Transform.GetTransform();
        NewTransform.SetLocation(LaneLocation.Position);
        NewTransform.SetRotation(LaneLocation.Direction.ToOrientationQuat());
        Transform.SetTransform(NewTransform);
    }
}

void UGSDNavigationProcessor::CheckLaneProgress(
    FGSDNavigationFragment& Nav,
    const UZoneGraphSubsystem* ZoneGraphSubsystem,
    UGSDDeterminismManager* DeterminismManager) const
{
    if (!Nav.CurrentLane.IsValid() || !ZoneGraphSubsystem)
    {
        return;
    }

    // Get lane length
    const FZoneGraphLaneHandle& Lane = Nav.CurrentLane;
    const float LaneLength = ZoneGraphSubsystem->GetLaneLength(Lane);

    if (Nav.LanePosition >= LaneLength)
    {
        Nav.bReachedDestination = true;

        // Try to find a connected lane or pick a new random one
        Nav.CurrentLane = PickRandomNearbyLane(
            FVector::ZeroVector, // Will use entity's current location
            ZoneGraphSubsystem,
            DeterminismManager
        );

        if (Nav.CurrentLane.IsValid())
        {
            Nav.LanePosition = 0.0f;
            Nav.bReachedDestination = false;
        }
        else
        {
            Nav.bIsOnLane = false;
        }
    }
}

void UGSDNavigationProcessor::ExecuteFallbackMovement(
    FGSDNavigationFragment& Nav,
    FDataFragment_Transform& Transform,
    const FGSDZombieStateFragment& Zombie,
    float DeltaTime,
    UGSDDeterminismManager* DeterminismManager) const
{
    // Simple direct movement when ZoneGraph unavailable
    FTransform CurrentTransform = Transform.GetTransform();
    FVector Location = CurrentTransform.GetLocation();

    // Move in a random direction if no target - use seeded random
    if (Nav.FallbackTargetLocation.IsNearlyZero())
    {
        float RandomAngle = 0.0f;
        if (DeterminismManager)
        {
            FRandomStream& NavStream = DeterminismManager->GetCategoryStream(UGSDDeterminismManager::NavigationCategory);
            RandomAngle = NavStream.FRand() * 2.0f * PI;
            DeterminismManager->RecordRandomCall(UGSDDeterminismManager::NavigationCategory, RandomAngle);
        }
        else
        {
            // Fallback to seeded random for determinism
            static FRandomStream FallbackAngleStream(22222);
            RandomAngle = FallbackAngleStream.FRand() * 2.0f * PI;
        }

        Nav.FallbackTargetLocation = Location + FVector(
            FMath::Cos(RandomAngle) * 500.0f,
            FMath::Sin(RandomAngle) * 500.0f,
            0.0f
        );
    }

    // Move toward target with randomized velocity (CROWD-08)
    const FVector Direction = (Nav.FallbackTargetLocation - Location).GetSafeNormal();
    const float RandomizedSpeed = ApplyVelocityRandomization(Zombie.MovementSpeed, VelocityRandomizationPercent, DeterminismManager);
    const float EffectiveSpeed = RandomizedSpeed * FallbackMoveSpeed / 100.0f;
    Location += Direction * EffectiveSpeed * DeltaTime;

    // Update transform
    CurrentTransform.SetLocation(Location);
    if (!Direction.IsNearlyZero())
    {
        CurrentTransform.SetRotation(Direction.ToOrientationQuat());
    }
    Transform.SetTransform(CurrentTransform);

    // Check if reached target
    if (FVector::DistSquared(Location, Nav.FallbackTargetLocation) < 100.0f)
    {
        Nav.FallbackTargetLocation = FVector::ZeroVector;
    }
}

FZoneGraphLaneHandle UGSDNavigationProcessor::PickRandomNearbyLane(
    const FVector& Location,
    const UZoneGraphSubsystem* ZoneGraphSubsystem,
    UGSDDeterminismManager* DeterminismManager) const
{
    if (!ZoneGraphSubsystem)
    {
        return FZoneGraphLaneHandle();
    }

    TArray<FZoneGraphLaneHandle> NearbyLanes;
    ZoneGraphSubsystem->FindLanesInBounds(
        FBoxCenterAndExtent(Location, FVector(LaneSearchRadius)),
        NearbyLanes
    );

    if (NearbyLanes.IsEmpty())
    {
        return FZoneGraphLaneHandle();
    }

    // Use seeded random for determinism
    int32 LaneIndex = 0;
    if (DeterminismManager)
    {
        FRandomStream& NavStream = DeterminismManager->GetCategoryStream(UGSDDeterminismManager::NavigationCategory);
        LaneIndex = NavStream.RandHelper(NearbyLanes.Num());
        DeterminismManager->RecordRandomCall(UGSDDeterminismManager::NavigationCategory, static_cast<float>(LaneIndex));
    }
    else
    {
        static FRandomStream FallbackPickStream(33333);
        LaneIndex = FallbackPickStream.RandHelper(NearbyLanes.Num());
    }

    return NearbyLanes[LaneIndex];
}
