# Blueprint Tools Suite

Comprehensive toolset for Unreal Engine Blueprint development, validation, migration, and documentation.

## Overview

This suite provides PowerShell scripts and checklists to streamline Blueprint development workflows in Unreal Engine projects.

## Tools

### Generation Tools

#### GenerateBlueprintTemplate.ps1
Generate Blueprint template files with proper structure and metadata.

```powershell
.\GenerateBlueprintTemplate.ps1 -BlueprintName "BP_MyActor" -BlueprintType "Actor" -OutputPath "D:/Project/Content/Blueprints"
```

**Parameters:**
- `BlueprintName` - Name of the Blueprint (required)
- `BlueprintType` - Type: Actor, Component, Widget, GameMode, Character, Pawn, Interface, Struct, Enum, MacroLibrary, FunctionLibrary (required)
- `OutputPath` - Output directory (required)
- `ParentClass` - Custom parent class (optional)
- `Verbose` - Enable verbose logging (optional)

#### GenerateWidgetBlueprint.ps1
Generate Widget Blueprint templates for UMG UI.

```powershell
.\GenerateWidgetBlueprint.ps1 -WidgetName "WBP_MainMenu" -OutputPath "D:/Project/Content/UI" -WidgetType "Menu" -IncludeAnimations
```

**Parameters:**
- `WidgetName` - Name of the Widget Blueprint (required)
- `OutputPath` - Output directory (required)
- `WidgetType` - Type: HUD, Menu, Dialog, Tooltip, Inventory, Notification, Custom (optional)
- `IncludeAnimations` - Include animation templates (optional)
- `Verbose` - Enable verbose logging (optional)

#### GenerateActorBlueprint.ps1
Generate Actor Blueprint templates with components.

```powershell
.\GenerateActorBlueprint.ps1 -ActorName "BP_Door" -ActorType "Interactable" -OutputPath "D:/Project/Content/Blueprints/Actors" -IncludePhysics
```

**Parameters:**
- `ActorName` - Name of the Actor Blueprint (required)
- `OutputPath` - Output directory (required)
- `ActorType` - Type: Interactable, Prop, Enemy, Trigger, Pickup, Light, Audio, Custom (optional)
- `IncludePhysics` - Include physics components (optional)
- `Verbose` - Enable verbose logging (optional)

#### GenerateComponentBlueprint.ps1
Generate Component Blueprint templates.

```powershell
.\GenerateComponentBlueprint.ps1 -ComponentName "BP_HealthComponent" -ComponentType "Actor" -OutputPath "D:/Project/Content/Blueprints/Components"
```

**Parameters:**
- `ComponentName` - Name of the Component Blueprint (required)
- `OutputPath` - Output directory (required)
- `ComponentType` - Type: Scene, Actor, Movement, Audio, Particle, Light, Camera, Custom (optional)
- `Verbose` - Enable verbose logging (optional)

### Validation Tools

#### ValidateBlueprints.ps1
Validate Blueprint integrity and completeness.

```powershell
.\ValidateBlueprints.ps1 -BlueprintPath "D:/Project/Content/Blueprints" -Recursive -OutputReport
```

**Parameters:**
- `BlueprintPath` - Path to Blueprint file or directory (required)
- `Recursive` - Search recursively (optional)
- `OutputReport` - Generate detailed report (optional)
- `Verbose` - Enable verbose logging (optional)

**Checks:**
- Required fields (Type, Name, ParentClass, GUID)
- GUID format validation
- Naming conventions
- Component integrity
- Circular dependencies
- File size anomalies

#### CheckBlueprintReferences.ps1
Check Blueprint references and dependencies.

```powershell
.\CheckBlueprintReferences.ps1 -BlueprintPath "D:/Project/Content/Blueprints" -CheckExternal -FindUnused
```

**Parameters:**
- `BlueprintPath` - Path to Blueprint file or directory (required)
- `CheckExternal` - Check external asset references (optional)
- `FindUnused` - Find unused references (optional)
- `Verbose` - Enable verbose logging (optional)

