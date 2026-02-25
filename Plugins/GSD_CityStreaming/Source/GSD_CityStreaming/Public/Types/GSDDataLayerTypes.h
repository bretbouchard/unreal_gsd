#pragma once

#include "CoreMinimal.h"
#include "GSDDataLayerTypes.generated.h"

/**
 * Priority for staged Data Layer activation.
 * Higher priority layers are activated first during staged loading.
 */
UENUM(BlueprintType)
enum class EGSDDataLayerPriority : uint8
{
    /** Background content - loaded last, not time-critical */
    Low         UMETA(DisplayName = "Low"),

    /** Standard content - normal loading priority */
    Normal      UMETA(DisplayName = "Normal"),

    /** Player-critical content - loaded early */
    High        UMETA(DisplayName = "High"),

    /** Must load immediately - bypasses staged activation */
    Critical    UMETA(DisplayName = "Critical")
};

/**
 * State change event for Data Layers.
 * Broadcast when a Data Layer's activation state changes.
 */
USTRUCT(BlueprintType)
struct FGSDDataLayerStateEvent
{
    GENERATED_BODY()

    /** Name of the Data Layer that changed state */
    UPROPERTY(BlueprintReadOnly)
    FName LayerName;

    /** New activation state (true = activated, false = deactivated) */
    UPROPERTY(BlueprintReadOnly)
    bool bIsActive = false;

    /** Time in milliseconds taken for activation/deactivation */
    UPROPERTY(BlueprintReadOnly)
    float ActivationTimeMs = 0.0f;

    /** Default constructor */
    FGSDDataLayerStateEvent()
        : LayerName(NAME_None)
        , bIsActive(false)
        , ActivationTimeMs(0.0f)
    {}

    /** Constructor with parameters */
    FGSDDataLayerStateEvent(FName InLayerName, bool bInIsActive, float InActivationTimeMs = 0.0f)
        : LayerName(InLayerName)
        , bIsActive(bInIsActive)
        , ActivationTimeMs(InActivationTimeMs)
    {}
};

/**
 * Pending activation request for staged loading.
 * Internal structure used by the Data Layer Manager.
 */
USTRUCT()
struct FGSDPendingLayerActivation
{
    GENERATED_BODY()

    /** Data Layer asset to activate/deactivate */
    UPROPERTY()
    FName LayerName;

    /** Target state (true = activate, false = deactivate) */
    UPROPERTY()
    bool bActivate = true;

    /** Priority for activation order */
    UPROPERTY()
    EGSDDataLayerPriority Priority = EGSDDataLayerPriority::Normal;

    /** Default constructor */
    FGSDPendingLayerActivation()
        : LayerName(NAME_None)
        , bActivate(true)
        , Priority(EGSDDataLayerPriority::Normal)
    {}

    /** Constructor with parameters */
    FGSDPendingLayerActivation(FName InLayerName, bool bInActivate, EGSDDataLayerPriority InPriority)
        : LayerName(InLayerName)
        , bActivate(bInActivate)
        , Priority(InPriority)
    {}

    /** Comparison operator for sorting by priority (higher priority first) */
    bool operator<(const FGSDPendingLayerActivation& Other) const
    {
        return static_cast<uint8>(Priority) < static_cast<uint8>(Other.Priority);
    }
};

/** Delegate for Data Layer state changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGSDDataLayerStateChanged, const FGSDDataLayerStateEvent&, Event);

/** Delegate for staged activation progress */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGSDStagedActivationProgress, int32, LayersActivated, int32, TotalLayers);

/** Delegate for staged activation completion */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGSDStagedActivationComplete);
