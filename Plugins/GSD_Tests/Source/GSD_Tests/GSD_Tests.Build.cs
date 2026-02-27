// Copyright Bret Bouchard. All Rights Reserved.

using UnrealBuildTool;

public class GSD_Tests : ModuleRules
{
    public GSD_Tests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "GSD_Core",
            "GSD_Crowds",
            "GSD_Vehicles",
            "GSD_Telemetry",
            "MassEntity",
            "MassRepresentation",
            "MassSpawner",
            "AutomationController",
            "AutomationTest",
            "ChaosVehicles"
        });
    }
}
