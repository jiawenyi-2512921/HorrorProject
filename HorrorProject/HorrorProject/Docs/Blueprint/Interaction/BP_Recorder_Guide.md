# Blueprint Recorder Implementation Guide

## Overview
Complete guide for creating recorder interactables using Blueprint, inheriting from `ARecorderInteractable` C++ class.

## 1. Blueprint Creation Steps

### 1.1 Create Blueprint Class
1. Content Browser → Right-click → Blueprint Class
2. Search for `RecorderInteractable` parent class
3. Name: `BP_Recorder_[Type]` (e.g., `BP_Recorder_Tape`, `BP_Recorder_Digital`)
4. Location: `Content/Blueprints/Interaction/Recorders/`

### 1.2 Component Structure
Default components from C++ class:
- **RecorderMesh** (StaticMeshComponent) - Recorder device visual
- **InteractionVolume** (BoxComponent) - Interaction trigger
- **AudioComponent** (AudioComponent) - Audio playback component

### 1.3 Collision Configuration
**InteractionVolume Settings:**
- Collision Preset: `OverlapAllDynamic`
- Generate Overlap Events: `true`
- Box Extent: `X=80, Y=80, Z=60` (adjust per recorder size)

**RecorderMesh Collision:**
- Collision Preset: `BlockAll`
- Simulate Physics: `false`

## 2. Mesh and Material Configuration

### 2.1 Recorder Mesh Setup
1. Select `RecorderMesh` component
2. Static Mesh: Choose recorder type
3. Materials: Apply device materials
4. Transform:
   - Location: `(0,0,0)`
   - Rotation: Facing player interaction direction
   - Scale: Appropriate device size

### 2.2 Recommended Assets by Type

**Tape Recorder:**
- Mesh: `SM_Recorder_Cassette`, `SM_Recorder_Reel`
- Material: `M_Plastic_Recorder`, `M_Metal_Vintage`
- Style: Retro, analog aesthetic

**Digital Recorder:**
- Mesh: `SM_Recorder_Digital`, `SM_Recorder_Modern`
- Material: `M_Plastic_Modern`, `M_Metal_Brushed`
- Style: Clean, modern design

**Answering Machine:**
- Mesh: `SM_AnsweringMachine`, `SM_Phone_Old`
- Material: `M_Plastic_Beige`, `M_Phone_Vintage`
- Style: 80s/90s office equipment

**Dictaphone:**
- Mesh: `SM_Dictaphone`, `SM_Recorder_Handheld`
- Material: `M_Plastic_Black`, `M_Metal_Chrome`
- Style: Professional, compact

### 2.3 Visual Feedback
**LED Indicator Setup:**
- Add material parameter for LED color
- Idle: Off or dim red
- Playing: Bright green or blue
- Paused: Blinking yellow
- Stopped: Dim red

**Animated Components:**
- Tape reels rotating during playback
- LED blinking patterns
- Button press animations

## 3. Interaction Parameters

### 3.1 Base Interaction Settings
**Category: Interaction**
- `InteractableId`: Unique name (e.g., `Recorder_Victim_Message`)
- `InteractionPrompt`: "Play Audio Log" / "Stop Audio"
- `bCanInteractMultipleTimes`: `true`
- `bRequireLineOfSight`: `true`

### 3.2 Audio Settings
**Category: Recorder | Audio**
- `AudioLog`: Sound wave asset (e.g., `SW_AudioLog_Victim_01`)
  - Format: WAV or OGG
  - Quality: 44.1kHz, 16-bit minimum
  - Length: 10 seconds to 3 minutes recommended
- `AudioLogTitle`: Display name (e.g., "Victim's Last Message")
- `AudioLogTranscript`: Full text transcript (multiline)
  - Used for subtitles
  - Accessibility feature
  - Evidence reference
- `bUse3DAudio`: `true` (spatial audio)
  - `true`: Audio attenuates with distance
  - `false`: Always full volume (2D audio)
- `MaxAudibleDistance`: `1000.0` units
  - Close range: `500.0`
  - Normal: `1000.0`
  - Long range: `2000.0`

### 3.3 Evidence Settings
**Category: Recorder | Evidence**
- `bIsEvidence`: `true` if collectible evidence
- `EvidenceMetadata`: (if bIsEvidence = true)
  - `EvidenceId`: Unique evidence ID (e.g., `Evidence_AudioLog_01`)
  - `EvidenceName`: Display name
  - `Description`: Audio log description
  - `Category`: "Audio Evidence" or specific category
  - `bIsOptional`: Optional evidence flag

