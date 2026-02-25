#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/IGSDComponentSpawnable.h"
#include "GSDComponent.generated.h"

class UGSDDataAsset;

UCLASS(Abstract, Blueprintable, ClassGroup = "GSD", meta = (BlueprintSpawnableComponent))
class GSD_CORE_API UGSDComponent : public UActorComponent, public IGSDComponentSpawnable
{
    GENERATED_BODY()

public:
    UGSDComponent();

    // ~ IGSDComponentSpawnable interface
    virtual UActorComponent* SpawnComponent_Implementation(AActor* Owner, const FTransform& Transform) override;
    virtual FName GetComponentType_Implementation() const override;
    virtual bool IsComponentActive_Implementation() const override;
    virtual void DeactivateComponent_Implementation() override;
    virtual void DestroyComponent_Implementation() override;

    // Common GSD component functionality
    UFUNCTION(BlueprintCallable, Category = "GSD|Component")
    UGSDDataAsset* GetGSDConfig() const { return GSDConfig; }

    UFUNCTION(BlueprintCallable, Category = "GSD|Component")
    void SetGSDConfig(UGSDDataAsset* InConfig) { GSDConfig = InConfig; }

    UFUNCTION(BlueprintPure, Category = "GSD|Component")
    bool IsGSDActive() const { return bGSDActive; }

protected:
    // The GSD config asset for this component
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GSD")
    UGSDDataAsset* GSDConfig = nullptr;

    // GSD-specific active state
    UPROPERTY(BlueprintReadOnly, Category = "GSD")
    bool bGSDActive = false;

    // Called when config is applied
    UFUNCTION(BlueprintNativeEvent, Category = "GSD|Component")
    void OnConfigApplied(UGSDDataAsset* Config);
    virtual void OnConfigApplied_Implementation(UGSDDataAsset* Config);

    // Called when component activates
    UFUNCTION(BlueprintNativeEvent, Category = "GSD|Component")
    void OnGSDActivate();
    virtual void OnGSDActivate_Implementation();

    // Called when component deactivates
    UFUNCTION(BlueprintNativeEvent, Category = "GSD|Component")
    void OnGSDDeactivate();
    virtual void OnGSDDeactivate_Implementation();
};
