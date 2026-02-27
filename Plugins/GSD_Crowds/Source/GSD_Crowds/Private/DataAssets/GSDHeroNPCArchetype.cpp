// Copyright Bret Bouchard. All Rights Reserved.

#include "DataAssets/GSDHeroNPCArchetype.h"
#include "DataAssets/GSDHeroAIConfig.h"
#include "BehaviorTree/BehaviorTree.h"

bool UGSDHeroNPCArchetype::ValidateConfig(FString& OutError) const
{
    if (!SkeletalMesh.IsValid())
    {
        OutError = TEXT("SkeletalMesh is required");
        return false;
    }

    if (!AIConfig && !BehaviorTreeOverride)
    {
        OutError = TEXT("Either AIConfig or BehaviorTreeOverride is required");
        return false;
    }

    if (MaxHealth <= 0.0f)
    {
        OutError = TEXT("MaxHealth must be positive");
        return false;
    }

    if (AttackDamage < 0.0f)
    {
        OutError = TEXT("AttackDamage cannot be negative");
        return false;
    }

    if (SpeedMultiplier <= 0.0f)
    {
        OutError = TEXT("SpeedMultiplier must be positive");
        return false;
    }

    if (DamageResistance < 0.0f || DamageResistance > 1.0f)
    {
        OutError = TEXT("DamageResistance must be between 0.0 and 1.0");
        return false;
    }

    if (bPlayerCanInteract && InteractionRadius <= 0.0f)
    {
        OutError = TEXT("InteractionRadius must be positive when player interaction is enabled");
        return false;
    }

    return true;
}

UGSDHeroNPCArchetype* UGSDHeroNPCArchetype::GetArchetypeByTag(UObject* WorldContextObject, const FGameplayTag& Tag)
{
    // This would typically query a subsystem or asset registry
    // For now, return nullptr (implementation depends on project structure)
    // Future implementation could use:
    // - UAssetRegistry to find all GSDHeroNPCArchetype assets
    // - Filter by ArchetypeTag
    // - Cache results for performance

    return nullptr;
}
