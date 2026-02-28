#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GSDCrowdHLODConfig.generated.h"

class UStaticMesh;
class UMaterialInterface;

/**
 * Configuration for crowd HLOD system.
 * Defines distances, clustering parameters, and visual settings for distant crowd proxies.
 */
UCLASS(BlueprintType)
class GSD_CROWDS_API UGSDCrowdHLODConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    // Distance at which to start showing HLOD proxies
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Distances")
    float HLODStartDistance = 3000.0f;

    // Distance at which only HLOD proxies are shown (entities culled)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Distances")
    float HLODOnlyDistance = 5000.0f;

    // Size of HLOD clusters in world units
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Clustering", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
    float ClusterSize = 1000.0f;

    // Maximum number of HLOD proxies to create
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Clustering", meta = (ClampMin = "10", ClampMax = "500"))
    int32 MaxProxies = 100;

    // Minimum entities per cluster to create a proxy
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Clustering", meta = (ClampMin = "1", ClampMax = "50"))
    int32 MinEntitiesPerCluster = 5;

    // Proxy mesh to use for impostors
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
    TObjectPtr<UStaticMesh> ProxyMesh = nullptr;

    // Proxy material override (optional)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
    TObjectPtr<UMaterialInterface> ProxyMaterial = nullptr;

    // Whether to fade between LOD and HLOD
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transitions")
    bool bUseFadeTransitions = true;

    // Fade duration in seconds
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transitions", meta = (ClampMin = "0.1", ClampMax = "2.0", EditCondition = "bUseFadeTransitions"))
    float FadeDuration = 0.5f;

    // Validate configuration
    bool ValidateConfig(FString& OutError) const;
};
