using UnrealBuildTool;

public class GSD_Crowds : ModuleRules
{
    public GSD_Crowds(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "GSD_Core",
            "GSD_CityStreaming",
            "MassEntity",
            "MassRepresentation",
            "MassSpawner",
            "MassLOD",
            "MassCommon",
            "MassAI",           // Moved from Private - AI behaviors and navigation
            "ZoneGraph",        // ZoneGraph lane navigation
            "MassMovement",     // Movement along lanes
            "StateTreeModule",  // Behavior control
            "SmartObjectsModule", // Interaction points
            "AIModule",         // Behavior Trees, AI Perception
            "NavigationSystem", // NavMesh for hero NPCs
            "GameplayTags",     // For FGameplayTag in density modifiers (EVT-09)
            "EnhancedInput"     // Input actions and mapping contexts for debug controls
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            // MassAI moved to Public for Phase 7 navigation support
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
