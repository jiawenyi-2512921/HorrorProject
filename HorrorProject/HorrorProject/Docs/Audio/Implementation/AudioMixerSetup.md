# Audio Mixer Setup Guide

## Overview

Complete guide for configuring the audio mixer system in HorrorProject, including submixes, effects chains, and dynamic mixing for different game states.

## Audio Mixer Fundamentals

### What is an Audio Mixer?

The audio mixer is a system that:
- Routes audio through submixes
- Applies effects processing
- Manages dynamic mixing
- Controls master output

### Benefits

- **Professional Mixing**: Industry-standard audio processing
- **Dynamic Control**: Change mix based on game state
- **Effects Processing**: Reverb, EQ, compression, etc.
- **Performance**: Efficient audio routing
- **Flexibility**: Easy to adjust and iterate

## Submix Architecture

### Submix Hierarchy

```
Master Submix
├── Music Submix
│   └── Effects: Compression, EQ
├── SFX Submix
│   ├── Effects: Reverb, EQ
│   └── Footsteps Submix
│       └── Effects: EQ, Compression
├── Ambient Submix
│   ├── Effects: Reverb, Low Pass
│   └── Underwater Submix
│       └── Effects: Heavy Low Pass, Reverb
├── Voice Submix
│   └── Effects: Compression, EQ, De-esser
└── UI Submix
    └── Effects: None (dry)
```

## Creating Submixes

### Master Submix

**Location:** `Content/Audio/Submixes/`

**Configuration:**

```
Name: Submix_Master
Parent: None (root)
```

**Properties:**
```
Output Volume: 1.0
Wet Level: 1.0
Dry Level: 0.0
```

**Effects Chain:**
```
1. Master Limiter
   - Threshold: -0.3 dB
   - Release Time: 100 ms
   - Lookahead: 10 ms

2. Master Compressor
   - Threshold: -6 dB
   - Ratio: 2:1
   - Attack: 10 ms
   - Release: 100 ms
   - Makeup Gain: 2 dB
```

**Purpose:**
- Final output stage
- Prevent clipping
- Gentle compression
- Consistent loudness

**Step-by-Step Creation:**

1. **Create Submix**
   - Content Browser → Right-click in `/Game/Audio/Submixes/`
   - Sounds → Submix
   - Name: `Submix_Master`

2. **Configure Properties**
   - Open submix asset
   - Set Output Volume: 1.0
   - Set Parent Submix: None

3. **Add Effects**
   - Effects Chain section
   - Add Effect → Limiter
   - Configure limiter settings
   - Add Effect → Compressor
   - Configure compressor settings

4. **Save and Test**
   - Save asset
   - Test with various sounds
   - Monitor output levels

---

### Music Submix

**Configuration:**

```
Name: Submix_Music
Parent: Submix_Master
```

**Properties:**
```
Output Volume: 1.0
Wet Level: 1.0
Dry Level: 0.0
```

**Effects Chain:**
```
1. EQ
   - Low Shelf: -2 dB @ 100 Hz (reduce rumble)
   - Mid Peak: +1 dB @ 2000 Hz (presence)
   - High Shelf: +2 dB @ 8000 Hz (air)

2. Compressor
   - Threshold: -12 dB
   - Ratio: 2:1
   - Attack: 20 ms
   - Release: 200 ms
   - Makeup Gain: 3 dB

3. Reverb
   - Reverb Type: Hall
   - Wet Level: 0.15
   - Decay Time: 2.0 seconds
   - Pre-delay: 20 ms
```

**Purpose:**
- Music processing
- Consistent dynamics
- Spatial enhancement
- Professional sound

**Usage:**
- Music sound class
- SC_Ambient_Tension

---

### SFX Submix

**Configuration:**

```
Name: Submix_SFX
Parent: Submix_Master
```

**Properties:**
```
Output Volume: 1.0
Wet Level: 1.0
Dry Level: 0.0
```

**Effects Chain:**
```
1. EQ
   - Low Cut: 80 Hz (remove rumble)
   - Mid Peak: +2 dB @ 3000 Hz (clarity)
   - High Shelf: +1 dB @ 10000 Hz (detail)

2. Reverb
   - Reverb Type: Room
   - Wet Level: 0.3
   - Decay Time: 1.2 seconds
   - Pre-delay: 10 ms
   - Damping: 0.5

3. Compressor
   - Threshold: -10 dB
   - Ratio: 3:1
   - Attack: 5 ms
   - Release: 50 ms
   - Makeup Gain: 2 dB
```

