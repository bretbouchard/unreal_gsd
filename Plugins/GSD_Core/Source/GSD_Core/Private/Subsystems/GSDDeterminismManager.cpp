// Copyright Bret Bouchard. All Rights Reserved.

#include "Subsystems/GSDDeterminismManager.h"

DEFINE_LOG_CATEGORY(LogGSDDeterminism);

void UGSDDeterminismManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // If a seed is already configured, use it
    if (GlobalSeed != 0)
    {
        InitializeWithSeed(GlobalSeed);
        UE_LOG(LogGSDDeterminism, Log, TEXT("DeterminismManager initialized with configured seed: %d"), GlobalSeed);
    }
    else if (bUseTimeBasedSeed)
    {
        // Use time-based seed for non-deterministic play
        int32 TimeSeed = FDateTime::Now().GetTicks() % INT_MAX;
        InitializeWithSeed(TimeSeed);
        UE_LOG(LogGSDDeterminism, Log, TEXT("DeterminismManager initialized with time-based seed: %d"), TimeSeed);
    }
    else
    {
        // Default seed of 0 (will be set via InitializeWithSeed later)
        UE_LOG(LogGSDDeterminism, Warning, TEXT("DeterminismManager initialized without seed - call InitializeWithSeed() for deterministic behavior"));
    }
}

void UGSDDeterminismManager::Deinitialize()
{
    CategoryStreams.Empty();
    RecordedCalls.Empty();
    bIsInitialized = false;

    Super::Deinitialize();
}

void UGSDDeterminismManager::InitializeWithSeed(int32 Seed)
{
    GlobalSeed = Seed;
    bIsInitialized = true;

    // Clear existing streams and recordings
    CategoryStreams.Empty();
    RecordedCalls.Empty();

    // Pre-seed common categories for determinism
    // Each category will be lazily created when accessed via GetCategoryStream

    UE_LOG(LogGSDDeterminism, Log, TEXT("DeterminismManager seeded with: %d"), Seed);
}

FRandomStream& UGSDDeterminismManager::GetCategoryStream(FName Category)
{
    // Check if stream already exists
    if (FRandomStream* ExistingStream = CategoryStreams.Find(Category))
    {
        return *ExistingStream;
    }

    // Create new stream with derived seed
    int32 CategorySeed = DeriveCategorySeed(Category);
    FRandomStream& NewStream = CategoryStreams.Add(Category, FRandomStream(CategorySeed));

    UE_LOG(LogGSDDeterminism, Verbose, TEXT("Created new random stream for category '%s' with seed %d"),
        *Category.ToString(), CategorySeed);

    return NewStream;
}

void UGSDDeterminismManager::RecordRandomCall(FName Category, float Value, bool bRecord)
{
    if (!bRecord || !bIsRecording)
    {
        return;
    }

    FGSDRandomCallRecord Record;
    Record.Category = Category;
    Record.FloatValue = Value;
    Record.bIsVector = false;
    Record.FrameNumber = GFrameNumber;

    RecordedCalls.Add(Record);

    UE_LOG(LogGSDDeterminism, Verbose, TEXT("Recorded random call: Category=%s, Value=%f, Frame=%d"),
        *Category.ToString(), Value, Record.FrameNumber);
}

void UGSDDeterminismManager::RecordRandomCall(FName Category, const FVector& Value, bool bRecord)
{
    if (!bRecord || !bIsRecording)
    {
        return;
    }

    FGSDRandomCallRecord Record;
    Record.Category = Category;
    Record.VectorValue = Value;
    Record.bIsVector = true;
    Record.FrameNumber = GFrameNumber;

    RecordedCalls.Add(Record);

    UE_LOG(LogGSDDeterminism, Verbose, TEXT("Recorded random vector call: Category=%s, Value=%s, Frame=%d"),
        *Category.ToString(), *Value.ToString(), Record.FrameNumber);
}

int32 UGSDDeterminismManager::DeriveCategorySeed(FName Category) const
{
    // Combine global seed with category hash for deterministic per-category seeds
    // This ensures each category has a different but reproducible sequence
    int32 CategoryHash = GetTypeHash(Category);

    // Use a simple mixing function to combine seeds
    // This prevents similar categories from having similar sequences
    int32 DerivedSeed = GlobalSeed ^ (CategoryHash * 2654435761U);

    // Ensure the seed is non-zero (zero produces constant stream)
    if (DerivedSeed == 0)
    {
        DerivedSeed = CategoryHash;
    }

    return DerivedSeed;
}
