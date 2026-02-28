#include "Managers/GSDDeterminismManager.h"
#include "GSDLog.h"

const FName UGSDDeterminismManager::SpawnCategory = TEXT("Spawn");
const FName UGSDDeterminismManager::EventCategory = TEXT("Event");
const FName UGSDDeterminismManager::CrowdCategory = TEXT("Crowd");
const FName UGSDDeterminismManager::VehicleCategory = TEXT("Vehicle");

// Crowd-specific categories for fine-grained debugging
const FName UGSDDeterminismManager::CrowdSpawnCategory = TEXT("CrowdSpawn");
const FName UGSDDeterminismManager::CrowdLODCategory = TEXT("CrowdLOD");
const FName UGSDDeterminismManager::CrowdVelocityCategory = TEXT("CrowdVelocity");
const FName UGSDDeterminismManager::ZombieWanderCategory = TEXT("ZombieWander");
const FName UGSDDeterminismManager::ZombieTargetCategory = TEXT("ZombieTarget");
const FName UGSDDeterminismManager::ZombieBehaviorCategory = TEXT("ZombieBehavior");
const FName UGSDDeterminismManager::ZombieSpeedCategory = TEXT("ZombieSpeed");

void UGSDDeterminismManager::Initialize(FSubsystemCollectionBase& Collection)
{
    GSD_LOG(Log, TEXT("UGSDDeterminismManager initialized"));

    // Create default category streams
    CreateCategoryStream(SpawnCategory);
    CreateCategoryStream(EventCategory);
    CreateCategoryStream(CrowdCategory);
    CreateCategoryStream(VehicleCategory);

    // Create crowd-specific streams
    CreateCategoryStream(CrowdSpawnCategory);
    CreateCategoryStream(CrowdLODCategory);
    CreateCategoryStream(CrowdVelocityCategory);
    CreateCategoryStream(ZombieWanderCategory);
    CreateCategoryStream(ZombieTargetCategory);
    CreateCategoryStream(ZombieBehaviorCategory);
    CreateCategoryStream(ZombieSpeedCategory);
}

void UGSDDeterminismManager::Deinitialize()
{
    CategoryStreams.Empty();
    RecordedCalls.Empty();
    bIsRecording = false;
    CallCounter = 0;
    GSD_LOG(Log, TEXT("UGSDDeterminismManager deinitialized"));
}

void UGSDDeterminismManager::InitializeWithSeed(int32 InSeed)
{
    CurrentSeed = InSeed;
    StateHash = 0;

    GSD_LOG(Log, TEXT("UGSDDeterminismManager seeded with %d"), CurrentSeed);

    // Reset and reseed all streams
    for (auto& Pair : CategoryStreams)
    {
        // Derive category-specific seed from main seed + category hash
        int32 CategorySeed = CurrentSeed + GetTypeHash(Pair.Key);
        Pair.Value.Initialize(CategorySeed);
    }
}

FRandomStream& UGSDDeterminismManager::GetStream(FName Category)
{
    if (!CategoryStreams.Contains(Category))
    {
        CreateCategoryStream(Category);
    }
    return CategoryStreams[Category];
}

int32 UGSDDeterminismManager::ComputeStateHash() const
{
    return StateHash;
}

float UGSDDeterminismManager::RandomFloat(FName Category)
{
    float Value = GetStream(Category).GetFraction();
    StateHash = HashCombine(StateHash, GetTypeHash(Value));
    return Value;
}

int32 UGSDDeterminismManager::RandomInteger(FName Category, int32 Max)
{
    int32 Value = GetStream(Category).RandHelper(Max);
    StateHash = HashCombine(StateHash, Value);
    return Value;
}

bool UGSDDeterminismManager::RandomBool(FName Category)
{
    bool Value = GetStream(Category).GetFraction() > 0.5f;
    StateHash = HashCombine(StateHash, GetTypeHash(Value));
    return Value;
}

FVector UGSDDeterminismManager::RandomUnitVector(FName Category)
{
    FVector Value = GetStream(Category).VRand();
    StateHash = HashCombine(StateHash, GetTypeHash(Value));
    return Value;
}

template<typename T>
void UGSDDeterminismManager::ShuffleArray(FName Category, TArray<T>& Array)
{
    FRandomStream& Stream = GetStream(Category);
    for (int32 i = Array.Num() - 1; i > 0; --i)
    {
        int32 j = Stream.RandHelper(i + 1);
        Array.Swap(i, j);
    }
    StateHash = HashCombine(StateHash, Array.Num());
}

void UGSDDeterminismManager::ResetStream(FName Category)
{
    if (CategoryStreams.Contains(Category))
    {
        int32 CategorySeed = CurrentSeed + GetTypeHash(Category);
        CategoryStreams[Category].Initialize(CategorySeed);
        GSD_LOG(Verbose, TEXT("Reset stream for category %s"), *Category.ToString());
    }
}

void UGSDDeterminismManager::ResetAllStreams()
{
    for (auto& Pair : CategoryStreams)
    {
        int32 CategorySeed = CurrentSeed + GetTypeHash(Pair.Key);
        Pair.Value.Initialize(CategorySeed);
    }
    GSD_LOG(Log, TEXT("Reset all streams with seed %d"), CurrentSeed);
}

void UGSDDeterminismManager::CreateCategoryStream(FName Category)
{
    int32 CategorySeed = CurrentSeed + GetTypeHash(Category);
    FRandomStream NewStream(CategorySeed);
    CategoryStreams.Add(Category, NewStream);
}

void UGSDDeterminismManager::RecordRandomCall(FName Category, float Value)
{
    if (bIsRecording)
    {
        FGSDRandomCallRecord Record;
        Record.Category = Category;
        Record.CallIndex = CallCounter++;
        Record.FloatValue = Value;
        Record.bIsVector = false;
        RecordedCalls.Add(Record);

        GSD_LOG(Verbose, TEXT("Recorded random call [%d] %s: %f"),
            Record.CallIndex, *Category.ToString(), Value);
    }
}

void UGSDDeterminismManager::RecordRandomCall(FName Category, FVector Value)
{
    if (bIsRecording)
    {
        FGSDRandomCallRecord Record;
        Record.Category = Category;
        Record.CallIndex = CallCounter++;
        Record.VectorValue = Value;
        Record.bIsVector = true;
        RecordedCalls.Add(Record);

        GSD_LOG(Verbose, TEXT("Recorded random call [%d] %s: %s"),
            Record.CallIndex, *Category.ToString(), *Value.ToString());
    }
}

void UGSDDeterminismManager::ClearRecordedCalls()
{
    RecordedCalls.Empty();
    CallCounter = 0;
    GSD_LOG(Log, TEXT("Cleared all recorded random calls"));
}

// Explicit template instantiation for common types
template void UGSDDeterminismManager::ShuffleArray<int32>(FName, TArray<int32>&);
template void UGSDDeterminismManager::ShuffleArray<FString>(FName, TArray<FString>&);
