// Copyright Bret Bouchard
// Input Component for Crowd Debug Controls

#include "Components/GSDCrowdInputComponent.h"
#include "Input/GSDCrowdInputConfig.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

UGSDCrowdInputComponent::UGSDCrowdInputComponent()
    : bDebugEnabled(false)
    , bMappingContextAdded(false)
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UGSDCrowdInputComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache player controller
    CachedPlayerController = Cast<APlayerController>(GetOwner());
    if (!CachedPlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("GSDCrowdInputComponent: Owner is not a PlayerController"));
        return;
    }

    // Cache enhanced input component
    CachedInputComponent = Cast<UEnhancedInputComponent>(CachedPlayerController->InputComponent);
    if (!CachedInputComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("GSDCrowdInputComponent: PlayerController does not have EnhancedInputComponent"));
        return;
    }
}

void UGSDCrowdInputComponent::EndPlay(const EEndPlayReason::Reason EndPlayReason)
{
    // Clean up: remove mapping context and unbind actions
    RemoveMappingContext();
    UnbindInputActions();

    Super::EndPlay(EndPlayReason);
}

void UGSDCrowdInputComponent::SetInputConfig(UGSDCrowdInputConfig* InConfig)
{
    // Remove old bindings if config changes
    if (InputConfig && bMappingContextAdded)
    {
        RemoveMappingContext();
        UnbindInputActions();
    }

    InputConfig = InConfig;

    // Apply new config if debug is enabled
    if (bDebugEnabled && InputConfig)
    {
        BindInputActions();
        AddMappingContext();
    }
}

void UGSDCrowdInputComponent::SetDebugEnabled(bool bEnabled)
{
    if (bDebugEnabled == bEnabled)
    {
        return;
    }

    bDebugEnabled = bEnabled;

    if (bDebugEnabled && InputConfig)
    {
        BindInputActions();
        AddMappingContext();
    }
    else
    {
        RemoveMappingContext();
        UnbindInputActions();
    }
}

void UGSDCrowdInputComponent::BindInputActions()
{
    if (!CachedInputComponent || !InputConfig)
    {
        return;
    }

    // Bind ToggleDebugWidget
    if (InputConfig->ToggleDebugWidgetAction)
    {
        CachedInputComponent->BindAction(
            InputConfig->ToggleDebugWidgetAction,
            ETriggerEvent::Started,
            this,
            &UGSDCrowdInputComponent::HandleToggleDebugWidget
        );
    }

    // Bind IncreaseDensity
    if (InputConfig->IncreaseDensityAction)
    {
        CachedInputComponent->BindAction(
            InputConfig->IncreaseDensityAction,
            ETriggerEvent::Started,
            this,
            &UGSDCrowdInputComponent::HandleIncreaseDensity
        );
    }

    // Bind DecreaseDensity
    if (InputConfig->DecreaseDensityAction)
    {
        CachedInputComponent->BindAction(
            InputConfig->DecreaseDensityAction,
            ETriggerEvent::Started,
            this,
            &UGSDCrowdInputComponent::HandleDecreaseDensity
        );
    }

    // Bind ToggleCrowdEnabled
    if (InputConfig->ToggleCrowdEnabledAction)
    {
        CachedInputComponent->BindAction(
            InputConfig->ToggleCrowdEnabledAction,
            ETriggerEvent::Started,
            this,
            &UGSDCrowdInputComponent::HandleToggleCrowdEnabled
        );
    }

    // Bind ResetCrowd
    if (InputConfig->ResetCrowdAction)
    {
        CachedInputComponent->BindAction(
            InputConfig->ResetCrowdAction,
            ETriggerEvent::Started,
            this,
            &UGSDCrowdInputComponent::HandleResetCrowd
        );
    }

    // Bind SpawnTestCrowd
    if (InputConfig->SpawnTestCrowdAction)
    {
        CachedInputComponent->BindAction(
            InputConfig->SpawnTestCrowdAction,
            ETriggerEvent::Started,
            this,
            &UGSDCrowdInputComponent::HandleSpawnTestCrowd
        );
    }

    // Bind DespawnAllCrowds
    if (InputConfig->DespawnAllCrowdsAction)
    {
        CachedInputComponent->BindAction(
            InputConfig->DespawnAllCrowdsAction,
            ETriggerEvent::Started,
            this,
            &UGSDCrowdInputComponent::HandleDespawnAllCrowds
        );
    }
}

void UGSDCrowdInputComponent::UnbindInputActions()
{
    // EnhancedInputComponent automatically handles unbinding when component is destroyed
    // No explicit unbind needed for individual actions
    CachedInputComponent = nullptr;
}

void UGSDCrowdInputComponent::AddMappingContext()
{
    if (!CachedPlayerController || !InputConfig || !InputConfig->DebugMappingContext)
    {
        return;
    }

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(CachedPlayerController->GetLocalPlayer()))
    {
        Subsystem->AddMappingContext(InputConfig->DebugMappingContext, InputConfig->MappingContextPriority);
        bMappingContextAdded = true;
    }
}

void UGSDCrowdInputComponent::RemoveMappingContext()
{
    if (!CachedPlayerController || !InputConfig || !InputConfig->DebugMappingContext || !bMappingContextAdded)
    {
        return;
    }

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(CachedPlayerController->GetLocalPlayer()))
    {
        Subsystem->RemoveMappingContext(InputConfig->DebugMappingContext);
        bMappingContextAdded = false;
    }
}

// Input Action Handlers

void UGSDCrowdInputComponent::HandleToggleDebugWidget(const FInputActionValue& Value)
{
    OnToggleDebugWidget.Broadcast();
}

void UGSDCrowdInputComponent::HandleIncreaseDensity(const FInputActionValue& Value)
{
    OnDensityChanged.Broadcast(1.0f);
}

void UGSDCrowdInputComponent::HandleDecreaseDensity(const FInputActionValue& Value)
{
    OnDensityChanged.Broadcast(-1.0f);
}

void UGSDCrowdInputComponent::HandleToggleCrowdEnabled(const FInputActionValue& Value)
{
    OnToggleCrowdEnabled.Broadcast();
}

void UGSDCrowdInputComponent::HandleResetCrowd(const FInputActionValue& Value)
{
    OnResetCrowd.Broadcast();
}

void UGSDCrowdInputComponent::HandleSpawnTestCrowd(const FInputActionValue& Value)
{
    OnSpawnTestCrowd.Broadcast();
}

void UGSDCrowdInputComponent::HandleDespawnAllCrowds(const FInputActionValue& Value)
{
    OnDespawnAllCrowds.Broadcast();
}
