# SM13 Route Design - 1300m² Level Layout

## Overview

SM13 is a medium-sized horror level (1300m²) designed for 35-minute gameplay sessions. The layout follows a hub-and-spoke pattern with interconnected loops.

## Spatial Breakdown

### Total Area: 1300m²

#### Zone Distribution

1. **Entry Hall** (150m² - 11.5%)
   - Spawn point
   - Tutorial area
   - Objective 1
   - Safe zone

2. **Main Corridor** (200m² - 15.4%)
   - Central hub
   - Connects all zones
   - Multiple branching paths
   - Objective 2

3. **West Wing** (250m² - 19.2%)
   - Exploration area
   - Evidence-rich
   - Objectives 3-4
   - Mixed safety

4. **East Wing** (250m² - 19.2%)
   - Danger zone
   - Enemy encounters
   - Objective 5
   - High tension

5. **Basement Level** (200m² - 15.4%)
   - Vertical exploration
   - Crisis point
   - Objective 6
   - Maximum danger

6. **Upper Floor** (150m² - 11.5%)
   - Optional area
   - Secrets and evidence
   - Objective 7
   - Shortcut access

7. **Exit Sequence** (100m² - 7.7%)
   - Final challenge
   - Objective 8
   - Escape route
   - Tension release

## Route Design

### Primary Path (Required - 25 minutes)

```
Entry Hall → Main Corridor → West Wing → East Wing → Basement → Exit Sequence
```

**Distance**: ~180m walking
**Objectives**: 1, 2, 3, 5, 6, 8 (6 required)
**Pacing**: Linear with controlled branching

### Secondary Path (Optional - +10 minutes)

```
Main Corridor → Upper Floor → West Wing (upper) → Basement (alternate) → Exit
```

**Distance**: +60m walking
**Objectives**: 4, 7 (2 optional)
**Rewards**: Extra evidence, resources, lore

### Speedrun Path (Minimum - 15 minutes)

```
Entry → Main Corridor → East Wing → Basement → Exit
```

**Distance**: ~120m walking
**Objectives**: 1, 2, 6, 8 (4 minimum required)
**Note**: Skips optional content

## Detailed Zone Design

### 1. Entry Hall (150m²)

#### Layout
- **Dimensions**: 10m × 15m main room
- **Height**: 4m ceiling
- **Connections**: 1 entrance, 2 exits

#### Features
- Reception desk (cover)
- Waiting area (safe zone)
- Information board (tutorial)
- Locked door (requires progression)

#### Objective 1: "Find the Keycard"
- **Location**: Reception desk
- **Type**: Simple pickup
- **Time**: 2 minutes
- **Tutorial**: Movement, interaction

#### Lighting
- Overhead fluorescents (flickering)
- Emergency exit signs
- Desk lamp (warm)
- **Lux Level**: 150-200 (well-lit)

#### Audio
- Ambient hum
- Distant sounds
- Paper rustling
- Fluorescent buzz

#### Performance
- Draw Calls: ~300
- Triangles: ~150K
- Lights: 8 static, 2 dynamic

### 2. Main Corridor (200m²)

#### Layout
- **Dimensions**: 40m × 5m hallway
- **Height**: 3.5m ceiling
- **Connections**: 6 doorways, 2 staircases

#### Features
- Branching paths (4 directions)
- Environmental hazards (sparking wires)
- Cover objects (crates, furniture)
- Shortcut doors (unlock later)

#### Objective 2: "Restore Power"
- **Location**: Electrical panel (mid-corridor)
- **Type**: Interaction puzzle
- **Time**: 3 minutes
- **Mechanic**: Circuit breaker sequence

#### Lighting
- Wall sconces (half broken)
- Emergency lighting (red)
- Flashlight required
- **Lux Level**: 50-100 (dim)

#### Audio
- Electrical sparking
- Distant footsteps
- Wind through vents
- Creaking doors

#### Performance
- Draw Calls: ~500
- Triangles: ~250K
- Lights: 12 static, 3 dynamic

### 3. West Wing (250m²)

#### Layout
- **Dimensions**: Multiple rooms (5-8m each)
- **Height**: 3m ceiling
- **Connections**: Corridor, Upper Floor, Basement

#### Features
- Office spaces (evidence-rich)
- Storage rooms (resources)
- Bathroom (safe zone)
- Collapsed section (navigation challenge)

#### Objective 3: "Collect 3 Documents"
- **Location**: Scattered across offices
- **Type**: Collection quest
- **Time**: 5 minutes
- **Mechanic**: Exploration reward

#### Objective 4: "Unlock Safe" (Optional)
- **Location**: Manager's office (upper)
- **Type**: Code puzzle
- **Time**: 3 minutes
- **Reward**: Extra evidence, resources

#### Lighting
- Desk lamps (few working)
- Window light (moonlight)
- Candles (player-placed)
- **Lux Level**: 30-80 (dark with pockets)

#### Audio
- Paper shuffling
- Clock ticking
- Distant groaning
- Window rattling

