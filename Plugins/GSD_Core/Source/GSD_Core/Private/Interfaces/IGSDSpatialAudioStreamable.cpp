#include "Interfaces/IGSDSpatialAudioStreamable.h"

FGSDSpatialAudioState IGSDSpatialAudioStreamable::GetSpatialState_Implementation() const
{
    return FGSDSpatialAudioState();
}

void IGSDSpatialAudioStreamable::RestoreSpatialState_Implementation(const FGSDSpatialAudioState& State)
{
    // Default implementation - override in C++ or Blueprint
}

float IGSDSpatialAudioStreamable::GetAudioLODDistance_Implementation() const
{
    return 500.0f; // Default to LOD0 distance
}

int32 IGSDSpatialAudioStreamable::GetCurrentAudioLOD_Implementation() const
{
    return 0;
}

void IGSDSpatialAudioStreamable::SetAudioLOD_Implementation(int32 LODLevel)
{
    // Default implementation - override in C++ or Blueprint
}

bool IGSDSpatialAudioStreamable::IsAudioPlaying_Implementation() const
{
    return false;
}

void IGSDSpatialAudioStreamable::VirtualizeAudio_Implementation()
{
    // Default implementation - override in C++ or Blueprint
}

void IGSDSpatialAudioStreamable::DevirtualizeAudio_Implementation()
{
    // Default implementation - override in C++ or Blueprint
}
