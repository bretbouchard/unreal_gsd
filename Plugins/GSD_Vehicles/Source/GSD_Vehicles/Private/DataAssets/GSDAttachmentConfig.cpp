// Copyright Bret Bouchard. All Rights Reserved.

#include "DataAssets/GSDAttachmentConfig.h"
#include "GSDVehicleLog.h"

UGSDAttachmentConfig::UGSDAttachmentConfig()
    : SocketName(NAME_None)
    , AttachmentType(EGSDAttachmentType::Custom)
    , DisplayName()
    , bHasCollision(true)
    , AdditionalMass(0.0f)
    , bAffectsPhysics(false)
    , DragModifier(0.0f)
{
}

bool UGSDAttachmentConfig::ValidateConfig(FString& OutError) const
{
    bool bIsValid = true;
    TArray<FString> Errors;

    // Check that socket name is set
    if (SocketName == NAME_None)
    {
        Errors.Add(TEXT("SocketName must be set."));
        bIsValid = false;
    }

    // Check that display name is set
    if (DisplayName.IsEmpty())
    {
        Errors.Add(TEXT("DisplayName must be set."));
        bIsValid = false;
    }

    // AttachmentMesh can be null for socket-only attachments
    // (some gameplay systems may spawn meshes dynamically)

    // Validate additional mass
    if (AdditionalMass < 0.0f)
    {
        Errors.Add(FString::Printf(TEXT("AdditionalMass (%.2f) cannot be negative."), AdditionalMass));
        bIsValid = false;
    }

    // Validate drag modifier
    if (DragModifier < -1.0f || DragModifier > 1.0f)
    {
        Errors.Add(FString::Printf(TEXT("DragModifier (%.2f) must be between -1 and 1."), DragModifier));
        bIsValid = false;
    }

    // Log all validation errors
    if (!bIsValid)
    {
        for (const FString& Error : Errors)
        {
            UE_LOG(LOG_GSDVEHICLES, Error, TEXT("AttachmentConfig '%s' validation failed: %s"), *GetName(), *Error);
        }
        OutError = FString::Join(Errors, TEXT(" "));
    }

    return bIsValid;
}

FString UGSDAttachmentConfig::GetDisplayName() const
{
    return DisplayName.IsEmpty() ? TEXT("Attachment Config") : DisplayName;
}
