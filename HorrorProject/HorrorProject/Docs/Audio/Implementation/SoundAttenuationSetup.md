# Sound Attenuation Setup Guide

## Overview

Complete guide for configuring 3D audio attenuation in HorrorProject, including distance falloff, spatialization, and environmental effects.

## Attenuation Fundamentals

### What is Sound Attenuation?

Sound attenuation controls how audio volume decreases with distance from the source, simulating realistic 3D audio behavior.

**Key Concepts:**
- **Inner Radius**: Distance where sound is at full volume
- **Falloff Distance**: Distance where sound reaches minimum volume
- **Attenuation Shape**: Sphere, capsule, box, or cone
- **Falloff Function**: How volume decreases (linear, logarithmic, natural)

### Why Use Attenuation?

- **Realism**: Sounds fade naturally with distance
- **Performance**: Distant sounds can be culled
- **Immersion**: Proper 3D audio positioning
- **Gameplay**: Audio cues for player awareness

## Attenuation Assets

### ATT_Ambient_Large

**Purpose**: Large ambient sounds (station base, machinery hum)

**Configuration:**

**Distance Settings:**
```
Attenuation Shape: Sphere
Inner Radius: 1000 units
Falloff Distance: 5000 units
```

**Attenuation Function:**
```
Distance Algorithm: Natural Sound
Falloff Mode: Linear
```

**Spatialization:**
```
Enable Spatialization: Yes
Spatial Algorithm: Equal Power
Spread: 200 degrees
```

**Air Absorption:**
```
Enable Air Absorption: Yes
Air Absorption High Frequency: 0.5
Air Absorption Low Frequency: 0.1
```

**Reverb:**
```
Enable Reverb Send: Yes
Reverb Send Level: 0.3
```

**Advanced:**
```
Enable Occlusion: Yes
Occlusion Trace Channel: Visibility
Occlusion Interpolation Time: 0.5 seconds
```

**Step-by-Step Creation:**

1. **Create Asset**
   - Content Browser → Right-click in `/Game/Audio/Attenuation/`
   - Sounds → Sound Attenuation
   - Name: `ATT_Ambient_Large`

2. **Configure Distance**
   - Open asset
   - Attenuation (Distance) section:
     - Enable "Attenuate"
     - Attenuation Shape: Sphere
     - Falloff Distance: 5000
     - Non-Spatialized Radius: 0
     - Attenuation Function: Natural Sound

3. **Configure Spatialization**
   - Attenuation (Spatialization) section:
     - Enable "Spatialization"
     - Spatial Algorithm: Equal Power
     - Radius: 0 (use falloff distance)
     - Spread: 200

4. **Configure Air Absorption**
   - Attenuation (Air Absorption) section:
     - Enable "Air Absorption"
     - High Frequency: 0.5
     - Low Frequency: 0.1

5. **Configure Occlusion**
   - Attenuation (Occlusion) section:
     - Enable "Occlusion"
     - Trace Channel: Visibility
     - Interpolation Time: 0.5

6. **Save and Test**
   - Save asset
   - Assign to ambient sound cues
   - Test in level

**Usage:**
- SC_Ambient_Station_Base
- SC_Ambient_Underwater

**Visualization:**
```
Volume
  1.0 |████████████
      |            ████
  0.5 |                ████
      |                    ████
  0.0 |________________________████
      0   1000      3000      5000  Distance (units)
      └─Inner─┘└────Falloff────┘
```

---

### ATT_Ambient_Medium

**Purpose**: Medium ambient sounds (machinery, water)

**Configuration:**

**Distance Settings:**
```
Attenuation Shape: Sphere
Inner Radius: 500 units
Falloff Distance: 3000 units
```

**Attenuation Function:**
```
Distance Algorithm: Natural Sound
Falloff Mode: Linear
```

**Spatialization:**
```
Enable Spatialization: Yes
Spatial Algorithm: Equal Power
Spread: 180 degrees
```

