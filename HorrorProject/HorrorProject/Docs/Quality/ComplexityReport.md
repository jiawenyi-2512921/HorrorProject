# Complexity Analysis Report
Generated: 2026-04-26 22:54:45

## Summary
- Files Analyzed: 164
- Functions Analyzed: 762
- Average Complexity: 2
- Average Function Length: 10.7 lines
- Average Maintainability: 89.5/100
- Overall Comment Ratio: 3.1%

## Risk Distribution
- High Risk (Complexity > 20): 0 functions
- Medium Risk (Complexity 10-20): 5 functions
- Low Risk (Complexity < 10): 757 functions

## High Risk Functions (Top 20)
These functions require immediate refactoring:

| Function | File | Lines | Complexity | Maintainability | Risk |
|----------|------|-------|------------|-----------------|------|| switch | FoundFootageObjectiveInteractable.cpp | 49 | 15 | 60 | Medium |
| switch | FoundFootageObjectiveInteractable.cpp | 50 | 13 | 64 | Medium |
| for | AssetValidator.cpp | 36 | 13 | 65 | Medium |
| switch | FoundFootageObjectiveInteractable.cpp | 71 | 13 | 62 | Medium |
| switch | HorrorGolemBehaviorComponent.cpp | 57 | 12 | 65 | Medium |
| switch | PostProcessController.cpp | 38 | 8 | 75 | Low |
| TEXT | HorrorGameModeBase.cpp | 61 | 8 | 73 | Low |
| switch | HorrorGolemBehaviorComponent.cpp | 21 | 7 | 79 | Low |
| switch | EvidenceBlueprintLibrary.cpp | 10 | 7 | 80 | Low |
| switch | EvidenceBlueprintLibrary.cpp | 10 | 7 | 80 | Low |
| AHorrorPlayerCharacter::AHorrorPlayerCharacter | HorrorPlayerCharacter.cpp | 49 | 7 | 76 | Low |
| switch | NoiseGeneratorComponent.cpp | 21 | 7 | 79 | Low |
| switch | UIAnimationLibrary.cpp | 17 | 7 | 79 | Low |
| for | CameraPhotoComponent.cpp | 16 | 6 | 81 | Low |
| if | DoorInteractable.cpp | 34 | 6 | 80 | Low |
| switch | FoundFootageObjectiveInteractable.cpp | 17 | 5 | 83 | Low |
| if | ExaminableInteractable.cpp | 13 | 5 | 84 | Low |
| switch | FoundFootageObjectiveInteractable.cpp | 17 | 5 | 83 | Low |
| if | HorrorAudioSubsystem.cpp | 11 | 5 | 84 | Low |
| if | PerformanceBudget.cpp | 18 | 5 | 83 | Low |

## Longest Functions (Top 20)
Functions exceeding 50 lines should be refactored:

| Function | File | Lines | Complexity | Maintainability |
|----------|------|-------|------------|-----------------|| SNew | HorrorEditorUI.cpp | 274 | 1 | 66 |
| UCLASS | HorrorAudioSubsystem.h | 131 | 1 | 80 |
| UCLASS | HorrorUI.h | 129 | 1 | 80 |
| UCLASS | HorrorGameModeBase.h | 129 | 1 | 80 |
| UCLASS | HorrorPlayerCharacter.h | 119 | 1 | 81 |
| UCLASS | RecorderInteractable.h | 90 | 1 | 79 |
| UCLASS | HorrorCharacter.h | 90 | 1 | 69 |
| UCLASS | ExaminableInteractable.h | 90 | 1 | 79 |
| UCLASS | DoorInteractable.h | 88 | 1 | 79 |
| UCLASS | MemoryTracker.h | 84 | 1 | 85 |
| UCLASS | DocumentInteractable.h | 81 | 1 | 80 |
| UCLASS | BodycamOverlayWidget.h | 75 | 1 | 76 |
| UCLASS | HorrorProjectCharacter.h | 73 | 1 | 71 |
| switch | FoundFootageObjectiveInteractable.cpp | 71 | 13 | 62 |
| UCLASS | ArchiveViewerWidget.h | 69 | 1 | 76 |
| UCLASS | PerformanceBudget.h | 69 | 1 | 86 |
| UCLASS | BaseInteractable.h | 67 | 1 | 76 |
| UCLASS | PickupInteractable.h | 66 | 1 | 81 |
| UCLASS | FrameTimeTracker.h | 63 | 1 | 87 |
| TEXT | HorrorGameModeBase.cpp | 61 | 8 | 73 |

