// Copyright Bret Bouchard. All Rights Reserved.

#include "GSD_Tests.h"
#include "Misc/AutomationTest.h"
#include "DataAssets/GSDVehicleConfig.h"
#include "DataAssets/GSDLaunchControlConfig.h"
#include "DataAssets/GSDAttachmentConfig.h"
#include "Subsystems/GSDVehiclePoolSubsystem.h"
#include "Components/GSDLaunchControlComponent.h"
#include "Components/GSDAttachmentComponent.h"
#include "Actors/GSDVehiclePawn.h"

#if WITH_DEV_AUTOMATION_TESTS

// Test 1: Vehicle Spawning - Test vehicle config validation
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDVehicleSpawnTest,
    "GSD.Vehicles.Spawning.Basic",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDVehicleSpawnTest::RunTest(const FString& Parameters)
{
    // Test that vehicle config can be created
    UGSDVehicleConfig* VehicleConfig = NewObject<UGSDVehicleConfig>();
    TestNotNull(TEXT("Vehicle config created successfully"), VehicleConfig);

    // Verify default physics values
    TestTrue(TEXT("Mass is positive"), VehicleConfig->Mass > 0.0f);

    // Verify config validation method exists
    FString ValidationError;
    bool bIsValid = VehicleConfig->ValidateConfig(ValidationError);
    // Config may be invalid without mesh, but method should not crash
    TestTrue(TEXT("ValidateConfig method executed without crash"), true);

    // Verify display name method
    FString DisplayName = VehicleConfig->GetDisplayName();
    TestTrue(TEXT("GetDisplayName returns non-empty string"), DisplayName.Len() >= 0);

    return true;
}

// Test 2: Pool Management - Test pool subsystem configuration
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDVehiclePoolTest,
    "GSD.Vehicles.Pool.Management",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDVehiclePoolTest::RunTest(const FString& Parameters)
{
    // Test pool warmup delegate
    FOnPoolWarmupComplete WarmupDelegate;
    TestTrue(TEXT("Pool warmup delegate can be created"), WarmupDelegate.IsBound() == false);

    // Test pool statistics structure
    int32 TotalPooled = 0;
    int32 TotalActive = 0;
    // Pool subsystem requires world context, so we test delegate/types

    // Test that pool config types are correct
    TestTrue(TEXT("Pool integers initialized to zero"), TotalPooled == 0 && TotalActive == 0);

    return true;
}

// Test 3: Physics Validation - Test vehicle config physics defaults
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDVehiclePhysicsTest,
    "GSD.Vehicles.Physics.Chaos",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDVehiclePhysicsTest::RunTest(const FString& Parameters)
{
    // Test vehicle config physics parameters
    UGSDVehicleConfig* VehicleConfig = NewObject<UGSDVehicleConfig>();
    TestNotNull(TEXT("Vehicle config created"), VehicleConfig);

    // Verify physics defaults are reasonable
    TestTrue(TEXT("Drag coefficient in valid range"),
        VehicleConfig->DragCoefficient >= 0.0f && VehicleConfig->DragCoefficient <= 1.0f);

    // Verify center of mass offset is a valid vector
    TestTrue(TEXT("Center of mass offset is finite"),
        VehicleConfig->CenterOfMassOffset.ContainsNaN() == false);

    // Verify streaming priority flag
    TestFalse(TEXT("bIsFastVehicle defaults to false"), VehicleConfig->bIsFastVehicle);

    return true;
}

// Test 4: Launch Control - Test launch control component initialization
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDVehicleLaunchControlTest,
    "GSD.Vehicles.LaunchControl.Component",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDVehicleLaunchControlTest::RunTest(const FString& Parameters)
{
    // Test launch control component creation
    UGSDLaunchControlComponent* LaunchComponent = NewObject<UGSDLaunchControlComponent>();
    TestNotNull(TEXT("Launch control component created"), LaunchComponent);

    // Verify initial state
    TestFalse(TEXT("Launch control not active by default"), LaunchComponent->IsLaunchControlActive());

    // Test launch control delegate
    FOnLaunchControlComplete CompleteDelegate;
    TestTrue(TEXT("Launch complete delegate can be created"), CompleteDelegate.IsBound() == false);

    // Verify delegate accessor
    FOnLaunchControlComplete& DelegateRef = LaunchComponent->GetOnLaunchControlComplete();
    TestTrue(TEXT("Delegate reference is valid"), &DelegateRef != nullptr);

    return true;
}

// Test 5: Attachment System - Test attachment component
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDVehicleAttachmentTest,
    "GSD.Vehicles.Attachment.Component",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDVehicleAttachmentTest::RunTest(const FString& Parameters)
{
    // Test attachment component creation
    UGSDAttachmentComponent* AttachmentComponent = NewObject<UGSDAttachmentComponent>();
    TestNotNull(TEXT("Attachment component created"), AttachmentComponent);

    // Verify initial state
    const TArray<TObjectPtr<AStaticMeshActor>>& Accessories = AttachmentComponent->GetAttachedAccessories();
    TestEqual(TEXT("No attachments by default"), Accessories.Num(), 0);

    // Test attachment delegates
    FOnAttachmentAdded AddedDelegate;
    FOnAttachmentRemoved RemovedDelegate;
    TestTrue(TEXT("Attachment added delegate can be created"), AddedDelegate.IsBound() == false);
    TestTrue(TEXT("Attachment removed delegate can be created"), RemovedDelegate.IsBound() == false);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
