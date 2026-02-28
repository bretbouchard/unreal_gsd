using UnrealBuildTool;

public class GSD_DailyEvents : ModuleRules
{
    public GSD_DailyEvents(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "GSD_Core",         // UGSDDataAsset, UGSDDeterminismManager, LOG_GSD macros
            "GameplayTags",     // FGameplayTag, FGameplayTagContainer
            "GSD_Crowds",       // UGSDCrowdManagerSubsystem for density modifiers
            "Niagara",          // UNiagaraFunctionLibrary for FX spawning
            "NavigationSystem"  // ANavModifierVolume for navigation blocking
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            // Empty for now, will add GameplayMessaging later if needed
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
