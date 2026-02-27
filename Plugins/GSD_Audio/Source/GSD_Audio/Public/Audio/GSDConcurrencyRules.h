// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundConcurrency.h"
#include "GSDConcurrencyRules.generated.h"

/**
 * Audio concurrency rule names for GSD Platform.
 * These prevent too many sounds of the same type playing simultaneously.
 */
namespace GSDConcurrencyRules
{
    // Crowd sounds - limit concurrent zombie moans
    static const FName CC_ZombieMoan = FName(TEXT("CC_ZombieMoan"));
    static const FName CC_CrowdAmbient = FName(TEXT("CC_CrowdAmbient"));

    // Vehicle sounds - limit engine sounds
    static const FName CC_VehicleEngine = FName(TEXT("CC_VehicleEngine"));
    static const FName CC_VehicleCollision = FName(TEXT("CC_VehicleCollision"));

    // Event sounds - limit repeating event audio
    static const FName CC_EventAmbient = FName(TEXT("CC_EventAmbient"));
    static const FName CC_Construction = FName(TEXT("CC_Construction"));
    static const FName CC_Party = FName(TEXT("CC_Party"));

    // UI sounds - prevent UI spam
    static const FName CC_UI = FName(TEXT("CC_UI"));

    // Voice - prevent voice overlap
    static const FName CC_Voice = FName(TEXT("CC_Voice"));
}

/**
 * Concurrency rule configuration.
 */
namespace GSDConcurrencyConfig
{
    // Maximum active sounds for each concurrency rule
    struct FConcurrencySettings
    {
        int32 MaxActiveSounds;
        ESoundConcurrencyResolution ResolutionRule;
    };

    static const FConcurrencySettings ZombieMoan = {
        8,  // Max 8 zombie moans at once
        ESoundConcurrencyResolution::StopQuietest
    };

    static const FConcurrencySettings CrowdAmbient = {
        4,  // Max 4 crowd ambient sounds
        ESoundConcurrencyResolution::StopQuietest
    };

    static const FConcurrencySettings VehicleEngine = {
        10,  // Max 10 vehicle engines (one per nearby vehicle)
        ESoundConcurrencyResolution::StopQuietest
    };

    static const FConcurrencySettings VehicleCollision = {
        5,  // Max 5 collision sounds
        ESoundConcurrencyResolution::StopQuietest
    };

    static const FConcurrencySettings EventAmbient = {
        3,  // Max 3 event ambient sounds
        ESoundConcurrencyResolution::StopQuietest
    };

    static const FConcurrencySettings UI = {
        2,  // Max 2 UI sounds at once
        ESoundConcurrencyResolution::StopQuietest
    };

    static const FConcurrencySettings Voice = {
        1,  // Only 1 voice at a time
        ESoundConcurrencyResolution::StopQuietest
    };
}
