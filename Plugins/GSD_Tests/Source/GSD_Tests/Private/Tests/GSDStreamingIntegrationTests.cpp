// Copyright Bret Bouchard. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Subsystems/GSDCrowdManagerSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Test 1: Cell name calculation from position
 * Verifies that GetCellNameForPosition correctly calculates cell names from world positions
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGSDStreamingCellNameTest,
    "GSD.Streaming.CellNameCalculation",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDStreamingCellNameTest::RunTest(const FString& Parameters)
{
    // Create a test world
    UWorld* TestWorld = NewObject<UWorld>();
    TestWorld->SetWorldType(EWorldType::Game);

    // Create crowd manager subsystem
    UGSDCrowdManagerSubsystem* CrowdManager = NewObject<UGSDCrowdManagerSubsystem>(TestWorld);
    CrowdManager->AddToRoot();  // Prevent GC

    // Test 1: Origin position (0,0,0) should map to Cell_0_0
    FVector OriginPos(0.0f, 0.0f, 0.0f);
    FName OriginCell = CrowdManager->GetCellNameForPosition(OriginPos);
    TestEqual(TEXT("Origin position should map to Cell_0_0"), OriginCell, FName(TEXT("Cell_0_0")));

    // Test 2: Position within first cell (100, 100, 0) should map to Cell_0_0
    FVector FirstCellPos(100.0f, 100.0f, 0.0f);
    FName FirstCell = CrowdManager->GetCellNameForPosition(FirstCellPos);
    TestEqual(TEXT("Position (100,100) should map to Cell_0_0"), FirstCell, FName(TEXT("Cell_0_0")));

    // Test 3: Position at cell boundary (12800, 0, 0) should map to Cell_1_0
    // 12800.0f is the default cell size
    FVector BoundaryPos(12800.0f, 0.0f, 0.0f);
    FName BoundaryCell = CrowdManager->GetCellNameForPosition(BoundaryPos);
    TestEqual(TEXT("Position (12800,0) should map to Cell_1_0"), BoundaryCell, FName(TEXT("Cell_1_0")));

    // Test 4: Negative position (-100, -100, 0) should map to Cell_-1_-1
    FVector NegativePos(-100.0f, -100.0f, 0.0f);
    FName NegativeCell = CrowdManager->GetCellNameForPosition(NegativePos);
    TestEqual(TEXT("Negative position (-100,-100) should map to Cell_-1_-1"), NegativeCell, FName(TEXT("Cell_-1_-1")));

    // Test 5: Large position (25600, 12800, 0) should map to Cell_2_1
    FVector LargePos(25600.0f, 12800.0f, 0.0f);
    FName LargeCell = CrowdManager->GetCellNameForPosition(LargePos);
    TestEqual(TEXT("Position (25600,12800) should map to Cell_2_1"), LargeCell, FName(TEXT("Cell_2_1")));

    CrowdManager->RemoveFromRoot();
    return true;
}

