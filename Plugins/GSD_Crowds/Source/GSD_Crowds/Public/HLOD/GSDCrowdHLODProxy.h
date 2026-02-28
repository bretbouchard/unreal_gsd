#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GSDCrowdHLODProxy.generated.h"

class UStaticMeshComponent;
class UInstancedStaticMeshComponent;

/**
 * HLOD proxy actor representing a cluster of distant crowd entities.
 * Replaces individual entity rendering with simplified impostor/billboard.
 */
UCLASS()
class GSD_CROWDS_API AGSDCrowdHLODProxy : public AActor
{
    GENERATED_BODY()

public:
    AGSDCrowdHLODProxy();

    // Initialize proxy with cluster data
    void InitializeCluster(const FVector& InCenter, int32 InEntityCount, float InRadius);

    // Update visibility based on distance to player
    void UpdateVisibility(const FVector& PlayerLocation, float HLODThreshold);

    // Get cluster data
    const FVector& GetClusterCenter() const { return ClusterCenter; }
    int32 GetEntityCount() const { return EntityCount; }
    float GetClusterRadius() const { return ClusterRadius; }

    // Set the proxy mesh
    void SetProxyMesh(UStaticMesh* InMesh);

    // Check if proxy is currently visible
    bool IsVisible() const { return bIsVisible; }

protected:
    virtual void BeginPlay() override;

    // Impostor/billboard mesh component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* ImpostorMesh;

    // Cluster data
    UPROPERTY()
    FVector ClusterCenter = FVector::ZeroVector;

    UPROPERTY()
    int32 EntityCount = 0;

    UPROPERTY()
    float ClusterRadius = 100.0f;

    // Whether currently visible
    bool bIsVisible = true;

    // Distance at which this proxy should be shown
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float ShowDistance = 3000.0f;
};
