// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "GSDValidateWPCommandlet.generated.h"

// Forward declaration - FGSDWorldPartitionValidationConfig is in GSD_ValidationTools
struct FGSDWorldPartitionValidationConfig;
struct FGSDValidationResult;

/**
 * World Partition validation commandlet for CI pipelines
 * Implements TEL-05 (GSDValidateWorldPartition commandlet)
 *
 * Usage:
 *   UnrealEditor-Cmd.exe MyProject -run=GSDValidateWP
 *
 * Parameters:
 *   - mincellsize=12800    : Minimum cell size in cm (default: 12800 = 128m)
 *   - minhlodlayers=3      : Minimum HLOD layers (default: 3)
 *   - json=true            : Output JSON to stdout (default: true)
 *
 * Exit codes:
 *   0 = World Partition configuration valid
 *   1 = Configuration issues detected
 */
UCLASS()
class GSD_TELEMETRY_API UGSDValidateWPCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    UGSDValidateWPCommandlet();

    virtual int32 Main(const FString& Params) override;

private:
    // Configuration
    float MinCellSize = 12800.0f;       // 128m default
    int32 MinHLODLayers = 3;            // 3 layers minimum
    bool bRequireDataLayers = true;     // Data layers required

    // Output options
    bool bOutputJSON = true;
    bool bVerbose = false;

    // Methods
    void ParseParameters(const FString& Params);
    bool ValidateWorldPartition(UWorld* World, struct FGSDValidationResultSimple& OutResult);
    void OutputJSON(const FGSDValidationResultSimple& Result);
    void OutputText(const FGSDValidationResultSimple& Result);
};

/**
 * Simplified validation result for this commandlet
 * (does not depend on GSD_ValidationTools types)
 */
USTRUCT()
struct FGSDValidationResultSimple
{
    GENERATED_BODY()

    UPROPERTY()
    bool bPassed = true;

    UPROPERTY()
    int32 TotalAssetsChecked = 0;

    UPROPERTY()
    int32 ErrorCount = 0;

    UPROPERTY()
    int32 WarningCount = 0;

    UPROPERTY()
    double ValidationTimeSeconds = 0.0;

    UPROPERTY()
    TArray<FString> Errors;

    UPROPERTY()
    TArray<FString> Warnings;

    void AddError(const FString& Message)
    {
        Errors.Add(Message);
        ErrorCount++;
        bPassed = false;
    }

    void AddWarning(const FString& Message)
    {
        Warnings.Add(Message);
        WarningCount++;
    }
};

/**
 * Validation issue for structured output
 */
USTRUCT()
struct FGSDValidationIssueSimple
{
    GENERATED_BODY()

    UPROPERTY()
    FString AssetPath;

    UPROPERTY()
    FString IssueType;

    UPROPERTY()
    FString Description;

    UPROPERTY()
    float Severity = 0.0f;  // 0.0 = warning, 1.0 = error

    UPROPERTY()
    FString Suggestion;
};
