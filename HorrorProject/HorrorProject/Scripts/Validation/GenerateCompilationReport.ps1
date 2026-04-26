# Generate comprehensive compilation report
# Analyzes project structure and generates detailed report

$ErrorActionPreference = "Stop"
$ProjectRoot = "D:\gptzuo\HorrorProject\HorrorProject"
$ReportDir = Join-Path $ProjectRoot "Docs\Compilation"

Write-Host "=== Generating Compilation Report ===" -ForegroundColor Cyan

# Create report directory
if (-not (Test-Path $ReportDir)) {
    New-Item -ItemType Directory -Path $ReportDir -Force | Out-Null
}

# Count files by system
$VFXFiles = @(Get-ChildItem -Path "$ProjectRoot\Source\HorrorProject\VFX" -Filter "*.cpp" -Recurse)
$AudioFiles = @(Get-ChildItem -Path "$ProjectRoot\Source\HorrorProject\Audio" -Filter "*.cpp" -Recurse)
$UIFiles = @(Get-ChildItem -Path "$ProjectRoot\Source\HorrorProject\UI" -Filter "*.cpp" -Recurse)
$TestFiles = @(Get-ChildItem -Path "$ProjectRoot\Source\HorrorProject" -Filter "*Tests.cpp" -Recurse)
$EditorFiles = @(Get-ChildItem -Path "$ProjectRoot\Source\HorrorProjectEditor\Tools" -Filter "*.cpp" -Recurse)

# Generate main report
$ReportFile = Join-Path $ReportDir "CompilationReport.md"
$Report = @"
# Compilation Report
Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')

## Project Overview
- **Project**: HorrorProject
- **Engine**: Unreal Engine 5.6
- **Platform**: Win64

## File Statistics

### VFX System
- Implementation Files: $($VFXFiles.Count)
- Components:
  - ParticleSpawner (Niagara-based)
  - PostProcessController
  - ScreenEffectManager

### Audio System
- Implementation Files: $($AudioFiles.Count)
- Components:
  - AmbientAudioComponent
  - BreathingAudioComponent
  - FootstepAudioComponent
  - UnderwaterAudioComponent
- Subsystem: HorrorAudioSubsystem
- Tools: AudioAssetAuditor, AudioAttenuationPresets

### UI System
- Implementation Files: $($UIFiles.Count)
- Widgets:
  - ArchiveMenuWidget
  - BodycamOverlayWidget
  - EvidenceToastWidget
  - ObjectiveToastWidget
  - PauseMenuWidget
  - SettingsMenuWidget
- Components:
  - NoiseOverlayComponent
  - ScanlineComponent
  - VHSEffectComponent
- Subsystem: UIManagerSubsystem

### Testing System
- Test Files: $($TestFiles.Count)
- Coverage: AI, Audio, Evidence, Game, Interaction, Player, Save, UI, VFX

### Editor Tools
- Tool Files: $($EditorFiles.Count)
- Tools:
  - AssetValidator
  - AudioBatchProcessor
  - LODGenerator
  - MaterialOptimizer

## Module Dependencies

### HorrorProject Module
Required modules:
- Core, CoreUObject, Engine
- InputCore, EnhancedInput
- GameplayTags
- AIModule, StateTreeModule, GameplayStateTreeModule
- UMG, SlateCore
- **Niagara** (for VFX system)

### HorrorProjectEditor Module
Required modules:
- Core, CoreUObject, Engine
- UnrealEd, EditorSubsystem
- AssetTools, ContentBrowser
- Slate, SlateCore, EditorStyle
- ToolMenus, PropertyEditor
- AudioEditor, MaterialEditor

## Known Issues

### Missing Module Dependencies
1. **Niagara module** not in HorrorProject.Build.cs
   - Required by: VFX/ParticleSpawner.cpp
   - Fix: Add "Niagara" to PublicDependencyModuleNames

