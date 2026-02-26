// Copyright Bret Bouchard. All Rights Reserved.

#include "Subsystems/GSDSmartObjectSubsystem.h"
#include "SmartObjectsModule/SmartObjectSubsystem.h"

bool UGSDSmartObjectSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UGSDSmartObjectSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    CachedSmartObjectSubsystem = GetWorld()->GetSubsystem<USmartObjectSubsystem>();
}

void UGSDSmartObjectSubsystem::Deinitialize()
{
    CachedSmartObjectSubsystem = nullptr;
    Super::Deinitialize();
}

TArray<FSmartObjectHandle> UGSDSmartObjectSubsystem::FindNearbySmartObjects(
    const FVector& Location,
    float Radius,
    const FGameplayTagContainer& FilterTags) const
{
    if (!CachedSmartObjectSubsystem)
    {
        return {};
    }

    FSmartObjectRequest Request;
    Request.QueryBox = FBoxCenterAndExtent(Location, FVector(Radius));
    Request.Filter = FSmartObjectRequestFilter(FilterTags);

    TArray<FSmartObjectHandle> Results;
    CachedSmartObjectSubsystem->FindSmartObjects(Request, Results);
    return Results;
}

FSmartObjectHandle UGSDSmartObjectSubsystem::FindNearestAvailableSmartObject(
    const FVector& Location,
    float Radius) const
{
    TArray<FSmartObjectHandle> NearbyObjects = FindNearbySmartObjects(Location, Radius);

    if (NearbyObjects.IsEmpty())
    {
        return FSmartObjectHandle();
    }

    // Find nearest available
    FSmartObjectHandle NearestHandle;
    float NearestDistSq = MAX_FLT;

    for (const FSmartObjectHandle& Handle : NearbyObjects)
    {
        if (!IsSmartObjectAvailable(Handle))
        {
            continue;
        }

        const FVector ObjectLocation = GetSmartObjectLocation(Handle);
        const float DistSq = FVector::DistSquared(Location, ObjectLocation);

        if (DistSq < NearestDistSq)
        {
            NearestDistSq = DistSq;
            NearestHandle = Handle;
        }
    }

    return NearestHandle;
}

FSmartObjectClaimHandle UGSDSmartObjectSubsystem::ClaimSmartObject(FSmartObjectHandle Handle)
{
    if (!CachedSmartObjectSubsystem || !Handle.IsValid())
    {
        return FSmartObjectClaimHandle::Invalid;
    }

    return CachedSmartObjectSubsystem->Claim(Handle);
}

void UGSDSmartObjectSubsystem::ReleaseSmartObject(FSmartObjectClaimHandle& Handle)
{
    if (CachedSmartObjectSubsystem && Handle.IsValid())
    {
        CachedSmartObjectSubsystem->Release(Handle);
        Handle = FSmartObjectClaimHandle::Invalid;
    }
}

bool UGSDSmartObjectSubsystem::IsSmartObjectAvailable(FSmartObjectHandle Handle) const
{
    if (!CachedSmartObjectSubsystem || !Handle.IsValid())
    {
        return false;
    }

    // Check if Smart Object can be claimed
    FSmartObjectClaimHandle TestHandle = CachedSmartObjectSubsystem->Claim(Handle);
    if (TestHandle.IsValid())
    {
        CachedSmartObjectSubsystem->Release(TestHandle);
        return true;
    }

    return false;
}

FVector UGSDSmartObjectSubsystem::GetSmartObjectLocation(FSmartObjectHandle Handle) const
{
    if (!CachedSmartObjectSubsystem || !Handle.IsValid())
    {
        return FVector::ZeroVector;
    }

    // Get transform from Smart Object subsystem
    FTransform SOTransform;
    if (CachedSmartObjectSubsystem->GetSmartObjectTransform(Handle, SOTransform))
    {
        return SOTransform.GetLocation();
    }

    return FVector::ZeroVector;
}
