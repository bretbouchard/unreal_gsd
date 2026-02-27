# Phase 10: Telemetry & Validation - Research

**Researched:** 2026-02-27
**Domain:** Unreal Engine 5 Telemetry, Commandlets, Performance Tracking, Validation Tooling
**Confidence:** HIGH

## Summary

Phase 10 provides observability infrastructure and validation tooling for the Unreal GSD Platform. The research covers six interconnected domains: Unreal Engine telemetry patterns, commandlet architecture for CI/CD, performance metrics collection, World Partition validation, editor tooling, and asset budget enforcement.

The standard approach leverages UE5's built-in stats system (`DECLARE_CYCLE_STAT`, `SCOPE_CYCLE_COUNTER`) for performance tracking, `UGameInstanceSubsystem` for cross-level telemetry aggregation, `UCommandlet` base class for headless validation, and `EditorUtilityWidget` for editor UI. The existing codebase already establishes patterns for commandlets (GSDVehicleTestCommandlet, GSDCrowdTestCommandlet, GSDEventTestCommandlet) and streaming telemetry (GSDStreamingTelemetry) that should be extended rather than replaced.

**Primary recommendation:** Extend existing GSDStreamingTelemetry subsystem into GSD_Telemetry plugin for unified metrics collection, create new commandlets following established JSON output patterns, and use EditorUtilityWidget for validation UI.

## Standard Stack

The established libraries/tools for this domain:

### Core
| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| UCommandlet | UE5 Native | Headless validation execution | Built-in commandlet infrastructure, CI/CD integration |
| UGameInstanceSubsystem | UE5 Native | Cross-level telemetry persistence | Survives level transitions, single instance per game |
| Stats System (DECLARE_CYCLE_STAT) | UE5 Native | Performance timing | Zero-overhead in shipping, stat group organization |
| FGameplayTag | UE5 Native | Metric categorization | Hierarchical organization, O(1) lookup |
| EditorUtilityWidget | UE5 Native | Editor validation UI | UMG-based, dockable, accessible from Windows menu |
| JsonUtilities Module | UE5 Native | JSON serialization | JsonObjectConverter for CI output |

### Supporting
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| TActorIterator | UE5 Native | Actor counting | Per-class enumeration in specific worlds |
| TRACE_CPUPROFILER_EVENT_SCOPE | UE5 Native | Unreal Insights integration | Deep profiling sessions |
| FVisualLogger | UE5 Native | Debug snapshots | Development builds, conditional compilation |
| UnrealBuildTool | UE5 Native | Build.cs validation | Pre-build asset budget checks |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Custom telemetry class | UGameInstanceSubsystem | Subsystem provides lifecycle management, auto-initialization |
| Manual JSON formatting | JsonObjectConverter | Converter handles complex types, consistent output |
| Slate-only validation UI | EditorUtilityWidget | UMG is faster to author, Blueprint-accessible |
| Per-frame broadcasts | Throttled delegates | Existing MinBroadcastInterval pattern prevents frame budget overrun |

**Installation:**
No external packages required. All components are UE5 native modules. Add module dependencies in Build.cs:
```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core", "CoreUObject", "Engine", "Json", "JsonUtilities", "DeveloperSettings"
});
```

## Architecture Patterns

