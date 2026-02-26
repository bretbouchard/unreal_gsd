// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityConfigAsset.h"
#include "GSDCrowdEntityConfig.generated.h"

class UMassEntityConfigAsset;
class UStaticMesh;

/**
 * Data Asset defining crowd entity configuration for Mass Entity.
 *
 * This config defines what fragments and processors a crowd entity has.
 * Used by Mass Spawner to create entities with the correct composition.
 *
 * CRITICAL: Always add Velocity Randomizer trait to prevent synchronized movement.
 * See RESEARCH.md Pitfall 1 for details.
 */
UCLASS(BlueprintType, Category = "GSD|Crowds")
class GSD_CROWDS_API UGSDCrowdEntityConfig : public UMassEntityConfigAsset
{
    GENERATED_BODY()

public:
    UGSDCrowdEntityConfig();

    //-- Fragment Configuration --
    // Fragments are added via C++ or Blueprint in Mass Entity Config

    //-- Velocity Randomization (CRITICAL) --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    float BaseVelocity = 150.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    float VelocityRandomRange = 0.2f;

    //-- Navigation Configuration (Phase 7) --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
    bool bEnableNavigation = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
    float DefaultMoveSpeed = 150.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
    float LaneSearchRadius = 2000.0f;

    //-- Velocity Randomization (CROWD-08) --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
    float VelocityRandomizationPercent = 20.0f;

    //-- LOD Representation --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LOD")
    TObjectPtr<UStaticMesh> HighDetailMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LOD")
    TObjectPtr<UStaticMesh> LowDetailMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LOD")
    TObjectPtr<UStaticMesh> ISMMesh;

    //-- Default Configuration --
    virtual void PostInitProperties() override;

protected:
    // ~UMassEntityConfigAsset interface
    virtual void ConfigureFragmentTypes() override;
    // ~End of UMassEntityConfigAsset interface
};
