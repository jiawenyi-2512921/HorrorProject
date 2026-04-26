# Audio Migration Step-by-Step Guide

## Overview

This guide provides a complete step-by-step process for migrating 33 audio files into the HorrorProject audio system. The migration includes importing, organizing, configuring, and integrating all audio assets.

## Prerequisites

- Unreal Engine 5.6 installed
- HorrorProject opened in UE5.6
- Source audio files (33 files) ready for import
- Audio components implemented in C++

## Migration Phases

### Phase 1: Preparation (30 minutes)

#### Step 1.1: Create Directory Structure

Create the following folder structure in Content Browser:

```
Content/Audio/
├── Ambient/
│   ├── Station/
│   ├── Underwater/
│   └── Tension/
├── Footsteps/
│   ├── Concrete/
│   ├── Metal/
│   ├── Water/
│   ├── Grate/
│   └── Carpet/
├── Breathing/
│   ├── Normal/
│   ├── Heavy/
│   ├── Panic/
│   └── LowOxygen/
├── Underwater/
│   ├── Ambience/
│   ├── Bubbles/
│   └── Movement/
├── SoundCues/
│   ├── Ambient/
│   ├── Footsteps/
│   ├── Breathing/
│   └── Underwater/
├── Attenuation/
└── Mixes/
```

**Actions:**
1. Open Content Browser
2. Navigate to Content/
3. Right-click → New Folder → "Audio"
4. Create all subfolders as shown above

#### Step 1.2: Verify Audio Files

Verify you have all 33 audio files:

**Ambient Audio (8 files):**
- ambient_station_base_01.wav
- ambient_station_base_02.wav
- ambient_machinery_01.wav
- ambient_machinery_02.wav
- ambient_water_drip_01.wav
- ambient_water_flow_01.wav
- ambient_underwater_01.wav
- ambient_tension_01.wav

**Footstep Audio (15 files):**
- footstep_concrete_01.wav through footstep_concrete_03.wav
- footstep_metal_01.wav through footstep_metal_03.wav
- footstep_water_01.wav through footstep_water_03.wav
- footstep_grate_01.wav through footstep_grate_03.wav
- footstep_carpet_01.wav through footstep_carpet_03.wav

**Breathing Audio (5 files):**
- breathing_normal_loop.wav
- breathing_heavy_loop.wav
- breathing_panic_loop.wav
- breathing_low_oxygen_loop.wav
- breathing_hold_breath.wav

**Underwater Audio (5 files):**
- underwater_ambience_loop.wav
- underwater_bubbles_01.wav
- underwater_movement_loop.wav
- underwater_enter.wav
- underwater_exit.wav

**Total: 33 files**

#### Step 1.3: Backup Current Project

1. Close Unreal Editor
2. Navigate to project folder
3. Create backup: `HorrorProject_Backup_[Date]`
4. Copy entire project folder
5. Reopen project

### Phase 2: Audio Import (1 hour)

#### Step 2.1: Import Ambient Audio

**Location:** Content/Audio/Ambient/

**Import Settings:**
- Sample Rate: 44100 Hz
- Compression Quality: 40 (OGG Vorbis)
- Looping: Yes (for loops)
- Sound Group: Ambient

**Process:**
1. Select all ambient audio files
2. Drag into Content/Audio/Ambient/
3. In Import Dialog:
   - Set Compression Quality: 40
   - Enable "Auto Create Cue"
   - Click "Import All"
4. Organize into subfolders:
   - Station files → Ambient/Station/
   - Underwater files → Ambient/Underwater/
   - Tension files → Ambient/Tension/

**Verify:**
- 8 Sound Wave assets created
- No import errors
- Files play correctly in preview

#### Step 2.2: Import Footstep Audio

**Location:** Content/Audio/Footsteps/

**Import Settings:**
- Sample Rate: 44100 Hz
- Compression Quality: 50 (higher quality for clarity)
- Looping: No
- Sound Group: SFX

