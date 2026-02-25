#include "DataAssets/GSDDataAsset.h"
#include "GSDLog.h"

UGSDDataAsset::UGSDDataAsset()
{
    DisplayName = TEXT("GSD Config");
}

FString UGSDDataAsset::GetDisplayName() const
{
    if (!DisplayName.IsEmpty())
    {
        return DisplayName;
    }
    return GetName();
}

bool UGSDDataAsset::ValidateConfig(FString& OutError) const
{
    // Base validation - override in derived classes
    OutError.Empty();
    return true;
}

void UGSDDataAsset::ApplyMigrations()
{
    // Override in derived classes to handle version migrations
    GSD_LOG(Verbose, TEXT("UGSDDataAsset::ApplyMigrations for %s (version %d)"),
        *GetName(), ConfigVersion);
}

bool UGSDDataAsset::HasTag(FName Tag) const
{
    return ConfigTags.Contains(Tag);
}