## File Metrics
Overview of code distribution:

| File | Lines | Code | Comments | Ratio | Functions |
|------|-------|------|----------|-------|-----------|| HorrorGameModeBase.cpp | 538 | 463 | 1 | 0.2% | 81 |
| HorrorAudioSubsystem.cpp | 499 | 422 | 1 | 0.2% | 55 |
| HorrorGolemBehaviorComponent.cpp | 431 | 341 | 18 | 5.3% | 76 |
| FoundFootageObjectiveInteractable.cpp | 388 | 335 | 1 | 0.3% | 47 |
| CameraPhotoComponent.cpp | 395 | 324 | 1 | 0.3% | 61 |
| DeepWaterStationRouteKit.cpp | 366 | 317 | 1 | 0.3% | 59 |
| InteractionComponent.cpp | 374 | 310 | 1 | 0.3% | 61 |
| InteractionComponentTests.cpp | 341 | 290 | 1 | 0.3% | 77 |
| HorrorPlayerCharacter.cpp | 339 | 286 | 1 | 0.3% | 59 |
| HorrorEncounterDirector.cpp | 334 | 274 | 1 | 0.4% | 50 |
| HorrorEditorUI.cpp | 318 | 267 | 7 | 2.6% | 35 |
| QuantumCameraComponent.cpp | 316 | 259 | 1 | 0.4% | 93 |
| AmbientAudioComponent.cpp | 287 | 245 | 1 | 0.4% | 46 |
| VHSEffectComponent.cpp | 290 | 238 | 1 | 0.4% | 70 |
| CameraRecordingComponent.cpp | 293 | 235 | 1 | 0.4% | 50 |
| HorrorFoundFootageContract.cpp | 269 | 230 | 1 | 0.4% | 52 |
| HorrorUI.cpp | 270 | 221 | 1 | 0.5% | 55 |
| EvidenceCollectionComponent.cpp | 276 | 221 | 3 | 1.4% | 33 |
| PostProcessController.cpp | 269 | 208 | 15 | 7.2% | 44 |
| HorrorSaveSubsystem.cpp | 247 | 208 | 1 | 0.5% | 26 |
| FootstepAudioComponent.cpp | 236 | 202 | 1 | 0.5% | 24 |
| DoorInteractable.cpp | 243 | 195 | 8 | 4.1% | 35 |
| ArchiveSubsystem.cpp | 226 | 187 | 3 | 1.6% | 26 |
| ParticleSpawner.cpp | 224 | 185 | 5 | 2.7% | 31 |
| CameraRecordingComponentTests.cpp | 244 | 180 | 1 | 0.6% | 31 |
| MemoryTracker.cpp | 232 | 179 | 7 | 3.9% | 31 |
| HorrorAudioSubsystem.h | 253 | 179 | 1 | 0.6% | 67 |
| ExaminableInteractable.cpp | 228 | 171 | 19 | 11.1% | 34 |
| NoteRecorderComponent.cpp | 208 | 169 | 1 | 0.6% | 45 |
| InventoryComponent.cpp | 208 | 169 | 1 | 0.6% | 45 |

## Recommendations

### Immediate Actions (High Priority)
1. Refactor functions with complexity > 20
2. Break down functions exceeding 100 lines
3. Add documentation to functions with maintainability < 40

### Medium Priority
1. Improve comment ratio in files below 10%
2. Reduce parameter count in functions with > 5 parameters
3. Extract complex conditional logic into helper functions

### Long Term
1. Maintain average complexity below 10
2. Keep functions under 50 lines
3. Target maintainability index above 70
4. Maintain comment ratio above 20%

## Complexity Guidelines
- **1-10**: Simple, easy to maintain
- **11-20**: Moderate complexity, acceptable
- **21-50**: High complexity, refactor recommended
- **50+**: Very high complexity, refactor required

## Maintainability Index
- **85-100**: Excellent maintainability
- **65-84**: Good maintainability
- **40-64**: Moderate maintainability
- **0-39**: Difficult to maintain
