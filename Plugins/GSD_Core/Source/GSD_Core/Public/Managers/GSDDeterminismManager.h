#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GSDDeterminismManager.generated.h"

/**
 * Determinism Manager Subsystem
 * Provides seeded RNG streams for reproducible runs.
 * MUST be first system initialized.
 */
UCLASS()
class GSD_CORE_API UGSDDeterminismManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Initialize with a specific seed
    UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
    void InitializeWithSeed(int32 InSeed);

    // Get category-specific RNG stream (isolated from other categories)
    UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
    FRandomStream& GetStream(FName Category);

    // Get the current seed
    UFUNCTION(BlueprintPure, Category = "GSD|Determinism")
    int32 GetCurrentSeed() const { return CurrentSeed; }

    // Compute state hash for verification
    UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
    int32 ComputeStateHash() const;

    // Get random float from category stream
    UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
    float RandomFloat(FName Category);

    // Get random integer from category stream
    UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
    int32 RandomInteger(FName Category, int32 Max);

    // Get random bool from category stream
    UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
    bool RandomBool(FName Category);

    // Get random vector from category stream
    UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
    FVector RandomUnitVector(FName Category);

    // Shuffle array using category stream
    template<typename T>
    void ShuffleArray(FName Category, TArray<T>& Array);

    // Reset a specific category stream
    UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
    void ResetStream(FName Category);

    // Reset all streams
    UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
    void ResetAllStreams();

    // Predefined categories
    static const FName SpawnCategory;
    static const FName EventCategory;
    static const FName CrowdCategory;
    static const FName VehicleCategory;

private:
    int32 CurrentSeed = 0;
    TMap<FName, FRandomStream> CategoryStreams;

    // Hash accumulator for state verification
    UPROPERTY()
    int32 StateHash = 0;

    void CreateCategoryStream(FName Category);
};
