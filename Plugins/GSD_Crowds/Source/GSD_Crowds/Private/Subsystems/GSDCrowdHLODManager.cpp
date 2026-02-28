#include "Subsystems/GSDCrowdHLODManager.h"
#include "HLOD/GSDCrowdHLODProxy.h"
#include "Engine/World.h"

void UGSDCrowdHLODManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    ActiveProxies.Empty();
}

void UGSDCrowdHLODManager::Deinitialize()
{
    ClearAllProxies();

    Super::Deinitialize();
}

void UGSDCrowdHLODManager::ClearAllProxies()
{
    for (AGSDCrowdHLODProxy* Proxy : ActiveProxies)
    {
        if (Proxy)
        {
            Proxy->Destroy();
        }
    }
    ActiveProxies.Empty();
}

void UGSDCrowdHLODManager::ClusterEntitiesForHLOD(const TArray<FMassEntityHandle>& Entities, const TArray<FVector>& Positions, UWorld* World)
{
    if (!World || Entities.Num() == 0 || Positions.Num() != Entities.Num())
    {
        return;
    }

    // Clear existing proxies
    ClearAllProxies();

    // Simple grid-based clustering
    TMap<FIntVector, FGSDCrowdCluster> Clusters;

    for (int32 i = 0; i < Entities.Num(); i++)
    {
        const FVector& Pos = Positions[i];
        FIntVector ClusterKey(
            FMath::FloorToInt(Pos.X / ClusterSize),
            FMath::FloorToInt(Pos.Y / ClusterSize),
            0
        );

        FGSDCrowdCluster& Cluster = Clusters.FindOrAdd(ClusterKey);
        Cluster.Entities.Add(Entities[i]);
        Cluster.EntityCount++;
    }

    // Calculate cluster centers and radii
    for (auto& Pair : Clusters)
    {
        FGSDCrowdCluster& Cluster = Pair.Value;

        // Calculate cluster center
        FVector Sum = FVector::ZeroVector;
        for (int32 i = 0; i < Cluster.Entities.Num(); i++)
        {
            // Find index in original arrays
            int32 EntityIndex = Entities.Find(Cluster.Entities[i]);
            if (EntityIndex != INDEX_NONE)
            {
                Sum += Positions[EntityIndex];
            }
        }
        Cluster.Center = Sum / Cluster.EntityCount;

        // Calculate radius
        float MaxDist = 0.0f;
        for (int32 i = 0; i < Cluster.Entities.Num(); i++)
        {
            int32 EntityIndex = Entities.Find(Cluster.Entities[i]);
            if (EntityIndex != INDEX_NONE)
            {
                float Dist = FVector::Dist(Cluster.Center, Positions[EntityIndex]);
                MaxDist = FMath::Max(MaxDist, Dist);
            }
        }
        Cluster.Radius = MaxDist;

        // Create proxy
        CreateHLODProxy(Cluster, World);
    }
}

AGSDCrowdHLODProxy* UGSDCrowdHLODManager::CreateHLODProxy(const FGSDCrowdCluster& Cluster, UWorld* World)
{
    if (!World || ActiveProxies.Num() >= MaxProxies)
    {
        return nullptr;
    }

    FActorSpawnParams SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AGSDCrowdHLODProxy* Proxy = World->SpawnActor<AGSDCrowdHLODProxy>(
        AGSDCrowdHLODProxy::StaticClass(),
        Cluster.Center,
        FRotator::ZeroRotator,
        SpawnParams
    );

    if (Proxy)
    {
        Proxy->InitializeCluster(Cluster.Center, Cluster.EntityCount, Cluster.Radius);

        if (ProxyMesh)
        {
            Proxy->SetProxyMesh(ProxyMesh);
        }

        ActiveProxies.Add(Proxy);
    }

    return Proxy;
}

void UGSDCrowdHLODManager::UpdateProxyVisibilities(const FVector& PlayerLocation)
{
    for (AGSDCrowdHLODProxy* Proxy : ActiveProxies)
    {
        if (Proxy)
        {
            Proxy->UpdateVisibility(PlayerLocation, HLODThreshold);
        }
    }
}

void UGSDCrowdHLODManager::RemoveProxiesForCell(FName CellName)
{
    // TODO(GSDCROWDS-113): Implement cell-based proxy removal when streaming cell integration is complete
    // For now, this is a placeholder for future streaming integration
}
