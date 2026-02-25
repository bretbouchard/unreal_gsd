#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/GSDHLODTypes.h"
#include "GSDHLODConfigAsset.generated.h"

/**
 * Data Asset for HLOD (Hierarchical Level of Detail) configuration.
 * Allows per-level HLOD configuration overrides with designer-friendly editing.
 *
 * Follows the GSD_DataAsset pattern from Phase 1 for consistency.
 * Inherit from UPrimaryDataAsset for proper asset management and cooking.
 */
UCLASS(BlueprintType, Category = "GSD|HLOD")
class GSD_CITYSTREAMING_API UGSDHLODConfigAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UGSDHLODConfigAsset();

    // === Accessors ===

    /**
     * Get the HLOD system configuration.
     * @return Reference to the HLOD system config struct
     */
    UFUNCTION(BlueprintPure, Category = "GSD|HLOD")
    const FGSDHLODSystemConfig& GetHLODConfig() const { return HLODConfig; }

    /**
     * Get the default HLOD configuration asset path.
     * @return Asset path for default city HLOD config
     */
    static FName GetDefaultAssetPath();

    /**
     * Load the default HLOD configuration asset.
     * @return Loaded asset, or nullptr if not found
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|HLOD", meta = (WorldContext = "WorldContextObject"))
    static UGSDHLODConfigAsset* LoadDefaultConfig(UObject* WorldContextObject);

    // === Validation ===

    /**
     * Validate the HLOD configuration.
     * @param OutError Error message if validation fails
     * @return True if configuration is valid
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|HLOD")
    bool ValidateConfig(FString& OutError) const;

    /**
     * Get the number of HLOD layers configured.
     * @return Number of layers
     */
    UFUNCTION(BlueprintPure, Category = "GSD|HLOD")
    int32 GetLayerCount() const { return HLODConfig.Layers.Num(); }

    /**
     * Get a specific HLOD layer configuration by index.
     * @param Index Layer index
     * @param OutLayer Output layer config
     * @return True if layer exists at index
     */
    UFUNCTION(BlueprintPure, Category = "GSD|HLOD")
    bool GetLayer(int32 Index, FGSDHLODLayerConfig& OutLayer) const;

protected:
    // === Configuration ===

    /** Main HLOD system configuration containing all layer tiers */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HLOD", meta = (ShowOnlyInnerProperties))
    FGSDHLODSystemConfig HLODConfig;

    /** Description for documentation purposes */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HLOD", meta = (MultiLine = true))
    FString Description;

    // === UPrimaryDataAsset Interface ===

    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
