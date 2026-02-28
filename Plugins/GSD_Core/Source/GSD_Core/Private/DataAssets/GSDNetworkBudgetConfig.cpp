// Copyright Bret Bouchard. All Rights Reserved.

#include "DataAssets/GSDNetworkBudgetConfig.h"

FGSDLODReplicationConfig UGSDNetworkBudgetConfig::DefaultLODConfig;

void UGSDNetworkBudgetConfig::PostLoad()
{
    Super::PostLoad();

    if (CategoryAllocations.IsEmpty())
    {
        CategoryAllocations.Add(EGSDBudgetCategory::Crowd, 0.30f);    // 30%
        CategoryAllocations.Add(EGSDBudgetCategory::Vehicle, 0.20f);  // 20%
        CategoryAllocations.Add(EGSDBudgetCategory::Event, 0.10f);    // 10%
        CategoryAllocations.Add(EGSDBudgetCategory::Player, 0.35f);   // 35%
        CategoryAllocations.Add(EGSDBudgetCategory::Other, 0.05f);    // 5%
    }

    if (LODConfigs.IsEmpty())
    {
        // LOD 0 - Close, high frequency
        FGSDLODReplicationConfig LOD0;
        LOD0.UpdateFrequency = 60.0f;
        LOD0.MaxEntitiesPerFrame = 100;
        LOD0.bUseDeltaCompression = false;
        LODConfigs.Add(LOD0);

        // LOD 1 - Medium
        FGSDLODReplicationConfig LOD1;
        LOD1.UpdateFrequency = 30.0f;
        LOD1.MaxEntitiesPerFrame = 50;
        LOD1.bUseDeltaCompression = true;
        LODConfigs.Add(LOD1);

        // LOD 2 - Far
        FGSDLODReplicationConfig LOD2;
        LOD2.UpdateFrequency = 10.0f;
        LOD2.MaxEntitiesPerFrame = 20;
        LOD2.bUseDeltaCompression = true;
        LODConfigs.Add(LOD2);

        // LOD 3 - Very far
        FGSDLODReplicationConfig LOD3;
        LOD3.UpdateFrequency = 2.0f;
        LOD3.MaxEntitiesPerFrame = 10;
        LOD3.bUseDeltaCompression = true;
        LODConfigs.Add(LOD3);
    }
}

int32 UGSDNetworkBudgetConfig::GetCategoryBudget(EGSDBudgetCategory Category) const
{
    if (const float* Allocation = CategoryAllocations.Find(Category))
    {
        return FMath::RoundToInt(static_cast<float>(TotalBitsPerSecond) * (*Allocation));
    }
    return 0;
}

const FGSDLODReplicationConfig& UGSDNetworkBudgetConfig::GetLODConfig(int32 LODLevel) const
{
    if (LODConfigs.IsValidIndex(LODLevel))
    {
        return LODConfigs[LODLevel];
    }
    return DefaultLODConfig;
}
