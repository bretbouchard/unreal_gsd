#include "DataAssets/GSDCrowdHLODConfig.h"

bool UGSDCrowdHLODConfig::ValidateConfig(FString& OutError) const
{
    if (HLODStartDistance <= 0.0f)
    {
        OutError = TEXT("HLODStartDistance must be positive");
        return false;
    }

    if (HLODOnlyDistance <= HLODStartDistance)
    {
        OutError = TEXT("HLODOnlyDistance must be greater than HLODStartDistance");
        return false;
    }

    if (ClusterSize <= 0.0f)
    {
        OutError = TEXT("ClusterSize must be positive");
        return false;
    }

    if (MaxProxies <= 0)
    {
        OutError = TEXT("MaxProxies must be positive");
        return false;
    }

    if (MinEntitiesPerCluster <= 0)
    {
        OutError = TEXT("MinEntitiesPerCluster must be positive");
        return false;
    }

    if (bUseFadeTransitions && FadeDuration <= 0.0f)
    {
        OutError = TEXT("FadeDuration must be positive when using fade transitions");
        return false;
    }

    return true;
}
