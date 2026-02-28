#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GSDDeterminismManager.generated.h"

/**
 * Struct for recording random calls during replay validation.
 */
USTRUCT(BlueprintType)
struct GSD_CORE_API FGSDRandomCallRecord
{
    GENERATED_BODY()

    UPROPERTY()
    FName Category;

    UPROPERTY()
    int32 CallIndex = 0;

    UPROPERTY()
    float FloatValue = 0.0f;

    UPROPERTY()
    FVector VectorValue = FVector::ZeroVector;

    UPROPERTY()
    bool bIsVector = false;
};

/**
 * Determinism Manager Subsystem
 * Provides seeded RNG streams for reproducible runs.
 * MUST be first system initialized.
 *
 * Usage:
 * 1. Get subsystem: GameInstance->GetSubsystem<UGSDDeterminismManager>()
 * 2. Initialize seed: InitializeWithSeed(12345)
 * 3. Get category stream: GetCategoryStream("ZombieWander")
 * 4. Use seeded random: Stream.VRand(), Stream.FRandRange(0.0f, 100.0f)
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

    // Alias for GetStream - for API clarity
    FRandomStream& GetCategoryStream(FName Category) { return GetStream(Category); }

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

    //-- Recording for Debugging/Replay Validation --

    /**
     * Record a random float call for replay validation.
     * Only records when bIsRecording is true.
     */
    void RecordRandomCall(FName Category, float Value);

    /**
     * Record a random vector call for replay validation.
     * Only records when bIsRecording is true.
     */
    void RecordRandomCall(FName Category, FVector Value);

    /**
     * Get all recorded random calls for debugging.
     */
    const TArray<FGSDRandomCallRecord>& GetRecordedCalls() const { return RecordedCalls; }

    /**
     * Clear all recorded calls.
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
    void ClearRecordedCalls();

    /**
     * Enable/disable call recording for debugging.
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
    void SetRecordingEnabled(bool bEnabled) { bIsRecording = bEnabled; }

    /**
     * Check if recording is enabled.
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Determinism")
    bool IsRecordingEnabled() const { return bIsRecording; }

    // Predefined categories
    static const FName SpawnCategory;
    static const FName EventCategory;
    static const FName CrowdCategory;
    static const FName VehicleCategory;

    // Crowd-specific categories (for fine-grained debugging)
    static const FName CrowdSpawnCategory;
    static const FName CrowdLODCategory;
    static const FName CrowdVelocityCategory;
    static const FName ZombieWanderCategory;
    static const FName ZombieTargetCategory;
    static const FName ZombieBehaviorCategory;
    static const FName ZombieSpeedCategory;

private:
    int32 CurrentSeed = 0;
    TMap<FName, FRandomStream> CategoryStreams;

    // Hash accumulator for state verification
    UPROPERTY()
    int32 StateHash = 0;

    //-- Recording State --
    UPROPERTY()
    TArray<FGSDRandomCallRecord> RecordedCalls;

    UPROPERTY()
    bool bIsRecording = false;

    int32 CallCounter = 0;

    void CreateCategoryStream(FName Category);
};
