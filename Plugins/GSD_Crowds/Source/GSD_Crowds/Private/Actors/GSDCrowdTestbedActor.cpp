// Copyright Bret Bouchard. All Rights Reserved.

#include "Actors/GSDCrowdTestbedActor.h"
#include "Actors/GSDHeroNPC.h"
#include "Subsystems/GSDCrowdManagerSubsystem.h"
#include "DataAssets/GSDCrowdEntityConfig.h"
#include "GSDCrowdLog.h"

AGSDCrowdTestbedActor::AGSDCrowdTestbedActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Pre-allocate circular buffer
    FrameTimeHistory.Init(0.0f, MaxFrameTimeHistory);
}

void AGSDCrowdTestbedActor::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LOG_GSDCROWDS, Log, TEXT("GSDCrowdTestbedActor initialized - Target: %d entities, Radius: %.1f, Target FPS: %.1f"),
        NumEntitiesToSpawn, SpawnRadius, TargetFPS);

    if (bAutoSpawnOnBeginPlay)
    {
        SpawnTestEntities();
    }
}

void AGSDCrowdTestbedActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update FPS metrics
    UpdateFPSMetrics(DeltaTime);

    // Check performance warnings
    if (bLogPerformanceWarnings)
    {
        CheckPerformanceWarnings();
    }

    // Update AI statistics (Phase 7)
    UpdateAIStatistics();

    // Draw navigation debug if enabled
    if (bEnableNavigation)
    {
        DrawNavigationDebug();
    }
}

void AGSDCrowdTestbedActor::SpawnTestEntities()
{
    UGSDCrowdManagerSubsystem* Manager = GetCrowdManager();
    if (!Manager)
    {
        UE_LOG(LOG_GSDCROWDS, Error, TEXT("SpawnTestEntities: Crowd manager not available"));
        return;
    }

    // Set random seed if using deterministic spawning
    if (bUseDeterministicSeed)
    {
        FMath::SRandInit(RandomSeed);
        UE_LOG(LOG_GSDCROWDS, Log, TEXT("Using deterministic seed: %d"), RandomSeed);
    }

    // Spawn entities
    const FVector SpawnCenter = GetActorLocation();
    const int32 NumSpawned = Manager->SpawnEntities(NumEntitiesToSpawn, SpawnCenter, SpawnRadius, EntityConfig);

    UE_LOG(LOG_GSDCROWDS, Log, TEXT("Spawned %d/%d entities at center %s"),
        NumSpawned, NumEntitiesToSpawn, *SpawnCenter.ToString());

    // Clear frame time history for fresh FPS measurement
    FrameTimeHistory.Init(0.0f, MaxFrameTimeHistory);
    FrameHistoryIndex = 0;
    FrameTimeSum = 0.0f;

    // Spawn Hero NPCs if configured (Phase 7)
    if (HeroNPCCount > 0 && HeroNPCClass)
    {
        SpawnHeroNPCs();
    }
}

void AGSDCrowdTestbedActor::DespawnTestEntities()
{
    UGSDCrowdManagerSubsystem* Manager = GetCrowdManager();
    if (!Manager)
    {
        return;
    }

    Manager->DespawnAllEntities();

    UE_LOG(LOG_GSDCROWDS, Log, TEXT("Despawned all test entities"));

    // Cleanup Hero NPCs (Phase 7)
    DespawnHeroNPCs();
}

void AGSDCrowdTestbedActor::RespawnTestEntities()
{
    DespawnTestEntities();
    SpawnTestEntities();
}

int32 AGSDCrowdTestbedActor::GetSpawnedEntityCount() const
{
    if (UGSDCrowdManagerSubsystem* Manager = const_cast<AGSDCrowdTestbedActor*>(this)->GetCrowdManager())
    {
        return Manager->GetActiveEntityCount();
    }
    return 0;
}

float AGSDCrowdTestbedActor::GetAverageFrameTime() const
{
    // Use pre-computed sum from circular buffer
    if (FrameTimeHistory.Num() == 0 || FrameTimeSum <= 0.0f)
    {
        return 0.0f;
    }

    return FrameTimeSum / static_cast<float>(FrameTimeHistory.Num());
}

float AGSDCrowdTestbedActor::GetAverageFPS() const
{
    const float AvgFrameTime = GetAverageFrameTime();
    if (AvgFrameTime > 0.0f)
    {
        return 1.0f / AvgFrameTime;
    }
    return 0.0f;
}

bool AGSDCrowdTestbedActor::IsPerformanceAcceptable() const
{
    const float AvgFPS = GetAverageFPS();
    const float MinAcceptableFPS = TargetFPS * (1.0f - PerformanceWarningThreshold / 100.0f);
    return AvgFPS >= MinAcceptableFPS;
}

UGSDCrowdManagerSubsystem* AGSDCrowdTestbedActor::GetCrowdManager()
{
    if (!CrowdManager)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            CrowdManager = World->GetSubsystem<UGSDCrowdManagerSubsystem>();
        }
    }
    return CrowdManager;
}

