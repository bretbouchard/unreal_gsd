#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Interfaces/IGSDSpawnable.h"
#include "Interfaces/IGSDStreamable.h"
#include "Interfaces/IGSDComponentSpawnable.h"
#include "Interfaces/IGSDSpatialAudioStreamable.h"
#include "Classes/GSDActor.h"
#include "Classes/GSDComponent.h"
#include "DataAssets/GSDDataAsset.h"

#if WITH_DEV_AUTOMATION_TESTS

// Test: IGSDSpawnable interface contract
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDSpawnableInterfaceTest,
    "GSD.Core.Interfaces.IGSDSpawnable",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDSpawnableInterfaceTest::RunTest(const FString& Parameters)
{
    // Create test actor implementing IGSDSpawnable
    AGSDActor* TestActor = NewObject<AGSDActor>();

    // Test: Initial spawn state is false
    TestFalse(TEXT("Initial IsSpawned should be false"),
        IGSDSpawnable::Execute_IsSpawned(TestActor));

    // Test: GetSpawnConfig returns nullptr initially
    TestNull(TEXT("Initial GetSpawnConfig should be null"),
        IGSDSpawnable::Execute_GetSpawnConfig(TestActor));

    // Test: SpawnAsync with valid config
    UGSDDataAsset* TestConfig = NewObject<UGSDDataAsset>();
    bool bCallbackExecuted = false;
    FOnSpawnComplete Callback;
    Callback.BindLambda([&bCallbackExecuted](AActor* SpawnedActor) {
        bCallbackExecuted = true;
    });

    IGSDSpawnable::Execute_SpawnAsync(TestActor, TestConfig, Callback);

    TestTrue(TEXT("SpawnAsync should set IsSpawned to true"),
        IGSDSpawnable::Execute_IsSpawned(TestActor));

    TestTrue(TEXT("SpawnAsync should execute callback"),
        bCallbackExecuted);

    TestEqual(TEXT("GetSpawnConfig should return set config"),
        IGSDSpawnable::Execute_GetSpawnConfig(TestActor), TestConfig);

    // Test: Despawn resets state
    IGSDSpawnable::Execute_Despawn(TestActor);
    TestFalse(TEXT("Despawn should set IsSpawned to false"),
        IGSDSpawnable::Execute_IsSpawned(TestActor));

    return true;
}

// Test: IGSDStreamable interface contract
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDStreamableInterfaceTest,
    "GSD.Core.Interfaces.IGSDStreamable",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDStreamableInterfaceTest::RunTest(const FString& Parameters)
{
    AGSDActor* TestActor = NewObject<AGSDActor>();

    // Test: GetStreamingBounds returns valid bounds
    FBoxSphereBounds Bounds = IGSDStreamable::Execute_GetStreamingBounds(TestActor);
    TestTrue(TEXT("GetStreamingBounds should return valid box"),
        Bounds.BoxExtent.Size() > 0.0f);

    // Test: Initial streaming state
    TestTrue(TEXT("Initial IsCurrentlyStreamedIn should be true"),
        IGSDStreamable::Execute_IsCurrentlyStreamedIn(TestActor));

    // Test: OnStreamIn/OnStreamOut
    IGSDStreamable::Execute_OnStreamOut(TestActor);
    TestFalse(TEXT("OnStreamOut should set IsCurrentlyStreamedIn to false"),
        IGSDStreamable::Execute_IsCurrentlyStreamedIn(TestActor));

    IGSDStreamable::Execute_OnStreamIn(TestActor);
    TestTrue(TEXT("OnStreamIn should set IsCurrentlyStreamedIn to true"),
        IGSDStreamable::Execute_IsCurrentlyStreamedIn(TestActor));

    return true;
}

// Test: IGSDComponentSpawnable interface contract
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDComponentSpawnableInterfaceTest,
    "GSD.Core.Interfaces.IGSDComponentSpawnable",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDComponentSpawnableInterfaceTest::RunTest(const FString& Parameters)
{
    UGSDComponent* TestComponent = NewObject<UGSDComponent>();

    // Test: GetComponentType returns valid name
    FName ComponentType = IGSDComponentSpawnable::Execute_GetComponentType(TestComponent);
    TestTrue(TEXT("GetComponentType should return valid name"),
        ComponentType != NAME_None);

    // Test: Initial active state
    TestFalse(TEXT("Initial IsComponentActive should be false"),
        IGSDComponentSpawnable::Execute_IsComponentActive(TestComponent));

    return true;
}

// Test: IGSDSpatialAudioStreamable interface contract
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDSpatialAudioStreamableInterfaceTest,
    "GSD.Core.Interfaces.IGSDSpatialAudioStreamable",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDSpatialAudioStreamableInterfaceTest::RunTest(const FString& Parameters)
{
    // This test requires an actor implementing IGSDSpatialAudioStreamable
    // For now, test that the struct works
    FGSDSpatialAudioState State;
    State.WorldLocation = FVector(100.0f, 200.0f, 300.0f);
    State.DistanceToListener = 500.0f;
    State.bIsOccluded = true;

    TestEqual(TEXT("SpatialAudioState WorldLocation X"),
        State.WorldLocation.X, 100.0f);
    TestEqual(TEXT("SpatialAudioState WorldLocation Y"),
        State.WorldLocation.Y, 200.0f);
    TestEqual(TEXT("SpatialAudioState WorldLocation Z"),
        State.WorldLocation.Z, 300.0f);
    TestEqual(TEXT("SpatialAudioState DistanceToListener"),
        State.DistanceToListener, 500.0f);
    TestTrue(TEXT("SpatialAudioState bIsOccluded"),
        State.bIsOccluded);

    // Test Reset
    State.Reset();
    TestEqual(TEXT("After Reset WorldLocation"),
        State.WorldLocation, FVector::ZeroVector);
    TestEqual(TEXT("After Reset DistanceToListener"),
        State.DistanceToListener, 0.0f);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
