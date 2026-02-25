#include "Interfaces/IGSDMetaSoundInterface.h"

UObject* IGSDMetaSoundInterface::GetMetaSoundSource_Implementation() const
{
    return nullptr;
}

TMap<FName, float> IGSDMetaSoundInterface::GetDefaultParameters_Implementation() const
{
    return TMap<FName, float>();
}

USoundSubmix* IGSDMetaSoundInterface::GetTargetSubmix_Implementation() const
{
    return nullptr;
}

USoundClass* IGSDMetaSoundInterface::GetSoundClass_Implementation() const
{
    return nullptr;
}

void IGSDMetaSoundInterface::SetFloatParameter_Implementation(FName ParameterName, float Value)
{
    // Default implementation - override in C++ or Blueprint
}

bool IGSDMetaSoundInterface::GetFloatParameter_Implementation(FName ParameterName, float& OutValue) const
{
    OutValue = 0.0f;
    return false;
}
