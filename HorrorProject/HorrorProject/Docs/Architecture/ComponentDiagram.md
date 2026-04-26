# Component Diagram

## System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                         HorrorProject                            │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                        Core Systems                              │
├─────────────────────────────────────────────────────────────────┤
│  ┌──────────────────┐  ┌──────────────────┐  ┌──────────────┐  │
│  │  Event Bus       │  │  Audio           │  │  Save        │  │
│  │  Subsystem       │  │  Subsystem       │  │  System      │  │
│  └──────────────────┘  └──────────────────┘  └──────────────┘  │
└─────────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│                        Game Logic                                │
├─────────────────────────────────────────────────────────────────┤
│  ┌──────────────────┐  ┌──────────────────┐  ┌──────────────┐  │
│  │  Encounter       │  │  Anomaly         │  │  Game        │  │
│  │  Director        │  │  Director        │  │  Mode        │  │
│  └──────────────────┘  └──────────────────┘  └──────────────┘  │
└─────────────────────────────────────────────────────────────────┘
         ↓                       ↓                      ↓
┌─────────────────────────────────────────────────────────────────┐
│                     Player Systems                               │
├─────────────────────────────────────────────────────────────────┤
│  ┌──────────────────┐  ┌──────────────────┐  ┌──────────────┐  │
│  │  Player          │  │  Player          │  │  Camera      │  │
│  │  Character       │  │  Controller      │  │  Manager     │  │
│  └──────────────────┘  └──────────────────┘  └──────────────┘  │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                        AI Systems                                │
├─────────────────────────────────────────────────────────────────┤
│  ┌──────────────────┐  ┌──────────────────┐  ┌──────────────┐  │
│  │  Threat AI       │  │  Threat          │  │  Golem       │  │
│  │  Controller      │  │  Character       │  │  Behavior    │  │
│  └──────────────────┘  └──────────────────┘  └──────────────┘  │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                     Evidence System                              │
├─────────────────────────────────────────────────────────────────┤
│  ┌──────────────────┐  ┌──────────────────┐  ┌──────────────┐  │
│  │  Archive         │  │  Evidence        │  │  Evidence    │  │
│  │  Subsystem       │  │  Collection      │  │  Actor       │  │
│  └──────────────────┘  └──────────────────┘  └──────────────┘  │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                    Interaction System                            │
├─────────────────────────────────────────────────────────────────┤
│  ┌──────────────────┐  ┌──────────────────┐                     │
│  │  Interaction     │  │  Interactable    │                     │
│  │  Component       │  │  Objects         │                     │
│  └──────────────────┘  └──────────────────┘                     │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                         UI System                                │
├─────────────────────────────────────────────────────────────────┤
│  ┌──────────────────┐  ┌──────────────────┐  ┌──────────────┐  │
│  │  HUD             │  │  Archive         │  │  Objective   │  │
│  │  Widget          │  │  Viewer          │  │  Widget      │  │
│  └──────────────────┘  └──────────────────┘  └──────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

## Component Relationships

### Event Bus (Central Hub)

**Publishers:**
- Game Mode
- Encounter Director
- Anomaly Director
- Evidence Collection
- Player Controller
- AI Controllers

**Subscribers:**
- Audio Subsystem
- UI Widgets
- Save System
- Analytics
- Debug Tools

### Encounter Director

**Dependencies:**
- Event Bus (event publishing)
- Audio Subsystem (sound playback)
- Threat Character (spawning)
- Camera Manager (camera shakes)

**Dependents:**
- Game Mode (encounter management)
- UI System (encounter feedback)
- Save System (encounter state)

### Player Systems

**Player Character:**
- Uses Interaction Component
- Controlled by Player Controller
- Publishes player events

**Player Controller:**
- Manages input
- Controls Camera Manager
- Interfaces with UI

**Camera Manager:**
- Handles camera effects
- Responds to encounter events
- Manages view transitions

### AI Systems

**Threat AI Controller:**
- Controls Threat Character
- Uses behavior trees
- Responds to player proximity

**Threat Character:**
- Spawned by Encounter Director
- Uses Golem Behavior Component
- Publishes threat events

**Golem Behavior:**
- Implements specific AI patterns
- Responds to encounter phases
- Manages threat animations

### Evidence System

**Archive Subsystem:**
- Stores collected evidence
- Manages evidence metadata
- Handles save/load

**Evidence Collection:**
- Attached to Player Character
- Detects evidence actors
- Publishes collection events

**Evidence Actor:**
- Placed in world
- Implements interactable interface
- Contains evidence data

### Interaction System

**Interaction Component:**
- Attached to Player Character
- Performs line traces
- Manages interaction state

**Interactable Objects:**
- Implement IHorrorInteractable
- Respond to player interaction
- Provide interaction feedback

### UI System

**HUD Widget:**
- Main player interface
- Subscribes to all events
- Displays health, objectives, hints

**Archive Viewer:**
- Evidence browser
- Reads from Archive Subsystem
- Supports filtering and search

**Objective Widget:**
- Displays current objectives
- Updates from event bus
- Shows progress and hints

## Data Flow Patterns

### Encounter Flow
```
Player Proximity → Encounter Director → Event Bus → Audio/UI/AI
                                          ↓
                                    Save System
```

### Evidence Collection Flow
```
Player Interaction → Evidence Actor → Evidence Collection → Archive Subsystem
                                                               ↓
                                                          Event Bus → UI
```

### Audio Flow
```
Event Bus → Audio Subsystem → Audio Zones → MetaSounds → Audio Output
```

### Save Flow
```
Game Systems → Event Bus → Save System → Save Game → Disk
```

## Module Dependencies

```
Core Systems (no dependencies)
    ↓
Game Logic (depends on Core)
    ↓
Player/AI/Evidence (depends on Game Logic)
    ↓
UI (depends on all above)
```

## Initialization Order

1. Core Subsystems (Event Bus, Audio, Save)
2. Game Mode
3. Player Controller and Character
4. Directors (Encounter, Anomaly)
5. AI Systems
6. UI Widgets

## Communication Patterns

### Synchronous
- Direct function calls within same system
- Interface implementations
- Component queries

### Asynchronous
- Event Bus messages
- Delegate broadcasts
- Timer callbacks

### One-Way
- Event publishing (fire and forget)
- UI updates from game state

### Two-Way
- Player interaction (request/response)
- Save/load operations

## Scalability Considerations

### Horizontal Scaling
- Multiple encounter directors per level
- Multiple audio zones
- Multiple evidence actors

### Vertical Scaling
- Event bus history size
- Audio channel count
- UI update frequency

---
Last Updated: 2026-04-26
