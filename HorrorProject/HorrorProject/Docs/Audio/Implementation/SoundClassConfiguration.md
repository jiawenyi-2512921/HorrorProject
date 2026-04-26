# Sound Class Configuration Guide

## Overview

Complete guide for configuring the sound class hierarchy in HorrorProject, enabling organized audio mixing, volume control, and effects processing.

## Sound Class Fundamentals

### What is a Sound Class?

A Sound Class is a category that groups related sounds for unified control of:
- Volume
- Pitch
- Effects (reverb, EQ, compression)
- Ducking
- Routing to submixes

### Benefits

- **Organization**: Logical grouping of sounds
- **Control**: Adjust entire categories at once
- **Mixing**: Balance different audio types
- **Effects**: Apply processing to groups
- **Performance**: Optimize by category

## Sound Class Hierarchy

### Complete Hierarchy

```
Master (Root)
├── Music
│   ├── Tension
│   └── Ambient_Music
├── SFX
│   ├── Footsteps
│   ├── Interactions
│   ├── Impacts
│   └── Creatures
├── Ambience
│   ├── Environmental
│   └── Underwater
├── Voice
│   ├── Player
│   └── NPC
└── UI
    ├── Menu
    └── HUD
```

### Hierarchy Principles

**Parent-Child Relationships:**
- Child inherits parent settings
- Child can override parent
- Volume multiplies down hierarchy
- Effects accumulate

**Example:**
```
Master Volume: 0.8
├── SFX Volume: 0.7
    └── Footsteps Volume: 0.6
    
Final Footsteps Volume: 0.8 × 0.7 × 0.6 = 0.336
```

## Creating Sound Classes

### Access Sound Classes

**Method 1: Project Settings**
1. Edit → Project Settings
2. Audio → Sound Classes
3. Click "+" to add new class

**Method 2: Content Browser**
1. Content Browser → Right-click
2. Sounds → Sound Class
3. Name and configure

### Master Sound Class

**Configuration:**

```
Name: Master
Parent: None (root)
```

**Properties:**
```
Volume: 1.0
Pitch: 1.0
Low Pass Filter Frequency: 20000 Hz (no filtering)
Apply Effects: No
Apply Ambient Volumes: No
```

**Advanced:**
```
Voice Center Channel Volume: 0.0
Radio Filter Volume: 0.0
Radio Filter Volume Threshold: 0.0
```

**Purpose:**
- Root of all audio
- Global volume control
- No effects applied
- Pass-through for all sounds

---

### Music Sound Class

**Configuration:**

```
Name: Music
Parent: Master
```

**Properties:**
```
Volume: 0.7
Pitch: 1.0
Low Pass Filter Frequency: 20000 Hz
Apply Effects: Yes
Apply Ambient Volumes: No
```

**Effects:**
```
Reverb Send Level: 0.2
Enable Compression: Yes
Compression Threshold: -12 dB
Compression Ratio: 2:1
```

**Advanced:**
```
Output to Submix: Submix_Music
Priority: 0.8
Virtualization Mode: Play When Silent
```

**Purpose:**
- Background music
- Tension music
- Ambient music tracks
- Always audible (no attenuation)

**Usage:**
- SC_Ambient_Tension
- Future music tracks

---

### SFX Sound Class

**Configuration:**

```
Name: SFX
Parent: Master
```

**Properties:**
```
Volume: 0.8
Pitch: 1.0
Low Pass Filter Frequency: 20000 Hz
Apply Effects: Yes
Apply Ambient Volumes: Yes
```

**Effects:**
```
Reverb Send Level: 0.4
Enable Compression: No
Enable EQ: Yes
EQ Low Gain: 0 dB
EQ Mid Gain: +2 dB
EQ High Gain: +1 dB
```

**Advanced:**
```
Output to Submix: Submix_SFX
Priority: 0.6
Virtualization Mode: Stop
Max Concurrent Sounds: 32
```

**Purpose:**
- Parent for all sound effects
- Unified SFX control
- Reverb enabled
- Concurrency limiting

---

#### Footsteps Sound Class

**Configuration:**

```
Name: Footsteps
Parent: SFX
```

