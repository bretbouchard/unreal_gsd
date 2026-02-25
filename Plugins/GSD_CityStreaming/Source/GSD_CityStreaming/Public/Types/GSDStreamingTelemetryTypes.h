// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GSDStreamingTelemetryTypes.generated.h"

/**
 * Individual streaming event for tracking cell load performance.
 */
USTRUCT(BlueprintType)
struct GSD_CITYSTREAMING_API FGSDStreamingEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
    FString CellName;

    UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
    float LoadTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
    FVector PlayerPosition = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
    float PlayerSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
    float Timestamp = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
    TArray<FString> ActiveLayers;
};

/**
 * Aggregated telemetry data for performance monitoring.
 * Designed for consumption by GSD backend monitoring system (Phase 10).
 */
USTRUCT(BlueprintType)
struct GSD_CITYSTREAMING_API FGSDStreamingTelemetryData
{
    GENERATED_BODY()

    // Cell counts
    UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
    int32 TotalCells = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
    int32 LoadedCells = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
    int32 StreamingCells = 0;

    // Performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
    int32 StreamingOperationsThisFrame = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
    float AverageLoadTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
    float PeakLoadTimeMs = 0.0f;

    // Memory (for Phase 10 integration)
    UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
    int64 EstimatedMemoryBytes = 0;

    // Bottleneck tracking
    UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
    FString BottleneckCell;
};

// Delegate for individual event logging (throttled)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStreamingEventLogged, const FGSDStreamingEvent&, Event);

// Delegate for batched telemetry updates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStreamingTelemetryUpdated, const FGSDStreamingTelemetryData&, Data);
