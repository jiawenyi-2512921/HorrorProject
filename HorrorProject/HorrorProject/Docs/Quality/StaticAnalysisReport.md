# Static Analysis Report
Generated: 2026-04-26 22:54:40

## Summary
- Files Analyzed: 164
- Errors: 0
- Warnings: 375
- Info: 0
- Total Issues: 375

## Issues by Category
### Maintainability (123 issues)
| Severity | File | Line | Issue | Recommendation |
|----------|------|------|-------|----------------|
| Warning | HorrorGolemBehaviorComponent.cpp | 410 | Magic number: 200 | Use named constant |
| Warning | HorrorGolemBehaviorComponent.cpp | 421 | Magic number: 250 | Use named constant |
| Warning | HorrorGolemBehaviorComponent.cpp | 425 | Magic number: 16 | Use named constant |
| Warning | HorrorGolemBehaviorComponent.cpp | 426 | Magic number: 16 | Use named constant |
| Warning | HorrorGolemBehaviorComponent.cpp | 427 | Magic number: 16 | Use named constant |
| Warning | HorrorGolemBehaviorComponent.cpp | 428 | Magic number: 16 | Use named constant |
| Warning | HorrorGolemBehaviorComponent.cpp | 429 | Magic number: 16 | Use named constant |
| Warning | HorrorGolemBehaviorComponent.h | 18 | Magic number: 70 | Use named constant |
| Warning | HorrorGolemBehaviorComponent.h | 61 | Magic number: 3000 | Use named constant |
| Warning | HorrorGolemBehaviorComponent.h | 64 | Magic number: 50 | Use named constant |
| Warning | HorrorGolemBehaviorComponent.h | 74 | Magic number: 1500 | Use named constant |
| Warning | HorrorGolemBehaviorComponent.h | 77 | Magic number: 150 | Use named constant |
| Warning | HorrorGolemBehaviorComponent.h | 84 | Magic number: 2000 | Use named constant |
| Warning | HorrorGolemBehaviorComponent.h | 90 | Magic number: 400 | Use named constant |
| Warning | HorrorGolemBehaviorComponent.h | 97 | Magic number: 2500 | Use named constant |
| Warning | HorrorGolemBehaviorComponent.h | 100 | Magic number: 600 | Use named constant |
| Warning | HorrorGolemBehaviorComponent.h | 107 | Magic number: 500 | Use named constant |
| Warning | HorrorThreatCharacter.h | 63 | Magic number: 800 | Use named constant |
| Warning | AmbientAudioComponent.cpp | 276 | Magic number: 3600 | Use named constant |
| Warning | AmbientAudioComponent.cpp | 277 | Magic number: 24 | Use named constant |
| ... | ... | ... | and 103 more | ... |

### ThreadSafety (85 issues)
| Severity | File | Line | Issue | Recommendation |
|----------|------|------|-------|----------------|
| Warning | AmbientAudioComponent.h | 12 | Non-const static variable | May cause race conditions |
| Warning | HorrorAudioLibrary.h | 17 | Non-const static variable | May cause race conditions |
| Warning | HorrorAudioLibrary.h | 20 | Non-const static variable | May cause race conditions |
| Warning | HorrorAudioLibrary.h | 23 | Non-const static variable | May cause race conditions |
| Warning | HorrorAudioLibrary.h | 26 | Non-const static variable | May cause race conditions |
| Warning | HorrorAudioLibrary.h | 29 | Non-const static variable | May cause race conditions |
| Warning | HorrorAudioLibrary.h | 32 | Non-const static variable | May cause race conditions |
| Warning | HorrorAudioLibrary.h | 35 | Non-const static variable | May cause race conditions |
| Warning | HorrorAudioLibrary.h | 38 | Non-const static variable | May cause race conditions |
| Warning | HorrorAudioLibrary.h | 41 | Non-const static variable | May cause race conditions |
| Warning | HorrorAudioLibrary.h | 44 | Non-const static variable | May cause race conditions |
| Warning | HorrorAudioLibrary.h | 47 | Non-const static variable | May cause race conditions |
| Warning | ArchiveSubsystem.cpp | 98 | Container modification without lock | Use FScopeLock if accessed from multiple threads |
| Warning | ArchiveSubsystem.cpp | 98 | Container modification without lock | Use FScopeLock if accessed from multiple threads |
| Warning | ArchiveSubsystem.cpp | 118 | Container modification without lock | Use FScopeLock if accessed from multiple threads |
| Warning | ArchiveSubsystem.cpp | 133 | Container modification without lock | Use FScopeLock if accessed from multiple threads |
| Warning | EvidenceBlueprintLibrary.h | 20 | Non-const static variable | May cause race conditions |
| Warning | EvidenceBlueprintLibrary.h | 23 | Non-const static variable | May cause race conditions |
| Warning | EvidenceBlueprintLibrary.h | 26 | Non-const static variable | May cause race conditions |
| Warning | EvidenceBlueprintLibrary.h | 36 | Non-const static variable | May cause race conditions |
| ... | ... | ... | and 65 more | ... |

