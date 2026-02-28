// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntitySubsystem.h"
#include "GameplayTagContainer.h"
#include "GSDCrowdManagerSubsystem.generated.h"

class UGSDCrowdEntityConfig;
class UWorldPartitionSubsystem;
struct FMassEntityHandle;

/**
 * Struct for tracking active density modifiers (EVT-09).
 */
USTRUCT(BlueprintType)
struct GSD_CROWDS_API FGSDensityModifier
{
    GENERATED_BODY()

    UPROPERTY()
    FGameplayTag ModifierTag;

    UPROPERTY()
    FVector Center = FVector::ZeroVector;

    UPROPERTY()
    float Radius = 1000.0f;

    UPROPERTY()
    float Multiplier = 1.0f;
};

/**
 * Delegate for async crowd spawn completion.
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnCrowdSpawnComplete, int32, NumSpawned);

/**
 * Delegate for all entities despawned notification.
 */
DECLARE_DYNAMIC_DELEGATE(FOnAllEntitiesDespawned);

/**
 * World subsystem for spawning and managing crowd entities.
 *
 * Provides centralized entity spawning from GSDCrowdEntityConfig Data Assets
 * with tracking and cleanup. Uses Mass Entity subsystem for efficient ECS-based crowds.
 *
 * Usage:
 * 1. Get subsystem: GetWorld()->GetSubsystem<UGSDCrowdManagerSubsystem>()
 * 2. Spawn entities: Subsystem->SpawnEntities(200, Location, Radius)
 * 3. Track entities: Subsystem->GetActiveEntityCount()
 * 4. Cleanup: Subsystem->DespawnAllEntities()
 */
UCLASS()
class GSD_CROWDS_API UGSDCrowdManagerSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    //-- Spawning --

    /**
     * Spawn crowd entities in a circular area around a center point.
     *
     * @param Count Number of entities to spawn
     * @param Center World location for spawn area center
     * @param Radius Radius of spawn area (entities randomly placed within)
     * @param EntityConfig Optional entity config (uses default if nullptr)
     * @return Number of entities successfully spawned
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds")
    int32 SpawnEntities(int32 Count, FVector Center, float Radius, UGSDCrowdEntityConfig* EntityConfig = nullptr);

    /**
     * Spawn entities asynchronously with completion callback.
     *
     * @param Count Number of entities to spawn
     * @param Center World location for spawn area center
     * @param Radius Radius of spawn area
     * @param EntityConfig Optional entity config
     * @param OnComplete Delegate called when spawn completes
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds")
    void SpawnEntitiesAsync(int32 Count, FVector Center, float Radius, UGSDCrowdEntityConfig* EntityConfig, const FOnCrowdSpawnComplete& OnComplete);

    /**
     * Despawn all tracked entities.
     * Uses Defer() for thread-safe destruction.
     * Broadcasts OnAllEntitiesDespawned when complete.
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds")
    void DespawnAllEntities();

    //-- Queries --

    /**
     * Get count of active spawned entities.
     *
     * @return Number of tracked entities
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds")
    int32 GetActiveEntityCount() const { return SpawnedEntityHandles.Num(); }

    /**
     * Get delegate for all entities despawned notification.
     */
    FOnAllEntitiesDespawned& GetOnAllEntitiesDespawned() { return AllEntitiesDespawnedDelegate; }

    //-- Density Modifiers (EVT-09) --

    /**
     * Add a density modifier for spawn calculations.
     * Modifiers stack multiplicatively within their radius.
     *
     * @param ModifierTag Unique tag for this modifier (used for removal)
     * @param Center World location of modifier center
     * @param Radius Area of effect
     * @param Multiplier Density multiplier (2.0 = double spawn rate)
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Density")
    void AddDensityModifier(FGameplayTag ModifierTag, FVector Center, float Radius, float Multiplier);

    /**
     * Remove a density modifier by tag.
     * Must be called when event ends to prevent state leaks.
     *
     * @param ModifierTag Tag of modifier to remove
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Crowds|Density")
    void RemoveDensityModifier(FGameplayTag ModifierTag);

    /**
     * Get the combined density multiplier at a world location.
     * Iterates all active modifiers and multipliers their effects.
     *
     * @param Location World location to check
     * @return Combined density multiplier (1.0 = no change)
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds|Density")
    float GetDensityMultiplierAtLocation(FVector Location) const;

    /**
     * Get all active density modifiers.
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds|Density")
    const TArray<FGSDensityModifier>& GetActiveDensityModifiers() const { return ActiveDensityModifiers; }

    //-- Network Validation (GSDNETWORK-107) --
    // These validation functions can be called from Server RPCs to prevent exploitable inputs.
    // Subsystems cannot have Server RPCs directly, so actors call these for validation.

    /**
     * Validate spawn parameters for network safety.
     * Call from Server RPCs before calling SpawnEntities.
     *
     * @param Count Number of entities to spawn
     * @param Center Spawn center location
     * @param Radius Spawn radius
     * @param OutError Error message if validation fails
     * @return True if parameters are valid and safe
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds|Network")
    bool ValidateSpawnParameters(int32 Count, FVector Center, float Radius, FString& OutError) const;

    /**
     * Validate density modifier parameters for network safety.
     * Call from Server RPCs before calling AddDensityModifier.
     *
     * @param Center Modifier center location
     * @param Radius Modifier radius
     * @param Multiplier Density multiplier
     * @param OutError Error message if validation fails
     * @return True if parameters are valid and safe
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds|Network")
    bool ValidateDensityModifier(FVector Center, float Radius, float Multiplier, FString& OutError) const;

    /**
     * Get maximum allowed crowd size for spawn validation.
     */
    static constexpr int32 GetMaxCrowdSize() { return MaxCrowdSize; }

    /**
     * Get maximum allowed spawn radius for validation.
     */
    static constexpr float GetMaxSpawnRadius() { return MaxSpawnRadius; }

    //-- World Partition Streaming Integration --

    /**
     * Check if a position is in a loaded streaming cell.
     * Returns true if no streaming system active or cell is loaded.
     *
     * @param Position World position to check
     * @return True if position is in loaded cell (or no streaming)
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds|Streaming")
    bool IsPositionInLoadedCell(const FVector& Position) const;

    /**
     * Get the cell name for a world position.
     * Uses grid-based cell naming (Cell_X_Y).
     *
     * @param Position World position
     * @return Cell name (e.g., "Cell_10_20")
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds|Streaming")
    FName GetCellNameForPosition(const FVector& Position) const;

    /**
     * Bind to World Partition streaming events.
     * Call during subsystem initialization.
     */
    void BindToStreamingEvents();

    /**
     * Unbind from World Partition streaming events.
     * Call during subsystem deinitialization.
     */
    void UnbindFromStreamingEvents();

