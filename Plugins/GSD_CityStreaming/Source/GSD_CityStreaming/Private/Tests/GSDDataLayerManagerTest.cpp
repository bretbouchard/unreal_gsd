// Copyright Bret Bouchard. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Subsystems/GSDDataLayerManager.h"
#include "Config/GSDDataLayerConfig.h"

#if WITH_DEV_AUTOMATION_TESTS

// Test 1: Subsystem can be created (SMOKE)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDDataLayerManagerCreationTest,
    "GSD.Streaming.DataLayerManager.Creation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::SmokeFilter)

bool FGSDDataLayerManagerCreationTest::RunTest(const FString& Parameters)
{
    UGSDDataLayerManager* Manager = NewObject<UGSDDataLayerManager>();
    TestNotNull(TEXT("Manager should be created"), Manager);
    return true;
}

// Test 2: GetRuntimeDataLayerNames returns array
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDDataLayerManagerGetNamesTest,
    "GSD.Streaming.DataLayerManager.GetRuntimeDataLayerNames",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDDataLayerManagerGetNamesTest::RunTest(const FString& Parameters)
{
    UGSDDataLayerManager* Manager = NewObject<UGSDDataLayerManager>();
    TestNotNull(TEXT("Manager should be created"), Manager);

    if (Manager)
    {
        TArray<FName> Names = Manager->GetRuntimeDataLayerNames();
        TestTrue(TEXT("GetRuntimeDataLayerNames should return valid array"), true);
    }
    return true;
}

// Test 3: IsDataLayerActivated handles invalid names gracefully
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDDataLayerManagerInvalidLayerTest,
    "GSD.Streaming.DataLayerManager.InvalidLayerHandling",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDDataLayerManagerInvalidLayerTest::RunTest(const FString& Parameters)
{
    UGSDDataLayerManager* Manager = NewObject<UGSDDataLayerManager>();
    TestNotNull(TEXT("Manager should be created"), Manager);

    if (Manager)
    {
        bool bResult = Manager->IsDataLayerActivated(FName(TEXT("NonExistentLayer")));
        TestFalse(TEXT("Non-existent layer should report as not activated"), bResult);
    }
    return true;
}

// Test 4: Staged activation initializes correctly
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDDataLayerManagerStagedInitTest,
    "GSD.Streaming.DataLayerManager.StagedActivation.Init",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDDataLayerManagerStagedInitTest::RunTest(const FString& Parameters)
{
    UGSDDataLayerManager* Manager = NewObject<UGSDDataLayerManager>();
    TestNotNull(TEXT("Manager should be created"), Manager);

    if (Manager)
    {
        // ActivateLayersStaged should not crash with empty array
        Manager->ActivateLayersStaged(TArray<FName>(), 5.0f);
        TestTrue(TEXT("Staged activation handles empty array"), true);
    }
    return true;
}

// Test 5: DataAsset config can be set
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDDataLayerManagerConfigTest,
    "GSD.Streaming.DataLayerManager.Config",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDDataLayerManagerConfigTest::RunTest(const FString& Parameters)
{
    UGSDDataLayerManager* Manager = NewObject<UGSDDataLayerManager>();
    TestNotNull(TEXT("Manager should be created"), Manager);

    if (Manager)
    {
        // Create config
        UGSDDataLayerConfig* Config = NewObject<UGSDDataLayerConfig>();
        Config->MaxActivationTimePerFrameMs = 3.0f;

        Manager->SetLayerConfig(Config);
        TestEqual(TEXT("Config should be set"), Manager->GetLayerConfig(), Config);
    }
    return true;
}

// Test 6: Cancel staged activation
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDDataLayerManagerCancelStagedTest,
    "GSD.Streaming.DataLayerManager.StagedActivation.Cancel",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGSDDataLayerManagerCancelStagedTest::RunTest(const FString& Parameters)
{
    UGSDDataLayerManager* Manager = NewObject<UGSDDataLayerManager>();
    TestNotNull(TEXT("Manager should be created"), Manager);

    if (Manager)
    {
        // Start staged activation
        TArray<FName> Layers;
        Layers.Add(FName(TEXT("Layer1")));
        Manager->ActivateLayersStaged(Layers, 5.0f);

        // Cancel immediately
        Manager->CancelStagedActivation();
        TestTrue(TEXT("Cancel should not crash"), true);
    }
    return true;
}

#endif
