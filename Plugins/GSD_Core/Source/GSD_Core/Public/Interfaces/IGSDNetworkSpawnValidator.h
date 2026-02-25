#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Types/GSDSpawnTypes.h"
#include "IGSDNetworkSpawnValidator.generated.h"

class AActor;
class APlayerController;

UINTERFACE(MinimalAPI, Blueprintable, Category = "GSD")
class UGSDNetworkSpawnValidator : public UInterface
{
    GENERATED_BODY()
};

class GSD_CORE_API IGSDNetworkSpawnValidator
{
    GENERATED_BODY()

public:
    // Called on server to validate spawn request
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Network")
    bool ValidateSpawnRequest(const FGSDSpawnTicket& Request, FString& OutRejectionReason);
    virtual bool ValidateSpawnRequest_Implementation(const FGSDSpawnTicket& Request, FString& OutRejectionReason);

    // Called when spawn completes to notify clients
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Network")
    void NotifySpawnComplete(AActor* SpawnedActor);
    virtual void NotifySpawnComplete_Implementation(AActor* SpawnedActor);

    // Called when spawn is rejected
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Network")
    void NotifySpawnRejected(const FGSDSpawnTicket& Request, const FString& Reason);
    virtual void NotifySpawnRejected_Implementation(const FGSDSpawnTicket& Request, const FString& Reason);

    // Check if this is a server-authoritative spawn
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Network")
    bool IsServerAuthoritative() const;
    virtual bool IsServerAuthoritative_Implementation() const;

    // Get the requesting player controller
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Network")
    APlayerController* GetRequestingPlayer() const;
    virtual APlayerController* GetRequestingPlayer_Implementation() const;

    // Set network spawn parameters
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GSD|Network")
    void SetNetworkParams(const FGSDNetworkSpawnParams& Params);
    virtual void SetNetworkParams_Implementation(const FGSDNetworkSpawnParams& Params);
};
