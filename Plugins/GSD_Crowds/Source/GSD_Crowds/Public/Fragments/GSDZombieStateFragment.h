// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "GSDZombieStateFragment.generated.h"

/**
 * Fragment containing entity state for crowd/flock members.
 *
 * This is a GAME-AGNOSTIC state fragment suitable for any crowd simulation:
 * - Zombies, NPCs, animals, vehicles, or any Mass Entity crowd
 * - Properties are generic (Health, MovementSpeed, etc.)
 * - Can be extended via additional fragments for game-specific behavior
 *
 * MIGRATION NOTE (GSDCROWDS-105):
 * The "Zombie" naming is deprecated. This fragment will be renamed to
 * FGSDEntityStateFragment in a future phase. Game code using
 * FGSDZombieStateFragment will need to update to the new name.
 * A typedef will be provided for backward compatibility during transition.
 *
 * CRITICAL: Do NOT store UObject pointers in fragments.
 * Fragments are not UObjects and cannot hold strong references.
 * Use indices or raw data instead.
 */
USTRUCT()
struct GSD_CROWDS_API FGSDZombieStateFragment : public FMassFragment
{
    GENERATED_BODY()

    //-- State --
    UPROPERTY()
    float Health = 100.0f;

    UPROPERTY()
    float MovementSpeed = 150.0f;

    UPROPERTY()
    float TargetMovementSpeed = 150.0f;

    //-- Flags --
    UPROPERTY()
    uint8 bIsAggressive : 1;

    UPROPERTY()
    uint8 bIsAlive : 1;

    UPROPERTY()
    uint8 bIsActive : 1;

    //-- Behavior --
    UPROPERTY()
    float WanderDirection = 0.0f;

    UPROPERTY()
    float TimeSinceLastBehaviorUpdate = 0.0f;

    //-- Constructor --
    FGSDZombieStateFragment()
        : bIsAggressive(false)
        , bIsAlive(true)
        , bIsActive(true)
    {
    }
};

//-- Backward Compatibility Typedef (GSDCROWDS-105) --
//-- TODO(GSDCROWDS-105): Remove in future phase after migration complete --
/**
 * Deprecated: Use FGSDZombieStateFragment for now.
 * Will be replaced with FGSDEntityStateFragment in future phase.
 *
 * This typedef exists to document the migration path.
 * Game code should plan to update when the new name is available.
 */
// typedef FGSDZombieStateFragment FGSDEntityStateFragment;  // Uncomment when ready
