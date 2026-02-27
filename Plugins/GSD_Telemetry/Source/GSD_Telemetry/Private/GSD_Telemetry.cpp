// Copyright Bret Bouchard. All Rights Reserved.

#include "GSD_Telemetry.h"
#include "GSDTelemetryLog.h"

#define LOCTEXT_NAMESPACE "FGSD_TelemetryModule"

void FGSD_TelemetryModule::StartupModule()
{
    bInitialized = true;
    GSDTELEMETRY_LOG(Log, TEXT("GSD_Telemetry module starting up..."));
}

void FGSD_TelemetryModule::ShutdownModule()
{
    if (bInitialized)
    {
        GSDTELEMETRY_LOG(Log, TEXT("GSD_Telemetry module shutting down..."));
        bInitialized = false;
    }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGSD_TelemetryModule, GSD_Telemetry)
