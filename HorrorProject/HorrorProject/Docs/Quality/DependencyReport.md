# Dependency Analysis Report
Generated: 2026-04-26 22:54:45

## Summary
- Files Analyzed: 186
- Average Coupling: 4.5 dependencies per file
- High Coupling Files: 2 (>15 dependencies)
- Circular Dependencies: 0
- Issues Found: 2

## Module Coupling Analysis

| Module | Files | Internal Deps | External Deps | Coupling Ratio |
|--------|-------|---------------|---------------|----------------|| AI | 7 | 8 | 9 | 52.9% |
| Audio | 13 | 11 | 29 | 72.5% |
| Evidence | 17 | 16 | 29 | 64.4% |
| Game | 32 | 57 | 79 | 58.1% |
| HorrorProject.cpp | 1 | 0 | 1 | 100% |
| HorrorProject.h | 1 | 0 | 0 | 0% |
| HorrorProjectCameraManager.cpp | 1 | 0 | 0 | 0% |
| HorrorProjectCameraManager.h | 1 | 0 | 1 | 100% |
| HorrorProjectCharacter.cpp | 1 | 0 | 5 | 100% |
| HorrorProjectCharacter.h | 1 | 0 | 2 | 100% |
| HorrorProjectGameMode.cpp | 1 | 0 | 0 | 0% |
| HorrorProjectGameMode.h | 1 | 0 | 1 | 100% |
| HorrorProjectPlayerController.cpp | 1 | 0 | 3 | 100% |
| HorrorProjectPlayerController.h | 1 | 0 | 1 | 100% |
| Interaction | 13 | 11 | 35 | 76.1% |
| Performance | 8 | 4 | 21 | 84% |
| Player | 35 | 32 | 93 | 74.4% |
| Save | 5 | 5 | 25 | 83.3% |
| Tests | 3 | 1 | 7 | 87.5% |
| UI | 20 | 0 | 20 | 100% |
| Variant_Horror | 4 | 1 | 18 | 94.7% |
| VFX | 7 | 3 | 13 | 81.2% |

## Module Dependencies
### AI depends on:
- GameFramework: 4 references
- Misc: 1 references
- Kismet: 1 references
- Components: 1 references
- Engine: 1 references
- Tests: 1 references

### Audio depends on:
- Components: 7 references
- Engine: 5 references
- Sound: 5 references
- Kismet: 4 references
- GameFramework: 3 references
- Misc: 1 references
- PhysicalMaterials: 1 references
- Subsystems: 1 references
- Game: 1 references
- Tests: 1 references

### Evidence depends on:
- Engine: 7 references
- Player: 4 references
- GameFramework: 4 references
- Components: 3 references
- Kismet: 3 references
- Tests: 2 references
- Misc: 2 references
- Game: 1 references
- Subsystems: 1 references
- Interaction: 1 references
- Blueprint: 1 references

### Game depends on:
- Player: 16 references
- Engine: 13 references
- GameFramework: 11 references
- Misc: 8 references
- AI: 7 references
- Components: 7 references
- Tests: 5 references
- Interaction: 3 references
- Subsystems: 2 references
- Kismet: 2 references
- Containers: 1 references
- Sound: 1 references
- Save: 1 references
- Audio: 1 references
- Camera: 1 references

### HorrorProject.cpp depends on:
- Modules: 1 references

### HorrorProjectCameraManager.h depends on:
- Camera: 1 references

### HorrorProjectCharacter.cpp depends on:
- Components: 2 references
- Animation: 1 references
- GameFramework: 1 references
- Camera: 1 references

### HorrorProjectCharacter.h depends on:
- GameFramework: 1 references
- Logging: 1 references

### HorrorProjectGameMode.h depends on:
- GameFramework: 1 references

### HorrorProjectPlayerController.cpp depends on:
- Widgets: 1 references
- Blueprint: 1 references
- Engine: 1 references

### HorrorProjectPlayerController.h depends on:
- GameFramework: 1 references

### Interaction depends on:
- GameFramework: 8 references
- Components: 8 references
- Player: 5 references
- Kismet: 5 references
- Sound: 5 references
- Game: 1 references
- Camera: 1 references
- Curves: 1 references
- UObject: 1 references

### Performance depends on:
- Engine: 7 references
- Misc: 6 references
- HAL: 4 references
- Subsystems: 3 references
- UObject: 1 references

