// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Subsystems/GSDCrowdManagerSubsystem.h"
#include "GSDCrowdDebugWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UVerticalBox;

/**
 * UMG debug dashboard widget for live crowd monitoring.
 *
 * Displays:
 * - Entity counts (total entities, active crowds)
 * - LOD distribution (progress bars for each LOD level)
 * - Performance metrics (frame time, draw calls, memory)
 * - Color-coded performance indicator
 *
 * Usage:
 * 1. Create Blueprint widget extending this class
 * 2. Add named widgets matching BindWidget meta
 * 3. Call BindToCrowdManager() after spawning
 * 4. Call RemoveFromParent() / UnbindFromCrowdManager() when done
 */
UCLASS(Abstract)
class GSD_CROWDS_API UGSDCrowdDebugWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UGSDCrowdDebugWidget(const FObjectInitializer& ObjectInitializer);

    //-- Lifecycle --

    /**
     * Bind to crowd manager for live metrics updates.
     * Starts metrics broadcasting if not already active.
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Debug")
    void BindToCrowdManager();

    /**
     * Unbind from crowd manager.
     * Stops metrics broadcasting if no other widgets are bound.
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Debug")
    void UnbindFromCrowdManager();

    //-- Widget Binding (Blueprint must have these named widgets) --

    // Entity Counts
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TotalEntitiesText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ActiveCrowdsText;

    // LOD Distribution
    UPROPERTY(meta = (BindWidget))
    UProgressBar* LOD0Bar;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* LOD1Bar;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* LOD2Bar;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* LOD3Bar;

    // Performance
    UPROPERTY(meta = (BindWidget))
    UTextBlock* FrameTimeText;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* PerformanceBar;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* MemoryText;

    UPROPERTY(meta = (BindWidget))
    UVerticalBox* WarningsBox;

protected:
    //-- UUserWidget Interface --
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    //-- End of UUserWidget Interface --

private:
    /**
     * Handle metrics update from crowd manager.
     * @param Metrics Updated metrics snapshot
     */
    UFUNCTION()
    void OnCrowdMetricsUpdated(const FGSDCrowdMetrics& Metrics);

    /**
     * Update all UI elements with new metrics.
     * @param Metrics Metrics to display
     */
    void UpdateUI(const FGSDCrowdMetrics& Metrics);

    /**
     * Format number with K/M suffix for large values.
     * @param Value Number to format
     * @return Formatted string (e.g., "1.2K", "3.4M")
     */
    FString FormatNumber(int32 Value) const;

    /**
     * Format time in milliseconds.
     * @param Seconds Time in seconds
     * @return Formatted string (e.g., "16.67ms")
     */
    FString FormatTime(float Seconds) const;

    // Cached crowd manager reference
    UPROPERTY()
    UGSDCrowdManagerSubsystem* CachedCrowdManager;

    // Delegate handle for cleanup
    FOnCrowdMetricsUpdated MetricsDelegate;
};
