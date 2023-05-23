using UnrealBuildTool;

public class RotaryWingAircraft : ModuleRules {
	public RotaryWingAircraft(ReadOnlyTargetRules target) : base(target) {
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new [] {
			"AnimGraphRuntime",
			"Core",
			"EnhancedInput",
			"Slate",
			"SlateCore",
			"UMG",
		});

		PrivateDependencyModuleNames.AddRange(new [] {
			"CoreUObject",
			"Engine",
			"PhysicsCore",
			"RenderCore",
			"RHI",
			"RHICore",
		});
	}
}
