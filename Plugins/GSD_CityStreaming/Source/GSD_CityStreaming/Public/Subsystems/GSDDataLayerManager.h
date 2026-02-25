#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataLayer.h"
#include "Types/GSDDataLayerTypes.h"
#include "Config/GSDDataLayerConfig.h"
#include "GSDDataLayerManager.generated.h"

// Forward declarations
class UDataLayerSubsystem;
class UDataLayerAsset;

/**
 * Interface for Data Layer providers.
 * Implement this interface to provide custom Data Layer resolution logic.
 */
UINTERFACE(Blueprintable)
class GSD_CITYSTREAMING_API UGSDDataLayerProvider : public UInterface
{
    GENERATED_BODY()
};

class GSD_CITYSTREAMING_API IGSDDataLayerProvider
{
    GENERATED_BODY()

public:
    /**
     * Get the Data Layer asset for a given layer name.
     * @param LayerName Name of the layer to resolve
     * @return The Data Layer asset, or nullptr if not found
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Data Layers")
    UDataLayerAsset* ResolveDataLayer(FName LayerName);
    virtual UDataLayerAsset* ResolveDataLayer_Implementation(FName LayerName) { return nullptr; }

    /**
     * Get all available Data Layer names.
     * @return Array of all layer names this provider can resolve
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Data Layers")
    TArray<FName> GetAllLayerNames();
    virtual TArray<FName> GetAllLayerNames_Implementation() { return TArray<FName>(); }
};

/**
 * World subsystem for runtime Data Layer control.
 *
 * Provides Blueprint-callable API for toggling and querying Data Layer states
 * at runtime. Supports staged activation to prevent frame hitches during
 * large layer activations.
 *
 * Usage:
 * 1. Get the subsystem from the world: GetWorld()->GetSubsystem<UGSDDataLayerManager>()
 * 2. Set the configuration asset: SetConfig(YourConfigAsset)
 * 3. Activate/deactivate layers: SetDataLayerState(LayerName, true/false)
 * 4. Query state: IsDataLayerActivated(LayerName)
 */
UCLASS()
class GSD_CITYSTREAMING_API UGSDDataLayerManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // === UWorldSubsystem Interface ===

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // === Configuration ===

    /**
     * Set the Data Layer configuration asset.
     * Must be called before using other functions.
     * @param InConfig The configuration asset to use
     */
    UFUNCTION(BlueprintCallable, Category = "Data Layers")
    void SetConfig(UGSDDataLayerConfig* InConfig);

    /**
     * Get the current configuration asset.
     * @return The current configuration, or nullptr if not set
     */
    UFUNCTION(BlueprintPure, Category = "Data Layers")
    UGSDDataLayerConfig* GetConfig() const { return Config; }

    // === Core API ===

    /**
     * Set the activation state of a Data Layer.
     * For Critical priority, activates immediately. For other priorities,
     * may use staged activation based on configuration.
     *
     * @param LayerName Name of the Data Layer to modify
     * @param bActivate True to activate, false to deactivate
     * @param Priority Priority for staged activation (default: Normal)
     */
    UFUNCTION(BlueprintCallable, Category = "Data Layers",
        meta = (AdvancedDisplay = "Priority"))
    void SetDataLayerState(FName LayerName, bool bActivate,
        EGSDDataLayerPriority Priority = EGSDDataLayerPriority::Normal);

    /**
     * Set the activation state of a Data Layer by asset.
     * @param LayerAsset The Data Layer asset to modify
     * @param bActivate True to activate, false to deactivate
     * @param Priority Priority for staged activation (default: Normal)
     */
    UFUNCTION(BlueprintCallable, Category = "Data Layers",
        meta = (AdvancedDisplay = "Priority"))
    void SetDataLayerStateByAsset(UDataLayerAsset* LayerAsset, bool bActivate,
        EGSDDataLayerPriority Priority = EGSDDataLayerPriority::Normal);

    /**
     * Check if a Data Layer is currently activated.
     * @param LayerName Name of the Data Layer to check
     * @return True if the layer is activated
     */
    UFUNCTION(BlueprintPure, Category = "Data Layers")
    bool IsDataLayerActivated(FName LayerName) const;

    /**
     * Check if a Data Layer is currently activated by asset.
     * @param LayerAsset The Data Layer asset to check
     * @return True if the layer is activated
     */
    UFUNCTION(BlueprintPure, Category = "Data Layers")
    bool IsDataLayerActivatedByAsset(UDataLayerAsset* LayerAsset) const;

    /**
     * Get all runtime Data Layer names available in the system.
     * @return Array of all available layer names
     */
    UFUNCTION(BlueprintPure, Category = "Data Layers")
    TArray<FName> GetRuntimeDataLayerNames() const;

    // === Staged Activation ===