### Recommended Project Structure
```
Plugins/GSD_Telemetry/
├── Source/GSD_Telemetry/
│   ├── Private/
│   │   ├── GSDTelemetry.cpp              # Plugin module
│   │   ├── Subsystems/
│   │   │   └── GSDPerformanceTelemetry.cpp  # Frame time, hitch tracking
│   │   ├── Commandlets/
│   │   │   ├── GSDValidateAssetsCommandlet.cpp
│   │   │   ├── GSDValidateWPCommandlet.cpp
│   │   │   └── GSDRunPerfRouteCommandlet.cpp
│   │   └── Widgets/
│   │       └── GSDValidationDashboardWidget.cpp  # EditorUtilityWidget
│   └── Public/
│       ├── GSD_Telemetry.h
│       ├── GSDTelemetryLog.h              # DECLARE_LOG_CATEGORY
│       ├── Subsystems/
│       │   └── GSDPerformanceTelemetry.h
│       ├── Commandlets/
│       │   └── GSDValidateAssetsCommandlet.h
│       ├── Types/
│       │   └── GSDTelemetryTypes.h        # Structs for metrics
│       └── Widgets/
│           └── GSDValidationDashboardWidget.h

Plugins/GSD_ValidationTools/
├── Source/GSD_ValidationTools/
│   ├── Private/
│   │   ├── GSDValidationTools.cpp
│   │   ├── Validators/
│   │   │   ├── GSDAssetBudgetValidator.cpp
│   │   │   └── GSDWorldPartitionValidator.cpp
│   │   └── Widgets/
│   │       └── GSDValidationPanel.cpp
│   └── Public/
│       └── ...
```

### Pattern 1: UCommandlet with JSON Output (Follows Existing Pattern)
**What:** Commandlet that outputs structured JSON for CI/CD parsing
**When to use:** All validation commandlets (TEL-04, TEL-05, TEL-06)
**Example:**
```cpp
// Source: Based on existing GSDVehicleTestCommandlet, GSDCrowdTestCommandlet patterns
// File: GSDValidateAssetsCommandlet.h
UCLASS()
class GSD_TELEMETRY_API UGSDValidateAssetsCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    UGSDValidateAssetsCommandlet();

    virtual int32 Main(const FString& Params) override;

private:
    int32 MaxAssetSizeMB = 100;
    bool bOutputJSON = true;
    FString OutputPath;

    void ParseParameters(const FString& Params);
    bool ValidateAsset(UObject* Asset, TArray<FString>& OutErrors);
    void OutputJSON(bool bSuccess, int32 TotalAssets, int32 FailedAssets, const TArray<FString>& Errors);
};

// File: GSDValidateAssetsCommandlet.cpp
UGSDValidateAssetsCommandlet::UGSDValidateAssetsCommandlet()
{
    HelpDescription = TEXT("Asset validation commandlet for CI pipelines");
    HelpUsage = TEXT("UnrealEditor-Cmd.exe MyProject -run=GSDValidateAssets");
    HelpParamNames.Add(TEXT("maxsize"));
    HelpParamDescriptions.Add(TEXT("Maximum asset size in MB (default: 100)"));
    HelpParamNames.Add(TEXT("output"));
    HelpParamDescriptions.Add(TEXT("Output file path for JSON report"));
}

int32 UGSDValidateAssetsCommandlet::Main(const FString& Params)
{
    GSDTELEMETRY_LOG(Log, TEXT("GSDValidateAssetsCommandlet starting..."));

    ParseParameters(Params);

    TArray<FString> Errors;
    int32 TotalAssets = 0;
    int32 FailedAssets = 0;

    // Iterate all assets
    for (TObjectIterator<UObject> It; It; ++It)
    {
        UObject* Asset = *It;
        if (!Asset->IsAsset()) continue;

        TotalAssets++;
        if (!ValidateAsset(Asset, Errors))
        {
            FailedAssets++;
        }
    }

    const bool bSuccess = FailedAssets == 0;

    if (bOutputJSON)
    {
        OutputJSON(bSuccess, TotalAssets, FailedAssets, Errors);
    }

    return bSuccess ? 0 : 1;
}

void UGSDValidateAssetsCommandlet::OutputJSON(bool bSuccess, int32 TotalAssets,
    int32 FailedAssets, const TArray<FString>& Errors)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetBoolField(TEXT("success"), bSuccess);
    JsonObject->SetNumberField(TEXT("total_assets"), TotalAssets);
    JsonObject->SetNumberField(TEXT("failed_assets"), FailedAssets);

    TArray<TSharedPtr<FJsonValue>> ErrorArray;
    for (const FString& Error : Errors)
    {
        ErrorArray.Add(MakeShareable(new FJsonValueString(Error)));
    }
    JsonObject->SetArrayField(TEXT("errors"), ErrorArray);

    FString OutputString;
    TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

    // stdout for CI parsing
    fprintf(stdout, "%s\n", *OutputString);
    fflush(stdout);

    GSDTELEMETRY_LOG(Log, TEXT("JSON_OUTPUT: %s"), *OutputString);
}
```