**Process:**
1. Select all footstep audio files
2. Drag into Content/Audio/Footsteps/
3. In Import Dialog:
   - Set Compression Quality: 50
   - Disable "Auto Create Cue" (we'll create custom cues)
   - Click "Import All"
4. Organize into subfolders by surface type:
   - Concrete files → Footsteps/Concrete/
   - Metal files → Footsteps/Metal/
   - Water files → Footsteps/Water/
   - Grate files → Footsteps/Grate/
   - Carpet files → Footsteps/Carpet/

**Verify:**
- 15 Sound Wave assets created
- Each surface type has 3 variations
- Files play correctly in preview

#### Step 2.3: Import Breathing Audio

**Location:** Content/Audio/Breathing/

**Import Settings:**
- Sample Rate: 44100 Hz
- Compression Quality: 60 (highest quality for player audio)
- Looping: Yes (for loop files)
- Sound Group: Voice

**Process:**
1. Select all breathing audio files
2. Drag into Content/Audio/Breathing/
3. In Import Dialog:
   - Set Compression Quality: 60
   - Enable "Auto Create Cue"
   - Click "Import All"
4. Organize into subfolders:
   - Normal → Breathing/Normal/
   - Heavy → Breathing/Heavy/
   - Panic → Breathing/Panic/
   - Low Oxygen → Breathing/LowOxygen/

**Verify:**
- 5 Sound Wave assets created
- Loop files loop seamlessly
- No clicking or popping

#### Step 2.4: Import Underwater Audio

**Location:** Content/Audio/Underwater/

**Import Settings:**
- Sample Rate: 44100 Hz
- Compression Quality: 40
- Looping: Yes (for loops)
- Sound Group: Ambient

**Process:**
1. Select all underwater audio files
2. Drag into Content/Audio/Underwater/
3. In Import Dialog:
   - Set Compression Quality: 40
   - Enable "Auto Create Cue"
   - Click "Import All"
4. Organize into subfolders:
   - Ambience → Underwater/Ambience/
   - Bubbles → Underwater/Bubbles/
   - Movement → Underwater/Movement/

**Verify:**
- 5 Sound Wave assets created
- All files play correctly
- Underwater ambience loops seamlessly

### Phase 3: Sound Cue Creation (2 hours)

#### Step 3.1: Create Ambient Sound Cues

**Location:** Content/Audio/SoundCues/Ambient/

##### SC_Ambient_Station_Base

1. Right-click in Content/Audio/SoundCues/Ambient/
2. Sound → Sound Cue
3. Name: "SC_Ambient_Station_Base"
4. Open Sound Cue Editor
5. Add nodes:
   - Random node (2 inputs)
   - Connect ambient_station_base_01 to input 0
   - Connect ambient_station_base_02 to input 1
   - Set weights: 0.5, 0.5
6. Add Looping node
7. Add Volume node (0.4)
8. Connect to Output
9. Save

##### SC_Ambient_Station_Machinery

1. Create new Sound Cue
2. Name: "SC_Ambient_Station_Machinery"
3. Add nodes:
   - Random node (2 inputs)
   - Connect ambient_machinery_01 and ambient_machinery_02
   - Set weights: 0.5, 0.5
4. Add Looping node
5. Add Volume node (0.3)
6. Add Modulator node:
   - Pitch Min: 0.95
   - Pitch Max: 1.05
7. Connect to Output
8. Save

##### SC_Ambient_Station_Water

1. Create new Sound Cue
2. Name: "SC_Ambient_Station_Water"
3. Add nodes:
   - Random node (2 inputs)
   - Connect ambient_water_drip_01 and ambient_water_flow_01
   - Set weights: 0.6, 0.4
4. Add Looping node
5. Add Volume node (0.35)
6. Add Delay node:
   - Delay Min: 0.5
   - Delay Max: 2.0
7. Connect to Output
8. Save

##### SC_Ambient_Underwater

1. Create new Sound Cue
2. Name: "SC_Ambient_Underwater"
3. Add nodes:
   - Connect underwater_ambience_loop
4. Add Looping node
5. Add Volume node (0.6)
6. Add Low Pass Filter:
   - Frequency: 1000 Hz
7. Connect to Output
8. Save

##### SC_Ambient_Tension

1. Create new Sound Cue
2. Name: "SC_Ambient_Tension"
3. Add nodes:
   - Connect ambient_tension_01
4. Add Looping node
5. Add Volume node (0.3)
6. Add Modulator node:
   - Volume Min: 0.8
   - Volume Max: 1.2
7. Connect to Output
8. Save

#### Step 3.2: Create Footstep Sound Cues

**Location:** Content/Audio/SoundCues/Footsteps/

##### SC_Footstep_Concrete

1. Create new Sound Cue
2. Name: "SC_Footstep_Concrete"
3. Add nodes:
   - Random node (3 inputs)
   - Connect footstep_concrete_01, 02, 03
   - Set weights: 0.33, 0.34, 0.33
4. Add Modulator node:
   - Pitch Min: 0.9
   - Pitch Max: 1.1
   - Volume Min: 0.9
   - Volume Max: 1.0
5. Add Volume node (0.6)
6. Connect to Output
7. Save

##### SC_Footstep_Metal

1. Create new Sound Cue
2. Name: "SC_Footstep_Metal"
3. Add nodes:
   - Random node (3 inputs)
   - Connect footstep_metal_01, 02, 03
   - Set weights: 0.33, 0.34, 0.33
4. Add Modulator node:
   - Pitch Min: 0.9
   - Pitch Max: 1.1
   - Volume Min: 0.9
   - Volume Max: 1.0
5. Add Volume node (0.7)
6. Connect to Output
7. Save

##### SC_Footstep_Water

1. Create new Sound Cue
2. Name: "SC_Footstep_Water"
3. Add nodes:
   - Random node (3 inputs)
   - Connect footstep_water_01, 02, 03
   - Set weights: 0.33, 0.34, 0.33
4. Add Modulator node:
   - Pitch Min: 0.95
   - Pitch Max: 1.05
   - Volume Min: 0.9
   - Volume Max: 1.0
5. Add Volume node (0.6)
6. Connect to Output
7. Save

##### SC_Footstep_Grate

1. Create new Sound Cue
2. Name: "SC_Footstep_Grate"
3. Add nodes:
   - Random node (3 inputs)
   - Connect footstep_grate_01, 02, 03
   - Set weights: 0.33, 0.34, 0.33
4. Add Modulator node:
   - Pitch Min: 0.9
   - Pitch Max: 1.1
   - Volume Min: 0.9
   - Volume Max: 1.0
5. Add Volume node (0.7)
6. Connect to Output
7. Save

##### SC_Footstep_Carpet

1. Create new Sound Cue
2. Name: "SC_Footstep_Carpet"
3. Add nodes:
   - Random node (3 inputs)
   - Connect footstep_carpet_01, 02, 03
   - Set weights: 0.33, 0.34, 0.33
4. Add Modulator node:
   - Pitch Min: 0.95
   - Pitch Max: 1.05
   - Volume Min: 0.9
   - Volume Max: 1.0
5. Add Volume node (0.4)
6. Connect to Output
7. Save

#### Step 3.3: Create Breathing Sound Cues

**Location:** Content/Audio/SoundCues/Breathing/

##### SC_Breathing_Normal

1. Create new Sound Cue
2. Name: "SC_Breathing_Normal"
3. Add nodes:
   - Connect breathing_normal_loop
4. Add Looping node
5. Add Volume node (0.35)
6. Connect to Output
7. Save

##### SC_Breathing_Heavy

1. Create new Sound Cue
2. Name: "SC_Breathing_Heavy"
3. Add nodes:
   - Connect breathing_heavy_loop
4. Add Looping node
5. Add Volume node (0.55)
6. Add Modulator node:
   - Pitch Min: 1.1
   - Pitch Max: 1.3
7. Connect to Output
8. Save

##### SC_Breathing_Panic

1. Create new Sound Cue
2. Name: "SC_Breathing_Panic"
3. Add nodes:
   - Connect breathing_panic_loop
4. Add Looping node
5. Add Volume node (0.7)
6. Add Modulator node:
   - Pitch Min: 1.4
   - Pitch Max: 1.6
7. Connect to Output
8. Save

##### SC_Breathing_LowOxygen

1. Create new Sound Cue
2. Name: "SC_Breathing_LowOxygen"
3. Add nodes:
   - Connect breathing_low_oxygen_loop
4. Add Looping node
5. Add Volume node (0.8)
6. Add Modulator node:
   - Pitch Min: 0.7
   - Pitch Max: 0.9
7. Connect to Output
8. Save

##### SC_Breathing_HoldBreath

1. Create new Sound Cue
2. Name: "SC_Breathing_HoldBreath"
3. Add nodes:
   - Connect breathing_hold_breath
4. Add Volume node (0.5)
5. Connect to Output
6. Save

#### Step 3.4: Create Underwater Sound Cues

**Location:** Content/Audio/SoundCues/Underwater/

##### SC_Underwater_Ambience

1. Create new Sound Cue
2. Name: "SC_Underwater_Ambience"
3. Add nodes:
   - Connect underwater_ambience_loop
4. Add Looping node
5. Add Volume node (0.6)
6. Add Low Pass Filter:
   - Frequency: 1000 Hz
7. Connect to Output
8. Save

##### SC_Underwater_Bubbles

1. Create new Sound Cue
2. Name: "SC_Underwater_Bubbles"
3. Add nodes:
   - Connect underwater_bubbles_01
4. Add Modulator node:
   - Pitch Min: 0.8
   - Pitch Max: 1.2
   - Volume Min: 0.8
   - Volume Max: 1.0
5. Add Volume node (0.5)
6. Connect to Output
7. Save

##### SC_Underwater_Movement

1. Create new Sound Cue
2. Name: "SC_Underwater_Movement"
3. Add nodes:
   - Connect underwater_movement_loop
4. Add Looping node
5. Add Volume node (0.4)
6. Connect to Output
7. Save

##### SC_Underwater_Enter

1. Create new Sound Cue
2. Name: "SC_Underwater_Enter"
3. Add nodes:
   - Connect underwater_enter
4. Add Volume node (0.8)
5. Connect to Output
6. Save

##### SC_Underwater_Exit

1. Create new Sound Cue
2. Name: "SC_Underwater_Exit"
3. Add nodes:
   - Connect underwater_exit
4. Add Volume node (0.8)
5. Connect to Output
6. Save

### Phase 4: Attenuation Setup (30 minutes)

See AudioAttenuationSetup.md for detailed configuration.

**Quick Steps:**
1. Create 5 attenuation assets in Content/Audio/Attenuation/
2. Configure each with appropriate falloff distances
3. Assign to corresponding sound cues

### Phase 5: Sound Class Configuration (30 minutes)

See AudioClassConfiguration.md for detailed setup.

**Quick Steps:**
1. Open Project Settings → Audio
2. Create sound class hierarchy
3. Configure volume and effects for each class
4. Assign sound cues to appropriate classes

### Phase 6: Integration (1 hour)

#### Step 6.1: Configure Player Character

1. Open BP_PlayerCharacter
2. Add FootstepAudioComponent
3. Add BreathingAudioComponent
4. Configure component properties:
   - Assign all footstep sound cues
   - Assign all breathing sound cues
   - Set timing parameters
5. Implement event graph logic
6. Compile and save

#### Step 6.2: Create Audio Zones

1. Create BP_AudioZone_Station
2. Add AmbientAudioComponent
3. Configure with station sound cues
4. Place in level
5. Test overlap triggers

#### Step 6.3: Create Underwater Zones

1. Create BP_AudioZone_Underwater
2. Add UnderwaterAudioComponent
3. Configure with underwater sound cues
4. Place in level
5. Test water entry/exit

### Phase 7: Testing (1 hour)

#### Step 7.1: Component Testing

Test each audio component individually:

**Ambient Audio:**
- [ ] Station ambience plays in zones
- [ ] Layers blend correctly
- [ ] Volume fades work
- [ ] Attenuation functions properly

**Footstep Audio:**
- [ ] Footsteps play on all surfaces
- [ ] Surface detection works
- [ ] Speed affects playback
- [ ] Crouch reduces volume

**Breathing Audio:**
- [ ] Normal breathing loops
- [ ] Stress increases breathing rate
- [ ] Low oxygen triggers gasping
- [ ] Panic mode works

**Underwater Audio:**
- [ ] Underwater ambience plays
- [ ] Muffling effect works
- [ ] Bubbles play randomly
- [ ] Entry/exit sounds trigger

#### Step 7.2: Integration Testing

Test complete audio system:

- [ ] Multiple audio sources play simultaneously
- [ ] No audio popping or clicking
- [ ] Volume mixing is balanced
- [ ] Performance is acceptable (< 5% CPU)
- [ ] Memory usage is reasonable (< 100 MB)

#### Step 7.3: Quality Assurance

- [ ] All 33 audio files are used
- [ ] No missing sound cues
- [ ] All attenuation settings correct
- [ ] Sound classes assigned properly
- [ ] No audio artifacts

### Phase 8: Optimization (30 minutes)

#### Step 8.1: Compression Optimization

1. Review all audio assets
2. Adjust compression quality:
   - Ambient: 40
   - Footsteps: 50
   - Breathing: 60
   - Underwater: 40
3. Enable streaming for long files (> 5 seconds)
4. Verify quality is acceptable

#### Step 8.2: Performance Optimization

1. Enable audio culling
2. Set max concurrent sounds: 32
3. Configure audio LOD
4. Test performance with stat commands:
   - `stat sounds`
   - `stat soundwaves`
   - `stat soundmixes`

### Phase 9: Documentation (30 minutes)

#### Step 9.1: Create Asset List

Document all created assets:
- 33 Sound Wave assets
- 18 Sound Cue assets
- 5 Attenuation assets
- Sound class hierarchy
- 2 Sound mix modifiers

#### Step 9.2: Create Usage Guide

Document how to use the audio system:
- Component configuration
- Blueprint integration
- Common use cases
- Troubleshooting

### Phase 10: Final Verification (30 minutes)

#### Step 10.1: Complete Checklist

- [ ] All 33 audio files imported
- [ ] All sound cues created (18 total)
- [ ] All attenuation assets created (5 total)
- [ ] Sound classes configured
- [ ] Audio components integrated
- [ ] Audio zones placed
- [ ] Testing completed
- [ ] Performance optimized
- [ ] Documentation complete

#### Step 10.2: Commit Changes

1. Save all assets
2. Close Unreal Editor
3. Commit to version control:
   ```
   git add Content/Audio/
   git commit -m "Audio: Complete audio system migration (33 files)"
   ```

## Timeline Summary

- Phase 1: Preparation - 30 minutes
- Phase 2: Import - 1 hour
- Phase 3: Sound Cues - 2 hours
- Phase 4: Attenuation - 30 minutes
- Phase 5: Sound Classes - 30 minutes
- Phase 6: Integration - 1 hour
- Phase 7: Testing - 1 hour
- Phase 8: Optimization - 30 minutes
- Phase 9: Documentation - 30 minutes
- Phase 10: Verification - 30 minutes

**Total Time: 8 hours**

## Troubleshooting

### Import Issues

**Problem:** Audio files won't import
**Solution:**
- Verify file format (WAV, MP3, OGG)
- Check file isn't corrupted
- Ensure file size < 100 MB
- Try importing one at a time

**Problem:** Import dialog doesn't appear
**Solution:**
- Use File → Import instead of drag-drop
- Check Content Browser is in correct folder
- Restart Unreal Editor

### Sound Cue Issues

**Problem:** Sound Cue doesn't play
**Solution:**
- Verify sound wave is connected
- Check volume nodes aren't at 0
- Ensure looping is set correctly
- Test sound wave directly

**Problem:** Random node always plays same sound
**Solution:**
- Check weights are set correctly
- Verify all inputs are connected
- Test with different random seeds

### Integration Issues

**Problem:** Component doesn't play audio
**Solution:**
- Verify component is active
- Check sound cue is assigned
- Ensure component is ticking
- Check volume levels

**Problem:** Footsteps don't trigger
**Solution:**
- Verify movement speed > threshold
- Check surface detection logic
- Ensure footstep sounds assigned
- Test with debug prints

## Next Steps

After completing migration:

1. Review AudioComponentUsage guides
2. Implement advanced features
3. Create additional audio zones
4. Add music system
5. Implement dialogue system
6. Create UI sounds
7. Add reverb zones
8. Implement audio occlusion

## Support

For issues or questions:
- Check AudioDebugging.md
- Review AudioTroubleshooting.md
- Consult Audio_Blueprint_Guide.md
- Test with minimal setup first