### Performance (82 issues)
| Severity | File | Line | Issue | Recommendation |
|----------|------|------|-------|----------------|
| Warning | HorrorGolemBehaviorComponent.h | 123 | Pass 'float' by const reference | Avoid unnecessary copies |
| Warning | HorrorGolemBehaviorComponent.h | 126 | Pass 'float' by const reference | Avoid unnecessary copies |
| Warning | HorrorGolemBehaviorComponent.h | 153 | Pass 'float' by const reference | Avoid unnecessary copies |
| Warning | HorrorGolemBehaviorComponent.h | 156 | Pass 'float' by const reference | Avoid unnecessary copies |
| Warning | HorrorGolemBehaviorComponent.h | 157 | Pass 'float' by const reference | Avoid unnecessary copies |
| Warning | HorrorGolemBehaviorComponent.h | 158 | Pass 'float' by const reference | Avoid unnecessary copies |
| Warning | HorrorGolemBehaviorComponent.h | 159 | Pass 'float' by const reference | Avoid unnecessary copies |
| Warning | HorrorGolemBehaviorComponent.h | 160 | Pass 'float' by const reference | Avoid unnecessary copies |
| Warning | HorrorGolemBehaviorComponent.h | 163 | Pass 'float' by const reference | Avoid unnecessary copies |
| Warning | AmbientAudioComponent.h | 69 | Pass 'float' by const reference | Avoid unnecessary copies |
| Warning | AmbientAudioComponent.h | 104 | Pass 'float' by const reference | Avoid unnecessary copies |
| Warning | AmbientAudioComponent.h | 105 | Pass 'float' by const reference | Avoid unnecessary copies |
| Warning | AmbientAudioComponent.h | 106 | Pass 'float' by const reference | Avoid unnecessary copies |
| Warning | FootstepAudioComponent.h | 67 | Pass 'float' by const reference | Avoid unnecessary copies |
| Warning | FootstepAudioComponent.h | 70 | Pass 'float' by const reference | Avoid unnecessary copies |
| Warning | HorrorAudioSubsystem.h | 168 | Pass 'FGameplayTag' by const reference | Avoid unnecessary copies |
| Warning | HorrorAudioSubsystem.h | 171 | Pass 'FName' by const reference | Avoid unnecessary copies |
| Warning | HorrorAudioSubsystem.h | 183 | Pass 'float' by const reference | Avoid unnecessary copies |
| Warning | HorrorAudioSubsystem.h | 251 | Pass 'float' by const reference | Avoid unnecessary copies |
| Warning | ArchiveViewerWidget.h | 30 | Pass 'FName' by const reference | Avoid unnecessary copies |
| ... | ... | ... | and 62 more | ... |

### ErrorHandling (49 issues)
| Severity | File | Line | Issue | Recommendation |
|----------|------|------|-------|----------------|
| Warning | HorrorGolemBehaviorComponent.cpp | 20 | Cast<> without null check | Use if (AActor* Actor = Cast<>()) or ensure() |
| Warning | HorrorGolemBehaviorComponent.cpp | 420 | Cast<> without null check | Use if (AActor* Actor = Cast<>()) or ensure() |
| Warning | HorrorThreatAIController.cpp | 14 | Cast<> without null check | Use if (AActor* Actor = Cast<>()) or ensure() |
| Warning | AmbientAudioComponent.cpp | 248 | Chained call without null check | Use ensure() or check() |
| Warning | AmbientAudioComponent.cpp | 253 | Chained call without null check | Use ensure() or check() |
| Warning | AmbientAudioComponent.cpp | 276 | Chained call without null check | Use ensure() or check() |
| Warning | FootstepAudioComponent.cpp | 125 | Chained call without null check | Use ensure() or check() |
| Warning | FootstepAudioComponent.cpp | 199 | Chained call without null check | Use ensure() or check() |
| Warning | HorrorAudioSubsystem.cpp | 19 | Chained call without null check | Use ensure() or check() |
| Warning | HorrorAudioSubsystem.cpp | 29 | Chained call without null check | Use ensure() or check() |
| Warning | HorrorAudioSubsystem.cpp | 314 | Chained call without null check | Use ensure() or check() |
| Warning | HorrorAudioSubsystem.cpp | 344 | Chained call without null check | Use ensure() or check() |
| Warning | HorrorAudioSubsystem.cpp | 364 | Chained call without null check | Use ensure() or check() |
| Warning | HorrorAudioSubsystem.cpp | 417 | Chained call without null check | Use ensure() or check() |
| Warning | HorrorAudioSubsystem.cpp | 446 | Chained call without null check | Use ensure() or check() |
| Warning | HorrorAudioSubsystem.cpp | 459 | Chained call without null check | Use ensure() or check() |
| Warning | HorrorAudioSubsystem.cpp | 475 | Chained call without null check | Use ensure() or check() |
| Warning | EvidenceCollectionComponent.cpp | 92 | Chained call without null check | Use ensure() or check() |
| Warning | EvidenceCollectionComponent.cpp | 105 | Chained call without null check | Use ensure() or check() |
| Warning | EvidenceCollectionComponent.cpp | 246 | Chained call without null check | Use ensure() or check() |
| ... | ... | ... | and 29 more | ... |

