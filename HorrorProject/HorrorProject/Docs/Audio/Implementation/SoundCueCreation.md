# Sound Cue Creation Guide

## Overview

Comprehensive guide for creating all 20 Sound Cues for the HorrorProject audio system, including node configurations, parameters, and best practices.

## Sound Cue Fundamentals

### What is a Sound Cue?

A Sound Cue is a container that defines how sound waves are played, including:
- Randomization
- Looping
- Volume/pitch modulation
- Mixing
- Effects processing
- Conditional playback

### Benefits

- **Flexibility**: Modify playback without changing source audio
- **Variation**: Random variations prevent repetition
- **Control**: Fine-tune volume, pitch, effects
- **Efficiency**: Reuse sound waves in different contexts
- **Modularity**: Easy to update and maintain

## Sound Cue Editor Basics

### Opening Sound Cue Editor

1. Content Browser → Right-click in folder
2. Sounds → Sound Cue
3. Name the Sound Cue
4. Double-click to open editor

### Interface Overview

**Main Areas:**
- **Graph Area**: Node-based editor
- **Palette**: Available nodes
- **Details Panel**: Node properties
- **Preview**: Play and test

### Common Nodes

**Input Nodes:**
- **Wave Player**: Plays a sound wave
- **Random**: Randomly selects from inputs

**Processing Nodes:**
- **Looping**: Loops the sound
- **Modulator**: Varies pitch/volume
- **Delay**: Adds delay before playing
- **Mixer**: Mixes multiple sounds

**Output Nodes:**
- **Output**: Final output (always present)

## Ambient Sound Cues

### SC_Ambient_Station_Base

**Purpose**: Base ambient layer for station areas with variation.

**Node Graph:**
```
[Wave Player: ambient_station_base_01] ─┐
                                        ├─> [Random] ─> [Looping] ─> [Volume: 0.4] ─> [Output]
[Wave Player: ambient_station_base_02] ─┘
```

**Step-by-Step Creation:**

1. **Create Sound Cue**
   - Location: `Content/Audio/SoundCues/Ambient/`
   - Name: `SC_Ambient_Station_Base`

2. **Add Wave Players**
   - Drag `ambient_station_base_01` from Content Browser
   - Drag `ambient_station_base_02` from Content Browser

3. **Add Random Node**
   - Palette → Random
   - Connect both Wave Players to Random inputs
   - Set Weights: [0.5, 0.5]

4. **Add Looping Node**
   - Palette → Looping
   - Connect Random output to Looping input
   - Properties:
     - Loop Start: 0.0
     - Loop End: -1.0 (end of file)
     - Loop Count: -1 (infinite)

5. **Add Volume Node**
   - Palette → Modulator
   - Connect Looping to Modulator
   - Properties:
     - Volume Min: 0.4
     - Volume Max: 0.4
     - Pitch Min: 1.0
     - Pitch Max: 1.0

6. **Connect to Output**
   - Connect Modulator to Output node

7. **Configure Sound Cue Properties**
   - Sound Class: `Ambience/Environmental`
   - Attenuation Settings: `ATT_Ambient_Large`
   - Priority: 0.5
   - Enable Virtualization: Yes

8. **Test**
   - Click Play in toolbar
   - Verify both variations play randomly
   - Check volume level
   - Verify seamless looping

---

### SC_Ambient_Station_Machinery

**Purpose**: Machinery detail layer with pitch variation.

**Node Graph:**
```
[Wave Player: ambient_machinery_01] ─┐
                                     ├─> [Random] ─> [Looping] ─> [Modulator] ─> [Volume: 0.3] ─> [Output]
[Wave Player: ambient_machinery_02] ─┘
```

**Configuration:**

**Random Node:**
- Weights: [0.5, 0.5]
- Randomize Without Replacement: Yes

**Looping Node:**
- Loop Count: -1 (infinite)

**Modulator Node:**
- Volume Min: 1.0
- Volume Max: 1.0
- Pitch Min: 0.95
- Pitch Max: 1.05

**Volume Node:**
- Volume: 0.3