### 3.4 Behavior Settings
**Category: Recorder | Behavior**
- `bAutoPlayOnInteract`: `true` (play immediately)
  - `true`: Starts playing on first interaction
  - `false`: Requires explicit play command
- `bAutoCollectAsEvidence`: `true` (auto-add to evidence)
- `bStopOnReInteract`: `true` (toggle play/stop)
  - `true`: Second interaction stops playback
  - `false`: Interaction only starts playback
- `bLoopAudio`: `false` (repeat audio)
  - `true`: Audio loops continuously
  - `false`: Plays once and stops

## 4. Specific Functionality

### 4.1 Audio Playback Control
**C++ Functions Available:**
- `PlayAudioLog()` - Start playback
- `StopAudioLog()` - Stop playback
- `PauseAudioLog()` - Pause playback
- `IsPlaying()` - Check if playing
- `GetPlaybackProgress()` - Get progress (0.0-1.0)

**Blueprint Implementation:**
```
Override: OnInteract
├─ Branch (Is Playing?)
│  ├─ True:
│  │  ├─ Branch (Stop On ReInteract?)
│  │  │  ├─ True:
│  │  │     ├─ Call StopAudioLog()
│  │  │     ├─ Update Prompt ("Play Audio Log")
│  │  │     └─ Stop Visual Effects
│  │  │  └─ False → Ignore
│  └─ False:
│     ├─ Call PlayAudioLog()
│     ├─ Update Prompt ("Stop Audio")
│     ├─ Start Visual Effects
│     └─ Show Transcript UI (Optional)
```

### 4.2 3D Audio Configuration
**For bUse3DAudio = true:**
1. AudioComponent settings:
   - Attenuation: Custom attenuation settings
   - Falloff Distance: Match MaxAudibleDistance
   - Spatial Blend: 1.0 (fully 3D)
   - Enable Reverb: true
   - Enable Occlusion: true (optional)

**Blueprint Audio Setup:**
```
Event BeginPlay
├─ Get Audio Component
├─ Set Attenuation Settings
│  ├─ Falloff Distance: MaxAudibleDistance
│  ├─ Attenuation Shape: Sphere
│  └─ Falloff Model: Linear or Natural
├─ Set Spatialization
└─ Enable Reverb Send
```

### 4.3 Subtitle System
**Display transcript during playback:**
```
Variable: SubtitleLines (Array of Struct)
  - Timestamp (Float)
  - Text (String)
  - Speaker (String)

Event Tick (During Playback)
├─ Get Playback Progress
├─ Calculate Current Time
├─ Find Current Subtitle Line
├─ Branch (New Line?)
│  ├─ True:
│  │  ├─ Update Subtitle Widget
│  │  └─ Display Current Line
│  └─ False → Continue
```

### 4.4 Evidence Collection
**Automatic behavior when bIsEvidence = true:**
1. Player interacts with recorder
2. Audio starts playing
3. Evidence automatically collected
4. Transcript added to evidence log
5. EventBus event published

**Blueprint Extension:**
```
Event OnAudioLogStarted
├─ Branch (Is Evidence AND Auto Collect?)
│  ├─ True:
│  │  ├─ Add to Evidence System
│  │  ├─ Show Evidence Notification
│  │  ├─ Update Quest Objectives
│  │  └─ Publish Event "AudioLog.Collected"
│  └─ False → Continue
```

### 4.5 Playback Progress UI
**Visual progress indicator:**
```
Widget: RecorderProgressBar
├─ Progress Bar (0.0 to 1.0)
├─ Time Display (Current / Total)
├─ Waveform Visualization (Optional)
└─ Pause/Stop Buttons

Event Tick (During Playback)
├─ Get Playback Progress
├─ Update Progress Bar
├─ Update Time Display
└─ Update Waveform Position
```

### 4.6 Conditional Playback
**Require specific conditions:**
```
Override: CanInteract
├─ Parent CanInteract → Result
├─ Branch (Requires Power?)
│  ├─ True:
│  │  ├─ Check Power Source Active
│  │  ├─ Branch (Has Power?)
│  │  │  ├─ True → Return True
│  │  │  └─ False:
│  │  │     ├─ Show Message "No Power"
│  │  │     └─ Return False
│  └─ False → Return Result
```

