// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GSDDataAsset.h"
#include "GSDEventSpawnZone.generated.h"

/**
 * Defines a valid spawn zone for daily events.
 * Spawn zones are authored in data assets and queried by the scheduler
 * for deterministic event placement.
 *
 * Usage:
 * 1. Create blueprint extending this class
 * 2. Set ZoneBounds (world-space AABB)
 * 3. Assign EventTypes that can spawn in this zone
 * 4. Set NavMesh requirements if needed
 *
 * Determinism: Zones are sorted by ZoneName for consistent selection order.
 */
UCLASS(BlueprintType, Category = "GSD|Events")
class GSD_DAILYEVENTS_API UGSDEventSpawnZone : public UGSDDataAsset
{
    GENERATED_BODY()

public:
    //-- Zone Identity --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zone")
    FName ZoneName;

    //-- Spatial Bounds --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spatial")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spatial")
    FVector ZoneExtent = FVector(5000.0f, 5000.0f, 500.0f);

    //-- Event Filtering --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Filtering")
    FGameplayTagContainer CompatibleEventTags;

    //-- Priority --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Priority", meta = (ClampMin = "0"))
    int32 Priority = 0;

    //-- Navigation Requirements --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
    bool bRequireNavMesh = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
    float NavMeshQueryExtent = 500.0f;

    //-- Validation --
    virtual bool ValidateConfig(FString& OutError) const override;

    //-- Helpers --

    /**
     * Check if this zone supports a given event tag.
     * Returns true if CompatibleEventTags contains the tag or is empty (any event).
     */
    bool SupportsEventTag(const FGameplayTag& EventTag) const;

    /**
     * Get a random point within this zone using the provided RNG stream.
     * Point is clamped to zone bounds.
     */
    FVector GetRandomPointInZone(FRandomStream& Stream) const;

    /**
     * Get the axis-aligned bounding box for this zone.
     */
    FBox GetZoneBounds() const;

    /**
     * Project a point to navmesh if navigation is required.
     * Returns the projected location, or original if no projection needed/possible.
     */
    FVector ProjectToNavMesh(UWorld* World, const FVector& Point) const;

    /**
     * Check if zone is enabled and valid for spawning.
     */
    bool IsZoneEnabled() const;
};
