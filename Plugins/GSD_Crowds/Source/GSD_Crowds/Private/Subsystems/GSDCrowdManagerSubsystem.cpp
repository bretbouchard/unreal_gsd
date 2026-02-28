// Copyright Bret Bouchard. All Rights Reserved.

#include "Subsystems/GSDCrowdManagerSubsystem.h"
#include "DataAssets/GSDCrowdEntityConfig.h"
#include "Fragments/GSDZombieStateFragment.h"
#include "MassCommonFragments.h"
#include "MassSpawner.h"
#include "GSDCrowdLog.h"
#include "Managers/GSDDeterminismManager.h"
#include "Kismet/GameplayStatics.h"

bool UGSDCrowdManagerSubsystem::ShouldCreateSubsystem(UWorld* World) const
{
    // Only create in game worlds (not editor preview worlds)
    return World && (World->IsGameWorld() || World->IsPlayInEditor());
}

int32 UGSDCrowdManagerSubsystem::SpawnEntities(int32 Count, FVector Center, float Radius, UGSDCrowdEntityConfig* EntityConfig)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LOG_GSDCROWDS, Error, TEXT("SpawnEntities: No world context"));
        return 0;
    }

    UMassEntitySubsystem* MassSubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    if (!MassSubsystem)
    {
        UE_LOG(LOG_GSDCROWDS, Error, TEXT("SpawnEntities: Mass Entity subsystem not found"));
        return 0;
    }

    // Use provided config or load default
    if (!EntityConfig)
    {
        EntityConfig = GetDefaultEntityConfig();
    }

    if (!EntityConfig)
    {
        UE_LOG(LOG_GSDCROWDS, Error, TEXT("SpawnEntities: No entity config available"));
        return 0;
    }

    // Generate spawn transforms
    TArray<FTransform> SpawnTransforms = GenerateSpawnTransforms(Count, Center, Radius);
    if (SpawnTransforms.Num() == 0)
    {
        UE_LOG(LOG_GSDCROWDS, Warning, TEXT("SpawnEntities: No spawn transforms generated"));
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
            // Fallback to unseeded random
            RandomOffset = FMath::RandPointInCircle(Radius);
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
            // Fallback to unseeded random
            RandomYaw = FMath::RandRange(0.0f, 360.0f);
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
