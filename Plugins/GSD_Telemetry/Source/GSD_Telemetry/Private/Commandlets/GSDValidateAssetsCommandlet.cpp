// Copyright Bret Bouchard. All Rights Reserved.

#include "Commandlets/GSDValidateAssetsCommandlet.h"
#include "GSDTelemetryLog.h"
#include "JsonObjectConverter.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UObject/UObjectIterator.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UObject/Package.h"

UGSDValidateAssetsCommandlet::UGSDValidateAssetsCommandlet()
{
    HelpDescription = TEXT("Asset validation commandlet for CI pipelines");
    HelpUsage = TEXT("UnrealEditor-Cmd.exe MyProject -run=GSDValidateAssets");
    HelpParamNames.Add(TEXT("maxsize"));
    HelpParamDescriptions.Add(TEXT("Maximum asset size in MB (default: 100)"));
    HelpParamNames.Add(TEXT("output"));
    HelpParamDescriptions.Add(TEXT("Output file path for JSON report"));
    HelpParamNames.Add(TEXT("json"));
    HelpParamDescriptions.Add(TEXT("Output JSON to stdout (default: true)"));
    HelpParamNames.Add(TEXT("verbose"));
    HelpParamDescriptions.Add(TEXT("Enable verbose logging (default: false)"));
}

void UGSDValidateAssetsCommandlet::ParseParameters(const FString& Params)
{
    // Parse maxsize
    float ParsedMaxSize;
    if (FParse::Value(*Params, TEXT("maxsize="), ParsedMaxSize))
    {
        MaxAssetSizeMB = ParsedMaxSize;
        GSDTELEMETRY_LOG(Log, TEXT("Parsed maxsize: %.2f MB"), MaxAssetSizeMB);
    }

    // Parse output path
    FString ParsedOutputPath;
    if (FParse::Value(*Params, TEXT("output="), ParsedOutputPath))
    {
        OutputPath = ParsedOutputPath;
        GSDTELEMETRY_LOG(Log, TEXT("Parsed output path: %s"), *OutputPath);
    }

    // Parse json flag
    FString JsonFlag;
    if (FParse::Value(*Params, TEXT("json="), JsonFlag))
    {
        bOutputJSON = JsonFlag.ToLower() == TEXT("true");
        GSDTELEMETRY_LOG(Log, TEXT("Parsed json output: %s"), bOutputJSON ? TEXT("true") : TEXT("false"));
    }

    // Parse verbose flag
    FString VerboseFlag;
    if (FParse::Value(*Params, TEXT("verbose="), VerboseFlag))
    {
        bVerbose = VerboseFlag.ToLower() == TEXT("true");
        GSDTELEMETRY_LOG(Log, TEXT("Parsed verbose: %s"), bVerbose ? TEXT("true") : TEXT("false"));
    }
}

void UGSDValidateAssetsCommandlet::InitializeDefaultBudgets()
{
    // Set per-asset-type budgets (in MB)
    AssetTypeBudgets.Add(TEXT("StaticMesh"), 50.0f);
    AssetTypeBudgets.Add(TEXT("Texture2D"), 100.0f);
    AssetTypeBudgets.Add(TEXT("SkeletalMesh"), 75.0f);
    AssetTypeBudgets.Add(TEXT("SoundWave"), 25.0f);
    AssetTypeBudgets.Add(TEXT("AnimSequence"), 50.0f);
    AssetTypeBudgets.Add(TEXT("Material"), 10.0f);
    // Default fallback uses MaxAssetSizeMB
}

