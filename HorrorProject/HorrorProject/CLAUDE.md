# HorrorProject (Deep Water Station) - Development Guidelines

## Project Overview
- **Engine:** Unreal Engine 5.6
- **Type:** First-person single-player horror prototype
- **Runtime Module:** `HorrorProject` (Source/HorrorProject/)
- **Editor Module:** `HorrorProjectEditor` (Source/HorrorProjectEditor/)
- **UE Root:** D:\UnrealEngine\UE_5.6
- **VS Root:** D:\vs

## Build Commands

```powershell
# Game build
$ProjectRoot = "D:\gptzuo\HorrorProject\HorrorProject"
$UERoot = "D:\UnrealEngine\UE_5.6"
& "$UERoot\Engine\Build\BatchFiles\Build.bat" HorrorProject Win64 Development -Project="$ProjectRoot\HorrorProject.uproject" -WaitMutex -NoHotReload

# Editor build
& "$UERoot\Engine\Build\BatchFiles\Build.bat" HorrorProjectEditor Win64 Development -Project="$ProjectRoot\HorrorProject.uproject" -WaitMutex -NoHotReload

# Run automation tests
& "$UERoot\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "$ProjectRoot\HorrorProject.uproject" -ExecCmds="Automation RunTests;Quit" -NullRHI -NoSplash -Unattended
```

## Source Layout
```
Source/HorrorProject/
├── AI/                  # Threat AI controller, Golem behavior
├── Accessibility/       # Color blind, subtitles, remapping
├── Achievements/        # Stats, achievements, metrics
├── Analytics/           # Telemetry, session tracking
├── Audio/               # Horror audio zones, attenuation
├── Debug/               # Console commands, HUD, visualization
├── Evidence/            # Archive subsystem, evidence collection
├── Game/                # Core: GameMode, EventBus, Objectives, Encounters, Campaign, Director, Anomaly, Narrative, ScriptedEvents
├── Interaction/         # IInteractableInterface (Door, Switch, Pickup, Document, Examinable, Recorder)
├── Localization/        # Audio/text localization
├── Performance/         # Budgets, memory, frame time, object pooling
├── Player/              # Components: Inventory, Flashlight, Camera, VHS, Fear, Sanity, EnvironmentalStory
├── Save/                # Save subsystem
├── SaveGame/            # 3-slot save, migration, validation, cloud, autosave
├── Security/            # Input validation, encryption
├── Settings/            # Graphics, audio, controls settings
├── UI/                  # UIManager, widgets, animations
└── VFX/                 # Screen effects, camera shakes
```

## Coding Standards (AAA Quality)

### Naming
- Classes: `U` prefix (UObject), `A` prefix (Actor), `F` prefix (Struct), `I` prefix (Interface), `E` prefix (Enum)
- All exported classes use `HORRORPROJECT_API` macro
- Properties: `UPROPERTY()` with `Category="Horror|SubSystem"` style
- Functions: `UFUNCTION()` with `BlueprintCallable` or `BlueprintNativeEvent` as appropriate
- Log category: `LogHorrorProject` (declared in HorrorProject.h)

### Patterns
- Subsystems over singletons: Use `UWorldSubsystem` / `UGameInstanceSubsystem`
- Event bus: `UHorrorEventBusSubsystem` for cross-system communication via GameplayTags
- Interaction: Implement `IInteractableInterface` with `CanInteract`/`Interact`
- Encounter state machine: Dormant -> Primed -> Revealed -> Resolved
- Objective state: Inactive -> Active -> Completed / Failed with prerequisite chains
- Sanity system: `USanityComponent` - environmental darkness/isolation drain, hallucinations
- Horror Director: `UHorrorDirectorSubsystem` - dynamic tension management, encounter pacing
- Anomaly events: `AHorrorAnomalyEvent` - reality fracture environmental events
- Narrative: `UNarrativeSubsystem` - journal entries, story beats, chapter progression
- Scripted events: `AHorrorScriptedEvent` - sequenced horror events with steps
- Environmental story: `UEnvironmentalStoryComponent` - ambient storytelling triggers

### Quality Requirements
- Every new system needs corresponding test files in its `Tests/` subdirectory
- Use `HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS=0` (modern UE5 automation)
- Error handling: Validate inputs at system boundaries, trust internal code
- Performance: Use object pooling for frequently spawned actors
- Memory: `TObjectPtr<>` for all UPROPERTY object references

### What NOT to Do
- Do not use deprecated `TP_FirstPerson*` classes
- Do not add dependencies without updating Build.cs
- Do not commit Saved/, Intermediate/, Binaries/, DerivedDataCache/
- Do not use static lighting (Lumen GI is configured)

## Content Structure
```
Content/
├── DeepWaterStation/    # Finale level and station assets
├── Blueprints/          # Game blueprints
├── Horror/              # Core horror assets + BP_HorrorGameModeBase
├── Characters/          # Character assets
├── SoundsOfHorror/      # Horror sound library
├── Bodycam_VHS_Effect/  # VHS post-processing
├── ForestOfSpikes/      # Campaign chapter environment
├── Scrapopolis/         # Campaign chapter environment
├── Fantastic_Dungeon_Pack/ # Dungeon and boss chapters
├── _SM13/               # Master materials
└── [EnvironmentPacks]/  # ForestOfSpikes, Scrapopolis, Fantastic_Dungeon_Pack
```

## Game Flow (DEEP WATER STATION: SIGNAL LOST)
Current 9-chapter campaign map chain:
1. Signal Calibration: `/Game/Bodycam_VHS_Effect/Maps/LVL_Showcase_01`
2. Forest of Spikes: `/Game/ForestOfSpikes/Levels/Level_ForestOfSpikes_Demo_Night`
3. Scrapopolis: `/Game/Scrapopolis/Levels/Level_Scrapopolis_Demo`
4. Dungeon Entrance: `/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_2_entrance`
5. Dungeon Depths: `/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_1_dungeon`
6. Dungeon Hall: `/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_3_hall`
7. Dungeon Temple: `/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_4_temple`
8. Stone Golem Boss: `/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_5_bossroom`
9. Deep Water Station Finale: `/Game/DeepWaterStation/Maps/DemoMap_VerticalSlice_Day1`

Key systems:
- **Horror Director** (`UHorrorDirectorSubsystem`): Dynamic tension pacing, encounter triggering
- **Sanity** (`USanityComponent`): Darkness/isolation drain, hallucinations
- **Anomaly Events** (`AHorrorAnomalyEvent`): Reality fractures, environmental disturbances
- **Narrative** (`UNarrativeSubsystem`): Journal entries, story beats, chapter progression
- **Scripted Events** (`AHorrorScriptedEvent`): Sequenced horror moments

## Quality Scripts
```powershell
# Full quality pipeline
powershell -File Scripts/Quality/GenerateQualityReport.ps1

# Code style check
powershell -File Scripts/Quality/CodeStyleCheck.ps1

# Dependency analysis
powershell -File Scripts/Quality/DependencyAnalysis.ps1
```
