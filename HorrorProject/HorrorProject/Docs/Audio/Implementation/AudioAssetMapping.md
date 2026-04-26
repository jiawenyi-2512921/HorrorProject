# Audio Asset Mapping

## Overview

Complete mapping of all 33 audio files including source files, imported assets, sound cues, and usage locations.

## Asset Categories

### 1. Ambient Audio (8 files)

#### Station Base Ambience

**Source Files:**
- `ambient_station_base_01.wav`
- `ambient_station_base_02.wav`

**Imported Assets:**
- `Content/Audio/Ambient/Station/ambient_station_base_01`
- `Content/Audio/Ambient/Station/ambient_station_base_02`

**Sound Cue:**
- `Content/Audio/SoundCues/Ambient/SC_Ambient_Station_Base`

**Properties:**
- Duration: 30-60 seconds each
- Sample Rate: 44100 Hz
- Bit Depth: 16-bit
- Channels: Stereo
- Looping: Yes
- Compression: OGG Vorbis 40%

**Usage:**
- BP_AudioZone_Station (Layer 0)
- Base ambient layer for station areas
- Volume: 0.4-0.5
- Attenuation: ATT_Ambient_Large (5000 units)

**Description:**
Low frequency hum and machinery drone providing base ambient layer for station environments.

---

#### Station Machinery

**Source Files:**
- `ambient_machinery_01.wav`
- `ambient_machinery_02.wav`

**Imported Assets:**
- `Content/Audio/Ambient/Station/ambient_machinery_01`
- `Content/Audio/Ambient/Station/ambient_machinery_02`

**Sound Cue:**
- `Content/Audio/SoundCues/Ambient/SC_Ambient_Station_Machinery`

**Properties:**
- Duration: 20-40 seconds each
- Sample Rate: 44100 Hz
- Bit Depth: 16-bit
- Channels: Stereo
- Looping: Yes
- Compression: OGG Vorbis 40%

**Usage:**
- BP_AudioZone_Station (Layer 1)
- Mechanical detail layer
- Volume: 0.3
- Attenuation: ATT_Ambient_Medium (3000 units)

**Description:**
Mechanical clanks, distant machinery, and industrial sounds for station detail layer.

---

#### Station Water

**Source Files:**
- `ambient_water_drip_01.wav`
- `ambient_water_flow_01.wav`

**Imported Assets:**
- `Content/Audio/Ambient/Station/ambient_water_drip_01`
- `Content/Audio/Ambient/Station/ambient_water_flow_01`

**Sound Cue:**
- `Content/Audio/SoundCues/Ambient/SC_Ambient_Station_Water`

**Properties:**
- Duration: 10-30 seconds each
- Sample Rate: 44100 Hz
- Bit Depth: 16-bit
- Channels: Stereo
- Looping: Yes
- Compression: OGG Vorbis 40%

**Usage:**
- BP_AudioZone_Station (Layer 2)
- Water detail layer
- Volume: 0.35
- Attenuation: ATT_Ambient_Medium (2000 units)

**Description:**
Water drips and flowing water sounds for environmental detail in station areas.

---

#### Underwater Ambience

**Source Files:**
- `ambient_underwater_01.wav`

**Imported Assets:**
- `Content/Audio/Ambient/Underwater/ambient_underwater_01`

**Sound Cue:**
- `Content/Audio/SoundCues/Ambient/SC_Ambient_Underwater`

**Properties:**
- Duration: 60 seconds
- Sample Rate: 44100 Hz
- Bit Depth: 16-bit
- Channels: Stereo
- Looping: Yes
- Compression: OGG Vorbis 40%
- Low Pass Filter: 1000 Hz

**Usage:**
- BP_AudioZone_Underwater
- UnderwaterAudioComponent
- Volume: 0.6
- Attenuation: ATT_Underwater (8000 units)

**Description:**
Deep underwater ambience with pressure sounds and muffled environment.

---

#### Tension Ambience

**Source Files:**
- `ambient_tension_01.wav`

**Imported Assets:**
- `Content/Audio/Ambient/Tension/ambient_tension_01`

**Sound Cue:**
- `Content/Audio/SoundCues/Ambient/SC_Ambient_Tension`

**Properties:**
- Duration: 45 seconds
- Sample Rate: 44100 Hz
- Bit Depth: 16-bit
- Channels: Stereo
- Looping: Yes
- Compression: OGG Vorbis 40%

