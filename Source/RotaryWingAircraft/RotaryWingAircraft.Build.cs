using UnrealBuildTool;

public class RotaryWingAircraft : ModuleRules {
	public RotaryWingAircraft(ReadOnlyTargetRules target) : base(target) {
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new [] {
			"AnimGraphRuntime",
			"Core",
			"EnhancedInput",
		});

		PrivateDependencyModuleNames.AddRange(new [] {
			"CoreUObject",
			"Engine",
			"PhysicsCore",
		});
	}
}
