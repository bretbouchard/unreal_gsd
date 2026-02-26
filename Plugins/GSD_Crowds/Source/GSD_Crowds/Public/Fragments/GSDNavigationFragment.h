// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntity/Types/MassEntityTypes.h"
#include "ZoneGraph/ZoneGraphTypes.h"
#include "GSDNavigationFragment.generated.h"

/**
 * Navigation fragment storing ZoneGraph lane reference and progress.
 * Used by navigation processor to move entities along defined lanes.
 *
 * CRITICAL: Do NOT store UObject pointers in fragments.
 * Fragments are not UObjects and cannot hold strong references.
 * Use lane handles and raw data instead.
 */
USTRUCT()
struct GSD_CROWDS_API FGSDNavigationFragment : public FMassFragment
{
    GENERATED_BODY()

    //-- Lane Reference --
    UPROPERTY()
    FZoneGraphLaneHandle CurrentLane;

    UPROPERTY()
    float LanePosition = 0.0f;  // Distance along lane

    UPROPERTY()
    uint8 bIsOnLane : 1;

    UPROPERTY()
    uint8 bReachedDestination : 1;

    //-- Target --
    UPROPERTY()
    FZoneGraphLaneHandle TargetLane;

    UPROPERTY()
    float TargetLanePosition = -1.0f;

    //-- Movement Config --
    UPROPERTY()
    float DesiredSpeed = 150.0f;

    //-- Fallback (when ZoneGraph unavailable) --
    UPROPERTY()
    FVector FallbackTargetLocation = FVector::ZeroVector;

    UPROPERTY()
    uint8 bUseFallbackMovement : 1;

    //-- Constructor --
    FGSDNavigationFragment()
        : bIsOnLane(false)
        , bReachedDestination(false)
        , bUseFallbackMovement(false)
    {}
};
