using UnrealBuildTool;

public class GSD_CityStreaming : ModuleRules
{
    public GSD_CityStreaming(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "GSD_Core",
            "WorldPartition"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
        });

        // Enable automation tests for editor builds
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] {
                "UnrealEd",
                "EditorScriptingUtilities",
                "AutomationController"
            });
        }
    }
}