**Features:**
- Extracts all Blueprint references
- Validates reference paths
- Finds unused references
- Generates dependency tree

#### FindBrokenBlueprints.ps1
Find broken or corrupted Blueprint files.

```powershell
.\FindBrokenBlueprints.ps1 -BlueprintPath "D:/Project/Content/Blueprints" -AutoFix -Quarantine
```

**Parameters:**
- `BlueprintPath` - Path to Blueprint file or directory (required)
- `AutoFix` - Attempt automatic fixes (optional)
- `Quarantine` - Move broken Blueprints to quarantine (optional)
- `Verbose` - Enable verbose logging (optional)

**Detects:**
- Empty files
- Malformed JSON
- Missing required fields
- Circular inheritance
- Duplicate component/variable names

#### AnalyzeBlueprintComplexity.ps1
Analyze Blueprint complexity metrics.

```powershell
.\AnalyzeBlueprintComplexity.ps1 -BlueprintPath "D:/Project/Content/Blueprints" -GenerateReport -Threshold 80
```

**Parameters:**
- `BlueprintPath` - Path to Blueprint file or directory (required)
- `GenerateReport` - Generate HTML report (optional)
- `Threshold` - Complexity threshold for warnings (default: 100)
- `Verbose` - Enable verbose logging (optional)

**Metrics:**
- Component count
- Variable count
- Function count
- Node count
- Cyclomatic complexity
- Maintainability index
- Lines of code estimate

### Migration Tools

#### MigrateBlueprintAssets.ps1
Migrate Blueprint assets between directories or projects.

```powershell
.\MigrateBlueprintAssets.ps1 -SourcePath "D:/Old/BP_Actor.uasset" -DestinationPath "D:/New/" -IncludeDependencies -UpdateReferences
```

**Parameters:**
- `SourcePath` - Source Blueprint path (required)
- `DestinationPath` - Destination path (required)
- `IncludeDependencies` - Migrate dependencies automatically (optional)
- `UpdateReferences` - Update references in other Blueprints (optional)
- `DryRun` - Preview migration without executing (optional)
- `Verbose` - Enable verbose logging (optional)

**Features:**
- Dependency tracking
- Metadata preservation
- Reference updating
- Dry run mode

#### UpdateBlueprintReferences.ps1
Update Blueprint references after asset moves or renames.

```powershell
.\UpdateBlueprintReferences.ps1 -OldPath "/Game/Blueprints/BP_OldName" -NewPath "/Game/Blueprints/BP_NewName" -SearchPath "D:/Project/Content"
```

**Parameters:**
- `OldPath` - Old asset path in UE format (required)
- `NewPath` - New asset path in UE format (required)
- `SearchPath` - Directory to search for Blueprints (required)
- `DryRun` - Preview updates without executing (optional)
- `Verbose` - Enable verbose logging (optional)

**Features:**
- Finds all Blueprints with references
- Updates references automatically
- Creates backups before modification
- Supports dry run mode

#### ConvertToBlueprint.ps1
Convert C++ classes to Blueprint assistance tool.

```powershell
.\ConvertToBlueprint.ps1 -CppHeaderPath "D:/Project/Source/MyActor.h" -OutputPath "D:/Project/Content/Blueprints" -GenerateStubs
```

**Parameters:**
- `CppHeaderPath` - Path to C++ header file (required)
- `OutputPath` - Output directory for Blueprint template (required)
- `GenerateStubs` - Generate Blueprint function stubs (optional)
- `Verbose` - Enable verbose logging (optional)

**Features:**
- Parses C++ header files
- Extracts UPROPERTY declarations
- Extracts UFUNCTION declarations
- Generates Blueprint template
- Creates function stubs

### Documentation Tools

#### GenerateBlueprintDocs.ps1
Generate documentation for Blueprint files.

