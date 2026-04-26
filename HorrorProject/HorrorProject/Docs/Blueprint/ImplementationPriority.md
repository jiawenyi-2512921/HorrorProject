# Blueprint Implementation Priority

## Overview

This document prioritizes Blueprint implementation tasks to ensure efficient development. Follow this order to minimize blocking dependencies and maximize parallel work.

## Priority Levels

- **Priority 0 (Blocking)** - Must be completed first, blocks other work
- **Priority 1 (Critical Path)** - Core gameplay features, high impact
- **Priority 2 (Enhancement)** - Polish and secondary features
- **Priority 3 (Optional)** - Nice-to-have, future optimization

---

## Priority 0: Blocking (Complete First)

These assets and systems must be created before other work can proceed.

### 1. Player Character Setup
**Estimated Time**: 2 hours

- [ ] Create BP_PlayerCharacter (inherits from HorrorProjectCharacter)
- [ ] Configure input mappings (IMC_Player, all Input Actions)
- [ ] Add basic movement and camera control
- [ ] Test in empty test map

**Why Priority 0**: Everything depends on having a functional player character.

**Blocks**: All other systems require player character to test.

### 2. Core UI Framework
**Estimated Time**: 3 hours

- [ ] Create BP_UIManager actor
- [ ] Create WBP_BodycamOverlay (basic version, no VFX yet)
- [ ] Implement basic HUD display
- [ ] Test UI shows on screen

**Why Priority 0**: UI framework needed for all system feedback.

**Blocks**: All systems need UI for player feedback.

### 3. Basic Interaction System
**Estimated Time**: 4 hours

- [ ] Create BP_BaseInteractable
- [ ] Create WBP_InteractionPrompt
- [ ] Add InteractionManager component to player
- [ ] Create one test interactable (BP_DocumentInteractable)
- [ ] Test interaction works

**Why Priority 0**: Interaction is core to all gameplay.

**Blocks**: Evidence, objectives, and most gameplay features.

### 4. Test Environment
**Estimated Time**: 2 hours

- [ ] Create TestMap_Main with basic geometry
- [ ] Add lighting
- [ ] Add player start
- [ ] Place test interactables
- [ ] Verify playable

**Why Priority 0**: Need environment to test all systems.

**Blocks**: Cannot test systems without environment.

**Total Priority 0 Time**: ~11 hours

---

## Priority 1: Critical Path (Core Gameplay)

These features are essential for the core gameplay loop.

### 5. Camera System
**Estimated Time**: 6 hours

- [ ] Add CameraPhotoComponent to player
- [ ] Add CameraRecordingComponent to player
- [ ] Add CameraBatteryComponent to player
- [ ] Create camera input bindings
- [ ] Create BP_CameraFlash effect
- [ ] Create WBP_BatteryIndicator
- [ ] Test photo capture works
- [ ] Test battery drains correctly

**Why Priority 1**: Camera is core mechanic for horror experience.

**Dependencies**: Player character (P0), UI framework (P0)

### 6. Evidence System
**Estimated Time**: 8 hours

- [ ] Create BP_EvidenceActor_Base
- [ ] Create BP_EvidenceActor_Document
- [ ] Create BP_EvidenceActor_Photo
- [ ] Create BP_EvidenceActor_Audio
- [ ] Add EvidenceCollectionComponent to player
- [ ] Create WBP_EvidenceToast
- [ ] Create WBP_ArchiveMenu (basic version)
- [ ] Place 5-10 test evidence in level
- [ ] Test evidence collection works
- [ ] Test archive displays evidence

**Why Priority 1**: Evidence collection is primary gameplay objective.

**Dependencies**: Interaction system (P0), UI framework (P0)

### 7. Objective System
**Estimated Time**: 8 hours

- [ ] Create BP_ObjectiveNode_Base
- [ ] Create BP_ObjectiveNode_Navigation
- [ ] Create BP_ObjectiveNode_Interaction
- [ ] Create BP_ObjectiveManager
- [ ] Create WBP_ObjectiveList
- [ ] Create WBP_ObjectiveToast
- [ ] Create BP_WaypointMarker
- [ ] Create 3-5 test objectives
- [ ] Test objective activation/completion
- [ ] Test objective UI updates

**Why Priority 1**: Objectives guide player through game.

**Dependencies**: Interaction system (P0), UI framework (P0)