**Usage:**
- Threat detection system
- Dynamic tension layer
- Volume: 0.3 (modulated by threat proximity)
- Attenuation: None (always audible)

**Description:**
Low drones and tension music that increases during threat encounters.

---

### 2. Footstep Audio (15 files)

#### Concrete Footsteps

**Source Files:**
- `footstep_concrete_01.wav`
- `footstep_concrete_02.wav`
- `footstep_concrete_03.wav`

**Imported Assets:**
- `Content/Audio/Footsteps/Concrete/footstep_concrete_01`
- `Content/Audio/Footsteps/Concrete/footstep_concrete_02`
- `Content/Audio/Footsteps/Concrete/footstep_concrete_03`

**Sound Cue:**
- `Content/Audio/SoundCues/Footsteps/SC_Footstep_Concrete`

**Properties:**
- Duration: 0.2-0.4 seconds each
- Sample Rate: 44100 Hz
- Bit Depth: 16-bit
- Channels: Mono
- Looping: No
- Compression: OGG Vorbis 50%
- Pitch Variation: 0.9-1.1
- Volume Variation: 0.9-1.0

**Usage:**
- FootstepAudioComponent
- Physical Material: PM_Concrete
- Volume: 0.6
- Attenuation: ATT_Footsteps (1500 units)

**Description:**
Hard surface footsteps for concrete floors, typical station flooring.

---

#### Metal Footsteps

**Source Files:**
- `footstep_metal_01.wav`
- `footstep_metal_02.wav`
- `footstep_metal_03.wav`

**Imported Assets:**
- `Content/Audio/Footsteps/Metal/footstep_metal_01`
- `Content/Audio/Footsteps/Metal/footstep_metal_02`
- `Content/Audio/Footsteps/Metal/footstep_metal_03`

**Sound Cue:**
- `Content/Audio/SoundCues/Footsteps/SC_Footstep_Metal`

**Properties:**
- Duration: 0.2-0.4 seconds each
- Sample Rate: 44100 Hz
- Bit Depth: 16-bit
- Channels: Mono
- Looping: No
- Compression: OGG Vorbis 50%
- Pitch Variation: 0.9-1.1
- Volume Variation: 0.9-1.0

**Usage:**
- FootstepAudioComponent
- Physical Material: PM_Metal
- Volume: 0.7
- Attenuation: ATT_Footsteps (1500 units)

**Description:**
Metallic clangy footsteps for metal grating and platforms.

---

#### Water Footsteps

**Source Files:**
- `footstep_water_01.wav`
- `footstep_water_02.wav`
- `footstep_water_03.wav`

**Imported Assets:**
- `Content/Audio/Footsteps/Water/footstep_water_01`
- `Content/Audio/Footsteps/Water/footstep_water_02`
- `Content/Audio/Footsteps/Water/footstep_water_03`

**Sound Cue:**
- `Content/Audio/SoundCues/Footsteps/SC_Footstep_Water`

**Properties:**
- Duration: 0.3-0.5 seconds each
- Sample Rate: 44100 Hz
- Bit Depth: 16-bit
- Channels: Mono
- Looping: No
- Compression: OGG Vorbis 50%
- Pitch Variation: 0.95-1.05
- Volume Variation: 0.9-1.0

**Usage:**
- FootstepAudioComponent
- Physical Material: PM_Water
- Swimming movement
- Volume: 0.6
- Attenuation: ATT_Footsteps (1500 units)

**Description:**
Splashing and wet footsteps for shallow water and puddles.

---

#### Grate Footsteps

**Source Files:**
- `footstep_grate_01.wav`
- `footstep_grate_02.wav`
- `footstep_grate_03.wav`

**Imported Assets:**
- `Content/Audio/Footsteps/Grate/footstep_grate_01`
- `Content/Audio/Footsteps/Grate/footstep_grate_02`
- `Content/Audio/Footsteps/Grate/footstep_grate_03`

**Sound Cue:**
- `Content/Audio/SoundCues/Footsteps/SC_Footstep_Grate`

**Properties:**
- Duration: 0.2-0.4 seconds each
- Sample Rate: 44100 Hz
- Bit Depth: 16-bit
- Channels: Mono
- Looping: No
- Compression: OGG Vorbis 50%
- Pitch Variation: 0.9-1.1
- Volume Variation: 0.9-1.0