**Properties:**
```
Volume: 0.7
Pitch: 1.0
Apply Effects: Yes
```

**Effects:**
```
Reverb Send Level: 0.5
Enable EQ: Yes
EQ Low Gain: -2 dB (reduce rumble)
EQ Mid Gain: +3 dB (clarity)
EQ High Gain: +2 dB (detail)
```

**Advanced:**
```
Priority: 0.6
Max Concurrent Sounds: 4 per surface type
Concurrency Resolution: Stop Oldest
```

**Purpose:**
- Player and NPC footsteps
- Surface-specific sounds
- Prevent footstep spam

**Usage:**
- All SC_Footstep_* sound cues

---

#### Interactions Sound Class

**Configuration:**

```
Name: Interactions
Parent: SFX
```

**Properties:**
```
Volume: 0.8
Pitch: 1.0
Apply Effects: Yes
```

**Effects:**
```
Reverb Send Level: 0.4
Enable EQ: Yes
EQ Low Gain: 0 dB
EQ Mid Gain: +2 dB
EQ High Gain: +1 dB
```

**Advanced:**
```
Priority: 0.7
Max Concurrent Sounds: 8
Concurrency Resolution: Stop Quietest
```

**Purpose:**
- Door opens/closes
- Item pickups
- Button presses
- Object interactions

---

#### Impacts Sound Class

**Configuration:**

```
Name: Impacts
Parent: SFX
```

**Properties:**
```
Volume: 0.9
Pitch: 1.0
Apply Effects: Yes
```

**Effects:**
```
Reverb Send Level: 0.6
Enable Compression: Yes
Compression Threshold: -6 dB
Compression Ratio: 3:1
```

**Advanced:**
```
Priority: 0.7
Max Concurrent Sounds: 16
Concurrency Resolution: Stop Oldest
```

**Purpose:**
- Object impacts
- Collisions
- Drops
- Crashes

---

#### Creatures Sound Class

**Configuration:**

```
Name: Creatures
Parent: SFX
```

**Properties:**
```
Volume: 0.85
Pitch: 1.0
Apply Effects: Yes
```

**Effects:**
```
Reverb Send Level: 0.5
Enable EQ: Yes
EQ Low Gain: +3 dB (menacing)
EQ Mid Gain: 0 dB
EQ High Gain: -2 dB (muffled)
```

**Advanced:**
```
Priority: 0.9 (high priority)
Max Concurrent Sounds: 8
Concurrency Resolution: Stop Quietest
```

**Purpose:**
- Creature sounds
- Monster audio
- Threat audio
- High priority for gameplay

---

### Ambience Sound Class

**Configuration:**

```
Name: Ambience
Parent: Master
```

**Properties:**
```
Volume: 0.6
Pitch: 1.0
Low Pass Filter Frequency: 20000 Hz
Apply Effects: Yes
Apply Ambient Volumes: Yes
```

**Effects:**
```
Reverb Send Level: 0.3
Enable EQ: Yes
EQ Low Gain: +2 dB (fullness)
EQ Mid Gain: 0 dB
EQ High Gain: -1 dB (smooth)
```

**Advanced:**
```
Output to Submix: Submix_Ambient
Priority: 0.5
Virtualization Mode: Restart
Max Concurrent Sounds: 16
```

**Purpose:**
- Environmental ambience
- Background sounds
- Atmospheric audio
- Looping sounds

---

#### Environmental Sound Class

**Configuration:**

```
Name: Environmental
Parent: Ambience
```

**Properties:**
```
Volume: 1.0 (inherit from parent)
Pitch: 1.0
Apply Effects: Yes
```

**Effects:**
```
Reverb Send Level: 0.3
```

**Advanced:**
```
Priority: 0.5
Max Concurrent Sounds: 8
```

**Purpose:**
- Station ambience
- Machinery sounds
- Water sounds
- General environment

**Usage:**
- SC_Ambient_Station_Base
- SC_Ambient_Station_Machinery
- SC_Ambient_Station_Water

---

#### Underwater Sound Class

**Configuration:**

```
Name: Underwater
Parent: Ambience
```

**Properties:**
```
Volume: 1.0
Pitch: 0.95 (slightly lower)
Low Pass Filter Frequency: 1000 Hz (heavy filtering)
Apply Effects: Yes
```