**Air Absorption:**
```
Enable Air Absorption: Yes
Air Absorption High Frequency: 0.6
Air Absorption Low Frequency: 0.2
```

**Reverb:**
```
Enable Reverb Send: Yes
Reverb Send Level: 0.4
```

**Advanced:**
```
Enable Occlusion: Yes
Occlusion Trace Channel: Visibility
Occlusion Interpolation Time: 0.4 seconds
Occlusion Volume Attenuation: 0.5
```

**Usage:**
- SC_Ambient_Station_Machinery
- SC_Ambient_Station_Water

**Visualization:**
```
Volume
  1.0 |████████
      |        ████
  0.5 |            ████
      |                ████
  0.0 |____________________████
      0  500     1500    3000  Distance (units)
```

---

### ATT_Footsteps

**Purpose**: Player and NPC footsteps

**Configuration:**

**Distance Settings:**
```
Attenuation Shape: Sphere
Inner Radius: 200 units
Falloff Distance: 1500 units
```

**Attenuation Function:**
```
Distance Algorithm: Linear
Falloff Mode: Linear
```

**Spatialization:**
```
Enable Spatialization: Yes
Spatial Algorithm: Equal Power
Spread: 90 degrees (focused)
```

**Air Absorption:**
```
Enable Air Absorption: Yes
Air Absorption High Frequency: 0.7
Air Absorption Low Frequency: 0.3
```

**Reverb:**
```
Enable Reverb Send: Yes
Reverb Send Level: 0.5
```

**Advanced:**
```
Enable Occlusion: Yes
Occlusion Trace Channel: Visibility
Occlusion Interpolation Time: 0.2 seconds
Occlusion Volume Attenuation: 0.7
Enable Focus: Yes
Focus Azimuth: 30 degrees
Focus Distance Scale: 1.0
```

**Usage:**
- All SC_Footstep_* sound cues

**Visualization:**
```
Volume
  1.0 |████
      |    ████
  0.5 |        ████
      |            ████
  0.0 |________________████
      0 200    750   1500  Distance (units)
```

---

### ATT_Breathing

**Purpose**: Player breathing (close to listener)

**Configuration:**

**Distance Settings:**
```
Attenuation Shape: Sphere
Inner Radius: 100 units
Falloff Distance: 500 units
```

**Attenuation Function:**
```
Distance Algorithm: Linear
Falloff Mode: Linear
```

**Spatialization:**
```
Enable Spatialization: No (2D sound, always centered)
```

**Air Absorption:**
```
Enable Air Absorption: No (too close)
```

**Reverb:**
```
Enable Reverb Send: No (dry, intimate sound)
```

**Advanced:**
```
Enable Occlusion: No (always audible)
Enable Low Pass Filter: No
```

**Usage:**
- All SC_Breathing_* sound cues

**Visualization:**
```
Volume
  1.0 |██
      |  ████
  0.5 |      ████
      |          ████
  0.0 |______________████
      0 100   300   500  Distance (units)
```

---

### ATT_Underwater

**Purpose**: Underwater sounds (large radius)

**Configuration:**

**Distance Settings:**
```
Attenuation Shape: Sphere
Inner Radius: 2000 units
Falloff Distance: 8000 units
```

**Attenuation Function:**
```
Distance Algorithm: Natural Sound
Falloff Mode: Logarithmic
```

**Spatialization:**
```
Enable Spatialization: Yes
Spatial Algorithm: Equal Power
Spread: 360 degrees (omnidirectional)
```

**Air Absorption:**
```
Enable Air Absorption: No (underwater)
Enable Low Pass Filter: Yes
Low Pass Filter Frequency: 1000 Hz
```

**Reverb:**
```
Enable Reverb Send: Yes
Reverb Send Level: 0.6
Reverb Type: Underwater
```

**Advanced:**
```
Enable Occlusion: Yes
Occlusion Trace Channel: Visibility
Occlusion Interpolation Time: 0.8 seconds
Occlusion Volume Attenuation: 0.3
```

