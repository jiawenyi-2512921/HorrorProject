# Audio Import Settings Guide

## Overview

Detailed configuration guide for importing all 33 audio files with optimal settings for quality, performance, and memory usage.

## General Import Principles

### Quality vs Performance

- **High Quality (60%)**: Player-centric sounds (breathing, dialogue)
- **Medium Quality (50%)**: Frequent sounds (footsteps, interactions)
- **Low Quality (40%)**: Ambient sounds, background audio

### Streaming vs Loaded

- **Stream**: Files > 5 seconds, ambient loops, music
- **Load**: Files < 5 seconds, one-shots, frequent sounds

### Mono vs Stereo

- **Mono**: Footsteps, small sounds, point sources
- **Stereo**: Ambient loops, music, large environmental sounds

## Import Settings by Category

### 1. Ambient Audio Import Settings

#### Station Base Ambience (2 files)

**Files:**
- ambient_station_base_01.wav
- ambient_station_base_02.wav

**Import Dialog Settings:**
```
Compression Quality: 40
Sample Rate Setting: High (44100 Hz)
Sound Group: Ambient
Load Type: Streaming
Enable Baked Analysis: Yes
```

**Asset Properties (after import):**
```
Looping: Yes
Volume: 1.0 (adjusted in Sound Cue)
Pitch: 1.0
Attenuation Settings: ATT_Ambient_Large
Sound Class: Ambience/Environmental
Priority: 0.5
```

**Advanced Settings:**
```
Enable Seek: Yes (for streaming)
Virtualization Mode: Restart
Concurrency Settings: None (ambient always plays)
```

**Rationale:**
- 40% compression: Ambient sounds can tolerate more compression
- Streaming: Long duration files (30-60s) save memory
- Stereo: Wide soundscape for immersion

---

#### Station Machinery (2 files)

**Files:**
- ambient_machinery_01.wav
- ambient_machinery_02.wav

**Import Dialog Settings:**
```
Compression Quality: 40
Sample Rate Setting: High (44100 Hz)
Sound Group: Ambient
Load Type: Streaming
Enable Baked Analysis: Yes
```

**Asset Properties:**
```
Looping: Yes
Volume: 1.0
Pitch: 1.0
Attenuation Settings: ATT_Ambient_Medium
Sound Class: Ambience/Environmental
Priority: 0.5
```

**Advanced Settings:**
```
Enable Seek: Yes
Virtualization Mode: Restart
Concurrency Settings: Max 2 instances
```

---

#### Station Water (2 files)

**Files:**
- ambient_water_drip_01.wav
- ambient_water_flow_01.wav

**Import Dialog Settings:**
```
Compression Quality: 40
Sample Rate Setting: High (44100 Hz)
Sound Group: Ambient
Load Type: Streaming
Enable Baked Analysis: Yes
```

**Asset Properties:**
```
Looping: Yes
Volume: 1.0
Pitch: 1.0
Attenuation Settings: ATT_Ambient_Medium
Sound Class: Ambience/Environmental
Priority: 0.5
```

**Advanced Settings:**
```
Enable Seek: Yes
Virtualization Mode: Restart
Concurrency Settings: Max 4 instances
```

---

#### Underwater Ambience (1 file)

**Files:**
- ambient_underwater_01.wav

**Import Dialog Settings:**
```
Compression Quality: 40
Sample Rate Setting: High (44100 Hz)
Sound Group: Ambient
Load Type: Streaming
Enable Baked Analysis: Yes
```

**Asset Properties:**
```
Looping: Yes
Volume: 1.0
Pitch: 1.0
Attenuation Settings: ATT_Underwater
Sound Class: Ambience/Underwater
Priority: 0.7
```

**Advanced Settings:**
```
Enable Seek: Yes
Virtualization Mode: Restart
Concurrency Settings: Max 1 instance
Submix: Underwater_Submix
```

---

#### Tension Ambience (1 file)

**Files:**
- ambient_tension_01.wav

**Import Dialog Settings:**
```
Compression Quality: 40
Sample Rate Setting: High (44100 Hz)
Sound Group: Music
Load Type: Streaming
Enable Baked Analysis: Yes
```

