# Objective Placement Guide

## Overview

Objectives are the core progression markers in horror levels. Proper placement ensures pacing, exploration, and player satisfaction.

## 8 Objective System

### Objective Types

1. **Tutorial Objectives**: Teach mechanics
2. **Exploration Objectives**: Reward curiosity
3. **Challenge Objectives**: Test skills
4. **Gate Objectives**: Control progression
5. **Crisis Objectives**: Peak tension moments
6. **Optional Objectives**: Extra content
7. **Narrative Objectives**: Story progression
8. **Completion Objectives**: Level exit

## Placement Principles

### 1. Visibility vs. Discovery

#### High Visibility (Objectives 1, 2, 8)
- Clear sightlines from main path
- Environmental cues (lighting, props)
- Audio hints
- Minimal searching required

#### Medium Visibility (Objectives 3, 5, 7)
- Requires exploration
- Subtle environmental hints
- Discoverable through gameplay
- Rewards attention

#### Low Visibility (Objectives 4, 6)
- Hidden or challenging to reach
- Optional or advanced content
- Requires thorough exploration
- High reward for discovery

### 2. Spatial Distribution

#### Horizontal Spread
- Minimum 30m between objectives
- Cover all major zones
- Avoid clustering
- Create exploration loops

#### Vertical Spread
- Use multiple floor levels
- Encourage vertical exploration
- Create memorable moments
- Vary perspective

### 3. Timing Distribution

| Objective | Target Time | Cumulative | Beat |
|-----------|-------------|------------|------|
| 1 | 2-5 min | 5 min | 1 |
| 2 | 5-8 min | 13 min | 2 |
| 3 | 8-13 min | 18 min | 2-3 |
| 4 | Optional | +3 min | 4 |
| 5 | 13-18 min | 23 min | 3-4 |
| 6 | 23-28 min | 32 min | 5 |
| 7 | Optional | +2 min | 6 |
| 8 | 32-35 min | 35 min | 7 |

## Detailed Objective Specifications

### Objective 1: "Find the Keycard"

#### Purpose
- Introduce interaction mechanics
- Establish baseline tension
- Orient player to environment
- Begin narrative

#### Placement Requirements
- **Zone**: Entry Hall
- **Distance from Spawn**: 10-20m
- **Visibility**: High (direct sightline)
- **Height**: Ground level
- **Lighting**: Well-lit (150+ lux)

#### Environmental Setup
- Clear path from spawn
- Reception desk or similar landmark
- Tutorial prompts nearby
- No threats present

#### Interaction Design
- Simple pickup (E key)
- Visual feedback (glow, outline)
- Audio cue (pickup sound)
- UI confirmation

#### Success Criteria
- [ ] Completable within 5 minutes
- [ ] No player confusion
- [ ] Clear next objective hint
- [ ] Teaches basic interaction

#### Blueprint Implementation
```
BP_Objective_Keycard
- Mesh: SM_Keycard
- Collision: Overlap trigger
- Highlight: Enabled
- Tutorial: "Press E to pick up"
```

### Objective 2: "Restore Power"

#### Purpose
- Introduce puzzle mechanics
- Change environment state
- Unlock new areas
- Build tension

#### Placement Requirements
- **Zone**: Main Corridor
- **Distance from Obj 1**: 40-50m
- **Visibility**: Medium (requires exploration)
- **Height**: Wall-mounted (1.5m)
- **Lighting**: Dim (50-100 lux)

#### Environmental Setup
- Electrical panel on wall
- Sparking wires nearby
- Flashlight recommended
- Environmental hazards

#### Interaction Design
- Multi-step puzzle (3 switches)
- Sequence matters
- Visual feedback (lights turn on)
- Audio cue (power surge)

#### Success Criteria
- [ ] Completable within 8 minutes
- [ ] Puzzle logic clear
- [ ] Environmental change obvious
- [ ] Unlocks new paths

#### Blueprint Implementation
```
BP_Objective_PowerRestore
- Components: 3x BP_CircuitBreaker
- Logic: Sequence validation
- Effect: Toggle level lighting
- Feedback: Sparks, sound, lights
```

### Objective 3: "Collect 3 Documents"