### Pattern 2: Performance Telemetry Subsystem
**What:** GameInstanceSubsystem for tracking frame time and hitches by district
**When to use:** TEL-01 (frame time/hitch tracking), TEL-02 (actor counts)
**Example:**
```cpp
// Source: Extends existing GSDStreamingTelemetry pattern
// File: GSDPerformanceTelemetry.h
UCLASS(Config=Game, DefaultConfig)
class GSD_TELEMETRY_API UGSDPerformanceTelemetry : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Frame time tracking
    void RecordFrameTime(float FrameTimeMs, const FName& DistrictName);
    void RecordHitch(float HitchTimeMs, const FName& DistrictName);

    // Actor counting
    void RecordActorCounts(int32 VehicleCount, int32 ZombieCount, int32 HumanCount);

    // Data access
    UFUNCTION(BlueprintPure, Category = "GSD|Telemetry")
    float GetAverageFrameTimeMs(const FName& DistrictName) const;

    UFUNCTION(BlueprintPure, Category = "GSD|Telemetry")
    int32 GetHitchCount(const FName& DistrictName) const;

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "GSD|Telemetry")
    FOnHitchDetected OnHitchDetected;

    // Config
    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Performance")
    float HitchThresholdMs = 16.67f;  // 60fps target

    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Performance")
    int32 FrameHistorySize = 60;  // Circular buffer for O(1) averaging

private:
    TMap<FName, FGSDFrameTimeHistory> DistrictFrameTimes;
    TMap<FName, int32> DistrictHitchCounts;
    FTimerHandle ActorCountTimerHandle;
};

// File: GSDTelemetryTypes.h
USTRUCT(BlueprintType)
struct FGSDFrameTimeHistory
{
    GENERATED_BODY()

    TArray<float> FrameTimes;
    int32 WriteIndex = 0;
    float TotalTime = 0.0f;

    void AddFrameTime(float TimeMs)
    {
        if (FrameTimes.Num() < MaxSize)
        {
            FrameTimes.Add(TimeMs);
            TotalTime += TimeMs;
        }
        else
        {
            TotalTime -= FrameTimes[WriteIndex];
            FrameTimes[WriteIndex] = TimeMs;
            TotalTime += TimeMs;
            WriteIndex = (WriteIndex + 1) % MaxSize;
        }
    }

    float GetAverageMs() const
    {
        return FrameTimes.Num() > 0 ? TotalTime / FrameTimes.Num() : 0.0f;
    }

    static constexpr int32 MaxSize = 60;  // 60 frame history
};
```

### Pattern 3: EditorUtilityWidget for Validation UI
**What:** UMG-based dockable panel for one-click validation
**When to use:** TEL-07 (Editor validation UI)
**Example:**
```cpp
// Source: UE5 EditorUtilityWidget pattern
// File: GSDValidationDashboardWidget.h
UCLASS(BlueprintType)
class GSD_VALIDATIONTOOLS_API UGSDValidationDashboardWidget : public UEditorUtilityWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "GSD|Validation")
    void RunAllValidations();

    UFUNCTION(BlueprintCallable, Category = "GSD|Validation")
    void ValidateAssets();

    UFUNCTION(BlueprintCallable, Category = "GSD|Validation")
    void ValidateWorldPartition();

    UFUNCTION(BlueprintCallable, Category = "GSD|Validation")
    void RunPerformanceRoute();

    UPROPERTY(BlueprintReadOnly, Category = "GSD|Validation")
    FString LastValidationResult;

    UPROPERTY(BlueprintReadOnly, Category = "GSD|Validation")
    bool bLastValidationPassed;

    UPROPERTY(BlueprintAssignable, Category = "GSD|Validation")
    FOnValidationComplete OnValidationComplete;
};
```

