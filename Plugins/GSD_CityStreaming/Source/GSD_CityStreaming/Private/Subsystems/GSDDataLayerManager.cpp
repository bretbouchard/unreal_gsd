#include "Subsystems/GSDDataLayerManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/DataLayerSubsystem.h"
#include "Algo/Sort.h"

// === UWorldSubsystem Interface ===

void UGSDDataLayerManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Cache reference to Data Layer subsystem
    DataLayerSubsystem = GetWorld()->GetSubsystem<UDataLayerSubsystem>();

    TotalStagedLayers = 0;
    ActivatedStagedLayers = 0;
    LastActivationStartTime = 0.0f;

    UE_LOG(LogTemp, Verbose, TEXT("GSDDataLayerManager: Initialized"));
}

void UGSDDataLayerManager::Deinitialize()
{
    // Cancel any pending activations
    CancelStagedActivation();

    // Clear references
    Config = nullptr;
    DataLayerSubsystem = nullptr;
    Providers.Empty();
    PendingActivations.Empty();

    UE_LOG(LogTemp, Verbose, TEXT("GSDDataLayerManager: Deinitialized"));

    Super::Deinitialize();
}

bool UGSDDataLayerManager::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create for game worlds (not editor preview worlds)
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

// === Configuration ===

void UGSDDataLayerManager::SetConfig(UGSDDataLayerConfig* InConfig)
{
    Config = InConfig;

    if (Config)
    {
        UE_LOG(LogTemp, Log, TEXT("GSDDataLayerManager: Config set with %d runtime layers"),
            Config->AllRuntimeLayers.Num());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GSDDataLayerManager: Config set to null"));
    }
}

// === Core API ===

void UGSDDataLayerManager::SetDataLayerState(FName LayerName, bool bActivate, EGSDDataLayerPriority Priority)
{
    UDataLayerAsset* LayerAsset = GetLayerAssetByName(LayerName);
    if (!LayerAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("GSDDataLayerManager: Layer '%s' not found"), *LayerName.ToString());
        return;
    }

    SetDataLayerStateByAsset(LayerAsset, bActivate, Priority);
}

void UGSDDataLayerManager::SetDataLayerStateByAsset(UDataLayerAsset* LayerAsset, bool bActivate, EGSDDataLayerPriority Priority)
{
    if (!LayerAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("GSDDataLayerManager: Cannot activate null layer asset"));
        return;
    }

    // Critical priority always activates immediately
    if (Priority == EGSDDataLayerPriority::Critical || !Config || !Config->bUseStagedActivation)
    {
        ActivateLayerInternal(LayerAsset, bActivate);
        return;
    }

    // Add to staged activation queue
    PendingActivations.Add(FGSDPendingLayerActivation(LayerAsset->GetFName(), bActivate, Priority));

    // Sort by priority (higher priority first)
    PendingActivations.Sort([](const FGSDPendingLayerActivation& A, const FGSDPendingLayerActivation& B)
    {
        return static_cast<uint8>(A.Priority) > static_cast<uint8>(B.Priority);
    });

    // Start staged activation if not already running
    if (!IsStagedActivationInProgress())
    {
        TotalStagedLayers = PendingActivations.Num();
        ActivatedStagedLayers = 0;

        // Start timer for staged processing
        float Delay = Config ? Config->StagedActivationDelay : 0.1f;
        GetWorld()->GetTimerManager().SetTimer(
            StagedActivationTimerHandle,
            this,
            &UGSDDataLayerManager::ProcessNextStagedActivation,
            Delay,
            true  // Loop
        );

        UE_LOG(LogTemp, Verbose, TEXT("GSDDataLayerManager: Started staged activation for %d layers"),
            TotalStagedLayers);
    }
}

bool UGSDDataLayerManager::IsDataLayerActivated(FName LayerName) const
{
    UDataLayerAsset* LayerAsset = GetLayerAssetByName(LayerName);
    return IsDataLayerActivatedByAsset(LayerAsset);
}