```powershell
.\GenerateBlueprintDocs.ps1 -BlueprintPath "D:/Project/Content/Blueprints" -OutputFormat "HTML"
```

**Parameters:**
- `BlueprintPath` - Path to Blueprint file or directory (required)
- `OutputFormat` - Format: HTML, Markdown, JSON (optional, default: HTML)
- `IncludeImages` - Include visual diagrams (optional)
- `Verbose` - Enable verbose logging (optional)

**Generates:**
- Comprehensive HTML documentation
- Markdown documentation
- JSON data export
- Component listings
- Variable tables
- Function documentation

#### ExtractBlueprintMetadata.ps1
Extract metadata from Blueprint files.

```powershell
.\ExtractBlueprintMetadata.ps1 -BlueprintPath "D:/Project/Content/Blueprints" -ExportFormat "CSV"
```

**Parameters:**
- `BlueprintPath` - Path to Blueprint file or directory (required)
- `ExportFormat` - Format: JSON, CSV, XML (optional, default: JSON)
- `Verbose` - Enable verbose logging (optional)

**Exports:**
- File information
- Blueprint metadata
- Component counts
- Variable counts
- Function counts
- Category information

#### GenerateBlueprintDiagram.ps1
Generate visual diagrams for Blueprint structure.

```powershell
.\GenerateBlueprintDiagram.ps1 -BlueprintPath "D:/Project/Content/Blueprints/BP_Actor.json" -DiagramType "Structure" -OutputFormat "Mermaid"
```

**Parameters:**
- `BlueprintPath` - Path to Blueprint file (required)
- `DiagramType` - Type: Structure, Hierarchy, Dependencies, Flow (optional)
- `OutputFormat` - Format: DOT, Mermaid, PlantUML (optional, default: Mermaid)
- `Verbose` - Enable verbose logging (optional)

**Generates:**
- Mermaid diagrams
- GraphViz DOT files
- PlantUML diagrams
- HTML preview (for Mermaid)

## Checklists

### UIWidgetChecklist.md
Comprehensive checklist for implementing UI Widget Blueprints.

**Covers:**
- Pre-implementation planning
- Widget setup
- UI elements
- Layout & styling
- Functionality
- Integration
- Testing
- Documentation

### InteractableChecklist.md
Checklist for implementing interactable object Blueprints.

**Covers:**
- Interaction types
- Component setup
- Visual/audio feedback
- State management
- Player integration
- Multiplayer considerations
- Testing & polish

### AssetMigrationChecklist.md
Checklist for migrating Blueprint assets.

**Covers:**
- Pre-migration planning
- Dependency analysis
- Source validation
- Migration execution
- Reference updates
- Post-migration validation
- Testing & rollback

### IntegrationChecklist.md
Checklist for integrating Blueprints into game systems.

**Covers:**
- System integration points
- Component integration
- Event system integration
- Data integration
- Network integration
- Testing & deployment

## Workflow Examples

### Creating a New Widget Blueprint

```powershell
# 1. Generate Widget Blueprint
.\GenerateWidgetBlueprint.ps1 -WidgetName "WBP_InventoryMenu" -OutputPath "D:/Project/Content/UI" -WidgetType "Menu" -IncludeAnimations

# 2. Follow UIWidgetChecklist.md for implementation

# 3. Validate the Blueprint
.\ValidateBlueprints.ps1 -BlueprintPath "D:/Project/Content/UI/WBP_InventoryMenu.json" -OutputReport

# 4. Generate documentation
.\GenerateBlueprintDocs.ps1 -BlueprintPath "D:/Project/Content/UI/WBP_InventoryMenu.json" -OutputFormat "HTML"
```

### Migrating Blueprints

