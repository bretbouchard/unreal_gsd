---
phase: 01-plugin-architecture-foundation
plan: 01
type: execute
wave: 1
depends_on: []
files_modified:
  - Plugins/GSD_Core/GSD_Core.uplugin
  - Plugins/GSD_Core/Source/GSD_Core/GSD_Core.Build.cs
  - Plugins/GSD_Core/Source/GSD_Core/Public/GSDLog.h
  - Plugins/GSD_Core/Source/GSD_Core/Private/GSDLog.cpp
autonomous: true
must_haves:
  truths:
    - "GSD_Core plugin appears in Unreal Editor plugin list"
    - "Plugin loads without errors on editor startup"
    - "GSD_LOG macros compile and output to LogGSD category"
    - "Build.cs correctly configures PreDefault loading phase"
  artifacts:
    - path: "Plugins/GSD_Core/GSD_Core.uplugin"
      provides: "Plugin manifest with PreDefault loading phase"
      contains: '"LoadingPhase": "PreDefault"'
    - path: "Plugins/GSD_Core/Source/GSD_Core/GSD_Core.Build.cs"
      provides: "Module build configuration"
      exports: ["GSD_Core"]
    - path: "Plugins/GSD_Core/Source/GSD_Core/Public/GSDLog.h"
      provides: "Logging macros for GSD modules"
      contains: "DECLARE_LOG_CATEGORY_EXTERN"
  key_links:
    - from: "GSD_Core.uplugin"
      to: "GSD_Core module"
      via: "Modules array"
      pattern: '"Name": "GSD_Core"'
---

# 01-PLAN: GSD_Core Plugin Structure

**Requirements:** PLUG-01, PLUG-10

**Objective:** Create the foundational GSD_Core plugin with correct directory structure, loading phase configuration, and logging infrastructure that all other GSD plugins will depend on.

**Purpose:** Establish the plugin skeleton that enables all subsequent interface and class implementations. The PreDefault loading phase ensures GSD_Core loads before any feature plugins.

**Output:** Compilable plugin structure with working logging macros.

---

## Context

@.planning/PROJECT.md
@.planning/ROADMAP.md
@.planning/STATE.md
@.planning/phases/01-plugin-architecture-foundation/01-RESEARCH.md
@.planning/phases/01-plugin-architecture-foundation/1-CONTEXT.md

---

## Tasks

<task type="auto">
  <name>T1: Create Plugin Directory Structure</name>
  <files>
    Plugins/GSD_Core/GSD_Core.uplugin
    Plugins/GSD_Core/Source/GSD_Core/Public/
    Plugins/GSD_Core/Source/GSD_Core/Private/
    Plugins/GSD_Core/Source/GSD_Core/GSD_Core.Build.cs
    Plugins/GSD_Core/Resources/Icon128.png
  </files>
  <action>
    Create the complete GSD_Core plugin directory structure following UE5 conventions:

    ```
    Plugins/GSD_Core/
    ├── GSD_Core.uplugin
    ├── Resources/
    │   └── Icon128.png (placeholder 128x128 PNG)
    └── Source/
        └── GSD_Core/
            ├── GSD_Core.Build.cs
            ├── Public/
            │   ├── Interfaces/
            │   ├── Classes/
            │   ├── DataAssets/
            │   ├── Managers/
            │   ├── Types/
            │   └── Macros/
            └── Private/
                ├── Interfaces/
                ├── Classes/
                ├── DataAssets/
                ├── Managers/
                ├── Types/
                └── Tests/
    ```

    Create GSD_Core.uplugin with PreDefault loading phase:
    ```json
    {
        "FileVersion": 3,
        "Version": 1,
        "VersionName": "1.0.0",
        "FriendlyName": "GSD Core",
        "Description": "Core interfaces and base classes for GSD Platform",
        "Category": "GSD",
        "CreatedBy": "Bret Bouchard",
        "CreatedByURL": "",
        "DocsURL": "",
        "MarketplaceURL": "",
        "SupportURL": "",
        "CanContainContent": false,
        "IsBetaVersion": false,
        "Installed": false,
        "Modules": [
            {
                "Name": "GSD_Core",
                "Type": "Runtime",
                "LoadingPhase": "PreDefault",
                "AdditionalDependencies": [
                    "Engine",
                    "CoreUObject"
                ]
            }
        ]
    }
    ```

    Create GSD_Core.Build.cs:
    ```csharp
    using UnrealBuildTool;

    public class GSD_Core : ModuleRules
    {
        public GSD_Core(ReadOnlyTargetRules Target) : base(Target)
        {
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

            PublicDependencyModuleNames.AddRange(new string[] {
                "Core",
                "CoreUObject",
                "Engine"
            });

            PrivateDependencyModuleNames.AddRange(new string[] {
                "AssetRegistry"
            });
        }
    }
    ```

    Create a placeholder Icon128.png (solid color 128x128 image).
  </action>
  <verify>
    - Directory structure exists with all folders
    - GSD_Core.uplugin is valid JSON
    - GSD_Core.Build.cs compiles without errors
    - Plugin appears in Unreal Editor plugin browser
  </verify>
  <done>
    GSD_Core plugin directory exists with correct structure, uplugin file has PreDefault loading phase, Build.cs configures Core/CoreUObject/Engine dependencies.
  </done>
