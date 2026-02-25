#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GSDCityStreamingSettings.generated.h"

/**
 * UGSDCityStreamingSettings
 *
 * Centralized configuration for World Partition grid settings used in Charlotte city streaming.
 * Access via Project Settings > GSD City Streaming.
 *
 * Grid Cell Size: 25600 UU (256m) - optimized for urban density
 * Loading Range: 76800 UU (768m) - smooth driving experience
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="GSD City Streaming"))
class GSD_CITYSTREAMING_API UGSDCityStreamingSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    /** Returns the singleton instance of the settings */
    static UGSDCityStreamingSettings* Get() { return GetDefault<UGSDCityStreamingSettings>(); }

    // ============================================
    // World Partition Grid Settings
    // ============================================

    /** Default grid cell size in unreal units. 25600 UU = 256 meters for urban density */
    UPROPERTY(EditAnywhere, Config, Category="World Partition", meta=(ClampMin=12800, ClampMax=102400, DisplayName="Grid Cell Size (UU)"))
    int32 DefaultGridCellSize = 25600;

    /** Default loading range in unreal units. 76800 UU = 768 meters for smooth driving */
    UPROPERTY(EditAnywhere, Config, Category="World Partition", meta=(ClampMin=25600, ClampMax=204800, DisplayName="Loading Range (UU)"))
    int32 DefaultLoadingRange = 76800;

    // ============================================
    // HLOD Settings
    // ============================================

    /** Automatically generate HLODs on level save */
    UPROPERTY(EditAnywhere, Config, Category="HLOD")
    bool bAutoGenerateHLODs = true;

    /** Number of HLOD tiers to generate */
    UPROPERTY(EditAnywhere, Config, Category="HLOD", meta=(ClampMin=1, ClampMax=5, EditCondition="bAutoGenerateHLODs"))
    int32 HLODTierCount = 3;

    /** Path where HLOD assets are stored */
    UPROPERTY(EditAnywhere, Config, Category="HLOD", meta=(ContentDir))
    FString HLODBuildPath = TEXT("/Game/HLOD");

    // ============================================
    // Import Settings
    // ============================================

    /** Automatically generate collision for imported meshes */
    UPROPERTY(EditAnywhere, Config, Category="Import")
    bool bAutoGenerateCollision = true;

    /** Scale multiplier for imported geometry */
    UPROPERTY(EditAnywhere, Config, Category="Import", meta=(ClampMin=0.01, ClampMax=100.0))
    float ImportScale = 1.0f;

    /** Preferred import format for city geometry */
    UPROPERTY(EditAnywhere, Config, Category="Import")
    FString PreferredImportFormat = TEXT("USD");
};
