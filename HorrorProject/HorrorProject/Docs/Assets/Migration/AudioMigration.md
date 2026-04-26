# Audio Asset Migration Plan - HorrorProject

**Generated:** 2026-04-26  
**Total Files:** 354 audio files  
**Estimated Time:** 4-5 hours  
**Priority:** HIGH - Week 2

---

## Migration Overview

### Source Packages

1. **Grimytheus_Vol_2** (150 files, 585 MB)
   - Location: `Content/Grimytheus_Vol_2/WAVs/`
   - Status: Not yet imported to project structure
   
2. **SoundsOfHorror** (204 files, 279 MB)
   - Location: `Content/SoundsOfHorror/`
   - Status: Not yet imported to project structure

### Target Structure

```
Content/Audio/
├── Ambient/
│   ├── Loops/
│   ├── OneShots/
│   └── Attenuation/
├── Horror/
│   ├── Creatures/
│   ├── Environmental/
│   ├── Stingers/
│   └── Voices/
├── Music/
│   ├── Tension/
│   ├── Chase/
│   └── Ambient/
├── SFX/
│   ├── Footsteps/
│   ├── Doors/
│   ├── Mechanical/
│   └── Misc/
└── Master/
    ├── Attenuations/
    ├── Classes/
    └── Mixes/
```

---

## Phase 1: Grimytheus_Vol_2 Migration

### File Analysis

**Expected Categories:**
- Ambient loops (40 files)
- Creature sounds (30 files)
- Environmental effects (40 files)
- Music stingers (20 files)
- Horror effects (20 files)

### Migration Steps

#### Step 1: Organize Source Files (30 min)

```powershell
# Scan and categorize files
.\Scripts\Assets\Migration\ScanAudioFiles.ps1 `
  -SourcePath "D:\gptzuo\HorrorProject\HorrorProject\Content\Grimytheus_Vol_2\WAVs" `
  -OutputReport "D:\gptzuo\HorrorProject\HorrorProject\Docs\Assets\Migration\Grimytheus_Inventory.csv"
```

**Manual Review:**
- Listen to samples from each category
- Identify file naming patterns
- Note any special requirements

#### Step 2: Create Target Folders (5 min)

```powershell
# Create folder structure
New-Item -ItemType Directory -Force -Path "D:\gptzuo\HorrorProject\HorrorProject\Content\Audio\Horror\Creatures"
New-Item -ItemType Directory -Force -Path "D:\gptzuo\HorrorProject\HorrorProject\Content\Audio\Horror\Environmental"
New-Item -ItemType Directory -Force -Path "D:\gptzuo\HorrorProject\HorrorProject\Content\Audio\Horror\Stingers"
New-Item -ItemType Directory -Force -Path "D:\gptzuo\HorrorProject\HorrorProject\Content\Audio\Horror\Voices"
New-Item -ItemType Directory -Force -Path "D:\gptzuo\HorrorProject\HorrorProject\Content\Audio\Music\Tension"
```

#### Step 3: Import and Compress (2 hours)

**Import Settings:**
```
Format: OGG Vorbis
Quality: 0.7 (70%)
Sample Rate: 44.1 kHz
Channels: Preserve original (mono/stereo)
```

**Batch Import Process:**
1. Open UE5 Editor
2. Content Browser → Import
3. Select all WAV files from category
4. Set import options:
   - Compression Quality: 0.7
   - Enable Streaming: True (for files >5s)
   - Auto Create Cue: False (manual later)
5. Import to target folder
6. Verify import success

**Expected Results:**
- File size reduction: 585 MB → ~180 MB
- Quality: Maintained (70% OGG is transparent)
- Format: .uasset (UE5 Sound Wave)

#### Step 4: Rename Assets (30 min)

**Naming Convention:**
```
SW_[Category]_[Description]_[Variation]
```

**Examples:**
```
Original: creature_growl_01.wav
Renamed:  SW_Creature_Growl_01

Original: ambient_wind_loop.wav
Renamed:  SW_Ambient_Wind_Loop

Original: stinger_jumpscare.wav
Renamed:  SW_Stinger_Jumpscare_01
```

