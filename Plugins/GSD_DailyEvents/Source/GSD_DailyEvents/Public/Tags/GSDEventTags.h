// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * GameplayTag constants for GSD Event System.
 * Use these instead of string literals to avoid typos.
 *
 * Tags are defined in Config/DefaultGameplayTags.ini
 *
 * Usage:
 *   FGameplayTag Tag = GSDEventTags::Daily::Construction;
 *   if (Tag.MatchesTag(GSDEventTags::Daily::Base)) { ... }
 */
namespace GSDEventTags
{
    //== Daily Events ==
    namespace Daily
    {
        /** Base tag for all daily events - Event.Daily */
        GSD_DAILYEVENTS_API extern const FGameplayTag Base;

        /** Lane closure event with barricades - Event.Daily.Construction */
        GSD_DAILYEVENTS_API extern const FGameplayTag Construction;

        /** Local density modifier with FX - Event.Daily.Bonfire */
        GSD_DAILYEVENTS_API extern const FGameplayTag Bonfire;

        /** Safe zone with crowd props - Event.Daily.BlockParty */
        GSD_DAILYEVENTS_API extern const FGameplayTag BlockParty;

        /** Density boost with attractor volumes and audio - Event.Daily.ZombieRave */
        GSD_DAILYEVENTS_API extern const FGameplayTag ZombieRave;
    }

    //== Event Modifiers ==
    namespace Modifier
    {
        /** Base tag for event modifiers - Event.Modifier */
        GSD_DAILYEVENTS_API extern const FGameplayTag Base;

        /** Increases spawn density in area - Event.Modifier.DensityBoost */
        GSD_DAILYEVENTS_API extern const FGameplayTag DensityBoost;

        /** Decreases spawn density in area - Event.Modifier.DensityReduce */
        GSD_DAILYEVENTS_API extern const FGameplayTag DensityReduce;

        /** Closes navigation lanes - Event.Modifier.NavigationBlock */
        GSD_DAILYEVENTS_API extern const FGameplayTag NavigationBlock;

        /** Disables hostile spawns in area - Event.Modifier.SafeZone */
        GSD_DAILYEVENTS_API extern const FGameplayTag SafeZone;
    }

    //== Event States ==
    namespace State
    {
        /** Base tag for event states - Event.State */
        GSD_DAILYEVENTS_API extern const FGameplayTag Base;

        /** Event is currently running - Event.State.Active */
        GSD_DAILYEVENTS_API extern const FGameplayTag Active;

        /** Event scheduled but not started - Event.State.Pending */
        GSD_DAILYEVENTS_API extern const FGameplayTag Pending;

        /** Event has finished - Event.State.Complete */
        GSD_DAILYEVENTS_API extern const FGameplayTag Complete;
    }

    /**
     * Initialize all tag constants.
     * Called from module startup to cache tag references.
     */
    GSD_DAILYEVENTS_API void InitializeTags();

    /**
     * Check if a tag is a valid event tag.
     * Returns true if the tag matches Event.Daily or its children.
     */
    GSD_DAILYEVENTS_API bool IsEventTag(const FGameplayTag& Tag);
}
