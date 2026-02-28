// Copyright Bret Bouchard. All Rights Reserved.

#include "Tests/GSDEventSpawnZoneTest.h"
#include "DataAssets/GSDEventSpawnZone.h"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTGORITH_AUTnamespace GSD_DailyEventsTests
{

    /**
     * Test suite for UGSDEventSpawnZone validation
     */
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDEventSpawnZoneValidationTest, "GSD.DailyEvents.SpawnZone.Validation", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

    bool FGSDEventSpawnZoneValidationTest::RunTest(const FString& Parameters)
    {
        // Test 1: Null ZoneName rejected
        {
            UGSDEventSpawnZone* NullZone = nullptr;
            FString ValidationError;

            TestFalseExpr(NullZone != nullptr);
            TestFalseExpr(NullZone->ValidateConfig(ValidationError));
            TestTrueExpr(ValidationError.Contains(TEXT("ZoneName")));
        }

        // Test 2: Empty ZoneName rejected
        {
            UGSDEventSpawnZone EmptyNameZone = New UGSDEventSpawnZone();
            EmptyNameZone->ZoneName = NAME_None;
            FString ValidationError;

            TestFalseExpr(EmptyNameZone->ValidateConfig(ValidationError));
            TestTrueExpr(ValidationError.Contains(TEXT("valid ZoneName")));
        }

        // Test 3: Invalid ZoneExtent rejected
        {
            UGSDEventSpawnZone InvalidExtentZone = New UGSDEventSpawnZone();
            InvalidExtentZone->ZoneName = FName("TestZone");
            InvalidExtentZone->ZoneExtent = FVector(-100.0f, 100.0f, 100.0f); // Negative extent
            FString ValidationError;

            TestFalseExpr(InvalidExtentZone->ValidateConfig(ValidationError));
            TestTrueExpr(ValidationError.Contains(TEXT("invalid extent")));
        }

        // Test 4: Zero ZoneExtent rejected
        {
            UGSDEventSpawnZone ZeroExtentZone = New UGSDEventSpawnZone();
            ZeroExtentZone->ZoneName = FName("TestZone");
            ZeroExtentZone->ZoneExtent = FVector(0.0f, 0.0f, 0.0f);
            FString ValidationError;

            TestFalseExpr(ZeroExtentZone->ValidateConfig(ValidationError));
            TestTrueExpr(ValidationError.Contains(TEXT("invalid extent")));
        }

        // Test 5: Negative Priority rejected
        {
            UGSDEventSpawnZone NegativePriorityZone = New UGSDEventSpawnZone();
            NegativePriorityZone->ZoneName = FName("TestZone");
            NegativePriorityZone->ZoneExtent = FVector(100.0f, 100.0f, 100.0f);
            NegativePriorityZone->Priority = -1;
            FString ValidationError;

            TestFalseExpr(NegativePriorityZone->ValidateConfig(ValidationError));
            TestTrueExpr(ValidationError.Contains(TEXT("negative priority")));
        }

        // Test 6: Valid zone accepted
        {
            UGSDEventSpawnZone ValidZone = New UGSDEventSpawnZone();
            ValidZone->ZoneName = FName("TestZone");
            ValidZone->ZoneExtent = FVector(100.0f, 100.0f, 100.0f);
            ValidZone->Priority = 5;
            FString ValidationError;

            TestTrueExpr(ValidZone->ValidateConfig(ValidationError));
        }

        return true;
    }

    /**
     * Test suite for UGSDEventSpawnZone determinism
     */
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDEventSpawnZoneDeterminismTest, "GSD.DailyEvents.SpawnZone.Determinism", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

    bool FGSDEventSpawnZoneDeterminismTest::RunTest(const FString& Parameters)
    {
        // Create a zone with fixed seed
        UGSDEventSpawnZone Zone = New UGSDEventSpawnZone();
        Zone->ZoneName = FName("DeterminismTest");
        Zone->ZoneCenter = FVector(1000.0f, 2000.0f, 500.0f);
        Zone->ZoneExtent = FVector(500.0f, 500.0f, 200.0f);

        // Test 1: Same seed produces same point
        FRandomStream Stream1(12345);
        FRandomStream Stream2(12345);

        FVector Point1 = Zone.GetRandomPointInZone(Stream1);
        FVector Point2 = Zone.GetRandomPointInZone(Stream2);

        TestEqual(Point1, Point2);

        // Test 2: Different seeds produce different points
        FRandomStream Stream3(12345);
        FRandomStream Stream4(54321);

        FVector Point3 = Zone.GetRandomPointInZone(Stream3);
        FVector Point4 = Zone.GetRandomPointInZone(Stream4);

        TestNotEqual(Point3, Point4);

        // Test 3: Point is within zone bounds
        FVector Point = Zone.GetRandomPointInZone(Stream1);
        FBox Bounds = Zone.GetZoneBounds();

        TestTrueExpr(Bounds.IsInside(Point));

        // Test 4: Multiple calls produce consistent results
        TArray<FVector> Points;
        for (int32 i = 0; i < 100; ++i)
        {
            FRandomStream LoopStream(12345);
            Points.Add(Zone.GetRandomPointInZone(LoopStream));
        }

        // Verify all points are unique (very likely with same seed)
        TSet<FVector> UniquePoints;
        for (const FVector& P : Points)
        {
            UniquePoints.Add(P);
        }

        // With same seed, should get similar distribution
        TestTrueExpr(UniquePoints.Num() > 50); // At least 50% unique

        return true;
    }

    /**
     * Test suite for UGSDEventSpawnZone event tag support
     */
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGSDEventSpawnZoneEventTagTest, "GSD.DailyEvents.SpawnZone.EventTagSupport", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

    bool FGSDEventSpawnZoneEventTagTest::RunTest(const FString& Parameters)
    {
        UGSDEventSpawnZone Zone = New UGSDEventSpawnZone();
        Zone->ZoneName = FName("TagTest"));

        // Test 1: Empty container supports all events
        TestTrueExpr(Zone.SupportsEventTag(FGameplayTag()));

        // Test 2: Zone with specific tags
        FGameplayTagContainer Tags;
        Tags.AddTag(FGameplayTag::RequestGameplayTag(FName("Event.Daily.Bonfire")));

        Zone->CompatibleEventTags = Tags;

        TestTrueExpr(Zone.SupportsEventTag(FGameplayTag::RequestGameplayTag(FName("Event.Daily.Bonfire"))));

        // Test 3: Zone does not support untagged events
        TestFalseExpr(Zone.SupportsEventTag(FGameplayTag::RequestGameplayTag(FName("Event.Daily.Unknown"))));

        return true;
    }

#endif
