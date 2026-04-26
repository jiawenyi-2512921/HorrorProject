# Data Flow Architecture

## Overview

This document describes how data flows through the HorrorProject system, from player input to game state changes, persistence, and feedback.

## Primary Data Flows

### 1. Player Input Flow

```
Keyboard/Mouse → Enhanced Input System → Player Controller → Player Character
                                              ↓
                                        Game Actions
                                              ↓
                                    ┌─────────┴─────────┐
                                    ↓                   ↓
                              Movement            Interaction
                                    ↓                   ↓
                              Animation         Interactable Objects
```

**Data Types:**
- Input Actions (Move, Look, Interact, Sprint)
- Input Values (Vector2D, Float, Boolean)
- Action States (Started, Ongoing, Completed, Canceled)

**Processing:**
1. Raw input captured by UE5 Input System
2. Mapped to Enhanced Input Actions
3. Processed by Player Controller
4. Dispatched to appropriate handlers
5. Game state updated

### 2. Event Bus Flow

```
Game Event Source → Event Bus Subsystem → Event History Buffer
                                              ↓
                                    ┌─────────┴─────────┐
                                    ↓                   ↓
                            Native Delegates    Dynamic Delegates
                                    ↓                   ↓
                            C++ Subscribers    Blueprint Subscribers
```

**Event Message Structure:**
```cpp
FHorrorEventMessage {
    FGameplayTag EventTag;      // 8 bytes
    FGameplayTag StateTag;      // 8 bytes
    FName SourceId;             // 8 bytes
    UObject* SourceObject;      // 8 bytes
    float WorldSeconds;         // 4 bytes
    FName TrailerBeatId;        // 8 bytes
    FText ObjectiveHint;        // ~32 bytes
    FText DebugLabel;           // ~32 bytes
}
// Total: ~108 bytes per message
```

**Throughput:**
- Average: 10-50 events/second
- Peak: 200 events/second (during intense sequences)
- History: 128 messages (circular buffer)

### 3. Audio Data Flow

```
Audio Event → Audio Subsystem → Audio Zone Query → MetaSound Selection
                                                          ↓
                                                    Audio Component
                                                          ↓
                                                    Audio Mixer
                                                          ↓
                                                    Audio Output
```

**Audio Data:**
- Sound Cues (asset references)
- Audio Parameters (volume, pitch, reverb)
- Spatial Data (location, attenuation)
- Zone Data (ambient sounds, reverb settings)

**Processing Pipeline:**
1. Event triggers audio request
2. Audio Subsystem queries active zones
3. MetaSound parameters calculated
4. Audio component spawned/updated
5. Audio mixer processes output

### 4. Evidence Collection Flow

```
Player Interaction → Evidence Actor → Evidence Collection Component
                                              ↓
                                    Archive Subsystem
                                              ↓
                                    ┌─────────┴─────────┐
                                    ↓                   ↓
                              Event Bus            Save Game
                                    ↓                   ↓
                                UI Update          Persistence
```

**Evidence Data Structure:**
```cpp
FEvidenceData {
    FName EvidenceId;           // Unique identifier
    FText DisplayName;          // Localized name
    FText Description;          // Localized description
    UTexture2D* Thumbnail;      // Preview image
    EEvidenceType Type;         // Document/Photo/Audio/Object
    TArray<uint8> ContentData;  // Actual evidence content
    FDateTime CollectedTime;    // When collected
    FVector CollectionLocation; // Where collected
}
```

**Storage:**
- Runtime: Archive Subsystem (TMap)
- Persistent: Save Game (serialized)
- Size: ~1-5 KB per evidence item

### 5. Encounter State Flow

```
Trigger Condition → Encounter Director → State Machine Update
                                              ↓
                                    ┌─────────┴─────────┐
                                    ↓                   ↓
                              Event Bus            Threat Spawn
                                    ↓                   ↓
                            Audio/UI/Save        AI Controller
```

**State Transitions:**
```
Dormant → Primed → Revealed → Resolved
   ↑                              ↓
   └──────────────────────────────┘
              Reset
```

**State Data:**
- Current Phase (enum)
- Encounter ID (FName)
- Threat Reference (TObjectPtr)
- Last Trigger Target (weak pointer)
- Timer Handles (for delayed reveals)

### 6. Save/Load Flow

**Save Flow:**
```
Game State → Save Game Object → Serialization → Disk Write
     ↓
Event Bus History
Evidence Archive
Player Progress
Encounter States
```