**Asset Properties:**
```
Looping: Yes
Volume: 1.0
Pitch: 1.0
Attenuation Settings: None (2D sound)
Sound Class: Music
Priority: 0.8
```

**Advanced Settings:**
```
Enable Seek: Yes
Virtualization Mode: Play When Silent
Concurrency Settings: Max 1 instance
```

---

### 2. Footstep Audio Import Settings

#### All Footstep Variations (15 files)

**Files:**
- footstep_concrete_01/02/03.wav
- footstep_metal_01/02/03.wav
- footstep_water_01/02/03.wav
- footstep_grate_01/02/03.wav
- footstep_carpet_01/02/03.wav

**Import Dialog Settings:**
```
Compression Quality: 50
Sample Rate Setting: High (44100 Hz)
Sound Group: SFX
Load Type: Load on Demand
Enable Baked Analysis: Yes
```

**Asset Properties:**
```
Looping: No
Volume: 1.0
Pitch: 1.0
Attenuation Settings: ATT_Footsteps
Sound Class: SFX/Footsteps
Priority: 0.6
```

**Advanced Settings:**
```
Enable Seek: No (short files)
Virtualization Mode: Stop
Concurrency Settings: Max 4 instances per surface type
Concurrency Resolution Rule: Stop Oldest
```

**Rationale:**
- 50% compression: Higher quality for clarity
- Load on Demand: Frequently used, need quick access
- Mono: Point source, saves memory
- Concurrency limit: Prevent footstep spam

---

### 3. Breathing Audio Import Settings

#### Normal Breathing (1 file)

**Files:**
- breathing_normal_loop.wav

**Import Dialog Settings:**
```
Compression Quality: 60
Sample Rate Setting: High (44100 Hz)
Sound Group: Voice
Load Type: Load on Demand
Enable Baked Analysis: Yes
```

**Asset Properties:**
```
Looping: Yes
Volume: 1.0
Pitch: 1.0
Attenuation Settings: ATT_Breathing
Sound Class: Voice/Player
Priority: 0.9
```

**Advanced Settings:**
```
Enable Seek: No
Virtualization Mode: Play When Silent
Concurrency Settings: Max 1 instance
```

**Rationale:**
- 60% compression: Highest quality for player audio
- Mono: Close to player, no need for stereo
- High priority: Always audible

---

#### Heavy Breathing (1 file)

**Files:**
- breathing_heavy_loop.wav

**Import Dialog Settings:**
```
Compression Quality: 60
Sample Rate Setting: High (44100 Hz)
Sound Group: Voice
Load Type: Load on Demand
Enable Baked Analysis: Yes
```

**Asset Properties:**
```
Looping: Yes
Volume: 1.0
Pitch: 1.0
Attenuation Settings: ATT_Breathing
Sound Class: Voice/Player
Priority: 0.9
```

**Advanced Settings:**
```
Enable Seek: No
Virtualization Mode: Play When Silent
Concurrency Settings: Max 1 instance
```

---

#### Panic Breathing (1 file)

**Files:**
- breathing_panic_loop.wav

**Import Dialog Settings:**
```
Compression Quality: 60
Sample Rate Setting: High (44100 Hz)
Sound Group: Voice
Load Type: Load on Demand
Enable Baked Analysis: Yes
```

**Asset Properties:**
```
Looping: Yes
Volume: 1.0
Pitch: 1.0
Attenuation Settings: ATT_Breathing
Sound Class: Voice/Player
Priority: 1.0 (highest)
```

**Advanced Settings:**
```
Enable Seek: No
Virtualization Mode: Play When Silent
Concurrency Settings: Max 1 instance
```

---

#### Low Oxygen Breathing (1 file)

**Files:**
- breathing_low_oxygen_loop.wav

**Import Dialog Settings:**
```
Compression Quality: 60
Sample Rate Setting: High (44100 Hz)
Sound Group: Voice
Load Type: Load on Demand
Enable Baked Analysis: Yes
```

**Asset Properties:**
```
Looping: Yes
Volume: 1.0
Pitch: 1.0
Attenuation Settings: ATT_Breathing
Sound Class: Voice/Player
Priority: 1.0 (highest)
```