### Pattern 4: Asset Budget Enforcement via Build.cs
**What:** Build script validation that fails builds when budgets exceeded
**When to use:** TEL-08 (Asset budget enforcement)
**Example:**
```csharp
// File: GSD_ValidationTools.Build.cs
// This would be a separate console app or UAT script, not Build.cs directly
// Real implementation uses RunUAT with custom script

// For CI integration, create a Python script:
// tools/validate_asset_budget.py
import os
import json
import sys

def get_asset_sizes(content_dir):
    """Scan Content directory for asset sizes"""
    asset_sizes = {}
    for root, dirs, files in os.walk(content_dir):
        for file in files:
            if file.endswith('.uasset'):
                path = os.path.join(root, file)
                size_mb = os.path.getsize(path) / (1024 * 1024)
                asset_sizes[path] = size_mb
    return asset_sizes

def validate_budget(config_path, content_dir):
    """Validate assets against budget configuration"""
    with open(config_path) as f:
        config = json.load(f)

    budgets = config.get('asset_budgets', {})
    asset_sizes = get_asset_sizes(content_dir)

    errors = []
    for asset_path, size_mb in asset_sizes.items():
        asset_type = get_asset_type(asset_path)
        budget_mb = budgets.get(asset_type, budgets.get('default', 100))

        if size_mb > budget_mb:
            errors.append(f"{asset_path}: {size_mb:.2f}MB exceeds {budget_mb}MB budget")

    return len(errors) == 0, errors

if __name__ == '__main__':
    success, errors = validate_budget('Config/AssetBudgets.json', 'Content/')
    if not success:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        sys.exit(1)
    print("Asset budget validation passed")
    sys.exit(0)
```

### Anti-Patterns to Avoid
- **Per-frame delegate broadcasts:** Use MinBroadcastInterval throttling (existing pattern in GSDStreamingTelemetry)
- **TObjectIterator in PIE:** Returns editor objects too; use TActorIterator with world parameter
- **String-based tag comparison:** Use FGameplayTag for O(1) hierarchical matching
- **Unthrottled telemetry writes:** Pre-allocate circular buffers, batch writes
- **Synchronous asset scanning in builds:** Cache results, use incremental validation

## Don't Hand-Roll

Problems that look simple but have existing solutions:

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Frame time averaging | Circular buffer from scratch | TArray with write index (existing pattern) | Edge cases: wraparound, empty state |
| Hitch detection | Custom threshold logic | UGSDPerformanceConfig::IsHitch() | Already exists with config support |
| JSON serialization | FString::Printf formatting | FJsonObjectConverter | Handles nested types, arrays, escaping |
| Actor enumeration | TObjectIterator<UObject> | TActorIterator<AClass>(World) | PIE safety, type filtering |
| Performance timing | GetTickCount() or custom | SCOPE_CYCLE_COUNTER | Stats system integration, shipping builds |
| CI exit codes | Custom error handling | Return 0/1 from Main() | Standard convention, tool integration |

**Key insight:** UE5 provides mature telemetry infrastructure. Custom solutions add complexity without benefit. The existing GSDStreamingTelemetry pattern already implements throttling, circular buffering, and delegate broadcasting correctly.

## Common Pitfalls

### Pitfall 1: Telemetry Overhead Degrading Performance
**What goes wrong:** Per-frame telemetry logging causes frame time spikes
**Why it happens:** String formatting, delegate broadcasts, and file I/O on hot paths
**How to avoid:**
- Use MinBroadcastInterval (existing pattern: 0.1f seconds)
- Pre-allocate circular buffers (existing pattern: Reserve(MaxRecentEvents))
- Batch telemetry writes, never write per-frame
**Warning signs:** Frame time correlates with telemetry volume, stat Game shows telemetry overhead

