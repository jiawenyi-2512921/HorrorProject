# Blueprint Quick Start Guide

## Overview

This guide provides a fast-track introduction to implementing Blueprints for the HorrorProject C++ systems. All C++ systems are designed with Blueprint integration in mind, using `UCLASS(BlueprintType, Blueprintable)` and `UFUNCTION(BlueprintCallable)` specifiers.

## Prerequisites

- Unreal Engine 5.6 installed
- HorrorProject compiled successfully
- Basic understanding of Blueprint visual scripting
- Familiarity with UE5 Content Browser

## Quick Setup Workflow

### 1. Compile C++ Code
```bash
# Ensure all C++ systems are compiled
# Open HorrorProject.sln in Visual Studio
# Build Solution (Ctrl+Shift+B)
```

### 2. Verify C++ Classes Available
- Open UE5 Editor
- Content Browser → View Options → Show C++ Classes
- Navigate to HorrorProject folder
- Verify all systems are visible

### 3. Create Blueprint Assets
Follow priority order:
1. **Priority 0 (Blocking)** - Core systems required for gameplay
2. **Priority 1 (Critical Path)** - Main gameplay features
3. **Priority 2 (Enhancement)** - Polish and effects
4. **Priority 3 (Optional)** - Future optimizations

## System Overview

### VFX System
- **PostProcessController** - 9 post-process effects (VHS, Underwater, Fear, etc.)
- **ParticleSpawner** - 8 Niagara particle types
- **ScreenEffectManager** - Camera shakes and screen effects

### UI System
- **BodycamOverlayWidget** - Recording HUD
- **EvidenceToastWidget** - Evidence notifications
- **ObjectiveToastWidget** - Objective notifications
- **ArchiveMenuWidget** - Evidence archive viewer
- **PauseMenuWidget** - Game pause menu
- **SettingsMenuWidget** - Settings interface

### Audio System
- **AmbientAudioComponent** - Layered ambient audio
- **FootstepAudioComponent** - Dynamic footstep sounds
- **BreathingAudioComponent** - Player breathing
- **UnderwaterAudioComponent** - Underwater audio effects

### Evidence System
- **EvidenceActor** - Collectible evidence objects
- **ArchiveViewerWidget** - Evidence viewing interface
- **EvidenceCollectionComponent** - Player collection system

### Camera System
- **CameraPhotoComponent** - Photo capture (36 photos max)
- **CameraRecordingComponent** - Video recording
- **CameraBatteryComponent** - Battery management

### Interaction System
- **BaseInteractable** - Base class for all interactables
- **DocumentInteractable** - Readable documents
- **DoorInteractable** - Doors and hatches
- **ExaminableInteractable** - Examinable objects
- **PickupInteractable** - Pickup items
- **RecorderInteractable** - Audio recorders
- **SwitchInteractable** - Switches and buttons

### Objective System
- **HorrorObjectiveNode** - Base objective node
- **HorrorNavigationNode** - Navigation objectives
- **HorrorInteractionNode** - Interaction objectives
- **HorrorEncounterNode** - Encounter objectives

## First Blueprint to Create

**Start with BP_PlayerCharacter** - This is the foundation:

1. Content Browser → Right-click → Blueprint Class
2. Parent Class: HorrorProjectCharacter
3. Name: BP_PlayerCharacter
4. Add components:
   - CameraPhotoComponent
   - CameraRecordingComponent
   - CameraBatteryComponent
   - EvidenceCollectionComponent

## Common Blueprint Patterns

### Adding C++ Components
```
1. Open Blueprint
2. Components Panel → Add Component
3. Search for component name (e.g., "CameraPhoto")
4. Configure properties in Details panel
```

### Calling C++ Functions
```
1. Event Graph → Right-click
2. Search function name (e.g., "Take Photo")
3. Connect to event or input
4. Configure parameters
```

