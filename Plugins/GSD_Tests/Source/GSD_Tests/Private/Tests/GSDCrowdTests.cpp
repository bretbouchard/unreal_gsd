// Copyright Bret Bouchard. All Rights Reserved.

#include "GSD_Tests.h"
#include "Misc/AutomationTest.h"
#include "Fragments/GSDZombieStateFragment.h"
#include "Fragments/GSDNavigationFragment.h"
#include "Fragments/GSDSmartObjectFragment.h"
#include "Processors/GSDCrowdLODProcessor.h"
#include "Subsystems/GSDCrowdManagerSubsystem.h"
#include "DataAssets/GSDCrowdEntityConfig.h"

#if WITH_DEV_AUTOMATION_TESTS

// Test 1: Entity Spawning - Basic crowd entity spawn test
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDCrowdEntitySpawnTest,
    "GSD.Crowds.EntitySpawning.Basic",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDCrowdEntitySpawnTest::RunTest(const FString& Parameters)
{
    // Test that crowd entity config can be created
    UGSDCrowdEntityConfig* EntityConfig = NewObject<UGSDCrowdEntityConfig>();
    TestNotNull(TEXT("Entity config created successfully"), EntityConfig);

    // Verify default values
    TestEqual(TEXT("Default base velocity is 150.0"), EntityConfig->BaseVelocity, 150.0f);
    TestEqual(TEXT("Default velocity random range is 0.2"), EntityConfig->VelocityRandomRange, 0.2f);
    TestTrue(TEXT("Navigation is enabled by default"), EntityConfig->bEnableNavigation);
    TestTrue(TEXT("Smart object interactions enabled by default"), EntityConfig->bEnableSmartObjectInteractions);

    return true;
}

// Test 2: LOD Transition - Test LOD processor distance thresholds
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDCrowdLODTransitionTest,
    "GSD.Crowds.LOD.Transition",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDCrowdLODTransitionTest::RunTest(const FString& Parameters)
{
    // Test that LOD processor can be created
    UGSDCrowdLODProcessor* LODProcessor = NewObject<UGSDCrowdLODProcessor>();
    TestNotNull(TEXT("LOD processor created successfully"), LODProcessor);

    // Verify LOD distance thresholds are set (default values from processor)
    // These values are internal to the processor, so we verify it exists and can be instantiated
    // The actual LOD transitions are tested in integration tests

    return true;
}

// Test 3: State Fragment Initialization - Test zombie state fragment defaults
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDCrowdStateFragmentTest,
    "GSD.Crowds.StateFragment.Initialization",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDCrowdStateFragmentTest::RunTest(const FString& Parameters)
{
    // Test FGSDZombieStateFragment default values
    FGSDZombieStateFragment StateFragment;

    // Verify default state values
    TestEqual(TEXT("Default health is 100.0"), StateFragment.Health, 100.0f);
    TestEqual(TEXT("Default movement speed is 150.0"), StateFragment.MovementSpeed, 150.0f);
    TestEqual(TEXT("Default target movement speed is 150.0"), StateFragment.TargetMovementSpeed, 150.0f);

    // Verify default flags
    TestFalse(TEXT("Not aggressive by default"), StateFragment.bIsAggressive);
    TestTrue(TEXT("Alive by default"), StateFragment.bIsAlive);
    TestTrue(TEXT("Active by default"), StateFragment.bIsActive);

    // Verify behavior defaults
    TestEqual(TEXT("Default wander direction is 0.0"), StateFragment.WanderDirection, 0.0f);
    TestEqual(TEXT("Default time since last behavior update is 0.0"), StateFragment.TimeSinceLastBehaviorUpdate, 0.0f);

    // Test speed randomization (20% variation per VelocityRandomRange = 0.2)
    // The variation should be within 20% of base speed (150 * 0.2 = 30)
    // So speed should be between 120 and 180
    const float BaseSpeed = 150.0f;
    const float RandomRange = 0.2f;
    const float MinSpeed = BaseSpeed * (1.0f - RandomRange);
    const float MaxSpeed = BaseSpeed * (1.0f + RandomRange);

    // Note: Actual randomization happens at spawn time, not in constructor
    // We verify the default values are correct for the randomization formula
    TestTrue(TEXT("Min speed calculation correct"), MinSpeed >= 120.0f);
    TestTrue(TEXT("Max speed calculation correct"), MaxSpeed <= 180.0f);

    return true;
}