**Usage:**
- SC_Underwater_Ambience
- SC_Underwater_Bubbles
- SC_Underwater_Movement

**Visualization:**
```
Volume
  1.0 |████████████████
      |                ████
  0.5 |                    ████
      |                        ████
  0.0 |____________________________████
      0     2000      5000      8000  Distance (units)
```

---

## Attenuation Shapes

### Sphere

**Best For:**
- Omnidirectional sounds
- Point sources
- Most common use case

**Configuration:**
```
Shape: Sphere
Radius: Falloff Distance
```

**Use Cases:**
- Ambient sounds
- Footsteps
- Explosions
- General SFX

### Capsule

**Best For:**
- Linear sources
- Corridors
- Pipes

**Configuration:**
```
Shape: Capsule
Half Height: Length of source
Radius: Falloff Distance
```

**Use Cases:**
- Pipe sounds
- Corridor ambience
- Linear machinery

### Box

**Best For:**
- Room ambience
- Rectangular areas

**Configuration:**
```
Shape: Box
Extents: Room dimensions
Falloff Distance: Additional padding
```

**Use Cases:**
- Room tone
- Area ambience
- Rectangular zones

### Cone

**Best For:**
- Directional sources
- Speakers
- Alarms

**Configuration:**
```
Shape: Cone
Cone Angle: Spread angle
Falloff Distance: Distance
```

**Use Cases:**
- Directional speakers
- Alarms
- Focused sounds

---

## Falloff Functions

### Natural Sound

**Characteristics:**
- Realistic falloff
- Inverse square law approximation
- Smooth transition

**Formula:**
```
Volume = 1 / (1 + Distance²)
```

**Best For:**
- Ambient sounds
- Realistic environments
- Large sounds

### Linear

**Characteristics:**
- Constant rate of decrease
- Predictable
- Simple

**Formula:**
```
Volume = 1 - (Distance / MaxDistance)
```

**Best For:**
- Footsteps
- Small sounds
- Gameplay-critical audio

### Logarithmic

**Characteristics:**
- Slow initial falloff
- Rapid distant falloff
- Perceptually linear

**Formula:**
```
Volume = log(MaxDistance / Distance)
```

**Best For:**
- Music
- Underwater sounds
- Atmospheric audio

### Inverse

**Characteristics:**
- Rapid initial falloff
- Slow distant falloff
- Realistic for point sources

**Formula:**
```
Volume = 1 / Distance
```

**Best For:**
- Explosions
- Impacts
- Sudden sounds

---

## Spatialization

### Equal Power

**Characteristics:**
- Constant perceived loudness
- Smooth panning
- Industry standard

**Configuration:**
```
Spatial Algorithm: Equal Power
Spread: 90-200 degrees
```

**Best For:**
- Most 3D sounds
- General use

### HRTF (Head-Related Transfer Function)

**Characteristics:**
- Realistic 3D positioning
- Elevation cues
- CPU intensive

**Configuration:**
```
Spatial Algorithm: HRTF
Enable Binaural: Yes
```

**Best For:**
- VR
- Headphone users
- Critical positioning

### Ambisonics

**Characteristics:**
- 360-degree soundfield
- Rotation-independent
- Complex setup

**Configuration:**
```
Spatial Algorithm: Ambisonics
Order: 1st or 3rd
```

**Best For:**
- VR
- 360 video
- Immersive audio

---

## Air Absorption

### What is Air Absorption?

Simulation of high-frequency loss over distance, mimicking real-world physics.

### Configuration

**High Frequency Absorption:**
```
Range: 0.0 (none) to 1.0 (full)
Recommended: 0.5-0.7
```

**Low Frequency Absorption:**
```
Range: 0.0 (none) to 1.0 (full)
Recommended: 0.1-0.3
```

### Distance-Based Filtering

**Near (< 1000 units):**
- High Freq: 0.3
- Low Freq: 0.1

