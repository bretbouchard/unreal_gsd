// Copyright Bret Bouchard. All Rights Reserved.

#include "Spatial/GSDSpatialHash.h"
#include "GSDCrowdLog.h"
#include "ProfilingDebugging/ScopedTimers.h"

void UGSDSpatialHash::Initialize(const FGSDSpatialHashConfig& InConfig)
{
    Config = InConfig;

    // Pre-allocate cells if pooling enabled
    if (Config.bEnableCellPooling)
    {
        Cells.Reserve(Config.InitialCellPoolSize);
        EntityToCellKey.Reserve(Config.InitialCellPoolSize * 4);  // Estimate 4 entities per cell
    }

    UE_LOG(LOG_GSDCROWDS, Log, TEXT("SpatialHash initialized: CellSize=%.1f, PoolSize=%d"),
        Config.CellSize, Config.InitialCellPoolSize);
}

void UGSDSpatialHash::Clear()
{
    Cells.Empty();
    EntityToCellKey.Empty();
    TotalEntityCount = 0;

    UE_LOG(LOG_GSDCROWDS, Verbose, TEXT("SpatialHash cleared"));
}

void UGSDSpatialHash::Insert(const FMassEntityHandle& Entity, const FVector& Position)
{
    if (!Entity.IsValid())
    {
        return;
    }

    // Check if entity already exists
    if (int64* ExistingCellKey = EntityToCellKey.Find(Entity))
    {
        // Entity exists - update position instead
        UpdatePosition(Entity, Position);
        return;
    }

    // Get cell coordinates
    FIntVector CellCoords = GetCellCoords(Position);
    int64 CellKey = MakeCellKey(CellCoords.X, CellCoords.Y);

    // Add to cell
    FGSDSpatialCell& Cell = GetOrCreateCell(CellCoords.X, CellCoords.Y);
    Cell.Entities.Add(Entity);
    Cell.bIsDirty = true;

    // Track entity -> cell mapping
    EntityToCellKey.Add(Entity, CellKey);

    TotalEntityCount++;
}

void UGSDSpatialHash::Remove(const FMassEntityHandle& Entity)
{
    if (!Entity.IsValid())
    {
        return;
    }

    // Find cell key for this entity
    int64* CellKeyPtr = EntityToCellKey.Find(Entity);
    if (!CellKeyPtr)
    {
        // Entity not in hash
        return;
    }

    int64 CellKey = *CellKeyPtr;

    // Remove from cell
    if (FGSDSpatialCell* Cell = Cells.Find(CellKey))
    {
        Cell->Entities.Remove(Entity);
        Cell->bIsDirty = true;

        // Remove empty cells to save memory
        if (Cell->Entities.Num() == 0)
        {
            Cells.Remove(CellKey);
        }
    }

    // Remove entity mapping
    EntityToCellKey.Remove(Entity);

    TotalEntityCount--;
}

void UGSDSpatialHash::UpdatePosition(const FMassEntityHandle& Entity, const FVector& NewPosition)
{
    if (!Entity.IsValid())
    {
        return;
    }

    // Find current cell
    int64* CurrentCellKeyPtr = EntityToCellKey.Find(Entity);
    if (!CurrentCellKeyPtr)
    {
        // Entity not in hash - insert instead
        Insert(Entity, NewPosition);
        return;
    }

    // Get new cell coordinates
    FIntVector NewCellCoords = GetCellCoords(NewPosition);
    int64 NewCellKey = MakeCellKey(NewCellCoords.X, NewCellCoords.Y);
    int64 CurrentCellKey = *CurrentCellKeyPtr;

    // Check if cell changed
    if (NewCellKey == CurrentCellKey)
    {
        // Same cell - no update needed
        return;
    }

    // Remove from old cell
    if (FGSDSpatialCell* OldCell = Cells.Find(CurrentCellKey))
    {
        OldCell->Entities.Remove(Entity);
        OldCell->bIsDirty = true;

        // Remove empty cells
        if (OldCell->Entities.Num() == 0)
        {
            Cells.Remove(CurrentCellKey);
        }
    }

    // Add to new cell
    FGSDSpatialCell& NewCell = GetOrCreateCell(NewCellCoords.X, NewCellCoords.Y);
    NewCell.Entities.Add(Entity);
    NewCell.bIsDirty = true;

    // Update mapping
    EntityToCellKey[Entity] = NewCellKey;
}