**Sound Cue Properties:**
- Sound Class: `Ambience/Environmental`
- Attenuation: `ATT_Ambient_Medium`
- Priority: 0.5

---

### SC_Ambient_Station_Water

**Purpose**: Water detail layer with delay variation.

**Node Graph:**
```
[Wave Player: ambient_water_drip_01] ─┐
                                      ├─> [Random] ─> [Delay] ─> [Looping] ─> [Volume: 0.35] ─> [Output]
[Wave Player: ambient_water_flow_01] ─┘
```

**Configuration:**

**Random Node:**
- Weights: [0.6, 0.4] (favor drips)

**Delay Node:**
- Delay Min: 0.5 seconds
- Delay Max: 2.0 seconds

**Looping Node:**
- Loop Count: -1

**Volume Node:**
- Volume: 0.35

**Sound Cue Properties:**
- Sound Class: `Ambience/Environmental`
- Attenuation: `ATT_Ambient_Medium`
- Priority: 0.5

---

### SC_Ambient_Underwater

**Purpose**: Underwater ambience with low-pass filter.

**Node Graph:**
```
[Wave Player: underwater_ambience_loop] ─> [Looping] ─> [Low Pass Filter] ─> [Volume: 0.6] ─> [Output]
```

**Configuration:**

**Looping Node:**
- Loop Count: -1

**Low Pass Filter Node:**
- Frequency: 1000 Hz
- Resonance: 1.0

**Volume Node:**
- Volume: 0.6

**Sound Cue Properties:**
- Sound Class: `Ambience/Underwater`
- Attenuation: `ATT_Underwater`
- Priority: 0.7
- Submix: `Submix_Underwater`

---

### SC_Ambient_Tension

**Purpose**: Dynamic tension music with volume modulation.

**Node Graph:**
```
[Wave Player: ambient_tension_01] ─> [Looping] ─> [Modulator] ─> [Volume: 0.3] ─> [Output]
```

**Configuration:**

**Looping Node:**
- Loop Count: -1

**Modulator Node:**
- Volume Min: 0.8
- Volume Max: 1.2
- Pitch Min: 1.0
- Pitch Max: 1.0

**Volume Node:**
- Volume: 0.3

**Sound Cue Properties:**
- Sound Class: `Music`
- Attenuation: None (2D sound)
- Priority: 0.8
- Is UI Sound: No

---

## Footstep Sound Cues

### SC_Footstep_Concrete

**Purpose**: Concrete footsteps with variation and modulation.

**Node Graph:**
```
[Wave Player: footstep_concrete_01] ─┐
[Wave Player: footstep_concrete_02] ─┼─> [Random] ─> [Modulator] ─> [Volume: 0.6] ─> [Output]
[Wave Player: footstep_concrete_03] ─┘
```

**Step-by-Step Creation:**

1. **Create Sound Cue**
   - Location: `Content/Audio/SoundCues/Footsteps/`
   - Name: `SC_Footstep_Concrete`

2. **Add Wave Players**
   - Drag all 3 concrete footstep files

3. **Add Random Node**
   - Connect all 3 Wave Players
   - Set Weights: [0.33, 0.34, 0.33]
   - Enable "Randomize Without Replacement"

4. **Add Modulator Node**
   - Pitch Min: 0.9
   - Pitch Max: 1.1
   - Volume Min: 0.9
   - Volume Max: 1.0

5. **Add Volume Node**
   - Volume: 0.6

6. **Configure Properties**
   - Sound Class: `SFX/Footsteps`
   - Attenuation: `ATT_Footsteps`
   - Priority: 0.6
   - Concurrency: `Concurrency_Footsteps`

7. **Test**
   - Play multiple times
   - Verify all 3 variations play
   - Check pitch variation
   - Verify no repetition

---

### SC_Footstep_Metal

**Configuration:**

**Random Node:**
- 3 inputs (metal_01, 02, 03)
- Weights: [0.33, 0.34, 0.33]
- Randomize Without Replacement: Yes

