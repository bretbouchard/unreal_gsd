#include "Types/GSDSaveGame.h"

UGSDSaveGame::UGSDSaveGame()
{
    SaveName = TEXT("GSDSave");
    SaveTimestamp = FDateTime::Now();
}

void UGSDSaveGame::AddActorState(const FGSDSerializedActorState& State)
{
    // Check if actor already exists
    for (int32 i = 0; i < ActorStates.Num(); ++i)
    {
        if (ActorStates[i].ActorName == State.ActorName)
        {
            ActorStates[i] = State;
            return;
        }
    }

    // Add new state
    ActorStates.Add(State);
}

bool UGSDSaveGame::GetActorState(FName ActorName, FGSDSerializedActorState& OutState) const
{
    for (const FGSDSerializedActorState& State : ActorStates)
    {
        if (State.ActorName == ActorName)
        {
            OutState = State;
            return true;
        }
    }
    return false;
}

void UGSDSaveGame::RemoveActorState(FName ActorName)
{
    ActorStates.RemoveAll([ActorName](const FGSDSerializedActorState& State)
    {
        return State.ActorName == ActorName;
    });
}

void UGSDSaveGame::ClearAllStates()
{
    ActorStates.Empty();
    PendingSpawns.Empty();
    RandomHistory.Empty();
    TotalSpawnCount = 0;
}

void UGSDSaveGame::RecordRandomCall(FName Category, int32 Hash)
{
    for (FGSDRandomCallLog& Log : RandomHistory)
    {
        if (Log.Category == Category)
        {
            Log.CallCount++;
            Log.LastHash = Hash;
            return;
        }
    }

    FGSDRandomCallLog NewLog;
    NewLog.Category = Category;
    NewLog.CallCount = 1;
    NewLog.LastHash = Hash;
    RandomHistory.Add(NewLog);
}

bool UGSDSaveGame::ValidateDeterminism(int32 ExpectedHash) const
{
    return DeterminismHash == ExpectedHash;
}