**Load Flow:**
```
Disk Read → Deserialization → Save Game Object → Game State Restoration
                                                        ↓
                                                  Subsystem Init
                                                        ↓
                                                  Actor Spawning
```

**Save Data Size:**
- Base Save: ~50 KB
- Evidence Data: ~5-50 KB (depending on collection)
- Event History: ~14 KB (128 events × 108 bytes)
- Total: ~70-120 KB per save slot

### 7. UI Update Flow

```
Game Event → Event Bus → UI Widget Subscriber → Widget Update
                                                      ↓
                                                Data Binding
                                                      ↓
                                                Slate Rendering
```

**UI Data Binding:**
- Direct property binding (for static data)
- Event-driven updates (for dynamic data)
- Polling (for performance metrics)

**Update Frequency:**
- HUD: Every frame (60 Hz)
- Objectives: Event-driven
- Archive: On-demand
- Performance: 1 Hz

## Data Persistence

### Transient Data (Runtime Only)
- Event Bus history
- Audio component states
- Animation states
- Particle system states
- Camera shake states

### Persistent Data (Saved)
- Evidence collection
- Encounter completion states
- Player progress markers
- Game settings
- Statistics

### Replicated Data (Future Multiplayer)
- Player position/rotation
- Encounter states
- Evidence collection (authority only)
- Audio triggers

## Memory Management

### Allocation Patterns

**Stack Allocation:**
- Event messages (temporary)
- Function parameters
- Local variables

**Heap Allocation:**
- UObjects (garbage collected)
- Evidence content data
- Audio buffers
- Texture data

**Memory Pools:**
- Audio components (pooled)
- Particle systems (pooled)
- UI widgets (cached)

### Garbage Collection

**UObject Lifecycle:**
1. Created (NewObject/SpawnActor)
2. Referenced (TObjectPtr/UPROPERTY)
3. Unreferenced (cleared pointers)
4. Marked for GC
5. Destroyed (next GC pass)

**GC Triggers:**
- Level transitions
- Manual GC calls
- Memory pressure
- Periodic (every 60 seconds)

## Performance Optimization

### Data Caching

**Subsystem Caching:**
```cpp
// Cache subsystem references
UHorrorEventBusSubsystem* EventBus = GetWorld()->GetSubsystem<UHorrorEventBusSubsystem>();
// Store in member variable, don't query every frame
```

**Asset Caching:**
- Preload frequently used assets
- Use asset manager for streaming
- Soft object references for large assets

### Data Compression

**Evidence Content:**
- Images: Compressed textures (BC7/ASTC)
- Audio: Vorbis compression
- Text: UTF-8 encoding

**Save Data:**
- Binary serialization (not JSON)
- Optional compression (zlib)
- Incremental saves (delta encoding)

### Batching

**Event Batching:**
- Collect events during frame
- Dispatch in batch at end of frame
- Reduces delegate overhead

**UI Updates:**
- Batch widget updates
- Invalidate once per frame
- Use Slate's invalidation system

## Data Validation

### Input Validation
- Sanitize player input
- Clamp numeric values
- Validate gameplay tags

### State Validation
- Check preconditions before state changes
- Validate object references
- Assert invariants in development builds

### Save Data Validation
- Version checking
- Checksum verification
- Graceful degradation for corrupted saves

## Debugging Data Flow

### Logging
```cpp
UE_LOG(LogHorrorProject, Log, TEXT("Event: %s from %s"), 
    *EventTag.ToString(), *SourceId.ToString());
```

### Visual Debugging
- Draw debug shapes for spatial data
- On-screen text for state information
- Gameplay debugger integration

### Profiling
- Insights for frame data
- Memory profiler for allocations
- Network profiler (future)

## Data Flow Diagrams

### Complete System Data Flow

```
┌─────────────┐
│   Player    │
└──────┬──────┘
       │ Input
       ↓
┌─────────────┐     ┌─────────────┐
│  Controller │────→│  Character  │
└──────┬──────┘     └──────┬──────┘
       │                   │
       │ Commands          │ Actions
       ↓                   ↓
┌─────────────────────────────────┐
│         Event Bus               │
└────┬────┬────┬────┬────┬────┬──┘
     │    │    │    │    │    │
     ↓    ↓    ↓    ↓    ↓    ↓
   Audio UI  AI  Save VFX Analytics
```

---
Last Updated: 2026-04-26
