#include "HLOD/GSDCrowdHLODProxy.h"
#include "Components/StaticMeshComponent.h"

AGSDCrowdHLODProxy::AGSDCrowdHLODProxy()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create impostor mesh component
    ImpostorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ImpostorMesh"));
    RootComponent = ImpostorMesh;

    // Configure for HLOD
    ImpostorMesh->SetCastShadow(false);
    ImpostorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ImpostorMesh->SetCanEverAffectNavigation(false);
}

void AGSDCrowdHLODProxy::BeginPlay()
{
    Super::BeginPlay();
}

void AGSDCrowdHLODProxy::InitializeCluster(const FVector& InCenter, int32 InEntityCount, float InRadius)
{
    ClusterCenter = InCenter;
    EntityCount = InEntityCount;
    ClusterRadius = InRadius;

    // Position proxy at cluster center
    SetActorLocation(ClusterCenter);

    // Scale based on entity count
    float Scale = FMath::Sqrt(static_cast<float>(EntityCount)) * 0.5f;
    SetActorScale3D(FVector(Scale, Scale, 1.0f));
}

void AGSDCrowdHLODProxy::UpdateVisibility(const FVector& PlayerLocation, float HLODThreshold)
{
    float Distance = FVector::Dist(PlayerLocation, ClusterCenter);
    bool bShouldBeVisible = Distance >= HLODThreshold;

    if (bShouldBeVisible != bIsVisible)
    {
        bIsVisible = bShouldBeVisible;
        ImpostorMesh->SetVisibility(bIsVisible);
    }
}

void AGSDCrowdHLODProxy::SetProxyMesh(UStaticMesh* InMesh)
{
    if (ImpostorMesh && InMesh)
    {
        ImpostorMesh->SetStaticMesh(InMesh);
    }
}