int32 UGSDValidateAssetsCommandlet::Main(const FString& Params)
{
    GSDTELEMETRY_LOG(Log, TEXT("GSDValidateAssetsCommandlet starting..."));

    const double StartTime = FPlatformTime::Seconds();

    // Parse parameters
    ParseParameters(Params);

    // Initialize budgets
    InitializeDefaultBudgets();

    // Validation result
    FGSDValidationResult Result;

    // Iterate all assets
    int32 TotalAssets = 0;
    for (TObjectIterator<UObject> It; It; ++It)
    {
        UObject* Asset = *It;
        if (!Asset || !Asset->IsAsset())
            continue;

        TotalAssets++;
        Result.TotalAssetsChecked++;

        // Validate asset
        if (!ValidateAsset(Asset, Result))
        {
            // Error already added by ValidateAsset
            if (bVerbose)
            {
                GSDTELEMETRY_LOG(Warning, TEXT("Asset validation failed: %s"), *Asset->GetPathName());
            }
        }
    }

    const double EndTime = FPlatformTime::Seconds();
    Result.ValidationTimeSeconds = EndTime - StartTime;

    // Output results
    if (bOutputJSON)
    {
        OutputJSON(Result);
    }
    else
    {
        OutputText(Result);
    }

    GSDTELEMETRY_LOG(Log, TEXT("GSDValidateAssetsCommandlet completed: %s"),
        Result.bPassed ? TEXT("SUCCESS") : TEXT("FAILED"));
    GSDTELEMETRY_LOG(Log, TEXT("Assets checked: %d | Errors: %d | Warnings: %d | Time: %.2fs"),
        Result.TotalAssetsChecked, Result.ErrorCount, Result.WarningCount, Result.ValidationTimeSeconds);

    return Result.bPassed ? 0 : 1;
}

bool UGSDValidateAssetsCommandlet::ValidateAsset(UObject* Asset, FGSDValidationResult& OutResult)
{
    if (!Asset)
        return false;

    const FString AssetPath = Asset->GetPathName();
    const FString AssetType = GetAssetType(Asset);
    const float AssetSizeMB = GetAssetSizeMB(Asset);

    // Get budget for this asset type (or use default)
    const float* BudgetPtr = AssetTypeBudgets.Find(AssetType);
    const float BudgetMB = BudgetPtr ? *BudgetPtr : MaxAssetSizeMB;

    // Check size against budget
    if (AssetSizeMB > BudgetMB)
    {
        const FString Description = FString::Printf(
            TEXT("Asset size %.2f MB exceeds budget %.2f MB for type %s"),
            AssetSizeMB, BudgetMB, *AssetType);

        const FString Suggestion = TEXT("Reduce asset size or increase budget in validation config");

        OutResult.AddError(AssetPath, TEXT("SizeExceeded"), Description, Suggestion);

        GSDTELEMETRY_LOG(Error, TEXT("%s"), *Description);
        return false;
    }

    if (bVerbose)
    {
        GSDTELEMETRY_LOG(Log, TEXT("Asset OK: %s (%.2f MB, budget: %.2f MB)"),
            *AssetPath, AssetSizeMB, BudgetMB);
    }

    return true;
}

FString UGSDValidateAssetsCommandlet::GetAssetType(UObject* Asset) const
{
    if (!Asset)
        return TEXT("Unknown");

    UClass* Class = Asset->GetClass();
    if (!Class)
        return TEXT("Unknown");

    // Return class name without U prefix
    FString ClassName = Class->GetName();
    if (ClassName.StartsWith(TEXT("U")))
    {
        ClassName.RemoveAt(0);
    }

    return ClassName;
}

float UGSDValidateAssetsCommandlet::GetAssetSizeMB(UObject* Asset) const
{
    if (!Asset)
        return 0.0f;

    // Get asset size from its outermost package
    if (UPackage* Package = Asset->GetOutermost())
    {
        FString PackagePath;
        if (FPackageName::TryConvertLongPackageNameToFilename(Package->GetName(), PackagePath))
        {
            if (FPaths::FileExists(PackagePath))
            {
                const int64 FileSize = IFileManager::Get().FileSize(*PackagePath);
                return static_cast<float>(FileSize) / (1024.0f * 1024.0f);
            }
        }
    }

    return 0.0f;
}

