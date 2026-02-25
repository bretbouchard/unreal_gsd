// Copyright Bret Bouchard. All Rights Reserved.

#include "Actors/GSDVehicleTestbedActor.h"
#include "Subsystems/GSDVehiclePoolSubsystem.h"
#include "Subsystems/GSDVehicleSpawnerSubsystem.h"
#include "DataAssets/GSDVehicleConfig.h"
#include "Actors/GSDVehiclePawn.h"
#include "GSDVehicleLog.h"
#include "Components/SceneComponent.h"

AGSDVehicleTestbedActor::AGSDVehicleTestbedActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    USceneComponent* RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    SetRootComponent(RootSceneComponent);
}

void AGSDVehicleTestbedActor::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoSpawnOnBeginPlay)
    {
        SpawnTestVehicles();
    }
}

void AGSDVehicleTestbedActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Calculate FPS from delta time
    if (DeltaTime > 0.0f)
    {
        CurrentFPS = 1.0f / DeltaTime;

        // Track frame time history
        FrameTimeHistory.Add(DeltaTime);
        if (FrameTimeHistory.Num() > MaxFrameTimeHistory)
        {
            FrameTimeHistory.RemoveAt(0);
        }

        // Log performance warning if FPS drops below target
        if (bLogPerformanceWarnings && CurrentFPS < TargetFPS)
        {
            GSD_VEHICLE_WARN(TEXT("Testbed FPS %.1f below target %.1f with %d vehicles"),
                CurrentFPS, TargetFPS, SpawnedVehicles.Num());
        }
    }
}

void AGSDVehicleTestbedActor::SpawnTestVehicles()
{
    if (VehicleConfigs.Num() == 0)
    {
        GSD_VEHICLE_ERROR(TEXT("SpawnTestVehicles: No vehicle configs assigned"));
        return;
    }

    // Initialize random stream for deterministic spawns
    FRandomStream RandomStream;
    if (bUseDeterministicSeed)
    {
        RandomStream.Initialize(RandomSeed);
    }
    else
    {
        RandomStream.GenerateNewSeed();
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        GSD_VEHICLE_ERROR(TEXT("SpawnTestVehicles: No world context"));
        return;
    }

    // Get subsystems
    UGSDVehiclePoolSubsystem* PoolSubsystem = World->GetSubsystem<UGSDVehiclePoolSubsystem>();
    UGSDVehicleSpawnerSubsystem* SpawnerSubsystem = World->GetSubsystem<UGSDVehicleSpawnerSubsystem>();

    // Warm up pools if using pooling
    if (bUsePooling && PoolSubsystem)
    {
        for (const auto& ConfigPtr : VehicleConfigs)
        {
            if (UGSDVehicleConfig* Config = ConfigPtr.LoadSynchronous())
            {
                PoolSubsystem->WarmUpPool(Config, PoolSizePerConfig);
            }
        }
    }

    FVector ActorLocation = GetActorLocation();

    // Spawn vehicles
    for (int32 i = 0; i < NumVehiclesToSpawn; ++i)
    {
        // Random config selection
        int32 ConfigIndex = RandomStream.RandRange(0, VehicleConfigs.Num() - 1);
        UGSDVehicleConfig* Config = VehicleConfigs[ConfigIndex].LoadSynchronous();

        if (!Config)
        {
            continue;
        }

        // Random position within spawn area
        FVector RandomOffset(
            RandomStream.FRandRange(-SpawnAreaSize.X / 2.0f, SpawnAreaSize.X / 2.0f),
            RandomStream.FRandRange(-SpawnAreaSize.Y / 2.0f, SpawnAreaSize.Y / 2.0f),
            0.0f
        );

        FVector SpawnLocation = ActorLocation + RandomOffset;

        // Random rotation (yaw only)
        FRotator SpawnRotation(0.0f, RandomStream.FRandRange(0.0f, 360.0f), 0.0f);

        AGSDVehiclePawn* Vehicle = nullptr;

        // Spawn using pool or regular spawner
        if (bUsePooling && PoolSubsystem)
        {
            Vehicle = PoolSubsystem->AcquireVehicle(Config, SpawnLocation, SpawnRotation);
        }
        else if (SpawnerSubsystem)
        {
            Vehicle = SpawnerSubsystem->SpawnVehicle(Config, SpawnLocation, SpawnRotation);
        }

        if (Vehicle)
        {
            SpawnedVehicles.Add(Vehicle);
        }
    }

    GSD_VEHICLE_LOG(Log, TEXT("SpawnTestVehicles: Spawned %d vehicles"), SpawnedVehicles.Num());
}

void AGSDVehicleTestbedActor::DespawnTestVehicles()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    UGSDVehiclePoolSubsystem* PoolSubsystem = World->GetSubsystem<UGSDVehiclePoolSubsystem>();
    UGSDVehicleSpawnerSubsystem* SpawnerSubsystem = World->GetSubsystem<UGSDVehicleSpawnerSubsystem>();

    int32 DespawnedCount = 0;

    for (AGSDVehiclePawn* Vehicle : SpawnedVehicles)
    {
        if (!Vehicle)
        {
            continue;
        }

        if (bUsePooling && PoolSubsystem)
        {
            PoolSubsystem->ReleaseVehicle(Vehicle);
        }
        else if (SpawnerSubsystem)
        {
            SpawnerSubsystem->DespawnVehicle(Vehicle);
        }

        ++DespawnedCount;
    }

    SpawnedVehicles.Empty();

    GSD_VEHICLE_LOG(Log, TEXT("DespawnTestVehicles: Despawned %d vehicles"), DespawnedCount);
}

void AGSDVehicleTestbedActor::RespawnTestVehicles()
{
    DespawnTestVehicles();
    SpawnTestVehicles();
}

float AGSDVehicleTestbedActor::GetAverageFrameTime() const
{
    if (FrameTimeHistory.Num() == 0)
    {
        return 0.0f;
    }

    float TotalTime = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        TotalTime += FrameTime;
    }

    return TotalTime / static_cast<float>(FrameTimeHistory.Num());
}

float AGSDVehicleTestbedActor::GetAverageFPS() const
{
    float AvgFrameTime = GetAverageFrameTime();
    if (AvgFrameTime > 0.0f)
    {
        return 1.0f / AvgFrameTime;
    }
    return 0.0f;
}
