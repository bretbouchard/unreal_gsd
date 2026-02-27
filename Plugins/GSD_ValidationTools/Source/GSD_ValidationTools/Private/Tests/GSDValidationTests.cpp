// Copyright Bret Bouchard. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Types/GSDValidationTypes.h"
#include "Widgets/GSDValidationDashboardWidget.h"

#if WITH_DEV_AUTOMATION_TESTS

// Test: Validation result helper methods
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGSDValidationResultTest,
    "GSD.Validation.Types.ValidationResult",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDValidationResultTest::RunTest(const FString& Parameters)
{
    FGSDValidationResult Result;

    // Verify initial state
    TestTrue(TEXT("Result initially passed"), Result.bPassed);
    TestEqual(TEXT("Initial error count is 0"), Result.ErrorCount, 0);
    TestEqual(TEXT("Initial warning count is 0"), Result.WarningCount, 0);

    // Add error
    Result.AddError(TEXT("/Game/Test/Asset"), TEXT("SizeExceeded"),
        TEXT("Asset exceeds budget"), TEXT("Reduce size"));

    TestFalse(TEXT("Result failed after error"), Result.bPassed);
    TestEqual(TEXT("Error count is 1"), Result.ErrorCount, 1);
    TestEqual(TEXT("Issues array has 1 item"), Result.Issues.Num(), 1);

    // Add warning
    Result.AddWarning(TEXT("/Game/Test/Asset2"), TEXT("HighSize"),
        TEXT("Asset approaching budget"), TEXT("Consider reducing"));

    TestFalse(TEXT("Result still failed"), Result.bPassed);
    TestEqual(TEXT("Error count still 1"), Result.ErrorCount, 1);
    TestEqual(TEXT("Warning count is 1"), Result.WarningCount, 1);
    TestEqual(TEXT("Issues array has 2 items"), Result.Issues.Num(), 2);

    return true;
}

// Test: Asset budget configuration
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGSDAssetBudgetTest,
    "GSD.Validation.Types.AssetBudget",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDAssetBudgetTest::RunTest(const FString& Parameters)
{
    FGSDAssetBudget Budget;

    Budget.AssetType = TEXT("StaticMesh");
    Budget.MaxSizeMB = 50.0f;
    Budget.Description = TEXT("Static mesh budget");

    TestEqual(TEXT("Asset type is StaticMesh"), Budget.AssetType, TEXT("StaticMesh"));
    TestEqual(TEXT("Max size is 50.0 MB"), Budget.MaxSizeMB, 50.0f);
    TestFalse(TEXT("Description is not empty"), Budget.Description.IsEmpty());

    return true;
}

// Test: World Partition validation config
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGSDWPValidationConfigTest,
    "GSD.Validation.Types.WPValidationConfig",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDWPValidationConfigTest::RunTest(const FString& Parameters)
{
    FGSDWorldPartitionValidationConfig Config;

    // Verify defaults
    TestEqual(TEXT("Default min cell size is 12800 cm"),
        Config.MinCellSize, 12800.0f);

    TestEqual(TEXT("Default min HLOD layers is 3"),
        Config.MinHLODLayers, 3);

    TestEqual(TEXT("Default max loading range is 50000 cm"),
        Config.MaxLoadingRange, 50000.0f);

    TestTrue(TEXT("Data layers required by default"),
        Config.bRequireDataLayers);

    return true;
}

// Test: Performance route waypoint
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGSDPerfRouteWaypointTest,
    "GSD.Validation.Types.PerfRouteWaypoint",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDPerfRouteWaypointTest::RunTest(const FString& Parameters)
{
    FGSDPerfRouteWaypoint Waypoint;

    Waypoint.Location = FVector(1000.0f, 2000.0f, 0.0f);
    Waypoint.WaypointName = TEXT("TestWaypoint");
    Waypoint.ExpectedFrameTimeMs = 16.67f;

    TestEqual(TEXT("Location is correct"),
        Waypoint.Location, FVector(1000.0f, 2000.0f, 0.0f));

    TestEqual(TEXT("Waypoint name is correct"),
        Waypoint.WaypointName, TEXT("TestWaypoint"));

    TestEqual(TEXT("Expected frame time is 16.67ms"),
        Waypoint.ExpectedFrameTimeMs, 16.67f);

    return true;
}

// Test: Validation dashboard widget initialization
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGSDValidationDashboardWidgetTest,
    "GSD.Validation.Widget.Dashboard",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDValidationDashboardWidgetTest::RunTest(const FString& Parameters)
{
    // Create widget
    UGSDValidationDashboardWidget* Widget = NewObject<UGSDValidationDashboardWidget>();
    TestNotNull(TEXT("Widget created"), Widget);

    // Verify initial state
    TestFalse(TEXT("Not validating initially"), Widget->IsValidationRunning());
    TestFalse(TEXT("Last validation result is empty initially"),
        Widget->GetLastValidationPassed());

    // Verify methods exist (callable from Blueprint)
    TestTrue(TEXT("RunAllValidations method exists"),
        Widget->FindFunction(TEXT("RunAllValidations")) != nullptr);

    TestTrue(TEXT("ValidateAssets method exists"),
        Widget->FindFunction(TEXT("ValidateAssets")) != nullptr);

    TestTrue(TEXT("ValidateWorldPartition method exists"),
        Widget->FindFunction(TEXT("ValidateWorldPartition")) != nullptr);

    TestTrue(TEXT("RunPerformanceRoute method exists"),
        Widget->FindFunction(TEXT("RunPerformanceRoute")) != nullptr);

    return true;
}

// Test: Validation issue structure
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGSDValidationIssueTest,
    "GSD.Validation.Types.ValidationIssue",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDValidationIssueTest::RunTest(const FString& Parameters)
{
    FGSDValidationIssue Issue;

    Issue.AssetPath = TEXT("/Game/Test/Asset");
    Issue.IssueType = TEXT("SizeExceeded");
    Issue.Description = TEXT("Asset exceeds 100MB budget");
    Issue.Severity = 1.0f;  // Error
    Issue.Suggestion = TEXT("Reduce texture resolution");

    TestEqual(TEXT("Asset path is correct"), Issue.AssetPath, TEXT("/Game/Test/Asset"));
    TestEqual(TEXT("Issue type is correct"), Issue.IssueType, TEXT("SizeExceeded"));
    TestEqual(TEXT("Severity is 1.0 (error)"), Issue.Severity, 1.0f);
    TestFalse(TEXT("Suggestion is not empty"), Issue.Suggestion.IsEmpty());

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
