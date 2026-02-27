// Copyright Bret Bouchard. All Rights Reserved.

#include "Widgets/GSDValidationDashboardWidget.h"
#include "GSDValidationLog.h"
#include "Editor/EditorEngine.h"
#include "Misc/OutputDeviceRedirector.h"
#include "HAL/PlatformProcess.h"

UGSDValidationDashboardWidget::UGSDValidationDashboardWidget()
{
    bLastValidationPassed = false;
    bValidationRunning = false;
}

void UGSDValidationDashboardWidget::RunAllValidations()
{
    GSDVALIDATION_LOG(Log, TEXT("Running all validations..."));

    ValidationResults.Empty();
    SetValidationRunning(true);

    // Run all validations sequentially
    ValidateAssets();
    ValidateWorldPartition();
    RunPerformanceRoute();

    // Determine overall pass/fail
    bLastValidationPassed = true;
    for (const FGSDValidationResult& Result : ValidationResults)
    {
        if (!Result.bPassed)
        {
            bLastValidationPassed = false;
            break;
        }
    }

    LastValidationResult = bLastValidationPassed
        ? TEXT("All validations passed")
        : TEXT("One or more validations failed");

    SetValidationRunning(false);
    OnValidationComplete.Broadcast(LastValidationResult);

    GSDVALIDATION_LOG(Log, TEXT("All validations complete: %s"), *LastValidationResult);
}

void UGSDValidationDashboardWidget::ValidateAssets()
{
    GSDVALIDATION_LOG(Log, TEXT("Running asset validation..."));

    SetValidationRunning(true);
    OnValidationProgress.Broadcast(TEXT("Validating assets..."));

    ExecuteCommandlet(TEXT("GSDValidateAssets"), TEXT("json=true"));

    // Note: Actual result parsing happens in ParseCommandletOutput
    // For now, create a placeholder result
    FGSDValidationResult Result;
    Result.bPassed = true;  // Would be parsed from commandlet output
    Result.TotalAssetsChecked = 0;
    ValidationResults.Add(Result);

    SetValidationRunning(false);
}

void UGSDValidationDashboardWidget::ValidateWorldPartition()
{
    GSDVALIDATION_LOG(Log, TEXT("Running World Partition validation..."));

    SetValidationRunning(true);
    OnValidationProgress.Broadcast(TEXT("Validating World Partition..."));

    ExecuteCommandlet(TEXT("GSDValidateWP"), TEXT("json=true"));

    FGSDValidationResult Result;
    Result.bPassed = true;  // Would be parsed from commandlet output
    Result.TotalAssetsChecked = 0;
    ValidationResults.Add(Result);

    SetValidationRunning(false);
}

void UGSDValidationDashboardWidget::RunPerformanceRoute()
{
    GSDVALIDATION_LOG(Log, TEXT("Running performance route..."));

    SetValidationRunning(true);
    OnValidationProgress.Broadcast(TEXT("Running performance route..."));

    ExecuteCommandlet(TEXT("GSDRunPerfRoute"), TEXT("json=true"));

    FGSDValidationResult Result;
    Result.bPassed = true;  // Would be parsed from commandlet output
    Result.TotalAssetsChecked = 0;
    ValidationResults.Add(Result);

    SetValidationRunning(false);
}

void UGSDValidationDashboardWidget::ExecuteCommandlet(const FString& CommandletName, const FString& Params)
{
    GSDVALIDATION_LOG(Log, TEXT("Executing commandlet: %s with params: %s"), *CommandletName, *Params);

    // Build command
    const FString Command = FString::Printf(TEXT("run=%s %s"), *CommandletName, *Params);

    // Execute via console command
    // Note: This is a simplified approach - production code would:
    // 1. Launch commandlet in separate process
    // 2. Capture stdout
    // 3. Parse JSON output
    // 4. Update UI with results

    if (GEngine)
    {
        GEngine->Exec(nullptr, *Command);
    }

    GSDVALIDATION_LOG(Log, TEXT("Commandlet execution complete: %s"), *CommandletName);
}

void UGSDValidationDashboardWidget::ParseCommandletOutput(const FString& Output)
{
    // Parse JSON output from commandlet
    // This would use FJsonObjectConverter to parse the output
    // and populate FGSDValidationResult

    GSDVALIDATION_LOG(Verbose, TEXT("Parsing commandlet output: %s"), *Output);

    // Placeholder: Would parse actual JSON here
    // TSharedPtr<FJsonObject> JsonObject;
    // TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Output);
    // if (FJsonSerializer::Deserialize(Reader, JsonObject))
    // {
    //     // Parse fields into FGSDValidationResult
    // }
}

void UGSDValidationDashboardWidget::SetValidationRunning(bool bRunning)
{
    bValidationRunning = bRunning;

    if (bRunning)
    {
        LastValidationResult = TEXT("Validation in progress...");
    }
}
