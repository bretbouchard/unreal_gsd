// Copyright Bret Bouchard. All Rights Reserved.

#include "Subsystems/GSDEventSpawnRegistry.h"
#include "DataAssets/GSDEventSpawnZone.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "NavigationSystem.h"
#include "GSDEventLog.h"

void UGSDEventSpawnRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Load config
    SpawnConfig = FGSDEventSpawnConfig();

    // Choose loading strategy based on config
    if (SpawnConfig.bUseAsyncLoading)
    {
        LoadZonesAsync();
    }
    else
    {
        LoadZonesSync();
    }
}

void UGSDEventSpawnRegistry::Deinitialize()
{
    // Wait for any pending loads
    while (bIsLoading.load())
    {
        FPlatformProcess::Sleep(0.001f);
    }

    // Clear all zones and cache
    FWriteScopeLock WriteLock(ZoneCacheLock);
    LoadedZones.Empty();
    CachedAllZones.Empty();
    ZoneCache.Empty();
    bZonesLoaded.store(false);

    Super::Deinitialize();
}

void UGSDEventSpawnRegistry::SetSpawnConfig(const FGSDEventSpawnConfig& NewConfig)
{
    SpawnConfig = NewConfig;

    // If config changed, refresh zones
    RefreshZones();
}

const TArray<UGSDEventSpawnZone*>& UGSDEventSpawnRegistry::GetAllSpawnZones() const
{
    FReadScopeLock ReadLock(ZoneCacheLock);
    return CachedAllZones;
}

int32 UGSDEventSpawnRegistry::GetZoneCount() const
{
    FReadScopeLock ReadLock(ZoneCacheLock);
    return LoadedZones.Num();
}

void UGSDEventSpawnRegistry::GetCompatibleZones(const FGameplayTag& EventTag, TArray<UGSDEventSpawnZone*>& OutZones) const
{
    FReadScopeLock ReadLock(ZoneCacheLock);

    OutZones.Empty();

    // Check if we have cached zones for this tag
    if (const FGSDEventTagZoneCache* TagCache = ZoneCache.Find(EventTag))
    {
        if (TagCache->bIsValid)
        {
            OutZones.Append(TagCache->Zones);
            return;
        }
    }

    // Fallback: return all zones (empty tag = any event)
    OutZones.Append(CachedAllZones);
}

FVector UGSDEventSpawnRegistry::GetSpawnLocationForEvent(const FGameplayTag& EventTag, FRandomStream& Stream, UWorld* World) const
{
    // Get compatible zones (thread-safe)
    TArray<UGSDEventSpawnZone*> CompatibleZones;
    GetCompatibleZones(EventTag, CompatibleZones);

    // Fallback: No zones defined - use world center with random offset
    if (CompatibleZones.Num() == 0)
    {
        GSDEVENT_LOG(Warning, TEXT("No spawn zones found for event %s, using fallback location"),
            *EventTag.ToString());

        return FVector(
            Stream.FRandRange(-2000.0f, 2000.0f),
            Stream.FRandRange(-2000.0f, 2000.0f),
            0.0f
        );
    }

    // Get cached total weight
    float TotalWeight = CachedTotalWeight;
    if (const FGSDEventTagZoneCache* TagCache = ZoneCache.Find(EventTag))
    {
        if (TagCache->bIsValid)
        {
            TotalWeight = TagCache->TotalWeight;
        }
    }

    // Select zone using weighted random (thread-safe)
    UGSDEventSpawnZone* SelectedZone = SelectWeightedZone(CompatibleZones, TotalWeight, Stream);
    if (!SelectedZone)
    {
        return FVector::ZeroVector;
    }

    // Get random point within zone
    FVector SpawnPoint = SelectedZone->GetRandomPointInZone(Stream);

    // Project to NavMesh if required (with retry logic)
    if (SelectedZone->bRequireNavMesh && World)
    {
        SpawnPoint = ProjectToNavMeshWithRetry(World, SpawnPoint, SelectedZone->NavMeshQueryExtent);
    }

    GSDEVENT_LOG(Verbose, TEXT("Selected spawn zone '%s' for event %s at location %s"),
        *SelectedZone->ZoneName.ToString(), *EventTag.ToString(), *SpawnPoint.ToString());

    return SpawnPoint;
}

void UGSDEventSpawnRegistry::RefreshZones()
{
    if (bIsLoading.load())
    {
        GSDEVENT_LOG(Warning, TEXT("RefreshZones called while already loading"));
        return;
    }

    bZonesLoaded.store(false);

    if (SpawnConfig.bUseAsyncLoading)
    {
        LoadZonesAsync();
    }
    else
    {
        LoadZonesSync();
    }
}

