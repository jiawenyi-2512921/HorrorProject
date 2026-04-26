# Dependencies

## External Dependencies

### Unreal Engine 5.6
**Version:** 5.6.0  
**License:** Unreal Engine EULA  
**Purpose:** Core game engine

**Key Systems Used:**
- Enhanced Input System
- Gameplay Tags
- Gameplay Abilities (future)
- MetaSounds
- Niagara VFX
- World Subsystems
- Automation Testing Framework

### Visual Studio 2022
**Version:** 17.x  
**License:** Microsoft EULA  
**Purpose:** C++ development and debugging

**Required Components:**
- C++ Desktop Development
- Windows 10/11 SDK
- .NET Framework 4.8

### Windows SDK
**Version:** 10.0.22621.0 or later  
**License:** Microsoft  
**Purpose:** Platform APIs

## Internal Module Dependencies

### Dependency Graph

```
HorrorProject (Core Module)
    ├── Game Module
    │   ├── Event Bus Subsystem
    │   ├── Encounter Director
    │   └── Anomaly Director
    │
    ├── Player Module
    │   ├── Player Character
    │   ├── Player Controller
    │   └── Camera Manager
    │
    ├── AI Module
    │   ├── Threat AI Controller
    │   ├── Threat Character
    │   └── Golem Behavior
    │
    ├── Audio Module
    │   ├── Audio Subsystem
    │   ├── Audio Zones
    │   └── Audio Library
    │
    ├── Evidence Module
    │   ├── Archive Subsystem
    │   ├── Evidence Collection
    │   └── Evidence Actors
    │
    ├── Interaction Module
    │   ├── Interaction Component
    │   └── Interactable Interface
    │
    ├── UI Module
    │   ├── HUD Widget
    │   ├── Archive Viewer
    │   └── Objective Widget
    │
    ├── Performance Module
    │   ├── Performance Monitor
    │   └── Memory Tracker
    │
    ├── Save Module
    │   ├── Save Game
    │   └── Save Subsystem
    │
    └── VFX Module
        ├── VFX Manager
        └── Camera Effects
```

### Module Dependency Rules

**Core Module (HorrorProject)**
- No dependencies on other modules
- Provides base classes and interfaces
- Defines project-wide types

**Game Module**
- Depends on: Core
- Provides: Event Bus, Directors
- Used by: All other modules

**Player Module**
- Depends on: Core, Game
- Provides: Player systems
- Used by: UI, Interaction

**AI Module**
- Depends on: Core, Game
- Provides: Enemy AI
- Used by: Game (Encounter Director)

**Audio Module**
- Depends on: Core, Game
- Provides: Audio management
- Used by: Game, Player, AI

**Evidence Module**
- Depends on: Core, Game
- Provides: Evidence system
- Used by: Player, UI, Save

**Interaction Module**
- Depends on: Core
- Provides: Interaction framework
- Used by: Player, Evidence

**UI Module**
- Depends on: Core, Game, Player, Evidence
- Provides: User interface
- Used by: None (top-level)

**Performance Module**
- Depends on: Core
- Provides: Profiling tools
- Used by: All modules (optional)

**Save Module**
- Depends on: Core, Game, Evidence
- Provides: Persistence
- Used by: Game, Player

**VFX Module**
- Depends on: Core, Game
- Provides: Visual effects
- Used by: Game, Player, AI

## Build Dependencies

### Build.cs Configuration

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core",
    "CoreUObject",
    "Engine",
    "InputCore",
    "EnhancedInput",
    "GameplayTags",
    "UMG",
    "Slate",
    "SlateCore",
    "Niagara",
    "MetasoundEngine"
});

