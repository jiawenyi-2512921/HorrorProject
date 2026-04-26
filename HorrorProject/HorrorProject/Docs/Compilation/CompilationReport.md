# Compilation Report

Generated: 2026-04-27 01:35:44

## Project

- Project file: D:\gptzuo\HorrorProject\HorrorProject\HorrorProject.uproject
- Engine association: 5.6
- Platform target: Win64
- Runtime target: HorrorProject
- Editor target: HorrorProjectEditor

## Source Inventory

- VFX cpp files: 4
- Audio cpp files: 12
- UI cpp files: 13
- Runtime test cpp files: 59
- Editor tool cpp files: 4

## Runtime Module Dependencies

- AIModule
- Core
- CoreUObject
- Engine
- EngineSettings
- EnhancedInput
- GameplayStateTreeModule
- GameplayTags
- HorrorProject
- HorrorProject/Analytics/Public
- HorrorProject/Settings/Public
- HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS=0
- InputCore
- Json
- JsonUtilities
- NavigationSystem
- Networking
- Niagara
- OnlineSubsystem
- OnlineSubsystemUtils
- RenderCore
- RHI
- Slate
- SlateCore
- Sockets
- StateTreeModule
- UMG

## Editor Module Dependencies

- AssetTools
- AudioEditor
- ContentBrowser
- Core
- CoreUObject
- EditorStyle
- EditorSubsystem
- Engine
- HorrorProject
- InputCore
- LevelEditor
- MaterialEditor
- Projects
- PropertyEditor
- RenderCore
- RHI
- Slate
- SlateCore
- ToolMenus
- UnrealEd
- WorkspaceMenuStructure

## Validation Entry Points

- Game build: Scripts\Validation\ValidateCompilation.ps1
- Editor build: Scripts\Validation\ValidateCompilation.ps1 -EditorOnly
- Dependency scan: Scripts\Validation\CheckDependencies.ps1
- Include scan: Scripts\Validation\CheckIncludes.ps1
- PowerShell syntax scan: Scripts\Validation\ValidatePowerShellSyntax.ps1

## Current Engineering Notes

- Niagara, UMG, networking, rendering, navigation, JSON, and editor dependencies are declared in Build.cs.
- Legacy automation tests are intentionally quarantined by HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS=0 until they are migrated to current APIs.
- PowerShell syntax scan currently covers 199 scripts.
- PowerShell files with syntax errors: 0
- PowerShell syntax error count: 0

## Recommended Next Work

1. Keep Game and Editor builds at zero warnings.
2. Migrate legacy automation tests module by module before re-enabling them.
3. Repair or retire corrupted PowerShell tools, starting with validation, build, package, and final-integration scripts.
4. Add Cook, Package, and Editor smoke tests after script syntax debt is reduced.
