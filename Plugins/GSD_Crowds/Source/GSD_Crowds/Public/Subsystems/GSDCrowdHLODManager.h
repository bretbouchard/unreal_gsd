#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "MassEntityHandle.h"
#include "GSDCrowdHLODManager.generated.h"

class AGSDCrowdHLODProxy;
class UStaticMesh;

USTRUCT()
struct FGSDCrowdCluster
{
    GENERATED_BODY()

    UPROPERTY()
    FVector Center = FVector::ZeroVector;

    UPROPERTY()
    int32 EntityCount = 0;

    UPROPERTY()
    float Radius = 100.0f;

    UPROPERTY()
    TArray<FMassEntityHandle> Entities;
};

/**
 * Manages HLOD proxy creation, updates, and lifecycle.
 * Clusters distant crowd entities and represents them as simplified proxy actors.
 */
UCLASS()
class GSD_CROWDS_API UGSDCrowdHLODManager : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Create HLOD proxy for a cluster
    AGSDCrowdHLODProxy* CreateHLODProxy(const FGSDCrowdCluster& Cluster, UWorld* World);

    // Update all proxy visibilities based on player location
    void UpdateProxyVisibilities(const FVector& PlayerLocation);

    // Set HLOD threshold distance
    void SetHLODThreshold(float InThreshold) { HLODThreshold = InThreshold; }

    // Get HLOD threshold
    float GetHLODThreshold() const { return HLODThreshold; }

    // Remove all proxies for a streaming cell
    void RemoveProxiesForCell(FName CellName);

    // Cluster entities for HLOD and create proxies
    void ClusterEntitiesForHLOD(const TArray<FMassEntityHandle>& Entities, const TArray<FVector>& Positions, UWorld* World);

    // Get active proxy count
    int32 GetActiveProxyCount() const { return ActiveProxies.Num(); }

    // Set proxy mesh to use for all new proxies
    void SetProxyMesh(UStaticMesh* InMesh) { ProxyMesh = InMesh; }

    // Clear all active proxies
    void ClearAllProxies();

protected:
    // All active HLOD proxies
    UPROPERTY()
    TArray<TObjectPtr<AGSDCrowdHLODProxy>> ActiveProxies;

    // Proxy mesh to use
    UPROPERTY()
    TObjectPtr<UStaticMesh> ProxyMesh = nullptr;

    // Distance at which to show HLOD proxies
    UPROPERTY(Config)
    float HLODThreshold = 5000.0f;

    // Cluster size for HLOD grid
    UPROPERTY(Config)
    float ClusterSize = 1000.0f;

    // Maximum proxies to create
    UPROPERTY(Config)
    int32 MaxProxies = 100;
};
