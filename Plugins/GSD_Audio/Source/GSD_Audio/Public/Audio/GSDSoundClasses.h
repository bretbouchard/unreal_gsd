// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundClass.h"
#include "GSDSoundClasses.generated.h"

/**
 * Sound Class hierarchy for GSD Platform.
 *
 * Hierarchy:
 *   Master
 *   ├── Music
 *   ├── SFX
 *   │   ├── Voice
 *   │   ├── Crowd
 *   │   └── Ambience
 *   ├── Vehicle
 *   │   ├── VehicleEngine
 *   │   └── VehicleCollision
 *   ├── Event
 *   │   ├── Construction
 *   │   └── Party
 *   └── UI
 */
namespace GSDSoundClasses
{
    // Root sound class
    static const FName Master = FName(TEXT("Master"));

    // Primary categories
    static const FName Music = FName(TEXT("Music"));
    static const FName SFX = FName(TEXT("SFX"));
    static const FName UI = FName(TEXT("UI"));

    // SFX sub-categories
    static const FName Voice = FName(TEXT("Voice"));
    static const FName Crowd = FName(TEXT("Crowd"));
    static const FName Ambience = FName(TEXT("Ambience"));

    // Vehicle-specific
    static const FName Vehicle = FName(TEXT("Vehicle"));
    static const FName VehicleEngine = FName(TEXT("VehicleEngine"));
    static const FName VehicleCollision = FName(TEXT("VehicleCollision"));

    // Event-specific
    static const FName Event = FName(TEXT("Event"));
    static const FName Construction = FName(TEXT("Construction"));
    static const FName Party = FName(TEXT("Party"));
}

/**
 * Sound Class asset paths for loading.
 */
namespace GSDSoundClassPaths
{
    static const FString Master = TEXT("/GSD_Audio/Audio/SoundClasses/Master.Master");
    static const FString Music = TEXT("/GSD_Audio/Audio/SoundClasses/Music.Music");
    static const FString SFX = TEXT("/GSD_Audio/Audio/SoundClasses/SFX.SFX");
    static const FString Voice = TEXT("/GSD_Audio/Audio/SoundClasses/Voice.Voice");
    static const FString Crowd = TEXT("/GSD_Audio/Audio/SoundClasses/Crowd.Crowd");
    static const FString Ambience = TEXT("/GSD_Audio/Audio/SoundClasses/Ambience.Ambience");
}

/**
 * Default volume levels for sound classes.
 */
namespace GSDSoundClassDefaults
{
    static constexpr float MasterVolume = 1.0f;
    static constexpr float MusicVolume = 0.7f;
    static constexpr float SFXVolume = 1.0f;
    static constexpr float VoiceVolume = 1.0f;
    static constexpr float CrowdVolume = 0.8f;
    static constexpr float AmbienceVolume = 0.6f;
}
