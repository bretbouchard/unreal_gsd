#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GSDDataAsset.generated.h"

/**
 * Base class for all GSD configuration data assets.
 * All spawn configs, vehicle configs, crowd configs, etc. inherit from this.
 */
UCLASS(Abstract, BlueprintType, Category = "GSD")
class GSD_CORE_API UGSDDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UGSDDataAsset();

    // Get the display name for this config
    UFUNCTION(BlueprintPure, Category = "GSD")
    FString GetDisplayName() const;

    // Get the config version for migration support
    UFUNCTION(BlueprintPure, Category = "GSD")
    int32 GetConfigVersion() const { return ConfigVersion; }

    // Validate the config data
    UFUNCTION(BlueprintCallable, Category = "GSD")
    virtual bool ValidateConfig(FString& OutError) const;

    // Called after loading to apply any migrations
    UFUNCTION(BlueprintCallable, Category = "GSD")
    virtual void ApplyMigrations();

    // Get tags associated with this config
    UFUNCTION(BlueprintPure, Category = "GSD")
    const TArray<FName>& GetConfigTags() const { return ConfigTags; }

    // Check if config has a specific tag
    UFUNCTION(BlueprintPure, Category = "GSD")
    bool HasTag(FName Tag) const;

    // Get priority for spawning (higher = more important)
    UFUNCTION(BlueprintPure, Category = "GSD")
    int32 GetSpawnPriority() const { return SpawnPriority; }

protected:
    // Human-readable name for this config
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GSD")
    FString DisplayName;

    // Version number for config migration
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GSD")
    int32 ConfigVersion = 1;

    // Tags for categorization and filtering
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GSD")
    TArray<FName> ConfigTags;

    // Priority for spawn ordering
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GSD")
    int32 SpawnPriority = 0;

    // Description for documentation
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GSD", meta = (MultiLine = true))
    FString Description;
};
