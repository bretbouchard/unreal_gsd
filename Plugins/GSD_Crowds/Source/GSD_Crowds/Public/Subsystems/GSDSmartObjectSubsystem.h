// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SmartObjectsModule/SmartObjectTypes.h"
#include "GSDSmartObjectSubsystem.generated.h"

class USmartObjectSubsystem;

/**
 * World subsystem for Smart Object queries and claiming.
 * Wraps USmartObjectSubsystem for GSD-specific usage.
 *
 * Provides centralized Smart Object access for crowd entities:
 * - Find nearby Smart Objects with optional tag filtering
 * - Claim Smart Objects (thread-safe)
 * - Release claimed Smart Objects
 * - Query availability and locations
 *
 * Usage:
 * 1. Get subsystem: GetWorld()->GetSubsystem<UGSDSmartObjectSubsystem>()
 * 2. Find objects: Subsystem->FindNearbySmartObjects(Location, Radius)
 * 3. Claim object: Subsystem->ClaimSmartObject(Handle)
 * 4. Release object: Subsystem->ReleaseSmartObject(ClaimHandle)
 */
UCLASS()
class GSD_CROWDS_API UGSDSmartObjectSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // ~UWorldSubsystem interface
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    // ~End of UWorldSubsystem interface

    //-- Query Operations --

    /**
     * Find Smart Objects near a location.
     *
     * @param Location World location to search around
     * @param Radius Search radius in centimeters
     * @param FilterTags Optional gameplay tag filter
     * @return Array of Smart Object handles
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|SmartObjects")
    TArray<FSmartObjectHandle> FindNearbySmartObjects(
        const FVector& Location,
        float Radius,
        const FGameplayTagContainer& FilterTags = FGameplayTagContainer()) const;

    /**
     * Find the nearest available Smart Object.
     *
     * @param Location World location to search from
     * @param Radius Search radius in centimeters
     * @return Handle to nearest available Smart Object (invalid if none found)
     */
    FSmartObjectHandle FindNearestAvailableSmartObject(
        const FVector& Location,
        float Radius) const;

    //-- Claim Operations --

    /**
     * Claim a Smart Object (thread-safe).
     *
     * @param Handle Smart Object handle to claim
     * @return Claim handle (invalid if claim failed)
     */
    FSmartObjectClaimHandle ClaimSmartObject(FSmartObjectHandle Handle);

    /**
     * Release a claimed Smart Object.
     * Resets the claim handle to invalid after release.
     *
     * @param Handle Claim handle to release
     */
    void ReleaseSmartObject(FSmartObjectClaimHandle& Handle);

    /**
     * Check if a Smart Object is available for claiming.
     *
     * @param Handle Smart Object handle to check
     * @return True if available, false if claimed or invalid
     */
    bool IsSmartObjectAvailable(FSmartObjectHandle Handle) const;

    //-- Location Operations --

    /**
     * Get world location of a Smart Object.
     *
     * @param Handle Smart Object handle
     * @return World location (zero vector if invalid)
     */
    FVector GetSmartObjectLocation(FSmartObjectHandle Handle) const;

protected:
    //-- Cached Subsystem --
    UPROPERTY()
    TObjectPtr<USmartObjectSubsystem> CachedSmartObjectSubsystem;
};