### 8. Door System
**Estimated Time**: 4 hours

- [ ] Create BP_DoorInteractable
- [ ] Create BP_Door_Standard
- [ ] Create BP_Door_Sliding
- [ ] Create door open/close sounds (SC_Door_Open, SC_Door_Close, SC_Door_Locked)
- [ ] Place 5-10 doors in test level
- [ ] Test doors open/close
- [ ] Test locked doors work

**Why Priority 1**: Doors are essential for level navigation.

**Dependencies**: Interaction system (P0)

### 9. Audio System - Essential
**Estimated Time**: 6 hours

- [ ] Add FootstepAudioComponent to player
- [ ] Add BreathingAudioComponent to player
- [ ] Create footstep sound cues (5 surface types)
- [ ] Create breathing sound cues (4 types)
- [ ] Create sound attenuation assets
- [ ] Test footsteps play on different surfaces
- [ ] Test breathing responds to stress

**Why Priority 1**: Audio is critical for horror atmosphere.

**Dependencies**: Player character (P0)

### 10. Save/Load System
**Estimated Time**: 4 hours

- [ ] Implement evidence save/load
- [ ] Implement objective save/load
- [ ] Implement camera photo save/load
- [ ] Implement battery level save/load
- [ ] Test save/load works correctly

**Why Priority 1**: Players need to save progress.

**Dependencies**: Evidence system (P1), Objective system (P1), Camera system (P1)

**Total Priority 1 Time**: ~36 hours

**Cumulative Time**: ~47 hours (Priority 0 + Priority 1)

---

## Priority 2: Enhancement (Polish & Secondary Features)

These features enhance the experience but aren't blocking.

### 11. VFX System - Post Process
**Estimated Time**: 8 hours

- [ ] Create all 9 post-process materials
- [ ] Create BP_VFXManager
- [ ] Add PostProcessController component to player
- [ ] Configure post-process volume
- [ ] Test VHS effect
- [ ] Test underwater effect
- [ ] Test fear effect
- [ ] Test death effect

**Why Priority 2**: VFX enhances atmosphere but not required for core gameplay.

**Dependencies**: Player character (P0)

### 12. VFX System - Particles
**Estimated Time**: 8 hours

- [ ] Create all 8 Niagara particle systems
- [ ] Add ParticleSpawner component to player
- [ ] Test underwater debris
- [ ] Test blood splatter
- [ ] Test water drips
- [ ] Test spark shower
- [ ] Optimize particle budgets

**Why Priority 2**: Particles add visual polish.

**Dependencies**: VFX Manager (P2)

### 13. VFX System - Camera Shakes
**Estimated Time**: 4 hours

- [ ] Create all 6 camera shake classes
- [ ] Add ScreenEffectManager component to player
- [ ] Test impact shake
- [ ] Test fear shake
- [ ] Test death shake

**Why Priority 2**: Camera shakes enhance feedback.

**Dependencies**: VFX Manager (P2)

### 14. Advanced Interactables
**Estimated Time**: 6 hours

- [ ] Create BP_ExaminableInteractable
- [ ] Create WBP_ExamineUI
- [ ] Create BP_PickupInteractable
- [ ] Create BP_Pickup_Battery
- [ ] Create BP_Pickup_Key
- [ ] Create BP_RecorderInteractable
- [ ] Create WBP_RecordingPlayer
- [ ] Create BP_SwitchInteractable
- [ ] Test all interactable types

**Why Priority 2**: Adds variety to interactions.

**Dependencies**: Interaction system (P0)

### 15. Photo Gallery System
**Estimated Time**: 4 hours

- [ ] Create WBP_PhotoGallery
- [ ] Create WBP_PhotoThumbnail
- [ ] Implement photo viewing
- [ ] Implement photo deletion
- [ ] Test gallery works

**Why Priority 2**: Nice feature but not essential.

**Dependencies**: Camera system (P1)

### 16. Battery Charging System
**Estimated Time**: 3 hours

- [ ] Create BP_BatteryChargingStation
- [ ] Create charging station mesh/materials
- [ ] Place charging stations in level
- [ ] Test charging works

**Why Priority 2**: Adds resource management depth.

**Dependencies**: Camera system (P1)

### 17. Audio System - Ambient
**Estimated Time**: 6 hours

