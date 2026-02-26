// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "Commandlets/Commandlet.h"
#include "GSDCrowdTestCommandlet.generated.h"

/**
 * Commandlet for automated crowd system validation.
 *
 * Tests crowd spawning and performance without requiring editor interaction.
 * Designed for CI pipeline integration to verify crowd system functionality.
 *
 * Usage:
 *   UE-Editor.exe UnrealGSD.exe -run=GSDCrowdTest -targetfps=60 -entitycount=200
 *
 * Exit codes:
 *   0 = All tests passed
 *   1 = Test failed (performance or spawning issues)
 *
 * JSON Output:
 * {
 *   "success": true/false,
 *   "entity_count": 200,
 *   "average_fps": 60.5,
 *   "target_fps": 60.0,
 *   "performance_acceptable": true,
 *   "test_duration_seconds": 5.2
 * }
 */
UCLASS()
class GSD_CROWDS_API UGSDCrowdTestCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    UGSDCrowdTestCommandlet();

    // ~UCommandlet interface
    virtual int32 Main(const FString& Params) override;
    // ~End of UCommandlet interface

private:
    //-- Test Parameters --
    int32 TargetEntityCount = 200;
    float TargetFPS = 60.0f;
    float TestDuration = 5.0f;
    bool bOutputJSON = true;

    /**
     * Parse command line parameters.
     *
     * @param Params Command line parameters string
     */
    void ParseParameters(const FString& Params);

    /**
     * Run crowd spawn test.
     *
     * @param World World context for spawning
     * @param OutEntityCount Output: Number of entities spawned
     * @param OutAverageFPS Output: Average FPS during test
     * @return True if test completed successfully
     */
    bool RunCrowdTest(UWorld* World, int32& OutEntityCount, float& OutAverageFPS);

    /**
     * Output test results as JSON to stdout.
     *
     * @param bSuccess Whether test passed
     * @param EntityCount Number of entities spawned
     * @param AverageFPS Average FPS during test
     * @param TestDuration Duration of test in seconds
     */
    void OutputJSON(bool bSuccess, int32 EntityCount, float AverageFPS, float TestDuration);

    /**
     * Output test results as human-readable text.
     *
     * @param bSuccess Whether test passed
     * @param EntityCount Number of entities spawned
     * @param AverageFPS Average FPS during test
     */
    void OutputText(bool bSuccess, int32 EntityCount, float AverageFPS);

    //-- Phase 7: AI Verification --

    /**
     * Test navigation processor creation and fragment initialization.
     * Verifies ZoneGraph lane navigation system is functional.
     *
     * @return True if navigation processor tests pass
     */
    UFUNCTION()
    bool TestNavigationProcessor();

    /**
     * Test Smart Object processor and fragment initialization.
     * Verifies Smart Object interaction system is functional.
     *
     * @return True if Smart Object processor tests pass
     */
    UFUNCTION()
    bool TestSmartObjectProcessor();

    /**
     * Test Hero NPC and AI Controller class availability.
     * Verifies gameplay AI classes can be loaded.
     *
     * @return True if Hero NPC spawn tests pass
     */
    UFUNCTION()
    bool TestHeroNPCSpawn();

    //-- Phase 7 Test Parameters --
    UPROPERTY()
    int32 NavigationTestEntities = 50;

    UPROPERTY()
    int32 SmartObjectTestEntities = 20;

    UPROPERTY()
    int32 HeroNPCTestCount = 3;
};
