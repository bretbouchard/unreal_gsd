#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataLayer.h"
#include "GSDDataLayerConfig.generated.h"

/**
 * Configuration asset for Data Layer system.
 * Provides data-driven configuration for runtime Data Layer control.
 *
 * Create instances of this asset in the editor to configure which
 * Data Layers are available and how they should be activated.
 */
UCLASS(BlueprintType)
class GSD_CITYSTREAMING_API UGSDDataLayerConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    // === Core Layers ===

    /** Base city layer - always loaded during gameplay */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Core Layers")
    TObjectPtr<UDataLayerAsset> BaseCityLayer;

    // === Event Layers ===

    /** Events layer - contains event-related content */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Event Layers")
    TObjectPtr<UDataLayerAsset> EventsLayer;

    /** Construction layer - contains construction site content */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Event Layers")
    TObjectPtr<UDataLayerAsset> ConstructionLayer;

    /** Parties layer - contains party/event content */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Event Layers")
    TObjectPtr<UDataLayerAsset> PartiesLayer;

    // === All Runtime Layers ===

    /** Complete list of all runtime Data Layer assets */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "All Layers")
    TArray<TObjectPtr<UDataLayerAsset>> AllRuntimeLayers;

    // === Performance Settings ===

    /**
     * Maximum time per frame for Data Layer activation (in milliseconds).
     * Prevents frame hitches during large layer activations.
     * Range: 1.0 - 16.0 ms (default: 5.0 ms)
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Performance",
        meta = (ClampMin = "1.0", ClampMax = "16.0", Units = "ms"))
    float MaxActivationTimePerFrameMs = 5.0f;

    /**
     * Delay between staged activations (in seconds).
     * Allows spreading layer activation across multiple frames.
     * Range: 0.0 - 1.0 seconds (default: 0.1 seconds)
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Performance",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StagedActivationDelay = 0.1f;

    /**
     * Enable staged activation for large layer sets.
     * When true, layers are activated over multiple frames respecting
     * the frame budget. When false, all layers activate immediately.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Performance")
    bool bUseStagedActivation = true;

    // === Helper Functions ===

    /**
     * Get all event layer assets as an array.
     * @return Array containing Events, Construction, and Parties layers
     */
    UFUNCTION(BlueprintPure, Category = "Data Layers")
    TArray<UDataLayerAsset*> GetEventLayers() const
    {
        TArray<UDataLayerAsset*> EventLayers;
        if (EventsLayer) EventLayers.Add(EventsLayer);
        if (ConstructionLayer) EventLayers.Add(ConstructionLayer);
        if (PartiesLayer) EventLayers.Add(PartiesLayer);
        return EventLayers;
    }

    /**
     * Check if a layer is an event layer.
     * @param LayerAsset The layer asset to check
     * @return True if the layer is one of the event layers
     */
    UFUNCTION(BlueprintPure, Category = "Data Layers")
    bool IsEventLayer(UDataLayerAsset* LayerAsset) const
    {
        if (!LayerAsset) return false;
        return LayerAsset == EventsLayer ||
               LayerAsset == ConstructionLayer ||
               LayerAsset == PartiesLayer;
    }
};
