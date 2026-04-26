# Audio System Blueprint Implementation Guide

## Overview

The Audio system provides 4 specialized audio components:
- **UAmbientAudioComponent** - Layered ambient audio with time-of-day support
- **UFootstepAudioComponent** - Dynamic footstep sounds based on surface
- **UBreathingAudioComponent** - Player breathing with stress/oxygen modulation
- **UUnderwaterAudioComponent** - Underwater audio effects and muffling

All components inherit from `UAudioComponent` and are `BlueprintSpawnableComponent`.

## C++ Foundation

### AmbientAudioComponent

**Purpose**: Multi-layered ambient audio with dynamic mixing.

**Key Features**:
- 4 audio types: Static, Dynamic, Layered, Randomized
- Multiple audio layers with independent volume control
- Time-of-day intensity modulation
- Randomization support
- Fade in/out transitions
- Distance attenuation

**Audio Types**:
```cpp
enum class EAmbientAudioType : uint8
{
    Static,      // Single looping sound
    Dynamic,     // Changes based on game state
    Layered,     // Multiple simultaneous layers
    Randomized   // Random variations
};
```

### FootstepAudioComponent

**Purpose**: Surface-aware footstep audio.

**Key Features**:
- Surface type detection (concrete, metal, water, etc.)
- Speed-based playback rate
- Crouch/walk/run variations
- Automatic timing based on movement speed
- Volume modulation based on movement type

### BreathingAudioComponent

**Purpose**: Player breathing audio with stress modulation.

**Key Features**:
- Stress level affects breathing rate
- Oxygen level affects breathing intensity
- Smooth transitions between breathing states
- Panic breathing when threatened
- Holding breath support

### UnderwaterAudioComponent

**Purpose**: Underwater audio effects and muffling.

**Key Features**:
- Automatic muffling of world sounds
- Underwater ambience
- Bubble sounds
- Depth-based pressure effects
- Smooth transition when entering/exiting water

## Blueprint Implementation

### Step 1: Create Audio Assets

**Location**: `Content/Audio/`

#### Ambient Audio Assets

**Required Sound Cues** (create in `Content/Audio/Ambient/`):

1. **SC_Ambient_Station_Base**
   - Type: Looping
   - Sounds: Low hum, machinery drone
   - Volume: 0.3-0.5
   - Attenuation: Lar(5000 units)

2. **SC_Ambient_Station_Machinery**
   - Type: Looping
   - Sounds: Mechanical clanks, distant machinery
   - Volume: 0.2-0.4
   - Attenuation: Medium radius (3000 units)

3. **SC_Ambient_Station_Water**
   - Type: Looping
   - Sounds: Water drips, flowing water
   - Volume: 0.3-0.5
   - Attenuation: Medium radius (2000 units)

4. **SC_Ambient_Underwater**
   - Type: Looping
   - Sounds: Underwater ambience, pressure sounds
   - Volume: 0.5-0.7
   - Attenuation: Large radius (8000 units)

5. **SC_Ambient_Tension**
   - Type: Looping
   - Sounds: Low drones, tension music
   - Volume: 0.2-0.4
   - Attenuation: No attenuation (always audible)

#### Footstep Audio Assets

**Required Sound Cues** (create in `Content/Audio/Footsteps/`):

1. **SC_Footstep_Concrete**
   - Type: Random variations (5-8 sounds)
   - Sounds: Concrete footsteps
   - Volume: 0.5-0.7
   - Pitch variation: 0.9-1.1

2. **SC_Footstep_Metal**
   - Type: Random variations (5-8 sounds)
   - Sounds: Metal footsteps (clangy)
   - Volume: 0.6-0.8
   - Pitch variation: 0.9-1.1

3. **SC_Footstep_Water**
   - Type: Random variations (5-8 sounds)
   - Sounds: Splashing, wet footsteps
   - Volume: 0.5-0.7
   - Pitch variation: 0.95-1.05

4. **SC_Footstep_Grate**
   - Type: Random variations (5-8 sounds)
   - Sounds: Metal grate footsteps
   - Volume: 0.6-0.8
   - Pitch variation: 0.9-1.1

5. **SC_Footstep_Carpet**
   - Type: Random variations (5-8 sounds)
   - Sounds: Soft, muffled footsteps
   - Volume: 0.3-0.5
   - Pitch variation: 0.95-1.05

#### Breathing Audio Assets

**Required Sound Cues** (create in `Content/Audio/Breathing/`):

1. **SC_Breathing_Normal**
   - Type: Looping
   - Sounds: Calm breathing
   - Volume: 0.3-0.4
   - Playback rate: 1.0

2. **SC_Breathing_Heavy**
   - Type: Looping
   - Sounds: Heavy breathing (exertion)
   - Volume: 0.5-0.6
   - Playback rate: 1.2