## 5. Event Logic

### 5.1 OnInteract Event
**Default C++ Behavior:**
1. Check if can interact
2. Check current playback state
3. If not playing and auto-play enabled:
   - Start audio playback
   - Update recorder state
   - Try add to evidence
   - Play start sound
4. If playing and stop-on-reinteract enabled:
   - Stop audio playback
   - Update recorder state

**Blueprint Extension:**
```
Event OnInteract (Override)
├─ Call Parent OnInteract
├─ Branch (Playback Started?)
│  ├─ True:
│  │  ├─ Show Subtitle UI
│  │  ├─ Start LED Animation
│  │  ├─ Start Tape Reel Animation
│  │  └─ Publish Event "AudioLog.Started"
│  └─ False:
│     ├─ Hide Subtitle UI
│     ├─ Stop LED Animation
│     ├─ Stop Tape Reel Animation
│     └─ Publish Event "AudioLog.Stopped"
```

### 5.2 Playback State Events
**Blueprint Events:**
```
Event OnAudioLogStarted
├─ Set Recorder State (Playing)
├─ Update Interaction Prompt
├─ Show Playback UI
├─ Start Visual Effects
├─ Enable Subtitle System
└─ Start Progress Tracking

Event OnAudioLogStopped
├─ Set Recorder State (Stopped)
├─ Update Interaction Prompt
├─ Hide Playback UI
├─ Stop Visual Effects
├─ Disable Subtitle System
└─ Reset Progress

Event OnAudioLogPaused
├─ Set Recorder State (Paused)
├─ Pause Visual Effects
├─ Show Pause Indicator
└─ Maintain Progress

Event OnAudioLogFinished
├─ Set Recorder State (Idle)
├─ Mark as Listened
├─ Update Quest Objectives
├─ Branch (Loop Audio?)
│  ├─ True → Restart Playback
│  └─ False → Stop
└─ Publish Event "AudioLog.Completed"
```

### 5.3 EventBus Integration
**Automatic Events Published:**
- `AudioLog.Started` - When playback begins
- `AudioLog.Stopped` - When playback stops
- `AudioLog.Completed` - When audio finishes
- `AudioLog.Collected` - When evidence collected

**Custom Event Publishing:**
```
Event OnAudioLogCompleted
├─ Get EventBus Subsystem
├─ Create Event Data
│  ├─ Add "AudioLogId" → InteractableId
│  ├─ Add "AudioLogTitle" → AudioLogTitle
│  ├─ Add "Duration" → Audio Duration
│  ├─ Add "IsEvidence" → bIsEvidence
│  └─ Add "ListenCount" → Listen Count
└─ Publish Event ("AudioLog.FullyListened")
```

### 5.4 SaveGame Integration
**Automatic Save/Load:**
- Playback state (playing/stopped)
- Has been listened flag
- Listen count

**Blueprint Extension:**
```
Override: SaveState
├─ Call Parent SaveState
├─ Add State ("HasBeenListened", bHasBeenListened)
├─ Add State ("ListenCount", ListenCount)
├─ Add State ("WasCollected", bWasCollectedAsEvidence)
└─ Return State Map

Override: LoadState
├─ Call Parent LoadState
├─ Get State ("HasBeenListened")
├─ Get State ("ListenCount")
├─ Branch (Was Collected?)
│  ├─ True → Mark as Collected
│  └─ False → Continue
```

## 6. Testing Validation

### 6.1 Functional Tests
- [ ] Audio plays correctly on interaction
- [ ] 3D audio attenuates with distance
- [ ] Stop/pause controls work properly
- [ ] Playback progress accurate
- [ ] Subtitles sync with audio
- [ ] Evidence collection works
- [ ] Loop functionality works (if enabled)
- [ ] Audio finishes and stops correctly

### 6.2 Edge Cases
- [ ] Rapid play/stop spam
- [ ] Player leaves audio range during playback
- [ ] Multiple recorders playing simultaneously
- [ ] Playback during level transition
- [ ] Save/load during playback
- [ ] Audio file missing/corrupted
- [ ] Network replication (if multiplayer)
- [ ] Playback with no audio component

### 6.3 Performance Validation
- [ ] Audio streaming efficient
- [ ] No audio popping/crackling
- [ ] 3D audio calculations optimized
- [ ] Subtitle updates smooth
- [ ] Visual effects don't drop frames
- [ ] No memory leaks on stop

