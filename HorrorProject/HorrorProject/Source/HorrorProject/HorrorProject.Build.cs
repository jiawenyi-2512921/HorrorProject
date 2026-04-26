// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HorrorProject : ModuleRules
{
	public HorrorProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayTags",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"SlateCore",
			"Niagara"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PrivateIncludePaths.AddRange(new string[] {
			"HorrorProject"
		});
	}
}