3. **SC_Breathing_Panic**
   - Type: Looping
   - Sounds: Rapid, panicked breathing
   - Volume: 0.6-0.8
   - Playback rate: 1.5

4. **SC_Breathing_LowOxygen**
   - Type: Looping
   - Sounds: Gasping, struggling breathing
   - Volume: 0.7-0.9
   - Playback rate: 0.8

5. **SC_Breathing_HoldBreath**
   - Type: One-shot
   - Sounds: Inhale, hold, exhale
   - Volume: 0.4-0.5

#### Underwater Audio Assets

**Required Sound Cues** (create in `Content/Audio/Underwater/`):

1. **SC_Underwater_Ambience**
   - Type: Looping
   - Sounds: Underwater ambience, pressure
   - Volume: 0.5-0.7
   - Low-pass filter: 1000 Hz

2. **SC_Underwater_Bubbles**
   - Type: Random variations
   - Sounds: Bubble sounds
   - Volume: 0.4-0.6
   - Pitch variation: 0.8-1.2

3. **SC_Underwater_Movement**
   - Type: Looping
   - Sounds: Water movement, swimming
   - Volume: 0.3-0.5

4. **SC_Underwater_Enter**
   - Type: One-shot
   - Sounds: Splash, submersion
   - Volume: 0.7-0.9

5. **SC_Underwater_Exit**
   - Type: One-shot
   - Sounds: Surface break, gasp
   - Volume: 0.7-0.9

### Step 2: Create Sound Attenuation Assets

**Location**: `Content/Audio/Attenuation/`

#### ATT_Ambient_Large
- Attenuation Shape: Sphere
- Falloff Distance: 5000 units
- Attenuation Function: Natural Sound
- Enable Air Absorption: Yes

#### ATT_Ambient_Medium
- Attenuation Shape: Sphere
- Falloff Distance: 3000 units
- Attenuation Function: Natural Sound
- Enable Air Absorption: Yes

#### ATT_Footsteps
- Attenuation Shape: Sphere
- Falloff Distance: 1500 units
- Attenuation Function: Linear
- Enable Air Absorption: Yes

#### ATT_Breathing
- Attenuation Shape: Sphere
- Falloff Distance: 500 units (close to player)
- Attenuation Function: Linear
- Enable Air Absorption: No

#### ATT_Underwater
- Attenuation Shape: Sphere
- Falloff Distance: 8000 units
- Attenuation Function: Natural Sound
- Enable Air Absorption: No

### Step 3: Create Audio Zone Actors

**Purpose**: Define areas with specific ambient audio.

#### BP_AudioZone_Station

**Parent Class**: HorrorAudioZoneActor

**Components**:
- AmbientAudioComponent (name: `AmbientAudio`)
- BoxComponent (name: `TriggerVolume`)

**Configuration**:
- Ambient Type: Layered
- Audio Layers:
  - Layer 0: SC_Ambient_Station_Base (Volume: 0.5)
  - Layer 1: SC_Ambient_Station_Machinery (Volume: 0.3)
  - Layer 2: SC_Ambient_Station_Water (Volume: 0.2)
- Auto Start: true
- Max Distance: 5000

**Event Graph**:
```
Event BeginPlay
  → AmbientAudio → Start Ambient

On Actor Begin Overlap (TriggerVolume)
  → AmbientAudio → Set Layer Volume (0, 0.5, 2.0)
  → AmbientAudio → Set Layer Volume (1, 0.3, 2.0)
  → AmbientAudio → Set Layer Volume (2, 0.2, 2.0)

On Actor End Overlap (TriggerVolume)
  → AmbientAudio → Set Layer Volume (0, 0.0, 2.0)
  → AmbientAudio → Set Layer Volume (1, 0.0, 2.0)
  → AmbientAudio → Set Layer Volume (2, 0.0, 2.0)
```

#### BP_AudioZone_Underwater

**Parent Class**: HorrorAudioZoneActor

**Components**:
- UnderwaterAudioComponent (name: `UnderwaterAudio`)
- BoxComponent (name: `WaterVolume`)

**Configuration**:
- Underwater Sound: SC_Underwater_Ambience
- Bubble Sound: SC_Underwater_Bubbles
- Movement Sound: SC_Underwater_Movement
- Muffling Intensity: 0.8

**Event Graph**:
```
On Actor Begin Overlap (WaterVolume)
  → UnderwaterAudio → Enter Water
  → Play Sound 2D (SC_Underwater_Enter)

On Actor End Overlap (WaterVolume)
  → UnderwaterAudio → Exit Water
  → Play Sound 2D (SC_Underwater_Exit)
```

### Step 4: Integrate with Player Character

**In BP_PlayerCharacter**:

**Add Components**:
- FootstepAudioComponent (name: `FootstepAudio`)
- BreathingAudioComponent (name: `BreathingAudio`)

**Component Configuration**:

**FootstepAudioComponent**:
- Footstep Sounds (map):
  - Concrete → SC_Footstep_Concrete
  - Metal → SC_Footstep_Metal
  - Water → SC_Footstep_Water
  - Grate → SC_Footstep_Grate
  - Carpet → SC_Footstep_Carpet
- Base Step Interval: 0.5
- Walk Speed Threshold: 200
- Run Speed Threshold: 400
- Crouch Volume Multiplier: 0.5

**BreathingAudioComponent**:
- Normal Breathing: SC_Breathing_Normal
- Heavy Breathing: SC_Breathing_Heavy
- Panic Breathing: SC_Breathing_Panic
- Low Oxygen Breathing: SC_Breathing_LowOxygen
- Auto Start: true
- Base Breathing Rate: 1.0

**Event Graph**:

#### Footstep System
```
Event Tick
  → Get Velocity
  → Vector Length (get speed)
  → FootstepAudio → Update Movement Speed (Speed)

On Movement Mode Changed
  → Branch (Is Swimming)
    True: FootstepAudio → Set Surface Type (Water)
    False: FootstepAudio → Detect Surface Type
```

#### Breathing System
```
Event Tick
  → Get Stress Level (from game state)
  → BreathingAudio → Set Stress Level (Stress)
  
  → Get Oxygen Level (from oxygen component)
  → BreathingAudio → Set Oxygen Level (Oxygen)

On Threat Detected
  → BreathingAudio → Set Panic Mode (true)

On Threat Lost
  → BreathingAudio → Set Panic Mode (false)
```

### Step 5: Create Audio Manager Blueprint

**Purpose**: Central audio management and mixing.

**Creation Steps**:
1. Content Browser → Blueprint Class → Actor
2. Name: `BP_AudioManager`
3. Place one in each level

**Variables**:
- `MasterVolume` (float, 0-1)
- `MusicVolume` (float, 0-1)
- `SFXVolume` (float, 0-1)
- `AmbienceVolume` (float, 0-1)
- `VoiceVolume` (float, 0-1)
- `ActiveAudioZones` (Array of HorrorAudioZoneActor)

**Functions**:

#### SetMasterVolume
```
Input: float Volume (0-1)
Set MasterVolume = Volume
  → Set Sound Class Volume (Master, Volume)
```

#### SetMusicVolume
```
Input: float Volume (0-1)
Set MusicVolume = Volume
  → Set Sound Class Volume (Music, Volume * MasterVolume)
```

#### SetSFXVolume
```
Input: float Volume (0-1)
Set SFXVolume = Volume
  → Set Sound Class Volume (SFX, Volume * MasterVolume)
```

#### SetAmbienceVolume
```
Input: float Volume (0-1)
Set AmbienceVolume = Volume
  → Set Sound Class Volume (Ambience, Volume * MasterVolume)
```

#### RegisterAudioZone
```
Input: HorrorAudioZoneActor Zone
Add to ActiveAudioZones array
```

#### UnregisterAudioZone
```
Input: HorrorAudioZoneActor Zone
Remove from ActiveAudioZones array
```

### Step 6: Create Sound Classes

**Location**: Project Settings → Audio → Sound Classes

**Sound Class Hierarchy**:
```
Master
├── Music
├── SFX
│   ├── Footsteps
│   ├── Interactions
│   └── Impacts
├── Ambience
│   ├── Environmental
│   └── Underwater
├── Voice
│   ├── Player
│   └── NPC
└── UI
```

**Sound Class Properties**:
- Master: No attenuation, no effects
- Music: No attenuation, reverb enabled
- SFX: Attenuation enabled, reverb enabled
- Ambience: Attenuation enabled, reverb enabled
- Voice: Slight attenuation, reverb enabled
- UI: No attenuation, no effects

### Step 7: Create Audio Mix Modifiers

**Location**: `Content/Audio/Mixes/`

#### Mix_Underwater
- Apply when player underwater
- Effects:
  - Low-pass filter: 1000 Hz
  - Reverb: Underwater preset
  - Volume reduction: -10 dB for non-underwater sounds

#### Mix_Tension
- Apply during threat encounters
- Effects:
  - High-pass filter: 200 Hz (remove low rumble)
  - Music volume: +5 dB
  - Ambience volume: -5 dB

#### Mix_Pause
- Apply when game paused
- Effects:
  - Low-pass filter: 500 Hz
  - Volume reduction: -20 dB for all except UI
  - Time dilation: 0.5x

## Blueprint Usage Examples

### Example 1: Layered Ambient Audio