</task>

<task type="auto">
  <name>T2: Implement Logging Macros</name>
  <files>
    Plugins/GSD_Core/Source/GSD_Core/Public/GSDLog.h
    Plugins/GSD_Core/Source/GSD_Core/Private/GSDLog.cpp
  </files>
  <action>
    Create the GSD logging system that all modules will use for consistent output.

    GSDLog.h:
    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "GSDLog.generated.h"

    // Declare log category (header)
    GSD_CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogGSD, Log, All);

    // Convenience macros for consistent logging
    #define GSD_LOG(Verbosity, Format, ...) \
        UE_LOG(LogGSD, Verbosity, TEXT(Format), ##__VA_ARGS__)

    #define GSD_WARN(Format, ...) \
        UE_LOG(LogGSD, Warning, TEXT(Format), ##__VA_ARGS__)

    #define GSD_ERROR(Format, ...) \
        UE_LOG(LogGSD, Error, TEXT(Format), ##__VA_ARGS__)

    #define GSD_TRACE(Format, ...) \
        UE_LOG(LogGSD, Verbose, TEXT(Format), ##__VA_ARGS__)

    #define GSD_VERY_TRACE(Format, ...) \
        UE_LOG(LogGSD, VeryVerbose, TEXT(Format), ##__VA_ARGS__)
    ```

    GSDLog.cpp:
    ```cpp
    #include "GSDLog.h"

    // Define log category (source)
    DEFINE_LOG_CATEGORY(LogGSD);
    ```

    These macros provide:
    - GSD_LOG(Log, ...) - Standard logging
    - GSD_WARN(...) - Warnings
    - GSD_ERROR(...) - Errors
    - GSD_TRACE(...) - Verbose debugging
    - GSD_VERY_TRACE(...) - Very verbose debugging
  </action>
  <verify>
    - GSDLog.h compiles without errors
    - GSDLog.cpp compiles without errors
    - Test macro usage: GSD_LOG(Log, "Test message %s", *FString("test")) compiles
    - LogGSD category appears in console output filter
  </verify>
  <done>
    Logging macros compile and are ready for use by all GSD modules. LogGSD category is defined and can be filtered in output log.
  </done>
</task>

<task type="auto">
  <name>T3: Verify Plugin Loads in Editor</name>
  <files>
    Plugins/GSD_Core/Source/GSD_Core/Private/GSD_Core.cpp (module implementation)
  </files>
  <action>
    Create minimal module implementation to verify plugin loads correctly:

    GSD_Core.cpp:
    ```cpp
    #include "CoreMinimal.h"
    #include "Modules/ModuleManager.h"
    #include "GSDLog.h"

    #define LOCTEXT_NAMESPACE "FGSD_CoreModule"

    class FGSD_CoreModule : public IModuleInterface
    {
    public:
        virtual void StartupModule() override
        {
            GSD_LOG(Log, "GSD_Core module starting up");
        }

        virtual void ShutdownModule() override
        {
            GSD_LOG(Log, "GSD_Core module shutting down");
        }
    };

    IMPLEMENT_MODULE(FGSD_CoreModule, GSD_Core)

    #undef LOCTEXT_NAMESPACE
    ```

    Steps to verify:
    1. Generate project files (if using Visual Studio)
    2. Build the project
    3. Open Unreal Editor
    4. Check Output Log for "GSD_Core module starting up"
    5. Verify plugin appears in Edit > Plugins > GSD category
    6. Verify plugin is enabled
  </action>
  <verify>
    - Project compiles without errors
    - Unreal Editor launches successfully
    - Output Log shows "GSD_Core module starting up"
    - Plugin appears in Edit > Plugins under GSD category
    - Plugin is enabled by default
  </verify>
  <done>
    GSD_Core plugin compiles, loads in Unreal Editor, logs startup message, and appears in plugin browser under GSD category.
  </done>
</task>

---

## Verification Checklist

- [ ] GSD_Core.uplugin exists with valid JSON structure
- [ ] LoadingPhase is set to "PreDefault"
- [ ] GSD_Core.Build.cs compiles without errors
- [ ] All directory folders exist (Public/, Private/, subdirectories)
- [ ] GSDLog.h defines DECLARE_LOG_CATEGORY_EXTERN
- [ ] GSDLog.cpp defines DEFINE_LOG_CATEGORY
- [ ] Logging macros compile (GSD_LOG, GSD_WARN, GSD_ERROR)
- [ ] Plugin appears in Unreal Editor plugin browser
- [ ] Plugin loads without errors on editor startup
- [ ] LogGSD category visible in output log filter

---

## Success Criteria

| Criterion | Target |
|-----------|--------|
| Plugin compiles | 100% |
| Plugin loads in Editor | Yes |
| Logging macros work | Yes |
| PreDefault loading phase | Verified |

---

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| Build.cs syntax errors | Low | Medium | Use exact syntax from RESEARCH.md |
| Plugin not discovered | Low | High | Verify Plugins/ folder location |
| Module not loading | Low | High | Check IMPLEMENT_MODULE macro |

---

## Estimated Complexity

**Low** - Standard UE5 plugin boilerplate

---

## Output

After completion, create `.planning/phases/01-plugin-architecture-foundation/01-SUMMARY.md`