#### Performance
- Draw Calls: ~600
- Triangles: ~300K
- Lights: 15 static, 4 dynamic

### 4. East Wing (250m²)

#### Layout
- **Dimensions**: Long corridor with side rooms
- **Height**: 3m ceiling
- **Connections**: Corridor, Basement, Upper Floor

#### Features
- Medical rooms (horror elements)
- Examination rooms (enemy spawns)
- Morgue (crisis point)
- Narrow passages (tension)

#### Objective 5: "Find the Specimen"
- **Location**: Morgue
- **Type**: Stealth/combat challenge
- **Time**: 6 minutes
- **Mechanic**: Enemy encounter

#### Lighting
- Surgical lights (harsh)
- Flickering fluorescents
- Darkness in corners
- **Lux Level**: 20-150 (high contrast)

#### Audio
- Medical equipment beeping
- Dripping water
- Enemy sounds
- Heartbeat (player stress)

#### Performance
- Draw Calls: ~700
- Triangles: ~350K
- Lights: 10 static, 5 dynamic
- AI: 2-3 active

### 5. Basement Level (200m²)

#### Layout
- **Dimensions**: Irregular, maze-like
- **Height**: 2.5m ceiling (claustrophobic)
- **Connections**: 2 staircases, 1 ladder

#### Features
- Boiler room (heat hazard)
- Storage areas (limited visibility)
- Flooded sections (slow movement)
- Ritual chamber (objective 6)

#### Objective 6: "Destroy the Source"
- **Location**: Ritual chamber
- **Type**: Combat/puzzle hybrid
- **Time**: 8 minutes
- **Mechanic**: Peak tension moment

#### Lighting
- Single bulbs (sparse)
- Boiler glow (red/orange)
- Flashlight essential
- **Lux Level**: 10-50 (very dark)

#### Audio
- Boiler rumbling
- Water dripping
- Enemy presence
- Ritual chanting (ambient)

#### Performance
- Draw Calls: ~800
- Triangles: ~400K
- Lights: 8 static, 6 dynamic
- AI: 3-4 active
- Particles: Steam, water

### 6. Upper Floor (150m²)

#### Layout
- **Dimensions**: Partial second floor
- **Height**: 3m ceiling
- **Connections**: 2 staircases, 1 collapsed section

#### Features
- Observation deck (vantage point)
- Archives (lore-heavy)
- Collapsed floor (danger)
- Shortcut to Exit

#### Objective 7: "Retrieve the Key"
- **Location**: Archives
- **Type**: Exploration reward
- **Time**: 4 minutes
- **Reward**: Shortcut unlock

#### Lighting
- Moonlight through windows
- Candles (atmospheric)
- Flashlight required
- **Lux Level**: 20-60 (dark)

#### Audio
- Wind howling
- Creaking floorboards
- Distant sounds below
- Paper rustling

#### Performance
- Draw Calls: ~400
- Triangles: ~200K
- Lights: 6 static, 2 dynamic

### 7. Exit Sequence (100m²)

#### Layout
- **Dimensions**: 20m × 5m corridor
- **Height**: 3m ceiling
- **Connections**: 1 entrance, 1 exit

#### Features
- Collapsing environment
- Final enemy encounter (optional)
- Escape timer (optional)
- Exit door

#### Objective 8: "Escape"
- **Location**: Exit door
- **Type**: Completion trigger
- **Time**: 3 minutes
- **Mechanic**: Final challenge

#### Lighting
- Emergency lighting (flashing)
- Sparks and fire
- Darkness encroaching
- **Lux Level**: 30-100 (chaotic)

#### Audio
- Collapse sounds
- Alarm blaring
- Enemy pursuit
- Music crescendo

#### Performance
- Draw Calls: ~500
- Triangles: ~250K
- Lights: 8 static, 4 dynamic
- Particles: Dust, sparks, fire

## Navigation Flow

### Vertical Movement

#### Staircases (4 total)
1. Main Corridor → Upper Floor
2. Main Corridor → Basement
3. West Wing → Upper Floor
4. East Wing → Basement

#### Ladders (2 total)
1. Basement → Main Corridor (emergency)
2. Upper Floor → Exit (shortcut)

### Shortcuts (3 total)

1. **Upper Floor → Exit**
   - Unlocks: After Objective 7
   - Saves: 5 minutes
   - Bypasses: Final basement section

2. **West Wing → East Wing**
   - Unlocks: After Objective 3
   - Saves: 2 minutes
   - Bypasses: Main Corridor return

3. **Basement → Entry Hall**
   - Unlocks: After Objective 6
   - Saves: 3 minutes
   - Bypasses: All upper floors

## Timing Breakdown

### Beat-by-Beat Timing