### Potential Issues
- Audio system may benefit from AudioMixer module for advanced features
- Test files may need additional test framework modules

## Compilation Status
Status will be updated after running ValidateCompilation.ps1

## Next Steps
1. Add missing Niagara module to Build.cs
2. Run full compilation test
3. Verify all tests compile
4. Check for warnings
"@

Set-Content -Path $ReportFile -Value $Report -Encoding UTF8
Write-Host "[OK] Generated: $ReportFile" -ForegroundColor Green

# Generate dependency graph
$DependencyFile = Join-Path $ReportDir "DependencyGraph.md"
$DependencyGraph = @"
# Module Dependency Graph

## HorrorProject Module Dependencies

\`\`\`
HorrorProject
├── Core (Engine)
├── CoreUObject (Engine)
├── Engine (Engine)
├── InputCore (Engine)
├── EnhancedInput (Engine)
├── GameplayTags (Engine)
├── AIModule (Engine)
├── StateTreeModule (Engine)
├── GameplayStateTreeModule (Engine)
├── UMG (Engine)
├── SlateCore (Engine)
└── Niagara (Engine) [MISSING]
\`\`\`

## HorrorProjectEditor Module Dependencies

\`\`\`
HorrorProjectEditor
├── Core (Engine)
├── CoreUObject (Engine)
├── Engine (Engine)
├── UnrealEd (Editor)
├── EditorSubsystem (Editor)
├── AssetTools (Editor)
├── ContentBrowser (Editor)
├── Slate (Engine)
├── SlateCore (Engine)
├── EditorStyle (Editor)
├── ToolMenus (Editor)
├── PropertyEditor (Editor)
├── AudioEditor (Editor)
├── MaterialEditor (Editor)
├── InputCore (Engine)
├── LevelEditor (Editor)
├── Projects (Engine)
├── RenderCore (Engine)
├── RHI (Engine)
└── HorrorProject (Game)
\`\`\`

## System Dependencies

### VFX System
- Niagara (for particle effects)
- Engine (for post-process volumes)
- Camera (for camera shakes)

### Audio System
- Engine (for audio components)
- GameplayTags (for event system)
- Sound (for sound assets)

### UI System
- UMG (for widgets)
- SlateCore (for UI framework)
- Engine (for player controller)

### Editor Tools
- AssetTools (for asset operations)
- ContentBrowser (for asset browsing)
- EditorSubsystem (for editor integration)
"@

Set-Content -Path $DependencyFile -Value $DependencyGraph -Encoding UTF8
Write-Host "[OK] Generated: $DependencyFile" -ForegroundColor Green

# Generate issues file
$IssuesFile = Join-Path $ReportDir "CompilationIssues.md"
$Issues = @"
# Compilation Issues

## Critical Issues

### 1. Missing Niagara Module
**Severity**: Critical
**File**: Source/HorrorProject/HorrorProject.Build.cs
**Description**: ParticleSpawner.cpp uses Niagara types but module not declared
**Fix**:
\`\`\`csharp
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
    "Niagara"  // ADD THIS LINE
});
\`\`\`

## Warnings

### 1. Optional Audio Modules
**Severity**: Low
**Description**: AudioMixer and AudioExtensions modules could enhance audio system
**Recommendation**: Consider adding for advanced audio features

## Resolved Issues
None yet - first validation run

## Testing Issues
All test files need to be verified for compilation
"@

Set-Content -Path $IssuesFile -Value $Issues -Encoding UTF8
Write-Host "[OK] Generated: $IssuesFile" -ForegroundColor Green

Write-Host "`n=== Report Generation Complete ===" -ForegroundColor Cyan
Write-Host "Reports saved to: $ReportDir" -ForegroundColor Green
Write-Host "  - CompilationReport.md" -ForegroundColor Gray
Write-Host "  - DependencyGraph.md" -ForegroundColor Gray
Write-Host "  - CompilationIssues.md" -ForegroundColor Gray