- [ ] Create BP_AudioManager
- [ ] Create BP_AudioZone_Station
- [ ] Create BP_AudioZone_Underwater
- [ ] Create ambient sound cues (5 types)
- [ ] Create sound mix modifiers
- [ ] Place audio zones in level
- [ ] Test ambient audio layers

**Why Priority 2**: Enhances atmosphere.

**Dependencies**: Audio system essential (P1)

### 18. Pause Menu & Settings
**Estimated Time**: 6 hours

- [ ] Create WBP_PauseMenu
- [ ] Create WBP_SettingsMenu
- [ ] Implement pause functionality
- [ ] Implement settings (graphics, audio, controls)
- [ ] Test pause/resume works
- [ ] Test settings save/load

**Why Priority 2**: Important for player comfort.

**Dependencies**: UI framework (P0)

### 19. Objective System - Advanced
**Estimated Time**: 4 hours

- [ ] Create BP_ObjectiveNode_Encounter
- [ ] Create encounter objective variants
- [ ] Create multi-stage objectives
- [ ] Create branching objectives
- [ ] Test advanced objective types

**Why Priority 2**: Adds objective variety.

**Dependencies**: Objective system (P1)

### 20. Evidence System - Advanced
**Estimated Time**: 4 hours

- [ ] Create WBP_EvidenceDetailViewer
- [ ] Create WBP_DocumentViewer
- [ ] Implement evidence search/filtering
- [ ] Implement evidence-based progression
- [ ] Test advanced evidence features

**Why Priority 2**: Enhances evidence system.

**Dependencies**: Evidence system (P1)

**Total Priority 2 Time**: ~53 hours

**Cumulative Time**: ~100 hours (Priority 0 + Priority 1 + Priority 2)

---

## Priority 3: Optional (Future Optimization)

These features are nice-to-have and can be added later.

### 21. Tutorial System
**Estimated Time**: 4 hours

- [ ] Create BP_InteractionTutorial
- [ ] Create tutorial UI widgets
- [ ] Implement tutorial steps
- [ ] Test tutorial flow

**Why Priority 3**: Can be added after core gameplay is solid.

**Dependencies**: All core systems (P0-P1)

### 22. Achievement System
**Estimated Time**: 3 hours

- [ ] Create achievement tracking
- [ ] Create achievement UI
- [ ] Implement achievement triggers
- [ ] Test achievements unlock

**Why Priority 3**: Nice feature but not essential.

**Dependencies**: All core systems (P0-P1)

### 23. Photo-Based Puzzles
**Estimated Time**: 4 hours

- [ ] Create BP_PhotoPuzzle
- [ ] Implement photo verification
- [ ] Create puzzle variants
- [ ] Test puzzles work

**Why Priority 3**: Advanced gameplay feature.

**Dependencies**: Camera system (P1), Evidence system (P1)

### 24. Advanced Audio Features
**Estimated Time**: 4 hours

- [ ] Implement audio occlusion
- [ ] Implement reverb zones
- [ ] Implement dynamic audio mixing
- [ ] Optimize audio performance

**Why Priority 3**: Polish for audio system.

**Dependencies**: Audio system (P1-P2)

### 25. Debug Tools
**Estimated Time**: 3 hours

- [ ] Create BP_ObjectiveDebugger
- [ ] Create EUW_EvidencePlacementTool
- [ ] Create EUW_ObjectivePlacementTool
- [ ] Create debug console commands

**Why Priority 3**: Helpful for development but not player-facing.

**Dependencies**: All systems

### 26. Performance Optimization
**Estimated Time**: 8 hours

- [ ] Profile all systems
- [ ] Optimize VFX performance
- [ ] Optimize audio performance
- [ ] Optimize UI performance
- [ ] Implement LOD systems
- [ ] Optimize particle budgets

**Why Priority 3**: Do after core features are complete.

**Dependencies**: All systems implemented

### 27. Accessibility Features
**Estimated Time**: 6 hours

- [ ] Implement colorblind modes
- [ ] Implement text size options
- [ ] Implement subtitle system
- [ ] Implement control remapping
- [ ] Test accessibility features

**Why Priority 3**: Important but can be added later.

**Dependencies**: All core systems (P0-P1)

### 28. Advanced VFX
**Estimated Time**: 6 hours

