# Audio Design Guide

## Overview

Audio is critical for horror atmosphere, providing tension, navigation cues, and emotional impact. Strategic audio design enhances immersion and fear.

## Audio Philosophy

### Core Principles

1. **Silence is Powerful**: Absence of sound creates tension
2. **Layering Creates Depth**: Multiple audio layers build atmosphere
3. **Contrast Drives Impact**: Quiet to loud creates scares
4. **Spatial Audio is Essential**: 3D positioning enhances immersion
5. **Subtlety Over Bombast**: Suggestion is scarier than revelation

## Audio Categories

### 1. Ambient Audio (40%)
- Background atmosphere
- Room tone
- Environmental loops
- Constant presence

### 2. Music (20%)
- Emotional guidance
- Tension building
- Climax moments
- Sparse usage

### 3. Sound Effects (25%)
- Player actions
- Environmental interactions
- Object sounds
- Feedback sounds

### 4. Enemy Audio (10%)
- Creature sounds
- Footsteps
- Breathing
- Attack sounds

### 5. Narrative Audio (5%)
- Voice lines
- Audio logs
- Dialogue
- Story delivery

## Technical Foundation

### Audio System Budget (SM13)

#### Simultaneous Sounds
- **Total Active**: < 32 sounds
- **3D Positioned**: < 16 sounds
- **2D UI/Music**: < 8 sounds
- **High Priority**: < 4 sounds

#### Memory Budget
- **Total Audio**: < 200MB loaded
- **Streaming Pool**: 50MB
- **Ambient Loops**: 30MB
- **SFX Library**: 80MB
- **Music**: 40MB (streaming)

#### CPU Budget
- **Audio Thread**: < 2ms per frame
- **DSP Effects**: < 1ms per frame
- **Spatialization**: < 0.5ms per frame
- **Total**: < 3.5ms per frame

### Audio Quality Standards

#### Sample Rates
- **Music**: 44.1kHz, stereo
- **Ambient**: 44.1kHz, stereo
- **SFX**: 44.1kHz, mono/stereo
- **Voice**: 22.05kHz, mono
- **Footsteps**: 22.05kHz, mono

#### Compression
- **Music**: Vorbis, quality 0.4 (streaming)
- **Ambient**: Vorbis, quality 0.3
- **SFX**: ADPCM (short) / Vorbis (long)
- **Voice**: Vorbis, quality 0.5

#### File Sizes (Target)
- **Music Track**: 3-5MB per minute
- **Ambient Loop**: 500KB-2MB
- **SFX**: 10-100KB each
- **Voice Line**: 50-200KB per line

## Zone-Based Audio Design

### 1. Entry Hall - False Security

#### Ambient Layer
**Base Ambience**
- Sound: Fluorescent hum, air conditioning
- Volume: -20dB
- Loop: Seamless 60s
- Attenuation: 500 radius, 1000 falloff

**Detail Layer**
- Sound: Distant traffic, clock ticking
- Volume: -30dB
- Loop: 120s with variation
- Attenuation: 300 radius, 600 falloff

**Tension Layer** (subtle)
- Sound: Very faint whispers (barely audible)
- Volume: -40dB
- Trigger: Random (5-10 min intervals)
- Purpose: Unease

#### Music
- Track: None (silence builds tension)
- Trigger: Only on objective 1 completion
- Type: Subtle string drone
- Volume: -35dB

#### SFX
- Door open/close: -15dB
- Footsteps: Material-based, -20dB
- Paper rustle: -25dB
- Keycard pickup: -18dB

#### Reverb
- Type: Medium hall
- Decay: 1.5s
- Wet/Dry: 20/80
- Pre-delay: 20ms

### 2. Main Corridor - Growing Unease

#### Ambient Layer
**Base Ambience**
- Sound: Electrical hum, distant machinery
- Volume: -22dB
- Loop: 90s
- Attenuation: 800 radius, 1500 falloff

**Detail Layer**
- Sound: Dripping water, creaking
- Volume: -28dB
- Randomized: 3-5 variations
- Attenuation: 400 radius, 800 falloff

**Tension Layer**
- Sound: Metallic groans, distant footsteps
- Volume: -32dB
- Trigger: Random + proximity-based
- Purpose: Paranoia

