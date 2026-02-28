// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * GSD_Crowds Module Interface
 *
 * Provides crowd simulation using UE5's Mass Entity framework.
 * Supports 200+ entities with LOD-based visual representation switching.
 * This plugin depends on GSD_Core for shared logging infrastructure
 * and GSD_CityStreaming for World Partition integration.
 */
class GSD_CROWDS_API FGSD_CrowdsModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    /** Track module initialization state */
    bool bInitialized = false;
};
