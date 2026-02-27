// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GSDHeroNPCArchetype.generated.h"

class UBehaviorTree;
class UGSDHeroAIConfig;
class USkeletalMesh;
class UMaterialInterface;
class UAnimInstance;

/**
 * Archetype definition for Hero NPCs.
 * Contains all configuration needed to spawn a fully-functional Hero NPC.
 *
 * Designers create these as DataAssets to define different NPC types
 * (Survivor, Soldier, Medic, etc.) without code changes.
 *
 * Usage:
 * 1. Create a DataAsset derived from GSDHeroNPCArchetype
 * 2. Configure visual appearance, stats, AI config
 * 3. Assign to spawner or spawn system
 * 4. System spawns fully-configured Hero NPC
 */
UCLASS(BlueprintType)
class GSD_CROWDS_API UGSDHeroNPCArchetype : public UDataAsset
{
    GENERATED_BODY()

public:
    // === Identity ===

    /** Display name for this NPC type */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
    FText DisplayName;

    /** Gameplay tag for this archetype (e.g., NPC.Hero.Survivor) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
    FGameplayTag ArchetypeTag;

    /** Description for designers */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
    FText Description;

    // === Visual ===

    /** Skeletal mesh for this NPC type */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
    TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

    /** Material overrides (slot -> material) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
    TMap<FName, TSoftObjectPtr<UMaterialInterface>> MaterialOverrides;

    /** Animation blueprint class */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
    TSoftClassPtr<UAnimInstance> AnimBlueprint;

    // === Stats ===

    /** Maximum health */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
    float MaxHealth = 100.0f;

    /** Attack damage per hit */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
    float AttackDamage = 25.0f;

    /** Movement speed multiplier (1.0 = normal) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
    float SpeedMultiplier = 1.0f;

    /** Damage resistance (0.0 = immune, 1.0 = normal) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
    float DamageResistance = 1.0f;

    // === AI Configuration ===

    /** AI configuration to use */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UGSDHeroAIConfig* AIConfig;

    /** Default behavior tree override (if different from AIConfig) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBehaviorTree* BehaviorTreeOverride;

    // === Interaction ===

    /** Can this NPC be interacted with by player? */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
    bool bPlayerCanInteract = true;

    /** Interaction radius (cm) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
    float InteractionRadius = 200.0f;

    /** Gameplay tags for interaction filtering */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
    FGameplayTagContainer InteractionTags;

    // === Validation ===

    /**
     * Validate archetype configuration.
     * @param OutError Error message if validation fails
     * @return True if configuration is valid
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|NPC")
    bool ValidateConfig(FString& OutError) const;

    /**
     * Get archetype by tag.
     * @param WorldContextObject World context for subsystem access
     * @param Tag Gameplay tag to search for
     * @return Archetype with matching tag, or nullptr if not found
     */
    UFUNCTION(BlueprintPure, Category = "GSD|NPC", meta = (WorldContext = "WorldContextObject"))
    static UGSDHeroNPCArchetype* GetArchetypeByTag(UObject* WorldContextObject, const FGameplayTag& Tag);
};
