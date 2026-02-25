#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "GSDLog.h"

#define LOCTEXT_NAMESPACE "FGSD_CoreModule"

class FGSD_CoreModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        GSD_LOG(Log, TEXT("GSD_Core module starting up"));
    }

    virtual void ShutdownModule() override
    {
        GSD_LOG(Log, TEXT("GSD_Core module shutting down"));
    }
};

IMPLEMENT_MODULE(FGSD_CoreModule, GSD_Core)

#undef LOCTEXT_NAMESPACE
