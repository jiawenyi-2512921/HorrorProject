# API Reference

Complete API documentation for HorrorProject C++ classes and Blueprint libraries.

## Core Systems

### Audio System
- [HorrorAudioSubsystem](Audio/HorrorAudioSubsystem.md) - Main audio management system
- [HorrorAudioLibrary](Audio/HorrorAudioLibrary.md) - Audio asset library
- [AmbientAudioComponent](Audio/AmbientAudioComponent.md) - Ambient sound management
- [BreathingAudioComponent](Audio/BreathingAudioComponent.md) - Player breathing audio
- [FootstepAudioComponent](Audio/FootstepAudioComponent.md) - Footstep sound system
- [UnderwaterAudioComponent](Audio/UnderwaterAudioComponent.md) - Underwater audio effects
- [HorrorAudioZoneActor](Audio/HorrorAudioZoneActor.md) - Audio zone management
- [HorrorAudioAttenuationPresets](Audio/HorrorAudioAttenuationPresets.md) - Audio attenuation settings

### Evidence System
- [EvidenceCollectionComponent](Evidence/EvidenceCollectionComponent.md) - Evidence collection management
- [EvidenceActor](Evidence/EvidenceActor.md) - Base evidence actor class
- [ArchiveSubsystem](Evidence/ArchiveSubsystem.md) - Evidence archive system
- [ArchiveViewerWidget](Evidence/ArchiveViewerWidget.md) - Archive UI widget
- [EvidenceBlueprintLibrary](Evidence/EvidenceBlueprintLibrary.md) - Blueprint function library
- [EvidenceEventBridge](Evidence/EvidenceEventBridge.md) - Event system bridge
- [EvidenceSaveGame](Evidence/EvidenceSaveGame.md) - Save game functionality
- [EvidenceTypes](Evidence/EvidenceTypes.md) - Evidence type definitions

### AI System
- [HorrorThreatAIController](AI/HorrorThreatAIController.md) - AI controller for threats
- [HorrorThreatCharacter](AI/HorrorThreatCharacter.md) - Threat character base class
- [HorrorGolemBehaviorComponent](AI/HorrorGolemBehaviorComponent.md) - Golem AI behavior

### Game Systems
- [HorrorGameModeBase](Game/HorrorGameModeBase.md) - Base game mode
- [HorrorEventBusSubsystem](Game/HorrorEventBusSubsystem.md) - Event bus system
- [HorrorEncounterDirector](Game/HorrorEncounterDirector.md) - Encounter management
- [HorrorAnomalyDirector](Game/HorrorAnomalyDirector.md) - Anomaly system director
- [HorrorFoundFootageContract](Game/HorrorFoundFootageContract.md) - Found footage mechanics
- [DeepWaterStationRouteKit](Game/DeepWaterStationRouteKit.md) - Route management

### Interaction System
- [HorrorInteractionNode](Game/HorrorInteractionNode.md) - Interaction node base
- [HorrorEncounterNode](Game/HorrorEncounterNode.md) - Encounter node system
- [FoundFootageObjectiveInteractable](Game/FoundFootageObjectiveInteractable.md) - Objective interactions

## Documentation Conventions

### Code Examples
All code examples follow this format:
```cpp
// C++ Example
UHorrorAudioSubsystem* AudioSubsystem = GetWorld()->GetSubsystem<UHorrorAudioSubsystem>();
```

```blueprint
// Blueprint Example (pseudocode)
Get Horror Audio Subsystem -> Play Ambient Sound
```

### Parameter Documentation
- **Required Parameters**: Marked with `[Required]`
- **Optional Parameters**: Marked with `[Optional]` and include default values
- **Output Parameters**: Marked with `[Out]`

### Return Values
All functions document their return values with type and description.

## Generating Documentation

Use Doxygen to generate HTML documentation:
```bash
cd Docs/API
doxygen Doxyfile
```

See [Doxygen Configuration](DoxygenConfig.md) for setup details.

## Quick Links
- [Getting Started](../Tutorials/GettingStarted.md)
- [Blueprint Guide](../Blueprint/Blueprint_QuickStart.md)
- [Troubleshooting](../Troubleshooting/FAQ.md)
- [Best Practices](../BestPractices/README.md)
