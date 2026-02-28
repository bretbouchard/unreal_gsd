// Copyright Bret Bouchard. All Rights Reserved.

#include "Subsystems/GSDCrowdPerformanceBudget.h"
#include "GSDCrowdLog.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"

//-- UWorldSubsystem Interface --

bool UGSDCrowdPerformanceBudget::ShouldCreateSubsystem(UWorld* World) const
{
    // Only create for game worlds (not editor preview, etc.)
    return World && (World->IsGameWorld() || World->IsPreviewWorld());
}

void UGSDCrowdPerformanceBudget::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Pre-allocate frame time history
    FrameTimeHistory.Reserve(FrameTimeHistorySize);

    GSD_CROWD_LOG(Log, TEXT("GSDCrowdPerformanceBudget initialized - Quality: %d, MaxEntities: %d, Budget: %.1fms"),
        static_cast<int32>(Config.CurrentQuality),
        Config.GetMaxEntities(),
        Config.GetBudgetMs());
}

void UGSDCrowdPerformanceBudget::Deinitialize()
{
    // Clear all active scopes
    ActiveScopes.Empty();

    // Clear frame time history
    FrameTimeHistory.Empty();

    // Reset counters
    ConsecutiveSlowFrames = 0;
    ConsecutiveFastFrames = 0;
    CurrentEntityCount = 0;
    NextScopeId = 1;

    GSD_CROWD_LOG(Log, TEXT("GSDCrowdPerformanceBudget deinitialized"));
}

//-- Budget Checking --

int32 UGSDCrowdPerformanceBudget::BeginBudgetScope(const FString& Category)
{
    const int32 ScopeId = NextScopeId++;
    const double CurrentTime = FPlatformTime::Seconds();

    ActiveScopes.Add(ScopeId, CurrentTime);

    GSD_CROWD_VERY_TRACE(TEXT("BeginBudgetScope: Category='%s', ScopeId=%d"),
        *Category, ScopeId);

    return ScopeId;
}

FGSDCrowdBudgetResult UGSDCrowdPerformanceBudget::EndBudgetScope(int32 ScopeId)
{
    FGSDCrowdBudgetResult Result;

    const double* StartTimePtr = ActiveScopes.Find(ScopeId);
    if (!StartTimePtr)
    {
        GSD_CROWD_WARN(TEXT("EndBudgetScope: Invalid ScopeId %d (not found)"), ScopeId);
        Result.bWithinBudget = false;
        Result.bShouldEarlyOut = true;
        return Result;
    }

    // Calculate elapsed time
    const double StartTime = *StartTimePtr;
    const double EndTime = FPlatformTime::Seconds();
    const double ElapsedSeconds = EndTime - StartTime;
    const float ElapsedMs = static_cast<float>(ElapsedSeconds * 1000.0);

    // Remove the scope
    ActiveScopes.Remove(ScopeId);

    // Get budget for current quality
    const float BudgetMs = Config.GetBudgetMs();

    // Populate result
    Result.ElapsedTimeMs = ElapsedMs;
    Result.BudgetMs = BudgetMs;
    Result.UtilizationPercent = (ElapsedMs / BudgetMs) * 100.0f;
    Result.bWithinBudget = ElapsedMs <= BudgetMs;

    // Determine if we should early out
    // Early out if:
    // 1. This scope exceeded budget significantly (>150%)
    // 2. Or we're over warning threshold and utilization is high
    const bool bSignificantlyOverBudget = ElapsedMs > (BudgetMs * 1.5f);
    const bool bOverWarningThreshold = ElapsedMs > Config.WarningThresholdMs;

    Result.bShouldEarlyOut = bSignificantlyOverBudget ||
        (bOverWarningThreshold && Result.UtilizationPercent > 100.0f);

    // Log and broadcast if budget exceeded
    if (!Result.bWithinBudget)
    {
        GSD_CROWD_WARN(TEXT("Budget exceeded: %.2fms / %.1fms (%.1f%% utilized)"),
            ElapsedMs, BudgetMs, Result.UtilizationPercent);

        // Broadcast delegate if bound
        if (BudgetExceededDelegate.IsBound())
        {
            BudgetExceededDelegate.Broadcast(ElapsedMs, BudgetMs);
        }
    }
    else
    {
        GSD_CROWD_VERY_TRACE(TEXT("Budget scope complete: %.2fms / %.1fms (%.1f%% utilized)"),
            ElapsedMs, BudgetMs, Result.UtilizationPercent);
    }

    return Result;
}

bool UGSDCrowdPerformanceBudget::ShouldEarlyOut() const
{
    // Check if any active scope is significantly over budget
    const double CurrentTime = FPlatformTime::Seconds();
    const float BudgetMs = Config.GetBudgetMs();
    const float EarlyOutThresholdMs = BudgetMs * 1.5f;

    for (const TPair<int32, double>& Pair : ActiveScopes)
    {
        const double StartTime = Pair.Value;
        const double ElapsedSeconds = CurrentTime - StartTime;
        const float ElapsedMs = static_cast<float>(ElapsedSeconds * 1000.0);

        if (ElapsedMs > EarlyOutThresholdMs)
        {
            return true;
        }
    }

    return false;
}

