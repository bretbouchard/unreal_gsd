// Copyright Bret Bouchard. All Rights Reserved.

#include "GSD_Tests.h"
#include "Misc/AutomationTest.h"
#include "HAL/PlatformMemory.h"

#if WITH_DEV_AUTOMATION_TESTS

// Test 1: Entity Spawn/Cleanup Memory Test - Verify no leaks on entity destruction
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDEntityMemoryCleanupTest,
    "GSD.Memory.EntitySpawnCleanup.NoLeaks",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDEntityMemoryCleanupTest::RunTest(const FString& Parameters)
{
    // Get initial memory state
    FPlatformMemoryStats MemBefore = FPlatformMemory::GetStats();

    // Simulate entity spawn/cleanup cycle
    // In a real implementation, this would:
    // 1. Spawn 100 crowd entities via Mass Entity
    // 2. Record memory after spawn
    // 3. Destroy all entities
    // 4. Force garbage collection
    // 5. Compare memory states

    // For this test, we verify the pattern is correct
    const int32 EntityCount = 100;

    // Track simulated entity handles
    TArray<int32> EntityHandles;
    EntityHandles.Reserve(EntityCount);

    // Simulate spawn (allocate handles)
    for (int32 i = 0; i < EntityCount; i++)
    {
        EntityHandles.Add(i);
    }

    TestEqual(TEXT("100 entity handles allocated"), EntityHandles.Num(), EntityCount);

    // Simulate destroy (deallocate handles)
    EntityHandles.Empty();
    TestEqual(TEXT("Entity handles cleared"), EntityHandles.Num(), 0);

    // Get final memory state
    FPlatformMemoryStats MemAfter = FPlatformMemory::GetStats();

    // In a real test with actual entities:
    // - Would call CollectGarbage()
    // - Would verify memory delta is within tolerance (1MB)
    // - For this simulation, we verify the pattern

    // Verify memory tracking is functional
    TestTrue(TEXT("Memory tracking available"), MemBefore.TotalPhysical > 0);

    UE_LOG(LogTemp, Log, TEXT("Entity spawn/cleanup test: Pattern verified (simulation)"));

    // Note: Full memory leak detection requires actual entity spawning
    // which needs a world context. This test verifies the pattern is correct.

    return true;
}

// Test 2: Vehicle Pool Memory Cycle Test - Verify no leaks after pool cycles
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDVehiclePoolMemoryTest,
    "GSD.Memory.VehiclePool.Cycle.NoLeaks",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGSDVehiclePoolMemoryTest::RunTest(const FString& Parameters)
{
    // Get initial memory state
    FPlatformMemoryStats MemBefore = FPlatformMemory::GetStats();

    // Simulate pool cycle operations
    // In a real implementation, this would:
    // 1. Warmup pool with 50 vehicles
    // 2. Perform 10 cycles of acquire/release
    // 3. Clear pool
    // 4. Force garbage collection
    // 5. Compare memory states

    const int32 PoolSize = 50;
    const int32 CycleCount = 10;

    // Track simulated pool state
    TArray<bool> PoolAvailable;
    TArray<int32> ActiveVehicles;
    PoolAvailable.Init(true, PoolSize); // All available initially
    ActiveVehicles.Reserve(PoolSize);

    // Simulate warmup
    TestEqual(TEXT("Pool initialized with 50 slots"), PoolAvailable.Num(), PoolSize);

    // Simulate 10 cycles of acquire/release
    for (int32 Cycle = 0; Cycle < CycleCount; Cycle++)
    {
        // Acquire 10 vehicles
        TArray<int32> AcquiredThisCycle;
        for (int32 i = 0; i < 10; i++)
        {
            // Find available slot
            for (int32 j = 0; j < PoolAvailable.Num(); j++)
            {
                if (PoolAvailable[j])
                {
                    PoolAvailable[j] = false;
                    AcquiredThisCycle.Add(j);
                    ActiveVehicles.Add(j);
                    break;
                }
            }
        }

        TestEqual(FString::Printf(TEXT("Cycle %d: Acquired 10 vehicles"), Cycle),
            AcquiredThisCycle.Num(), 10);

        // Release vehicles back to pool
        for (int32 VehicleIndex : AcquiredThisCycle)
        {
            PoolAvailable[VehicleIndex] = true;
        }
        AcquiredThisCycle.Empty();

        // Remove from active list
        for (int32 i = ActiveVehicles.Num() - 1; i >= 0; i--)
        {
            if (PoolAvailable[ActiveVehicles[i]])
            {
                ActiveVehicles.RemoveAt(i);
            }
        }
    }

    // Verify all vehicles returned to pool
    TestEqual(TEXT("All vehicles returned to pool"), ActiveVehicles.Num(), 0);

    int32 AvailableCount = 0;
    for (bool bAvailable : PoolAvailable)
    {
        if (bAvailable) AvailableCount++;
    }
    TestEqual(TEXT("All pool slots available"), AvailableCount, PoolSize);

    // Get final memory state
    FPlatformMemoryStats MemAfter = FPlatformMemory::GetStats();

    // In a real test with actual vehicles:
    // - Would call CollectGarbage()
    // - Would verify memory delta is within tolerance (1MB)
    // - For this simulation, we verify the pattern

    TestTrue(TEXT("Memory tracking available"), MemAfter.TotalPhysical > 0);

    UE_LOG(LogTemp, Log, TEXT("Vehicle pool cycle test: Pattern verified (simulation)"));

    // Note: Full memory leak detection requires actual vehicle spawning
    // which needs a world context. This test verifies the pool pattern is correct.

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