**Purpose:**
- SFX processing
- Environmental reverb
- Dynamic control
- Clarity enhancement

**Usage:**
- SFX sound class
- Footsteps, interactions, impacts

---

### Ambient Submix

**Configuration:**

```
Name: Submix_Ambient
Parent: Submix_Master
```

**Properties:**
```
Output Volume: 1.0
Wet Level: 1.0
Dry Level: 0.0
```

**Effects Chain:**
```
1. Low Pass Filter
   - Frequency: 12000 Hz
   - Resonance: 1.0
   - Purpose: Smooth high end

2. EQ
   - Low Shelf: +2 dB @ 100 Hz (fullness)
   - Mid Cut: -1 dB @ 1000 Hz (space for dialogue)
   - High Shelf: -2 dB @ 8000 Hz (smooth)

3. Reverb
   - Reverb Type: Large Room
   - Wet Level: 0.25
   - Decay Time: 3.0 seconds
   - Pre-delay: 30 ms
   - Damping: 0.6

4. Compressor
   - Threshold: -18 dB
   - Ratio: 2:1
   - Attack: 50 ms
   - Release: 300 ms
   - Makeup Gain: 1 dB
```

**Purpose:**
- Ambient processing
- Smooth, immersive sound
- Environmental character
- Consistent background

**Usage:**
- Ambience sound class
- SC_Ambient_Station_*

---

### Underwater Submix

**Configuration:**

```
Name: Submix_Underwater
Parent: Submix_Ambient
```

**Properties:**
```
Output Volume: 1.0
Wet Level: 1.0
Dry Level: 0.0
```

**Effects Chain:**
```
1. Low Pass Filter
   - Frequency: 1000 Hz (heavy filtering)
   - Resonance: 1.5
   - Purpose: Muffled underwater sound

2. EQ
   - Low Shelf: +6 dB @ 80 Hz (pressure)
   - Mid Cut: -4 dB @ 2000 Hz (muffled)
   - High Cut: -12 dB @ 4000 Hz (very muffled)

3. Reverb
   - Reverb Type: Underwater
   - Wet Level: 0.6
   - Decay Time: 4.0 seconds
   - Pre-delay: 50 ms
   - Damping: 0.8

4. Chorus
   - Depth: 0.3
   - Rate: 0.5 Hz
   - Purpose: Underwater movement

5. Compressor
   - Threshold: -15 dB
   - Ratio: 4:1
   - Attack: 30 ms
   - Release: 200 ms
   - Makeup Gain: 4 dB
```

**Purpose:**
- Underwater audio processing
- Heavy filtering and muffling
- Unique underwater character
- Immersive underwater experience

**Usage:**
- Underwater sound class
- SC_Underwater_*

---

### Voice Submix

**Configuration:**

```
Name: Submix_Voice
Parent: Submix_Master
```

**Properties:**
```
Output Volume: 1.0
Wet Level: 1.0
Dry Level: 0.0
```

**Effects Chain:**
```
1. High Pass Filter
   - Frequency: 80 Hz
   - Purpose: Remove rumble

2. De-esser
   - Frequency: 6000 Hz
   - Threshold: -20 dB
   - Ratio: 4:1
   - Purpose: Reduce sibilance

3. EQ
   - Low Cut: 100 Hz (clarity)
   - Mid Peak: +4 dB @ 3000 Hz (presence)
   - High Shelf: +2 dB @ 8000 Hz (air)

4. Compressor
   - Threshold: -18 dB
   - Ratio: 4:1
   - Attack: 5 ms
   - Release: 100 ms
   - Makeup Gain: 6 dB

5. Reverb
   - Reverb Type: Small Room
   - Wet Level: 0.2
   - Decay Time: 0.8 seconds
   - Pre-delay: 5 ms
```

**Purpose:**
- Voice processing
- Maximum clarity
- Consistent levels
- Professional sound

**Usage:**
- Voice sound class
- SC_Breathing_*
- Future dialogue

---

### UI Submix

**Configuration:**

```
Name: Submix_UI
Parent: Submix_Master
```

**Properties:**
```
Output Volume: 1.0
Wet Level: 1.0
Dry Level: 0.0
```

**Effects Chain:**
```
None (dry signal)
```

**Purpose:**
- UI audio routing
- No processing needed
- Clean, direct sound
- Always audible

**Usage:**
- UI sound class
- Menu sounds
- HUD sounds

---

## Sound Mix Modifiers

### Mix_Underwater