//-- Entity Limits --

bool UGSDCrowdPerformanceBudget::CanSpawnEntities(int32 AdditionalCount) const
{
    const int32 MaxEntities = Config.GetMaxEntities();
    const int32 ProjectedCount = CurrentEntityCount + AdditionalCount;

    return ProjectedCount <= MaxEntities;
}

//-- Quality Control --

void UGSDCrowdPerformanceBudget::SetQuality(EGSDCrowdQuality NewQuality)
{
    if (Config.CurrentQuality == NewQuality)
    {
        return;
    }

    const EGSDCrowdQuality OldQuality = Config.CurrentQuality;
    Config.CurrentQuality = NewQuality;

    // Reset auto-scaling counters when manually changing quality
    ConsecutiveSlowFrames = 0;
    ConsecutiveFastFrames = 0;

    GSD_CROWD_LOG(Log, TEXT("Quality changed: %d -> %d (MaxEntities: %d, Budget: %.1fms)"),
        static_cast<int32>(OldQuality),
        static_cast<int32>(NewQuality),
        Config.GetMaxEntities(),
        Config.GetBudgetMs());
}

//-- Auto-Quality Scaling --

void UGSDCrowdPerformanceBudget::UpdateAutoQualityScaling()
{
    // Skip if auto-scaling is disabled
    if (!Config.bEnableAutoQualityScaling)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Get frame time from game state or use delta time
    float FrameTimeMs = 16.67f; // Default to 60fps

    if (AGameStateBase* GameState = World->GetGameState())
    {
        // Use server frame time if available
        const float ServerFrameTime = GameState->GetServerWorldTimeSecondsDelta();
        if (ServerFrameTime > 0.0f)
        {
            FrameTimeMs = ServerFrameTime * 1000.0f;
        }
    }

    // Fallback to platform time if frame time not available from game state
    if (FrameTimeMs <= 0.0f || FrameTimeMs > 1000.0f)
    {
        static double LastFrameTime = FPlatformTime::Seconds();
        const double CurrentTime = FPlatformTime::Seconds();
        FrameTimeMs = static_cast<float>((CurrentTime - LastFrameTime) * 1000.0);
        LastFrameTime = CurrentTime;
    }

    // Add to history
    FrameTimeHistory.Add(FrameTimeMs);
    if (FrameTimeHistory.Num() > FrameTimeHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
    }

    // Check for downscale condition (performance is poor)
    if (FrameTimeMs > Config.FrameTimeThresholdForDownscale)
    {
        ConsecutiveSlowFrames++;
        ConsecutiveFastFrames = 0;

        // Downscale if we've had enough slow frames
        if (ConsecutiveSlowFrames >= Config.FramesBeforeDownscale)
        {
            EGSDCrowdQuality NewQuality = Config.CurrentQuality;

            switch (Config.CurrentQuality)
            {
            case EGSDCrowdQuality::Epic:
                NewQuality = EGSDCrowdQuality::High;
                break;
            case EGSDCrowdQuality::High:
                NewQuality = EGSDCrowdQuality::Medium;
                break;
            case EGSDCrowdQuality::Medium:
                NewQuality = EGSDCrowdQuality::Low;
                break;
            case EGSDCrowdQuality::Low:
                // Already at lowest quality
                break;
            }

            if (NewQuality != Config.CurrentQuality)
            {
                Config.CurrentQuality = NewQuality;
                ConsecutiveSlowFrames = 0;

                GSD_CROWD_WARN(TEXT("Auto-downscaled quality to %d due to poor performance (frame time: %.1fms)"),
                    static_cast<int32>(NewQuality), FrameTimeMs);
            }
        }
    }
    // Check for upscale condition (performance is good)
    else if (FrameTimeMs < Config.FrameTimeThresholdForUpscale)
    {
        ConsecutiveFastFrames++;
        ConsecutiveSlowFrames = 0;

        // Upscale if we've had enough fast frames
        if (ConsecutiveFastFrames >= Config.FramesBeforeUpscale)
        {
            EGSDCrowdQuality NewQuality = Config.CurrentQuality;

            switch (Config.CurrentQuality)
            {
            case EGSDCrowdQuality::Low:
                NewQuality = EGSDCrowdQuality::Medium;
                break;
            case EGSDCrowdQuality::Medium:
                NewQuality = EGSDCrowdQuality::High;
                break;
            case EGSDCrowdQuality::High:
                NewQuality = EGSDCrowdQuality::Epic;
                break;
            case EGSDCrowdQuality::Epic:
                // Already at highest quality
                break;
            }

            if (NewQuality != Config.CurrentQuality)
            {
                Config.CurrentQuality = NewQuality;
                ConsecutiveFastFrames = 0;

                GSD_CROWD_LOG(Log, TEXT("Auto-upscaled quality to %d due to good performance (frame time: %.1fms)"),
                    static_cast<int32>(NewQuality), FrameTimeMs);
            }
        }
    }
    else
    {
        // Frame time is in acceptable range, reset counters
        ConsecutiveSlowFrames = FMath::Max(0, ConsecutiveSlowFrames - 1);
        ConsecutiveFastFrames = FMath::Max(0, ConsecutiveFastFrames - 1);
    }
}