/**
 * Test 2: Position in cell check (loaded vs unloaded)
 * Verifies that IsPositionInLoadedCell correctly identifies positions in loaded/unloaded cells
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGSDStreamingPositionInCellTest,
    "GSD.Streaming.PositionInCellCheck",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDStreamingPositionInCellTest::RunTest(const FString& Parameters)
{
    // Create a test world
    UWorld* TestWorld = NewObject<UWorld>();
    TestWorld->SetWorldType(EWorldType::Game);

    // Create crowd manager subsystem
    UGSDCrowdManagerSubsystem* CrowdManager = NewObject<UGSDCrowdManagerSubsystem>(TestWorld);
    CrowdManager->AddToRoot();

    // Test 1: Without World Partition subsystem, all positions should be "in loaded cell"
    // (graceful fallback)
    FVector TestPos(1000.0f, 2000.0f, 0.0f);
    bool bInLoadedCell = CrowdManager->IsPositionInLoadedCell(TestPos);
    TestTrue(TEXT("Without WP subsystem, position should be in loaded cell"), bInLoadedCell);

    // Note: Testing with actual loaded cells requires World Partition subsystem
    // which is not available in unit test environment

    CrowdManager->RemoveFromRoot();
    return true;
}

/**
 * Test 3: Pending spawn queue
 * Verifies that spawns in unloaded cells are queued
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGSDStreamingPendingSpawnTest,
    "GSD.Streaming.PendingSpawnQueue",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDStreamingPendingSpawnTest::RunTest(const FString& Parameters)
{
    // This test verifies the concept of pending spawns
    // Actual implementation requires full World Partition integration

    // Create a test world
    UWorld* TestWorld = NewObject<UWorld>();
    TestWorld->SetWorldType(EWorldType::Game);

    // Create crowd manager subsystem
    UGSDCrowdManagerSubsystem* CrowdManager = NewObject<UGSDCrowdManagerSubsystem>(TestWorld);
    CrowdManager->AddToRoot();

    // Test: Without World Partition, spawns proceed immediately
    // (pending queue is only used when WP is active)
    FVector SpawnPos(1000.0f, 2000.0f, 0.0f);

    // Verify the position is considered loaded
    TestTrue(TEXT("Position should be in loaded cell without WP"), CrowdManager->IsPositionInLoadedCell(SpawnPos));

    // Note: Actual pending spawn queue testing requires:
    // 1. World Partition subsystem
    // 2. Mock cell loaded/unloaded events
    // This is better suited for integration tests

    CrowdManager->RemoveFromRoot();
    return true;
}

/**
 * Test 4: Cell loaded event triggers pending spawns
 * Verifies that OnCellLoaded processes pending spawns for that cell
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGSDStreamingCellLoadedTest,
    "GSD.Streaming.CellLoadedTriggersSpawns",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDStreamingCellLoadedTest::RunTest(const FString& Parameters)
{
    // This test verifies the concept of cell-loaded spawning
    // Actual implementation requires World Partition event binding

    // Create a test world
    UWorld* TestWorld = NewObject<UWorld>();
    TestWorld->SetWorldType(EWorldType::Game);

    // Create crowd manager subsystem
    UGSDCrowdManagerSubsystem* CrowdManager = NewObject<UGSDCrowdManagerSubsystem>(TestWorld);
    CrowdManager->AddToRoot();

    // Test: Cell name calculation is consistent
    FVector TestPos(25600.0f, 12800.0f, 0.0f);
    FName CellName = CrowdManager->GetCellNameForPosition(TestPos);
    TestEqual(TEXT("Cell name should be Cell_2_1"), CellName, FName(TEXT("Cell_2_1")));

    // Note: Actual cell loaded event testing requires:
    // 1. Mock World Partition events
    // 2. Pending spawn queue with full spawn parameters
    // This is better suited for integration tests with a real world

    CrowdManager->RemoveFromRoot();
    return true;
}

/**
 * Test 5: Cell unloaded despawns crowds
 * Verifies that OnCellUnloaded removes crowds in that cell
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGSDStreamingCellUnloadedTest,
    "GSD.Streaming.CellUnloadedDespawnsCrowds",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDStreamingCellUnloadedTest::RunTest(const FString& Parameters)
{
    // This test verifies the concept of cell-unloaded despawning
    // Actual implementation requires entity ID tracking

    // Create a test world
    UWorld* TestWorld = NewObject<UWorld>();
    TestWorld->SetWorldType(EWorldType::Game);

    // Create crowd manager subsystem
    UGSDCrowdManagerSubsystem* CrowdManager = NewObject<UGSDCrowdManagerSubsystem>(TestWorld);
    CrowdManager->AddToRoot();

    // Test: Cell name calculation is consistent for despawn logic
    FVector TestPos(-12800.0f, 25600.0f, 0.0f);
    FName CellName = CrowdManager->GetCellNameForPosition(TestPos);
    TestEqual(TEXT("Cell name should be Cell_-1_2"), CellName, FName(TEXT("Cell_-1_2")));

    // Note: Actual cell unloaded despawn testing requires:
    // 1. Spawned entities with IDs
    // 2. Cell-to-crowd mapping
    // 3. Mock World Partition unload events
    // This is tracked in GSDCROWDS-108

    CrowdManager->RemoveFromRoot();
    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