### Pitfall 2: Commandlet World Context Missing
**What goes wrong:** Commandlet crashes when accessing World-dependent subsystems
**Why it happens:** Commandlets run without a game world by default
**How to avoid:**
- Check GWorld and GetWorld() for nullptr
- Create PIE world explicitly if needed: `UWorld* TestWorld = NewObject<UWorld>()`
- Use GameInstance subsystems for world-independent data
**Warning signs:** nullptr access in Main(), subsystem Get() returning null

### Pitfall 3: PIE Object Contamination in Counts
**What goes wrong:** Actor counts include editor-only actors, inflating numbers
**Why it happens:** TObjectIterator iterates all UObjects including editor objects
**How to avoid:**
- Use TActorIterator with explicit World parameter
- Check `It->GetWorld() == TargetWorld` before counting
- Use world context from GameInstance for filtering
**Warning signs:** Counts differ between PIE and standalone, unexpected actor types

### Pitfall 4: Asset Budget False Positives
**What goes wrong:** Build fails on assets that are legitimately large
**Why it happens:** Hard-coded thresholds without exceptions
**How to avoid:**
- Use per-asset-type budgets (StaticMesh: 50MB, Texture: 100MB)
- Allow override metadata: `AssetBudgetOverride="true"`
- Report warnings before failing, grace period for remediation
**Warning signs:** Failed builds on known-large assets, developer frustration

### Pitfall 5: Stats Disabled in Shipping
**What goes wrong:** Telemetry silently stops working in shipping builds
**Why it happens:** SCOPE_CYCLE_COUNTER compiled out in shipping
**How to avoid:**
- Use conditional compilation: `#if !UE_BUILD_SHIPPING`
- For shipping telemetry, use TRACE_CPUPROFILER_EVENT_SCOPE (works in all builds)
- Test commandlets in Shipping configuration
**Warning signs:** Zero metrics in telemetry, empty JSON reports

## Code Examples

Verified patterns from official sources and existing codebase:

### Frame Time Tracking with Circular Buffer (Based on Phase 6 Pattern)
```cpp
// Source: Extends GSDStreamingTelemetry pattern from Phase 3
// Uses 60-frame circular buffer for O(1) averaging

void UGSDPerformanceTelemetry::RecordFrameTime(float FrameTimeMs, const FName& DistrictName)
{
    FGSDFrameTimeHistory& History = DistrictFrameTimes.FindOrAdd(DistrictName);
    History.AddFrameTime(FrameTimeMs);

    // Check for hitch
    if (FrameTimeMs > HitchThresholdMs)
    {
        RecordHitch(FrameTimeMs, DistrictName);
    }
}

float UGSDPerformanceTelemetry::GetAverageFrameTimeMs(const FName& DistrictName) const
{
    const FGSDFrameTimeHistory* History = DistrictFrameTimes.Find(DistrictName);
    return History ? History->GetAverageMs() : 0.0f;
}
```