    /**
     * Activate multiple layers using staged activation.
     * Layers are activated in priority order over multiple frames
     * to respect the frame budget.
     *
     * @param LayerNames Array of layer names to activate
     * @param Priority Priority for all layers (default: Normal)
     */
    UFUNCTION(BlueprintCallable, Category = "Data Layers",
        meta = (AdvancedDisplay = "Priority"))
    void ActivateLayersStaged(const TArray<FName>& LayerNames,
        EGSDDataLayerPriority Priority = EGSDDataLayerPriority::Normal);

    /**
     * Cancel any pending staged activations.
     */
    UFUNCTION(BlueprintCallable, Category = "Data Layers")
    void CancelStagedActivation();

    /**
     * Check if staged activation is in progress.
     * @return True if there are pending layer activations
     */
    UFUNCTION(BlueprintPure, Category = "Data Layers")
    bool IsStagedActivationInProgress() const;

    // === Async Activation ===

    /**
     * Asynchronously activate a Data Layer.
     * Uses latent action manager for async operation.
     *
     * @param LayerName Name of the layer to activate
     * @param Callback Callback to invoke when activation completes
     */
    UFUNCTION(BlueprintCallable, Category = "Data Layers",
        meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
    void SetDataLayerStateAsync(UObject* WorldContextObject, FName LayerName, bool bActivate,
        FLatentActionInfo LatentInfo, EGSDDataLayerPriority Priority = EGSDDataLayerPriority::Normal);

    // === Event Layer Helpers ===

    /**
     * Activate all event layers.
     * Convenience function for enabling all event-related content.
     */
    UFUNCTION(BlueprintCallable, Category = "Data Layers|Events")
    void ActivateAllEventLayers();

    /**
     * Deactivate all event layers.
     * Convenience function for disabling all event-related content.
     */
    UFUNCTION(BlueprintCallable, Category = "Data Layers|Events")
    void DeactivateAllEventLayers();

    /**
     * Activate a specific event layer by type.
     * @param EventType Name of the event type (Events, Construction, Parties)
     */
    UFUNCTION(BlueprintCallable, Category = "Data Layers|Events")
    void ActivateEventLayer(FName EventType);

    /**
     * Deactivate a specific event layer by type.
     * @param EventType Name of the event type (Events, Construction, Parties)
     */
    UFUNCTION(BlueprintCallable, Category = "Data Layers|Events")
    void DeactivateEventLayer(FName EventType);

    // === Providers ===

    /**
     * Register a custom Data Layer provider.
     * @param Provider The provider to register
     */
    UFUNCTION(BlueprintCallable, Category = "Data Layers")
    void RegisterProvider(TScriptInterface<IGSDDataLayerProvider> Provider);

    /**
     * Unregister a custom Data Layer provider.
     * @param Provider The provider to unregister
     */
    UFUNCTION(BlueprintCallable, Category = "Data Layers")
    void UnregisterProvider(TScriptInterface<IGSDDataLayerProvider> Provider);

    // === Delegates ===

    /** Broadcast when a Data Layer's state changes */
    UPROPERTY(BlueprintAssignable, Category = "Data Layers")
    FOnGSDDataLayerStateChanged OnDataLayerStateChanged;

    /** Broadcast during staged activation progress */
    UPROPERTY(BlueprintAssignable, Category = "Data Layers")
    FOnGSDStagedActivationProgress OnStagedActivationProgress;

    /** Broadcast when staged activation completes */
    UPROPERTY(BlueprintAssignable, Category = "Data Layers")
    FOnGSDStagedActivationComplete OnStagedActivationComplete;

protected:
    // === Internal State ===

    /** Configuration asset reference */
    UPROPERTY()
    TObjectPtr<UGSDDataLayerConfig> Config;

    /** Reference to the engine's Data Layer subsystem */
    UPROPERTY()
    TObjectPtr<UDataLayerSubsystem> DataLayerSubsystem;

    /** Queue of pending layer activations for staged loading */
    TArray<FGSDPendingLayerActivation> PendingActivations;

    /** Timer handle for staged activation processing */
    FTimerHandle StagedActivationTimerHandle;

    /** Total layers in current staged activation (for progress tracking) */
    int32 TotalStagedLayers;

    /** Layers activated in current staged activation (for progress tracking) */
    int32 ActivatedStagedLayers;

    /** Registered custom providers */
    UPROPERTY()
    TArray<TScriptInterface<IGSDDataLayerProvider>> Providers;

    // === Internal Functions ===

    /** Get the Data Layer subsystem (cached) */
    UDataLayerSubsystem* GetDataLayerSubsystem() const;

    /** Process next pending activation in staged queue */
    void ProcessNextStagedActivation();

    /** Get layer asset by name from config or providers */
    UDataLayerAsset* GetLayerAssetByName(FName LayerName) const;

    /** Internal activation implementation */
    void ActivateLayerInternal(UDataLayerAsset* LayerAsset, bool bActivate);

    /** Broadcast state change event */
    void BroadcastStateChange(FName LayerName, bool bIsActive, float ActivationTimeMs);

private:
    /** Track last activation time for frame budget */
    float LastActivationStartTime;
};
