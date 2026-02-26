// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntity/Types/MassEntityTypes.h"
#include "SmartObjectsModule/SmartObjectTypes.h"
#include "GSDSmartObjectFragment.generated.h"

/**
 * Smart Object fragment for interaction state.
 * Stores claim handle and interaction progress.
 *
 * CRITICAL: Do NOT store UObject pointers in fragments.
 * Fragments are not UObjects and cannot hold strong references.
 * Use handles and raw data instead.
 */
USTRUCT()
struct GSD_CROWDS_API FGSDSmartObjectFragment : public FMassFragment
{
    GENERATED_BODY()

    //-- Current Interaction --
    UPROPERTY()
    FSmartObjectClaimHandle ClaimedHandle;

    UPROPERTY()
    float InteractionTime = 0.0f;

    UPROPERTY()
    float InteractionDuration = 3.0f;  // Default 3 seconds

    //-- State --
    UPROPERTY()
    uint8 bIsInteracting : 1;

    UPROPERTY()
    uint8 bHasClaimedObject : 1;

    UPROPERTY()
    uint8 bInteractionComplete : 1;

    //-- Search Config --
    UPROPERTY()
    float SearchRadius = 1000.0f;

    UPROPERTY()
    float SearchCooldown = 5.0f;

    UPROPERTY()
    float TimeSinceLastSearch = 0.0f;

    FGSDSmartObjectFragment()
        : bIsInteracting(false)
        , bHasClaimedObject(false)
        , bInteractionComplete(false)
    {}

    bool HasValidClaim() const
    {
        return bHasClaimedObject && ClaimedHandle.IsValid();
    }
};
