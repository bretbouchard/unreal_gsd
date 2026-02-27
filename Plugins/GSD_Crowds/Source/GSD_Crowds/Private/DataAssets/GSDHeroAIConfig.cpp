// Copyright Bret Bouchard. All Rights Reserved.

#include "DataAssets/GSDHeroAIConfig.h"

UGSDHeroAIConfig* UGSDHeroAIConfig::GetDefaultConfig()
{
    static UGSDHeroAIConfig* DefaultConfig = nullptr;

    if (!DefaultConfig)
    {
        DefaultConfig = LoadObject<UGSDHeroAIConfig>(
            nullptr,
            TEXT("/GSD_Crowds/Config/DefaultHeroAIConfig.DefaultHeroAIConfig")
        );
    }

    return DefaultConfig;
}