### 6.4 Audio Quality
- [ ] Audio volume appropriate
- [ ] No distortion or clipping
- [ ] 3D positioning accurate
- [ ] Attenuation feels natural
- [ ] Reverb/occlusion works correctly
- [ ] Subtitles match audio timing

## 7. Common Configurations

### 7.1 Basic Tape Recorder
```
AudioLog: SW_AudioLog_Victim_01
AudioLogTitle: "Victim's Message"
AudioLogTranscript: "Full transcript here..."
bUse3DAudio: true
MaxAudibleDistance: 1000.0
bIsEvidence: true
EvidenceMetadata:
  EvidenceId: "Evidence_AudioLog_Victim"
  EvidenceName: "Victim's Recording"
  Category: "Audio Evidence"
bAutoPlayOnInteract: true
bAutoCollectAsEvidence: true
bStopOnReInteract: true
bLoopAudio: false
```

### 7.2 Answering Machine (Looping)
```
AudioLog: SW_AudioLog_AnsweringMachine
AudioLogTitle: "Answering Machine Message"
AudioLogTranscript: "You've reached [number]..."
bUse3DAudio: true
MaxAudibleDistance: 800.0
bIsEvidence: false
bAutoPlayOnInteract: true
bAutoCollectAsEvidence: false
bStopOnReInteract: true
bLoopAudio: true
```

### 7.3 Digital Recorder (2D Audio)
```
AudioLog: SW_AudioLog_Interview
AudioLogTitle: "Police Interview Recording"
AudioLogTranscript: "Interview transcript..."
bUse3DAudio: false
MaxAudibleDistance: 0.0
bIsEvidence: true
EvidenceMetadata:
  EvidenceId: "Evidence_Interview_01"
  EvidenceName: "Police Interview"
  Category: "Official Records"
bAutoPlayOnInteract: true
bAutoCollectAsEvidence: true
bStopOnReInteract: true
bLoopAudio: false
```

### 7.4 Hidden Recorder (Auto-Play)
```
AudioLog: SW_AudioLog_Hidden
AudioLogTitle: "Hidden Recording"
AudioLogTranscript: "Secret message..."
bUse3DAudio: true
MaxAudibleDistance: 500.0
bIsEvidence: true
EvidenceMetadata:
  EvidenceId: "Evidence_Hidden_Recording"
  EvidenceName: "Hidden Message"
  Category: "Secret Evidence"
  bIsOptional: true
bAutoPlayOnInteract: true
bAutoCollectAsEvidence: true
bStopOnReInteract: false
bLoopAudio: false
```

### 7.5 Ambient Recorder (Background)
```
AudioLog: SW_AudioLog_Ambient
AudioLogTitle: "Background Audio"
AudioLogTranscript: ""
bUse3DAudio: true
MaxAudibleDistance: 1500.0
bIsEvidence: false
bAutoPlayOnInteract: false
bAutoCollectAsEvidence: false
bStopOnReInteract: true
bLoopAudio: true
```

## 8. Advanced Features

### 8.1 Multi-Track Recorder
**Multiple audio logs on one device:**
```
Variable: AudioLogTracks (Array of Sound Base)
Variable: CurrentTrackIndex (Integer) = 0

Custom Event: NextTrack
├─ Stop Current Audio
├─ Increment Track Index
├─ Branch (Index < Track Count?)
│  ├─ True:
│  │  ├─ Set AudioLog (AudioLogTracks[Index])
│  │  ├─ Play New Track
│  │  └─ Update UI
│  └─ False:
│     ├─ Reset to Track 0
│     └─ Stop Playback

Custom Event: PreviousTrack
├─ Stop Current Audio
├─ Decrement Track Index
├─ Branch (Index >= 0?)
│  ├─ True → Play Track
│  └─ False → Set Index to Last Track
```

### 8.2 Corrupted Audio Effect
**Glitchy, distorted playback:**
```
Variable: bIsCorrupted (Boolean) = false
Variable: CorruptionLevel (Float) = 0.5

Event OnAudioLogStarted
├─ Branch (Is Corrupted?)
│  ├─ True:
│  │  ├─ Apply Distortion Effect
│  │  ├─ Random Audio Skips
│  │  ├─ Pitch Variation
│  │  └─ Visual Glitch Effects
│  └─ False → Normal Playback

Custom Event: ApplyCorruption
├─ Set Audio Pitch (Random 0.8-1.2)
├─ Add Distortion Filter
├─ Random Pause/Resume
└─ Glitch Subtitle Display
```

