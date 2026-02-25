#pragma once

#include "CoreMinimal.h"
#include "GSDImportTypes.generated.h"

/**
 * Import format options for city tiles.
 * USD is preferred for better layering and material handling.
 * FBX is provided as a fallback for compatibility.
 */
UENUM(BlueprintType)
enum class EGSDImportFormat : uint8
{
    /** USD format - preferred, supports layering, better material handling */
    USD     UMETA(DisplayName = "USD"),

    /** FBX format - fallback, most compatible format */
    FBX     UMETA(DisplayName = "FBX")
};

/**
 * Collision generation options for imported geometry.
 * Different collision types offer varying precision vs performance tradeoffs.
 */
UENUM(BlueprintType)
enum class EGSDCollisionType : uint8
{
    /** No collision generated - for decorative-only geometry */
    None            UMETA(DisplayName = "None"),

    /** Convex hull collision - simple, fast, good for most objects */
    ConvexHull      UMETA(DisplayName = "Convex Hull"),

    /** Auto-generated simple collision - balanced precision and performance */
    Simple          UMETA(DisplayName = "Simple"),

    /** Per-triangle collision - exact but expensive, use sparingly */
    Complex         UMETA(DisplayName = "Complex")
};

/**
 * Import configuration for a city tile.
 * Contains all settings needed to import geometry from external sources.
 */
USTRUCT(BlueprintType)
struct FGSDImportConfig
{
    GENERATED_BODY()

    // === Source Settings ===

    /** Source file path (relative to project or absolute) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import|Source")
    FString SourceFilePath;

    /** Import format (auto-detected from extension if not specified) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import|Source")
    EGSDImportFormat Format = EGSDImportFormat::USD;

    /** Import scale (1.0 = no scale adjustment, USD/FBX should be in cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import|Source", meta = (ClampMin = "0.01", ClampMax = "100.0"))
    float ImportScale = 1.0f;

    // === Collision Settings ===

    /** Collision generation type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import|Collision")
    EGSDCollisionType CollisionType = EGSDCollisionType::ConvexHull;

    /** Auto-generate collision on import */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import|Collision")
    bool bAutoGenerateCollision = true;

    // === Mesh Settings ===

    /** Combine meshes during import (usually false for city tiles to preserve HLOD boundaries) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import|Mesh")
    bool bCombineMeshes = false;

    // === Destination Settings ===

    /** Destination path for imported assets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import|Destination")
    FString DestinationPath = "/Game/CityTiles";

    /** Tile identifier for naming */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import|Destination")
    FString TileName;

    /** Default constructor */
    FGSDImportConfig()
        : SourceFilePath(TEXT(""))
        , Format(EGSDImportFormat::USD)
        , ImportScale(1.0f)
        , CollisionType(EGSDCollisionType::ConvexHull)
        , bAutoGenerateCollision(true)
        , bCombineMeshes(false)
        , DestinationPath("/Game/CityTiles")
        , TileName(TEXT(""))
    {}

    /** Constructor with source file */
    FGSDImportConfig(const FString& InSourcePath, const FString& InTileName)
        : SourceFilePath(InSourcePath)
        , Format(EGSDImportFormat::USD)
        , ImportScale(1.0f)
        , CollisionType(EGSDCollisionType::ConvexHull)
        , bAutoGenerateCollision(true)
        , bCombineMeshes(false)
        , DestinationPath("/Game/CityTiles")
        , TileName(InTileName)
    {}
};

/**
 * Result of an import operation.
 * Contains success status, error messages, and paths to imported assets.
 */
USTRUCT(BlueprintType)
struct FGSDImportResult
{
    GENERATED_BODY()

    /** Whether the import operation succeeded */
    UPROPERTY(BlueprintReadOnly)
    bool bSuccess = false;

    /** Error message if import failed */
    UPROPERTY(BlueprintReadOnly)
    FString ErrorMessage;

    /** Array of all imported asset paths */
    UPROPERTY(BlueprintReadOnly)
    TArray<FString> ImportedAssets;

    /** Path to the primary imported static mesh */
    UPROPERTY(BlueprintReadOnly)
    FString ImportedStaticMeshPath;

    /** Default constructor */
    FGSDImportResult()
        : bSuccess(false)
        , ErrorMessage(TEXT(""))
        , ImportedStaticMeshPath(TEXT(""))
    {}

    /** Create a success result */
    static FGSDImportResult Success(const FString& InStaticMeshPath, const TArray<FString>& InAssets)
    {
        FGSDImportResult Result;
        Result.bSuccess = true;
        Result.ImportedStaticMeshPath = InStaticMeshPath;
        Result.ImportedAssets = InAssets;
        return Result;
    }

    /** Create a failure result */
    static FGSDImportResult Failure(const FString& InErrorMessage)
    {
        FGSDImportResult Result;
        Result.bSuccess = false;
        Result.ErrorMessage = InErrorMessage;
        return Result;
    }
};

/**
 * Helper functions for import operations.
 */
namespace GSDImportHelpers
{
    /**
     * Detect import format from file extension.
     *
     * @param FilePath Path to the source file
     * @return Detected format (USD for .usd/.usda/.usdc, FBX for .fbx)
     * @return EGSDImportFormat::USD if extension cannot be determined
     */
    GSD_CITYSTREAMING_API EGSDImportFormat DetectFormatFromExtension(const FString& FilePath);

    /**
     * Check if a file extension is supported.
     *
     * @param FilePath Path to check
     * @return True if the file extension is .usd, .usda, .usdc, or .fbx
     */
    GSD_CITYSTREAMING_API bool IsSupportedExtension(const FString& FilePath);

    /**
     * Get file extension in lowercase.
     *
     * @param FilePath Path to extract extension from
     * @return Lowercase extension without the dot (e.g., "usd", "fbx")
     */
    GSD_CITYSTREAMING_API FString GetFileExtension(const FString& FilePath);
}
