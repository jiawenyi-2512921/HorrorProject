// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HorrorProjectEditor : ModuleRules
{
	public HorrorProjectEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"UnrealEd",
			"EditorSubsystem",
			"AssetTools",
			"ContentBrowser",
			"Slate",
			"SlateCore",
			"EditorStyle",
			"ToolMenus",
			"WorkspaceMenuStructure",
			"PropertyEditor",
			"HorrorProject"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"InputCore",
			"LevelEditor",
			"Projects",
			"RenderCore",
			"RHI",
			"AudioEditor",
			"MaterialEditor"
		});
	}
}
