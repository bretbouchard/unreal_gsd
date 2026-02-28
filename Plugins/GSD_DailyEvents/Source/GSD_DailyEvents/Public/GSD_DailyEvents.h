// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * GSD_DailyEvents Module Interface
 *
 * Provides deterministic daily event scheduling system.
 * Events are scheduled using GameplayTags and generated deterministically
 * from date + seed inputs via GSDDeterminismManager.
 *
 * This plugin depends on:
 * - GSD_Core: UGSDDataAsset, UGSDDeterminismManager, logging infrastructure
 * - GameplayTags: FGameplayTag for event categorization
 * - GSD_Crowds: Density modifier integration for event effects
 */
class GSD_DAILYEVENTS_API FGSD_DailyEventsModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    /** Track module initialization state */
    bool bInitialized = false;
};
