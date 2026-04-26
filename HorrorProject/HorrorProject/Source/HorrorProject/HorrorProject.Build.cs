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
			"Niagara",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"Sockets",
			"Networking",
			"Slate",
			"EngineSettings",
			"RHI",
			"RenderCore",
			"NavigationSystem",
			"Json",
			"JsonUtilities"
		});

		PublicDefinitions.Add("HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS=0");

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PrivateIncludePaths.AddRange(new string[] {
			"HorrorProject",
			"HorrorProject/Analytics/Public",
			"HorrorProject/Settings/Public"
		});
	}
}
