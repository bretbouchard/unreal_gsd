// Copyright Bret Bouchard. All Rights Reserved.

#include "Subsystems/GSDCrowdManagerSubsystem.h"
#include "DataAssets/GSDCrowdEntityConfig.h"
#include "Fragments/GSDZombieStateFragment.h"
#include "MassCommonFragments.h"
#include "MassSpawner.h"
#include "GSDCrowdLog.h"
#include "Managers/GSDDeterminismManager.h"
#include "Kismet/GameplayStatics.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#include "MassEntitySubsystem.h"
#include "Engine/Engine.h"
#include "ProfilingDebugging/ScopedTimers.h"

bool UGSDCrowdManagerSubsystem::ShouldCreateSubsystem(UWorld* World) const
{
    // Only create in game worlds (not editor preview worlds)
    return World && (World->IsGameWorld() || World->IsPlayInEditor());
}

void UGSDCrowdManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Get World Partition subsystem reference
    if (UWorld* World = GetWorld())
    {
        WorldPartitionSubsystem = World->GetSubsystem<UWorldPartitionSubsystem>();

        // Bind to streaming events
        BindToStreamingEvents();
    }
}

void UGSDCrowdManagerSubsystem::Deinitialize()
{
    // Unbind from streaming events
    UnbindFromStreamingEvents();

    Super::Deinitialize();
}

int32 UGSDCrowdManagerSubsystem::SpawnEntities(int32 Count, FVector Center, float Radius, UGSDCrowdEntityConfig* EntityConfig)
{
    // Check if spawn location is in loaded cell
    if (!IsPositionInLoadedCell(Center))
    {
        // Queue for when cell loads
        FName CellName = GetCellNameForPosition(Center);
        PendingSpawnCenters.Add(Center);

        UE_LOG(LOG_GSDCROWDS, Verbose,
            TEXT("Queueing spawn for unloaded cell: %s (count=%d)"), *CellName.ToString(), Count);
        return 0;  // Will spawn when cell loads
    }

    // Proceed with spawn
    int32 NumSpawned = SpawnEntitiesInternal(Count, Center, Radius, EntityConfig);

    // Track which cell this crowd belongs to
    if (NumSpawned > 0)
    {
        FName CellName = GetCellNameForPosition(Center);
        // TODO: Track entity IDs when entity ID system is implemented (GSDCROWDS-108)
        // CellToCrowdMapping.FindOrAdd(CellName).Append(SpawnedEntityIds);
    }

    return NumSpawned;
}

void UGSDCrowdManagerSubsystem::SpawnEntitiesAsync(int32 Count, FVector Center, float Radius, UGSDCrowdEntityConfig* EntityConfig, const FOnCrowdSpawnComplete& OnComplete)
{
    // NOTE: Async spawning with loading screen support is tracked in GSDCROWDS-104
    // Current implementation spawns synchronously - acceptable for Phase 6 scope
    int32 NumSpawned = SpawnEntities(Count, Center, Radius, EntityConfig);

    if (OnComplete.IsBound())
    {
        OnComplete.Execute(NumSpawned);
    }
}

