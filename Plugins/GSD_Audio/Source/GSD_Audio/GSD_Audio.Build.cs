// Copyright Bret Bouchard

using UnrealBuildTool;

public class GSD_Audio : ModuleRules
{
	public GSD_Audio(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"GSD_Core",
			"AudioEditor"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Engine"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");
	}
}
