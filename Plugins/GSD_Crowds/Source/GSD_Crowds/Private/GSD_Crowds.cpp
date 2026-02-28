// Copyright Bret Bouchard. All Rights Reserved.

#include "GSD_Crowds.h"
#include "GSDCrowdLog.h"

#define LOCTEXT_NAMESPACE "FGSD_CrowdsModule"

void FGSD_CrowdsModule::StartupModule()
{
    UE_LOG(LOG_GSDCROWDS, Log, TEXT("GSD_Crowds module started"));
}

void FGSD_CrowdsModule::ShutdownModule()
{
    UE_LOG(LOG_GSDCROWDS, Log, TEXT("GSD_Crowds module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGSD_CrowdsModule, GSD_Crowds)