**Advanced Settings:**
```
Enable Seek: No
Virtualization Mode: Play When Silent
Concurrency Settings: Max 1 instance
```

---

#### Hold Breath (1 file)

**Files:**
- breathing_hold_breath.wav

**Import Dialog Settings:**
```
Compression Quality: 60
Sample Rate Setting: High (44100 Hz)
Sound Group: Voice
Load Type: Load on Demand
Enable Baked Analysis: Yes
```

**Asset Properties:**
```
Looping: No
Volume: 1.0
Pitch: 1.0
Attenuation Settings: ATT_Breathing
Sound Class: Voice/Player
Priority: 0.9
```

**Advanced Settings:**
```
Enable Seek: No
Virtualization Mode: Play When Silent
Concurrency Settings: Max 1 instance
```

---

### 4. Underwater Audio Import Settings

#### Underwater Ambience (1 file)

**Files:**
- underwater_ambience_loop.wav

**Import Dialog Settings:**
```
Compression Quality: 40
Sample Rate Setting: High (44100 Hz)
Sound Group: Ambient
Load Type: Streaming
Enable Baked Analysis: Yes
```

**Asset Properties:**
```
Looping: Yes
Volume: 1.0
Pitch: 1.0
Attenuation Settings: ATT_Underwater
Sound Class: Ambience/Underwater
Priority: 0.7
```

**Advanced Settings:**
```
Enable Seek: Yes
Virtualization Mode: Restart
Concurrency Settings: Max 1 instance
Submix: Underwater_Submix
```

---

#### Underwater Bubbles (1 file)

**Files:**
- underwater_bubbles_01.wav

**Import Dialog Settings:**
```
Compression Quality: 40
Sample Rate Setting: High (44100 Hz)
Sound Group: SFX
Load Type: Load on Demand
Enable Baked Analysis: Yes
```

**Asset Properties:**
```
Looping: No
Volume: 1.0
Pitch: 1.0
Attenuation Settings: ATT_Underwater
Sound Class: SFX
Priority: 0.4
```

**Advanced Settings:**
```
Enable Seek: No
Virtualization Mode: Stop
Concurrency Settings: Max 8 instances
Concurrency Resolution Rule: Stop Oldest
```

---

#### Underwater Movement (1 file)

**Files:**
- underwater_movement_loop.wav

**Import Dialog Settings:**
```
Compression Quality: 40
Sample Rate Setting: High (44100 Hz)
Sound Group: SFX
Load Type: Load on Demand
Enable Baked Analysis: Yes
```

**Asset Properties:**
```
Looping: Yes
Volume: 1.0
Pitch: 1.0
Attenuation Settings: ATT_Underwater
Sound Class: SFX
Priority: 0.6
```

**Advanced Settings:**
```
Enable Seek: No
Virtualization Mode: Restart
Concurrency Settings: Max 1 instance
```

---

#### Underwater Enter/Exit (2 files)

**Files:**
- underwater_enter.wav
- underwater_exit.wav

**Import Dialog Settings:**
```
Compression Quality: 40
Sample Rate Setting: High (44100 Hz)
Sound Group: SFX
Load Type: Load on Demand
Enable Baked Analysis: Yes
```

**Asset Properties:**
```
Looping: No
Volume: 1.0
Pitch: 1.0
Attenuation Settings: None (2D sound)
Sound Class: SFX
Priority: 0.8
```

**Advanced Settings:**
```
Enable Seek: No
Virtualization Mode: Stop
Concurrency Settings: Max 1 instance
```

---

## Batch Import Workflow

### Method 1: Drag and Drop

1. Select all files of same category
2. Drag into appropriate Content Browser folder
3. In Import Dialog:
   - Set compression quality
   - Set sound group
   - Enable/disable auto-create cue
   - Click "Import All"
4. Adjust individual asset properties after import

### Method 2: Import Dialog

1. File → Import to /Game/Audio/...
2. Select files
3. Configure settings in dialog
4. Import
5. Adjust properties

### Method 3: Python Script (Batch)