bool UGSDDataLayerManager::IsDataLayerActivatedByAsset(UDataLayerAsset* LayerAsset) const
{
    if (!LayerAsset || !DataLayerSubsystem)
    {
        return false;
    }

    // Use the Data Layer subsystem to check state
    return DataLayerSubsystem->IsDataLayerActive(LayerAsset);
}

TArray<FName> UGSDDataLayerManager::GetRuntimeDataLayerNames() const
{
    TArray<FName> LayerNames;

    // Add layers from config
    if (Config)
    {
        for (const TObjectPtr<UDataLayerAsset>& LayerAsset : Config->AllRuntimeLayers)
        {
            if (LayerAsset)
            {
                LayerNames.Add(LayerAsset->GetFName());
            }
        }
    }

    // Add layers from providers
    for (const TScriptInterface<IGSDDataLayerProvider>& Provider : Providers)
    {
        if (Provider)
        {
            TArray<FName> ProviderNames = IGSDDataLayerProvider::Execute_GetAllLayerNames(Provider.GetObject());
            LayerNames.Append(ProviderNames);
        }
    }

    return LayerNames;
}

// === Staged Activation ===

void UGSDDataLayerManager::ActivateLayersStaged(const TArray<FName>& LayerNames, EGSDDataLayerPriority Priority)
{
    for (const FName& LayerName : LayerNames)
    {
        SetDataLayerState(LayerName, true, Priority);
    }
}

void UGSDDataLayerManager::CancelStagedActivation()
{
    if (IsStagedActivationInProgress())
    {
        GetWorld()->GetTimerManager().ClearTimer(StagedActivationTimerHandle);
        PendingActivations.Empty();
        TotalStagedLayers = 0;
        ActivatedStagedLayers = 0;

        UE_LOG(LogTemp, Verbose, TEXT("GSDDataLayerManager: Staged activation cancelled"));
    }
}

bool UGSDDataLayerManager::IsStagedActivationInProgress() const
{
    return GetWorld()->GetTimerManager().IsTimerActive(StagedActivationTimerHandle);
}

// === Async Activation ===

void UGSDDataLayerManager::SetDataLayerStateAsync(UObject* WorldContextObject, FName LayerName, bool bActivate,
    FLatentActionInfo LatentInfo, EGSDDataLayerPriority Priority)
{
    // For async activation, we use a simple delayed callback
    // In a full implementation, this would use a latent action
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
    {
        FTimerHandle AsyncTimerHandle;
        World->GetTimerManager().SetTimer(
            AsyncTimerHandle,
            [this, LayerName, bActivate, Priority]()
            {
                SetDataLayerState(LayerName, bActivate, Priority);
            },
            0.001f,  // Minimal delay for async behavior
            false
        );
    }
}

// === Event Layer Helpers ===

void UGSDDataLayerManager::ActivateAllEventLayers()
{
    if (!Config)
    {
        UE_LOG(LogTemp, Warning, TEXT("GSDDataLayerManager: No config set, cannot activate event layers"));
        return;
    }

    TArray<UDataLayerAsset*> EventLayers = Config->GetEventLayers();
    for (UDataLayerAsset* LayerAsset : EventLayers)
    {
        if (LayerAsset)
        {
            SetDataLayerStateByAsset(LayerAsset, true, EGSDDataLayerPriority::Normal);
        }
    }

    UE_LOG(LogTemp, Verbose, TEXT("GSDDataLayerManager: Activating %d event layers"), EventLayers.Num());
}

void UGSDDataLayerManager::DeactivateAllEventLayers()
{
    if (!Config)
    {
        UE_LOG(LogTemp, Warning, TEXT("GSDDataLayerManager: No config set, cannot deactivate event layers"));
        return;
    }

    TArray<UDataLayerAsset*> EventLayers = Config->GetEventLayers();
    for (UDataLayerAsset* LayerAsset : EventLayers)
    {
        if (LayerAsset)
        {
            SetDataLayerStateByAsset(LayerAsset, false, EGSDDataLayerPriority::Normal);
        }
    }

    UE_LOG(LogTemp, Verbose, TEXT("GSDDataLayerManager: Deactivating %d event layers"), EventLayers.Num());
}

