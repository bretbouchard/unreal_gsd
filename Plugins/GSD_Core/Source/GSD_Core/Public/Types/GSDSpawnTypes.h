#pragma once

#include "CoreMinimal.h"
#include "GSDSpawnTypes.generated.h"

class UGSDDataAsset;

// Forward declarations
class AActor;
class UActorComponent;

// Network spawn parameters (for future multiplayer support)
USTRUCT(BlueprintType)
struct GSD_CORE_API FGSDNetworkSpawnParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bReplicates = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENetRole RemoteRole = ROLE_SimulatedProxy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NetUpdateFrequency = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NetCullDistanceSquared = 10000.0f * 10000.0f; // 100m

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* Owner = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bServerAuthoritative = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    APlayerController* RequestingController = nullptr;
};

// Spawn ticket containing all spawn parameters
USTRUCT(BlueprintType)
struct GSD_CORE_API FGSDSpawnTicket
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AActor> ActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UGSDDataAsset* Config = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGSDNetworkSpawnParams NetworkParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority = 0;
};

// Seeded spawn ticket for determinism
USTRUCT(BlueprintType)
struct GSD_CORE_API FGSDSeededSpawnTicket : public FGSDSpawnTicket
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SpawnSeed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SpawnOrder = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint32 ParameterHash = 0;
};

// Spawn comparator for deterministic ordering
struct FGSDSpawnComparator
{
    bool operator()(const FGSDSeededSpawnTicket& A, const FGSDSeededSpawnTicket& B) const
    {
        if (A.Priority != B.Priority)
            return A.Priority > B.Priority;
        return A.SpawnOrder < B.SpawnOrder;
    }
};

// Delegates for spawn callbacks
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSpawnComplete, AActor*, SpawnedActor);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnSpawnFailed, const FString&, Reason, const FGSDSpawnTicket&, Ticket);

// Mass spawn delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMassSpawnComplete, const TArray<AActor*>&, SpawnedActors);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMassSpawnChunkComplete, int32, ChunkIndex, int32, TotalChunks);

// Tick context with budget tracking
USTRUCT(BlueprintType)
struct GSD_CORE_API FGSDTickContext
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float DeltaTime = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float GameTime = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float AudioBudgetRemainingMs = 2.0f;

    UFUNCTION(BlueprintPure, Category = "GSD")
    bool HasAudioBudget() const { return AudioBudgetRemainingMs > 0.1f; }
};
