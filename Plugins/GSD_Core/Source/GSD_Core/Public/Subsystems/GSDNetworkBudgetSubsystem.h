// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "DataAssets/GSDNetworkBudgetConfig.h"
#include "GSDNetworkBudgetSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGSDNetworkBudget, Log, All);

USTRUCT(BlueprintType)
struct FGSDReplicationRecord
{
    GENERATED_BODY()

    UPROPERTY()
    EGSDBudgetCategory Category = EGSDBudgetCategory::Other;

    UPROPERTY()
    int32 BitsReplicated = 0;

    UPROPERTY()
    float Timestamp = 0.0f;
};

/**
 * Network bandwidth budget tracking subsystem.
 * Monitors replication and enforces budget limits.
 */
UCLASS()
class GSD_CORE_API UGSDNetworkBudgetSubsystem : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Track replication for a category
    void TrackReplication(EGSDBudgetCategory Category, int32 Bits);

    // Check if replication is allowed this frame for a category/LOD
    bool CanReplicateThisFrame(EGSDBudgetCategory Category, int32 LODLevel = 0);

    // Get current bandwidth usage for a category
    float GetCurrentBandwidthUsage(EGSDBudgetCategory Category) const;

    // Get total bandwidth usage
    float GetTotalBandwidthUsage() const;

    // Get remaining budget for a category
    int32 GetRemainingBudget(EGSDBudgetCategory Category) const;

    // Set configuration
    void SetConfig(UGSDNetworkBudgetConfig* InConfig);

    // Reset frame counters (call at end of frame)
    void ResetFrameCounters();

    // Log current status
    void LogStatus() const;

protected:
    // Configuration
    UPROPERTY()
    TObjectPtr<UGSDNetworkBudgetConfig> Config = nullptr;

    // Current frame usage per category
    TMap<EGSDBudgetCategory, int32> FrameUsage;

    // Rolling average usage (1 second window)
    TMap<EGSDBudgetCategory, TArray<int32>> UsageHistory;

    // Entity counts per LOD
    TArray<int32> LODReplicationCounts;

    // Last warning time
    float LastWarningTime = 0.0f;

    // Update interval for history
    static constexpr float HistoryInterval = 1.0f;
    static constexpr int32 HistorySize = 60;  // 60 seconds of history
};
