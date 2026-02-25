using UnrealBuildTool;

public class GSD_Core : ModuleRules
{
    public GSD_Core(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "DeveloperSettings"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "AssetRegistry"
        });

        // Enable automation tests
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] {
                "UnrealEd",
                "EditorScriptingUtilities"
            });
        }
    }
}