**Effects:**
```
Reverb Send Level: 0.6
Reverb Type: Underwater
Enable EQ: Yes
EQ Low Gain: +4 dB (pressure)
EQ Mid Gain: -2 dB
EQ High Gain: -8 dB (muffled)
```

**Advanced:**
```
Output to Submix: Submix_Underwater
Priority: 0.7
Virtualization Mode: Restart
```

**Purpose:**
- Underwater ambience
- Submerged sounds
- Muffled environment
- Special underwater processing

**Usage:**
- SC_Ambient_Underwater
- SC_Underwater_Ambience
- SC_Underwater_Bubbles
- SC_Underwater_Movement

---

### Voice Sound Class

**Configuration:**

```
Name: Voice
Parent: Master
```

**Properties:**
```
Volume: 0.9
Pitch: 1.0
Low Pass Filter Frequency: 20000 Hz
Apply Effects: Yes
Apply Ambient Volumes: No
```

**Effects:**
```
Reverb Send Level: 0.3
Enable Compression: Yes
Compression Threshold: -18 dB
Compression Ratio: 4:1
Enable EQ: Yes
EQ Low Gain: -3 dB (reduce rumble)
EQ Mid Gain: +4 dB (clarity)
EQ High Gain: +2 dB (presence)
```

**Advanced:**
```
Output to Submix: Submix_Voice
Priority: 0.9 (high priority)
Virtualization Mode: Play When Silent
Voice Center Channel Volume: 1.0
```

**Purpose:**
- All voice audio
- Player breathing
- NPC dialogue
- Always clear and audible

---

#### Player Sound Class

**Configuration:**

```
Name: Player
Parent: Voice
```

**Properties:**
```
Volume: 1.0
Pitch: 1.0
Apply Effects: Yes
```

**Effects:**
```
Reverb Send Level: 0.1 (minimal)
Enable Compression: Yes
Compression Threshold: -15 dB
Compression Ratio: 3:1
```

**Advanced:**
```
Priority: 1.0 (highest)
Max Concurrent Sounds: 2
Concurrency Resolution: Stop Oldest
```

**Purpose:**
- Player breathing
- Player vocalizations
- Player-centric audio
- Always audible

**Usage:**
- All SC_Breathing_* sound cues

---

#### NPC Sound Class

**Configuration:**

```
Name: NPC
Parent: Voice
```

**Properties:**
```
Volume: 0.9
Pitch: 1.0
Apply Effects: Yes
```

**Effects:**
```
Reverb Send Level: 0.4
Enable Compression: Yes
Compression Threshold: -18 dB
Compression Ratio: 4:1
```

**Advanced:**
```
Priority: 0.8
Max Concurrent Sounds: 4
Concurrency Resolution: Stop Quietest
```

**Purpose:**
- NPC dialogue
- NPC vocalizations
- Character audio

---

### UI Sound Class

**Configuration:**

```
Name: UI
Parent: Master
```

**Properties:**
```
Volume: 0.8
Pitch: 1.0
Low Pass Filter Frequency: 20000 Hz
Apply Effects: No
Apply Ambient Volumes: No
```

**Effects:**
```
Reverb Send Level: 0.0 (dry)
Enable Compression: No
Enable EQ: No
```

**Advanced:**
```
Output to Submix: Submix_UI
Priority: 0.9
Virtualization Mode: Play When Silent
Is UI Sound: Yes
```

**Purpose:**
- Menu sounds
- HUD sounds
- UI feedback
- Always 2D, no attenuation

---

#### Menu Sound Class

**Configuration:**

```
Name: Menu
Parent: UI
```

**Properties:**
```
Volume: 1.0
Pitch: 1.0
Apply Effects: No
```

**Advanced:**
```
Priority: 0.9
Max Concurrent Sounds: 4
```

**Purpose:**
- Menu navigation
- Button clicks
- Menu transitions

---

#### HUD Sound Class

**Configuration:**

```
Name: HUD
Parent: UI
```

**Properties:**
```
Volume: 0.8
Pitch: 1.0
Apply Effects: No
```

**Advanced:**
```
Priority: 0.8
Max Concurrent Sounds: 8
```