### Complexity (18 issues)
| Severity | File | Line | Issue | Recommendation |
|----------|------|------|-------|----------------|
| Warning | HorrorGolemBehaviorComponent.cpp | 95 | Function too long (66 lines) | Consider refactoring into smaller functions |
| Warning | HorrorGolemBehaviorComponent.cpp | 370 | Function too long (59 lines) | Consider refactoring into smaller functions |
| Warning | FootstepAudioComponent.cpp | 109 | Function too long (62 lines) | Consider refactoring into smaller functions |
| Warning | DeepWaterStationRouteKit.cpp | 69 | Function too long (128 lines) | Consider refactoring into smaller functions |
| Warning | FoundFootageObjectiveInteractable.cpp | 68 | Function too long (62 lines) | Consider refactoring into smaller functions |
| Warning | FoundFootageObjectiveInteractable.cpp | 137 | Function too long (80 lines) | Consider refactoring into smaller functions |
| Warning | FoundFootageObjectiveInteractable.cpp | 219 | Function too long (62 lines) | Consider refactoring into smaller functions |
| Warning | HorrorGameModeBase.cpp | 415 | Function too long (67 lines) | Consider refactoring into smaller functions |
| Warning | CameraBatteryComponent.cpp | 92 | Function too long (59 lines) | Consider refactoring into smaller functions |
| Warning | CameraPhotoComponent.cpp | 119 | Function too long (73 lines) | Consider refactoring into smaller functions |
| Warning | CameraPhotoComponent.cpp | 260 | Function too long (58 lines) | Consider refactoring into smaller functions |
| Warning | InteractionComponentTests.cpp | 58 | Function too long (55 lines) | Consider refactoring into smaller functions |
| Warning | InteractionComponentTests.cpp | 269 | Function too long (51 lines) | Consider refactoring into smaller functions |
| Warning | InventoryComponent.cpp | 22 | Function too long (65 lines) | Consider refactoring into smaller functions |
| Warning | NoteRecorderComponent.cpp | 22 | Function too long (65 lines) | Consider refactoring into smaller functions |
| Warning | PostProcessController.cpp | 211 | Function too long (56 lines) | Consider refactoring into smaller functions |
| Warning | AssetValidator.cpp | 13 | Function too long (56 lines) | Consider refactoring into smaller functions |
| Warning | HorrorEditorUI.cpp | 23 | Function too long (106 lines) | Consider refactoring into smaller functions |

### API (12 issues)
| Severity | File | Line | Issue | Recommendation |
|----------|------|------|-------|----------------|
| Warning | HorrorAudioSubsystem.h | 128 | Non-const reference parameter | Use const& for input, pointer for output |
| Warning | ArchiveSubsystem.h | 61 | Non-const reference parameter | Use const& for input, pointer for output |
| Warning | FrameTimeTracker.h | 61 | Non-const reference parameter | Use const& for input, pointer for output |
| Warning | MemoryTracker.h | 73 | Non-const reference parameter | Use const& for input, pointer for output |
| Warning | PerformanceBudget.h | 61 | Non-const reference parameter | Use const& for input, pointer for output |
| Warning | HorrorSaveSubsystem.cpp | 30 | Non-const reference parameter | Use const& for input, pointer for output |
| Warning | HorrorCharacter.h | 74 | Public member without UPROPERTY | Encapsulate or add UPROPERTY |
| Warning | HorrorCharacter.h | 77 | Public member without UPROPERTY | Encapsulate or add UPROPERTY |
| Warning | MaterialOptimizer.h | 40 | Non-const reference parameter | Use const& for input, pointer for output |
| Warning | MaterialOptimizer.h | 41 | Non-const reference parameter | Use const& for input, pointer for output |
| Warning | MaterialOptimizer.h | 42 | Non-const reference parameter | Use const& for input, pointer for output |
| Warning | MaterialOptimizer.h | 43 | Non-const reference parameter | Use const& for input, pointer for output |

### Memory (6 issues)
| Severity | File | Line | Issue | Recommendation |
|----------|------|------|-------|----------------|
| Warning | ArchiveViewerWidget.h | 54 | Raw 'new' detected | Use NewObject<> or TSharedPtr/TUniquePtr |
| Warning | HorrorPlayerController.h | 37 | Raw 'new' detected | Use NewObject<> or TSharedPtr/TUniquePtr |
| Warning | HorrorUI.h | 80 | Raw 'new' detected | Use NewObject<> or TSharedPtr/TUniquePtr |
| Warning | HorrorUI.h | 84 | Raw 'new' detected | Use NewObject<> or TSharedPtr/TUniquePtr |
| Warning | HorrorUI.h | 128 | Raw 'new' detected | Use NewObject<> or TSharedPtr/TUniquePtr |
| Warning | HorrorUI.h | 132 | Raw 'new' detected | Use NewObject<> or TSharedPtr/TUniquePtr |

## Critical Issues (Errors)
No critical issues found.