void UGSDEventSpawnRegistry::LoadZonesAsync()
{
    bIsLoading.store(true);

    GSDEVENT_LOG(Log, TEXT("Starting async load of spawn zones..."));

    // Get asset registry
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    // Wait for registry to finish loading
    AssetRegistry.SearchAllAssets(true);

    // Find all UGSDEventSpawnZone assets
    FARFilter Filter;
    Filter.ClassPaths.Add(UGSDEventSpawnZone::StaticClass()->GetClassPathName());
    Filter.bRecursiveClasses = true;
    Filter.bIncludeOnlyOnDiskAssets = false;

    // Add configured search paths
    for (const FName& Path : SpawnConfig.SpawnZoneSearchPaths)
    {
        Filter.PackagePaths.Add(*Path.ToString());
    }

    TArray<FAssetData> AssetDataList;
    AssetRegistry.GetAssets(Filter, AssetDataList);

    GSDEVENT_LOG(Log, TEXT("Found %d spawn zone assets in registry"), AssetDataList.Num());

    if (AssetDataList.Num() == 0)
    {
        bIsLoading.store(false);
        bZonesLoaded.store(true);
        GSDEVENT_LOG(Warning, TEXT("No spawn zones found"));
        return;
    }

    // Build soft object paths for async loading
    TArray<FSoftObjectPath> AssetPaths;
    AssetPaths.Reserve(AssetDataList.Num());

    for (const FAssetData& AssetData : AssetDataList)
    {
        AssetPaths.Add(AssetData.ToSoftObjectPath());
    }

    // Use streamable manager for async load
    FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();

    StreamableManager.RequestAsyncLoad(
        AssetPaths,
        FStreamableDelegate::CreateLambda([this, AssetDataList]()
        {
            TArray<UGSDEventSpawnZone*> LoadedZonesTmp;

            for (const FAssetData& AssetData : AssetDataList)
            {
                if (UGSDEventSpawnZone* Zone = Cast<UGSDEventSpawnZone>(AssetData.GetAsset()))
                {
                    FString ValidationError;
                    if (Zone->ValidateConfig(ValidationError))
                    {
                        LoadedZonesTmp.Add(Zone);
                        GSDEVENT_LOG(Log, TEXT("Loaded spawn zone: %s (Priority: %d)"),
                            *Zone->ZoneName.ToString(), Zone->Priority);
                    }
                    else
                    {
                        GSDEVENT_LOG(Warning, TEXT("Spawn zone '%s' failed validation: %s"),
                            *AssetData.AssetName.ToString(), *ValidationError);
                    }
                }
            }

            // Process loaded zones on game thread
            AsyncTask(ENamedThreads::GameThread, [this, LoadedZonesTmp]()
            {
                ProcessLoadedZones(LoadedZonesTmp);
                bIsLoading.store(false);
                bZonesLoaded.store(true);

                GSDEVENT_LOG(Log, TEXT("GSDEventSpawnRegistry initialized with %d zones"), LoadedZones.Num());

                // Broadcast loaded event
                OnZonesLoaded.Broadcast();
            });
        }),
        FStreamableManager::DefaultAsyncLoadPriority,
        false,
        nullptr,
        SpawnConfig.AsyncLoadTimeoutSeconds
    );
}

void UGSDEventSpawnRegistry::LoadZonesSync()
{
    bIsLoading.store(true);

    TArray<UGSDEventSpawnZone*> LoadedZonesTmp;

    // Get asset registry
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    // Wait for registry to finish loading
    AssetRegistry.SearchAllAssets(true);

    // Find all UGSDEventSpawnZone assets
    FARFilter Filter;
    Filter.ClassPaths.Add(UGSDEventSpawnZone::StaticClass()->GetClassPathName());
    Filter.bRecursiveClasses = true;
    Filter.bIncludeOnlyOnDiskAssets = false;

    // Add configured search paths
    for (const FName& Path : SpawnConfig.SpawnZoneSearchPaths)
    {
        Filter.PackagePaths.Add(*Path.ToString());
    }

    TArray<FAssetData> AssetDataList;
    AssetRegistry.GetAssets(Filter, AssetDataList);

    GSDEVENT_LOG(Log, TEXT("Found %d spawn zone assets in registry"), AssetDataList.Num());

    // Load each zone synchronously
    for (const FAssetData& AssetData : AssetDataList)
    {
        if (UGSDEventSpawnZone* Zone = Cast<UGSDEventSpawnZone>(AssetData.GetAsset()))
        {
            FString ValidationError;
            if (Zone->ValidateConfig(ValidationError))
            {
                LoadedZonesTmp.Add(Zone);
                GSDEVENT_LOG(Log, TEXT("Loaded spawn zone: %s (Priority: %d)"),
                    *Zone->ZoneName.ToString(), Zone->Priority);
            }
            else
            {
                GSDEVENT_LOG(Warning, TEXT("Spawn zone '%s' failed validation: %s"),
                    *AssetData.AssetName.ToString(), *ValidationError);
            }
        }
    }

    ProcessLoadedZones(LoadedZonesTmp);
    bIsLoading.store(false);
    bZonesLoaded.store(true);

    GSDEVENT_LOG(Log, TEXT("GSDEventSpawnRegistry initialized with %d zones"), LoadedZones.Num());

    // Broadcast loaded event
    OnZonesLoaded.Broadcast();
}

