# Blueprint Tools - Quick Start Guide

## Installation

1. Navigate to the Blueprint tools directory:
```powershell
cd D:/gptzuo/HorrorProject/HorrorProject/Tools/Blueprint
```

2. Verify PowerShell execution policy allows scripts:
```powershell
Get-ExecutionPolicy
# If Restricted, run: Set-ExecutionPolicy RemoteSigned -Scope CurrentUser
```

## Quick Examples

### Generate a New Actor Blueprint
```powershell
.\GenerateActorBlueprint.ps1 -ActorName "BP_Flashlight" -ActorType "Pickup" -OutputPath "D:/gptzuo/HorrorProject/HorrorProject/Content/Blueprints/Items"
```

### Generate a UI Widget
```powershell
.\GenerateWidgetBlueprint.ps1 -WidgetName "WBP_PauseMenu" -WidgetType "Menu" -OutputPath "D:/gptzuo/HorrorProject/HorrorProject/Content/UI" -IncludeAnimations
```

### Validate All Blueprints
```powershell
.\ValidateBlueprints.ps1 -BlueprintPath "D:/gptzuo/HorrorProject/HorrorProject/Content/Blueprints" -Recursive -OutputReport
```

### Check Blueprint Complexity
```powershell
.\AnalyzeBlueprintComplexity.ps1 -BlueprintPath "D:/gptzuo/HorrorProject/HorrorProject/Content/Blueprints" -GenerateReport -Threshold 80
```

### Find and Fix Broken Blueprints
```powershell
.\FindBrokenBlueprints.ps1 -BlueprintPath "D:/gptzuo/HorrorProject/HorrorProject/Content/Blueprints" -AutoFix -Quarantine
```

### Generate Documentation
```powershell
.\GenerateBlueprintDocs.ps1 -BlueprintPath "D:/gptzuo/HorrorProject/HorrorProject/Content/Blueprints" -OutputFormat "HTML"
```

## Common Workflows

### Daily Development
1. Generate new Blueprints as needed
2. Follow implementation checklists
3. Validate before committing
4. Document complex Blueprints

### Weekly Maintenance
1. Run validation on all Blueprints
2. Check for broken references
3. Analyze complexity metrics
4. Update documentation

### Before Major Changes
1. Backup current state
2. Validate all Blueprints
3. Check dependencies
4. Document current state

### Asset Migration
1. Follow AssetMigrationChecklist.md
2. Use dry run mode first
3. Validate before and after
4. Update all references

## Tool Categories

**Generation (4 tools):**
- GenerateBlueprintTemplate.ps1
- GenerateWidgetBlueprint.ps1
- GenerateActorBlueprint.ps1
- GenerateComponentBlueprint.ps1

**Validation (4 tools):**
- ValidateBlueprints.ps1
- CheckBlueprintReferences.ps1
- FindBrokenBlueprints.ps1
- AnalyzeBlueprintComplexity.ps1

**Migration (3 tools):**
- MigrateBlueprintAssets.ps1
- UpdateBlueprintReferences.ps1
- ConvertToBlueprint.ps1

**Documentation (3 tools):**
- GenerateBlueprintDocs.ps1
- ExtractBlueprintMetadata.ps1
- GenerateBlueprintDiagram.ps1

**Checklists (4 files):**
- UIWidgetChecklist.md
- InteractableChecklist.md
- AssetMigrationChecklist.md
- IntegrationChecklist.md

## Tips

- Always use `-Verbose` flag when troubleshooting
- Use `-DryRun` for migration/update operations first
- Check `Logs/` directory for detailed output
- Keep backups before major operations
- Run validation regularly
- Follow naming conventions
- Document as you go

## Getting Help

Each script supports `-?` for help:
```powershell
Get-Help .\ValidateBlueprints.ps1 -Full
```

View examples:
```powershell
Get-Help .\ValidateBlueprints.ps1 -Examples
```

## Next Steps

1. Review README.md for detailed documentation
2. Check implementation checklists
3. Run validation on existing Blueprints
4. Generate documentation for current Blueprints
5. Integrate tools into your workflow

## Output Locations

- **Logs:** `Tools/Blueprint/Logs/`
- **Reports:** `Tools/Blueprint/Logs/`
- **Generated Files:** Specified output paths
- **Backups:** Same directory as original with `.backup_*` suffix
- **Quarantine:** `Tools/Blueprint/Logs/Quarantine/`

## Support

Check logs first, then review documentation. All tools provide detailed error messages and stack traces.
