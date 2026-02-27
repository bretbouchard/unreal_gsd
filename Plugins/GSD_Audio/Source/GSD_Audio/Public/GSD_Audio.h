// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * GSD_Audio Module Interface
 *
 * Provides audio infrastructure for the GSD Platform including Sound Classes,
 * Submixes, Concurrency Rules, and audio utilities.
 * This plugin depends on GSD_Core for shared logging infrastructure
 * and AudioEditor for sound asset management.
 */
class FGSD_AudioModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
