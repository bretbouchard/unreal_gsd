#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Types/GSDSpawnTypes.h"
#include "IGSDSpawnStateValidator.generated.h"

UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
class UGSDSpawnStateValidator : public UInterface
{
    GENERATED_BODY()
};

class GSD_CORE_API IGSDSpawnStateValidator
{
    GENERATED_BODY()

public:
    // Validate current spawn state matches recorded state
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Determinism")
    bool ValidateSpawnState(const TArray<FGSDSeededSpawnTicket>& Expected);
    virtual bool ValidateSpawnState_Implementation(const TArray<FGSDSeededSpawnTicket>& Expected);

    // Compute hash of current spawn state
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Determinism")
    int32 ComputeSpawnHash() const;
    virtual int32 ComputeSpawnHash_Implementation() const;

    // Log divergence for debugging
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Determinism")
    void LogDivergence(const FGSDSeededSpawnTicket& Expected, const FGSDSeededSpawnTicket& Actual);
    virtual void LogDivergence_Implementation(const FGSDSeededSpawnTicket& Expected, const FGSDSeededSpawnTicket& Actual);

    // Get current spawn state as tickets
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Determinism")
    void GetCurrentSpawnState(TArray<FGSDSeededSpawnTicket>& OutState) const;
    virtual void GetCurrentSpawnState_Implementation(TArray<FGSDSeededSpawnTicket>& OutState) const;

    // Record spawn for replay
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Determinism")
    void RecordSpawn(const FGSDSeededSpawnTicket& Ticket);
    virtual void RecordSpawn_Implementation(const FGSDSeededSpawnTicket& Ticket);

    // Clear recorded spawns
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Determinism")
    void ClearSpawnHistory();
    virtual void ClearSpawnHistory_Implementation();

    // Get spawn count for this session
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Determinism")
    int32 GetSpawnCount() const;
    virtual int32 GetSpawnCount_Implementation() const;
};