**Hazard Layer**
- Sound: Sparking electricity
- Volume: -18dB
- Position: Specific hazard locations
- Attenuation: 300 radius, 600 falloff

#### Music
- Track: Low frequency drone
- Volume: -38dB
- Trigger: Entering from Entry Hall
- Fade: 10s crossfade

#### SFX
- Footsteps: Echo added, -18dB
- Door creak: -20dB
- Electrical spark: -15dB
- Breaker switch: -12dB

#### Reverb
- Type: Long corridor
- Decay: 2.5s
- Wet/Dry: 35/65
- Pre-delay: 30ms

### 3. West Wing - Exploration

#### Ambient Layer
**Base Ambience**
- Sound: Wind through windows, old building settling
- Volume: -24dB
- Loop: 120s with variation
- Attenuation: Per-room (300 radius, 600 falloff)

**Detail Layer**
- Sound: Paper rustling, clock ticking, computer hum
- Volume: -30dB
- Position: Specific objects
- Attenuation: 200 radius, 400 falloff

**Tension Layer**
- Sound: Scratching in walls, distant crying
- Volume: -35dB
- Trigger: Exploration-based (entering new rooms)
- Purpose: Reward/punish curiosity

**Window Layer**
- Sound: Wind, rain (weather-dependent)
- Volume: -26dB
- Position: Each window
- Attenuation: 400 radius, 800 falloff

#### Music
- Track: Melancholic piano (sparse notes)
- Volume: -40dB
- Trigger: Finding evidence
- Duration: 30-60s, then fade

#### SFX
- Drawer open: -22dB
- Document pickup: -20dB
- Footsteps: Carpet/wood variation, -22dB
- Safe code entry: -18dB

#### Reverb
- Type: Small room (per room)
- Decay: 0.8-1.2s (varies)
- Wet/Dry: 15/85
- Pre-delay: 10ms

### 4. East Wing - Clinical Horror

#### Ambient Layer
**Base Ambience**
- Sound: Medical equipment beeping, ventilation
- Volume: -20dB
- Loop: 60s
- Attenuation: 600 radius, 1200 falloff

**Detail Layer**
- Sound: Heart monitor, IV drip, fluorescent buzz
- Volume: -25dB
- Position: Specific equipment
- Attenuation: 250 radius, 500 falloff

**Tension Layer**
- Sound: Distant screaming, metal scraping
- Volume: -30dB
- Trigger: Proximity to morgue
- Purpose: Dread building

**Enemy Layer**
- Sound: Breathing, footsteps, growling
- Volume: -22dB (dynamic based on distance)
- Position: Enemy location (3D)
- Attenuation: 800 radius, 1500 falloff

#### Music
- Track: Dissonant strings, industrial noise
- Volume: -32dB
- Trigger: Enemy detection
- Dynamic: Intensity increases with threat

#### SFX
- Morgue drawer: -15dB (loud, intentional)
- Medical equipment: -20dB
- Footsteps: Tile echo, -18dB
- Specimen pickup: -16dB

#### Reverb
- Type: Tiled room
- Decay: 1.8s
- Wet/Dry: 30/70
- Pre-delay: 15ms

### 5. Basement - Oppressive Dread

#### Ambient Layer
**Base Ambience**
- Sound: Boiler rumbling, deep hum
- Volume: -18dB
- Loop: 180s
- Attenuation: 1000 radius, 2000 falloff
- Low-pass: 500Hz (muffled)

**Detail Layer**
- Sound: Water dripping, pipes groaning, steam hissing
- Volume: -24dB
- Position: Environmental sources
- Attenuation: 300 radius, 600 falloff

**Tension Layer**
- Sound: Chanting (very faint), whispers, breathing
- Volume: -36dB
- Trigger: Approaching ritual chamber
- Purpose: Supernatural dread

**Hazard Layer**
- Sound: Boiler stress, steam release
- Volume: -16dB
- Position: Boiler room
- Attenuation: 500 radius, 1000 falloff

**Water Layer**
- Sound: Splashing, flowing water
- Volume: -22dB
- Position: Flooded areas
- Attenuation: 400 radius, 800 falloff

