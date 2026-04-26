# System Overview

## Introduction

HorrorProject is an Unreal Engine 5 horror game built with a modular, event-driven architecture. The system emphasizes decoupled components communicating through a central event bus, enabling flexible gameplay scripting and dynamic horror encounters.

## Core Architecture Principles

### Event-Driven Design
All major game systems communicate through the `UHorrorEventBusSubsystem`, providing:
- Loose coupling between systems
- Easy debugging and logging
- Flexible gameplay scripting
- Replay and analytics support

### Subsystem-Based Organization
Key functionality is implemented as UE5 World Subsystems:
- `UHorrorEventBusSubsystem` - Central event dispatcher
- `UHorrorAudioSubsystem` - Spatial audio management
- `UArchiveSubsystem` - Evidence collection and persistence

### Director Pattern
Complex sequences are managed by Director actors:
- `AHorrorEncounterDirector` - Manages threat encounters
- `AHorrorAnomalyDirector` - Controls environmental anomalies

## System Modules

### Game Module
Core gameplay logic and directors.

**Key Classes:**
- `AHorrorGameModeBase` - Game mode and session management
- `AHorrorEncounterDirector` - Encounter state machine
- `UHorrorEventBusSubsystem` - Event bus implementation

### Player Module
Player character, controller, and camera systems.

**Key Classes:**
- `AHorrorProjectCharacter` - Player character with interaction
- `AHorrorProjectPlayerController` - Input and UI management
- `AHorrorProjectCameraManager` - Camera effects and shakes

### AI Module
Enemy AI and behavior systems.

**Key Classes:**
- `AHorrorThreatCharacter` - Enemy character base
- `AHorrorThreatAIController` - AI decision making
- `UHorrorGolemBehaviorComponent` - Specific enemy behaviors

### Audio Module
Spatial audio and dynamic soundscapes.

**Key Classes:**
- `UHorrorAudioSubsystem` - Audio management
- `AHorrorAudioZoneActor` - Spatial audio zones
- `UHorrorAudioLibrary` - Sound asset management

### Evidence Module
Evidence collection and archive system.

**Key Classes:**
- `UArchiveSubsystem` - Evidence storage
- `UEvidenceCollectionComponent` - Collection logic
- `AEvidenceActor` - Collectible evidence items

### Interaction Module
Player interaction with world objects.

**Key Classes:**
- `UHorrorInteractionComponent` - Interaction detection
- `IHorrorInteractable` - Interactable interface

### UI Module
User interface and HUD systems.

**Key Classes:**
- `UHorrorHUDWidget` - Main HUD
- `UArchiveViewerWidget` - Evidence viewer
- `UObjectiveWidget` - Objective display

### Performance Module
Performance monitoring and optimization.

**Key Classes:**
- `UPerformanceMonitorSubsystem` - Performance tracking
- `UMemoryTrackerComponent` - Memory profiling

### Save Module
Save game and persistence.

**Key Classes:**
- `UHorrorSaveGame` - Save data structure
- `USaveGameSubsystem` - Save/load management

## Data Flow

```
Player Input → Player Controller → Game Logic → Event Bus → Subsystems
                                                    ↓
                                            Audio/UI/AI Systems
                                                    ↓
                                              Save System
```

## Event Bus Architecture

The Event Bus is the central nervous system of the game:

1. **Publishers** emit events with tags and metadata
2. **Event Bus** stores history and routes messages
3. **Subscribers** receive events and react accordingly

**Event Message Structure:**
- `EventTag` - Gameplay tag identifying the event
- `StateTag` - Current state (e.g., Started, Completed)
- `SourceId` - Unique identifier for the source
- `SourceObject` - Reference to the emitting object
- `Metadata` - Optional objective hints and debug info

## Threading Model

- **Game Thread** - All gameplay logic
- **Render Thread** - Rendering and VFX
- **Audio Thread** - Audio processing
- **Async Tasks** - Save/load operations

## Memory Management

- Smart pointers (`TObjectPtr`) for UObject references
- Garbage collection for UObjects
- Manual cleanup for non-UObject resources
- Memory pools for frequently allocated objects

## Performance Targets

- **Frame Rate:** 60 FPS (PC), 30 FPS (Console)
- **Memory:** < 8GB RAM usage
- **Load Times:** < 5 seconds for level transitions
- **Draw Calls:** < 3000 per frame

## Scalability

The system supports multiple quality levels:
- Graphics quality (Low/Medium/High/Epic)
- Audio quality (channels, reverb complexity)
- AI complexity (update frequency, perception range)

## Testing Strategy

- **Unit Tests** - Individual component testing
- **Integration Tests** - System interaction testing
- **Automation Tests** - Gameplay sequence validation
- **Performance Tests** - Frame rate and memory profiling

## Dependencies

### External
- Unreal Engine 5.6
- Visual Studio 2022
- Windows 10/11

### Internal
- Gameplay Tags system
- Enhanced Input system
- MetaSounds audio system

## Future Considerations

- Network replication for multiplayer
- Procedural content generation
- Advanced AI behaviors
- VR support

---
Last Updated: 2026-04-26