### World Partition Validation Commandlet
```cpp
// Source: UE5 WorldPartitionBuilderCommandlet pattern
// File: GSDValidateWPCommandlet.cpp

int32 UGSDValidateWPCommandlet::Main(const FString& Params)
{
    GSDTELEMETRY_LOG(Log, TEXT("GSDValidateWPCommandlet starting..."));

    TArray<FString> Errors;
    int32 CellsValidated = 0;
    int32 CellsFailed = 0;

    // Get World Partition world
    UWorld* World = GWorld;
    if (!World)
    {
        GSDTELEMETRY_LOG(Error, TEXT("No world available"));
        return 1;
    }

    UWorldPartition* WP = World->GetWorldPartition();
    if (!WP)
    {
        Errors.Add(TEXT("World does not use World Partition"));
        OutputJSON(false, 0, 1, Errors);
        return 1;
    }

    // Validate cell configuration
    const FVector2D CellSize = WP->GetCellSize();
    if (CellSize.X < MinCellSize || CellSize.Y < MinCellSize)
    {
        Errors.Add(FString::Printf(TEXT("Cell size too small: %.0fx%.0f (min: %.0f)"),
            CellSize.X, CellSize.Y, MinCellSize));
        CellsFailed++;
    }
    else
    {
        CellsValidated++;
    }

    // Validate HLOD layers
    const TArray<UHLODLayer*>& HLODLayers = WP->GetHLODLayers();
    if (HLODLayers.Num() < MinHLODLayers)
    {
        Errors.Add(FString::Printf(TEXT("Insufficient HLOD layers: %d (min: %d)"),
            HLODLayers.Num(), MinHLODLayers));
        CellsFailed++;
    }
    else
    {
        CellsValidated++;
    }

    const bool bSuccess = CellsFailed == 0;
    OutputJSON(bSuccess, CellsValidated, CellsFailed, Errors);

    return bSuccess ? 0 : 1;
}
```

### Actor Counting with Periodic Logging
```cpp
// Source: TActorIterator pattern from UE5 docs
// File: GSDPerformanceTelemetry.cpp

void UGSDPerformanceTelemetry::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Start periodic actor counting timer
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(
            ActorCountTimerHandle,
            FTimerDelegate::CreateUObject(this, &UGSDPerformanceTelemetry::CountActors),
            ActorCountInterval,  // e.g., 5.0 seconds
            true  // Loop
        );
    }
}

void UGSDPerformanceTelemetry::CountActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    int32 VehicleCount = 0;
    int32 ZombieCount = 0;
    int32 HumanCount = 0;

    // Count vehicles
    for (TActorIterator<AGSDVehiclePawn> It(World); It; ++It)
    {
        VehicleCount++;
    }

    // Count zombies (Mass Entity - use crowd manager)
    if (UGSDCrowdManagerSubsystem* CrowdManager = World->GetSubsystem<UGSDCrowdManagerSubsystem>())
    {
        ZombieCount = CrowdManager->GetActiveEntityCount();
    }

    // Count human NPCs
    for (TActorIterator<AGSDHeroNPC> It(World); It; ++It)
    {
        HumanCount++;
    }

    RecordActorCounts(VehicleCount, ZombieCount, HumanCount);

    GSDTELEMETRY_LOG(Verbose, TEXT("Actor counts: Vehicles=%d, Zombies=%d, Humans=%d"),
        VehicleCount, ZombieCount, HumanCount);
}
```

