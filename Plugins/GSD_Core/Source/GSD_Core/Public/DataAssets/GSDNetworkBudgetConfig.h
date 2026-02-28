// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GSDNetworkBudgetConfig.generated.h"

UENUM(BlueprintType)
enum class EGSDBudgetCategory : uint8
{
    Crowd,
    Vehicle,
    Event,
    Player,
    Other
};

USTRUCT(BlueprintType)
struct FGSDLODReplicationConfig
{
    GENERATED_BODY()

    // Update frequency for this LOD level (Hz)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float UpdateFrequency = 30.0f;

    // Maximum entities to update per frame at this LOD
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 MaxEntitiesPerFrame = 50;

    // Whether to use delta compression
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    bool bUseDeltaCompression = true;
};

/**
 * Network bandwidth budget configuration.
 * Controls how much bandwidth each system can use.
 */
UCLASS(BlueprintType)
class GSD_CORE_API UGSDNetworkBudgetConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    // Total bandwidth budget (bits per second)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Budget")
    int32 TotalBitsPerSecond = 100000;  // 100 kbps

    // Per-category bandwidth allocation (percentage of total)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Budget")
    TMap<EGSDBudgetCategory, float> CategoryAllocations;

    // LOD-based replication configs
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LOD")
    TArray<FGSDLODReplicationConfig> LODConfigs;

    // Warning threshold (percentage)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monitoring")
    float WarningThreshold = 0.8f;

    // Critical threshold (percentage)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monitoring")
    float CriticalThreshold = 0.95f;

    // Whether to log bandwidth warnings
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monitoring")
    bool bLogBandwidthWarnings = true;

    // Get allocated bandwidth for a category
    int32 GetCategoryBudget(EGSDBudgetCategory Category) const;

    // Get LOD config
    const FGSDLODReplicationConfig& GetLODConfig(int32 LODLevel) const;

protected:
    virtual void PostLoad() override;

private:
    // Default LOD config for fallback
    static FGSDLODReplicationConfig DefaultLODConfig;
};
