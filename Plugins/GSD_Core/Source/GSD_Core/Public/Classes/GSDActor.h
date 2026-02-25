#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IGSDSpawnable.h"
#include "Interfaces/IGSDStreamable.h"
#include "GSDActor.generated.h"

class UGSDDataAsset;
class UGSDComponent;

UCLASS(Abstract, Blueprintable, Category = "GSD")
class GSD_CORE_API AGSDActor : public AActor, public IGSDSpawnable, public IGSDStreamable
{
    GENERATED_BODY()

public:
    AGSDActor();

    // ~ IGSDSpawnable interface
    virtual void SpawnAsync(UGSDDataAsset* Config) override;
    virtual UGSDDataAsset* GetSpawnConfig() override;
    virtual bool IsSpawned() override;
    virtual void Despawn() override;
    virtual void ResetSpawnState() override;

    // ~ IGSDStreamable interface
    virtual FBoxSphereBounds GetStreamingBounds_Implementation() override;
    virtual void OnStreamIn_Implementation() override;
    virtual void OnStreamOut_Implementation() override;
    virtual int32 GetStreamingPriority_Implementation() override;
    virtual bool ShouldPersist_Implementation() override;

    // Common GSD actor functionality
    UFUNCTION(BlueprintCallable, Category = "GSD|Actor")
    UGSDDataAsset* GetGSDConfig() const { return GSDConfig; }

    UFUNCTION(BlueprintCallable, Category = "GSD|Actor")
    void SetGSDConfig(UGSDDataAsset* InConfig) { GSDConfig = InConfig; }

    UFUNCTION(BlueprintCallable, Category = "GSD|Actor")
    bool GetIsSpawned() const { return bIsSpawned; }

    UFUNCTION(BlueprintCallable, Category = "GSD|Actor")
    bool GetIsStreamedIn() const { return bIsStreamedIn; }

protected:
    // The GSD config asset for this actor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GSD")
    UGSDDataAsset* GSDConfig = nullptr;

    // Spawn state
    UPROPERTY(BlueprintReadOnly, Category = "GSD")
    bool bIsSpawned = false;

    // Streaming state
    UPROPERTY(BlueprintReadOnly, Category = "GSD")
    bool bIsStreamedIn = true;

    // Called when config is applied
    UFUNCTION(BlueprintNativeEvent, Category = "GSD|Actor")
    void OnConfigApplied(UGSDDataAsset* Config);
    virtual void OnConfigApplied_Implementation(UGSDDataAsset* Config);

    // Called when spawn completes
    UFUNCTION(BlueprintNativeEvent, Category = "GSD|Actor")
    void OnSpawnComplete();
    virtual void OnSpawnComplete_Implementation();

    // Called when despawn starts
    UFUNCTION(BlueprintNativeEvent, Category = "GSD|Actor")
    void OnDespawnStart();
    virtual void OnDespawnStart_Implementation();
};