**Medium (1000-3000 units):**
- High Freq: 0.5
- Low Freq: 0.2

**Far (> 3000 units):**
- High Freq: 0.7
- Low Freq: 0.3

---

## Occlusion

### What is Occlusion?

Volume reduction when objects block the sound path.

### Configuration

**Basic Setup:**
```
Enable Occlusion: Yes
Trace Channel: Visibility
Interpolation Time: 0.5 seconds
Volume Attenuation: 0.5 (50% reduction)
```

**Advanced Setup:**
```
Enable Low Pass Filter: Yes
Occlusion Filter Frequency: 500 Hz
Enable Reverb Occlusion: Yes
Reverb Occlusion: 0.3
```

### Performance Considerations

**Trace Frequency:**
- High Priority: Every frame
- Medium Priority: Every 0.1 seconds
- Low Priority: Every 0.5 seconds

**Optimization:**
```
Max Occlusion Traces: 32
Trace Complex Collision: No
Trace Distance: Falloff Distance
```

---

## Reverb Send

### What is Reverb Send?

Amount of signal sent to reverb bus for environmental effects.

### Configuration by Category

**Ambient:**
```
Reverb Send Level: 0.3-0.4
Reverb Type: Large Room
```

**Footsteps:**
```
Reverb Send Level: 0.5-0.6
Reverb Type: Room
```

**Underwater:**
```
Reverb Send Level: 0.6-0.8
Reverb Type: Underwater
```

**Breathing:**
```
Reverb Send Level: 0.0 (dry)
```

---

## Testing Attenuation

### In-Editor Testing

**Method 1: Place in Level**
1. Place audio actor in level
2. Assign sound cue with attenuation
3. Move player camera around
4. Listen to volume changes

**Method 2: Visualize**
1. Select audio actor
2. Show → Audio Radius
3. See attenuation sphere
4. Verify inner/outer radius

**Method 3: Console Commands**
```
au.3dVisualize.Enabled 1
au.Debug.Sounds 1
stat sounds
```

### In-Game Testing

**Checklist:**
- [ ] Volume decreases with distance
- [ ] Spatialization works (left/right)
- [ ] Occlusion reduces volume through walls
- [ ] Air absorption filters distant sounds
- [ ] Reverb adds environmental character
- [ ] No sudden volume jumps
- [ ] Performance acceptable

---

## Best Practices

1. **Match Attenuation to Sound**: Large sounds = large radius
2. **Use Natural Sound**: For most ambient sounds
3. **Enable Occlusion**: For realism
4. **Test Thoroughly**: Walk around sound sources
5. **Visualize**: Use debug visualization
6. **Optimize**: Limit occlusion traces
7. **Consistent**: Use same settings for similar sounds
8. **Document**: Note why specific settings chosen
9. **Iterate**: Adjust based on playtesting
10. **Performance**: Monitor with stat commands

## Quick Reference Table

| Attenuation | Inner | Falloff | Shape | Algorithm | Occlusion | Usage |
|-------------|-------|---------|-------|-----------|-----------|-------|
| ATT_Ambient_Large | 1000 | 5000 | Sphere | Natural | Yes | Large ambient |
| ATT_Ambient_Medium | 500 | 3000 | Sphere | Natural | Yes | Medium ambient |
| ATT_Footsteps | 200 | 1500 | Sphere | Linear | Yes | Footsteps |
| ATT_Breathing | 100 | 500 | Sphere | Linear | No | Breathing |
| ATT_Underwater | 2000 | 8000 | Sphere | Natural | Yes | Underwater |

## Troubleshooting

**Sound too quiet at distance:**
- Increase falloff distance
- Adjust attenuation function
- Check sound class volume

**Sound too loud everywhere:**
- Decrease inner radius
- Steeper falloff function
- Enable air absorption

**Occlusion not working:**
- Verify trace channel
- Check collision setup
- Increase interpolation time

**Performance issues:**
- Reduce occlusion traces
- Increase trace interval
- Disable for distant sounds