protected:
    //-- Validation Constants (GSDNETWORK-107) --
    // Maximum crowd size to prevent server overload exploits
    static constexpr int32 MaxCrowdSize = 500;
    // Maximum spawn radius (10km) to prevent spawning in unloaded areas
    static constexpr float MaxSpawnRadius = 10000.0f;
    // Maximum density multiplier to prevent performance exploits
    static constexpr float MaxDensityMultiplier = 10.0f;
    // Maximum density modifier radius
    static constexpr float MaxDensityRadius = 5000.0f;

    //-- Tracked Entities --
    UPROPERTY()
    TArray<FMassEntityHandle> SpawnedEntityHandles;

    //-- Density Modifiers (EVT-09) --
    UPROPERTY()
    TArray<FGSDensityModifier> ActiveDensityModifiers;

    //-- Delegates --
    FOnAllEntitiesDespawned AllEntitiesDespawnedDelegate;

    //-- Streaming Cell Tracking --
    // Set of currently loaded cell names
    TSet<FName> LoadedCellNames;

    // Mapping from cell name to crowd entity IDs
    TMap<FName, TArray<int32>> CellToCrowdMapping;

    // Pending spawns for unloaded cells
    UPROPERTY()
    TArray<FVector> PendingSpawnCenters;

    // World Partition subsystem reference
    TWeakObjectPtr<UWorldPartitionSubsystem> WorldPartitionSubsystem;

    // ~UWorldSubsystem interface
    virtual bool ShouldCreateSubsystem(UWorld* World) const override;
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    // ~End of UWorldSubsystem interface

private:

    /**
     * Handle cell loaded event from World Partition.
     * @param CellName Name of the loaded cell
     */
    void OnCellLoaded(const FName& CellName);

    /**
     * Handle cell unloaded event from World Partition.
     * @param CellName Name of the unloaded cell
     */
    void OnCellUnloaded(const FName& CellName);

    /**
     * Spawn entities internally (without cell checks).
     * Used for pending spawns and normal spawns.
     */
    int32 SpawnEntitiesInternal(int32 Count, FVector Center, float Radius, UGSDCrowdEntityConfig* EntityConfig);
    /**
     * Get default entity config asset.
     * Loads from /GSD_Crowds/EntityConfigs/BP_GSDZombieEntityConfig
     *
     * @return Default entity config, or nullptr if not found
     */
    UGSDCrowdEntityConfig* GetDefaultEntityConfig();

    /**
     * Generate random spawn transforms in a circular area.
     *
     * @param Count Number of transforms to generate
     * @param Center Center of spawn area
     * @param Radius Radius of spawn area
     * @return Array of spawn transforms
     */
    TArray<FTransform> GenerateSpawnTransforms(int32 Count, FVector Center, float Radius) const;
};
