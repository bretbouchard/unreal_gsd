#include "Actors/GSDCityLevelActor.h"
#include "DataAssets/GSDHLODConfigAsset.h"

// Forward declaration for GSD_Core logging
GSD_CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogGSD, Log, All);

AGSDCityLevelActor::AGSDCityLevelActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // This actor is purely for configuration, no visual representation
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
}

void AGSDCityLevelActor::BeginPlay()
{
    Super::BeginPlay();
    LogConfiguration();
}

void AGSDCityLevelActor::LogConfiguration()
{
    UE_LOG(LogGSD, Log, TEXT("GSDCityLevelActor: World Partition Configuration"));
    UE_LOG(LogGSD, Log, TEXT("  Grid Cell Size: %d UU (%.1f m)"), GridCellSize, GridCellSize / 100.0f);
    UE_LOG(LogGSD, Log, TEXT("  Loading Range: %d UU (%.1f m)"), LoadingRange, LoadingRange / 100.0f);
    UE_LOG(LogGSD, Log, TEXT("  Predictive Loading: %s"), bEnablePredictiveLoading ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogGSD, Log, TEXT("  Block on Slow Streaming: %s"), bBlockOnSlowStreaming ? TEXT("Yes") : TEXT("No"));

    if (HLODConfig)
    {
        UE_LOG(LogGSD, Log, TEXT("  HLOD Config: %s"), *HLODConfig->GetName());
    }
    else
    {
        UE_LOG(LogGSD, Log, TEXT("  HLOD Config: None (using defaults)"));
    }
}