**Batch Rename Script:**
```powershell
.\Scripts\Assets\Migration\RenameAudioAssets.ps1 `
  -SourceFolder "/Game/Audio/Horror" `
  -Prefix "SW_" `
  -DryRun $false
```

#### Step 5: Create Sound Cues (1 hour)

**For Each Category:**

**Ambient Loops:**
```
Sound Cue: SC_Ambient_[Name]_Loop
├── SW_Ambient_[Name]
├── Looping: True
├── Volume Modulator: 0.8-1.0
└── Attenuation: SA_Ambient_Large
```

**Creature Sounds:**
```
Sound Cue: SC_Creature_[Name]
├── Random Node
│   ├── SW_Creature_[Name]_01
│   ├── SW_Creature_[Name]_02
│   └── SW_Creature_[Name]_03
├── Volume Modulator: 0.9-1.1
├── Pitch Modulator: 0.95-1.05
└── Attenuation: SA_Creature_Medium
```

**Stingers:**
```
Sound Cue: SC_Stinger_[Name]
├── SW_Stinger_[Name]
├── Volume: 1.0
└── No Attenuation (2D sound)
```

#### Step 6: Setup Attenuation (30 min)

**Create Attenuation Assets:**

**SA_Ambient_Large:**
```
Falloff Distance: 50m
Attenuation Shape: Sphere
Volume Attenuation: Natural Sound
Enable Occlusion: True
Occlusion Trace Channel: Visibility
```

**SA_Creature_Medium:**
```
Falloff Distance: 25m
Attenuation Shape: Sphere
Volume Attenuation: Natural Sound
Enable Occlusion: True
Occlusion High Frequency: 0.8
```

**SA_Horror_Close:**
```
Falloff Distance: 10m
Attenuation Shape: Sphere
Volume Attenuation: Linear
Enable Occlusion: True
```

#### Step 7: Test & Validate (30 min)

**Test Checklist:**
- [ ] All files imported successfully
- [ ] No compression artifacts
- [ ] Sound cues play correctly
- [ ] Attenuation works as expected
- [ ] Looping sounds loop seamlessly
- [ ] Random variations work
- [ ] Occlusion functions properly

**Test Level:**
Create `L_AudioTest` with:
- Audio sources at various distances
- Occlusion geometry
- Player character for testing

---

## Phase 2: SoundsOfHorror Migration

### File Analysis

**Expected Categories:**
- Atmospheric ambience (80 files)
- One-shot effects (60 files)
- Voice/whispers (30 files)
- Mechanical sounds (34 files)

### Migration Steps

#### Step 1: Organize Source Files (30 min)

```powershell
.\Scripts\Assets\Migration\ScanAudioFiles.ps1 `
  -SourcePath "D:\gptzuo\HorrorProject\HorrorProject\Content\SoundsOfHorror" `
  -OutputReport "D:\gptzuo\HorrorProject\HorrorProject\Docs\Assets\Migration\SoundsOfHorror_Inventory.csv"
```

#### Step 2: Create Target Folders (5 min)

```powershell
New-Item -ItemType Directory -Force -Path "D:\gptzuo\HorrorProject\HorrorProject\Content\Audio\Ambient\Loops"
New-Item -ItemType Directory -Force -Path "D:\gptzuo\HorrorProject\HorrorProject\Content\Audio\Ambient\OneShots"
New-Item -ItemType Directory -Force -Path "D:\gptzuo\HorrorProject\HorrorProject\Content\Audio\Horror\Voices"
New-Item -ItemType Directory -Force -Path "D:\gptzuo\HorrorProject\HorrorProject\Content\Audio\SFX\Mechanical"
```

#### Step 3: Import and Compress (1.5 hours)

**Import Settings:**
```
Ambient Loops:
  - Quality: 0.6 (60% - lower for background)
  - Streaming: Enabled
  - Sample Rate: 44.1 kHz

Effects:
  - Quality: 0.7 (70%)
  - Streaming: Disabled (short files)
  - Sample Rate: 44.1 kHz

