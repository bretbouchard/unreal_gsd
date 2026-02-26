// Copyright Bret Bouchard. All Rights Reserved.

#include "DataAssets/GSDCrowdEntityConfig.h"
#include "Fragments/GSDZombieStateFragment.h"
#include "Fragments/GSDNavigationFragment.h"
#include "Fragments/GSDSmartObjectFragment.h"
#include "MassRepresentationFragments.h"
#include "MassCommonFragments.h"
#include "GSDCrowdLog.h"

UGSDCrowdEntityConfig::UGSDCrowdEntityConfig()
{
    // Set default velocity randomization
    BaseVelocity = 150.0f;
    VelocityRandomRange = 0.2f;
}

void UGSDCrowdEntityConfig::PostInitProperties()
{
    Super::PostInitProperties();

    // Log configuration for debugging
    UE_LOG(LOG_GSDCROWDS, Log, TEXT("GSDCrowdEntityConfig initialized - BaseVelocity: %.1f, RandomRange: %.2f"),
        BaseVelocity, VelocityRandomRange);
}

void UGSDCrowdEntityConfig::ConfigureFragmentTypes()
{
    // Add required fragments for crowd entities

    // Core transform fragment (required by Mass Entity)
    AddFragment<FDataFragment_Transform>();

    // Custom zombie state
    AddFragment<FGSDZombieStateFragment>();

    // LOD representation fragments
    AddFragment<FMassRepresentationFragment>();
    AddFragment<FMassRepresentationLODFragment>();

    // CRITICAL: Add Velocity Randomizer trait
    // This prevents all entities from moving in perfect synchronization
    // See RESEARCH.md Pitfall 1 for details
    // Note: Velocity Randomizer is added as a trait in Blueprint, not C++
    // The BaseVelocity and VelocityRandomRange properties are used by the trait

    UE_LOG(LOG_GSDCROWDS, Log, TEXT("Configured GSDCrowdEntityConfig fragments"));
}

FGSDNavigationFragment UGSDCrowdEntityConfig::CreateNavigationFragment() const
{
    FGSDNavigationFragment Fragment;
    Fragment.DesiredSpeed = DefaultMoveSpeed;
    Fragment.bUseFallbackMovement = false;
    Fragment.bIsOnLane = false;
    Fragment.bReachedDestination = false;
    return Fragment;
}

FGSDSmartObjectFragment UGSDCrowdEntityConfig::CreateSmartObjectFragment() const
{
    FGSDSmartObjectFragment Fragment;
    Fragment.SearchRadius = SmartObjectSearchRadius;
    Fragment.SearchCooldown = SmartObjectSearchCooldown;
    Fragment.InteractionDuration = DefaultInteractionDuration;
    Fragment.bIsInteracting = false;
    Fragment.bHasClaimedObject = false;
    Fragment.bInteractionComplete = false;
    return Fragment;
}