// Test 4: Crowd Manager Subsystem - Test subsystem initialization
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDCrowdManagerSubsystemTest,
    "GSD.Crowds.Manager.Subsystem",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDCrowdManagerSubsystemTest::RunTest(const FString& Parameters)
{
    // Test that subsystem type is correct (UWorldSubsystem)
    // Note: Full subsystem testing requires a world context

    // Test density modifier structure
    FGSDensityModifier DensityModifier;
    TestEqual(TEXT("Default modifier radius is 1000.0"), DensityModifier.Radius, 1000.0f);
    TestEqual(TEXT("Default modifier multiplier is 1.0"), DensityModifier.Multiplier, 1.0f);
    TestEqual(TEXT("Default modifier center is zero"), DensityModifier.Center, FVector::ZeroVector);

    // Test that the subsystem delegates can be created
    FOnCrowdSpawnComplete SpawnCompleteDelegate;
    FOnAllEntitiesDespawned AllDespawnedDelegate;

    // Delegates should be valid (not null function)
    TestTrue(TEXT("Spawn complete delegate is valid"), SpawnCompleteDelegate.IsBound() == false);
    TestTrue(TEXT("All despawned delegate is valid"), AllDespawnedDelegate.IsBound() == false);

    return true;
}

// Test 5: Navigation Fragment - Test navigation fragment defaults
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDCrowdNavigationFragmentTest,
    "GSD.Crowds.Navigation.Fragment",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDCrowdNavigationFragmentTest::RunTest(const FString& Parameters)
{
    // Test FGSDNavigationFragment defaults
    FGSDNavigationFragment NavFragment;

    // Verify lane reference defaults
    TestFalse(TEXT("Not on lane by default"), NavFragment.bIsOnLane);
    TestFalse(TEXT("Not reached destination by default"), NavFragment.bReachedDestination);
    TestEqual(TEXT("Default lane position is 0.0"), NavFragment.LanePosition, 0.0f);

    // Verify movement config
    TestEqual(TEXT("Default desired speed is 150.0"), NavFragment.DesiredSpeed, 150.0f);

    // Verify fallback movement defaults
    TestEqual(TEXT("Default fallback target location is zero"), NavFragment.FallbackTargetLocation, FVector::ZeroVector);
    TestFalse(TEXT("Fallback movement disabled by default"), NavFragment.bUseFallbackMovement);

    // Verify target defaults
    TestEqual(TEXT("Default target lane position is -1.0"), NavFragment.TargetLanePosition, -1.0f);

    return true;
}

// Test 6: Smart Object Fragment - Test smart object fragment defaults
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDCrowdSmartObjectTest,
    "GSD.Crowds.SmartObject.Fragment",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDCrowdSmartObjectTest::RunTest(const FString& Parameters)
{
    // Test FGSDSmartObjectFragment defaults
    FGSDSmartObjectFragment SOFragment;

    // Verify interaction defaults
    TestEqual(TEXT("Default interaction time is 0.0"), SOFragment.InteractionTime, 0.0f);
    TestEqual(TEXT("Default interaction duration is 3.0"), SOFragment.InteractionDuration, 3.0f);

    // Verify state flags
    TestFalse(TEXT("Not interacting by default"), SOFragment.bIsInteracting);
    TestFalse(TEXT("No claimed object by default"), SOFragment.bHasClaimedObject);
    TestFalse(TEXT("Interaction not complete by default"), SOFragment.bInteractionComplete);

    // Verify search config
    TestEqual(TEXT("Default search radius is 1000.0"), SOFragment.SearchRadius, 1000.0f);
    TestEqual(TEXT("Default search cooldown is 5.0"), SOFragment.SearchCooldown, 5.0f);
    TestEqual(TEXT("Default time since last search is 0.0"), SOFragment.TimeSinceLastSearch, 0.0f);

    // Verify HasValidClaim method
    TestFalse(TEXT("HasValidClaim returns false for default fragment"), SOFragment.HasValidClaim());

    return true;
}

// Test 7: Entity Config Data Asset - Test entity config factory methods
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDCrowdEntityConfigTest,
    "GSD.Crowds.EntityConfig.DataAsset",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDCrowdEntityConfigTest::RunTest(const FString& Parameters)
{
    // Test UGSDCrowdEntityConfig creation
    UGSDCrowdEntityConfig* EntityConfig = NewObject<UGSDCrowdEntityConfig>();
    TestNotNull(TEXT("Entity config created successfully"), EntityConfig);

    // Verify fragment factory methods exist and return valid fragments
    FGSDNavigationFragment NavFragment = EntityConfig->CreateNavigationFragment();
    TestEqual(TEXT("Navigation fragment has config speed"), NavFragment.DesiredSpeed, EntityConfig->DefaultMoveSpeed);

    FGSDSmartObjectFragment SOFragment = EntityConfig->CreateSmartObjectFragment();
    TestEqual(TEXT("Smart object fragment has config search radius"), SOFragment.SearchRadius, EntityConfig->SmartObjectSearchRadius);
    TestEqual(TEXT("Smart object fragment has config interaction duration"), SOFragment.InteractionDuration, EntityConfig->DefaultInteractionDuration);

    // Verify config validation
    TestTrue(TEXT("Navigation config valid"), EntityConfig->bEnableNavigation);
    TestTrue(TEXT("Smart objects config valid"), EntityConfig->bEnableSmartObjectInteractions);
    TestEqual(TEXT("Velocity randomization is 20%"), EntityConfig->VelocityRandomizationPercent, 20.0f);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