FGSDSpatialQueryResult UGSDSpatialHash::GetEntitiesInRadius(const FVector& Center, float Radius) const
{
    FGSDSpatialQueryResult Result;
    Result.QueryCenter = Center;
    Result.QueryRadius = Radius;

    double StartTime = FPlatformTime::Seconds();

    // Calculate cell range to search
    int32 CellRadius = FMath::CeilToInt(Radius / Config.CellSize);
    CellRadius = FMath::Min(CellRadius, Config.MaxCellSearchRadius);

    FIntVector CenterCell = GetCellCoords(Center);
    float RadiusSq = Radius * Radius;

    // Iterate cells in square around center
    for (int32 dx = -CellRadius; dx <= CellRadius; dx++)
    {
        for (int32 dy = -CellRadius; dy <= CellRadius; dy++)
        {
            int64 CellKey = MakeCellKey(CenterCell.X + dx, CenterCell.Y + dy);

            const FGSDSpatialCell* Cell = Cells.Find(CellKey);
            if (!Cell || Cell->Entities.Num() == 0)
            {
                continue;
            }

            // Check each entity in cell
            for (const FMassEntityHandle& Entity : Cell->Entities)
            {
                // Note: For full implementation, we'd need entity positions
                // For now, we return all entities in overlapping cells
                // The caller can do precise distance filtering if needed
                Result.Entities.Add(Entity);
            }
        }
    }

    Result.Count = Result.Entities.Num();

    double EndTime = FPlatformTime::Seconds();
    Result.QueryTimeMicroseconds = (EndTime - StartTime) * 1000000.0;

    return Result;
}

FGSDSpatialQueryResult UGSDSpatialHash::GetEntitiesInBox(const FVector& Min, const FVector& Max) const
{
    FGSDSpatialQueryResult Result;

    double StartTime = FPlatformTime::Seconds();

    // Calculate cell range
    FIntVector MinCell = GetCellCoords(Min);
    FIntVector MaxCell = GetCellCoords(Max);

    // Iterate all cells in box
    for (int32 x = MinCell.X; x <= MaxCell.X; x++)
    {
        for (int32 y = MinCell.Y; y <= MaxCell.Y; y++)
        {
            int64 CellKey = MakeCellKey(x, y);

            const FGSDSpatialCell* Cell = Cells.Find(CellKey);
            if (!Cell || Cell->Entities.Num() == 0)
            {
                continue;
            }

            Result.Entities.Append(Cell->Entities);
        }
    }

    Result.Count = Result.Entities.Num();

    double EndTime = FPlatformTime::Seconds();
    Result.QueryTimeMicroseconds = (EndTime - StartTime) * 1000000.0;

    return Result;
}

FIntVector UGSDSpatialHash::GetCellCoords(const FVector& Position) const
{
    return FIntVector(
        FMath::FloorToInt(Position.X / Config.CellSize),
        FMath::FloorToInt(Position.Y / Config.CellSize),
        0  // Z not used for 2D spatial hash
    );
}

int64 UGSDSpatialHash::GetCellKey(int32 CellX, int32 CellY) const
{
    return MakeCellKey(CellX, CellY);
}

bool UGSDSpatialHash::IsCellOccupied(int32 CellX, int32 CellY) const
{
    int64 Key = MakeCellKey(CellX, CellY);
    const FGSDSpatialCell* Cell = Cells.Find(Key);
    return Cell && Cell->Entities.Num() > 0;
}

void UGSDSpatialHash::SetCellSize(float NewCellSize)
{
    if (NewCellSize <= 0.0f)
    {
        UE_LOG(LOG_GSDCROWDS, Warning, TEXT("Invalid cell size: %.2f"), NewCellSize);
        return;
    }

    Config.CellSize = NewCellSize;

    // Must clear - entity positions need to be re-inserted
    Clear();

    UE_LOG(LOG_GSDCROWDS, Log, TEXT("SpatialHash cell size changed to %.1f (entities cleared)"), NewCellSize);
}

FGSDSpatialCell& UGSDSpatialHash::GetOrCreateCell(int32 CellX, int32 CellY)
{
    int64 Key = MakeCellKey(CellX, CellY);

    FGSDSpatialCell& Cell = Cells.FindOrAdd(Key);
    Cell.CellX = CellX;
    Cell.CellY = CellY;

    return Cell;
}
