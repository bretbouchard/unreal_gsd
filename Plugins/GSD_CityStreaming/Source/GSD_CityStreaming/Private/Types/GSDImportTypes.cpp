#include "Types/GSDImportTypes.h"

EGSDImportFormat GSDImportHelpers::DetectFormatFromExtension(const FString& FilePath)
{
    const FString Extension = GetFileExtension(FilePath);

    if (Extension == TEXT("usd") || Extension == TEXT("usda") || Extension == TEXT("usdc"))
    {
        return EGSDImportFormat::USD;
    }
    else if (Extension == TEXT("fbx"))
    {
        return EGSDImportFormat::FBX;
    }

    // Default to USD if extension is unknown
    return EGSDImportFormat::USD;
}

bool GSDImportHelpers::IsSupportedExtension(const FString& FilePath)
{
    const FString Extension = GetFileExtension(FilePath);

    return Extension == TEXT("usd")
        || Extension == TEXT("usda")
        || Extension == TEXT("usdc")
        || Extension == TEXT("fbx");
}

FString GSDImportHelpers::GetFileExtension(const FString& FilePath)
{
    // Find the last dot in the path
    int32 DotIndex;
    if (FilePath.FindLastChar('.', DotIndex))
    {
        // Extract extension after the dot and convert to lowercase
        FString Extension = FilePath.Mid(DotIndex + 1).ToLower();
        return Extension;
    }

    return TEXT("");
}
