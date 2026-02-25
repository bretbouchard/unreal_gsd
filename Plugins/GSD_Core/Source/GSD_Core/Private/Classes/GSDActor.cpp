#include "Classes/GSDActor.h"
#include "DataAssets/GSDDataAsset.h"
#include "GSDLog.h"

AGSDActor::AGSDActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AGSDActor::SpawnAsync(UGSDDataAsset* Config)
{
    GSDConfig = Config;
    bIsSpawned = true;

    OnConfigApplied(Config);
    OnSpawnComplete();

    UE_LOG(LogGSD, Log, TEXT("AGSDActor %s spawned with config %s"),
        *GetName(), Config ? *Config->GetName() : TEXT("nullptr"));
}

UGSDDataAsset* AGSDActor::GetSpawnConfig()
{
    return GSDConfig;
}

bool AGSDActor::IsSpawned()
{
    return bIsSpawned;
}

void AGSDActor::Despawn()
{
    OnDespawnStart();
    bIsSpawned = false;
    UE_LOG(LogGSD, Log, TEXT("AGSDActor %s despawned"), *GetName());
}

void AGSDActor::ResetSpawnState()
{
    bIsSpawned = false;
    GSDConfig = nullptr;
}

FBoxSphereBounds AGSDActor::GetStreamingBounds_Implementation()
{
    FVector Origin;
    FVector BoxExtent;
    GetActorBounds(true, Origin, BoxExtent);
    return FBoxSphereBounds(FBox(Origin - BoxExtent, Origin + BoxExtent));
}

void AGSDActor::OnStreamIn_Implementation()
{
    bIsStreamedIn = true;
    UE_LOG(LogGSD, Verbose, TEXT("AGSDActor %s streamed in"), *GetName());
}

void AGSDActor::OnStreamOut_Implementation()
{
    bIsStreamedIn = false;
    UE_LOG(LogGSD, Verbose, TEXT("AGSDActor %s streamed out"), *GetName());
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