void UGSDCrowdManagerSubsystem::DespawnAllEntities()
{
    if (SpawnedEntityHandles.Num() == 0)
    {
        UE_LOG(LOG_GSDCROWDS, Log, TEXT("DespawnAllEntities: No entities to despawn"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    UMassEntitySubsystem* MassSubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    if (!MassSubsystem)
    {
        return;
    }

    // CRITICAL: Use Defer() for thread-safe entity destruction
    // See RESEARCH.md Pitfall 2 for details
    // Direct destruction during processing causes crashes
    MassSubsystem->Defer().DestroyEntities(SpawnedEntityHandles);

    const int32 NumDespawned = SpawnedEntityHandles.Num();
    SpawnedEntityHandles.Empty();

    UE_LOG(LOG_GSDCROWDS, Log, TEXT("Despawned %d crowd entities"), NumDespawned);

    // Broadcast completion
    if (AllEntitiesDespawnedDelegate.IsBound())
    {
        AllEntitiesDespawnedDelegate.Broadcast();
    }
}

UGSDCrowdEntityConfig* UGSDCrowdManagerSubsystem::GetDefaultEntityConfig()
{
    // Load default zombie entity config
    static const FString DefaultConfigPath = TEXT("/GSD_Crowds/EntityConfigs/BP_GSDZombieEntityConfig.BP_GSDZombieEntityConfig");

    UGSDCrowdEntityConfig* Config = LoadObject<UGSDCrowdEntityConfig>(nullptr, *DefaultConfigPath);
    if (!Config)
    {
        UE_LOG(LOG_GSDCROWDS, Warning, TEXT("Default entity config not found at: %s"), *DefaultConfigPath);
    }

    return Config;
}

TArray<FTransform> UGSDCrowdManagerSubsystem::GenerateSpawnTransforms(int32 Count, FVector Center, float Radius) const
{
    TArray<FTransform> Transforms;
    Transforms.Reserve(Count);

    // Get DeterminismManager for seeded random (if available)
    UGSDDeterminismManager* DeterminismManager = nullptr;
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            DeterminismManager = GameInstance->GetSubsystem<UGSDDeterminismManager>();
        }
    }

    for (int32 i = 0; i < Count; ++i)
    {
        FTransform SpawnTransform;

        // Random position within circular area
        FVector2D RandomOffset;
        if (DeterminismManager)
        {
            // Use seeded random for deterministic spawns
            FRandomStream& SpawnStream = DeterminismManager->GetCategoryStream(UGSDDeterminismManager::CrowdSpawnCategory);
            const float Angle = SpawnStream.FRand() * 2.0f * PI;
            const float Distance = SpawnStream.FRand() * Radius;
            RandomOffset = FVector2D(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance);

            // Record for debugging
            DeterminismManager->RecordRandomCall(UGSDDeterminismManager::CrowdSpawnCategory, Angle);
            DeterminismManager->RecordRandomCall(UGSDDeterminismManager::CrowdSpawnCategory, Distance);
        }
        else
        {
            // Fallback to seeded random for determinism even without manager
            static FRandomStream FallbackSpawnStream(67890);
            const float Angle = FallbackSpawnStream.FRand() * 2.0f * PI;
            const float Distance = FallbackSpawnStream.FRand() * Radius;
            RandomOffset = FVector2D(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance);
        }
        const FVector SpawnLocation = Center + FVector(RandomOffset.X, RandomOffset.Y, 0.0f);

        // Random rotation (yaw only for ground-based entities)
        float RandomYaw;
        if (DeterminismManager)
        {
            // Use seeded random for deterministic rotation
            FRandomStream& SpawnStream = DeterminismManager->GetCategoryStream(UGSDDeterminismManager::CrowdSpawnCategory);
            RandomYaw = SpawnStream.FRandRange(0.0f, 360.0f);
            DeterminismManager->RecordRandomCall(UGSDDeterminismManager::CrowdSpawnCategory, RandomYaw);
        }
        else
        {
            // Fallback to seeded random for determinism even without manager
            static FRandomStream FallbackYawStream(67891);
            RandomYaw = FallbackYawStream.FRandRange(0.0f, 360.0f);
        }
        const FRotator SpawnRotation(0.0f, RandomYaw, 0.0f);

        SpawnTransform.SetLocation(SpawnLocation);
        SpawnTransform.SetRotation(SpawnRotation.Quaternion());
        SpawnTransform.SetScale3D(FVector::OneVector);

        Transforms.Add(SpawnTransform);
    }

    return Transforms;
}

void UGSDCrowdManagerSubsystem::AddDensityModifier(FGameplayTag ModifierTag, FVector Center, float Radius, float Multiplier)
{
    // Remove existing modifier with same tag (replacement behavior)
    RemoveDensityModifier(ModifierTag);

    FGSDensityModifier NewModifier;
    NewModifier.ModifierTag = ModifierTag;
    NewModifier.Center = Center;
    NewModifier.Radius = Radius;
    NewModifier.Multiplier = Multiplier;

    ActiveDensityModifiers.Add(NewModifier);

    UE_LOG(LOG_GSDCROWDS, Log, TEXT("Added density modifier %s at %s (radius=%.0f, mult=%.2f)"),
        *ModifierTag.ToString(), *Center.ToString(), Radius, Multiplier);
}

void UGSDCrowdManagerSubsystem::RemoveDensityModifier(FGameplayTag ModifierTag)
{
    int32 RemovedCount = ActiveDensityModifiers.RemoveAll([ModifierTag](const FGSDensityModifier& Mod)
    {
        return Mod.ModifierTag == ModifierTag;
    });

    if (RemovedCount > 0)
    {
        UE_LOG(LOG_GSDCROWDS, Log, TEXT("Removed density modifier %s (%d instances)"),
            *ModifierTag.ToString(), RemovedCount);
    }
}

float UGSDCrowdManagerSubsystem::GetDensityMultiplierAtLocation(FVector Location) const
{
    float CombinedMultiplier = 1.0f;

    for (const FGSDensityModifier& Modifier : ActiveDensityModifiers)
    {
        float DistanceSq = FVector::DistSquared(Location, Modifier.Center);
        float RadiusSq = Modifier.Radius * Modifier.Radius;

        if (DistanceSq <= RadiusSq)
        {
            // Inside modifier radius - apply multiplier
            CombinedMultiplier *= Modifier.Multiplier;
        }
    }

    return CombinedMultiplier;
}

//-- Network Validation (GSDNETWORK-107) --

bool UGSDCrowdManagerSubsystem::ValidateSpawnParameters(int32 Count, FVector Center, float Radius, FString& OutError) const
{
    OutError.Empty();

    // Validate spawn count
    if (Count <= 0)
    {
        OutError = FString::Printf(TEXT("Invalid spawn count: %d (must be positive)"), Count);
        UE_LOG(LOG_GSDCROWDS, Warning, TEXT("%s"), *OutError);
        return false;
    }

    if (Count > MaxCrowdSize)
    {
        OutError = FString::Printf(TEXT("Spawn count %d exceeds maximum %d"), Count, MaxCrowdSize);
        UE_LOG(LOG_GSDCROWDS, Warning, TEXT("%s"), *OutError);
        return false;
    }

    // Validate spawn radius
    if (Radius <= 0.0f)
    {
        OutError = FString::Printf(TEXT("Invalid spawn radius: %.2f (must be positive)"), Radius);
        UE_LOG(LOG_GSDCROWDS, Warning, TEXT("%s"), *OutError);
        return false;
    }

    if (Radius > MaxSpawnRadius)
    {
        OutError = FString::Printf(TEXT("Spawn radius %.2f exceeds maximum %.2f"), Radius, MaxSpawnRadius);
        UE_LOG(LOG_GSDCROWDS, Warning, TEXT("%s"), *OutError);
        return false;
    }

    // Validate spawn location is finite (not NaN or Inf)
    if (!FMath::IsFinite(Center.X) || !FMath::IsFinite(Center.Y) || !FMath::IsFinite(Center.Z))
    {
        OutError = FString::Printf(TEXT("Invalid spawn center: %s (contains NaN or Inf)"), *Center.ToString());
        UE_LOG(LOG_GSDCROWDS, Warning, TEXT("%s"), *OutError);
        return false;
    }

    // Validate spawn location magnitude (prevent extreme values)
    const float MaxWorldExtent = 1000000.0f; // 10km from origin
    if (FMath::Abs(Center.X) > MaxWorldExtent || FMath::Abs(Center.Y) > MaxWorldExtent || FMath::Abs(Center.Z) > MaxWorldExtent)
    {
        OutError = FString::Printf(TEXT("Spawn center %s exceeds world bounds"), *Center.ToString());
        UE_LOG(LOG_GSDCROWDS, Warning, TEXT("%s"), *OutError);
        return false;
    }

    return true;
}

bool UGSDCrowdManagerSubsystem::ValidateDensityModifier(FVector Center, float Radius, float Multiplier, FString& OutError) const
{
    OutError.Empty();

    // Validate modifier radius
    if (Radius <= 0.0f)
    {
        OutError = FString::Printf(TEXT("Invalid modifier radius: %.2f (must be positive)"), Radius);
        UE_LOG(LOG_GSDCROWDS, Warning, TEXT("%s"), *OutError);
        return false;
    }

    if (Radius > MaxDensityRadius)
    {
        OutError = FString::Printf(TEXT("Modifier radius %.2f exceeds maximum %.2f"), Radius, MaxDensityRadius);
        UE_LOG(LOG_GSDCROWDS, Warning, TEXT("%s"), *OutError);
        return false;
    }

    // Validate multiplier (must be positive, capped to prevent exploits)
    if (Multiplier <= 0.0f)
    {
        OutError = FString::Printf(TEXT("Invalid multiplier: %.2f (must be positive)"), Multiplier);
        UE_LOG(LOG_GSDCROWDS, Warning, TEXT("%s"), *OutError);
        return false;
    }

    if (Multiplier > MaxDensityMultiplier)
    {
        OutError = FString::Printf(TEXT("Multiplier %.2f exceeds maximum %.2f"), Multiplier, MaxDensityMultiplier);
        UE_LOG(LOG_GSDCROWDS, Warning, TEXT("%s"), *OutError);
        return false;
    }

    // Validate center is finite
    if (!FMath::IsFinite(Center.X) || !FMath::IsFinite(Center.Y) || !FMath::IsFinite(Center.Z))
    {
        OutError = FString::Printf(TEXT("Invalid modifier center: %s (contains NaN or Inf)"), *Center.ToString());
        UE_LOG(LOG_GSDCROWDS, Warning, TEXT("%s"), *OutError);
        return false;
    }

    return true;
}

//-- World Partition Streaming Integration --

FName UGSDCrowdManagerSubsystem::GetCellNameForPosition(const FVector& Position) const
{
    // Calculate cell name from position using grid cell size
    // World Partition uses 12800.0f (128m) cells by default
    static constexpr float CellSize = 12800.0f;

    const int32 CellX = FMath::FloorToInt(Position.X / CellSize);
    const int32 CellY = FMath::FloorToInt(Position.Y / CellSize);

    return FName(*FString::Printf(TEXT("Cell_%d_%d"), CellX, CellY));
}

bool UGSDCrowdManagerSubsystem::IsPositionInLoadedCell(const FVector& Position) const
{
    // If no World Partition subsystem, always allow spawning
    if (!WorldPartitionSubsystem.IsValid())
    {
        return true;
    }

    const FName CellName = GetCellNameForPosition(Position);
    return LoadedCellNames.Contains(CellName);
}

void UGSDCrowdManagerSubsystem::BindToStreamingEvents()
{
    // TODO(GSDCROWDS-112): Bind to actual World Partition streaming events when API is available
    // For now, mark all cells as loaded (no streaming constraints)
    UE_LOG(LOG_GSDCROWDS, Log, TEXT("CrowdManager: Streaming event binding (placeholder - all cells considered loaded)"));

    // Mark a default cell as loaded for non-streaming worlds
    LoadedCellNames.Add(FName(TEXT("DefaultCell")));
}

void UGSDCrowdManagerSubsystem::UnbindFromStreamingEvents()
{
    UE_LOG(LOG_GSDCROWDS, Log, TEXT("CrowdManager: Streaming event unbinding"));
    LoadedCellNames.Empty();
}

void UGSDCrowdManagerSubsystem::OnCellLoaded(const FName& CellName)
{
    UE_LOG(LOG_GSDCROWDS, Log, TEXT("Cell loaded: %s"), *CellName.ToString());

    // Track loaded cell
    LoadedCellNames.Add(CellName);

    // Process pending spawns for this cell
    // Check pending spawn centers that belong to this cell
    TArray<FVector> RemainingPendings;
    for (const FVector& PendingCenter : PendingSpawnCenters)
    {
        FName PendingCellName = GetCellNameForPosition(PendingCenter);
        if (PendingCellName == CellName)
        {
            // This pending spawn belongs to the loaded cell
            // TODO: Trigger spawn with stored parameters (GSDCROWDS-109)
            UE_LOG(LOG_GSDCROWDS, Verbose,
                TEXT("Would process pending spawn at %s for loaded cell %s"),
                *PendingCenter.ToString(), *CellName.ToString());
        }
        else
        {
            // Keep for later
            RemainingPendings.Add(PendingCenter);
        }
    }
    PendingSpawnCenters = RemainingPendings;
}

void UGSDCrowdManagerSubsystem::OnCellUnloaded(const FName& CellName)
{
    UE_LOG(LOG_GSDCROWDS, Log, TEXT("Cell unloaded: %s"), *CellName.ToString());

    // Remove from loaded set
    LoadedCellNames.Remove(CellName);

    // Despawn crowds in this cell
    if (TArray<int32>* CrowdIds = CellToCrowdMapping.Find(CellName))
    {
        // TODO: Despawn specific entities by ID when entity ID system is implemented (GSDCROWDS-108)
        // For now, we just clear the mapping
        UE_LOG(LOG_GSDCROWDS, Verbose,
            TEXT("Would despawn %d crowd entities in cell %s"),
            CrowdIds->Num(), *CellName.ToString());

        CellToCrowdMapping.Remove(CellName);
    }
}

int32 UGSDCrowdManagerSubsystem::SpawnEntitiesInternal(int32 Count, FVector Center, float Radius, UGSDCrowdEntityConfig* EntityConfig)
{
    // Internal spawn without cell checks (used by pending spawn processing)
    // This is the original SpawnEntities logic

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LOG_GSDCROWDS, Error, TEXT("SpawnEntitiesInternal: No world context"));
        return 0;
    }

    UMassEntitySubsystem* MassSubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    if (!MassSubsystem)
    {
        UE_LOG(LOG_GSDCROWDS, Error, TEXT("SpawnEntitiesInternal: Mass Entity subsystem not found"));
        return 0;
    }

    // Use provided config or load default
    if (!EntityConfig)
    {
        EntityConfig = GetDefaultEntityConfig();
    }

    if (!EntityConfig)
    {
        UE_LOG(LOG_GSDCROWDS, Error, TEXT("SpawnEntitiesInternal: No entity config available"));
        return 0;
    }

    // Generate spawn transforms
    TArray<FTransform> SpawnTransforms = GenerateSpawnTransforms(Count, Center, Radius);
    if (SpawnTransforms.Num() == 0)
    {
        UE_LOG(LOG_GSDCROWDS, Warning, TEXT("SpawnEntitiesInternal: No spawn transforms generated"));
        return 0;
    }

    // Spawn entities using Mass Entity subsystem
    TArray<FMassEntityHandle> NewEntityHandles;
    MassSubsystem->SpawnEntities(EntityConfig, SpawnTransforms, NewEntityHandles);

    // Track spawned entities
    SpawnedEntityHandles.Append(NewEntityHandles);

    UE_LOG(LOG_GSDCROWDS, Log, TEXT("Spawned %d crowd entities at center %s with radius %.1f"),
        NewEntityHandles.Num(), *Center.ToString(), Radius);

    return NewEntityHandles.Num();
}