Voices:
  - Quality: 0.75 (75% - higher for clarity)
  - Streaming: Enabled (if >3s)
  - Sample Rate: 22.05 kHz (mono voices)
```

**Expected Results:**
- File size reduction: 279 MB → ~90 MB
- Total reduction: 864 MB → ~270 MB (69% savings)

#### Step 4: Rename Assets (30 min)

**Naming Patterns:**
```
Ambient: SW_Ambient_[Description]_Loop
Effects:  SW_SFX_[Category]_[Description]_[Var]
Voices:   SW_Voice_[Type]_[Var]
Mechanical: SW_Mechanical_[Description]_[Var]
```

#### Step 5: Create Sound Cues (1 hour)

**Ambient Sound Actors:**
```
BP_AmbientSound_[Location]
├── Audio Component
│   └── Sound: SC_Ambient_[Name]_Loop
├── Attenuation: SA_Ambient_Large
└── Auto Activate: True
```

**Effect Sound Cues:**
```
SC_SFX_[Name]
├── Random Node (if multiple variations)
├── Volume Modulator: 0.9-1.1
├── Pitch Modulator: 0.95-1.05
└── Attenuation: SA_SFX_Medium
```

#### Step 6: Setup Sound Classes (30 min)

**Create Sound Class Hierarchy:**
```
SCl_Master
├── SCl_Music
│   ├── SCl_Music_Tension
│   └── SCl_Music_Ambient
├── SCl_SFX
│   ├── SCl_SFX_Horror
│   ├── SCl_SFX_Mechanical
│   └── SCl_SFX_Environmental
├── SCl_Ambient
│   ├── SCl_Ambient_Loops
│   └── SCl_Ambient_OneShots
└── SCl_Voice
    ├── SCl_Voice_Whispers
    └── SCl_Voice_Screams
```

**Assign to Sound Cues:**
- All ambient → SCl_Ambient
- All effects → SCl_SFX
- All voices → SCl_Voice
- All music → SCl_Music

#### Step 7: Setup Sound Mix (30 min)

**Create Sound Mixes:**

**SM_Gameplay (Default):**
```
SCl_Music: 0.6
SCl_SFX: 0.8
SCl_Ambient: 0.5
SCl_Voice: 1.0
```

**SM_Tension (High Tension):**
```
SCl_Music: 0.8
SCl_SFX: 0.9
SCl_Ambient: 0.3
SCl_Voice: 1.0
```

**SM_Quiet (Stealth):**
```
SCl_Music: 0.3
SCl_SFX: 0.6
SCl_Ambient: 0.7
SCl_Voice: 1.0
```

#### Step 8: Test & Validate (30 min)

**Test Scenarios:**
1. Multiple ambient sounds playing
2. Rapid-fire sound effects
3. Voice clarity at distance
4. Sound mix transitions
5. Occlusion through walls
6. Memory usage monitoring

---

## Phase 3: Integration & Optimization

### Step 1: Setup Sound Concurrency (30 min)

**Create Concurrency Settings:**

**SCon_Ambient:**
```
Max Count: 8
Resolution Rule: Stop Oldest
Volume Scale: 1.0
```

**SCon_SFX:**
```
Max Count: 32
Resolution Rule: Stop Quietest
Volume Scale: 0.9
```

**SCon_Voice:**
```
Max Count: 4
Resolution Rule: Stop Lowest Priority
Volume Scale: 1.0
```

**SCon_Music:**
```
Max Count: 2
Resolution Rule: Stop Oldest
Volume Scale: 1.0
```

### Step 2: Setup Reverb (30 min)

**Create Reverb Effects:**

**RE_SmallRoom:**
```
Density: 0.8
Diffusion: 0.7
Gain: 0.3
Decay Time: 0.8s
```

**RE_LargeRoom:**
```
Density: 0.9
Diffusion: 0.8
Gain: 0.4
Decay Time: 2.0s
```

**RE_Corridor:**
```
Density: 0.7
Diffusion: 0.9
Gain: 0.5
Decay Time: 1.5s
```

**Place Reverb Volumes:**
- Small rooms: RE_SmallRoom
- Large halls: RE_LargeRoom
- Corridors: RE_Corridor

### Step 3: Performance Optimization (30 min)

**Audio Settings:**
```ini
[/Script/Engine.AudioSettings]
MaxChannels=32
NumStoppedSoundsToRetain=8
QualityLevels=3
```

**Quality Level Settings:**
```
Low:
  - Max Channels: 24
  - Sample Rate: 22050
  - Compression Quality: 0.5