void UGSDEventSpawnRegistry::ProcessLoadedZones(const TArray<UGSDEventSpawnZone*>& Zones)
{
    FWriteScopeLock WriteLock(ZoneCacheLock);

    // Update loaded zones
    LoadedZones = Zones;

    // Sort for deterministic order
    SortZones();

    // Rebuild cache
    RebuildZoneCache();
}

void UGSDEventSpawnRegistry::SortZones()
{
    // Sort by Priority (descending), then ZoneName (alphabetical)
    LoadedZones.Sort([](const UGSDEventSpawnZone& A, const UGSDEventSpawnZone& B)
    {
        if (A.Priority != B.Priority)
        {
            return A.Priority > B.Priority;  // Higher priority first
        }
        return A.ZoneName.ToString() < B.ZoneName.ToString();  // Alphabetical for determinism
    });
}

void UGSDEventSpawnRegistry::RebuildZoneCache()
{
    // Must be called with write lock held

    // Cache all zones
    CachedAllZones = LoadedZones;
    CachedTotalWeight = CalculateTotalWeight(LoadedZones);

    // Clear old cache
    ZoneCache.Empty();

    // Build per-tag cache
    TMap<FGameplayTag, TArray<UGSDEventSpawnZone*>> ZonesByTag;

    for (UGSDEventSpawnZone* Zone : LoadedZones)
    {
        if (!Zone)
        {
            continue;
        }

        // Get compatible tags for this zone
        TArray<FGameplayTag> ZoneTags;
        Zone->CompatibleEventTags.GetGameplayTagArray(ZoneTags);

        if (ZoneTags.Num() == 0)
        {
            // Zone supports all events - add to all existing tag caches
            for (auto& Pair : ZonesByTag)
            {
                Pair.Value.Add(Zone);
            }
        }
        else
        {
            // Zone supports specific tags
            for (const FGameplayTag& Tag : ZoneTags)
            {
                ZonesByTag.FindOrAdd(Tag).Add(Zone);
            }
        }
    }

    // Build final cache structures
    for (auto& Pair : ZonesByTag)
    {
        FGSDEventTagZoneCache& TagCache = ZoneCache.Add(Pair.Key);
        TagCache.Zones = Pair.Value;
        TagCache.TotalWeight = CalculateTotalWeight(Pair.Value);
        TagCache.bIsValid = true;
    }
}

float UGSDEventSpawnRegistry::CalculateTotalWeight(const TArray<UGSDEventSpawnZone*>& Zones)
{
    float TotalWeight = 0.0f;
    for (const UGSDEventSpawnZone* Zone : Zones)
    {
        if (Zone)
        {
            // Weight = Priority + 1 (ensures all zones have some chance)
            TotalWeight += FMath::Max(1.0f, static_cast<float>(Zone->Priority + 1));
        }
    }
    return TotalWeight;
}

UGSDEventSpawnZone* UGSDEventSpawnRegistry::SelectWeightedZone(const TArray<UGSDEventSpawnZone*>& Zones, float TotalWeight, FRandomStream& Stream) const
{
    if (Zones.Num() == 0 || TotalWeight <= 0.0f)
    {
        return nullptr;
    }

    float Selection = Stream.FRand() * TotalWeight;
    float Accumulated = 0.0f;

    for (UGSDEventSpawnZone* Zone : Zones)
    {
        if (!Zone)
        {
            continue;
        }

        Accumulated += FMath::Max(1.0f, static_cast<float>(Zone->Priority + 1));
        if (Selection <= Accumulated)
        {
            return Zone;
        }
    }

    // Fallback to last zone (should not reach here if weights are correct)
    return Zones.Last();
}