//-- Metrics (Debug Dashboard) --

void UGSDCrowdManagerSubsystem::StartMetricsUpdates()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Start timer for metrics updates (10 Hz)
    World->GetTimerManager().SetTimer(
        MetricsUpdateTimerHandle,
        this,
        &UGSDCrowdManagerSubsystem::UpdateMetrics,
        MetricsUpdateInterval,
        true,  // Loop
        0.0f   // First update immediately
    );

    UE_LOG(LOG_GSDCROWDS, Log, TEXT("Started crowd metrics updates at %.1f Hz"), 1.0f / MetricsUpdateInterval);
}

void UGSDCrowdManagerSubsystem::StopMetricsUpdates()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    World->GetTimerManager().ClearTimer(MetricsUpdateTimerHandle);
    UE_LOG(LOG_GSDCROWDS, Log, TEXT("Stopped crowd metrics updates"));
}

void UGSDCrowdManagerSubsystem::UpdateMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Track frame time
    float CurrentFrameTime = FApp::GetDeltaTime();
    CurrentMetrics.LastFrameTime = CurrentFrameTime;

    // Update frame time history (circular buffer pattern)
    FrameTimeHistory.Add(CurrentFrameTime);
    if (FrameTimeHistory.Num() > 60)
    {
        FrameTimeHistory.RemoveAt(0);
    }

    // Calculate average frame time
    if (FrameTimeHistory.Num() > 0)
    {
        float TotalTime = 0.0f;
        for (float Time : FrameTimeHistory)
        {
            TotalTime += Time;
        }
        CurrentMetrics.AverageFrameTime = TotalTime / FrameTimeHistory.Num();
    }

    // Update entity counts
    CurrentMetrics.TotalEntities = SpawnedEntityHandles.Num();
    CurrentMetrics.ActiveCrowds = (SpawnedEntityHandles.Num() > 0) ? 1 : 0;

    // TODO: LOD distribution requires querying Mass LOD processor (GSDCROWDS-110)
    // For now, placeholder values
    CurrentMetrics.LOD0Count = 0;
    CurrentMetrics.LOD1Count = 0;
    CurrentMetrics.LOD2Count = 0;
    CurrentMetrics.LOD3Count = CurrentMetrics.TotalEntities;  // Assume all at lowest LOD for now

    // TODO: Draw calls requires integration with rendering stats (GSDCROWDS-111)
    CurrentMetrics.DrawCalls = 0;

    // Estimate memory usage (rough approximation)
    // Each entity: ~2KB for transforms, fragments, etc.
    CurrentMetrics.MemoryUsedMB = (CurrentMetrics.TotalEntities * 2.0f) / 1024.0f;

    // Broadcast to bound widgets
    if (CrowdMetricsUpdatedDelegate.IsBound())
    {
        CrowdMetricsUpdatedDelegate.Broadcast(CurrentMetrics);
    }
}

//-- Static Convenience API --

int32 UGSDCrowdManagerSubsystem::SpawnCrowd(UObject* World, int32 Count, FVector Center, float Radius, UGSDCrowdEntityConfig* EntityConfig)
{
    if (!World)
    {
        UE_LOG(LOG_GSDCROWDS, Error, TEXT("SpawnCrowd: World context is null"));
        return 0;
    }

    UWorld* WorldPtr = World->GetWorld();
    if (!WorldPtr)
    {
        UE_LOG(LOG_GSDCROWDS, Error, TEXT("SpawnCrowd: Could not get world from context"));
        return 0;
    }

    UGSDCrowdManagerSubsystem* Subsystem = WorldPtr->GetSubsystem<UGSDCrowdManagerSubsystem>();
    if (!Subsystem)
    {
        UE_LOG(LOG_GSDCROWDS, Error, TEXT("SpawnCrowd: CrowdManagerSubsystem not found"));
        return 0;
    }

    return Subsystem->SpawnEntities(Count, Center, Radius, EntityConfig);
}