```python
import unreal

def import_audio_batch(file_paths, destination_path, settings):
    task = unreal.AssetImportTask()
    task.destination_path = destination_path
    task.replace_existing = True
    task.automated = True
    task.save = True
    
    for file_path in file_paths:
        task.filename = file_path
        task.options = settings
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

# Example usage
ambient_settings = unreal.SoundFactory()
ambient_settings.compression_quality = 40
ambient_settings.sound_group = unreal.SoundGroup.SOUNDGROUP_AMBIENT

import_audio_batch(
    ['C:/Audio/ambient_station_base_01.wav', 'C:/Audio/ambient_station_base_02.wav'],
    '/Game/Audio/Ambient/Station',
    ambient_settings
)
```

---

## Post-Import Configuration

### Step 1: Verify Import

For each imported asset:
1. Double-click to open Sound Wave Editor
2. Verify waveform displays correctly
3. Play audio to check quality
4. Check file size (should be compressed)
5. Verify looping (if applicable)

### Step 2: Configure Properties

For each asset, set:
1. Looping (if loop file)
2. Sound Class
3. Attenuation Settings
4. Priority
5. Concurrency Settings

### Step 3: Enable Streaming

For files > 5 seconds:
1. Open Sound Wave asset
2. Enable "Streaming"
3. Set "Streaming Priority" (0-100)
4. Save

### Step 4: Configure Concurrency

1. Create Concurrency Settings asset
2. Set max instances
3. Set resolution rule (Stop Oldest, Stop Quietest, etc.)
4. Assign to Sound Wave or Sound Cue

---

## Compression Quality Guidelines

### Quality Levels

| Quality | Use Case | File Size | CPU Cost |
|---------|----------|-----------|----------|
| 1-20 | Very low quality, placeholder | Very small | Very low |
| 20-40 | Ambient, background | Small | Low |
| 40-60 | Standard SFX | Medium | Medium |
| 60-80 | Player audio, dialogue | Large | High |
| 80-100 | Music, critical audio | Very large | Very high |

### Recommended Settings

| Category | Quality | Rationale |
|----------|---------|-----------|
| Ambient | 40 | Background, can tolerate compression |
| Footsteps | 50 | Frequent, needs clarity |
| Breathing | 60 | Player-centric, high quality |
| Underwater | 40 | Muffled by design |
| Music | 70 | High quality for immersion |
| Dialogue | 70 | Clarity critical |
| UI | 50 | Short, frequent |

---

## Sample Rate Guidelines

### Sample Rates

| Rate | Use Case | Quality | File Size |
|------|----------|---------|-----------|
| 22050 Hz | Low quality, mobile | Low | Small |
| 44100 Hz | Standard, most audio | High | Medium |
| 48000 Hz | Professional, music | Very high | Large |

### Recommendation

Use 44100 Hz for all audio in HorrorProject:
- Industry standard
- Good quality/size balance
- Compatible with all platforms
- No resampling needed

---

## Memory Budget

### Target Memory Usage

| Category | Files | Avg Size | Total | Streaming |
|----------|-------|----------|-------|-----------|
| Ambient | 8 | 1.5 MB | 12 MB | Yes (8 MB) |
| Footsteps | 15 | 50 KB | 750 KB | No |
| Breathing | 5 | 200 KB | 1 MB | No |
| Underwater | 5 | 1 MB | 5 MB | Partial (3 MB) |
| **Total** | **33** | - | **18.75 MB** | **11 MB** |

**Loaded in Memory: ~7.75 MB**
**Streamed: ~11 MB**

### Optimization Tips

1. **Enable Streaming**: Files > 5 seconds
2. **Use Mono**: When stereo not needed
3. **Compress Aggressively**: Background sounds
4. **Limit Concurrency**: Prevent audio spam
5. **Unload Unused**: Use asset manager

---

## Platform-Specific Settings

### Windows (Development)

```
Compression Quality: As specified above
Sample Rate: 44100 Hz
Format: OGG Vorbis
Streaming: Enabled for long files
```

### Console (Future)

```
Compression Quality: -10% (more aggressive)
Sample Rate: 44100 Hz
Format: Platform-specific (ADPCM, etc.)
Streaming: Enabled for all ambient
```

