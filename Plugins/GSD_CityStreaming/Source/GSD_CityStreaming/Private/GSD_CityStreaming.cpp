#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// Forward declaration for GSD_Core logging
GSD_CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogGSD, Log, All);

#define LOCTEXT_NAMESPACE "FGSD_CityStreamingModule"

/**
 * GSD_CityStreaming Module
 *
 * Provides World Partition configuration for Charlotte city streaming.
 * This plugin depends on GSD_Core for shared logging infrastructure.
 */
class FGSD_CityStreamingModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        UE_LOG(LogGSD, Log, TEXT("GSD_CityStreaming module starting up"));
    }

    virtual void ShutdownModule() override
    {
        UE_LOG(LogGSD, Log, TEXT("GSD_CityStreaming module shutting down"));
    }
};

IMPLEMENT_MODULE(FGSD_CityStreamingModule, GSD_CityStreaming)

#undef LOCTEXT_NAMESPACE