**Purpose:** Applied when player is underwater.

**Configuration:**

```
Name: Mix_Underwater
Duration: Infinite (until popped)
Fade In Time: 1.0 seconds
Fade Out Time: 1.5 seconds
```

**Sound Class Adjustments:**
```
Music:
  - Volume: 0.3 (70% reduction)
  - Pitch: 0.95
  - Low Pass Filter: 800 Hz

SFX:
  - Volume: 0.2 (80% reduction)
  - Pitch: 0.9
  - Low Pass Filter: 600 Hz

Ambience:
  - Volume: 0.4 (60% reduction)
  - Low Pass Filter: 1000 Hz

Voice:
  - Volume: 0.8 (20% reduction)
  - Low Pass Filter: 1500 Hz

Underwater:
  - Volume: 1.0 (no change)
  - Pitch: 1.0
```

**Effects:**
```
Global Low Pass Filter: 1000 Hz
Global Reverb: Underwater preset
Global Wet Level: 0.6
```

**Blueprint Usage:**
```cpp
// When entering water
PushSoundMixModifier(Mix_Underwater);

// When exiting water
PopSoundMixModifier(Mix_Underwater);
```

**Step-by-Step Creation:**

1. **Create Sound Mix**
   - Content Browser → Right-click in `/Game/Audio/Mixes/`
   - Sounds → Sound Mix
   - Name: `Mix_Underwater`

2. **Configure Duration**
   - Open asset
   - Set Duration: -1 (infinite)
   - Set Fade In Time: 1.0
   - Set Fade Out Time: 1.5

3. **Add Sound Class Adjustments**
   - Sound Class Effects section
   - Add entry for each sound class
   - Configure volume, pitch, filters

4. **Test**
   - Push mix in Blueprint
   - Verify all sounds muffled
   - Check smooth transitions
   - Pop mix and verify return to normal

---

### Mix_Tension

**Purpose:** Applied during threat encounters.

**Configuration:**

```
Name: Mix_Tension
Duration: Infinite
Fade In Time: 0.5 seconds
Fade Out Time: 2.0 seconds
```

**Sound Class Adjustments:**
```
Music:
  - Volume: 1.2 (+20%)
  - Pitch: 1.0
  - High Pass Filter: 200 Hz

SFX:
  - Volume: 1.0 (no change)
  - Pitch: 1.0

Ambience:
  - Volume: 0.5 (50% reduction)
  - Low Pass Filter: 8000 Hz

Voice:
  - Volume: 1.0 (no change)
  - Pitch: 1.0

Creatures:
  - Volume: 1.3 (+30%)
  - Pitch: 1.0
```

**Effects:**
```
Global High Pass Filter: 150 Hz (remove low rumble)
Music Reverb: Reduced (0.1)
Ambience Reverb: Increased (0.5)
```

**Blueprint Usage:**
```cpp
// When threat detected
PushSoundMixModifier(Mix_Tension);

// When threat lost
PopSoundMixModifier(Mix_Tension);
```

**Purpose:**
- Emphasize music and threats
- Reduce ambient distraction
- Increase tension
- Clear audio focus

---

### Mix_Pause

**Purpose:** Applied when game is paused.

**Configuration:**

```
Name: Mix_Pause
Duration: Infinite
Fade In Time: 0.2 seconds
Fade Out Time: 0.2 seconds
```

**Sound Class Adjustments:**
```
Music:
  - Volume: 0.3 (70% reduction)
  - Pitch: 0.8
  - Low Pass Filter: 500 Hz

SFX:
  - Volume: 0.1 (90% reduction)
  - Pitch: 0.8
  - Low Pass Filter: 500 Hz

Ambience:
  - Volume: 0.2 (80% reduction)
  - Pitch: 0.8
  - Low Pass Filter: 500 Hz

Voice:
  - Volume: 0.2 (80% reduction)
  - Pitch: 0.8
  - Low Pass Filter: 500 Hz

UI:
  - Volume: 1.0 (no change)
  - Pitch: 1.0
```

**Effects:**
```
Global Low Pass Filter: 500 Hz
Global Pitch: 0.8
Global Reverb: Increased (0.6)
Time Dilation: 0.5x (slow motion effect)
```

**Blueprint Usage:**
```cpp
// When pausing
PushSoundMixModifier(Mix_Pause);

// When unpausing
PopSoundMixModifier(Mix_Pause);
```

**Purpose:**
- Muffled, distant sound
- UI remains clear
- Pause state feedback
- Smooth transitions

