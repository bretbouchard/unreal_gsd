// Copyright Bret Bouchard. All Rights Reserved.

#include "GSD_Vehicles.h"
#include "GSDVehicleLog.h"

#define LOCTEXT_NAMESPACE "FGSD_VehiclesModule"

void FGSD_VehiclesModule::StartupModule()
{
    UE_LOG(LOG_GSDVEHICLES, Log, TEXT("GSD_Vehicles module started"));
}

void FGSD_VehiclesModule::ShutdownModule()
{
    UE_LOG(LOG_GSDVEHICLES, Log, TEXT("GSD_Vehicles module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGSD_VehiclesModule, GSD_Vehicles)