FVector UGSDEventSpawnRegistry::ProjectToNavMeshWithRetry(UWorld* World, const FVector& Point, float QueryExtent) const
{
    if (!World)
    {
        return Point;
    }

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!NavSys)
    {
        GSDEVENT_LOG(Warning, TEXT("NavMesh required but NavigationSystem not found"));
        return Point;
    }

    FNavLocation ProjectedLocation;
    const FVector Extent(QueryExtent);

    // Try projection with retry logic
    for (int32 Retry = 0; Retry < SpawnConfig.MaxNavMeshRetries; ++Retry)
    {
        if (NavSys->ProjectPointToNavigation(Point, ProjectedLocation, Extent))
        {
            return ProjectedLocation.Location;
        }

        // Wait before retry
        if (Retry < SpawnConfig.MaxNavMeshRetries - 1)
        {
            FPlatformProcess::Sleep(SpawnConfig.NavMeshRetryDelayMs / 1000.0f);
        }
    }

    GSDEVENT_LOG(Warning, TEXT("Failed to project point %s to NavMesh after %d retries"),
        *Point.ToString(), SpawnConfig.MaxNavMeshRetries);
    return Point;
}

void FGSDEventSpawnZoneRegistrySearch::Search(const TArray<FName>& SearchPaths)
{
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    FARFilter Filter;
    Filter.ClassPaths.Add(UGSDEventSpawnZone::StaticClass()->GetClassPathName());
    Filter.bRecursiveClasses = true;

    for (const FName& Path : SearchPaths)
    {
        Filter.PackagePaths.Add(*Path.ToString());
    }

    AssetRegistry.GetAssets(Filter, FoundAssets);
}

//-- Network Validation (GSDNETWORK-107) --

bool UGSDEventSpawnRegistry::ValidateEventTag(const FGameplayTag& EventTag, FString& OutError) const
{
    OutError.Empty();

    // Validate event tag is valid
    if (!EventTag.IsValid())
    {
        OutError = TEXT("Event tag is not valid");
        GSDEVENT_LOG(Warning, TEXT("ValidateEventTag: %s"), *OutError);
        return false;
    }

    // Check if we have zones loaded for this tag
    FReadScopeLock ReadLock(ZoneCacheLock);

    if (!bZonesLoaded.load())
    {
        OutError = TEXT("Spawn zones not yet loaded");
        GSDEVENT_LOG(Warning, TEXT("ValidateEventTag: %s"), *OutError);
        return false;
    }

    // Check if tag has compatible zones (unless it's a wildcard/empty tag)
    if (const FGSDEventTagZoneCache* TagCache = ZoneCache.Find(EventTag))
    {
        if (TagCache->Zones.Num() == 0)
        {
            OutError = FString::Printf(TEXT("No spawn zones configured for event tag %s"), *EventTag.ToString());
            GSDEVENT_LOG(Warning, TEXT("ValidateEventTag: %s"), *OutError);
            return false;
        }

        if (TagCache->Zones.Num() > MaxZonesPerTag)
        {
            OutError = FString::Printf(TEXT("Too many zones (%d) for event tag %s (max %d)"),
                TagCache->Zones.Num(), *EventTag.ToString(), MaxZonesPerTag);
            GSDEVENT_LOG(Warning, TEXT("ValidateEventTag: %s"), *OutError);
            return false;
        }
    }

    return true;
}

bool UGSDEventSpawnRegistry::ValidateSpawnZone(UGSDEventSpawnZone* Zone, FString& OutError) const
{
    OutError.Empty();

    // Validate zone is not null
    if (!Zone)
    {
        OutError = TEXT("Spawn zone is null");
        GSDEVENT_LOG(Warning, TEXT("ValidateSpawnZone: %s"), *OutError);
        return false;
    }

    // Validate zone is in our loaded list (prevent arbitrary zone injection)
    FReadScopeLock ReadLock(ZoneCacheLock);

    if (!LoadedZones.Contains(Zone))
    {
        OutError = FString::Printf(TEXT("Spawn zone '%s' is not registered"), *Zone->ZoneName.ToString());
        GSDEVENT_LOG(Warning, TEXT("ValidateSpawnZone: %s"), *OutError);
        return false;
    }

    // Validate zone center is within world bounds
    if (FMath::Abs(Zone->Center.X) > MaxSpawnExtent ||
        FMath::Abs(Zone->Center.Y) > MaxSpawnExtent ||
        FMath::Abs(Zone->Center.Z) > MaxSpawnExtent)
    {
        OutError = FString::Printf(TEXT("Spawn zone '%s' center %s exceeds world bounds"),
            *Zone->ZoneName.ToString(), *Zone->Center.ToString());
        GSDEVENT_LOG(Warning, TEXT("ValidateSpawnZone: %s"), *OutError);
        return false;
    }

    // Validate zone radius is reasonable
    if (Zone->Radius <= 0.0f || Zone->Radius > MaxSpawnExtent)
    {
        OutError = FString::Printf(TEXT("Spawn zone '%s' has invalid radius %.2f"),
            *Zone->ZoneName.ToString(), Zone->Radius);
        GSDEVENT_LOG(Warning, TEXT("ValidateSpawnZone: %s"), *OutError);
        return false;
    }

    return true;
}
