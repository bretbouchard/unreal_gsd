#include "Interfaces/IGSDCityTileImporter.h"
#include "HAL/FileManager.h"

FGSDImportResult IGSDCityTileImporter::ImportTile(const FGSDImportConfig& Config)
{
    // Validate configuration first
    FString ErrorMessage;
    if (!ValidateConfig(Config, ErrorMessage))
    {
        return FGSDImportResult::Failure(ErrorMessage);
    }

    // Detect format and route to appropriate importer
    const EGSDImportFormat Format = GSDImportHelpers::DetectFormatFromExtension(Config.SourceFilePath);

    switch (Format)
    {
        case EGSDImportFormat::USD:
            return ImportUSD(Config);

        case EGSDImportFormat::FBX:
            return ImportFBX(Config);

        default:
            return FGSDImportResult::Failure(TEXT("Unknown import format"));
    }
}

TArray<FGSDImportResult> IGSDCityTileImporter::ImportTilesBatch(const TArray<FGSDImportConfig>& Configs)
{
    TArray<FGSDImportResult> Results;
    Results.Reserve(Configs.Num());

    for (const FGSDImportConfig& Config : Configs)
    {
        Results.Add(ImportTile(Config));
    }

    return Results;
}

bool IGSDCityTileImporter::ValidateConfig(const FGSDImportConfig& Config, FString& OutErrorMessage)
{
    // Check source file path is not empty
    if (Config.SourceFilePath.IsEmpty())
    {
        OutErrorMessage = TEXT("Source file path is empty");
        return false;
    }

    // Check source file exists
    if (!IFileManager::Get().FileExists(*Config.SourceFilePath))
    {
        OutErrorMessage = FString::Printf(TEXT("Source file does not exist: %s"), *Config.SourceFilePath);
        return false;
    }

    // Check file extension is supported
    if (!GSDImportHelpers::IsSupportedExtension(Config.SourceFilePath))
    {
        OutErrorMessage = FString::Printf(TEXT("Unsupported file extension. Supported formats: USD (.usd, .usda, .usdc), FBX (.fbx)"));
        return false;
    }

    // Check destination path is not empty
    if (Config.DestinationPath.IsEmpty())
    {
        OutErrorMessage = TEXT("Destination path is empty");
        return false;
    }

    // Check import scale is valid
    if (Config.ImportScale <= 0.0f)
    {
        OutErrorMessage = TEXT("Import scale must be greater than 0");
        return false;
    }

    return true;
}

TArray<EGSDImportFormat> IGSDCityTileImporter::GetSupportedFormats() const
{
    return { EGSDImportFormat::USD, EGSDImportFormat::FBX };
}

FGSDImportResult IGSDCityTileImporter::ImportUSD(const FGSDImportConfig& Config)
{
    // Placeholder implementation - actual import requires editor-only USD module
    // This will be implemented in future plans when editor integration is added
    //
    // Implementation will use:
    // - UUsdStageImportContext for USD stage setup
    // - UsdUtils module for USD processing
    // - StaticMesh import pipeline

    FString Message = FString::Printf(
        TEXT("USD import placeholder for: %s\n")
        TEXT("Actual import requires editor-only USD module integration.\n")
        TEXT("Destination: %s\n")
        TEXT("Scale: %.2f\n")
        TEXT("Collision: %s"),
        *Config.SourceFilePath,
        *Config.DestinationPath,
        Config.ImportScale,
        Config.bAutoGenerateCollision ? TEXT("Enabled") : TEXT("Disabled")
    );

    return FGSDImportResult::Failure(Message);
}

FGSDImportResult IGSDCityTileImporter::ImportFBX(const FGSDImportConfig& Config)
{
    // Placeholder implementation - actual import requires editor-only FBX import module
    // This will be implemented in future plans when editor integration is added
    //
    // Implementation will use:
    // - UFBXImportOptions for FBX settings
    // - UFbxMeshImportData for mesh processing
    // - StaticMesh import pipeline

    FString Message = FString::Printf(
        TEXT("FBX import placeholder for: %s\n")
        TEXT("Actual import requires editor-only FBX module integration.\n")
        TEXT("Destination: %s\n")
        TEXT("Scale: %.2f\n")
        TEXT("Collision: %s"),
        *Config.SourceFilePath,
        *Config.DestinationPath,
        Config.ImportScale,
        Config.bAutoGenerateCollision ? TEXT("Enabled") : TEXT("Disabled")
    );

    return FGSDImportResult::Failure(Message);
}