**Modulator Node:**
- Pitch Min: 0.9
- Pitch Max: 1.1
- Volume Min: 0.9
- Volume Max: 1.0

**Volume Node:**
- Volume: 0.7 (louder than concrete)

**Sound Cue Properties:**
- Sound Class: `SFX/Footsteps`
- Attenuation: `ATT_Footsteps`
- Priority: 0.6

---

### SC_Footstep_Water

**Configuration:**

**Random Node:**
- 3 inputs (water_01, 02, 03)
- Weights: [0.33, 0.34, 0.33]
- Randomize Without Replacement: Yes

**Modulator Node:**
- Pitch Min: 0.95
- Pitch Max: 1.05 (less variation)
- Volume Min: 0.9
- Volume Max: 1.0

**Volume Node:**
- Volume: 0.6

**Sound Cue Properties:**
- Sound Class: `SFX/Footsteps`
- Attenuation: `ATT_Footsteps`
- Priority: 0.6

---

### SC_Footstep_Grate

**Configuration:**

**Random Node:**
- 3 inputs (grate_01, 02, 03)
- Weights: [0.33, 0.34, 0.33]
- Randomize Without Replacement: Yes

**Modulator Node:**
- Pitch Min: 0.9
- Pitch Max: 1.1
- Volume Min: 0.9
- Volume Max: 1.0

**Volume Node:**
- Volume: 0.7 (metallic, louder)

**Sound Cue Properties:**
- Sound Class: `SFX/Footsteps`
- Attenuation: `ATT_Footsteps`
- Priority: 0.6

---

### SC_Footstep_Carpet

**Configuration:**

**Random Node:**
- 3 inputs (carpet_01, 02, 03)
- Weights: [0.33, 0.34, 0.33]
- Randomize Without Replacement: Yes

**Modulator Node:**
- Pitch Min: 0.95
- Pitch Max: 1.05
- Volume Min: 0.9
- Volume Max: 1.0

**Volume Node:**
- Volume: 0.4 (quieter, muffled)

**Sound Cue Properties:**
- Sound Class: `SFX/Footsteps`
- Attenuation: `ATT_Footsteps`
- Priority: 0.6

---

## Breathing Sound Cues

### SC_Breathing_Normal

**Purpose**: Normal calm breathing loop.

**Node Graph:**
```
[Wave Player: breathing_normal_loop] ─> [Looping] ─> [Volume: 0.35] ─> [Output]
```

**Configuration:**

**Looping Node:**
- Loop Count: -1

**Volume Node:**
- Volume: 0.35

**Sound Cue Properties:**
- Sound Class: `Voice/Player`
- Attenuation: `ATT_Breathing`
- Priority: 0.9
- Concurrency: Max 1 instance

---

### SC_Breathing_Heavy

**Purpose**: Heavy breathing with increased pitch.

**Node Graph:**
```
[Wave Player: breathing_heavy_loop] ─> [Looping] ─> [Modulator] ─> [Volume: 0.55] ─> [Output]
```

**Configuration:**

**Looping Node:**
- Loop Count: -1

**Modulator Node:**
- Pitch Min: 1.1
- Pitch Max: 1.3
- Volume Min: 1.0
- Volume Max: 1.0

**Volume Node:**
- Volume: 0.55

**Sound Cue Properties:**
- Sound Class: `Voice/Player`
- Attenuation: `ATT_Breathing`
- Priority: 0.9

---

### SC_Breathing_Panic

**Purpose**: Rapid panicked breathing.

**Node Graph:**
```
[Wave Player: breathing_panic_loop] ─> [Looping] ─> [Modulator] ─> [Volume: 0.7] ─> [Output]
```

**Configuration:**

**Looping Node:**
- Loop Count: -1

**Modulator Node:**
- Pitch Min: 1.4
- Pitch Max: 1.6
- Volume Min: 1.0
- Volume Max: 1.0

**Volume Node:**
- Volume: 0.7

**Sound Cue Properties:**
- Sound Class: `Voice/Player`
- Attenuation: `ATT_Breathing`
- Priority: 1.0 (highest)