void AGSDCrowdTestbedActor::UpdateFPSMetrics(float DeltaTime)
{
    // Calculate current FPS
    if (DeltaTime > 0.0f)
    {
        CurrentFPS = 1.0f / DeltaTime;
    }

    // Circular buffer update - O(1) operation
    // Subtract the oldest value from sum before overwriting
    FrameTimeSum -= FrameTimeHistory[FrameHistoryIndex];

    // Store new value in circular buffer
    FrameTimeHistory[FrameHistoryIndex] = DeltaTime;

    // Add new value to sum
    FrameTimeSum += DeltaTime;

    // Advance index (wrap around)
    FrameHistoryIndex = (FrameHistoryIndex + 1) % MaxFrameTimeHistory;
}

void AGSDCrowdTestbedActor::CheckPerformanceWarnings()
{
    // Only check after we have enough samples
    if (FrameTimeHistory.Num() < 30)
    {
        return;
    }

    // Throttle warnings
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    const float CurrentTime = World->GetTimeSeconds();
    if (CurrentTime - LastPerformanceWarningTime < PerformanceWarningCooldown)
    {
        return;
    }

    // Check if FPS is below threshold
    const float AvgFPS = GetAverageFPS();
    const float MinAcceptableFPS = TargetFPS * (1.0f - PerformanceWarningThreshold / 100.0f);

    if (AvgFPS < MinAcceptableFPS)
    {
        const int32 EntityCount = GetSpawnedEntityCount();
        UE_LOG(LOG_GSDCROWDS, Warning,
            TEXT("PERFORMANCE WARNING: Average FPS %.1f below target %.1f (threshold: %.1f) with %d entities"),
            AvgFPS, TargetFPS, MinAcceptableFPS, EntityCount);

        LastPerformanceWarningTime = CurrentTime;
    }
}

void AGSDCrowdTestbedActor::UpdateAIStatistics()
{
    // These would be queried from the Mass Entity system
    // For now, we'll track via the crowd manager

    EntitiesOnLanes = 0;
    EntitiesInteracting = 0;

    if (UGSDCrowdManagerSubsystem* Manager = GetCrowdManager())
    {
        // Query entity states
        // This would require extending the crowd manager with query methods
        // For Phase 7, we'll use placeholder values
        const int32 SpawnedEntityCount = Manager->GetActiveEntityCount();
        EntitiesOnLanes = FMath::Min(SpawnedEntityCount, SpawnedEntityCount / 2);
        EntitiesInteracting = FMath::Max(0, SpawnedEntityCount / 10);
    }

    ActiveHeroNPCs = SpawnedHeroNPCs.Num();
}

void AGSDCrowdTestbedActor::DrawNavigationDebug()
{
    // Draw debug visualization for navigation
    // This would show lane bindings and movement vectors
    // For Phase 7, we'll use basic debug drawing

    const FVector Center = GetActorLocation();

    // Draw lane search radius
    DrawDebugSphere(
        GetWorld(),
        Center,
        2000.0f,  // Lane search radius
        16,
        FColor::Green,
        false,
        -1.0f,
        0,
        1.0f
    );

    // Draw Smart Object search radius
    if (bEnableSmartObjects)
    {
        DrawDebugSphere(
            GetWorld(),
            Center,
            1000.0f,  // Smart Object search radius
            12,
            FColor::Cyan,
            false,
            -1.0f,
            0,
            1.0f
        );
    }
}

void AGSDCrowdTestbedActor::SpawnHeroNPCs()
{
    if (!HeroNPCClass)
    {
        UE_LOG(LOG_GSDCROWDS, Warning, TEXT("SpawnHeroNPCs: HeroNPCClass not set"));
        return;
    }

    const FVector SpawnCenter = GetActorLocation();

    for (int32 i = 0; i < HeroNPCCount; ++i)
    {
        // Random position in spawn area
        const FVector RandomOffset = FMath::VRand() * FMath::RandRange(500.0f, SpawnRadius);
        const FVector SpawnLocation = SpawnCenter + RandomOffset;

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AGSDHeroNPC* HeroNPC = GetWorld()->SpawnActor<AGSDHeroNPC>(
            HeroNPCClass,
            SpawnLocation,
            FRotator::ZeroRotator,
            SpawnParams
        );

        if (HeroNPC)
        {
            SpawnedHeroNPCs.Add(HeroNPC);
            UE_LOG(LOG_GSDCROWDS, Log, TEXT("Spawned Hero NPC %d at %s"), i, *SpawnLocation.ToString());
        }
    }

    UE_LOG(LOG_GSDCROWDS, Log, TEXT("Spawned %d Hero NPCs"), SpawnedHeroNPCs.Num());
}

void AGSDCrowdTestbedActor::DespawnHeroNPCs()
{
    for (AGSDHeroNPC* HeroNPC : SpawnedHeroNPCs)
    {
        if (HeroNPC)
        {
            HeroNPC->Destroy();
        }
    }
    SpawnedHeroNPCs.Empty();

    UE_LOG(LOG_GSDCROWDS, Log, TEXT("Despawned all Hero NPCs"));
}