**Usage:**
- FootstepAudioComponent
- Physical Material: PM_Grate
- Volume: 0.7
- Attenuation: ATT_Footsteps (1500 units)

**Description:**
Hollow metallic footsteps for metal grating walkways.

---

#### Carpet Footsteps

**Source Files:**
- `footstep_carpet_01.wav`
- `footstep_carpet_02.wav`
- `footstep_carpet_03.wav`

**Imported Assets:**
- `Content/Audio/Footsteps/Carpet/footstep_carpet_01`
- `Content/Audio/Footsteps/Carpet/footstep_carpet_02`
- `Content/Audio/Footsteps/Carpet/footstep_carpet_03`

**Sound Cue:**
- `Content/Audio/SoundCues/Footsteps/SC_Footstep_Carpet`

**Properties:**
- Duration: 0.2-0.3 seconds each
- Sample Rate: 44100 Hz
- Bit Depth: 16-bit
- Channels: Mono
- Looping: No
- Compression: OGG Vorbis 50%
- Pitch Variation: 0.95-1.05
- Volume Variation: 0.9-1.0

**Usage:**
- FootstepAudioComponent
- Physical Material: PM_Carpet
- Volume: 0.4
- Attenuation: ATT_Footsteps (1500 units)

**Description:**
Soft muffled footsteps for carpeted areas and crew quarters.

---

### 3. Breathing Audio (5 files)

#### Normal Breathing

**Source Files:**
- `breathing_normal_loop.wav`

**Imported Assets:**
- `Content/Audio/Breathing/Normal/breathing_normal_loop`

**Sound Cue:**
- `Content/Audio/SoundCues/Breathing/SC_Breathing_Normal`

**Properties:**
- Duration: 4 seconds (seamless loop)
- Sample Rate: 44100 Hz
- Bit Depth: 16-bit
- Channels: Mono
- Looping: Yes
- Compression: OGG Vorbis 60%
- Playback Rate: 1.0

**Usage:**
- BreathingAudioComponent
- Default breathing state
- Stress Level: 0.0-0.3
- Volume: 0.35
- Attenuation: ATT_Breathing (500 units)

**Description:**
Calm, relaxed breathing for normal gameplay state.

---

#### Heavy Breathing

**Source Files:**
- `breathing_heavy_loop.wav`

**Imported Assets:**
- `Content/Audio/Breathing/Heavy/breathing_heavy_loop`

**Sound Cue:**
- `Content/Audio/SoundCues/Breathing/SC_Breathing_Heavy`

**Properties:**
- Duration: 3 seconds (seamless loop)
- Sample Rate: 44100 Hz
- Bit Depth: 16-bit
- Channels: Mono
- Looping: Yes
- Compression: OGG Vorbis 60%
- Playback Rate: 1.1-1.3

**Usage:**
- BreathingAudioComponent
- Exertion state
- Stress Level: 0.3-0.6
- Volume: 0.55
- Attenuation: ATT_Breathing (500 units)

**Description:**
Heavy breathing from running or physical exertion.

---

#### Panic Breathing

**Source Files:**
- `breathing_panic_loop.wav`

**Imported Assets:**
- `Content/Audio/Breathing/Panic/breathing_panic_loop`

**Sound Cue:**
- `Content/Audio/SoundCues/Breathing/SC_Breathing_Panic`

**Properties:**
- Duration: 2 seconds (seamless loop)
- Sample Rate: 44100 Hz
- Bit Depth: 16-bit
- Channels: Mono
- Looping: Yes
- Compression: OGG Vorbis 60%
- Playback Rate: 1.4-1.6

**Usage:**
- BreathingAudioComponent
- Panic state
- Stress Level: 0.6-1.0
- Volume: 0.7
- Attenuation: ATT_Breathing (500 units)

**Description:**
Rapid panicked breathing during threat encounters.

---

#### Low Oxygen Breathing

**Source Files:**
- `breathing_low_oxygen_loop.wav`

**Imported Assets:**
- `Content/Audio/Breathing/LowOxygen/breathing_low_oxygen_loop`

**Sound Cue:**
- `Content/Audio/SoundCues/Breathing/SC_Breathing_LowOxygen`

