// Copyright Bret Bouchard. All Rights Reserved.

#include "Processors/GSDZombieBehaviorProcessor.h"
#include "Processors/GSDNavigationProcessor.h"
#include "Fragments/GSDZombieStateFragment.h"
#include "DataAssets/GSDCrowdConfig.h"
#include "MassCommonFragments.h"
#include "GSDCrowdLog.h"
#include "Managers/GSDDeterminismManager.h"
#include "Kismet/GameplayStatics.h"

// NOTE: This processor will be renamed to UGSDMassBehaviorProcessor in a future phase (GSDCROWDS-105)
// The behavior logic is game-agnostic and suitable for any crowd/flock simulation.

UGSDZombieBehaviorProcessor::UGSDZombieBehaviorProcessor()
{
    // Execute AFTER navigation, BEFORE LOD
    // Behavior depends on current navigation data
    // LOD should see final behavior-updated state
    // TODO(GSDCROWDS-105): Update to reference UGSDMassBehaviorProcessor when renamed
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
    ExecutionOrder.ExecuteAfter.Add(UGSDNavigationProcessor::StaticClass());
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

    //-- Pursuit/Attack Config Values --
    const bool bEnablePursuitBehavior = CachedConfig ? CachedConfig->bEnablePursuitBehavior : true;
    const float DetectionRange = CachedConfig ? CachedConfig->DetectionRange : DefaultDetectionRange;
    const float PursuitSpeedMultiplier = CachedConfig ? CachedConfig->PursuitSpeedMultiplier : DefaultPursuitSpeedMultiplier;
    const float AttackRange = CachedConfig ? CachedConfig->AttackRange : DefaultAttackRange;
    const float AttackCooldown = CachedConfig ? CachedConfig->AttackCooldown : DefaultAttackCooldown;
    const float LoseTargetDistance = CachedConfig ? CachedConfig->LoseTargetDistance : DefaultLoseTargetDistance;
    const float BaseMoveSpeed = CachedConfig ? CachedConfig->BaseMoveSpeed : 150.0f;

    // Get DeterminismManager for seeded random (if available)
    UGSDDeterminismManager* DeterminismManager = nullptr;
    if (UWorld* World = Context.GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            DeterminismManager = GameInstance->GetSubsystem<UGSDDeterminismManager>();
        }
    }

    EntityQuery.ForEachEntityChunk(EntityManager, Context,
        [this, BehaviorUpdateInterval, SpeedVariation, WanderDirectionChange, SpeedInterpolationRate,
         bEnablePursuitBehavior, DetectionRange, PursuitSpeedMultiplier, AttackRange, AttackCooldown,
         LoseTargetDistance, BaseMoveSpeed, DeterminismManager](FMassExecutionContext& Context)
        {
            const int32 NumEntities = Context.GetNumEntities();
            auto EntityStates = Context.GetMutableFragmentView<FGSDZombieStateFragment>();
            auto Transforms = Context.GetFragmentView<FDataFragment_Transform>();
            const float DeltaTime = Context.GetDeltaTimeSeconds();

            for (int32 i = 0; i < NumEntities; ++i)
            {
                FGSDZombieStateFragment& State = EntityStates[i];
                const FDataFragment_Transform& Transform = Transforms[i];

                if (!State.bIsAlive || !State.bIsActive)
                {
                    continue;
                }

                // Update behavior timer
                State.TimeSinceLastBehaviorUpdate += DeltaTime;
                State.TimeSinceLastAttack += DeltaTime;

                //-- Pursuit/Attack Behavior --
                if (bEnablePursuitBehavior && State.bIsAggressive)
                {
                    const FVector CurrentLocation = Transform.GetTransform().GetLocation();

                    // Check if we have a valid target
                    if (State.TargetEntityID != INDEX_NONE)
                    {
                        const float DistToTargetSq = FVector::DistSquared(CurrentLocation, State.TargetLocation);

                        // Check if target is lost (too far away)
                        if (DistToTargetSq > LoseTargetDistance * LoseTargetDistance)
                        {
                            // Lost target - return to wandering
                            State.TargetEntityID = INDEX_NONE;
                            State.TargetLocation = FVector::ZeroVector;
                            State.TargetMovementSpeed = BaseMoveSpeed;
                        }
                        // Check if in attack range
                        else if (DistToTargetSq <= AttackRange * AttackRange)
                        {
                            // In attack range - attack if cooldown ready
                            if (State.TimeSinceLastAttack >= AttackCooldown)
                            {
                                State.TimeSinceLastAttack = 0.0f;
                                // TODO: Trigger attack event/animation (GSDCROWDS-201)
                                // For now, just log
                                UE_LOG(LOG_GSDCROWDS, Verbose, TEXT("Entity attacking target at range %.1f"), FMath::Sqrt(DistToTargetSq));
                            }
                            // Stop moving when in attack range
                            State.TargetMovementSpeed = 0.0f;
                        }
                        // Pursue target
                        else
                        {
                            // Move toward target at pursuit speed
                            State.TargetMovementSpeed = BaseMoveSpeed * PursuitSpeedMultiplier;
                        }
                    }
                    // No target - try to find one
                    else
                    {
                        // TODO: Target acquisition via spatial query (GSDCROWDS-202)
                        // For now, this is a placeholder - targets are set via events
                    }
                }

                //-- Wandering Behavior (when not pursuing) --
                if (State.TargetEntityID == INDEX_NONE)
                {
                    if (State.TimeSinceLastBehaviorUpdate >= BehaviorUpdateInterval)
                    {
                        State.TimeSinceLastBehaviorUpdate = 0.0f;

                        // Apply speed variation (prevents synchronized movement)
                        // Use seeded random from DeterminismManager for determinism
                        float SpeedMultiplier = 1.0f;
                        if (DeterminismManager)
                        {
                            FRandomStream& SpeedStream = DeterminismManager->GetCategoryStream(UGSDDeterminismManager::ZombieSpeedCategory);
                            SpeedMultiplier = 1.0f + SpeedStream.FRandRange(-SpeedVariation, SpeedVariation);
                            DeterminismManager->RecordRandomCall(UGSDDeterminismManager::ZombieSpeedCategory, SpeedMultiplier);
                        }
                        else
                        {
                            // Fallback to seeded random for determinism even without manager
                            static FRandomStream FallbackSpeedStream(12345);
                            SpeedMultiplier = 1.0f + FallbackSpeedStream.FRandRange(-SpeedVariation, SpeedVariation);
                        }
                        State.TargetMovementSpeed = State.MovementSpeed * SpeedMultiplier;

                        // Update wander direction
                        // Use seeded random from DeterminismManager for determinism
                        float DirectionChange = 0.0f;
                        if (DeterminismManager)
                        {
                            FRandomStream& WanderStream = DeterminismManager->GetCategoryStream(UGSDDeterminismManager::ZombieWanderCategory);
                            DirectionChange = WanderStream.FRandRange(-WanderDirectionChange, WanderDirectionChange);
                            DeterminismManager->RecordRandomCall(UGSDDeterminismManager::ZombieWanderCategory, DirectionChange);
                        }
                        else
                        {
                            // Fallback to seeded random for determinism even without manager
                            static FRandomStream FallbackWanderStream(54321);
                            DirectionChange = FallbackWanderStream.FRandRange(-WanderDirectionChange, WanderDirectionChange);
                        }
                        State.WanderDirection += DirectionChange;
                        State.WanderDirection = FMath::Clamp(State.WanderDirection, -180.0f, 180.0f);
                    }
                }

                // Smooth speed interpolation
                // Uses Config->SpeedInterpolationRate instead of hardcoded 2.0f
                State.MovementSpeed = FMath::Lerp(State.MovementSpeed, State.TargetMovementSpeed, DeltaTime * SpeedInterpolationRate);
            }
        });
}