- [ ] Create advanced post-process effects
- [ ] Create custom particle effects
- [ ] Implement dynamic weather effects
- [ ] Implement advanced lighting effects

**Why Priority 3**: Visual polish.

**Dependencies**: VFX system (P2)

**Total Priority 3 Time**: ~38 hours

**Total All Priorities**: ~138 hours

---

## Implementation Schedule

### Week 1: Foundation (Priority 0)
**Goal**: Playable character in test environment with basic interaction

- Day 1-2: Player character setup, input mappings
- Day 3: Core UI framework
- Day 4-5: Basic interaction system
- Day 5: Test environment

**Deliverable**: Playable test level with working interaction

### Week 2-3: Core Gameplay (Priority 1 Part 1)
**Goal**: Camera, evidence, and objective systems working

- Day 6-7: Camera system
- Day 8-10: Evidence system
- Day 11-13: Objective system

**Deliverable**: Can take photos, collect evidence, complete objectives

### Week 3-4: Core Gameplay (Priority 1 Part 2)
**Goal**: Doors, audio, and save/load working

- Day 14-15: Door system
- Day 16-17: Audio system essential
- Day 18-19: Save/load system

**Deliverable**: Complete core gameplay loop

### Week 5-6: Polish (Priority 2 Part 1)
**Goal**: VFX and advanced interactables

- Day 20-22: VFX system (post-process, particles, shakes)
- Day 23-24: Advanced interactables
- Day 25: Photo gallery

**Deliverable**: Polished visual experience

### Week 6-7: Polish (Priority 2 Part 2)
**Goal**: Complete secondary features

- Day 26: Battery charging
- Day 27-28: Audio system ambient
- Day 29-30: Pause menu & settings
- Day 31: Advanced objectives
- Day 32: Advanced evidence

**Deliverable**: Feature-complete game

### Week 8+: Optional Features (Priority 3)
**Goal**: Add nice-to-have features as time permits

- Tutorial system
- Achievement system
- Photo puzzles
- Advanced audio
- Debug tools
- Performance optimization
- Accessibility features
- Advanced VFX

**Deliverable**: Fully polished game

---

## Parallel Work Opportunities

These tasks can be done simultaneously by different team members:

### Parallel Track A: Gameplay Systems
- Player character setup
- Interaction system
- Objective system
- Evidence system

### Parallel Track B: Visual Systems
- VFX system
- UI system
- Materials and textures

### Parallel Track C: Audio Systems
- Audio system
- Sound asset creation
- Audio zones

### Parallel Track D: Content Creation
- Level design
- Evidence placement
- Objective placement
- Interactable placement

---

## Critical Path

The absolute minimum to have a playable game:

1. Player character (P0) - 2 hours
2. Basic interaction (P0) - 4 hours
3. Test environment (P0) - 2 hours
4. Camera system (P1) - 6 hours
5. Evidence system (P1) - 8 hours
6. Objective system (P1) - 8 hours
7. Door system (P1) - 4 hours
8. Basic audio (P1) - 6 hours

**Minimum Viable Product Time**: ~40 hours

---

## Risk Mitigation

### High Risk Items
- Camera photo capture (complex rendering)
- Save/load system (data persistence)
- VFX performance (GPU intensive)

### Mitigation Strategies
- Prototype camera system early
- Test save/load frequently
- Profile VFX performance continuously
- Have fallback options for complex features

---

## Success Criteria

### Priority 0 Complete
- [ ] Player can move and interact
- [ ] UI displays on screen
- [ ] Test environment is playable

### Priority 1 Complete
- [ ] Player can take photos
- [ ] Player can collect evidence
- [ ] Player can complete objectives
- [ ] Doors open/close
- [ ] Audio plays correctly
- [ ] Progress saves/loads

### Priority 2 Complete
- [ ] VFX effects work
- [ ] All interactable types work
- [ ] Photo gallery works
- [ ] Pause menu works
- [ ] Settings work

### Priority 3 Complete
- [ ] Tutorial works
- [ ] Achievements work
- [ ] Performance is optimized
- [ ] Accessibility features work

---

## Notes

- Estimated times are for experienced Blueprint developers
- Add 50% buffer for unexpected issues
- Test frequently to catch issues early
- Prioritize player feedback over feature completion
- Be prepared to cut Priority 3 features if time is limited
- Focus on making Priority 0-1 features excellent rather than adding more features
