// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityConfigAsset.h"
#include "GSDCrowdEntityConfig.generated.h"

class UMassEntityConfigAsset;
class UStaticMesh;

struct FGSDNavigationFragment;
struct FGSDSmartObjectFragment;

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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
    bool bEnableFallbackMovement = true;

    //-- Velocity Randomization (CROWD-08) --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
    float VelocityRandomizationPercent = 20.0f;

    //-- Smart Object Configuration (Phase 7) --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SmartObjects")
    bool bEnableSmartObjectInteractions = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SmartObjects")
    float SmartObjectSearchRadius = 1000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SmartObjects")
    float SmartObjectSearchCooldown = 5.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SmartObjects")
    float DefaultInteractionDuration = 3.0f;

    //-- LOD Representation --
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LOD")
    TObjectPtr<UStaticMesh> HighDetailMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LOD")
    TObjectPtr<UStaticMesh> LowDetailMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LOD")
    TObjectPtr<UStaticMesh> ISMMesh;

    //-- Default Configuration --
    virtual void PostInitProperties() override;

    //-- Helper Methods (Phase 7) --

    /** Create a navigation fragment with config settings */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds")
    FGSDNavigationFragment CreateNavigationFragment() const;

    /** Create a smart object fragment with config settings */
    UFUNCTION(BlueprintPure, Category = "GSD|Crowds")
    FGSDSmartObjectFragment CreateSmartObjectFragment() const;

protected:
    // ~UMassEntityConfigAsset interface
    virtual void ConfigureFragmentTypes() override;
    // ~End of UMassEntityConfigAsset interface
};