void UGSDValidateAssetsCommandlet::OutputJSON(const FGSDValidationResult& Result)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

    JsonObject->SetBoolField(TEXT("passed"), Result.bPassed);
    JsonObject->SetNumberField(TEXT("total_assets_checked"), Result.TotalAssetsChecked);
    JsonObject->SetNumberField(TEXT("error_count"), Result.ErrorCount);
    JsonObject->SetNumberField(TEXT("warning_count"), Result.WarningCount);
    JsonObject->SetNumberField(TEXT("validation_time_seconds"), Result.ValidationTimeSeconds);

    // Issues array
    TArray<TSharedPtr<FJsonValue>> IssuesArray;
    for (const FGSDValidationIssue& Issue : Result.Issues)
    {
        TSharedPtr<FJsonObject> IssueObj = MakeShareable(new FJsonObject);
        IssueObj->SetStringField(TEXT("asset_path"), Issue.AssetPath);
        IssueObj->SetStringField(TEXT("issue_type"), Issue.IssueType);
        IssueObj->SetStringField(TEXT("description"), Issue.Description);
        IssueObj->SetNumberField(TEXT("severity"), Issue.Severity);
        IssueObj->SetStringField(TEXT("suggestion"), Issue.Suggestion);
        IssuesArray.Add(MakeShareable(new FJsonValueObject(IssueObj)));
    }
    JsonObject->SetArrayField(TEXT("issues"), IssuesArray);

    // Serialize to string
    FString OutputString;
    TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

    // Output to stdout for CI parsing
    fprintf(stdout, "%s\n", *OutputString);
    fflush(stdout);

    // Log for visibility
    GSDTELEMETRY_LOG(Log, TEXT("JSON_OUTPUT: %s"), *OutputString);

    // Write to file if path specified
    if (!OutputPath.IsEmpty())
    {
        FFileHelper::SaveStringToFile(OutputString, *OutputPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
        GSDTELEMETRY_LOG(Log, TEXT("JSON report written to: %s"), *OutputPath);
    }
}

void UGSDValidateAssetsCommandlet::OutputText(const FGSDValidationResult& Result)
{
    const FString Status = Result.bPassed ? TEXT("PASSED") : TEXT("FAILED");

    GSDTELEMETRY_LOG(Log, TEXT("=== ASSET VALIDATION RESULT ==="));
    GSDTELEMETRY_LOG(Log, TEXT("Status: %s"), *Status);
    GSDTELEMETRY_LOG(Log, TEXT("Total Assets Checked: %d"), Result.TotalAssetsChecked);
    GSDTELEMETRY_LOG(Log, TEXT("Errors: %d"), Result.ErrorCount);
    GSDTELEMETRY_LOG(Log, TEXT("Warnings: %d"), Result.WarningCount);
    GSDTELEMETRY_LOG(Log, TEXT("Validation Time: %.2f seconds"), Result.ValidationTimeSeconds);

    if (Result.Issues.Num() > 0)
    {
        GSDTELEMETRY_LOG(Log, TEXT(""));
        GSDTELEMETRY_LOG(Log, TEXT("Issues:"));
        for (const FGSDValidationIssue& Issue : Result.Issues)
        {
            const FString SeverityStr = Issue.Severity > 0.5f ? TEXT("ERROR") : TEXT("WARNING");
            GSDTELEMETRY_LOG(Log, TEXT("  [%s] %s"), *SeverityStr, *Issue.AssetPath);
            GSDTELEMETRY_LOG(Log, TEXT("    Type: %s"), *Issue.IssueType);
            GSDTELEMETRY_LOG(Log, TEXT("    Description: %s"), *Issue.Description);
            if (!Issue.Suggestion.IsEmpty())
            {
                GSDTELEMETRY_LOG(Log, TEXT("    Suggestion: %s"), *Issue.Suggestion);
            }
        }
    }
}
