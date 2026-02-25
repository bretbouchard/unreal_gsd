#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IGSDMetaSoundInterface.generated.h"

class USoundSubmix;
class USoundClass;

UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
class UGSDMetaSoundInterface : public UInterface
{
    GENERATED_BODY()
};

class GSD_CORE_API IGSDMetaSoundInterface
{
    GENERATED_BODY()

public:
    // Get the MetaSound source for spawning
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
    UObject* GetMetaSoundSource() const;
    virtual UObject* GetMetaSoundSource_Implementation() const;

    // Default parameters to pass on spawn
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
    TMap<FName, float> GetDefaultParameters() const;
    virtual TMap<FName, float> GetDefaultParameters_Implementation() const;

    // Output submix routing
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
    USoundSubmix* GetTargetSubmix() const;
    virtual USoundSubmix* GetTargetSubmix_Implementation() const;

    // Sound class for volume/priority
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
    USoundClass* GetSoundClass() const;
    virtual USoundClass* GetSoundClass_Implementation() const;

    // Set a runtime parameter on the MetaSound
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
    void SetFloatParameter(FName ParameterName, float Value);
    virtual void SetFloatParameter_Implementation(FName ParameterName, float Value);

    // Get a runtime parameter from the MetaSound
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Audio")
    bool GetFloatParameter(FName ParameterName, float& OutValue) const;
    virtual bool GetFloatParameter_Implementation(FName ParameterName, float& OutValue) const;
};
