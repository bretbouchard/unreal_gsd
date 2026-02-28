// Copyright Bret Bouchard. All Rights Reserved.

#include "UI/GSDCrowdDebugWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "Subsystems/GSDCrowdManagerSubsystem.h"
#include "GSDCrowdLog.h"

UGSDCrowdDebugWidget::UGSDCrowdDebugWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , TotalEntitiesText(nullptr)
    , ActiveCrowdsText(nullptr)
    , LOD0Bar(nullptr)
    , LOD1Bar(nullptr)
    , LOD2Bar(nullptr)
    , LOD3Bar(nullptr)
    , FrameTimeText(nullptr)
    , PerformanceBar(nullptr)
    , MemoryText(nullptr)
    , WarningsBox(nullptr)
    , CachedCrowdManager(nullptr)
{
}

void UGSDCrowdDebugWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Auto-bind to crowd manager
    BindToCrowdManager();
}

void UGSDCrowdDebugWidget::NativeDestruct()
{
    // Auto-unbind from crowd manager
    UnbindFromCrowdManager();

    Super::NativeDestruct();
}

void UGSDCrowdDebugWidget::BindToCrowdManager()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LOG_GSDCROWDS, Warning, TEXT("GSDCrowdDebugWidget: No world context"));
        return;
    }

    CachedCrowdManager = World->GetSubsystem<UGSDCrowdManagerSubsystem>();
    if (!CachedCrowdManager)
    {
        UE_LOG(LOG_GSDCROWDS, Warning, TEXT("GSDCrowdDebugWidget: Crowd manager not found"));
        return;
    }

    // Bind to metrics updates
    MetricsDelegate.BindDynamic(this, &UGSDCrowdDebugWidget::OnCrowdMetricsUpdated);
    CachedCrowdManager->GetOnCrowdMetricsUpdated().Add(MetricsDelegate);

    // Start metrics broadcasting
    CachedCrowdManager->StartMetricsUpdates();

    UE_LOG(LOG_GSDCROWDS, Log, TEXT("GSDCrowdDebugWidget: Bound to crowd manager"));
}

void UGSDCrowdDebugWidget::UnbindFromCrowdManager()
{
    if (CachedCrowdManager)
    {
        // Unbind from metrics updates
        CachedCrowdManager->GetOnCrowdMetricsUpdated().Remove(MetricsDelegate);

        // Stop metrics broadcasting if no other widgets are bound
        if (!CachedCrowdManager->GetOnCrowdMetricsUpdated().IsBound())
        {
            CachedCrowdManager->StopMetricsUpdates();
        }

        UE_LOG(LOG_GSDCROWDS, Log, TEXT("GSDCrowdDebugWidget: Unbound from crowd manager"));
    }

    CachedCrowdManager = nullptr;
}

void UGSDCrowdDebugWidget::OnCrowdMetricsUpdated(const FGSDCrowdMetrics& Metrics)
{
    UpdateUI(Metrics);
}

void UGSDCrowdDebugWidget::UpdateUI(const FGSDCrowdMetrics& Metrics)
{
    //-- Entity Counts --
    if (TotalEntitiesText)
    {
        TotalEntitiesText->SetText(FText::FromString(FormatNumber(Metrics.TotalEntities)));
    }

    if (ActiveCrowdsText)
    {
        ActiveCrowdsText->SetText(FText::FromString(FString::FromInt(Metrics.ActiveCrowds)));
    }

    //-- LOD Distribution --
    const int32 TotalEntities = FMath::Max(1, Metrics.TotalEntities);  // Avoid divide by zero

    if (LOD0Bar)
    {
        const float Percent = static_cast<float>(Metrics.LOD0Count) / TotalEntities;
        LOD0Bar->SetPercent(Percent);
    }

    if (LOD1Bar)
    {
        const float Percent = static_cast<float>(Metrics.LOD1Count) / TotalEntities;
        LOD1Bar->SetPercent(Percent);
    }

    if (LOD2Bar)
    {
        const float Percent = static_cast<float>(Metrics.LOD2Count) / TotalEntities;
        LOD2Bar->SetPercent(Percent);
    }

    if (LOD3Bar)
    {
        const float Percent = static_cast<float>(Metrics.LOD3Count) / TotalEntities;
        LOD3Bar->SetPercent(Percent);
    }

    //-- Performance Metrics --
    if (FrameTimeText)
    {
        FrameTimeText->SetText(FText::FromString(FormatTime(Metrics.LastFrameTime)));
    }

    if (PerformanceBar)
    {
        // Performance indicator: 0-16.67ms = green (0-100%), 16.67-33.33ms = yellow, >33.33ms = red
        const float TargetFrameTime = 1.0f / 60.0f;  // 60 FPS target
        const float Percent = FMath::Clamp(Metrics.LastFrameTime / TargetFrameTime, 0.0f, 2.0f);

        // Map to progress bar percent (0-100%)
        const float BarPercent = FMath::Clamp(1.0f - (Percent - 1.0f), 0.0f, 1.0f);
        PerformanceBar->SetPercent(BarPercent);

        // Color coding: Green (good) -> Yellow (warning) -> Red (bad)
        FLinearColor BarColor;
        if (Percent <= 1.0f)
        {
            // Under 16.67ms - Green (good)
            BarColor = FLinearColor::Green;
        }
        else if (Percent <= 1.5f)
        {
            // 16.67-25ms - Yellow (warning)
            BarColor = FLinearColor::Yellow;
        }
        else
        {
            // Over 25ms - Red (bad)
            BarColor = FLinearColor::Red;
        }

        // Set bar color (requires setting fill color in Blueprint)
        // Note: Actual color setting requires dynamic material or style customization in Blueprint
    }

    if (MemoryText)
    {
        MemoryText->SetText(FText::FromString(FString::Printf(TEXT("%.2f MB"), Metrics.MemoryUsedMB)));
    }

    //-- Warnings Box (show if performance issues) --
    if (WarningsBox)
    {
        const float TargetFrameTime = 1.0f / 60.0f;
        const bool bPerformanceWarning = Metrics.LastFrameTime > (TargetFrameTime * 1.5f);  // Under 40 FPS
        WarningsBox->SetVisibility(bPerformanceWarning ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

FString UGSDCrowdDebugWidget::FormatNumber(int32 Value) const
{
    if (Value >= 1000000)
    {
        return FString::Printf(TEXT("%.1fM"), Value / 1000000.0f);
    }
    else if (Value >= 1000)
    {
        return FString::Printf(TEXT("%.1fK"), Value / 1000.0f);
    }
    else
    {
        return FString::FromInt(Value);
    }
}

FString UGSDCrowdDebugWidget::FormatTime(float Seconds) const
{
    return FString::Printf(TEXT("%.2fms"), Seconds * 1000.0f);
}
