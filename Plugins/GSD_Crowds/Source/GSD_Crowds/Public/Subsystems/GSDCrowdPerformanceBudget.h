// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GSDCrowdPerformanceBudget.generated.h"

/**
 * Performance quality tiers for crowd system scalability.
 */
UENUM(BlueprintType)
enum class EGSDCrowdQuality : uint8
{
    Low     UMETA(DisplayName = "Low (500 entities, 1ms budget)"),
    Medium  UMETA(DisplayName = "Medium (2000 entities, 2ms budget)"),
    High    UMETA(DisplayName = "High (5000 entities, 3ms budget)"),
    Epic    UMETA(DisplayName = "Epic (10000 entities, 4ms budget)")
};

/**
 * Performance budget configuration for crowd system.
 * Controls entity limits, frame time budgets, and automatic quality scaling.
 */
USTRUCT(BlueprintType)
struct GSD_CROWDS_API FGSDCrowdPerformanceConfig
{
    GENERATED_BODY()

    //-- Frame Time Budgets (in milliseconds) --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Budget")
    float MaxProcessingTimeMs = 2.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Budget")
    float WarningThresholdMs = 1.5f;

    //-- Entity Limits by Quality Tier --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limits")
    int32 MaxEntitiesLow = 500;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limits")
    int32 MaxEntitiesMedium = 2000;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limits")
    int32 MaxEntitiesHigh = 5000;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limits")
    int32 MaxEntitiesEpic = 10000;

    //-- Auto-Quality Scaling --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AutoScale")
    bool bEnableAutoQualityScaling = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AutoScale")
    float FrameTimeThresholdForDownscale = 20.0f;  // ms

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AutoScale")
    float FrameTimeThresholdForUpscale = 12.0f;  // ms

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AutoScale")
    int32 FramesBeforeDownscale = 30;  // 0.5 seconds at 60fps

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AutoScale")
    int32 FramesBeforeUpscale = 180;  // 3 seconds at 60fps

    //-- Current Quality Tier --
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EGSDCrowdQuality CurrentQuality = EGSDCrowdQuality::High;

    //-- Get max entities for current quality --
    int32 GetMaxEntities() const
    {
        switch (CurrentQuality)
        {
        case EGSDCrowdQuality::Low: return MaxEntitiesLow;
        case EGSDCrowdQuality::Medium: return MaxEntitiesMedium;
        case EGSDCrowdQuality::High: return MaxEntitiesHigh;
        case EGSDCrowdQuality::Epic: return MaxEntitiesEpic;
        default: return MaxEntitiesHigh;
        }
    }

    //-- Get budget for current quality --
    float GetBudgetMs() const
    {
        switch (CurrentQuality)
        {
        case EGSDCrowdQuality::Low: return 1.0f;
        case EGSDCrowdQuality::Medium: return 2.0f;
        case EGSDCrowdQuality::High: return 3.0f;
        case EGSDCrowdQuality::Epic: return 4.0f;
        default: return 2.0f;
        }
    }
};

/**
 * Performance budget tracking result.
 */
USTRUCT(BlueprintType)
struct GSD_CROWDS_API FGSDCrowdBudgetResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Result")
    bool bWithinBudget = true;

    UPROPERTY(BlueprintReadOnly, Category = "Result")
    float ElapsedTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Result")
    float BudgetMs = 2.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Result")
    float UtilizationPercent = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Result")
    bool bShouldEarlyOut = false;
};

/**
 * Delegate for budget exceeded warnings.
 */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnBudgetExceeded, float, ElapsedMs, float, BudgetMs);

/**
 * World subsystem for tracking and enforcing crowd performance budgets.
 *
 * Provides:
 * - Frame time budget enforcement with early-out
 * - Automatic quality scaling based on performance
 * - Entity count limits per quality tier
 * - Budget utilization metrics
 */
UCLASS()
class GSD_CROWDS_API UGSDCrowdPerformanceBudget : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    //-- Budget Checking --

    /**
     * Start a budget scope. Call before processing begins.
     * @param Category Name of the budget category (e.g., "CrowdAI", "LODUpdate")
     * @return Unique scope ID for EndBudgetScope
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Performance")
    int32 BeginBudgetScope(const FString& Category);

    /**
     * End a budget scope and check if exceeded.
     * @param ScopeId The scope ID from BeginBudgetScope
     * @return Budget result with timing and early-out recommendation
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Performance")
    FGSDCrowdBudgetResult EndBudgetScope(int32 ScopeId);

    /**
     * Check if processing should early-out due to budget exhaustion.
     * Call periodically during long operations.
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds|Performance")
    bool ShouldEarlyOut() const;

    //-- Entity Limits --

    /**
     * Check if we can spawn more entities within budget.
     * @param AdditionalCount Number of entities to potentially spawn
     * @return True if spawn would be within limits
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds|Performance")
    bool CanSpawnEntities(int32 AdditionalCount) const;

    /**
     * Get maximum allowed entities for current quality tier.
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds|Performance")
    int32 GetMaxEntities() const { return Config.GetMaxEntities(); }

    /**
     * Get current entity count (set by CrowdManager).
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds|Performance")
    int32 GetCurrentEntityCount() const { return CurrentEntityCount; }

    /**
     * Set current entity count (called by CrowdManager).
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Performance")
    void SetCurrentEntityCount(int32 Count) { CurrentEntityCount = Count; }

    //-- Quality Control --

    /**
     * Get current quality tier.
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds|Performance")
    EGSDCrowdQuality GetCurrentQuality() const { return Config.CurrentQuality; }

    /**
     * Set quality tier manually.
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Performance")
    void SetQuality(EGSDCrowdQuality NewQuality);

    /**
     * Get performance configuration.
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds|Performance")
    const FGSDCrowdPerformanceConfig& GetConfig() const { return Config; }

    //-- Auto-Quality Scaling --

    /**
     * Update auto-quality scaling based on recent frame times.
     * Call once per frame.
     */
    void UpdateAutoQualityScaling();

    /**
     * Enable/disable auto-quality scaling.
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Performance")
    void SetAutoQualityScaling(bool bEnabled) { Config.bEnableAutoQualityScaling = bEnabled; }

    //-- Delegates --

    /**
     * Get delegate for budget exceeded events.
     */
    FOnBudgetExceeded& GetOnBudgetExceeded() { return BudgetExceededDelegate; }

protected:
    //-- Configuration --
    UPROPERTY(EditDefaultsOnly, Category = "Config")
    FGSDCrowdPerformanceConfig Config;

    //-- Budget Tracking --
    TMap<int32, double> ActiveScopes;
    int32 NextScopeId = 1;

    //-- Entity Tracking --
    int32 CurrentEntityCount = 0;

    //-- Auto-Scaling State --
    int32 ConsecutiveSlowFrames = 0;
    int32 ConsecutiveFastFrames = 0;

    //-- Delegates --
    FOnBudgetExceeded BudgetExceededDelegate;

    //-- Frame Time History --
    TArray<float> FrameTimeHistory;
    static constexpr int32 FrameTimeHistorySize = 60;

    //-- UWorldSubsystem Interface --
    virtual bool ShouldCreateSubsystem(UWorld* World) const override;
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
};
