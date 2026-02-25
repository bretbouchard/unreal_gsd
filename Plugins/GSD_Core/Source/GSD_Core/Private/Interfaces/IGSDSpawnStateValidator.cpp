#include "Interfaces/IGSDSpawnStateValidator.h"
#include "GSDLog.h"

bool IGSDSpawnStateValidator::ValidateSpawnState_Implementation(const TArray<FGSDSeededSpawnTicket>& Expected)
{
    // Default: Always passes
    return true;
}

int32 IGSDSpawnStateValidator::ComputeSpawnHash_Implementation() const
{
    return 0;
}

void IGSDSpawnStateValidator::LogDivergence_Implementation(const FGSDSeededSpawnTicket& Expected, const FGSDSeededSpawnTicket& Actual)
{
    GSD_WARN(TEXT("Spawn divergence detected - Expected Order: %d, Actual Order: %d"),
        Expected.SpawnOrder, Actual.SpawnOrder);
}

void IGSDSpawnStateValidator::GetCurrentSpawnState_Implementation(TArray<FGSDSeededSpawnTicket>& OutState) const
{
    OutState.Empty();
}

void IGSDSpawnStateValidator::RecordSpawn_Implementation(const FGSDSeededSpawnTicket& Ticket)
{
    // Default implementation - override in C++ or Blueprint
}

void IGSDSpawnStateValidator::ClearSpawnHistory_Implementation()
{
    // Default implementation - override in C++ or Blueprint
}

int32 IGSDSpawnStateValidator::GetSpawnCount_Implementation() const
{
    return 0;
}
