// Copyright Bret Bouchard. All Rights Reserved.

#include "GSD_ValidationTools.h"
#include "GSDValidationLog.h"

DEFINE_LOG_CATEGORY(LogGSDValidation);

#define LOCTEXT_NAMESPACE "FGSD_ValidationToolsModule"

void FGSD_ValidationToolsModule::StartupModule()
{
    GSDVALIDATION_LOG(Log, TEXT("GSD_ValidationTools module starting up..."));
}

void FGSD_ValidationToolsModule::ShutdownModule()
{
    GSDVALIDATION_LOG(Log, TEXT("GSD_ValidationTools module shutting down..."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGSD_ValidationToolsModule, GSD_ValidationTools)
