// Copyright Bret Bouchard. All Rights Reserved.

#include "Subsystems/GSDStreamingTelemetry.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "HAL/PlatformTime.h"
#include "GSDLog.h"

void UGSDStreamingTelemetry::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // CRITICAL: Pre-allocate array to avoid runtime reallocations
    RecentEvents.Reserve(MaxRecentEvents);

    // Bind to world partition (version-dependent, see notes)
    BindToWorldPartition();

    GSD_LOG(Log, TEXT("GSDStreamingTelemetry: Initialized (MaxEvents=%d, BroadcastInterval=%.2f)"),
        MaxRecentEvents, MinBroadcastInterval);
}

void UGSDStreamingTelemetry::Deinitialize()
{
    UnbindFromWorldPartition();
    RecentEvents.Empty();
    Super::Deinitialize();
}

void UGSDStreamingTelemetry::LogStreamingEvent(const FString& CellName, float LoadTimeMs,
    const FVector& PlayerPosition, float PlayerSpeed)
{
    FGSDStreamingEvent Event;
    Event.CellName = CellName;
    Event.LoadTimeMs = LoadTimeMs;
    Event.PlayerPosition = PlayerPosition;
    Event.PlayerSpeed = PlayerSpeed;
    Event.Timestamp = FPlatformTime::Seconds();

    // Add event
    RecentEvents.Add(Event);

    // Trim if over max (remove oldest)
    if (RecentEvents.Num() > MaxRecentEvents)
    {
        RecentEvents.RemoveAt(0, RecentEvents.Num() - MaxRecentEvents);
    }

    // Update bottleneck tracking
    UpdateBottleneckTracking(Event);

    // CRITICAL: Throttled broadcasting to prevent frame budget overrun
    TryBroadcastEvent(Event);
}

void UGSDStreamingTelemetry::TryBroadcastEvent(const FGSDStreamingEvent& Event)
{
    if (!bEnableBroadcasting)
    {
        return;
    }

    float CurrentTime = FPlatformTime::Seconds();
    float TimeSinceLastBroadcast = CurrentTime - LastBroadcastTime;

    // CRITICAL: Enforce minimum broadcast interval
    if (TimeSinceLastBroadcast < MinBroadcastInterval)
    {
        // Skip broadcast to protect frame budget
        return;
    }

    LastBroadcastTime = CurrentTime;

    if (bUseBatchedMode)
    {
        // Broadcast aggregated data instead of individual events
        FGSDStreamingTelemetryData Data = GetAggregatedData();
        OnTelemetryUpdated.Broadcast(Data);
    }
    else
    {
        // Broadcast individual event
        OnStreamingEventLogged.Broadcast(Event);
    }
}

void UGSDStreamingTelemetry::UpdateBottleneckTracking(const FGSDStreamingEvent& Event)
{
    if (Event.LoadTimeMs > PeakLoadTimeMs)
    {
        PeakLoadTimeMs = Event.LoadTimeMs;
        BottleneckCell = Event.CellName;
    }
}

float UGSDStreamingTelemetry::GetAverageLoadTimeMs() const
{
    if (RecentEvents.Num() == 0)
    {
        return 0.0f;
    }

    float Total = 0.0f;
    for (const auto& Event : RecentEvents)
    {
        Total += Event.LoadTimeMs;
    }
    return Total / RecentEvents.Num();
}

FGSDStreamingTelemetryData UGSDStreamingTelemetry::GetAggregatedData() const
{
    FGSDStreamingTelemetryData Data;
    Data.LoadedCells = RecentEvents.Num();
    Data.AverageLoadTimeMs = GetAverageLoadTimeMs();
    Data.PeakLoadTimeMs = PeakLoadTimeMs;
    Data.BottleneckCell = BottleneckCell;
    return Data;
}

void UGSDStreamingTelemetry::ResetTelemetry()
{
    RecentEvents.Empty();
    RecentEvents.Reserve(MaxRecentEvents);
    PeakLoadTimeMs = 0.0f;
    BottleneckCell.Empty();
    LastBroadcastTime = 0.0f;
}

void UGSDStreamingTelemetry::BindToWorldPartition()
{
    // Note: Actual World Partition delegate binding varies by UE5 version
    // This is a placeholder - full integration in Phase 10
    // For now, telemetry is logged via LogStreamingEvent() calls
}

void UGSDStreamingTelemetry::UnbindFromWorldPartition()
{
    // Cleanup delegate binding
}

void UGSDStreamingTelemetry::OnStreamingProgressUpdated(UWorld* World)
{
    // Placeholder for World Partition progress callback
    // Full implementation in Phase 10
}