---

### SC_Breathing_LowOxygen

**Purpose**: Gasping, struggling breathing.

**Node Graph:**
```
[Wave Player: breathing_low_oxygen_loop] ─> [Looping] ─> [Modulator] ─> [Volume: 0.8] ─> [Output]
```

**Configuration:**

**Looping Node:**
- Loop Count: -1

**Modulator Node:**
- Pitch Min: 0.7
- Pitch Max: 0.9
- Volume Min: 1.0
- Volume Max: 1.0

**Volume Node:**
- Volume: 0.8

**Sound Cue Properties:**
- Sound Class: `Voice/Player`
- Attenuation: `ATT_Breathing`
- Priority: 1.0

---

### SC_Breathing_HoldBreath

**Purpose**: One-shot hold breath action.

**Node Graph:**
```
[Wave Player: breathing_hold_breath] ─> [Volume: 0.5] ─> [Output]
```

**Configuration:**

**Volume Node:**
- Volume: 0.5

**Sound Cue Properties:**
- Sound Class: `Voice/Player`
- Attenuation: `ATT_Breathing`
- Priority: 0.9
- Concurrency: Max 1 instance

---

## Underwater Sound Cues

### SC_Underwater_Ambience

**Purpose**: Underwater ambient loop with filtering.

**Node Graph:**
```
[Wave Player: underwater_ambience_loop] ─> [Looping] ─> [Low Pass Filter] ─> [Volume: 0.6] ─> [Output]
```

**Configuration:**

**Looping Node:**
- Loop Count: -1

**Low Pass Filter:**
- Frequency: 1000 Hz
- Resonance: 1.0

**Volume Node:**
- Volume: 0.6

**Sound Cue Properties:**
- Sound Class: `Ambience/Underwater`
- Attenuation: `ATT_Underwater`
- Priority: 0.7
- Submix: `Submix_Underwater`

---

### SC_Underwater_Bubbles

**Purpose**: Random bubble sounds with variation.

**Node Graph:**
```
[Wave Player: underwater_bubbles_01] ─> [Modulator] ─> [Volume: 0.5] ─> [Output]
```

**Configuration:**

**Modulator Node:**
- Pitch Min: 0.8
- Pitch Max: 1.2
- Volume Min: 0.8
- Volume Max: 1.0

**Volume Node:**
- Volume: 0.5

**Sound Cue Properties:**
- Sound Class: `SFX`
- Attenuation: `ATT_Underwater`
- Priority: 0.4
- Concurrency: `Concurrency_Bubbles` (max 8)

---

### SC_Underwater_Movement

**Purpose**: Swimming movement loop.

**Node Graph:**
```
[Wave Player: underwater_movement_loop] ─> [Looping] ─> [Volume: 0.4] ─> [Output]
```

**Configuration:**

**Looping Node:**
- Loop Count: -1

**Volume Node:**
- Volume: 0.4

**Sound Cue Properties:**
- Sound Class: `SFX`
- Attenuation: `ATT_Underwater`
- Priority: 0.6
- Concurrency: Max 1 instance

---

### SC_Underwater_Enter

**Purpose**: Water entry splash.

**Node Graph:**
```
[Wave Player: underwater_enter] ─> [Volume: 0.8] ─> [Output]
```

**Configuration:**

**Volume Node:**
- Volume: 0.8

**Sound Cue Properties:**
- Sound Class: `SFX`
- Attenuation: None (2D sound)
- Priority: 0.8
- Concurrency: Max 1 instance

---

### SC_Underwater_Exit

**Purpose**: Water exit and gasp.

**Node Graph:**
```
[Wave Player: underwater_exit] ─> [Volume: 0.8] ─> [Output]
```

**Configuration:**

**Volume Node:**
- Volume: 0.8

**Sound Cue Properties:**
- Sound Class: `SFX`
- Attenuation: None (2D sound)
- Priority: 0.8
- Concurrency: Max 1 instance

---

## Advanced Techniques

### Dynamic Volume Control

