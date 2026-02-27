// Copyright Bret Bouchard. All Rights Reserved.

#include "Processors/GSDZombieBehaviorProcessor.h"
#include "Fragments/GSDZombieStateFragment.h"
#include "DataAssets/GSDCrowdConfig.h"
#include "MassCommonFragments.h"
#include "GSDCrowdLog.h"

UGSDZombieBehaviorProcessor::UGSDZombieBehaviorProcessor()
{
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::SyncWorld;
    ProcessingPhase = EMassProcessingPhase::PrePhysics;
}

void UGSDZombieBehaviorProcessor::ConfigureQueries()
{
    // CRITICAL: Specify correct access flags
    EntityQuery.AddRequirement<FGSDZombieStateFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FDataFragment_Transform>(EMassFragmentAccess::ReadOnly);
}

void UGSDZombieBehaviorProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Load config (cached for frame)
    if (!CachedConfig)
    {
        CachedConfig = UGSDCrowdConfig::GetDefaultConfig();
    }

    // Get config values with fallbacks for backward compatibility
    const float BehaviorUpdateInterval = CachedConfig
        ? CachedConfig->BehaviorUpdateInterval
        : DefaultBehaviorUpdateInterval;

    const float SpeedVariation = CachedConfig
        ? CachedConfig->SpeedVariationPercent
        : DefaultSpeedVariation;

    const float WanderDirectionChange = CachedConfig
        ? CachedConfig->WanderDirectionChange
        : DefaultWanderDirectionChange;

    const float SpeedInterpolationRate = CachedConfig
        ? CachedConfig->SpeedInterpolationRate
        : DefaultSpeedInterpolationRate;

    EntityQuery.ForEachEntityChunk(EntityManager, Context,
        [this, BehaviorUpdateInterval, SpeedVariation, WanderDirectionChange, SpeedInterpolationRate](FMassExecutionContext& Context)
        {
            const int32 NumEntities = Context.GetNumEntities();
            auto ZombieStates = Context.GetMutableFragmentView<FGSDZombieStateFragment>();
            const float DeltaTime = Context.GetDeltaTimeSeconds();

            for (int32 i = 0; i < NumEntities; ++i)
            {
                FGSDZombieStateFragment& State = ZombieStates[i];

                if (!State.bIsAlive || !State.bIsActive)
                {
                    continue;
                }

                // Update behavior timer
                State.TimeSinceLastBehaviorUpdate += DeltaTime;

                if (State.TimeSinceLastBehaviorUpdate >= BehaviorUpdateInterval)
                {
                    State.TimeSinceLastBehaviorUpdate = 0.0f;

                    // Apply speed variation (prevents synchronized movement)
                    // Uses Config->SpeedVariationPercent instead of hardcoded 0.2f
                    const float SpeedMultiplier = 1.0f + FMath::FRandRange(-SpeedVariation, SpeedVariation);
                    State.TargetMovementSpeed = State.MovementSpeed * SpeedMultiplier;

                    // Update wander direction
                    // Uses Config->WanderDirectionChange instead of hardcoded 45.0f
                    State.WanderDirection += FMath::FRandRange(-WanderDirectionChange, WanderDirectionChange);
                    State.WanderDirection = FMath::Clamp(State.WanderDirection, -180.0f, 180.0f);
                }

                // Smooth speed interpolation
                // Uses Config->SpeedInterpolationRate instead of hardcoded 2.0f
                State.MovementSpeed = FMath::Lerp(State.MovementSpeed, State.TargetMovementSpeed, DeltaTime * SpeedInterpolationRate);
            }
        });
}
