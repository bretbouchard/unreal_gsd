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
UINTERFACE(MinimalAPI, NotBlueprintable, Category = "GSD|Import")
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
    UFUNCTION(BlueprintCallable, Category = "GSD|Import")
    virtual FGSDImportResult ImportTile(const FGSDImportConfig& Config);

    /**
     * Import multiple tiles in batch.
     *
     * @param Configs Array of import configurations for each tile
     * @return Array of import results, one per configuration
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Import")
    virtual TArray<FGSDImportResult> ImportTilesBatch(const TArray<FGSDImportConfig>& Configs);

    /**
     * Validate import configuration before import.
     * Checks file existence, valid extension, and destination path.
     *
     * @param Config Import configuration to validate
     * @param OutErrorMessage Error message if validation fails
     * @return True if configuration is valid
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Import")
    virtual bool ValidateConfig(const FGSDImportConfig& Config, FString& OutErrorMessage);

    /**
     * Get supported import formats.
     *
     * @return Array of supported EGSDImportFormat values
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Import")
    virtual TArray<EGSDImportFormat> GetSupportedFormats() const;

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