### 8.3 Recorder Battery System
**Limited playback time:**
```
Variable: BatteryLevel (Float) = 100.0
Variable: BatteryDrainRate (Float) = 10.0

Event Tick (During Playback)
├─ Drain Battery (DeltaTime * DrainRate)
├─ Update Battery UI
├─ Branch (Battery <= 0?)
│  ├─ True:
│  │  ├─ Stop Playback
│  │  ├─ Show "Battery Dead" Message
│  │  └─ Disable Interaction
│  └─ False → Continue

Custom Event: ReplaceBattery
├─ Check Inventory for Battery
├─ Branch (Has Battery?)
│  ├─ True:
│  │  ├─ Consume Battery Item
│  │  ├─ Set Battery Level (100.0)
│  │  └─ Enable Interaction
│  └─ False → Show Message
```

### 8.4 Reverse Playback
**Play audio backwards for secrets:**
```
Variable: bIsReversed (Boolean) = false
Variable: ReversedAudioLog (Sound Base)

Custom Event: ToggleReverse
├─ Stop Current Playback
├─ Toggle Reversed Flag
├─ Branch (Is Reversed?)
│  ├─ True:
│  │  ├─ Set AudioLog (ReversedAudioLog)
│  │  ├─ Show "Reversed" Indicator
│  │  └─ Play Reversed Audio
│  └─ False:
│     ├─ Set AudioLog (Original Audio)
│     ├─ Hide Indicator
│     └─ Play Normal Audio
```

### 8.5 Spectral Analysis
**Visual frequency display:**
```
Widget: SpectralAnalyzer
├─ Frequency Bars (Array)
├─ Waveform Display
└─ Peak Indicators

Event Tick (During Playback)
├─ Get Audio Spectrum Data
├─ Update Frequency Bars
├─ Draw Waveform
├─ Highlight Peaks
└─ Detect Hidden Frequencies
```

### 8.6 Voice Recognition
**Identify speakers in audio:**
```
Variable: DetectedSpeakers (Array of String)
Variable: SpeakerTimestamps (Map)

Event OnAudioLogStarted
├─ Analyze Audio for Speakers
├─ Populate Speaker List
├─ Show Speaker UI
└─ Color-Code Subtitles by Speaker

Custom Event: OnSpeakerDetected
├─ Add Speaker to List
├─ Update Evidence Notes
└─ Publish Event "Speaker.Identified"
```

## 9. UI Integration

### 9.1 Playback HUD
**Display during playback:**
- Audio log title
- Playback progress bar
- Current time / Total duration
- Play/Pause/Stop controls
- Volume slider
- Subtitle display area

### 9.2 Subtitle Widget
**Subtitle display:**
```
Widget: SubtitleDisplay
├─ Background (Semi-transparent black)
├─ Speaker Name (Optional)
├─ Subtitle Text (Large, readable font)
├─ Timestamp Indicator
└─ Fade In/Out Animation
```

### 9.3 Evidence Notification
**When audio log collected:**
```
Widget: AudioLogCollected
├─ Icon (Recorder symbol)
├─ Title ("Audio Log Collected")
├─ Audio Log Name
├─ Brief Description
└─ Fade Out After 3 Seconds
```

## 10. Troubleshooting

### Issue: Audio doesn't play
**Solution:** Verify AudioLog asset is assigned and valid

### Issue: 3D audio not working
**Solution:** Check bUse3DAudio is true and AudioComponent has attenuation settings

### Issue: Audio too quiet/loud
**Solution:** Adjust AudioComponent volume multiplier (0.5-2.0)

### Issue: Subtitles out of sync
**Solution:** Verify subtitle timestamps match audio timing

### Issue: Evidence not collecting
**Solution:** Check bIsEvidence and bAutoCollectAsEvidence are true

### Issue: Can't stop playback
**Solution:** Ensure bStopOnReInteract is true and interaction is enabled

### Issue: Audio loops unexpectedly
**Solution:** Check bLoopAudio setting and OnAudioFinished event

### Issue: Playback progress incorrect
**Solution:** Verify audio duration is correctly calculated

### Issue: Multiple recorders interfere
**Solution:** Ensure each recorder has unique AudioComponent instance

### Issue: Audio cuts off at distance
**Solution:** Increase MaxAudibleDistance and attenuation falloff distance