### Binding to C++ Events
```
1. Select component in Components panel
2. Details panel → Events section
3. Click + next to event (e.g., "On Photo Taken")
4. Implement response in Event Graph
```

## Asset Organization

Recommended Content folder structure:
```
Content/
├── Blueprints/
│   ├── Characters/
│   │   └── BP_PlayerCharacter
│   ├── Interactables/
│   │   ├── BP_DocumentInteractable
│   │   ├── BP_DoorInteractable
│   │   └── ...
│   ├── Evidence/
│   │   └── BP_EvidenceActor
│   └── Objectives/
│       └── BP_ObjectiveNode
├── UI/
│   ├── Widgets/
│   │   ├── WBP_BodycamOverlay
│   │   ├── WBP_EvidenceToast
│   │   └── ...
│   └── Materials/
├── VFX/
│   ├── Niagara/
│   │   ├── NS_UnderwaterDebris
│   │   ├── NS_BloodSplatter
│   │   └── ...
│   └── PostProcess/
│       ├── M_VHSEffect
│       ├── M_UnderwaterEffect
│       └── ...
└── Audio/
    ├── Ambient/
    ├── Footsteps/
    └── Effects/
```

## Testing Workflow

### 1. Component Testing
- Create test map: `Maps/Test/TestMap_[SystemName]`
- Place test actor with component
- Use Print String nodes to verify functionality
- Check Output Log for errors

### 2. Integration Testing
- Test in main game map
- Verify component interactions
- Check performance (Stat FPS, Stat Unit)
- Profile with Unreal Insights

### 3. Validation Checklist
- [ ] Component appears in editor
- [ ] Properties are editable
- [ ] Functions execute without errors
- [ ] Events fire correctly
- [ ] Performance is acceptable
- [ ] No memory leaks (check with Stat Memory)

## Common Issues

### C++ Class Not Visible
- Solution: Recompile C++ code, restart editor
- Check: Module loaded in .uproject file

### Blueprint Compile Errors
- Solution: Check C++ function signatures match
- Verify: All required parameters provided

### Component Not Working
- Solution: Verify BeginPlay called
- Check: Component registered and initialized

### Performance Issues
- Solution: Enable LOD settings
- Check: Particle budgets, audio attenuation

## Next Steps

1. Read system-specific guides:
   - [VFX Blueprint Guide](VFX_Blueprint_Guide.md)
   - [UI Blueprint Guide](UI_Blueprint_Guide.md)
   - [Audio Blueprint Guide](Audio_Blueprint_Guide.md)
   - [Evidence Blueprint Guide](Evidence_Blueprint_Guide.md)
   - [Camera Blueprint Guide](Camera_Blueprint_Guide.md)
   - [Interaction Blueprint Guide](Interaction_Blueprint_Guide.md)
   - [Objective Blueprint Guide](Objective_Blueprint_Guide.md)

2. Review templates in `Templates/` folder

3. Check [Asset Checklist](AssetChecklist.md) for required assets

4. Follow [Implementation Priority](ImplementationPriority.md) order

5. Use [Validation Checklist](ValidationChecklist.md) before marking complete

## Support Resources

- C++ API Documentation: `Docs/API/`
- Architecture Documentation: `Docs/Architecture/`
- Performance Guidelines: `Docs/Performance/`
- Code Examples: `Source/HorrorProject/*/Tests/`

## Best Practices

1. **Always inherit from C++ classes** - Don't recreate functionality
2. **Use Blueprint Interfaces** - For cross-Blueprint communication
3. **Keep Blueprints simple** - Complex logic stays in C++
4. **Name consistently** - BP_ prefix for Blueprints, WBP_ for Widgets
5. **Comment your graphs** - Explain non-obvious logic
6. **Use Reroute nodes** - Keep graphs clean and readable
7. **Group related nodes** - Use comment boxes
8. **Test incrementally** - Don't build everything at once
9. **Profile early** - Check performance as you build
10. **Version control** - Commit working states frequently
