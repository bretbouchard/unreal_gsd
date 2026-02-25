// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * GSD_Vehicles Module Interface
 *
 * Provides Chaos Vehicle physics integration for the GSD Platform.
 * This plugin depends on GSD_Core for shared logging infrastructure
 * and GSD_CityStreaming for streaming source integration.
 */
class FGSD_VehiclesModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