Medium:
  - Max Channels: 32
  - Sample Rate: 44100
  - Compression Quality: 0.6

High:
  - Max Channels: 48
  - Sample Rate: 44100
  - Compression Quality: 0.7
```

### Step 4: Memory Budget Validation (15 min)

**Target Budget:**
- Loaded Audio: <50 MB
- Streaming Audio: <256 MB total
- Concurrent Sounds: <32 active

**Validation:**
```
Console Commands:
  stat SoundWaves
  stat SoundMixes
  au.Debug.Sounds 1
```

**Expected Results:**
- Memory usage within budget
- No audio dropouts
- Smooth streaming
- No pops or clicks

---

## Migration Checklist

### Pre-Migration
- [ ] Backup project
- [ ] Create target folder structure
- [ ] Install audio editing tools (if needed)
- [ ] Review source files

### Grimytheus_Vol_2
- [ ] Scan and categorize files
- [ ] Import with compression
- [ ] Rename to convention
- [ ] Create sound cues
- [ ] Setup attenuation
- [ ] Test all sounds

### SoundsOfHorror
- [ ] Scan and categorize files
- [ ] Import with compression
- [ ] Rename to convention
- [ ] Create sound cues
- [ ] Setup sound classes
- [ ] Setup sound mixes
- [ ] Test all sounds

### Integration
- [ ] Setup concurrency
- [ ] Setup reverb volumes
- [ ] Optimize settings
- [ ] Validate memory budget
- [ ] Performance test
- [ ] Update documentation

### Post-Migration
- [ ] Delete original packages (after validation)
- [ ] Update asset inventory
- [ ] Commit changes
- [ ] Create backup

---

## Validation Criteria

### Quality
- [ ] No audible compression artifacts
- [ ] Proper volume levels
- [ ] Clean loop points (for loops)
- [ ] No clipping or distortion

### Performance
- [ ] Memory usage <50 MB loaded
- [ ] Streaming working correctly
- [ ] No audio stuttering
- [ ] Concurrency limits working

### Integration
- [ ] All sounds accessible in editor
- [ ] Sound cues configured correctly
- [ ] Attenuation working
- [ ] Reverb zones active
- [ ] Sound mixes functional

### Documentation
- [ ] Asset inventory updated
- [ ] Usage guide created
- [ ] Sound list documented
- [ ] Integration notes written

---

## Rollback Plan

If migration fails:

1. **Restore Backup:**
   ```powershell
   Copy-Item "D:\Backups\HorrorProject_PreAudioMigration" `
     -Destination "D:\gptzuo\HorrorProject\HorrorProject" `
     -Recurse -Force
   ```

2. **Identify Issues:**
   - Check error logs
   - Review failed imports
   - Note problematic files

3. **Fix and Retry:**
   - Address specific issues
   - Re-import failed files
   - Test incrementally

---

## Timeline

### Day 1 (4 hours)
- Morning: Grimytheus_Vol_2 migration (2.5 hours)
- Afternoon: SoundsOfHorror migration (1.5 hours)

### Day 2 (2 hours)
- Morning: Integration & optimization (1 hour)
- Afternoon: Testing & validation (1 hour)

**Total Time:** 6 hours (estimated 4-5 hours with experience)

---

## Success Metrics

- ✅ All 354 files migrated successfully
- ✅ 69% disk space reduction (864 MB → 270 MB)
- ✅ No quality degradation
- ✅ Memory budget maintained
- ✅ Performance targets met
- ✅ Documentation complete

---

**Document Owner:** Asset Manager Agent  
**Last Updated:** 2026-04-26  
**Status:** Ready for execution
