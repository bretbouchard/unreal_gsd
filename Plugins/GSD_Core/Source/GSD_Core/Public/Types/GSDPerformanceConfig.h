#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GSDPerformanceConfig.generated.h"

/**
 * Performance budget configuration for the GSD platform.
 * Access via GetDefault<UGSDPerformanceConfig>().
 */
UCLASS(Config = Game, DefaultConfig, Category = "GSD")
class GSD_CORE_API UGSDPerformanceConfig : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UGSDPerformanceConfig();

    // ~ Spawning budgets
    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning",
        meta = (ClampMin = "0.1", ClampMax = "10.0", Units = "ms"))
    float SpawningFrameBudgetMs = 2.0f;

    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning",
        meta = (ClampMin = "1", ClampMax = "50"))
    int32 MaxSpawnsPerFrame = 10;

    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning",
        meta = (ClampMin = "1", ClampMax = "1000"))
    int32 BatchSpawnChunkSize = 20;

    // ~ Memory budgets
    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Memory",
        meta = (ClampMin = "64", ClampMax = "4096", Units = "MB"))
    float EntityMemoryBudgetMB = 512.0f;

    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Memory",
        meta = (ClampMin = "10", ClampMax = "500"))
    int32 MaxPooledEntities = 100;

    // ~ Audio budgets
    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Audio",
        meta = (ClampMin = "0.1", ClampMax = "10.0", Units = "ms"))
    float AudioBudgetMs = 2.0f;

    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Audio",
        meta = (ClampMin = "8", ClampMax = "128"))
    int32 MaxConcurrentAudioSources = 32;

    // ~ Hitch thresholds
    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Performance",
        meta = (ClampMin = "8.0", ClampMax = "33.0", Units = "ms"))
    float HitchThresholdMs = 16.67f; // 60fps target

    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Performance",
        meta = (ClampMin = "30", ClampMax = "120"))
    float TargetFPS = 60.0f;

    // ~ Debug settings
    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
    bool bLogSpawnPerformance = false;

    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
    bool bLogMemoryUsage = false;

    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
    bool bWarnOnBudgetExceeded = true;

    // Helper functions
    UFUNCTION(BlueprintPure, Category = "GSD|Performance")
    bool IsWithinSpawnBudget(float CurrentFrameTimeMs) const;

    UFUNCTION(BlueprintPure, Category = "GSD|Performance")
    bool IsHitch(float FrameTimeMs) const;

    UFUNCTION(BlueprintPure, Category = "GSD|Performance")
    float GetFrameBudgetRemaining(float CurrentFrameTimeMs) const;
};
