#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IGSDComponentSpawnable.generated.h"

class UActorComponent;
class AActor;

UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
class UGSDComponentSpawnable : public UInterface
{
    GENERATED_BODY()
};

class GSD_CORE_API IGSDComponentSpawnable
{
    GENERATED_BODY()

public:
    // Spawn component on owner actor
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Components")
    UActorComponent* SpawnComponent(AActor* Owner, const FTransform& Transform);
    virtual UActorComponent* SpawnComponent_Implementation(AActor* Owner, const FTransform& Transform);

    // Get component type identifier
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Components")
    FName GetComponentType() const;
    virtual FName GetComponentType_Implementation() const;

    // Check if component is active
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Components")
    bool IsComponentActive() const;
    virtual bool IsComponentActive_Implementation() const;

    // Deactivate component
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Components")
    void DeactivateComponent();
    virtual void DeactivateComponent_Implementation();

    // Destroy component
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Components")
    void DestroyComponent();
    virtual void DestroyComponent_Implementation();
};