| Beat | Zones | Objectives | Time | Cumulative |
|------|-------|------------|------|------------|
| 1. Arrival | Entry Hall | 1 | 5 min | 5 min |
| 2. Exploration | Main Corridor, West Wing | 2, 3 | 8 min | 13 min |
| 3. First Threat | East Wing | 5 | 5 min | 18 min |
| 4. Rising Tension | West Wing (upper), Basement | 4, 6 | 10 min | 28 min |
| 5. Crisis Point | Basement | 6 (complete) | 4 min | 32 min |
| 6. Resolution | Upper Floor | 7 | 2 min | 34 min |
| 7. Escape | Exit Sequence | 8 | 1 min | 35 min |

### Movement Speed Assumptions
- **Walk Speed**: 3 m/s
- **Run Speed**: 6 m/s
- **Crouch Speed**: 1.5 m/s
- **Average**: 4 m/s (mixed movement)

## Sightlines and Vistas

### Key Vistas (6 total)

1. **Entry Hall → Main Corridor**
   - Purpose: Show scale and branching paths
   - Distance: 25m sightline

2. **Main Corridor → All Wings**
   - Purpose: Orient player to layout
   - Distance: 40m sightline

3. **Upper Floor → Basement**
   - Purpose: Vertical reveal, foreshadowing
   - Distance: 15m vertical

4. **West Wing → East Wing** (through windows)
   - Purpose: Show parallel spaces
   - Distance: 20m across

5. **Basement → Ritual Chamber**
   - Purpose: Objective reveal
   - Distance: 30m sightline

6. **Exit Sequence → Freedom**
   - Purpose: Goal visualization
   - Distance: 20m to exit

## Chokepoints and Bottlenecks

### Tension Chokepoints (5 total)

1. **Entry → Main Corridor**
   - Single doorway
   - Tutorial gate
   - No return

2. **Main Corridor → East Wing**
   - Narrow passage
   - Enemy introduction
   - High tension

3. **Basement Entrance**
   - Staircase descent
   - Point of no return (temporary)
   - Maximum danger ahead

4. **Ritual Chamber Entrance**
   - Single door
   - Boss arena
   - Commitment required

5. **Exit Sequence Start**
   - One-way trigger
   - Final challenge
   - Escape begins

## Safe Zones

### Designated Safe Areas (4 total)

1. **Entry Hall Waiting Area**
   - No enemy spawns
   - Good lighting
   - Save point

2. **West Wing Bathroom**
   - Lockable door
   - Resource cache
   - Breather moment

3. **Upper Floor Observation Deck**
   - High ground advantage
   - Good visibility
   - Optional area

4. **Main Corridor (after power restore)**
   - Improved lighting
   - Enemy avoidance possible
   - Hub safety

## Enemy Patrol Routes

### East Wing Patrol
- **Path**: Morgue → Exam Room 1 → Corridor → Exam Room 2 → Morgue
- **Loop Time**: 60 seconds
- **Behavior**: Investigates sounds

### Basement Patrol
- **Path**: Boiler Room → Storage → Ritual Chamber → Storage → Boiler Room
- **Loop Time**: 90 seconds
- **Behavior**: Guards objective 6

### Dynamic Spawns
- **Trigger**: Objective 3 completion
- **Location**: Random in East Wing
- **Count**: 1-2 enemies
- **Behavior**: Hunt player

## Performance Optimization Zones

### High-Detail Zones (require optimization)
1. Entry Hall (first impression)
2. Ritual Chamber (climax moment)
3. Exit Sequence (final impression)

### Medium-Detail Zones
1. Main Corridor
2. West Wing
3. East Wing

### Low-Detail Zones (can be simpler)
1. Basement (darkness hides detail)
2. Upper Floor (partial area)
3. Transition spaces

## Playtesting Checkpoints

### Navigation Tests
- [ ] Can reach all objectives without hints?
- [ ] Are shortcuts discoverable?
- [ ] Do players get lost? Where?
- [ ] Is backtracking frustrating?

### Timing Tests
- [ ] First playthrough: 35-45 minutes?
- [ ] Speedrun: 15-20 minutes?
- [ ] 100% completion: 45-60 minutes?

### Tension Tests
- [ ] Does tension build appropriately?
- [ ] Are safe zones effective?
- [ ] Do chokepoints create stress?
- [ ] Is pacing consistent?

### Performance Tests
- [ ] Framerate stable in all zones?
- [ ] Loading times acceptable?
- [ ] No hitching during transitions?
- [ ] Memory usage within budget?

## Iteration Notes

### Version History
- **v1.0**: Initial blockout
- **v1.1**: Adjusted West Wing size (+50m²)
- **v1.2**: Added Upper Floor shortcuts
- **v1.3**: Rebalanced enemy patrols
- **v2.0**: Art pass complete
- **v2.1**: Performance optimization
- **v2.2**: Playtesting adjustments

### Known Issues
- Basement flooding may cause performance dips
- Upper Floor collapse needs better telegraphing
- Exit Sequence timing too tight for some players

### Future Improvements
- Add more environmental storytelling
- Improve objective 4 discoverability
- Balance resource distribution
- Add more safe zone options
