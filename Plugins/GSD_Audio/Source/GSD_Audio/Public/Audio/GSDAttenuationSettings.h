// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Sound/AttenuationSettings.h"
#include "GSDAttenuationSettings.generated.h"

/**
 * Attenuation distances for different entity types.
 * Based on gameplay requirements and audio design guidelines.
 */
namespace GSDAttenuationDistances
{
    // Crowd entities (zombies, NPCs)
    static constexpr float CrowdInnerRadius = 100.0f;   // Full volume
    static constexpr float CrowdFalloffDistance = 1000.0f;  // Begin falloff
    static constexpr float CrowdCutoffDistance = 5000.0f;  // Silent

    // Vehicles
    static constexpr float VehicleInnerRadius = 200.0f;
    static constexpr float VehicleFalloffDistance = 2000.0f;
    static constexpr float VehicleCutoffDistance = 10000.0f;

    // Voice (hero NPCs, dialogue)
    static constexpr float VoiceInnerRadius = 50.0f;
    static constexpr float VoiceFalloffDistance = 500.0f;
    static constexpr float VoiceCutoffDistance = 2000.0f;

    // Ambient sounds (fires, parties)
    static constexpr float AmbienceInnerRadius = 300.0f;
    static constexpr float AmbienceFalloffDistance = 3000.0f;
    static constexpr float AmbienceCutoffDistance = 10000.0f;

    // UI sounds (no attenuation - 2D)
    static constexpr float UIAttenuation = 0.0f;  // No distance attenuation
}

/**
 * Audio LOD distances for crowd processor.
 * Controls when sounds are played, attenuated, or culled.
 */
namespace GSDAudioLOD
{
    // LOD 0: Full audio (close entities)
    static constexpr float LOD0_Distance = 500.0f;

    // LOD 1: Reduced audio (mid-distance entities)
    static constexpr float LOD1_Distance = 2000.0f;
    static constexpr float LOD1_VolumeMultiplier = 0.5f;

    // LOD 2: Minimal audio (far entities)
    static constexpr float LOD2_Distance = 4000.0f;
    static constexpr float LOD2_VolumeMultiplier = 0.25f;

    // LOD 3: Culled (no audio)
    static constexpr float CullDistance = 5000.0f;
}

/**
 * Attenuation asset paths for loading.
 */
namespace GSDAttenuationPaths
{
    static const FString CrowdAttenuation = TEXT("/GSD_Audio/Audio/Attenuation/CrowdAttenuation.CrowdAttenuation");
    static const FString VehicleAttenuation = TEXT("/GSD_Audio/Audio/Attenuation/VehicleAttenuation.VehicleAttenuation");
    static const FString VoiceAttenuation = TEXT("/GSD_Audio/Audio/Attenuation/VoiceAttenuation.VoiceAttenuation");
    static const FString AmbienceAttenuation = TEXT("/GSD_Audio/Audio/Attenuation/AmbienceAttenuation.AmbienceAttenuation");
}

/**
 * Occlusion and reverb settings.
 */
namespace GSDSpatialAudioSettings
{
    // Enable occlusion for these sound types
    static const TArray<FName> OcclusionEnabled = {
        FName("Voice"),
        FName("Crowd"),
        FName("Vehicle")
    };

    // Enable reverb for these sound types
    static const TArray<FName> ReverbEnabled = {
        FName("Ambience"),
        FName("Voice"),
        FName("Crowd")
    };

    // Occlusion interpolation time (smooth occlusion changes)
    static constexpr float OcclusionInterpolationTime = 0.3f;

    // Reverb wet level multiplier
    static constexpr float ReverbWetLevel = 0.4f;
}
