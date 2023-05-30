using UnrealBuildTool;

public class RotaryWingAircraftEditor : ModuleRules {
	public RotaryWingAircraftEditor(ReadOnlyTargetRules target) : base(target) {
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new [] {
			"AnimGraph",
			"Core",
			"Projects",
			"RenderCore",
			"RotaryWingAircraft",
		});

		PrivateDependencyModuleNames.AddRange(new [] {
			"BlueprintGraph",
			"CoreUObject",
			"Engine",
			"UnrealEd",
			"Slate",
			"SlateCore",
		});
	}
}
