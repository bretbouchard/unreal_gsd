#include "DataAssets/GSDHLODConfigAsset.h"
#include "Types/GSDHLODTypes.h"

UGSDHLODConfigAsset::UGSDHLODConfigAsset()
{
    // Initialize with default 3-tier city HLOD configuration
    HLODConfig = GSDHLODHelpers::CreateDefaultCityHLODConfig();
    Description = TEXT("Default HLOD configuration for Charlotte city environment");
}

FName UGSDHLODConfigAsset::GetDefaultAssetPath()
{
    // Default asset path following GSD naming convention
    return FName(TEXT("/GSD_CityStreaming/HLOD/DefaultCityHLODConfig"));
}

UGSDHLODConfigAsset* UGSDHLODConfigAsset::LoadDefaultConfig(UObject* WorldContextObject)
{
    // Attempt to load the default asset
    const FName AssetPath = GetDefaultAssetPath();
    UGSDHLODConfigAsset* LoadedAsset = LoadObject<UGSDHLODConfigAsset>(WorldContextObject, *AssetPath.ToString());

    // If asset doesn't exist, create a transient one with default settings
    if (!LoadedAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("UGSDHLODConfigAsset: Default asset not found at %s, creating transient config"), *AssetPath.ToString());
        LoadedAsset = NewObject<UGSDHLODConfigAsset>(GetTransientPackage(), TEXT("DefaultHLODConfig"));
    }

    return LoadedAsset;
}

bool UGSDHLODConfigAsset::ValidateConfig(FString& OutError) const
{
    // Check that we have at least one layer
    if (HLODConfig.Layers.Num() == 0)
    {
        OutError = TEXT("HLOD configuration must have at least one layer");
        return false;
    }

    // Validate each layer
    for (int32 i = 0; i < HLODConfig.Layers.Num(); ++i)
    {
        const FGSDHLODLayerConfig& Layer = HLODConfig.Layers[i];

        // Check for empty layer name
        if (Layer.LayerName.IsEmpty())
        {
            OutError = FString::Printf(TEXT("Layer %d has empty name"), i);
            return false;
        }

        // Validate cell size
        if (Layer.CellSize < 12800 || Layer.CellSize > 204800)
        {
            OutError = FString::Printf(TEXT("Layer '%s' has invalid cell size %d (must be 12800-204800)"), *Layer.LayerName, Layer.CellSize);
            return false;
        }

        // Validate loading range
        if (Layer.LoadingRange < 25600)
        {
            OutError = FString::Printf(TEXT("Layer '%s' has invalid loading range %d (must be >= 25600)"), *Layer.LayerName, Layer.LoadingRange);
            return false;
        }

        // Validate simplified mesh specific settings
        if (Layer.LayerType == EGSDHLODLayerType::SimplifiedMesh)
        {
            if (Layer.ScreenSize < 0.01f || Layer.ScreenSize > 1.0f)
            {
                OutError = FString::Printf(TEXT("Layer '%s' has invalid screen size %f (must be 0.01-1.0)"), *Layer.LayerName, Layer.ScreenSize);
                return false;
            }
        }
    }

    return true;
}

bool UGSDHLODConfigAsset::GetLayer(int32 Index, FGSDHLODLayerConfig& OutLayer) const
{
    if (HLODConfig.Layers.IsValidIndex(Index))
    {
        OutLayer = HLODConfig.Layers[Index];
        return true;
    }
    return false;
}

FPrimaryAssetId UGSDHLODConfigAsset::GetPrimaryAssetId() const
{
    // Use the asset path as the primary asset ID for cooking and streaming
    return FPrimaryAssetId(TEXT("GSDHLODConfig"), GetPackage()->GetFName());
}
