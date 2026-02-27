// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GSDTelemetryTypes.generated.h"

/**
 * Circular buffer for frame time history with O(1) averaging
 * Based on Phase 6 pattern from GSDStreamingTelemetry
 */
USTRUCT(BlueprintType)
struct FGSDFrameTimeHistory
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telemetry")
    TArray<float> FrameTimes;

    int32 WriteIndex = 0;
    float TotalTime = 0.0f;

    void AddFrameTime(float TimeMs)
    {
        if (FrameTimes.Num() < MaxSize)
        {
            FrameTimes.Add(TimeMs);
            TotalTime += TimeMs;
        }
        else
        {
            // Circular buffer: overwrite oldest
            TotalTime -= FrameTimes[WriteIndex];
            FrameTimes[WriteIndex] = TimeMs;
            TotalTime += TimeMs;
            WriteIndex = (WriteIndex + 1) % MaxSize;
        }
    }

    float GetAverageMs() const
    {
        return FrameTimes.Num() > 0 ? TotalTime / FrameTimes.Num() : 0.0f;
    }

    int32 GetSampleCount() const { return FrameTimes.Num(); }

    void Reset()
    {
        FrameTimes.Empty();
        WriteIndex = 0;
        TotalTime = 0.0f;
    }

    static constexpr int32 MaxSize = 60;  // 60 frame history for 1-second average at 60fps
};

/**
 * Hitch event recorded when frame time exceeds threshold
 */
USTRUCT(BlueprintType)
struct FGSDHitchEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Telemetry")
    float HitchTimeMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Telemetry")
    FName DistrictName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Telemetry")
    double Timestamp = 0.0;
};

/**
 * Actor count snapshot for periodic logging
 */
USTRUCT(BlueprintType)
struct FGSDActorCountSnapshot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Telemetry")
    int32 VehicleCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Telemetry")
    int32 ZombieCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Telemetry")
    int32 HumanCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Telemetry")
    double Timestamp = 0.0;
};

/**
 * Streaming cell load time record
 */
USTRUCT(BlueprintType)
struct FGSDCellLoadTimeRecord
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Telemetry")
    FName CellName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Telemetry")
    float LoadTimeMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Telemetry")
    FName DistrictName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Telemetry")
    double Timestamp = 0.0;
};
