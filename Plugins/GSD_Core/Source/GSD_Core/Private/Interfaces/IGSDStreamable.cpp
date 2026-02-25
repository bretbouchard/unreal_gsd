#include "Interfaces/IGSDStreamable.h"

FBoxSphereBounds IGSDStreamable::GetStreamingBounds_Implementation()
{
    return FBoxSphereBounds(FVector::ZeroVector, FVector(100.0f), 100.0f);
}

void IGSDStreamable::OnStreamIn_Implementation()
{
    // Default implementation - override in C++ or Blueprint
}

void IGSDStreamable::OnStreamOut_Implementation()
{
    // Default implementation - override in C++ or Blueprint
}

int32 IGSDStreamable::GetStreamingPriority_Implementation()
{
    return 0;
}

bool IGSDStreamable::ShouldPersist_Implementation()
{
    return false;
}

float IGSDStreamable::GetStreamingDistanceOverride_Implementation()
{
    return 0.0f;
}

bool IGSDStreamable::IsCurrentlyStreamedIn_Implementation() const
{
    return true;
}
