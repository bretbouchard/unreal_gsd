using UnrealBuildTool;

public class GSD_ValidationTools : ModuleRules
{
    public GSD_ValidationTools(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "Json",
            "JsonUtilities",
            "EditorScriptingUtilities",
            "Blutility",
            "UMG",
            "UMGEditor",
            "GSD_Core"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "UnrealEd",
            "AutomationController",
            "GSD_Telemetry"
        });
    }
}
