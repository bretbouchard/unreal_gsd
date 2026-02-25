using UnrealBuildTool;

public class GSD_Vehicles : ModuleRules
{
    public GSD_Vehicles(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "GSD_Core",
            "GSD_CityStreaming",
            "DeveloperSettings"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "ChaosVehicles",
            "ChaosVehiclesCore"
        });

        // Enable automation tests for editor builds
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] {
                "UnrealEd",
                "AutomationController"
            });
        }
    }
}