**Purpose:**
- HUD notifications
- Objective updates
- Status indicators

---

## Ducking Configuration

### What is Ducking?

Automatic volume reduction of background sounds when important sounds play.

### Dialogue Ducking

**Configuration:**
```
Trigger Sound Class: Voice
Affected Sound Classes: [Music, Ambience, SFX]
Duck Volume: 0.3 (70% reduction)
Fade In Time: 0.5 seconds
Fade Out Time: 1.0 seconds
```

**Purpose:**
- Ensure dialogue clarity
- Reduce background noise
- Smooth transitions

### Music Ducking

**Configuration:**
```
Trigger Sound Class: Creatures
Affected Sound Classes: [Music]
Duck Volume: 0.5 (50% reduction)
Fade In Time: 0.2 seconds
Fade Out Time: 0.5 seconds
```

**Purpose:**
- Emphasize threat sounds
- Maintain tension
- Quick response

---

## Volume Mixing

### Default Mix Levels

| Sound Class | Volume | Relative to Master |
|-------------|--------|--------------------|
| Master | 1.0 | 100% |
| Music | 0.7 | 70% |
| SFX | 0.8 | 80% |
| Ambience | 0.6 | 60% |
| Voice | 0.9 | 90% |
| UI | 0.8 | 80% |

### Submix Routing

```
Master Submix
├── Music Submix (Music class)
├── SFX Submix (SFX class)
├── Ambient Submix (Ambience class)
├── Voice Submix (Voice class)
├── Underwater Submix (Underwater class)
└── UI Submix (UI class)
```

---

## Blueprint Integration

### Setting Sound Class Volume

**C++ Example:**
```cpp
// In AudioManager or GameInstance
void SetSoundClassVolume(USoundClass* SoundClass, float Volume)
{
    if (SoundClass)
    {
        SoundClass->Properties.Volume = Volume;
    }
}
```

**Blueprint Example:**
```
Set Sound Class Volume
├── Sound Class: Music
└── Volume: 0.5
```

### Getting Sound Class Volume

**C++ Example:**
```cpp
float GetSoundClassVolume(USoundClass* SoundClass)
{
    if (SoundClass)
    {
        return SoundClass->Properties.Volume;
    }
    return 0.0f;
}
```

---

## Testing Sound Classes

### Testing Checklist

**For Each Sound Class:**
- [ ] Volume control works
- [ ] Effects apply correctly
- [ ] Ducking functions properly
- [ ] Concurrency limits work
- [ ] Priority respected
- [ ] Submix routing correct
- [ ] Child classes inherit properly

### Console Commands

```
au.SoundClass.Debug 1          // Show sound class info
au.SoundClass.Volume Music 0.5 // Set music volume
stat soundclasses              // Show all sound classes
```

---

## Best Practices

1. **Logical Hierarchy**: Group related sounds
2. **Consistent Naming**: Follow conventions
3. **Appropriate Volumes**: Balance mix
4. **Use Ducking**: For important sounds
5. **Limit Concurrency**: Prevent spam
6. **Test Thoroughly**: Check all classes
7. **Document**: Note design decisions
8. **Iterate**: Adjust based on feedback
9. **Performance**: Monitor CPU usage
10. **Accessibility**: Provide volume controls

---

## Quick Reference Table

| Sound Class | Parent | Volume | Priority | Max Concurrent | Usage |
|-------------|--------|--------|----------|----------------|-------|
| Master | None | 1.0 | - | - | Root |
| Music | Master | 0.7 | 0.8 | - | Music |
| SFX | Master | 0.8 | 0.6 | 32 | Effects |
| Footsteps | SFX | 0.7 | 0.6 | 4 | Footsteps |
| Ambience | Master | 0.6 | 0.5 | 16 | Ambient |
| Environmental | Ambience | 1.0 | 0.5 | 8 | Station |
| Underwater | Ambience | 1.0 | 0.7 | - | Underwater |
| Voice | Master | 0.9 | 0.9 | - | Voice |
| Player | Voice | 1.0 | 1.0 | 2 | Breathing |
| UI | Master | 0.8 | 0.9 | - | UI |

**Total Sound Classes: 14**