#### Purpose
- Encourage exploration
- Deliver narrative
- Reward thoroughness
- Teach collection mechanics

#### Placement Requirements
- **Zone**: West Wing (3 locations)
- **Distance from Obj 2**: 30-60m (total)
- **Visibility**: Medium (scattered)
- **Height**: Various (desk, floor, shelf)
- **Lighting**: Dark with pockets (30-80 lux)

#### Environmental Setup
- Office spaces with desks
- Filing cabinets
- Readable documents
- Evidence markers

#### Interaction Design
- Pickup and read
- Progress tracker (1/3, 2/3, 3/3)
- Optional reading
- Lore rewards

#### Success Criteria
- [ ] Completable within 13 minutes
- [ ] Documents findable without frustration
- [ ] Narrative coherent
- [ ] Exploration rewarded

#### Blueprint Implementation
```
BP_Objective_DocumentCollection
- Array: 3x BP_Document
- Tracker: CollectedCount
- UI: Progress widget
- Completion: All 3 collected
```

### Objective 4: "Unlock Safe" (Optional)

#### Purpose
- Reward exploration
- Provide extra resources
- Optional challenge
- Lore depth

#### Placement Requirements
- **Zone**: West Wing Upper (Manager's Office)
- **Distance from Obj 3**: 20-30m vertical
- **Visibility**: Low (hidden room)
- **Height**: Floor level (safe)
- **Lighting**: Very dark (20-40 lux)

#### Environmental Setup
- Hidden office (requires key or puzzle)
- Safe in corner
- Code hints scattered
- High-value loot

#### Interaction Design
- Code entry (4 digits)
- Hints in environment
- Trial and error allowed
- Reward: Resources + evidence

#### Success Criteria
- [ ] Optional (not required for completion)
- [ ] Code discoverable through exploration
- [ ] Reward worth effort
- [ ] Adds 3-5 minutes to playthrough

#### Blueprint Implementation
```
BP_Objective_SafeUnlock
- Widget: Keypad UI
- Code: 4-digit validation
- Hints: Environmental clues
- Reward: Spawn loot on success
```

### Objective 5: "Find the Specimen"

#### Purpose
- First major challenge
- Introduce enemy threat
- Test learned skills
- Escalate tension

#### Placement Requirements
- **Zone**: East Wing (Morgue)
- **Distance from Obj 3**: 50-70m
- **Visibility**: Medium (clear goal, dangerous path)
- **Height**: Ground level (morgue drawer)
- **Lighting**: High contrast (20-150 lux)

#### Environmental Setup
- Morgue with examination tables
- Enemy patrol route nearby
- Cover objects available
- Multiple approach paths

#### Interaction Design
- Stealth or combat approach
- Drawer opening (loud)
- Enemy alert possible
- Quick escape needed

#### Success Criteria
- [ ] Completable within 23 minutes
- [ ] Enemy encounter fair
- [ ] Multiple strategies viable
- [ ] Tension peak achieved

#### Blueprint Implementation
```
BP_Objective_Specimen
- Trigger: Morgue drawer
- Enemy: Patrol nearby
- Alert: Sound-based detection
- Escape: Multiple exits
```

### Objective 6: "Destroy the Source"

#### Purpose
- Crisis point
- Peak tension
- Major challenge
- Narrative climax

#### Placement Requirements
- **Zone**: Basement (Ritual Chamber)
- **Distance from Obj 5**: 40-60m + vertical
- **Visibility**: Low (deep in basement)
- **Height**: Below ground level
- **Lighting**: Very dark (10-50 lux)

#### Environmental Setup
- Large ritual chamber
- Central altar/object
- Multiple enemies possible
- Limited escape routes

#### Interaction Design
- Combat or puzzle hybrid
- Multi-stage destruction
- Enemy waves or boss
- High stakes

#### Success Criteria
- [ ] Completable within 32 minutes
- [ ] Challenge appropriate for skill level
- [ ] Memorable moment
- [ ] Clear success feedback

#### Blueprint Implementation
```
BP_Objective_DestroySource
- Phases: 3 stages
- Enemies: Wave spawner
- Destruction: Health-based object
- Completion: Trigger level change
```

### Objective 7: "Retrieve the Key" (Optional)

#### Purpose
- Unlock shortcut
- Reward exploration
- Optional optimization
- Lore completion

#### Placement Requirements
- **Zone**: Upper Floor (Archives)
- **Distance from Obj 6**: 50-80m + vertical
- **Visibility**: Medium (requires backtracking)
- **Height**: Upper level
- **Lighting**: Dark (20-60 lux)

#### Environmental Setup
- Archive room with shelves
- Key on desk or in drawer
- Lore documents nearby
- Shortcut door visible

#### Interaction Design
- Simple pickup
- Unlocks shortcut to exit
- Saves 5 minutes
- Optional but valuable

#### Success Criteria
- [ ] Optional (not required)
- [ ] Shortcut benefit clear
- [ ] Worth backtracking
- [ ] Adds 2-3 minutes if pursued

#### Blueprint Implementation
```
BP_Objective_ShortcutKey
- Pickup: Key item
- Unlock: Exit shortcut door
- Benefit: Skip final basement
- Hint: Environmental clue
```

### Objective 8: "Escape"

#### Purpose
- Level completion
- Tension release
- Final challenge
- Satisfaction

#### Placement Requirements
- **Zone**: Exit Sequence
- **Distance from Obj 6**: 60-80m
- **Visibility**: High (clear goal)
- **Height**: Ground level
- **Lighting**: Chaotic (30-100 lux)

#### Environmental Setup
- Clear exit door
- Collapsing environment
- Final enemy encounter (optional)
- Escape timer (optional)

#### Interaction Design
- Door interaction
- Possible final challenge
- Completion trigger
- Victory sequence

#### Success Criteria
- [ ] Completable within 35 minutes
- [ ] Clear and satisfying
- [ ] Final challenge fair
- [ ] Proper closure

#### Blueprint Implementation
```
BP_Objective_Escape
- Trigger: Exit door
- Challenge: Optional enemy/timer
- Completion: Level end
- Feedback: Victory screen
```

## Placement Workflow

### Step 1: Paper Design
1. Draw level layout
2. Mark 7 beats zones
3. Place 8 objectives on map
4. Verify distribution
5. Calculate distances

### Step 2: Blockout Placement
1. Use placeholder meshes
2. Place at approximate locations
3. Test navigation between objectives
4. Adjust based on playtest
5. Verify timing

### Step 3: Refinement
1. Replace with final assets
2. Add environmental cues
3. Implement interactions
4. Polish feedback
5. Final playtesting

### Step 4: Validation
1. Run `ValidateLevelLayout.ps1`
2. Check objective spacing
3. Verify timing targets
4. Test all paths
5. Document any issues

## Environmental Cues

### Visual Cues
- **Lighting**: Highlight objective areas
- **Color**: Use distinct colors (yellow glow)
- **Composition**: Frame objectives in sightlines
- **Props**: Contextual objects guide attention

### Audio Cues
- **Ambient**: Unique sounds near objectives
- **Music**: Subtle musical cues
- **SFX**: Interaction sounds
- **Voiceover**: Optional hints

### Spatial Cues
- **Landmarks**: Memorable locations
- **Sightlines**: Clear views to objectives
- **Paths**: Natural flow toward goals
- **Contrast**: Objectives stand out

## Common Mistakes

### 1. Clustering
- **Problem**: Multiple objectives too close
- **Solution**: Minimum 30m spacing
- **Tool**: `ValidateLevelLayout.ps1`

### 2. Hidden Objectives
- **Problem**: Players can't find objectives
- **Solution**: Add environmental cues
- **Test**: Blind playtesting

### 3. Poor Timing
- **Problem**: Objectives too fast or slow
- **Solution**: Follow timing table
- **Tool**: `TestObjectiveFlow.ps1`

### 4. Unclear Interactions
- **Problem**: Players don't know what to do
- **Solution**: Clear UI prompts
- **Test**: First-time player observation

### 5. Backtracking Hell
- **Problem**: Too much backtracking required
- **Solution**: Forward progression design
- **Tool**: Route analysis

### 6. No Rewards
- **Problem**: Objectives feel pointless
- **Solution**: Meaningful rewards (story, items, progression)
- **Design**: Every objective unlocks something

## Testing Checklist

### Per-Objective Tests
- [ ] Reachable from previous objective
- [ ] Completable without external help
- [ ] Timing within target range
- [ ] Interaction clear and responsive
- [ ] Feedback satisfying
- [ ] Rewards appropriate

### System Tests
- [ ] All 8 objectives present
- [ ] Proper spacing maintained
- [ ] Timing totals to 35 minutes
- [ ] Optional objectives skippable
- [ ] No progression blockers
- [ ] Performance stable at each objective

### Player Experience Tests
- [ ] First-time players complete without frustration
- [ ] Experienced players can speedrun
- [ ] Exploration rewarded
- [ ] Tension builds appropriately
- [ ] Satisfaction at completion

## Blueprint Reference

### Base Objective Class
```
BP_ObjectiveBase (Parent Class)
├── Variables
│   ├── ObjectiveID (int)
│   ├── ObjectiveName (string)
│   ├── IsOptional (bool)
│   ├── IsCompleted (bool)
│   └── CompletionReward (struct)
├── Functions
│   ├── OnObjectiveActivated()
│   ├── OnObjectiveCompleted()
│   ├── UpdateObjectiveUI()
│   └── GiveReward()
└── Components
    ├── TriggerVolume
    ├── HighlightMesh
    └── AudioComponent
```

### Objective Manager
```
BP_ObjectiveManager (Level Blueprint)
├── Variables
│   ├── ObjectiveArray (array)
│   ├── CurrentObjective (int)
│   ├── CompletedCount (int)
│   └── TotalTime (float)
├── Functions
│   ├── InitializeObjectives()
│   ├── ActivateNextObjective()
│   ├── CheckCompletion()
│   └── GenerateReport()
└── Events
    ├── OnLevelStart
    ├── OnObjectiveComplete
    └── OnLevelComplete
```

## Performance Considerations

### Per-Objective Budget
- **Draw Calls**: +50-100
- **Triangles**: +10K-20K
- **Lights**: +1-2 dynamic
- **Audio**: +1-2 sources
- **Particles**: +1-2 emitters (if needed)

### Optimization Tips
1. Use instanced meshes for objective markers
2. Pool objective UI widgets
3. Disable inactive objective highlights
4. Use LODs for objective props
5. Optimize trigger volumes (simple collision)

## Tools Integration

### Generation
```powershell
.\PlaceObjectiveNodes.ps1 -LevelName "SM13" -ObjectiveCount 8
```

### Validation
```powershell
.\ValidateLevelLayout.ps1 -CheckObjectives -CheckSpacing -CheckTiming
```

### Testing
```powershell
.\TestObjectiveFlow.ps1 -RunFullTest -GenerateReport
```

## Documentation Template

### Per-Objective Documentation
```markdown
## Objective [ID]: "[Name]"

**Type**: [Tutorial/Exploration/Challenge/Gate/Crisis/Optional/Narrative/Completion]
**Zone**: [Zone Name]
**Beat**: [1-7]
**Time**: [Target minutes]
**Optional**: [Yes/No]

### Location
- Coordinates: (X, Y, Z)
- Distance from previous: [meters]
- Height: [Ground/Elevated/Below]

### Requirements
- Previous objectives: [List]
- Items needed: [List]
- Skills needed: [List]

### Interaction
- Type: [Pickup/Puzzle/Combat/Stealth]
- Steps: [Description]
- Feedback: [Visual/Audio/UI]

### Rewards
- Progression: [What unlocks]
- Items: [What player receives]
- Narrative: [Story beats]

### Notes
- [Design notes]
- [Known issues]
- [Future improvements]
```

## Version Control

Track objective changes in level design document:
- Position changes
- Timing adjustments
- Interaction modifications
- Reward balancing
- Bug fixes

## Resources

- Level Design Document: `LevelDesignDocument.md`
- SM13 Route Design: `SM13_RouteDesign.md`
- Testing Tools: `Tools/TestObjectiveFlow.ps1`
- Validation Tools: `Tools/ValidateLevelLayout.ps1`