**Use Case**: Volume changes based on game state.

**Implementation:**
1. Add Sound Cue Parameter node
2. Name parameter: "DynamicVolume"
3. Connect to Modulator volume input
4. Control from Blueprint:
   ```cpp
   AudioComponent->SetFloatParameter("DynamicVolume", NewVolume);
   ```

### Conditional Playback

**Use Case**: Different sounds based on conditions.

**Implementation:**
1. Add Branch node
2. Add Sound Cue Parameter (bool)
3. Connect different Wave Players to True/False outputs
4. Control from Blueprint

### Crossfading

**Use Case**: Smooth transition between sounds.

**Implementation:**
1. Add Crossfade by Param node
2. Add two Wave Players
3. Add Sound Cue Parameter (float 0-1)
4. Control crossfade from Blueprint

### Distance-Based Variation

**Use Case**: Different sounds at different distances.

**Implementation:**
1. Add Distance Crossfade node
2. Add Wave Players for near/far
3. Set crossfade distances
4. Automatic based on listener distance

## Testing and Validation

### Testing Checklist

**For Each Sound Cue:**
- [ ] Plays correctly in editor
- [ ] All variations work
- [ ] Volume level appropriate
- [ ] Pitch variation sounds natural
- [ ] Looping is seamless (if applicable)
- [ ] No clicks or pops
- [ ] Attenuation works correctly
- [ ] Priority is appropriate
- [ ] Concurrency limits work
- [ ] Sound class assigned

### Testing Tools

**In-Editor:**
- Sound Cue Editor preview
- Content Browser preview
- Place in level and test

**In-Game:**
- Console commands:
  - `stat sounds` - Show active sounds
  - `au.Debug.Sounds 1` - Debug sound playback
  - `au.3dVisualize.Enabled 1` - Visualize 3D sounds

### Common Issues

**No Sound:**
- Check volume nodes (not 0)
- Verify sound wave connected
- Check sound class volume
- Verify attenuation radius

**Repetitive:**
- Add more variations
- Enable "Randomize Without Replacement"
- Add pitch/volume modulation

**Clicks/Pops:**
- Check loop points in source audio
- Verify seamless loop
- Add fade in/out nodes

**Too Loud/Quiet:**
- Adjust volume nodes
- Check sound class volume
- Verify attenuation settings

## Best Practices

1. **Always Use Random**: Even with one sound, for future expansion
2. **Modulate**: Add pitch/volume variation to prevent repetition
3. **Test Thoroughly**: Play each cue multiple times
4. **Document**: Add descriptions to Sound Cue properties
5. **Organize**: Keep node graphs clean and readable
6. **Reuse**: Create reusable node setups
7. **Version Control**: Commit Sound Cues separately from audio
8. **Performance**: Limit node complexity
9. **Naming**: Use clear, descriptive names
10. **Consistency**: Follow same patterns across all cues

## Quick Reference

### Node Shortcuts

- **Drag Sound Wave**: Creates Wave Player node
- **Right-click**: Context menu with all nodes
- **C + Click**: Create comment box
- **Ctrl + D**: Duplicate selected nodes
- **Ctrl + C/V**: Copy/paste nodes

### Common Node Configurations

**Simple Loop:**
```
Wave Player → Looping → Volume → Output
```

**Random Variations:**
```
Wave Player 1 ┐
Wave Player 2 ├→ Random → Modulator → Volume → Output
Wave Player 3 ┘
```

**Filtered Ambient:**
```
Wave Player → Looping → Low Pass Filter → Volume → Output
```

### Sound Cue Summary

| Sound Cue | Nodes | Complexity | Priority |
|-----------|-------|------------|----------|
| SC_Ambient_Station_Base | 5 | Medium | 0.5 |
| SC_Footstep_Concrete | 4 | Low | 0.6 |
| SC_Breathing_Normal | 3 | Low | 0.9 |
| SC_Underwater_Ambience | 4 | Medium | 0.7 |

**Total Sound Cues: 20**
**Average Nodes per Cue: 4**
**Total Nodes: ~80**
