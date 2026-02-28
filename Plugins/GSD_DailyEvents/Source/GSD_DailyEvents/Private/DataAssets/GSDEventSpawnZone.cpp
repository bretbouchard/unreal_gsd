// Copyright Bret Bouchard. All Rights Reserved.

#include "DataAssets/GSDEventSpawnZone.h"
#include "NavigationSystem.h"
#include "GSDEventLog.h"

bool UGSDEventSpawnZone::ValidateConfig(FString& OutError) const
{
    if (!Super::ValidateConfig(OutError))
    {
        return false;
    }

    if (ZoneName.IsNone())
    {
        OutError = TEXT("Spawn zone requires a valid ZoneName");
        return false;
    }

    if (ZoneExtent.X <= 0.0f || ZoneExtent.Y <= 0.0f || ZoneExtent.Z <= 0.0f)
    {
        OutError = FString::Printf(TEXT("Spawn zone '%s' has invalid extent: %s"),
            *ZoneName.ToString(), *ZoneExtent.ToString());
        return false;
    }

    if (Priority < 0)
    {
        OutError = FString::Printf(TEXT("Spawn zone '%s' has negative priority: %d"),
            *ZoneName.ToString(), Priority);
        return false;
    }

    return true;
}

bool UGSDEventSpawnZone::IsZoneEnabled() const
{
    // Zone is enabled if it has valid bounds and can be used for spawning
    return ZoneExtent.X > 0.0f && ZoneExtent.Y > 0.0f && ZoneExtent.Z > 0.0f;
}

bool UGSDEventSpawnZone::SupportsEventTag(const FGameplayTag& EventTag) const
{
    // Empty container = supports all events
    if (CompatibleEventTags.IsEmpty())
    {
        return true;
    }

    return CompatibleEventTags.HasTag(EventTag);
}

FVector UGSDEventSpawnZone::GetRandomPointInZone(FRandomStream& Stream) const
{
    // Generate random offset within extent
    FVector RandomOffset(
        Stream.FRandRange(-ZoneExtent.X, ZoneExtent.X),
        Stream.FRandRange(-ZoneExtent.Y, ZoneExtent.Y),
        Stream.FRandRange(-ZoneExtent.Z * 0.5f, ZoneExtent.Z * 0.5f)  // Bias toward ground
    );

    return ZoneCenter + RandomOffset;
}

FBox UGSDEventSpawnZone::GetZoneBounds() const
{
    return FBox::BuildAABB(ZoneCenter, ZoneExtent);
}

FVector UGSDEventSpawnZone::ProjectToNavMesh(UWorld* World, const FVector& Point) const
{
    if (!bRequireNavMesh || !World)
    {
        return Point;
    }

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!NavSys)
    {
        GSDEVENT_LOG(Warning, TEXT("SpawnZone '%s': NavMesh required but NavigationSystem not found"),
            *ZoneName.ToString());
        return Point;
    }

    FNavLocation ProjectedLocation;
    const FVector QueryExtent(NavMeshQueryExtent);

    if (NavSys->ProjectPointToNavigation(Point, ProjectedLocation, QueryExtent))
    {
        return ProjectedLocation.Location;
    }

    GSDEVENT_LOG(Warning, TEXT("SpawnZone '%s': Failed to project point %s to NavMesh"),
        *ZoneName.ToString(), *Point.ToString());
    return Point;
}
