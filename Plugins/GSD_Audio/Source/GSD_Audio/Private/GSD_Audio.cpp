// Copyright Bret Bouchard. All Rights Reserved.

#include "GSD_Audio.h"
#include "GSDAudioLog.h"

#define LOCTEXT_NAMESPACE "FGSD_AudioModule"

void FGSD_AudioModule::StartupModule()
{
    UE_LOG(LogGSDAudio, Log, TEXT("GSD_Audio module started"));
}

void FGSD_AudioModule::ShutdownModule()
{
    UE_LOG(LogGSDAudio, Log, TEXT("GSD_Audio module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGSD_AudioModule, GSD_Audio)