#### Music
- Track: Deep bass drone, ritual percussion
- Volume: -28dB
- Trigger: Entering basement
- Crescendo: Builds to objective 6

#### SFX
- Footsteps: Water splashing, -20dB
- Boiler interaction: -14dB
- Ritual object: -12dB (dramatic)
- Enemy sounds: Multiple, -20dB

#### Reverb
- Type: Large basement
- Decay: 3.0s
- Wet/Dry: 40/60
- Pre-delay: 40ms
- Low-pass: 800Hz (damp)

### 6. Upper Floor - Melancholic Beauty

#### Ambient Layer
**Base Ambience**
- Sound: Wind howling, building creaking
- Volume: -22dB
- Loop: 150s
- Attenuation: 700 radius, 1400 falloff

**Detail Layer**
- Sound: Floorboards creaking, window rattling, birds (distant)
- Volume: -28dB
- Position: Environmental triggers
- Attenuation: 300 radius, 600 falloff

**Tension Layer**
- Sound: Whispers, music box (faint), child's laughter
- Volume: -38dB
- Trigger: Exploration-based
- Purpose: Unsettling beauty

**Collapse Layer**
- Sound: Wood stress, debris falling
- Volume: -20dB
- Position: Collapsed section
- Attenuation: 400 radius, 800 falloff

#### Music
- Track: Sad piano melody
- Volume: -36dB
- Trigger: Entering archives
- Emotion: Bittersweet

#### SFX
- Footsteps: Old wood creak, -20dB
- Document pickup: -22dB
- Key pickup: -18dB
- Collapse warning: -10dB (loud)

#### Reverb
- Type: Large open space
- Decay: 2.2s
- Wet/Dry: 25/75
- Pre-delay: 25ms

### 7. Exit Sequence - Chaos

#### Ambient Layer
**Base Ambience**
- Sound: Alarm blaring, fire crackling
- Volume: -12dB (loud)
- Loop: 30s
- Attenuation: 1000 radius, 2000 falloff

**Detail Layer**
- Sound: Debris falling, structure collapsing, sparks
- Volume: -18dB
- Position: Dynamic (event-based)
- Attenuation: 500 radius, 1000 falloff

**Tension Layer**
- Sound: Enemy pursuit (if applicable)
- Volume: -20dB
- Position: Behind player
- Purpose: Urgency

**Fire Layer**
- Sound: Fire roar, smoke
- Volume: -16dB
- Position: Fire sources
- Attenuation: 400 radius, 800 falloff

#### Music
- Track: Intense orchestral crescendo
- Volume: -25dB
- Trigger: Exit sequence start
- Dynamic: Builds to escape

#### SFX
- Footsteps: Running, -18dB
- Door interaction: -10dB
- Collapse: -8dB (very loud)
- Escape success: -15dB (victory)

#### Reverb
- Type: Chaotic (dynamic)
- Decay: 1.5s
- Wet/Dry: 20/80
- Pre-delay: 10ms

## Special Audio Systems

### 1. Player Heartbeat System

#### Purpose
- Indicate stress/danger
- Feedback for stealth
- Enhance immersion

#### Implementation
- **Trigger**: Enemy proximity, low health, high tension areas
- **Volume**: -30dB to -20dB (scales with stress)
- **Rate**: 60 BPM (calm) to 140 BPM (panic)
- **Position**: 2D (non-spatial)
- **Priority**: High

#### States
- **Calm**: No heartbeat
- **Alert**: Slow heartbeat (80 BPM)
- **Danger**: Fast heartbeat (120 BPM)
- **Panic**: Very fast (140 BPM) + breathing

### 2. Flashlight Audio

#### Sounds
- **On**: Click, -20dB
- **Off**: Click, -20dB
- **Flicker** (low battery): Buzz, -25dB
- **Battery warning**: Beep, -22dB

#### Integration
- Tied to flashlight state
- Warning at 20% battery
- Flicker at 10% battery
- Audio cue for recharge

### 3. Footstep System

#### Material Types (8 total)
1. **Concrete**: Hard, echoing
2. **Wood**: Creaky, varied
3. **Carpet**: Soft, muffled
4. **Tile**: Sharp, clinical
5. **Metal**: Loud, resonant
6. **Water**: Splashing, wet
7. **Debris**: Crunchy, varied
8. **Grass**: Soft, rustling

