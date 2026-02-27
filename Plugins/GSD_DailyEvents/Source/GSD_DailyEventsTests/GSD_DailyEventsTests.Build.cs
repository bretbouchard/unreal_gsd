// Copyright Bret Bouchard. All Rights Reserved.

using UnrealBuildTool;

public class GSD_DailyEventsTests : ModuleRules
{
    public GSD_DailyEventsTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "GSD_Core"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "GSD_DailyEvents",
            "AutomationController"
        });
    }
}
