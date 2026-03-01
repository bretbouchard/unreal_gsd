// Copyright Bret Bouchard. All Rights Reserved.

#include "DataAssets/GSDZombieBehaviorConfig.h"

UGSDZombieBehaviorConfig* UGSDZombieBehaviorConfig::GetDefaultConfig()
{
    static const FString DefaultConfigPath = TEXT("/GSD_Crowds/Config/DA_GSDZombieBehaviorConfig.DA_GSDZombieBehaviorConfig");

    UGSDZombieBehaviorConfig* Config = LoadObject<UGSDZombieBehaviorConfig>(nullptr, *DefaultConfigPath);
    return Config;
}