#### Implementation
- Physical material detection
- Speed-based variation (walk/run/crouch)
- Random variation (3-5 samples per material)
- Volume: -20dB to -18dB (based on speed)
- Attenuation: 600 radius, 1200 falloff

#### Enemy Detection
- Louder footsteps alert enemies
- Crouch reduces volume (-10dB)
- Running increases volume (+5dB)
- Material affects detection range

### 4. Environmental Triggers

#### Proximity Triggers
- Activate sounds when player approaches
- Fade in/out based on distance
- Used for localized ambience
- Example: Computer hum near desks

#### Event Triggers
- One-shot sounds for events
- Example: Door slam, glass break
- Priority: High
- Volume: Varies by event

#### Random Triggers
- Periodic random sounds
- Create unpredictability
- Example: Distant noises
- Interval: 30s-5min

### 5. Music System

#### Layers (Adaptive)
- **Layer 1**: Ambient drone (always)
- **Layer 2**: Melody (exploration)
- **Layer 3**: Percussion (tension)
- **Layer 4**: Strings (danger)

#### States
- **Exploration**: Layers 1-2
- **Tension**: Layers 1-3
- **Combat/Chase**: All layers
- **Safe**: Layer 1 only

#### Transitions
- Crossfade: 5-10s
- Beat-matched when possible
- Dynamic intensity scaling
- Silence for maximum tension

### 6. Reverb Zones

#### Zone Types
- **Small Room**: 0.8-1.2s decay
- **Medium Room**: 1.5-2.0s decay
- **Large Room**: 2.5-3.5s decay
- **Corridor**: 2.0-3.0s decay (long)
- **Outdoor**: 0.5s decay (minimal)

#### Implementation
- Audio volumes per zone
- Smooth transitions (2s blend)
- Per-zone settings
- Performance: Minimal cost

## Audio for Gameplay

### Navigation Audio Cues

#### Objective Hints
- Subtle audio cue near objectives
- Example: Humming, glowing sound
- Volume: -32dB (very subtle)
- Attenuation: 400 radius, 800 falloff

#### Directional Cues
- Sounds guide player to goals
- Example: Alarm from exit
- Volume: Varies
- Purpose: Prevent getting lost

#### Danger Warnings
- Audio indicates hazards
- Example: Sparking electricity, enemy breathing
- Volume: -18dB to -15dB
- Purpose: Fair gameplay

### Enemy Audio Design

#### Detection States
- **Idle**: Ambient breathing, -28dB
- **Alert**: Investigating sounds, -24dB
- **Chase**: Aggressive sounds, -18dB
- **Attack**: Loud attack sounds, -12dB

#### Spatial Audio
- Full 3D positioning
- Occlusion (walls muffle sound)
- Distance attenuation
- Doppler effect (if moving fast)

#### Footsteps
- Material-based (same as player)
- Louder than player (+5dB)
- Distinct pattern (slower/heavier)
- Purpose: Player can track enemy

### Evidence Audio

#### Discovery Sound
- Pickup: -20dB
- Inspection: Page turn, -25dB
- Audio log: Voice, -22dB
- Purpose: Feedback

#### Audio Logs
- Voice acting quality: High
- Background ambience: Minimal
- Subtitles: Always available
- Playback: Can continue during gameplay

## Audio Mixing

### Priority System

#### Priority Levels (0-255)
1. **Critical** (200-255): Player death, major events
2. **High** (150-199): Enemy sounds, important SFX
3. **Medium** (100-149): Player actions, environment
4. **Low** (50-99): Ambient details, distant sounds
5. **Very Low** (0-49): Background ambience

#### Voice Ducking
- Music ducks -10dB during dialogue
- Ambience ducks -6dB during dialogue
- SFX unaffected
- Smooth transitions (1s)

### Frequency Management

#### Frequency Ranges
- **Sub Bass** (20-60Hz): Drones, rumbles
- **Bass** (60-250Hz): Boiler, deep sounds
- **Low Mid** (250-500Hz): Body of sounds
- **Mid** (500-2kHz): Clarity, presence
- **High Mid** (2-4kHz): Detail, intelligibility
- **High** (4-20kHz): Air, sparkle

