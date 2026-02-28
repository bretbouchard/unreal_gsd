// Copyright Bret Bouchard
// Input Component for Crowd Debug Controls

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GSDCrowdInputComponent.generated.h"

class UGSDCrowdInputConfig;
class UInputMappingContext;
class UEnhancedInputComponent;
class APlayerController;

// Delegates for debug control events

/** Called when debug widget should toggle visibility */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnToggleDebugWidget);

/** Called when crowd density should change (positive delta = increase, negative = decrease) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDensityChanged, float, Delta);

/** Called when crowd spawning should toggle enabled/disabled */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnToggleCrowdEnabled);

/** Called when crowds should reset to initial state */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResetCrowd);

/** Called when a test crowd should spawn */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpawnTestCrowd);

/** Called when all crowds should despawn */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDespawnAllCrowds);

/**
 * Input component for binding crowd debug controls to EnhancedInput system.
 * Attach to PlayerController to enable debug input handling.
 */
UCLASS(ClassGroup = "GSD|Crowds", meta = (BlueprintSpawnableComponent))
class GSD_CROWDS_API UGSDCrowdInputComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGSDCrowdInputComponent();

    // Configuration

    /** Set the input configuration DataAsset */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Input")
    void SetInputConfig(UGSDCrowdInputConfig* InConfig);

    /** Enable or disable debug input handling */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Input")
    void SetDebugEnabled(bool bEnabled);

    // Delegates (Events)

    /** Broadcast when debug widget should toggle */
    UPROPERTY(BlueprintAssignable, Category = "GSD|Crowds|Input")
    FOnToggleDebugWidget OnToggleDebugWidget;

    /** Broadcast when density should change */
    UPROPERTY(BlueprintAssignable, Category = "GSD|Crowds|Input")
    FOnDensityChanged OnDensityChanged;

    /** Broadcast when crowd enabled should toggle */
    UPROPERTY(BlueprintAssignable, Category = "GSD|Crowds|Input")
    FOnToggleCrowdEnabled OnToggleCrowdEnabled;

    /** Broadcast when crowds should reset */
    UPROPERTY(BlueprintAssignable, Category = "GSD|Crowds|Input")
    FOnResetCrowd OnResetCrowd;

    /** Broadcast when test crowd should spawn */
    UPROPERTY(BlueprintAssignable, Category = "GSD|Crowds|Input")
    FOnSpawnTestCrowd OnSpawnTestCrowd;

    /** Broadcast when all crowds should despawn */
    UPROPERTY(BlueprintAssignable, Category = "GSD|Crowds|Input")
    FOnDespawnAllCrowds OnDespawnAllCrowds;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Reason EndPlayReason) override;

    /** Bind all input actions from the config */
    void BindInputActions();

    /** Unbind all input actions */
    void UnbindInputActions();

    /** Add the mapping context to the player controller */
    void AddMappingContext();

    /** Remove the mapping context from the player controller */
    void RemoveMappingContext();

    // Input Action Handlers

    void HandleToggleDebugWidget(const FInputActionValue& Value);
    void HandleIncreaseDensity(const FInputActionValue& Value);
    void HandleDecreaseDensity(const FInputActionValue& Value);
    void HandleToggleCrowdEnabled(const FInputActionValue& Value);
    void HandleResetCrowd(const FInputActionValue& Value);
    void HandleSpawnTestCrowd(const FInputActionValue& Value);
    void HandleDespawnAllCrowds(const FInputActionValue& Value);

private:
    /** Input configuration DataAsset */
    UPROPERTY()
    TObjectPtr<UGSDCrowdInputConfig> InputConfig;

    /** Whether debug input is currently enabled */
    UPROPERTY()
    bool bDebugEnabled;

    /** Whether mapping context has been added */
    UPROPERTY()
    bool bMappingContextAdded;

    /** Cached player controller */
    UPROPERTY()
    TObjectPtr<APlayerController> CachedPlayerController;

    /** Cached enhanced input component */
    UPROPERTY()
    TObjectPtr<UEnhancedInputComponent> CachedInputComponent;
};
