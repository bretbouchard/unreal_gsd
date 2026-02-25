#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "GSDCityStreamingSettings.h"

// Forward declaration for GSD_Core logging
GSD_CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogGSD, Log, All);

#define LOCTEXT_NAMESPACE "FGSD_CityStreamingModule"

/**
 * GSD_CityStreaming Module
 *
 * Provides World Partition configuration for Charlotte city streaming.
 * This plugin depends on GSD_Core for shared logging infrastructure.
 *
 * Key settings:
 * - Grid Cell Size: 25600 UU (256m) for urban density
 * - Loading Range: 76800 UU (768m) for smooth driving gameplay
 */
class FGSD_CityStreamingModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        UE_LOG(LogGSD, Log, TEXT("GSD_CityStreaming module started"));

        // Log configured settings to confirm plugin loaded correctly
        if (const UGSDCityStreamingSettings* Settings = UGSDCityStreamingSettings::Get())
        {
            UE_LOG(LogGSD, Log, TEXT("  Grid Cell Size: %d UU"), Settings->DefaultGridCellSize);
            UE_LOG(LogGSD, Log, TEXT("  Loading Range: %d UU"), Settings->DefaultLoadingRange);
            UE_LOG(LogGSD, Log, TEXT("  Auto HLOD: %s"), Settings->bAutoGenerateHLODs ? TEXT("true") : TEXT("false"));
        }
    }

    virtual void ShutdownModule() override
    {
        UE_LOG(LogGSD, Log, TEXT("GSD_CityStreaming module shut down"));
    }
};

IMPLEMENT_MODULE(FGSD_CityStreamingModule, GSD_CityStreaming)

#undef LOCTEXT_NAMESPACE
