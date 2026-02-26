// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntity/Processors/MassProcessor.h"
#include "GSDSmartObjectProcessor.generated.h"

struct FGSDSmartObjectFragment;
struct FGSDNavigationFragment;
struct FDataFragment_Transform;
class UGSDSmartObjectSubsystem;

/**
 * Processor for Smart Object interactions.
 * Handles search, claim, interact, release lifecycle.
 *
 * Lifecycle:
 * 1. Search for nearby Smart Objects (with cooldown)
 * 2. Claim available Smart Object
 * 3. Process interaction progress
 * 4. Release Smart Object when interaction completes
 *
 * CRITICAL: Smart Objects MUST be released when interaction completes
 * to prevent resource leaks and allow other entities to use them.
 */
UCLASS()
class GSD_CROWDS_API UGSDSmartObjectProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UGSDSmartObjectProcessor();

    // ~UMassProcessor interface
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
    // ~End of UMassProcessor interface

protected:
    //-- Helper Methods --

    /**
     * Search for nearby Smart Objects.
     * Respects search cooldown to prevent constant queries.
     *
     * @param SOFragment Smart Object fragment to update
     * @param Transform Entity transform for location
     * @param SOSubsystem Smart Object subsystem for queries
     * @param DeltaTime Frame delta time
     */
    void SearchForSmartObject(
        FGSDSmartObjectFragment& SOFragment,
        const FDataFragment_Transform& Transform,
        UGSDSmartObjectSubsystem* SOSubsystem,
        float DeltaTime);

    /**
     * Claim a Smart Object.
     * Thread-safe claim operation.
     *
     * @param SOFragment Smart Object fragment to update
     * @param Transform Entity transform for location
     * @param SOSubsystem Smart Object subsystem for claiming
     */
    void TryClaimSmartObject(
        FGSDSmartObjectFragment& SOFragment,
        const FDataFragment_Transform& Transform,
        UGSDSmartObjectSubsystem* SOSubsystem);

    /**
     * Process interaction progress.
     * Updates interaction time and sets completion flag.
     *
     * @param SOFragment Smart Object fragment to update
     * @param NavFragment Navigation fragment (speed paused during interaction)
     * @param DeltaTime Frame delta time
     */
    void ProcessInteraction(
        FGSDSmartObjectFragment& SOFragment,
        FGSDNavigationFragment& NavFragment,
        float DeltaTime);

    /**
     * Release claimed Smart Object.
     * MUST be called when interaction completes.
     *
     * @param SOFragment Smart Object fragment to update
     * @param SOSubsystem Smart Object subsystem for releasing
     */
    void ReleaseSmartObject(
        FGSDSmartObjectFragment& SOFragment,
        UGSDSmartObjectSubsystem* SOSubsystem);

private:
    FMassEntityQuery EntityQuery;

    //-- Configuration --
    UPROPERTY(EditDefaultsOnly, Category = "Configuration")
    float DefaultSearchRadius = 1000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Configuration")
    float DefaultInteractionDuration = 3.0f;
};