**Properties:**
- Duration: 5 seconds (seamless loop)
- Sample Rate: 44100 Hz
- Bit Depth: 16-bit
- Channels: Mono
- Looping: Yes
- Compression: OGG Vorbis 60%
- Playback Rate: 0.7-0.9

**Usage:**
- BreathingAudioComponent
- Low oxygen state
- Oxygen Level: < 30%
- Volume: 0.8
- Attenuation: ATT_Breathing (500 units)

**Description:**
Gasping, struggling breathing when oxygen is low.

---

#### Hold Breath

**Source Files:**
- `breathing_hold_breath.wav`

**Imported Assets:**
- `Content/Audio/Breathing/breathing_hold_breath`

**Sound Cue:**
- `Content/Audio/SoundCues/Breathing/SC_Breathing_HoldBreath`

**Properties:**
- Duration: 3 seconds (one-shot)
- Sample Rate: 44100 Hz
- Bit Depth: 16-bit
- Channels: Mono
- Looping: No
- Compression: OGG Vorbis 60%

**Usage:**
- BreathingAudioComponent
- Hold breath action
- Volume: 0.5
- Attenuation: ATT_Breathing (500 units)

**Description:**
Deep inhale, hold, and exhale sequence for stealth gameplay.

---

### 4. Underwater Audio (5 files)

#### Underwater Ambience

**Source Files:**
- `underwater_ambience_loop.wav`

**Imported Assets:**
- `Content/Audio/Underwater/Ambience/underwater_ambience_loop`

**Sound Cue:**
- `Content/Audio/SoundCues/Underwater/SC_Underwater_Ambience`

**Properties:**
- Duration: 60 seconds (seamless loop)
- Sample Rate: 44100 Hz
- Bit Depth: 16-bit
- Channels: Stereo
- Looping: Yes
- Compression: OGG Vorbis 40%
- Low Pass Filter: 1000 Hz

**Usage:**
- UnderwaterAudioComponent
- Underwater zones
- Volume: 0.6
- Attenuation: ATT_Underwater (8000 units)

**Description:**
Deep underwater ambience with pressure sounds and muffled environment.

---

#### Underwater Bubbles

**Source Files:**
- `underwater_bubbles_01.wav`

**Imported Assets:**
- `Content/Audio/Underwater/Bubbles/underwater_bubbles_01`

**Sound Cue:**
- `Content/Audio/SoundCues/Underwater/SC_Underwater_Bubbles`

**Properties:**
- Duration: 1-2 seconds
- Sample Rate: 44100 Hz
- Bit Depth: 16-bit
- Channels: Mono
- Looping: No
- Compression: OGG Vorbis 40%
- Pitch Variation: 0.8-1.2
- Volume Variation: 0.8-1.0

**Usage:**
- UnderwaterAudioComponent
- Random bubble sounds
- Volume: 0.5
- Attenuation: ATT_Underwater (8000 units)

**Description:**
Bubble sounds that play randomly while underwater.

---

#### Underwater Movement

**Source Files:**
- `underwater_movement_loop.wav`

**Imported Assets:**
- `Content/Audio/Underwater/Movement/underwater_movement_loop`

**Sound Cue:**
- `Content/Audio/SoundCues/Underwater/SC_Underwater_Movement`

**Properties:**
- Duration: 3 seconds (seamless loop)
- Sample Rate: 44100 Hz
- Bit Depth: 16-bit
- Channels: Stereo
- Looping: Yes
- Compression: OGG Vorbis 40%

**Usage:**
- UnderwaterAudioComponent
- Swimming movement
- Volume: 0.4 (modulated by speed)
- Attenuation: ATT_Underwater (8000 units)

**Description:**
Water movement and swimming sounds while moving underwater.

---

#### Underwater Enter

**Source Files:**
- `underwater_enter.wav`

**Imported Assets:**
- `Content/Audio/Underwater/underwater_enter`

**Sound Cue:**
- `Content/Audio/SoundCues/Underwater/SC_Underwater_Enter`

**Properties:**
- Duration: 1.5 seconds
- Sample Rate: 44100 Hz
- Bit Depth: 16-bit
- Channels: Stereo
- Looping: No
- Compression: OGG Vorbis 40%

**Usage:**
- UnderwaterAudioComponent
- Water entry event
- Volume: 0.8
- Attenuation: None (2D sound)

**Description:**
Splash and submersion sound when entering water.

---

#### Underwater Exit

**Source Files:**
- `underwater_exit.wav`