#### EQ Strategy
- Avoid frequency masking
- Cut unnecessary lows (< 80Hz) from most sounds
- Boost mids for clarity
- High-pass filter on ambience
- Low-pass filter on distant sounds

### Dynamic Range

#### Loudness Targets
- **Peak**: -6dB
- **Average**: -20dB
- **Quiet moments**: -40dB
- **Dynamic range**: 34dB

#### Compression
- Light compression on master (2:1 ratio)
- Heavy compression on dialogue (4:1 ratio)
- No compression on music (preserve dynamics)
- Limiter on master (-0.3dB ceiling)

## Performance Optimization

### Optimization Checklist
- [ ] Sounds use appropriate compression
- [ ] Long sounds stream from disk
- [ ] Short sounds loaded in memory
- [ ] Attenuation radii optimized
- [ ] Priority system implemented
- [ ] Occlusion enabled
- [ ] Reverb zones efficient
- [ ] No unnecessary 3D sounds
- [ ] Sound cue variations randomized
- [ ] Memory budget maintained

### Common Issues

#### Too Many Sounds
- **Problem**: Audio cuts out
- **Fix**: Reduce simultaneous sounds, increase priority of important sounds

#### Performance Spikes
- **Problem**: Audio thread overload
- **Fix**: Optimize DSP, reduce reverb quality, stream more sounds

#### Pops/Clicks
- **Problem**: Audio artifacts
- **Fix**: Proper crossfades, fix sample rates, check compression

#### Inaudible Sounds
- **Problem**: Sounds playing but not heard
- **Fix**: Adjust attenuation, increase volume, check occlusion

## Testing Checklist

### Technical Tests
- [ ] All sounds play correctly
- [ ] No audio dropouts
- [ ] Performance within budget
- [ ] Memory usage acceptable
- [ ] Streaming works properly
- [ ] No pops or clicks

### Gameplay Tests
- [ ] Navigation cues clear
- [ ] Enemy audio provides fair warning
- [ ] Footsteps audible and distinct
- [ ] Objectives have audio hints
- [ ] Flashlight audio responsive
- [ ] Heartbeat system works

### Immersion Tests
- [ ] Atmosphere appropriate per zone
- [ ] Tension builds correctly
- [ ] Silence used effectively
- [ ] Music enhances emotion
- [ ] Reverb feels natural
- [ ] Overall cohesive soundscape

## Tools Integration

### Optimization
```powershell
.\OptimizeLevelAudio.ps1 -LevelName "SM13" -CompressAll -OptimizeAttenuation
```

### Performance Analysis
```powershell
.\AnalyzeLevelPerformance.ps1 -Focus "Audio" -GenerateReport
```

### Validation
```powershell
.\ValidateLevelLayout.ps1 -CheckAudio -CheckPerformance
```

## Documentation Template

### Per-Zone Audio Documentation
```markdown
## Zone: [Name]

**Mood**: [Description]
**Sound Count**: [Active sounds]
**Memory**: [MB]

### Ambient Layers
1. [Name]: [Description], [Volume], [Attenuation]
2. ...

### Music
- Track: [Name]
- Trigger: [Condition]
- Volume: [dB]

### SFX
- [Action]: [Sound], [Volume]

### Reverb
- Type: [Preset]
- Decay: [Seconds]
- Wet/Dry: [Ratio]

### Performance
- CPU: [ms]
- Memory: [MB]
- Simultaneous: [Count]

### Notes
- [Design notes]
- [Known issues]
```

## Audio Asset Naming Convention

### Format
```
[Category]_[Zone]_[Type]_[Variation]
```

### Examples
- `AMB_EntryHall_Fluorescent_01`
- `MUS_Basement_Tension_Loop`
- `SFX_Door_Open_Wood_03`
- `VO_AudioLog_Doctor_01`
- `ENM_Creature_Growl_02`

## Resources

- Level Design Principles: `LevelDesignPrinciples.md`
- SM13 Route Design: `SM13_RouteDesign.md`
- Lighting Design: `LightingDesign.md`
- Performance Budget: `LevelDesignPrinciples.md#performance-budget`