### Player depends on:
- Components: 19 references
- GameFramework: 15 references
- Misc: 11 references
- Engine: 9 references
- Game: 7 references
- Materials: 5 references
- Camera: 5 references
- UObject: 4 references
- Variant_Horror: 3 references
- Kismet: 3 references
- Audio: 3 references
- Interaction: 2 references
- Tests: 2 references
- Sound: 2 references
- Templates: 1 references
- Widgets: 1 references
- Perception: 1 references

### Save depends on:
- Player: 9 references
- Game: 4 references
- GameFramework: 4 references
- Engine: 3 references
- Kismet: 2 references
- Subsystems: 1 references
- Tests: 1 references
- Misc: 1 references

### Tests depends on:
- Player: 3 references
- Misc: 2 references
- GameFramework: 1 references
- Engine: 1 references

### UI depends on:
- Blueprint: 7 references
- Player: 4 references
- Components: 4 references
- Evidence: 2 references
- GameFramework: 1 references
- Kismet: 1 references
- UObject: 1 references

### Variant_Horror depends on:
- Player: 7 references
- Game: 4 references
- Engine: 2 references
- GameFramework: 1 references
- Camera: 1 references
- Misc: 1 references
- Blueprint: 1 references
- Components: 1 references

### VFX depends on:
- Camera: 3 references
- Components: 3 references
- Kismet: 2 references
- GameFramework: 1 references
- Misc: 1 references
- Engine: 1 references
- Materials: 1 references
- Tests: 1 references


## Circular Dependencies
No circular dependencies detected.

## High Coupling Files (Top 20)
Files with excessive dependencies:

| File | Dependencies | Risk |
|------|--------------|------|| HorrorSaveSubsystemTests.cpp | 17 | Medium |
| HorrorPlayerCharacter.cpp | 17 | Medium |
| FoundFootageObjectiveInteractableTests.cpp | 15 | Low |
| HorrorEditorUI.cpp | 14 | Low |
| InteractionComponentTests.cpp | 14 | Low |
| HorrorGameModeBase.cpp | 12 | Low |
| HorrorEncounterDirector.cpp | 12 | Low |
| DeepWaterStationRouteKitTests.cpp | 11 | Low |
| CameraPhotoComponent.cpp | 11 | Low |
| HorrorEditorModule.cpp | 10 | Low |
| HorrorSaveSubsystem.cpp | 10 | Low |
| FoundFootageObjectiveInteractable.cpp | 10 | Low |
| HorrorGameModeBaseObjectiveTests.cpp | 10 | Low |
| HorrorUI.cpp | 10 | Low |
| HorrorObjectiveNodeTests.cpp | 9 | Low |
| HorrorProjectCharacter.cpp | 9 | Low |
| HorrorPlayerController.cpp | 9 | Low |
| ExaminableInteractable.cpp | 9 | Low |
| DocumentInteractable.cpp | 9 | Low |
| InteractionComponent.cpp | 9 | Low |

## Issues by Category
### Coupling (2 issues)

- **Warning** [HorrorPlayerCharacter.cpp] Excessive dependencies (17 includes)
- **Warning** [HorrorSaveSubsystemTests.cpp] Excessive dependencies (17 includes)


## Recommendations

### Critical (Immediate Action Required)
1. **Resolve circular dependencies** - Break cycles by introducing interfaces or dependency inversion
2. **Fix .cpp includes** - Never include implementation files

### High Priority
1. **Reduce high coupling files** - Files with >15 dependencies should be refactored
2. **Improve module boundaries** - Modules with >50% external coupling need better encapsulation
3. **Remove platform-specific dependencies** - Use abstraction layers

### Medium Priority
1. **Balance module dependencies** - Avoid one-way dependencies where possible
2. **Extract common interfaces** - Reduce direct dependencies between modules
3. **Apply dependency inversion** - High-level modules should not depend on low-level modules

### Best Practices
1. **Keep coupling low** - Target <10 dependencies per file
2. **Favor composition over inheritance** - Reduces tight coupling
3. **Use interfaces** - Define clear contracts between modules
4. **Apply SOLID principles** - Especially Dependency Inversion Principle
5. **Regular refactoring** - Continuously improve module boundaries

## Coupling Guidelines
- **0-5**: Excellent, loosely coupled
- **6-10**: Good, acceptable coupling
- **11-15**: Moderate, monitor for growth
- **16-20**: High, refactoring recommended
- **20+**: Very high, refactoring required
