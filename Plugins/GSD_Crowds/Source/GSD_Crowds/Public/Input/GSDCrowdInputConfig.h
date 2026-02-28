// Copyright Bret Bouchard
// EnhancedInput Configuration for Crowd Debug Controls

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "GSDCrowdInputConfig.generated.h"

class UInputAction;
class UInputMappingContext;

/**
 * DataAsset containing input configuration for crowd debug controls.
 * Allows designers to configure debug input actions and key bindings.
 */
UCLASS(BlueprintType, ClassGroup = "GSD|Crowds")
class GSD_CROWDS_API UGSDCrowdInputConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    // Input Actions for Debug Controls

    /** Toggle crowd debug widget visibility */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Actions")
    TObjectPtr<UInputAction> ToggleDebugWidgetAction;

    /** Increase crowd density */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Actions")
    TObjectPtr<UInputAction> IncreaseDensityAction;

    /** Decrease crowd density */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Actions")
    TObjectPtr<UInputAction> DecreaseDensityAction;

    /** Toggle crowd spawning enabled/disabled */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Actions")
    TObjectPtr<UInputAction> ToggleCrowdEnabledAction;

    /** Reset all crowds to initial state */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Actions")
    TObjectPtr<UInputAction> ResetCrowdAction;

    /** Spawn test crowd for debugging */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Actions")
    TObjectPtr<UInputAction> SpawnTestCrowdAction;

    /** Despawn all crowd entities */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Actions")
    TObjectPtr<UInputAction> DespawnAllCrowdsAction;

    // Input Mapping Context

    /** Mapping context containing debug control key bindings */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Mapping")
    TObjectPtr<UInputMappingContext> DebugMappingContext;

    /** Priority for debug mapping context (higher = takes precedence) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Mapping", meta = (ClampMin = "0", ClampMax = "1000"))
    int32 MappingContextPriority;

    UGSDCrowdInputConfig();
};
