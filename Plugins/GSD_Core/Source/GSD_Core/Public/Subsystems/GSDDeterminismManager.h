// Copyright Bret Bouchard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GSDDeterminismManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGSDDeterminism, Log, All);

/**
 * Record of a random call for replay validation
 */
USTRUCT(BlueprintType)
struct GSD_CORE_API FGSDRandomCallRecord
{
    GENERATED_BODY()

    UPROPERTY()
    FName Category;

    UPROPERTY()
    float FloatValue = 0.0f;

    UPROPERTY()
    FVector VectorValue = FVector::ZeroVector;

    UPROPERTY()
    bool bIsVector = false;

    UPROPERTY()
    int32 FrameNumber = 0;
};

/**
 * Determinism Manager Subsystem
 *
 * Provides seeded random streams for deterministic gameplay.
 * Each category (spawn, behavior, etc.) gets its own seeded stream
 * derived from the global seed, ensuring reproducible results.
 *
 * Usage:
 *   UGSDDeterminismManager* Manager = GetGameInstance()->GetSubsystem<UGSDDeterminismManager>();
 *   FRandomStream& Stream = Manager->GetCategoryStream(FName("CrowdSpawn"));
 *   float RandomValue = Stream.FRandRange(0.0f, 1.0f);
 */
UCLASS(Config = Game, DefaultConfig)
class GSD_CORE_API UGSDDeterminismManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // ~USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    // ~End USubsystem interface

    /**
     * Initialize the determinism system with a specific seed.
     * Call this at game start for reproducible runs.
     * @param Seed The global seed to use for all random streams
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
    void InitializeWithSeed(int32 Seed);

    /**
     * Get the global seed value
     * @return The current global seed
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Determinism")
    int32 GetGlobalSeed() const { return GlobalSeed; }

    /**
     * Get a seeded random stream for a specific category.
     * Each category gets its own deterministic stream derived from the global seed.
     * @param Category The category name (e.g., "CrowdSpawn", "ZombieWander")
     * @return Reference to the random stream for this category
     */
    FRandomStream& GetCategoryStream(FName Category);

    /**
     * Alias for GetCategoryStream for clearer API
     */
    FRandomStream& GetStream(FName Category) { return GetCategoryStream(Category); }

    /**
     * Record a random call for replay validation (float version)
     * @param Category The category of the random call
     * @param Value The value that was generated
     * @param bRecord Whether to actually record (default: true when bIsRecording)
     */
    void RecordRandomCall(FName Category, float Value, bool bRecord = true);

    /**
     * Record a random call for replay validation (FVector version)
     * @param Category The category of the random call
     * @param Value The vector value that was generated
     * @param bRecord Whether to actually record (default: true when bIsRecording)
     */
    void RecordRandomCall(FName Category, const FVector& Value, bool bRecord = true);

    /**
     * Enable or disable recording mode
     * @param bEnable Whether to enable recording
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
    void SetRecordingMode(bool bEnable) { bIsRecording = bEnable; }

    /**
     * Check if recording mode is enabled
     */
    UFUNCTION(BlueprintPure, Category = "GSD|Determinism")
    bool IsRecordingMode() const { return bIsRecording; }

    /**
     * Get all recorded calls for validation
     */
    const TArray<FGSDRandomCallRecord>& GetRecordedCalls() const { return RecordedCalls; }

    /**
     * Clear all recorded calls
     */
    UFUNCTION(BlueprintCallable, Category = "GSD|Determinism")
    void ClearRecordedCalls() { RecordedCalls.Empty(); }

    // Category constants for crowd and gameplay systems
    namespace GSDRandomCategories
    {
        inline const FName CrowdSpawn() { return FName("CrowdSpawn"); }
        inline const FName CrowdRotation() { return FName("CrowdRotation"); }
        inline const FName ZombieWander() { return FName("ZombieWander"); }
        inline const FName ZombieSpeed() { return FName("ZombieSpeed"); }
        inline const FName VehicleSpawn() { return FName("VehicleSpawn"); }
        inline const FName EventTrigger() { return FName("EventTrigger"); }
        inline const FName Gameplay() { return FName("Gameplay"); }
    }

protected:
    /** The global seed used to derive all category streams */
    UPROPERTY(Config)
    int32 GlobalSeed = 0;

    /** Whether to use time-based seed if no seed is set */
    UPROPERTY(Config)
    bool bUseTimeBasedSeed = true;

    /** Per-category random streams */
    UPROPERTY()
    TMap<FName, FRandomStream> CategoryStreams;

    /** Recorded calls for replay validation */
    UPROPERTY()
    TArray<FGSDRandomCallRecord> RecordedCalls;

    /** Whether we're in recording mode */
    UPROPERTY()
    bool bIsRecording = false;

    /** Whether the system has been initialized with a seed */
    bool bIsInitialized = false;

private:
    // Make Deinitialize accessible
    virtual void Deinitialize() override;
    /** Derive a category seed from the global seed */
    int32 DeriveCategorySeed(FName Category) const;
};