PrivateDependencyModuleNames.AddRange(new string[] {
    "RenderCore",
    "RHI",
    "AudioMixer"
});
```

### Plugin Dependencies

**Required Plugins:**
- Enhanced Input
- Gameplay Tags
- MetaSounds
- Niagara

**Optional Plugins:**
- Gameplay Abilities (future)
- Online Subsystem (future)
- Steam Integration (future)

## Asset Dependencies

### Content Dependencies

**Starter Content:**
- Basic materials
- Placeholder meshes
- Test textures

**Third-Party Assets:**
- Audio libraries (licensed)
- Texture packs (licensed)
- 3D models (licensed)

### Asset References

**Hard References:**
- Player character blueprint
- Game mode blueprint
- Essential UI widgets
- Core audio assets

**Soft References:**
- Level-specific assets
- Evidence content
- Optional audio
- Cinematic assets

## Runtime Dependencies

### System Requirements

**Minimum:**
- OS: Windows 10 64-bit
- CPU: Intel Core i5-8400 / AMD Ryzen 5 2600
- RAM: 8 GB
- GPU: NVIDIA GTX 1060 / AMD RX 580
- DirectX: Version 12
- Storage: 20 GB

**Recommended:**
- OS: Windows 11 64-bit
- CPU: Intel Core i7-10700K / AMD Ryzen 7 3700X
- RAM: 16 GB
- GPU: NVIDIA RTX 3060 / AMD RX 6700 XT
- DirectX: Version 12
- Storage: 20 GB SSD

### Runtime Libraries

**Required:**
- Visual C++ Redistributable 2022
- DirectX 12 Runtime
- .NET Framework 4.8

**Optional:**
- Steam Client (for Steam builds)
- Epic Online Services (for EOS builds)

## Development Dependencies

### Tools

**Required:**
- Visual Studio 2022
- Unreal Engine 5.6
- Git (version control)

**Recommended:**
- Visual Assist (C++ productivity)
- RenderDoc (graphics debugging)
- Unreal Insights (profiling)
- PlantUML (documentation)
- Doxygen (API docs)

### Testing Dependencies

**Unit Testing:**
- Unreal Automation Framework
- Google Test (future)

**Integration Testing:**
- Gauntlet Framework
- Custom test harness

## Dependency Management

### Version Control

**Git Submodules:**
- None currently
- Future: Third-party libraries

**Package Management:**
- UE Marketplace assets
- Manual asset integration

### Dependency Updates

**Update Strategy:**
- Pin Unreal Engine version
- Test updates in separate branch
- Document breaking changes
- Maintain compatibility layer

**Update Frequency:**
- Engine: Major versions only
- Plugins: As needed
- Assets: As needed
- Tools: Monthly

## Circular Dependency Prevention

### Design Rules

**Prohibited:**
- UI depending on AI
- AI depending on UI
- Player depending on Evidence (use events)
- Evidence depending on Player (use events)

**Allowed:**
- All modules depending on Core
- All modules depending on Game (Event Bus)
- UI depending on all data modules

### Dependency Injection

**Subsystem Pattern:**
```cpp
// Get subsystem reference (no hard dependency)
UHorrorEventBusSubsystem* EventBus = GetWorld()->GetSubsystem<UHorrorEventBusSubsystem>();
```

**Interface Pattern:**
```cpp
// Use interface (no concrete class dependency)
IHorrorInteractable* Interactable = Cast<IHorrorInteractable>(Actor);
```

## Dependency Visualization

### Module Dependency Matrix

```
           Core Game Player AI Audio Evidence Interaction UI Perf Save VFX
Core        -    -    -     -   -      -         -        -   -    -    -
Game        X    -    -     -   -      -         -        -   -    -    -
Player      X    X    -     -   -      -         -        -   -    -    -
AI          X    X    -     -   -      -         -        -   -    -    -
Audio       X    X    -     -   -      -         -        -   -    -    -
Evidence    X    X    -     -   -      -         -        -   -    -    -
Interaction X    -    -     -   -      -         -        -   -    -    -
UI          X    X    X     -   -      X         -        -   -    -    -
Perf        X    -    -     -   -      -         -        -   -    -    -
Save        X    X    -     -   -      X         -        -   -    -    -
VFX         X    X    -     -   -      -         -        -   -    -    -
```

X = Depends on

## Troubleshooting

### Common Dependency Issues

**Circular Dependencies:**
- Symptom: Linker errors
- Solution: Use forward declarations and interfaces

**Missing Dependencies:**
- Symptom: Unresolved external symbols
- Solution: Add to Build.cs PublicDependencyModuleNames

**Version Conflicts:**
- Symptom: Compilation errors after update
- Solution: Clean rebuild, check compatibility

**Asset Reference Issues:**
- Symptom: Missing assets at runtime
- Solution: Fix asset references, check soft references

---
Last Updated: 2026-04-26