```powershell
# 1. Validate source Blueprints
.\ValidateBlueprints.ps1 -BlueprintPath "D:/OldProject/Content/Blueprints" -Recursive -OutputReport

# 2. Check references
.\CheckBlueprintReferences.ps1 -BlueprintPath "D:/OldProject/Content/Blueprints" -CheckExternal

# 3. Dry run migration
.\MigrateBlueprintAssets.ps1 -SourcePath "D:/OldProject/Content/Blueprints" -DestinationPath "D:/NewProject/Content/Blueprints" -DryRun

# 4. Execute migration
.\MigrateBlueprintAssets.ps1 -SourcePath "D:/OldProject/Content/Blueprints" -DestinationPath "D:/NewProject/Content/Blueprints" -IncludeDependencies -UpdateReferences

# 5. Validate migrated Blueprints
.\ValidateBlueprints.ps1 -BlueprintPath "D:/NewProject/Content/Blueprints" -Recursive -OutputReport
```

### Blueprint Health Check

```powershell
# 1. Find broken Blueprints
.\FindBrokenBlueprints.ps1 -BlueprintPath "D:/Project/Content/Blueprints" -AutoFix -Quarantine

# 2. Validate all Blueprints
.\ValidateBlueprints.ps1 -BlueprintPath "D:/Project/Content/Blueprints" -Recursive -OutputReport

# 3. Check references
.\CheckBlueprintReferences.ps1 -BlueprintPath "D:/Project/Content/Blueprints" -CheckExternal -FindUnused

# 4. Analyze complexity
.\AnalyzeBlueprintComplexity.ps1 -BlueprintPath "D:/Project/Content/Blueprints" -GenerateReport -Threshold 80
```

### Converting C++ to Blueprint

```powershell
# 1. Convert C++ class
.\ConvertToBlueprint.ps1 -CppHeaderPath "D:/Project/Source/HorrorProject/MyActor.h" -OutputPath "D:/Project/Content/Blueprints" -GenerateStubs

# 2. Review generated template and stubs

# 3. Validate generated Blueprint
.\ValidateBlueprints.ps1 -BlueprintPath "D:/Project/Content/Blueprints/BP_MyActor.json" -OutputReport

# 4. Generate documentation
.\GenerateBlueprintDocs.ps1 -BlueprintPath "D:/Project/Content/Blueprints/BP_MyActor.json" -OutputFormat "HTML"
```

## Best Practices

### Naming Conventions
- Actor Blueprints: `BP_ActorName`
- Widget Blueprints: `WBP_WidgetName` or `W_WidgetName`
- Component Blueprints: `BP_ComponentName`
- Interfaces: `BPI_InterfaceName`
- Enums: `E_EnumName`
- Structs: `F_StructName`

### Organization
- Group Blueprints by type in folders
- Use consistent folder structure
- Keep related assets together
- Use descriptive names

### Validation
- Run validation regularly
- Fix issues promptly
- Keep Blueprints clean
- Remove unused elements

### Documentation
- Document complex logic
- Add function descriptions
- Use categories effectively
- Keep documentation updated

### Performance
- Minimize tick usage
- Use event-driven logic
- Optimize complex graphs
- Profile regularly

## Logs

All tools generate logs in the `Logs/` subdirectory:
- Timestamped log files
- Detailed operation logs
- Error tracking
- Report generation

## Requirements

- PowerShell 5.1 or higher
- Unreal Engine project
- Write permissions to output directories

## Troubleshooting

### Common Issues

**"File not found" errors:**
- Verify paths are correct
- Use absolute paths
- Check file permissions

**"Invalid Blueprint" errors:**
- Run ValidateBlueprints.ps1
- Check JSON syntax
- Verify required fields

**Reference update failures:**
- Use correct UE asset paths (/Game/...)
- Check for circular references
- Verify search path

**Performance issues:**
- Process smaller batches
- Use specific paths instead of recursive
- Close other applications

## Support

For issues or questions:
1. Check logs in `Logs/` directory
2. Review checklist documentation
3. Verify tool parameters
4. Check file permissions

## Version

Version: 1.0.0
Generated: 2026-04-26

## License

Internal tool for HorrorProject development.