**Imported Assets:**
- `Content/Audio/Underwater/underwater_exit`

**Sound Cue:**
- `Content/Audio/SoundCues/Underwater/SC_Underwater_Exit`

**Properties:**
- Duration: 1.5 seconds
- Sample Rate: 44100 Hz
- Bit Depth: 16-bit
- Channels: Stereo
- Looping: No
- Compression: OGG Vorbis 40%

**Usage:**
- UnderwaterAudioComponent
- Water exit event
- Volume: 0.8
- Attenuation: None (2D sound)

**Description:**
Surface break and gasp sound when exiting water.

---

## Asset Summary

### By Category

| Category | Source Files | Sound Waves | Sound Cues | Total Size (Est.) |
|----------|--------------|-------------|------------|-------------------|
| Ambient | 8 | 8 | 5 | 15 MB |
| Footsteps | 15 | 15 | 5 | 5 MB |
| Breathing | 5 | 5 | 5 | 8 MB |
| Underwater | 5 | 5 | 5 | 10 MB |
| **Total** | **33** | **33** | **20** | **38 MB** |

### By Usage

| Component | Sound Cues Used | Audio Files Used |
|-----------|-----------------|------------------|
| AmbientAudioComponent | 5 | 8 |
| FootstepAudioComponent | 5 | 15 |
| BreathingAudioComponent | 5 | 5 |
| UnderwaterAudioComponent | 5 | 5 |
| **Total** | **20** | **33** |

### Compression Settings

| Category | Compression Quality | Format | Streaming |
|----------|---------------------|--------|-----------|
| Ambient | 40% | OGG Vorbis | Yes (> 5s) |
| Footsteps | 50% | OGG Vorbis | No |
| Breathing | 60% | OGG Vorbis | No |
| Underwater | 40% | OGG Vorbis | Yes (> 5s) |

### Attenuation Settings

| Attenuation Asset | Falloff Distance | Used By |
|-------------------|------------------|---------|
| ATT_Ambient_Large | 5000 units | Station Base |
| ATT_Ambient_Medium | 3000 units | Machinery, Water |
| ATT_Footsteps | 1500 units | All Footsteps |
| ATT_Breathing | 500 units | All Breathing |
| ATT_Underwater | 8000 units | Underwater Sounds |

## File Naming Conventions

### Source Files
- `[category]_[type]_[variation].wav`
- Example: `footstep_concrete_01.wav`

### Imported Assets
- `Content/Audio/[Category]/[Type]/[name]`
- Example: `Content/Audio/Footsteps/Concrete/footstep_concrete_01`

### Sound Cues
- `SC_[Category]_[Type]`
- Example: `SC_Footstep_Concrete`

### Attenuation Assets
- `ATT_[Category]_[Size]`
- Example: `ATT_Ambient_Large`

## Quick Reference

### Find Asset by Source File

Use this table to quickly locate assets:

| Source File | Content Path | Sound Cue |
|-------------|--------------|-----------|
| ambient_station_base_01.wav | Audio/Ambient/Station/ | SC_Ambient_Station_Base |
| footstep_concrete_01.wav | Audio/Footsteps/Concrete/ | SC_Footstep_Concrete |
| breathing_normal_loop.wav | Audio/Breathing/Normal/ | SC_Breathing_Normal |
| underwater_ambience_loop.wav | Audio/Underwater/Ambience/ | SC_Underwater_Ambience |

### Find Usage by Component

| Component | Configuration Location | Sound Cues |
|-----------|------------------------|------------|
| AmbientAudioComponent | BP_AudioZone_Station | SC_Ambient_Station_* |
| FootstepAudioComponent | BP_PlayerCharacter | SC_Footstep_* |
| BreathingAudioComponent | BP_PlayerCharacter | SC_Breathing_* |
| UnderwaterAudioComponent | BP_AudioZone_Underwater | SC_Underwater_* |

## Verification Checklist

Use this checklist to verify all assets are properly mapped:

- [ ] All 33 source files accounted for
- [ ] All 33 sound waves imported
- [ ] All 20 sound cues created
- [ ] All 5 attenuation assets created
- [ ] All sound cues assigned to components
- [ ] All compression settings correct
- [ ] All looping settings correct
- [ ] All attenuation settings correct
- [ ] All file paths match conventions
- [ ] All assets organized correctly
