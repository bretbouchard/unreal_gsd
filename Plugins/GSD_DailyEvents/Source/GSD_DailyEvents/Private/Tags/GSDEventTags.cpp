// Copyright Bret Bouchard. All Rights Reserved.

#include "Tags/GSDEventTags.h"

namespace GSDEventTags
{
    namespace Daily
    {
        const FGameplayTag Base = FGameplayTag::RequestGameplayTag(FName("Event.Daily"));
        const FGameplayTag Construction = FGameplayTag::RequestGameplayTag(FName("Event.Daily.Construction"));
        const FGameplayTag Bonfire = FGameplayTag::RequestGameplayTag(FName("Event.Daily.Bonfire"));
        const FGameplayTag BlockParty = FGameplayTag::RequestGameplayTag(FName("Event.Daily.BlockParty"));
        const FGameplayTag ZombieRave = FGameplayTag::RequestGameplayTag(FName("Event.Daily.ZombieRave"));
    }

    namespace Modifier
    {
        const FGameplayTag Base = FGameplayTag::RequestGameplayTag(FName("Event.Modifier"));
        const FGameplayTag DensityBoost = FGameplayTag::RequestGameplayTag(FName("Event.Modifier.DensityBoost"));
        const FGameplayTag DensityReduce = FGameplayTag::RequestGameplayTag(FName("Event.Modifier.DensityReduce"));
        const FGameplayTag NavigationBlock = FGameplayTag::RequestGameplayTag(FName("Event.Modifier.NavigationBlock"));
        const FGameplayTag SafeZone = FGameplayTag::RequestGameplayTag(FName("Event.Modifier.SafeZone"));
    }

    namespace State
    {
        const FGameplayTag Base = FGameplayTag::RequestGameplayTag(FName("Event.State"));
        const FGameplayTag Active = FGameplayTag::RequestGameplayTag(FName("Event.State.Active"));
        const FGameplayTag Pending = FGameplayTag::RequestGameplayTag(FName("Event.State.Pending"));
        const FGameplayTag Complete = FGameplayTag::RequestGameplayTag(FName("Event.State.Complete"));
    }

    void InitializeTags()
    {
        // Tags are initialized statically via RequestGameplayTag
        // This function can be used for validation logging at startup
        // All tags should be valid if DefaultGameplayTags.ini is loaded
    }

    bool IsEventTag(const FGameplayTag& Tag)
    {
        return Tag.IsValid() && Tag.MatchesTag(Daily::Base);
    }
}