void UGSDDataLayerManager::ActivateEventLayer(FName EventType)
{
    if (!Config)
    {
        UE_LOG(LogTemp, Warning, TEXT("GSDDataLayerManager: No config set, cannot activate event layer"));
        return;
    }

    UDataLayerAsset* TargetLayer = nullptr;

    // Map event type names to config layers
    FString EventTypeStr = EventType.ToString().ToLower();
    if (EventTypeStr == TEXT("events"))
    {
        TargetLayer = Config->EventsLayer;
    }
    else if (EventTypeStr == TEXT("construction"))
    {
        TargetLayer = Config->ConstructionLayer;
    }
    else if (EventTypeStr == TEXT("parties"))
    {
        TargetLayer = Config->PartiesLayer;
    }

    if (TargetLayer)
    {
        SetDataLayerStateByAsset(TargetLayer, true, EGSDDataLayerPriority::High);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GSDDataLayerManager: Unknown event type '%s'"), *EventType.ToString());
    }
}

void UGSDDataLayerManager::DeactivateEventLayer(FName EventType)
{
    if (!Config)
    {
        UE_LOG(LogTemp, Warning, TEXT("GSDDataLayerManager: No config set, cannot deactivate event layer"));
        return;
    }

    UDataLayerAsset* TargetLayer = nullptr;

    // Map event type names to config layers
    FString EventTypeStr = EventType.ToString().ToLower();
    if (EventTypeStr == TEXT("events"))
    {
        TargetLayer = Config->EventsLayer;
    }
    else if (EventTypeStr == TEXT("construction"))
    {
        TargetLayer = Config->ConstructionLayer;
    }
    else if (EventTypeStr == TEXT("parties"))
    {
        TargetLayer = Config->PartiesLayer;
    }

    if (TargetLayer)
    {
        SetDataLayerStateByAsset(TargetLayer, false, EGSDDataLayerPriority::Normal);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GSDDataLayerManager: Unknown event type '%s'"), *EventType.ToString());
    }
}

// === Providers ===

void UGSDDataLayerManager::RegisterProvider(TScriptInterface<IGSDDataLayerProvider> Provider)
{
    if (Provider && !Providers.Contains(Provider))
    {
        Providers.Add(Provider);
        UE_LOG(LogTemp, Verbose, TEXT("GSDDataLayerManager: Provider registered"));
    }
}

void UGSDDataLayerManager::UnregisterProvider(TScriptInterface<IGSDDataLayerProvider> Provider)
{
    if (Providers.Remove(Provider) > 0)
    {
        UE_LOG(LogTemp, Verbose, TEXT("GSDDataLayerManager: Provider unregistered"));
    }
}

// === Internal Functions ===

UDataLayerSubsystem* UGSDDataLayerManager::GetDataLayerSubsystem() const
{
    if (!DataLayerSubsystem.IsValid())
    {
        return GetWorld()->GetSubsystem<UDataLayerSubsystem>();
    }
    return DataLayerSubsystem.Get();
}

void UGSDDataLayerManager::ProcessNextStagedActivation()
{
    if (PendingActivations.Num() == 0)
    {
        // All activations complete
        GetWorld()->GetTimerManager().ClearTimer(StagedActivationTimerHandle);

        // Broadcast completion
        OnStagedActivationComplete.Broadcast();

        UE_LOG(LogTemp, Verbose, TEXT("GSDDataLayerManager: Staged activation complete (%d layers)"),
            ActivatedStagedLayers);

        TotalStagedLayers = 0;
        ActivatedStagedLayers = 0;
        return;
    }

    // Check frame budget
    float MaxTimeMs = Config ? Config->MaxActivationTimePerFrameMs : 5.0f;
    float FrameStartTime = FPlatformTime::Seconds() * 1000.0f;
    float TimeUsed = 0.0f;

    // Process as many activations as fit within frame budget
    while (PendingActivations.Num() > 0 && TimeUsed < MaxTimeMs)
    {
        FGSDPendingLayerActivation Activation = PendingActivations[0];
        PendingActivations.RemoveAt(0);

        UDataLayerAsset* LayerAsset = GetLayerAssetByName(Activation.LayerName);
        if (LayerAsset)
        {
            float ActivationStart = FPlatformTime::Seconds() * 1000.0f;
            ActivateLayerInternal(LayerAsset, Activation.bActivate);
            float ActivationEnd = FPlatformTime::Seconds() * 1000.0f;

            TimeUsed += (ActivationEnd - FrameStartTime);
        }

        ActivatedStagedLayers++;

        // Broadcast progress
        OnStagedActivationProgress.Broadcast(ActivatedStagedLayers, TotalStagedLayers);
    }

    // If we exceeded frame budget, remaining activations will process next frame
    if (PendingActivations.Num() > 0 && TimeUsed >= MaxTimeMs)
    {
        UE_LOG(LogTemp, Verbose, TEXT("GSDDataLayerManager: Frame budget exceeded, %d layers remaining"),
            PendingActivations.Num());
    }
}

