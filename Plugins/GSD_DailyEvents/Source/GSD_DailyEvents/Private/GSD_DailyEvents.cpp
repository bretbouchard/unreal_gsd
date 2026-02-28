// Copyright Bret Bouchard. All Rights Reserved.

#include "GSD_DailyEvents.h"
#include "GSDEventLog.h"

#define LOCTEXT_NAMESPACE "FGSD_DailyEventsModule"

void FGSD_DailyEventsModule::StartupModule()
{
    // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
    GSDEVENT_LOG(Log, TEXT("GSD_DailyEvents module starting up"));
    bInitialized = true;
}

void FGSD_DailyEventsModule::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.
    GSDEVENT_LOG(Log, TEXT("GSD_DailyEvents module shutting down"));
    bInitialized = false;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGSD_DailyEventsModule, GSD_DailyEvents)