### Mobile (Future)

```
Compression Quality: -20% (very aggressive)
Sample Rate: 22050 Hz (downsample)
Format: Platform-specific
Streaming: Enabled for all > 3 seconds
```

---

## Quality Assurance Checklist

After importing all audio:

### Import Verification
- [ ] All 33 files imported successfully
- [ ] No import errors or warnings
- [ ] All files play correctly in editor
- [ ] File sizes are reasonable (compressed)

### Property Verification
- [ ] Looping set correctly for all loop files
- [ ] Sound classes assigned
- [ ] Attenuation settings assigned
- [ ] Priorities set appropriately
- [ ] Concurrency settings configured

### Quality Verification
- [ ] No audio artifacts (clicks, pops)
- [ ] Loops are seamless
- [ ] Volume levels are consistent
- [ ] Compression quality is acceptable
- [ ] No distortion or clipping

### Performance Verification
- [ ] Streaming enabled for long files
- [ ] Memory usage within budget
- [ ] Concurrency limits prevent spam
- [ ] No performance issues in editor

---

## Troubleshooting

### Import Fails

**Problem:** File won't import
**Solutions:**
- Check file format (WAV, MP3, OGG supported)
- Verify file isn't corrupted
- Check file size (< 100 MB recommended)
- Try different import method
- Check file permissions

### Poor Quality

**Problem:** Audio sounds bad after import
**Solutions:**
- Increase compression quality
- Check source file quality
- Verify sample rate
- Disable aggressive compression
- Use higher bit depth source

### Large File Size

**Problem:** Imported files too large
**Solutions:**
- Reduce compression quality
- Enable streaming
- Convert to mono if possible
- Reduce sample rate (carefully)
- Trim silence from source

### Looping Issues

**Problem:** Loop has gap or click
**Solutions:**
- Fix source file (ensure seamless loop)
- Enable "Looping" in asset properties
- Check for silence at start/end
- Use loop points in source file
- Re-export source with proper loop

### Performance Issues

**Problem:** Audio causes lag or stuttering
**Solutions:**
- Enable streaming for long files
- Reduce concurrent sound count
- Lower compression quality (less CPU)
- Use audio culling
- Optimize concurrency settings

---

## Best Practices Summary

1. **Compression**: Use appropriate quality for each category
2. **Streaming**: Enable for files > 5 seconds
3. **Mono vs Stereo**: Use mono when possible
4. **Looping**: Verify seamless loops
5. **Concurrency**: Limit instances to prevent spam
6. **Priority**: Set appropriately for mixing
7. **Sound Classes**: Organize with hierarchy
8. **Attenuation**: Always use for 3D sounds
9. **Testing**: Test each file after import
10. **Documentation**: Track all settings

---

## Quick Reference Table

| File | Compression | Sample Rate | Looping | Streaming | Channels | Priority |
|------|-------------|-------------|---------|-----------|----------|----------|
| ambient_station_base_* | 40% | 44100 | Yes | Yes | Stereo | 0.5 |
| ambient_machinery_* | 40% | 44100 | Yes | Yes | Stereo | 0.5 |
| ambient_water_* | 40% | 44100 | Yes | Yes | Stereo | 0.5 |
| ambient_underwater_* | 40% | 44100 | Yes | Yes | Stereo | 0.7 |
| ambient_tension_* | 40% | 44100 | Yes | Yes | Stereo | 0.8 |
| footstep_* | 50% | 44100 | No | No | Mono | 0.6 |
| breathing_*_loop | 60% | 44100 | Yes | No | Mono | 0.9-1.0 |
| breathing_hold_breath | 60% | 44100 | No | No | Mono | 0.9 |
| underwater_ambience_* | 40% | 44100 | Yes | Yes | Stereo | 0.7 |
| underwater_bubbles_* | 40% | 44100 | No | No | Mono | 0.4 |
| underwater_movement_* | 40% | 44100 | Yes | No | Stereo | 0.6 |
| underwater_enter/exit | 40% | 44100 | No | No | Stereo | 0.8 |
