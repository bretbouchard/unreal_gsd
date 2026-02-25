#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Types/GSDSpawnTypes.h"
#include "GSDSaveGame.generated.h"

/**
 * Serialized actor state for save/load
 */
USTRUCT(BlueprintType)
struct GSD_CORE_API FGSDSerializedActorState
{
    GENERATED_BODY()

    UPROPERTY(SaveGame)
    FName ActorName;

    UPROPERTY(SaveGame)
    FTransform Transform;

    UPROPERTY(SaveGame)
    TArray<uint8> ComponentState;

    UPROPERTY(SaveGame)
    FString ActorClassPath;

    UPROPERTY(SaveGame)
    bool bIsActive = true;
};

/**
 * Random call log for debugging determinism issues
 */
USTRUCT(BlueprintType)
struct GSD_CORE_API FGSDRandomCallLog
{
    GENERATED_BODY()

    UPROPERTY(SaveGame)
    FName Category;

    UPROPERTY(SaveGame)
    int32 CallCount = 0;

    UPROPERTY(SaveGame)
    int32 LastHash = 0;
};

/**
 * Main SaveGame object for the GSD platform
 */
UCLASS(BlueprintType)
class GSD_CORE_API UGSDSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UGSDSaveGame();

    // ~ Game identification
    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD")
    FString SaveName;

    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD")
    int32 SaveVersion = 1;

    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD")
    FDateTime SaveTimestamp;

    // ~ Determinism
    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD|Determinism")
    int32 GameSeed;

    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD|Determinism")
    int32 DeterminismHash;

    // ~ Spawn state
    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD|Spawning")
    TArray<FGSDSeededSpawnTicket> PendingSpawns;

    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD|Spawning")
    TArray<FGSDSerializedActorState> ActorStates;

    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD|Spawning")
    int32 TotalSpawnCount = 0;

    // ~ Random history for debugging
    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD|Debug")
    TArray<FGSDRandomCallLog> RandomHistory;

    // ~ Game time
    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD|Game")
    float GameTime = 0.0f;

    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD|Game")
    int32 GameDay = 1;

    // ~ Custom data (for game-specific data)
    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "GSD|Custom")
    TArray<uint8> CustomData;

    // Helper functions
    UFUNCTION(BlueprintCallable, Category = "GSD|Save")
    void AddActorState(const FGSDSerializedActorState& State);

    UFUNCTION(BlueprintCallable, Category = "GSD|Save")
    bool GetActorState(FName ActorName, FGSDSerializedActorState& OutState) const;

    UFUNCTION(BlueprintCallable, Category = "GSD|Save")
    void RemoveActorState(FName ActorName);

    UFUNCTION(BlueprintCallable, Category = "GSD|Save")
    void ClearAllStates();

    UFUNCTION(BlueprintPure, Category = "GSD|Save")
    int32 GetActorStateCount() const { return ActorStates.Num(); }

    UFUNCTION(BlueprintCallable, Category = "GSD|Save")
    void RecordRandomCall(FName Category, int32 Hash);

    UFUNCTION(BlueprintPure, Category = "GSD|Save")
    bool ValidateDeterminism(int32 ExpectedHash) const;
};
