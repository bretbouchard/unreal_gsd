#include "Interfaces/IGSDNetworkSpawnValidator.h"
#include "GameFramework/PlayerController.h"

bool IGSDNetworkSpawnValidator::ValidateSpawnRequest_Implementation(const FGSDSpawnTicket& Request, FString& OutRejectionReason)
{
    // Default: Allow all spawns
    return true;
}

void IGSDNetworkSpawnValidator::NotifySpawnComplete_Implementation(AActor* SpawnedActor)
{
    // Default implementation - override in C++ or Blueprint
}

void IGSDNetworkSpawnValidator::NotifySpawnRejected_Implementation(const FGSDSpawnTicket& Request, const FString& Reason)
{
    // Default implementation - override in C++ or Blueprint
}

bool IGSDNetworkSpawnValidator::IsServerAuthoritative_Implementation() const
{
    return true; // Default to server-authoritative
}

APlayerController* IGSDNetworkSpawnValidator::GetRequestingPlayer_Implementation() const
{
    return nullptr;
}

void IGSDNetworkSpawnValidator::SetNetworkParams_Implementation(const FGSDNetworkSpawnParams& Params)
{
    // Default implementation - override in C++ or Blueprint
}
