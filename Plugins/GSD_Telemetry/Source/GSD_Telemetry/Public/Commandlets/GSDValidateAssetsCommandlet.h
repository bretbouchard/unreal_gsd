// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "Types/GSDValidationTypes.h"
#include "GSDValidateAssetsCommandlet.generated.h"

/**
 * Asset validation commandlet for CI pipelines
 * Implements TEL-04 (GSDValidateAssets commandlet)
 *
 * Usage:
 *   UnrealEditor-Cmd.exe MyProject -run=GSDValidateAssets
 *
 * Parameters:
 *   - maxsize=100        : Maximum asset size in MB (default: 100)
 *   - output=path.json   : Output file path for JSON report
 *   - json=true          : Output JSON to stdout (default: true)
 *   - verbose=true       : Enable verbose logging (default: false)
 *
 * Exit codes:
 *   0 = All assets passed validation
 *   1 = One or more assets failed validation
 */
UCLASS()
class GSD_TELEMETRY_API UGSDValidateAssetsCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    UGSDValidateAssetsCommandlet();

    virtual int32 Main(const FString& Params) override;

private:
    // Configuration
    UPROPERTY()
    float MaxAssetSizeMB = 100.0f;

    UPROPERTY()
    bool bOutputJSON = true;

    UPROPERTY()
    bool bVerbose = false;

    UPROPERTY()
    FString OutputPath;

    // Per-asset-type budgets
    TMap<FString, float> AssetTypeBudgets;

    // Methods
    void ParseParameters(const FString& Params);
    void InitializeDefaultBudgets();
    bool ValidateAsset(UObject* Asset, FGSDValidationResult& OutResult);
    FString GetAssetType(UObject* Asset) const;
    float GetAssetSizeMB(UObject* Asset) const;
    void OutputJSON(const FGSDValidationResult& Result);
    void OutputText(const FGSDValidationResult& Result);
};