**In BP_AudioZone_Station**:
```
Event BeginPlay
  → AmbientAudio → Add Layer
     - Sound: SC_Ambient_Station_Base
     - Base Volume: 0.5
     - Fade In Time: 2.0
  → AmbientAudio → Add Layer
     - Sound: SC_Ambient_Station_Machinery
     - Base Volume: 0.3
     - Fade In Time: 2.0
  → AmbientAudio → Start Ambient
```

### Example 2: Dynamic Footsteps

**In BP_PlayerCharacter**:
```
Event Tick
  → Get Velocity → Vector Length
  → FootstepAudio → Update Movement Speed (Speed)
  
  → Line Trace (down from feet)
  → Get Physical Material
  → FootstepAudio → Set Surface Type (Physical Material)
```

### Example 3: Stress-Based Breathing

**In BP_PlayerCharacter**:
```
On Threat Proximity Changed
  → Map Range In Range (Distance: 2000-0, Out: 0-1)
  → BreathingAudio → Set Stress Level (Mapped Value)
```

### Example 4: Underwater Audio Transition

**In BP_WaterVolume**:
```
On Actor Begin Overlap
  → Cast to Character
  → Get Component (UnderwaterAudioComponent)
  → Enter Water
  → Push Sound Mix Modifier (Mix_Underwater)

On Actor End Overlap
  → Cast to Character
  → Get Component (UnderwaterAudioComponent)
  → Exit Water
  → Pop Sound Mix Modifier (Mix_Underwater)
```

### Example 5: Time-of-Day Ambient Intensity

**In BP_AudioZone_Station**:
```
Event Tick
  → Get Time of Day (from game state)
  → Select (based on time):
     - Night (0-6): Intensity 0.3
     - Day (6-18): Intensity 0.7
     - Evening (18-24): Intensity 0.5
  → AmbientAudio → Set Time of Day Intensity (Intensity)
```

## Performance Optimization

### Audio Optimization
- Limit concurrent sounds (max 32-64)
- Use sound attenuation aggressively
- Enable audio occlusion
- Use compressed audio formats (OGG Vorbis)
- Stream large audio files
- Use audio LOD for distant sounds

### Component Optimization
- Disable tick when not needed
- Use timers instead of tick for periodic updates
- Pool audio components
- Unload unused audio assets
- Use audio culling distance

### Memory Optimization
- Compress audio assets
- Use streaming for music and ambience
- Unload audio when not in use
- Use shared audio assets
- Limit audio quality for distant sounds

## Troubleshooting

### Audio Not Playing
- Check sound asset is assigned
- Verify component is active
- Check volume levels (component, sound class, master)
- Ensure attenuation radius is sufficient
- Check audio device is working

### Footsteps Not Triggering
- Verify movement speed > threshold
- Check surface type detection
- Ensure footstep sounds are assigned
- Check step interval timing
- Verify component is ticking

### Breathing Too Loud/Quiet
- Adjust base volume in component
- Check stress/oxygen levels
- Verify breathing state transitions
- Adjust attenuation settings

### Underwater Effect Not Working
- Check water volume overlap
- Verify sound mix is pushed
- Ensure underwater sounds are assigned
- Check low-pass filter settings

### Performance Issues
- Check concurrent sound count (Stat Sounds)
- Reduce attenuation radius
- Enable audio culling
- Use compressed audio
- Limit audio quality

## Best Practices

1. **Use Sound Classes**: Organize audio with sound class hierarchy
2. **Attenuation**: Always use attenuation for 3D sounds
3. **Compression**: Use OGG Vorbis for all audio
4. **Streaming**: Stream music and long ambience
5. **Variations**: Use random variations for repeated sounds
6. **Mixing**: Use sound mix modifiers for game states
7. **Occlusion**: Enable audio occlusion for realism
8. **Testing**: Test audio on target hardware
9. **Volume**: Keep master volume reasonable (0.7-0.8)
10. **Accessibility**: Provide volume controls for all categories

## Testing Checklist

- [ ] All sound assets created and imported
- [ ] Sound attenuation assets configured
- [ ] Sound classes set up correctly
- [ ] Audio components added to player
- [ ] Audio zones placed in level
- [ ] Footsteps play on all surfaces
- [ ] Breathing responds to stress/oxygen
- [ ] Underwater audio works correctly
- [ ] Ambient audio layers correctly
- [ ] Volume controls work
- [ ] No audio popping or clicking
- [ ] Performance acceptable (< 5% CPU)
- [ ] No memory leaks
- [ ] Audio occlusion works
- [ ] Mixing sounds balanced

## Next Steps

1. Import all audio assets
2. Create sound cues and attenuation
3. Set up sound classes
4. Add audio components to player
5. Create audio zones for level
6. Test each audio system
7. Balance audio mixing
8. Optimize performance
9. Add volume controls to settings
10. Test on target hardware
