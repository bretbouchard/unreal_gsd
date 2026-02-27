// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * GSD_Telemetry Module Interface
 *
 * Provides performance monitoring, metrics collection, and observability
 * for the GSD Platform. Tracks frame times, hitches, actor counts, and
 * streaming cell load times.
 * This plugin depends on GSD_Core for shared infrastructure.
 */
class GSD_TELEMETRY_API FGSD_TelemetryModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    /** Track module initialization state */
    bool bInitialized = false;
};
