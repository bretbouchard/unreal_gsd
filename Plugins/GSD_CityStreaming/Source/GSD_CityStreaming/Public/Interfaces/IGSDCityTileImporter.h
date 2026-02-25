#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Types/GSDImportTypes.h"
#include "IGSDCityTileImporter.generated.h"

// Delegate for async import completion
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGSDOnImportComplete, const FGSDImportResult&, Result, const FString&, TileName);

/**
 * Interface for importing city tiles from external sources.
 *
 * Supports USD (preferred) and FBX formats from blender_gsd pipeline.
 * USD provides better layering support and material handling.
 * FBX is provided as a fallback for maximum compatibility.
 *
 * Usage:
 * 1. Configure FGSDImportConfig with source path and settings
 * 2. Call ImportTile() for single tile or ImportTilesBatch() for multiple
 * 3. Check FGSDImportResult for success and imported asset paths
 *
 * For async import, use ImportTileAsync() with completion delegate.
 */
UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD|Import", meta = (CannotImplementInterfaceInBlueprint))
class UGSDCityTileImporter : public UInterface
{
    GENERATED_BODY()
};

class GSD_CITYSTREAMING_API IGSDCityTileImporter
{
    GENERATED_BODY()

public:
    /**
     * Import a single city tile with the given configuration.
     *
     * @param Config Import configuration containing source path, format, and settings
     * @return FGSDImportResult with success status and imported asset paths
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Import")
    FGSDImportResult ImportTile(const FGSDImportConfig& Config);
    virtual FGSDImportResult ImportTile_Implementation(const FGSDImportConfig& Config);

    /**
     * Import multiple tiles in batch.
     *
     * @param Configs Array of import configurations for each tile
     * @return Array of import results, one per configuration
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Import")
    TArray<FGSDImportResult> ImportTilesBatch(const TArray<FGSDImportConfig>& Configs);
    virtual TArray<FGSDImportResult> ImportTilesBatch_Implementation(const TArray<FGSDImportConfig>& Configs);

    /**
     * Validate import configuration before import.
     * Checks file existence, valid extension, and destination path.
     *
     * @param Config Import configuration to validate
     * @param OutErrorMessage Error message if validation fails
     * @return True if configuration is valid
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Import")
    bool ValidateConfig(const FGSDImportConfig& Config, FString& OutErrorMessage);
    virtual bool ValidateConfig_Implementation(const FGSDImportConfig& Config, FString& OutErrorMessage);

    /**
     * Get supported import formats.
     *
     * @return Array of supported EGSDImportFormat values
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Import")
    TArray<EGSDImportFormat> GetSupportedFormats() const;
    virtual TArray<EGSDImportFormat> GetSupportedFormats_Implementation() const;

    /**
     * Import a tile asynchronously with callback.
     *
     * @param Config Import configuration
     * @param OnComplete Delegate to call when import completes
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Import")
    void ImportTileAsync(const FGSDImportConfig& Config, const FGSDOnImportComplete& OnComplete);
    virtual void ImportTileAsync_Implementation(const FGSDImportConfig& Config, const FGSDOnImportComplete& OnComplete);

protected:
    /**
     * Internal import implementation for USD format.
     * Placeholder - actual implementation requires editor-only functionality.
     *
     * @param Config Import configuration
     * @return FGSDImportResult with import outcome
     */
    virtual FGSDImportResult ImportUSD(const FGSDImportConfig& Config);

    /**
     * Internal import implementation for FBX format.
     * Placeholder - actual implementation requires editor-only functionality.
     *
     * @param Config Import configuration
     * @return FGSDImportResult with import outcome
     */
    virtual FGSDImportResult ImportFBX(const FGSDImportConfig& Config);
};
