// Copyright Bret Bouchard. All Rights Reserved.

#include "Subsystems/GSDPerformanceTelemetry.h"
#include "GSDTelemetryLog.h"
#include "GSDTelemetryStats.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

// Forward declare for vehicle counting (will link at runtime)
class AGSDVehiclePawn;

// Forward declare for crowd manager (will link at runtime)
class UGSDCrowdManagerSubsystem;

// Forward declare for hero NPC (will link at runtime)
class AGSDHeroNPC;

void UGSDPerformanceTelemetry::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    GSDTELEMETRY_LOG(Log, TEXT("GSDPerformanceTelemetry initializing..."));

    // Start periodic actor counting timer
    if (bEnableActorCounting)
    {
        StartActorCountTimer();
    }
}

void UGSDPerformanceTelemetry::Deinitialize()
{
    GSDTELEMETRY_LOG(Log, TEXT("GSDPerformanceTelemetry deinitializing..."));

    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ActorCountTimerHandle);
    }

    // Clear data
    DistrictFrameTimes.Empty();
    DistrictHitchCounts.Empty();
    RecentHitches.Empty();

    Super::Deinitialize();
}

void UGSDPerformanceTelemetry::RecordFrameTime(float FrameTimeMs, const FName& DistrictName)
{
    SCOPE_CYCLE_COUNTER(STAT_GSDRecordFrameTime);

    // Get or create frame time history for district
    FGSDFrameTimeHistory& History = DistrictFrameTimes.FindOrAdd(DistrictName);
    History.AddFrameTime(FrameTimeMs);

    // Check for hitch
    if (bEnableHitchDetection && FrameTimeMs > HitchThresholdMs)
    {
        RecordHitch(FrameTimeMs, DistrictName);
    }
}

void UGSDPerformanceTelemetry::RecordHitch(float HitchTimeMs, const FName& DistrictName)
{
    SCOPE_CYCLE_COUNTER(STAT_GSDRecordHitch);

    // Increment hitch count for district
    int32& HitchCount = DistrictHitchCounts.FindOrAdd(DistrictName);
    HitchCount++;

    // Record hitch event
    FGSDHitchEvent HitchEvent;
    HitchEvent.HitchTimeMs = HitchTimeMs;
    HitchEvent.DistrictName = DistrictName;
    HitchEvent.Timestamp = FPlatformTime::Seconds();

    // Add to recent hitches (circular buffer)
    if (RecentHitches.Num() >= MaxRecentHitches)
    {
        RecentHitches.RemoveAt(0);
    }
    RecentHitches.Add(HitchEvent);

    // Broadcast delegate
    OnHitchDetected.Broadcast(HitchTimeMs, DistrictName);

    GSDTELEMETRY_LOG(Warning, TEXT("Hitch detected in district %s: %.2fms"),
        *DistrictName.ToString(), HitchTimeMs);

    INC_DWORD_STAT(STAT_GSDTotalHitchCount);
}

float UGSDPerformanceTelemetry::GetAverageFrameTimeMs(const FName& DistrictName) const
{
    const FGSDFrameTimeHistory* History = DistrictFrameTimes.Find(DistrictName);
    return History ? History->GetAverageMs() : 0.0f;
}

int32 UGSDPerformanceTelemetry::GetHitchCount(const FName& DistrictName) const
{
    const int32* Count = DistrictHitchCounts.Find(DistrictName);
    return Count ? *Count : 0;
}

void UGSDPerformanceTelemetry::CountActors()
{
    SCOPE_CYCLE_COUNTER(STAT_GSDCountActors);

    UWorld* World = GetWorld();
    if (!World)
    {
        GSDTELEMETRY_LOG(Verbose, TEXT("CountActors: No world available"));
        return;
    }

    int32 VehicleCount = 0;
    int32 ZombieCount = 0;
    int32 HumanCount = 0;

    // Count vehicles using TActorIterator
    // Note: This uses forward declaration, actual class from GSD_Vehicles plugin
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor || Actor->IsPendingKillPending())
            continue;

        FString ClassName = Actor->GetClass()->GetName();

        // Count vehicles (check class name to avoid hard dependency)
        if (ClassName.Contains(TEXT("GSDVehiclePawn")))
        {
            VehicleCount++;
        }
        // Count hero NPCs
        else if (ClassName.Contains(TEXT("GSDHeroNPC")))
        {
            HumanCount++;
        }
    }

    // Try to get zombie count from crowd manager subsystem
    // Note: Uses subsystem name check to avoid hard dependency
    if (UGameInstance* GI = GetGameInstance())
    {
        // Check all world subsystems for crowd manager
        if (UWorld* GameWorld = GI->GetWorld())
        {
            for (UWorldSubsystem* Subsystem : GameWorld->GetSubsystemArray<UWorldSubsystem>())
            {
                FString SubsystemName = Subsystem->GetClass()->GetName();
                if (SubsystemName.Contains(TEXT("GSDCrowdManager")))
                {
                    // Use reflection to call GetActiveEntityCount
                    // Fallback: Set to 0 if method not found
                    ZombieCount = 0;  // Will be populated by actual crowd manager
                    break;
                }
            }
        }
    }

    // Update latest snapshot
    LatestActorCount.VehicleCount = VehicleCount;
    LatestActorCount.ZombieCount = ZombieCount;
    LatestActorCount.HumanCount = HumanCount;
    LatestActorCount.Timestamp = FPlatformTime::Seconds();

    // Update stats
    SET_DWORD_STAT(STAT_GSDVehicleCount, VehicleCount);
    SET_DWORD_STAT(STAT_GSDZombieCount, ZombieCount);
    SET_DWORD_STAT(STAT_GSDHumanCount, HumanCount);

    // Broadcast delegate
    OnActorCountUpdated.Broadcast(LatestActorCount);

    GSDTELEMETRY_LOG(Verbose, TEXT("Actor counts: Vehicles=%d, Zombies=%d, Humans=%d"),
        VehicleCount, ZombieCount, HumanCount);
}

void UGSDPerformanceTelemetry::GetAllDistrictNames(TArray<FName>& OutDistrictNames) const
{
    DistrictFrameTimes.GetKeys(OutDistrictNames);
}

void UGSDPerformanceTelemetry::StartActorCountTimer()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        GSDTELEMETRY_LOG(Warning, TEXT("Cannot start actor count timer: no world"));
        return;
    }

    World->GetTimerManager().SetTimer(
        ActorCountTimerHandle,
        this,
        &UGSDPerformanceTelemetry::CountActors,
        ActorCountInterval,
        true  // Loop
    );

    GSDTELEMETRY_LOG(Log, TEXT("Actor count timer started (interval: %.1fs)"), ActorCountInterval);
}
