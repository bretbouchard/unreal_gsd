#include "Types/GSDPerformanceConfig.h"

UGSDPerformanceConfig::UGSDPerformanceConfig()
{
    CategoryName = TEXT("GSD");
    SectionName = TEXT("Performance");
}

bool UGSDPerformanceConfig::IsWithinSpawnBudget(float CurrentFrameTimeMs) const
{
    return CurrentFrameTimeMs <= SpawningFrameBudgetMs;
}

bool UGSDPerformanceConfig::IsHitch(float FrameTimeMs) const
{
    return FrameTimeMs > HitchThresholdMs;
}

float UGSDPerformanceConfig::GetFrameBudgetRemaining(float CurrentFrameTimeMs) const
{
    return FMath::Max(0.0f, SpawningFrameBudgetMs - CurrentFrameTimeMs);
}
