// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "GSDNavigationProcessor.generated.h"

class UZoneGraphSubsystem;
struct FGSDNavigationFragment;
struct FGSDZombieStateFragment;
struct FDataFragment_Transform;
class UGSDDeterminismManager;

/**
 * Navigation processor for ZoneGraph-based crowd movement.
 * Moves entities along ZoneGraph lanes with fallback to direct movement.
 * Applies velocity randomization to prevent synchronized movement (CROWD-08).
 */
UCLASS()
class GSD_CROWDS_API UGSDNavigationProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UGSDNavigationProcessor();

protected:
    // ~UMassProcessor interface
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
    // ~End of UMassProcessor interface

    //-- Helper Methods --

    /** Find nearest lane to entity and bind to it */
    void FindNearestLane(
        FGSDNavigationFragment& Nav,
        const FDataFragment_Transform& Transform,
        const UZoneGraphSubsystem* ZoneGraphSubsystem) const;

    /** Update entity transform from current lane position */
    void UpdateTransformFromLane(
        FGSDNavigationFragment& Nav,
        FDataFragment_Transform& Transform,
        const UZoneGraphSubsystem* ZoneGraphSubsystem) const;

    /** Check if entity reached end of lane, transition to next if available */
    void CheckLaneProgress(
        FGSDNavigationFragment& Nav,
        const UZoneGraphSubsystem* ZoneGraphSubsystem) const;

    /** Fallback movement when ZoneGraph unavailable */
    void ExecuteFallbackMovement(
        FGSDNavigationFragment& Nav,
        FDataFragment_Transform& Transform,
        const FGSDZombieStateFragment& Zombie,
        float DeltaTime) const;

    /** Pick a random nearby lane for wandering */
    FZoneGraphLaneHandle PickRandomNearbyLane(
        const FVector& Location,
        const UZoneGraphSubsystem* ZoneGraphSubsystem) const;

    /** Apply velocity randomization to prevent synchronized movement (CROWD-08) */
    float ApplyVelocityRandomization(float BaseSpeed, float RandomizationPercent) const;

private:
    FMassEntityQuery EntityQuery;

    //-- Configuration --
    UPROPERTY(EditDefaultsOnly, Category = "Configuration")
    float LaneSearchRadius = 2000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Configuration")
    float FallbackMoveSpeed = 100.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Configuration")
    float VelocityRandomizationPercent = 20.0f;
};
