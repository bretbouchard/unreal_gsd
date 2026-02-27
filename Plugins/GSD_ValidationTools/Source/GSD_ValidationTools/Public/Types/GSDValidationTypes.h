// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GSDValidationTypes.generated.h"

/**
 * Asset budget configuration for a single asset type
 */
USTRUCT(BlueprintType)
struct FGSDAssetBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    FString AssetType;  // e.g., "StaticMesh", "Texture", "SkeletalMesh"

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    float MaxSizeMB = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    FString Description;
};

/**
 * Validation error or warning
 */
USTRUCT(BlueprintType)
struct FGSDValidationIssue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    FString AssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    FString IssueType;  // e.g., "SizeExceeded", "MissingLOD", "InvalidConfig"

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    float Severity = 0.0f;  // 0.0 = warning, 1.0 = error

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    FString Suggestion;  // How to fix
};

/**
 * Complete validation result for a single validation run
 */
USTRUCT(BlueprintType)
struct FGSDValidationResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    bool bPassed = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    int32 TotalAssetsChecked = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    int32 ErrorCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    int32 WarningCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    TArray<FGSDValidationIssue> Issues;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    double ValidationTimeSeconds = 0.0;

    void AddError(const FString& AssetPath, const FString& IssueType,
                  const FString& Description, const FString& Suggestion = TEXT(""))
    {
        FGSDValidationIssue Issue;
        Issue.AssetPath = AssetPath;
        Issue.IssueType = IssueType;
        Issue.Description = Description;
        Issue.Severity = 1.0f;
        Issue.Suggestion = Suggestion;
        Issues.Add(Issue);
        ErrorCount++;
        bPassed = false;
    }

    void AddWarning(const FString& AssetPath, const FString& IssueType,
                    const FString& Description, const FString& Suggestion = TEXT(""))
    {
        FGSDValidationIssue Issue;
        Issue.AssetPath = AssetPath;
        Issue.IssueType = IssueType;
        Issue.Description = Description;
        Issue.Severity = 0.0f;
        Issue.Suggestion = Suggestion;
        Issues.Add(Issue);
        WarningCount++;
    }
};

/**
 * World Partition validation configuration
 */
USTRUCT(BlueprintType)
struct FGSDWorldPartitionValidationConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    float MinCellSize = 12800.0f;  // 128m minimum

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    int32 MinHLODLayers = 3;  // Require 3 HLOD levels

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    float MaxLoadingRange = 50000.0f;  // 500m max

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    bool bRequireDataLayers = true;  // Data layers must exist
};

/**
 * Performance route waypoint for baseline capture
 */
USTRUCT(BlueprintType)
struct FGSDPerfRouteWaypoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    FString WaypointName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    float ExpectedFrameTimeMs = 16.67f;  // 60fps target
};
