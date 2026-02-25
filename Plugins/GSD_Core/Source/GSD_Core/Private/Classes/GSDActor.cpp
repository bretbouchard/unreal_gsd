#include "Classes/GSDActor.h"
#include "DataAssets/GSDDataAsset.h"
#include "GSDLog.h"

AGSDActor::AGSDActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AGSDActor::SpawnAsync_Implementation(UGSDDataAsset* Config, FOnSpawnComplete Callback)
{
    GSDConfig = Config;
    bIsSpawned = true;

    OnConfigApplied(Config);
    OnSpawnComplete();

    GSD_LOG(Log, "AGSDActor %s spawned with config %s",
        *GetName(), Config ? *Config->GetName() : TEXT("nullptr"));

    if (Callback.IsBound())
    {
        Callback.Execute(this);
    }
}

UGSDDataAsset* AGSDActor::GetSpawnConfig_Implementation()
{
    return GSDConfig;
}

bool AGSDActor::IsSpawned_Implementation()
{
    return bIsSpawned;
}

void AGSDActor::Despawn_Implementation()
{
    OnDespawnStart();
    bIsSpawned = false;
    GSD_LOG(Log, "AGSDActor %s despawned", *GetName());
}

void AGSDActor::ResetSpawnState_Implementation()
{
    bIsSpawned = false;
    GSDConfig = nullptr;
}

FBoxSphereBounds AGSDActor::GetStreamingBounds_Implementation()
{
    return FBoxSphereBounds(GetActorBounds(true));
}

void AGSDActor::OnStreamIn_Implementation()
{
    bIsStreamedIn = true;
    GSD_LOG(Verbose, "AGSDActor %s streamed in", *GetName());
}

void AGSDActor::OnStreamOut_Implementation()
{
    bIsStreamedIn = false;
    GSD_LOG(Verbose, "AGSDActor %s streamed out", *GetName());
}

int32 AGSDActor::GetStreamingPriority_Implementation()
{
    return 0;
}

bool AGSDActor::ShouldPersist_Implementation()
{
    return false;
}

void AGSDActor::OnConfigApplied_Implementation(UGSDDataAsset* Config)
{
    // Override in derived classes
}

void AGSDActor::OnSpawnComplete_Implementation()
{
    // Override in derived classes
}

void AGSDActor::OnDespawnStart_Implementation()
{
    // Override in derived classes
}
