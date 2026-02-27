// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "Types/GSDValidationTypes.h"
#include "GSDValidationDashboardWidget.generated.h"

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnValidationComplete, const FString&, ResultString);

/**
 * Editor validation dashboard widget for one-click validation
 * Implements TEL-07 (Editor validation UI)
 *
 * Blueprint creates the actual UI layout, this class provides:
 * - Validation execution methods (callable from Blueprint)
 * - Result storage and display
 * - Progress tracking
 */
UCLASS(BlueprintType)
class GSD_VALIDATIONTOOLS_API UGSDValidationDashboardWidget : public UEditorUtilityWidget
{
    GENERATED_BODY()

public:
    UGSDValidationDashboardWidget();

    // Validation execution methods (callable from Blueprint buttons)
    UFUNCTION(BlueprintCallable, Category = "GSD|Validation")
    void RunAllValidations();

    UFUNCTION(BlueprintCallable, Category = "GSD|Validation")
    void ValidateAssets();

    UFUNCTION(BlueprintCallable, Category = "GSD|Validation")
    void ValidateWorldPartition();

    UFUNCTION(BlueprintCallable, Category = "GSD|Validation")
    void RunPerformanceRoute();

    // Result access (for Blueprint display)
    UFUNCTION(BlueprintPure, Category = "GSD|Validation")
    FString GetLastValidationResult() const { return LastValidationResult; }

    UFUNCTION(BlueprintPure, Category = "GSD|Validation")
    bool GetLastValidationPassed() const { return bLastValidationPassed; }

    UFUNCTION(BlueprintPure, Category = "GSD|Validation")
    bool IsValidationRunning() const { return bValidationRunning; }

    // Delegates (for Blueprint event binding)
    UPROPERTY(BlueprintAssignable, Category = "GSD|Validation")
    FOnValidationComplete OnValidationComplete;

    UPROPERTY(BlueprintAssignable, Category = "GSD|Validation")
    FOnValidationComplete OnValidationProgress;

protected:
    // Helper methods
    void ExecuteCommandlet(const FString& CommandletName, const FString& Params);
    void ParseCommandletOutput(const FString& Output);
    void SetValidationRunning(bool bRunning);

    // Result storage
    UPROPERTY(BlueprintReadOnly, Category = "GSD|Validation")
    FString LastValidationResult;

    UPROPERTY(BlueprintReadOnly, Category = "GSD|Validation")
    bool bLastValidationPassed = false;

    UPROPERTY(BlueprintReadOnly, Category = "GSD|Validation")
    bool bValidationRunning = false;

    // Accumulated results
    UPROPERTY(BlueprintReadOnly, Category = "GSD|Validation")
    TArray<FGSDValidationResult> ValidationResults;

private:
    // Commandlet output buffer
    FString CommandletOutputBuffer;
};