### Stats Declaration for Performance Tracking
```cpp
// Source: UE5 Stats System documentation
// File: GSDTelemetryLog.h

#pragma once

#include "CoreMinimal.h"

// Declare log category
GSD_TELEMETRY_API DECLARE_LOG_CATEGORY_EXTERN(LogGSDTelemetry, Log, All);

// Convenience macros
#define GSDTELEMETRY_LOG(Verbosity, Format, ...) \
    UE_LOG(LogGSDTelemetry, Verbosity, Format, ##__VA_ARGS__)

// File: GSDTelemetryStats.h
#pragma once

#include "CoreMinimal.h"

// Declare stats group
DECLARE_STATS_GROUP(TEXT("GSD Telemetry"), STATGROUP_GSDTelemetry, STATCAT_Advanced);

// Declare individual stats
DECLARE_CYCLE_STAT(TEXT("RecordFrameTime"), STAT_GSDRecordFrameTime, STATGROUP_GSDTelemetry);
DECLARE_CYCLE_STAT(TEXT("CountActors"), STAT_GSDCountActors, STATGROUP_GSDTelemetry);
DECLARE_DWORD_COUNTER_STAT(TEXT("TotalHitchCount"), STAT_GSDTotalHitchCount, STATGROUP_GSDTelemetry);

// Usage in code:
void UGSDPerformanceTelemetry::RecordFrameTime(float FrameTimeMs, const FName& DistrictName)
{
    SCOPE_CYCLE_COUNTER(STAT_GSDRecordFrameTime);
    // ... implementation
}

// View in console: stat GSDTelemetry
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Custom timing via GetTickCount() | SCOPE_CYCLE_COUNTER + Unreal Insights | UE4.0+ | Zero-overhead in shipping, integrated tooling |
| Per-frame telemetry logging | Throttled batched updates | Phase 3 | Frame budget protection |
| Manual JSON string formatting | FJsonObjectConverter | UE4.20+ | Type safety, consistent output |
| Slate-only editor tools | EditorUtilityWidget (UMG) | UE4.22+ | Faster development, Blueprint access |
| Text output for CI | JSON with stdout | Phase 5+ | Machine-parseable results |

**Deprecated/outdated:**
- `stat unit` for frame breakdown: Use Unreal Insights for detailed analysis
- `FOutputDeviceFile` for logging: Use `UE_LOG` with file redirect
- Manual delegate broadcasting every frame: Use MinBroadcastInterval throttling

## Open Questions

Things that couldn't be fully resolved:

1. **District Definition Strategy**
   - What we know: Need to track metrics "by district" (TEL-01)
   - What's unclear: How districts are defined (bounds, names, overlap)
   - Recommendation: Start with FName-based district identification, let game code call `RecordFrameTime(time, DistrictName)`. Districts can be data-driven via config.

2. **Performance Route Recording**
   - What we know: GSDRunPerfRoute commandlet captures baseline (TEL-06)
   - What's unclear: Route definition format (spline, waypoints, recording)
   - Recommendation: Use AActor-based waypoints in a test map. Commandlet spawns player, follows spline, records metrics at each waypoint.

3. **Asset Budget Configuration Format**
   - What we know: Budgets should be configurable per-asset-type
   - What's unclear: Preferred config format (JSON, ini, DataAsset)
   - Recommendation: JSON for CI tooling (parseable), UDeveloperSettings for editor access. Both can reference same source.

## Sources

### Primary (HIGH confidence)
- Existing GSDVehicleTestCommandlet.cpp - Commandlet structure, JSON output
- Existing GSDCrowdTestCommandlet.cpp - Multi-phase testing, JSON output
- Existing GSDEventTestCommandlet.cpp - Parameter parsing, output formats
- Existing GSDStreamingTelemetry.cpp - Throttling, circular buffer patterns
- Existing UGSDPerformanceConfig - Hitch thresholds, frame budgets
- UE5 Official Documentation: Editor Utility Widgets (https://dev.epicgames.com/documentation/zh-cn/unreal-engine/editor-utility-widgets-in-unreal-engine)
- UE5 Official Documentation: World Partition Commandlets (https://dev.epicgames.com/documentation/zh-cn/unreal-engine/world-partition-builder-commandlet-reference)

### Secondary (MEDIUM confidence)
- UE5 Stats System documentation (DECLARE_CYCLE_STAT, SCOPE_CYCLE_COUNTER)
- UE5 Console Commands Reference (GameplayTags.DumpTagList, stat commands)
- UE5 Visual Logger documentation (TRACE_CPUPROFILER_EVENT_SCOPE)
- Community tutorials on EditorUtilityWidget implementation

### Tertiary (LOW confidence)
- Asset budget enforcement patterns - custom solution, no standard UE5 pattern found
- Performance route recording - no standard UE5 pattern, requires custom implementation

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH - All components are UE5 native with existing codebase patterns
- Architecture: HIGH - Existing commandlets and telemetry provide clear patterns to follow
- Pitfalls: HIGH - Based on documented UE5 behaviors and existing codebase patterns

**Research date:** 2026-02-27
**Valid until:** 2026-03-31 (UE5 APIs stable, patterns should remain valid)
