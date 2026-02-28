// Copyright Bret Bouchard. All Rights Reserved.

#include "Subsystems/GSDNetworkBudgetSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY(LogGSDNetworkBudget);

void UGSDNetworkBudgetSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Initialize default config if none set
    if (!Config)
    {
        Config = NewObject<UGSDNetworkBudgetConfig>(this);
        Config->AddToRoot();  // Prevent GC
    }

    UE_LOG(LogGSDNetworkBudget, Log, TEXT("GSDNetworkBudgetSubsystem initialized"));
}

void UGSDNetworkBudgetSubsystem::Deinitialize()
{
    Super::Deinitialize();

    if (Config)
    {
        Config->RemoveFromRoot();
        Config = nullptr;
    }

    FrameUsage.Empty();
    UsageHistory.Empty();
    LODReplicationCounts.Empty();

    UE_LOG(LogGSDNetworkBudget, Log, TEXT("GSDNetworkBudgetSubsystem deinitialized"));
}

void UGSDNetworkBudgetSubsystem::TrackReplication(EGSDBudgetCategory Category, int32 Bits)
{
    if (!Config) return;

    FrameUsage.FindOrAdd(Category) += Bits;

    // Check budget
    int32 Budget = Config->GetCategoryBudget(Category);
    int32 Used = FrameUsage[Category];

    if (Config->bLogBandwidthWarnings && Used > Budget * Config->WarningThreshold)
    {
        float Now = FPlatformTime::Seconds();
        if (Now - LastWarningTime > 5.0f)  // Throttle warnings
        {
            UE_LOG(LogGSDNetworkBudget, Warning,
                TEXT("Category %s at %.1f%% bandwidth budget (%d/%d bits)"),
                *UEnum::GetValueAsString(Category),
                Budget > 0 ? (static_cast<float>(Used) / Budget * 100.0f) : 0.0f,
                Used, Budget);
            LastWarningTime = Now;
        }
    }
}

bool UGSDNetworkBudgetSubsystem::CanReplicateThisFrame(EGSDBudgetCategory Category, int32 LODLevel)
{
    if (!Config) return true;

    // Check budget
    int32 Remaining = GetRemainingBudget(Category);
    if (Remaining <= 0)
    {
        return false;
    }

    // Check LOD limits
    const FGSDLODReplicationConfig& LODConfig = Config->GetLODConfig(LODLevel);
    int32 CurrentCount = LODReplicationCounts.IsValidIndex(LODLevel) ? LODReplicationCounts[LODLevel] : 0;

    if (CurrentCount >= LODConfig.MaxEntitiesPerFrame)
    {
        return false;
    }

    // Increment count
    if (LODReplicationCounts.Num() <= LODLevel)
    {
        LODReplicationCounts.SetNum(LODLevel + 1);
    }
    LODReplicationCounts[LODLevel]++;

    return true;
}

float UGSDNetworkBudgetSubsystem::GetCurrentBandwidthUsage(EGSDBudgetCategory Category) const
{
    return static_cast<float>(FrameUsage.FindRef(Category));
}

float UGSDNetworkBudgetSubsystem::GetTotalBandwidthUsage() const
{
    int32 Total = 0;
    for (const auto& Pair : FrameUsage)
    {
        Total += Pair.Value;
    }
    return static_cast<float>(Total);
}

int32 UGSDNetworkBudgetSubsystem::GetRemainingBudget(EGSDBudgetCategory Category) const
{
    if (!Config) return MAX_int32;

    int32 Budget = Config->GetCategoryBudget(Category);
    int32 Used = FrameUsage.FindRef(Category);
    return FMath::Max(0, Budget - Used);
}

void UGSDNetworkBudgetSubsystem::SetConfig(UGSDNetworkBudgetConfig* InConfig)
{
    if (Config)
    {
        Config->RemoveFromRoot();
    }

    Config = InConfig;

    if (Config)
    {
        Config->AddToRoot();
    }
}

void UGSDNetworkBudgetSubsystem::ResetFrameCounters()
{
    FrameUsage.Empty();
    LODReplicationCounts.Empty();
}

void UGSDNetworkBudgetSubsystem::LogStatus() const
{
    if (!Config)
    {
        UE_LOG(LogGSDNetworkBudget, Warning, TEXT("No config set"));
        return;
    }

    UE_LOG(LogGSDNetworkBudget, Log, TEXT("=== Network Budget Status ==="));
    UE_LOG(LogGSDNetworkBudget, Log, TEXT("Total Budget: %d bits/sec"), Config->TotalBitsPerSecond);

    for (const auto& Pair : Config->CategoryAllocations)
    {
        int32 Budget = Config->GetCategoryBudget(Pair.Key);
        int32 Used = FrameUsage.FindRef(Pair.Key);
        float PercentUsed = Budget > 0 ? (static_cast<float>(Used) / Budget * 100.0f) : 0.0f;

        UE_LOG(LogGSDNetworkBudget, Log, TEXT("  %s: %d/%d bits (%.1f%%)"),
            *UEnum::GetValueAsString(Pair.Key), Used, Budget, PercentUsed);
    }

    UE_LOG(LogGSDNetworkBudget, Log, TEXT("Total Used: %.0f bits"), GetTotalBandwidthUsage());
}
