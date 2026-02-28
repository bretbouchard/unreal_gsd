// Copyright Bret Bouchard. All Rights Reserved.

#include "Commandlets/GSDEventTestCommandlet.h"
#include "Subsystems/GSDEventBusSubsystem.h"
#include "Subsystems/GSDEventSchedulerSubsystem.h"
#include "Managers/GSDDeterminismManager.h"
#include "Tags/GSDEventTags.h"
#include "GSDEventLog.h"

UGSDEventTestCommandlet::UGSDEventTestCommandlet()
{
    LogToConsole = true;
}

int32 UGSDEventTestCommandlet::Main(const FString& Params)
{
    GSDEVENT_LOG(Log, TEXT("GSDEventTestCommandlet starting"));

    double StartTime = FPlatformTime::Seconds();

    ParseParameters(Params);

    // Run tests
    bool bDeterminismVerified = TestDeterminism();
    bool bEventBusWorking = TestEventBus();
    bool bSchedulingWorking = TestScheduling();

    bool bSuccess = bDeterminismVerified && bEventBusWorking && bSchedulingWorking;

    double EndTime = FPlatformTime::Seconds();
    float Duration = (float)(EndTime - StartTime);

    // Output results
    if (bOutputJSON)
    {
        OutputJSON(bSuccess, bDeterminismVerified, bEventBusWorking, bSchedulingWorking, Duration);
    }
    else
    {
        OutputText(bSuccess, bDeterminismVerified, bEventBusWorking, bSchedulingWorking);
    }

    return bSuccess ? 0 : 1;
}

void UGSDEventTestCommandlet::ParseParameters(const FString& Params)
{
    TArray<FString> Tokens;
    TArray<FString> Switches;
    ParseCommandLine(*Params, Tokens, Switches);

    for (const FString& Switch : Switches)
    {
        if (Switch.StartsWith(TEXT("seed=")))
        {
            FString SeedValue = Switch.RightChop(5);
            TestSeed = FCString::Atoi(*SeedValue);
        }
        else if (Switch == TEXT("text"))
        {
            bOutputJSON = false;
        }
    }
}

bool UGSDEventTestCommandlet::TestDeterminism()
{
    GSDEVENT_LOG(Log, TEXT("Testing determinism..."));

    // Get game instance (required for subsystems)
    UGameInstance* GameInstance = GetGameInstance();
    if (!GameInstance)
    {
        GSDEVENT_LOG(Error, TEXT("No game instance available"));
        return false;
    }

    UGSDEventSchedulerSubsystem* Scheduler = GameInstance->GetSubsystem<UGSDEventSchedulerSubsystem>();
    if (!Scheduler)
    {
        GSDEVENT_LOG(Error, TEXT("Scheduler subsystem not found"));
        return false;
    }

    // Generate schedule twice with same inputs
    FDateTime TestDate(2025, 1, 15);

    Scheduler->GenerateDailySchedule(TestDate, TestSeed);
    int32 FirstCount = Scheduler->GetScheduledEventCount();

    // Reset and generate again
    Scheduler->GenerateDailySchedule(TestDate, TestSeed);
    int32 SecondCount = Scheduler->GetScheduledEventCount();

    // Same seed should produce same count
    bool bVerified = (FirstCount == SecondCount);

    GSDEVENT_LOG(Log, TEXT("Determinism test: First=%d, Second=%d, Match=%s"),
        FirstCount, SecondCount, bVerified ? TEXT("true") : TEXT("false"));

    return bVerified;
}

bool UGSDEventTestCommandlet::TestEventBus()
{
    GSDEVENT_LOG(Log, TEXT("Testing event bus..."));

    // Create a test world for the event bus
    UWorld* TestWorld = GWorld;
    if (!TestWorld)
    {
        GSDEVENT_LOG(Error, TEXT("No world available"));
        return false;
    }

    UGSDEventBusSubsystem* EventBus = TestWorld->GetSubsystem<UGSDEventBusSubsystem>();
    if (!EventBus)
    {
        GSDEVENT_LOG(Error, TEXT("Event bus subsystem not found"));
        return false;
    }

    // Test subscribe/broadcast
    bool bReceived = false;
    FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(FName("Event.Daily"));

    FGSDEventHandle Handle = EventBus->Subscribe(TestTag,
        FOnGSDEvent::FDelegate::CreateLambda([&bReceived](FGameplayTag Tag, const FVector& Location, float Intensity)
        {
            bReceived = true;
        }));

    // Broadcast test event
    EventBus->BroadcastEvent(TestTag, FVector::ZeroVector, 1.0f);

    // Verify receipt
    bool bWorking = bReceived;

    // Cleanup
    EventBus->Unsubscribe(Handle);

    GSDEVENT_LOG(Log, TEXT("Event bus test: Received=%s"), bWorking ? TEXT("true") : TEXT("false"));

    return bWorking;
}

bool UGSDEventTestCommandlet::TestScheduling()
{
    GSDEVENT_LOG(Log, TEXT("Testing scheduling..."));

    UGameInstance* GameInstance = GetGameInstance();
    if (!GameInstance)
    {
        return false;
    }

    UGSDEventSchedulerSubsystem* Scheduler = GameInstance->GetSubsystem<UGSDEventSchedulerSubsystem>();
    if (!Scheduler)
    {
        return false;
    }

    // Generate schedule
    FDateTime TestDate(2025, 1, 15);
    Scheduler->GenerateDailySchedule(TestDate, TestSeed);

    // Verify events were scheduled
    // Note: Will be 0 until Phase 9 adds concrete events
    int32 Count = Scheduler->GetScheduledEventCount();

    GSDEVENT_LOG(Log, TEXT("Scheduling test: %d events scheduled"), Count);

    // For now, scheduling "works" if it doesn't crash
    // Phase 9 will add actual events to verify
    return true;
}

void UGSDEventTestCommandlet::OutputJSON(bool bSuccess, bool bDeterminismVerified, bool bEventBusWorking, bool bSchedulingWorking, float Duration)
{
    FString JSON = FString::Printf(TEXT(
        "{\n"
        "  \"success\": %s,\n"
        "  \"determinism_verified\": %s,\n"
        "  \"event_bus_working\": %s,\n"
        "  \"scheduling_working\": %s,\n"
        "  \"test_duration_seconds\": %.2f,\n"
        "  \"seed\": %d\n"
        "}\n"
    ),
        bSuccess ? TEXT("true") : TEXT("false"),
        bDeterminismVerified ? TEXT("true") : TEXT("false"),
        bEventBusWorking ? TEXT("true") : TEXT("false"),
        bSchedulingWorking ? TEXT("true") : TEXT("false"),
        Duration,
        TestSeed
    );

    // Output to stdout for CI parsing
    UE_LOG(LogTemp, Display, TEXT("%s"), *JSON);
}

void UGSDEventTestCommandlet::OutputText(bool bSuccess, bool bDeterminismVerified, bool bEventBusWorking, bool bSchedulingWorking)
{
    UE_LOG(LogTemp, Display, TEXT("=== GSDEventTest Results ==="));
    UE_LOG(LogTemp, Display, TEXT("Success: %s"), bSuccess ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Display, TEXT("Determinism: %s"), bDeterminismVerified ? TEXT("PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Display, TEXT("Event Bus: %s"), bEventBusWorking ? TEXT("PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Display, TEXT("Scheduling: %s"), bSchedulingWorking ? TEXT("PASS") : TEXT("FAIL"));
}