---

## Dynamic Mixing

### Adaptive Music System

**Concept:** Music volume and intensity adapt to gameplay.

**Implementation:**

```cpp
// In AudioManager
void UpdateMusicIntensity(float ThreatLevel)
{
    // ThreatLevel: 0.0 (safe) to 1.0 (danger)
    
    float MusicVolume = FMath::Lerp(0.5f, 1.0f, ThreatLevel);
    float TensionVolume = FMath::Lerp(0.0f, 0.8f, ThreatLevel);
    
    SetSoundClassVolume(MusicClass, MusicVolume);
    SetSoundClassVolume(TensionClass, TensionVolume);
}
```

**Usage:**
- Increase music during threats
- Reduce during exploration
- Smooth transitions
- Dynamic tension

---

### Environmental Mixing

**Concept:** Mix changes based on environment.

**Implementation:**

```cpp
// In AudioZone
void OnPlayerEnterZone()
{
    // Apply zone-specific mix
    if (ZoneType == EZoneType::Underwater)
    {
        PushSoundMixModifier(Mix_Underwater);
    }
    else if (ZoneType == EZoneType::Tension)
    {
        PushSoundMixModifier(Mix_Tension);
    }
}

void OnPlayerExitZone()
{
    // Remove zone mix
    PopSoundMixModifier(CurrentMix);
}
```

**Usage:**
- Underwater zones
- Tension zones
- Safe zones
- Smooth transitions

---

### Combat Mixing

**Concept:** Mix emphasizes combat sounds.

**Implementation:**

```cpp
// In CombatManager
void OnCombatStart()
{
    // Emphasize combat sounds
    SetSoundClassVolume(SFXClass, 1.2f);
    SetSoundClassVolume(AmbienceClass, 0.4f);
    PushSoundMixModifier(Mix_Combat);
}

void OnCombatEnd()
{
    // Return to normal
    SetSoundClassVolume(SFXClass, 0.8f);
    SetSoundClassVolume(AmbienceClass, 0.6f);
    PopSoundMixModifier(Mix_Combat);
}
```

---

## Testing and Validation

### Testing Checklist

**Submixes:**
- [ ] All submixes created
- [ ] Effects chains configured
- [ ] Routing correct
- [ ] No clipping or distortion
- [ ] Performance acceptable

**Sound Mixes:**
- [ ] All mixes created
- [ ] Transitions smooth
- [ ] Volumes appropriate
- [ ] Effects work correctly
- [ ] Push/pop functions properly

**Dynamic Mixing:**
- [ ] Adaptive systems work
- [ ] Smooth transitions
- [ ] No audio pops
- [ ] Performance good

### Console Commands

```
au.Debug.Submixes 1           // Show submix info
au.Debug.SoundMixes 1         // Show active mixes
stat soundmixes               // Performance stats
au.Submix.Volume Music 0.5    // Set submix volume
```

---

## Best Practices

1. **Gentle Processing**: Avoid over-processing
2. **Headroom**: Leave -6 dB headroom
3. **Smooth Transitions**: Use appropriate fade times
4. **Test on Target**: Test on actual hardware
5. **Monitor Levels**: Watch for clipping
6. **Iterate**: Adjust based on feedback
7. **Document**: Note all settings
8. **Performance**: Monitor CPU usage
9. **Consistency**: Maintain consistent loudness
10. **Accessibility**: Provide mix presets

## Quick Reference

### Submix Summary

| Submix | Parent | Effects | Usage |
|--------|--------|---------|-------|
| Submix_Master | None | Limiter, Compressor | All audio |
| Submix_Music | Master | EQ, Compressor, Reverb | Music |
| Submix_SFX | Master | EQ, Reverb, Compressor | SFX |
| Submix_Ambient | Master | Low Pass, EQ, Reverb | Ambient |
| Submix_Underwater | Ambient | Heavy Low Pass, Reverb | Underwater |
| Submix_Voice | Master | De-esser, EQ, Compressor | Voice |
| Submix_UI | Master | None | UI |

### Sound Mix Summary

| Mix | Purpose | Fade In | Fade Out | Key Changes |
|-----|---------|---------|----------|-------------|
| Mix_Underwater | Underwater | 1.0s | 1.5s | Heavy low pass, muffling |
| Mix_Tension | Threats | 0.5s | 2.0s | Music up, ambience down |
| Mix_Pause | Paused | 0.2s | 0.2s | All down except UI |

**Total Submixes: 7**
**Total Sound Mixes: 3**