UDataLayerAsset* UGSDDataLayerManager::GetLayerAssetByName(FName LayerName) const
{
    // First, check config
    if (Config)
    {
        // Check all runtime layers
        for (const TObjectPtr<UDataLayerAsset>& LayerAsset : Config->AllRuntimeLayers)
        {
            if (LayerAsset && LayerAsset->GetFName() == LayerName)
            {
                return LayerAsset;
            }
        }

        // Check individual layer references
        if (Config->BaseCityLayer && Config->BaseCityLayer->GetFName() == LayerName)
        {
            return Config->BaseCityLayer;
        }
        if (Config->EventsLayer && Config->EventsLayer->GetFName() == LayerName)
        {
            return Config->EventsLayer;
        }
        if (Config->ConstructionLayer && Config->ConstructionLayer->GetFName() == LayerName)
        {
            return Config->ConstructionLayer;
        }
        if (Config->PartiesLayer && Config->PartiesLayer->GetFName() == LayerName)
        {
            return Config->PartiesLayer;
        }
    }

    // Check providers
    for (const TScriptInterface<IGSDDataLayerProvider>& Provider : Providers)
    {
        if (Provider)
        {
            UDataLayerAsset* ResolvedLayer = IGSDDataLayerProvider::Execute_ResolveDataLayer(
                Provider.GetObject(), LayerName);
            if (ResolvedLayer)
            {
                return ResolvedLayer;
            }
        }
    }

    // Try to find in Data Layer subsystem directly
    if (UDataLayerSubsystem* Subsystem = GetDataLayerSubsystem())
    {
        // The Data Layer subsystem may have a way to find layers by name
        // This is engine-specific and may need adjustment
        UE_LOG(LogTemp, Verbose, TEXT("GSDDataLayerManager: Layer '%s' not found in config or providers"),
            *LayerName.ToString());
    }

    return nullptr;
}

void UGSDDataLayerManager::ActivateLayerInternal(UDataLayerAsset* LayerAsset, bool bActivate)
{
    if (!LayerAsset || !DataLayerSubsystem)
    {
        return;
    }

    float ActivationStartTime = FPlatformTime::Seconds() * 1000.0f;

    // Use the Data Layer subsystem to set the state
    DataLayerSubsystem->SetDataLayerState(LayerAsset, bActivate);

    float ActivationEndTime = FPlatformTime::Seconds() * 1000.0f;
    float ActivationTimeMs = ActivationEndTime - ActivationStartTime;

    // Broadcast state change
    BroadcastStateChange(LayerAsset->GetFName(), bActivate, ActivationTimeMs);

    UE_LOG(LogTemp, Verbose, TEXT("GSDDataLayerManager: Layer '%s' %s (%.2f ms)"),
        *LayerAsset->GetName(),
        bActivate ? TEXT("activated") : TEXT("deactivated"),
        ActivationTimeMs);
}

void UGSDDataLayerManager::BroadcastStateChange(FName LayerName, bool bIsActive, float ActivationTimeMs)
{
    FGSDDataLayerStateEvent Event(LayerName, bIsActive, ActivationTimeMs);
    OnDataLayerStateChanged.Broadcast(Event);
}
