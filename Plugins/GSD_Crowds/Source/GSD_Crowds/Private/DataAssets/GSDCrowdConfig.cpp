// Copyright Bret Bouchard. All Rights Reserved.

#include "DataAssets/GSDCrowdConfig.h"
#include "GSDCrowdLog.h"

UGSDCrowdConfig* UGSDCrowdConfig::GetDefaultConfig()
{
    // Load default config from asset path
    static UGSDCrowdConfig* DefaultConfig = nullptr;

    if (!DefaultConfig)
    {
        DefaultConfig = LoadObject<UGSDCrowdConfig>(
            nullptr,
            TEXT("/GSD_Crowds/Config/DefaultCrowdConfig.DefaultCrowdConfig")
        );

        if (!DefaultConfig)
        {
            // Log warning if config not found - this is expected if not yet created
            UE_LOG(LOG_GSDCROWDS, VeryVerbose,
                TEXT("UGSDCrowdConfig::GetDefaultConfig - Default config not found at /GSD_Crowds/Config/DefaultCrowdConfig. "
                     "Using processor defaults or create the asset."));
        }
    }

    return DefaultConfig;
}
