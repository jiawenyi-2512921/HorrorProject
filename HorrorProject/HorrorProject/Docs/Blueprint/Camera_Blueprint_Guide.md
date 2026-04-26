# Camera System Blueprint Implementation Guide

## Overview

The Camera system provides three main components for the bodycam mechanic:
- **UCameraPhotoComponent** - Photo capture with 36 photo capacity
- **UCameraRecordingComponent** - Video recording with battery drain
- **UCameraBatteryComponent** - Battery management and charging

All components are designed to work together for the bodycam horror experience.

## C++ Foundation

### CameraPhotoComponent

**Purpose**: Capture photos with flash, store up to 36 photos, detect evidence.

**Key Features**:
- 36 photo capacity (like film camera)
- Flash system with intensity control
- Evidence auto-detection in photos
- Photo metadata (location, timestamp, tags)
- Thumbnail generation
- Photo storage and retrieval

**Photo Resolution**:
- Full Photo: 1920x1080
- Thumbnail: 256x144

### CameraRecordingComponent

**Purpose**: Record video footage with battery drain.

**Key Features**:
- Continuous recording support
- Battery drain during recording
- Recording metadata
- Storage management
- Playback support

### CameraBatteryComponent

**Purpose**: Manage camera battery life and charging.

**Key Features**:
- Battery drain during use
- Charging system
- Low battery warnings
- Battery save mode
- Death when battery depleted

## Blueprint Implementation

### Step 1: Add Camera Components to Player

**In BP_PlayerCharacter**:

**Add Components**:
- CameraPhotoComponent (name: `CameraPhoto`)
- CameraRecordingComponent (name: `CameraRecording`)
- CameraBatteryComponent (name: `CameraBattery`)

**Component Configuration**:

**CameraPhotoComponent**:
- Photo Resolution Width: 1920
- Photo Resolution Height: 1080
- Thumbnail Resolution Width: 256
- Thumbnail Resolution Height: 144
- Max Photo Capacity: 36
- Photo Cooldown Time: 0.5
- Flash Enabled: true
- Flash Intensity: 5000
- Flash Duration: 0.1
- Flash Radius: 1000
- Auto Detect Evidence: true
- Evidence Detection Radius: 2000
- Shutter Sound: SC_Camera_Shutter
- Flash Charge Sound: SC_Camera_FlashCharge
- Shutter Volume: 1.0

**CameraRecordingComponent**:
- Max Recording Duration: 300 (5 minutes)
- Battery Drain Rate: 0.1 per second
- Auto Stop on Low Battery: true
- Low Battery Threshold: 0.1 (10%)
- Recording Quality: High
- Frame Rate: 30

**CameraBatteryComponent**:
- Max Battery Capacity: 100
- Initial Battery Level: 100
- Drain Rate (Idle): 0.01 per second
- Drain Rate (Photo): 2.0 per photo
- Drain Rate (Flash): 5.0 per flash
- Drain Rate (Recording): 0.1 per second
- Charge Rate: 10.0 per second
- Low Battery Threshold: 20
- Critical Battery Threshold: 10
- Death on Battery Depleted: true

### Step 2: Create Camera Input Bindings

**Project Settings → Input → Action Mappings**:

**Add Actions**:
- `TakePhoto` - Mouse Left Click / Gamepad Right Trigger
- `ToggleRecording` - R Key / Gamepad Y Button
- `ToggleFlash` - F Key / Gamepad D-Pad Up
- `ViewPhotos` - Tab Key / Gamepad D-Pad Down
- `DeleteLastPhoto` - Backspace / Gamepad D-Pad Left

**In BP_PlayerCharacter Event Graph**:

#### Take Photo Input
```
Input Action (TakePhoto)
  → CameraPhoto → Can Take Photo
  → Branch (Can Take Photo)
    True:
      → CameraPhoto → Take Photo (bUseFlash = Flash Enabled)
      → CameraBattery → Drain Battery (Photo Cost)
    False:
      → Get UI Manager
      → Show Error Toast ("Cannot take photo")
```

#### Toggle Recording Input
```
Input Action (ToggleRecording)
  → CameraRecording → Is Recording
  → Branch (Is Recording)
    True:
      → CameraRecording → Stop Recording
    False:
      → CameraBattery → Get Battery Level
      → Branch (Battery > 10%)
        True:
          → CameraRecording → Start Recording
        False:
          → Get UI Manager
          → Show Error Toast ("Battery too low")
```

#### Toggle Flash Input
```
Input Action (ToggleFlash)
  → CameraPhoto → Is Flash Enabled
  → Branch (Flash Enabled)
    True:
      → CameraPhoto → Set Flash Enabled (false)
      → Get UI Manager
      → Show Toast ("Flash OFF")
    False:
      → CameraPhoto → Set Flash Enabled (true)
      → Get UI Manager
      → Show Toast ("Flash ON")
```

#### View Photos Input
```
Input Action (ViewPhotos)
  → Get UI Manager
  → Show Photo Gallery (CameraPhoto → Get All Photos)
```

#### Delete Last Photo Input
```
Input Action (DeleteLastPhoto)
  → CameraPhoto → Get All Photos
  → Get Last Element
  → CameraPhoto → Delete Photo (Last Photo ID)
```

### Step 3: Create Photo Gallery Widget

**Create**: `WBP_PhotoGallery`

**Purpose**: View all captured photos.

**Widget Hierarchy**:
```
Canvas Panel
├── Image (Background Blur)
└── Border (Main Panel)
    ├── VerticalBox
    │   ├── HorizontalBox (Header)
    │   │   ├── TextBlock (Title: "Photo Gallery")
    │   │   ├── TextBlock (Photo Count: "12/36")
    │   │   └── Button (Close)
    │   ├── WrapBox (Photo Grid)
    │   │   └── WBP_PhotoThumbnail (repeated)
    │   └── Border (Selected Photo Panel)
    │       ├── Image (Full Photo)
    │       └── VerticalBox (Photo Info)
    │           ├── TextBlock (Timestamp)
    │           ├── TextBlock (Location)
    │           ├── TextBlock (Evidence Detected)
    │           └── Button (Delete Photo)
```

**Variables**:
- `Photos` (Array of FCameraPhoto)
- `SelectedPhoto` (FCameraPhoto)
- `PhotoComponent` (CameraPhotoComponent reference)

**Functions**:

#### PopulateGallery
```
Input: Array<FCameraPhoto> PhotoArray
Set Photos = PhotoArray
Clear Photo Grid
For Each Photo in Photos:
  → Create Widget (WBP_PhotoThumbnail)
  → Set Thumbnail Image (Photo.ThumbnailTexture)
  → Set Photo Data (Photo)
  → Add to Photo Grid
  → Bind On Clicked → SelectPhoto
```

#### SelectPhoto
```
Input: FCameraPhoto Photo
Set SelectedPhoto = Photo
Set Full Photo Image (Photo.PhotoTexture)
Set Timestamp Text (Photo.Metadata.CaptureTimestamp)
Set Location Text (Photo.Metadata.CaptureLocation)
Set Evidence Text (Photo.Metadata.DetectedEvidenceIds)
```

#### DeleteSelectedPhoto
```
PhotoComponent → Delete Photo (SelectedPhoto.Metadata.PhotoId)
Remove from Photos array
Refresh Gallery
```

### Step 4: Create Photo Thumbnail Widget

**Create**: `WBP_PhotoThumbnail`

**Purpose**: Thumbnail display in photo gallery.

**Widget Hierarchy**:
```
Button
└── Overlay
    ├── Image (Thumbnail)
    ├── Image (Frame)
    └── Border (Evidence Badge - if evidence detected)
        └── TextBlock ("!")
```

**Variables**:
- `PhotoData` (FCameraPhoto)
- `bHasEvidence` (bool)

**Functions**:

#### SetPhotoData
```
Input: FCameraPhoto Photo
Set PhotoData = Photo
Set Thumbnail Image (Photo.ThumbnailTexture)
Set bHasEvidence (Photo.Metadata.DetectedEvidenceIds.Num() > 0)
Set Evidence Badge Visibility (bHasEvidence)
```

### Step 5: Create Battery UI Widget

**Create**: `WBP_BatteryIndicator`

**Purpose**: Show battery level in HUD.

**Widget Hierarchy**:
```
HorizontalBox
├── Image (Battery Icon)
├── ProgressBar (Battery Level)
└── TextBlock (Battery Percent)
```

**Variables**:
- `BatteryLevel` (float, 0-1)
- `bIsCharging` (bool)
- `bIsLowBattery` (bool)

**Functions**:

#### UpdateBattery
```
Input: float Level, bool bCharging
Set BatteryLevel = Level
Set bIsCharging = bCharging
Set bIsLowBattery (Level < 0.2)

Set Progress Bar Percent (Level)
Set Battery Percent Text (Format: "{0}%", Level * 100)

Branch (bIsLowBattery)
  True: Set Progress Bar Color (Red)
  False: Set Progress Bar Color (Green)

Branch (bIsCharging)
  True: Play Charging Animation
  False: Stop Charging Animation
```

**Integrate into WBP_BodycamOverlay**:
```
Add WBP_BatteryIndicator to overlay

Event Tick
  → Get CameraBattery Component
  → Get Battery Level
  → Get Is Charging
  → WBP_BatteryIndicator → Update Battery (Level, Charging)
```

### Step 6: Create Flash Effect

**Create**: `BP_CameraFlash` (Actor)

**Purpose**: Visual flash effect when taking photos.

**Components**:
- PointLightComponent (name: `FlashLight`)
- SphereComponent (name: `FlashRadius`)

**Component Configuration**:

**FlashLight**:
- Intensity: 5000
- Attenuation Radius: 1000
- Color: White
- Cast Shadows: false
- Initially Hidden: true

**Event Graph**:
```
Custom Event: TriggerFlash
  Input: float Intensity, float Duration
  
  Set Light Intensity (Intensity)
  Set Light Hidden (false)
  
  Timeline (0 to 1 over Duration)
    → Lerp (Intensity to 0)
    → Set Light Intensity (Lerped Value)
  
  On Timeline Finished:
    → Set Light Hidden (true)
    → Destroy Actor
```

**In BP_PlayerCharacter**:
```
On Photo Taken Event (CameraPhoto)
  → Spawn Actor (BP_CameraFlash)
    - Location: Camera Location
  → BP_CameraFlash → Trigger Flash (5000, 0.1)
```

### Step 7: Create Battery Charging Station

**Create**: `BP_BatteryChargingStation`

**Parent Class**: Actor

**Purpose**: Stationary charging station for camera battery.

**Components**:
- StaticMeshComponent (name: `StationMesh`)
- BoxComponent (name: `ChargingZone`)
- PointLightComponent (name: `StatusLight`)
- AudioComponent (name: `ChargingSound`)

**Variables**:
- `ChargeRate` (float, default: 10.0)
- `bIsCharging` (bool)
- `ChargingPlayer` (PlayerCharacter reference)

**Event Graph**:
```
On Component Begin Overlap (ChargingZone)
  → Cast to PlayerCharacter
  → Set ChargingPlayer = Player
  → Get CameraBattery Component
  → Start Charging
  → Set bIsCharging = true
  → StatusLight → Set Color (Green)
  → ChargingSound → Play

On Component End Overlap (ChargingZone)
  → Get CameraBattery Component
  → Stop Charging
  → Set bIsCharging = false
  → Set ChargingPlayer = null
  → StatusLight → Set Color (Red)
  → ChargingSound → Stop

Event Tick
  → Branch (bIsCharging)
    True:
      → Get CameraBattery Component
      → Charge Battery (ChargeRate * DeltaTime)
```

### Step 8: Create Evidence Detection System

**In BP_PlayerCharacter**:

**Event Graph**:
```
On Photo Taken Event (CameraPhoto)
  Input: FCameraPhoto Photo
  
  → Get All Actors of Class (EvidenceActor)
  → For Each Evidence Actor:
      → Get Distance To Camera
      → Branch (Distance < 2000)
        True:
          → Get Dot Product (Camera Forward, Direction to Evidence)
          → Branch (Dot > 0.7) // In camera view
            True:
              → Add Evidence to Photo Metadata
              → Get UI Manager
              → Show Toast ("Evidence detected in photo!")
```

### Step 9: Create Photo-Based Puzzle System

**Create**: `BP_PhotoPuzzle`

**Purpose**: Puzzle that requires photographing specific objects.

**Variables**:
- `RequiredPhotoTags` (Array of FGameplayTag)
- `CollectedPhotoTags` (Array of FGameplayTag)
- `bPuzzleSolved` (bool)

**Event Graph**:
```
On Photo Taken Event (from Player)
  Input: FCameraPhoto Photo
  
  → For Each Tag in Photo.Metadata.ContextTags:
      → Branch (RequiredPhotoTags contains Tag)
        True:
          → Add to CollectedPhotoTags
          → Check Puzzle Completion

Check Puzzle Completion:
  → Branch (CollectedPhotoTags contains all RequiredPhotoTags)
    True:
      → Set bPuzzleSolved = true
      → Trigger Puzzle Solved Event
      → Unlock Door / Reveal Secret / etc.
```

### Step 10: Create Recording Playback System

**Create**: `WBP_RecordingPlayback`

**Purpose**: Playback recorded footage.

**Widget Hierarchy**:
```
Canvas Panel
└── Border (Main Panel)
    ├── Image (Video Frame)
    ├── HorizontalBox (Controls)
    │   ├── Button (Play/Pause)
    │   ├── Slider (Timeline)
    │   ├── TextBlock (Current Time)
    │   └── TextBlock (Total Duration)
    └── Button (Close)
```

**Variables**:
- `RecordingData` (FCameraRecording)
- `bIsPlaying` (bool)
- `CurrentTime` (float)

**Functions**:

#### PlayRecording
```
Input: FCameraRecording Recording
Set RecordingData = Recording
Set bIsPlaying = true
Start Playback Timeline
```

#### PauseRecording
```
Set bIsPlaying = false
Pause Playback Timeline
```

#### SeekTo
```
Input: float Time
Set CurrentTime = Time
Update Video Frame
```

## Blueprint Usage Examples

### Example 1: Take Photo with Flash

**In BP_PlayerCharacter**:
```
Input Action (TakePhoto)
  → CameraPhoto → Can Take Photo
  → Branch (Can Take Photo)
    True:
      → CameraPhoto → Take Photo (true) // Flash enabled
      → CameraBattery → Drain Battery (7.0) // Photo + Flash cost
```

### Example 2: Auto-Stop Recording on Low Battery

**In BP_PlayerCharacter**:
```
Event Tick
  → CameraRecording → Is Recording
  → Branch (Is Recording)
    True:
      → CameraBattery → Get Battery Level
      → Branch (Battery < 0.1)
        True:
          → CameraRecording → Stop Recording
          → Get UI Manager
          → Show Warning Toast ("Recording stopped - Low battery")
```

### Example 3: Evidence Detection in Photo

**In BP_PlayerCharacter**:
```
On Photo Taken Event (CameraPhoto)
  → Get Photo Metadata
  → Branch (DetectedEvidenceIds.Num() > 0)
    True:
      → For Each Evidence ID:
          → Get Archive Subsystem
          → Mark Evidence as Photographed
          → Award Achievement ("Photographer")
```

### Example 4: Battery Death Mechanic

**In BP_PlayerCharacter**:
```
On Battery Depleted Event (CameraBattery)
  → Get PostProcessController
  → Apply Death Effect
  → Disable Player Input
  → Delay (2.0)
  → Restart Level
```

### Example 5: Photo-Based Objective

**In BP_ObjectiveNode**:
```
On Photo Taken Event (from Player)
  → Check Photo Contains Required Evidence
  → Branch (Contains Required Evidence)
    True:
      → Complete Objective
      → Get UI Manager
      → Show Objective Toast ("Objective Complete: Photograph Evidence")
```

## Event Binding

### CameraPhotoComponent Events

**OnPhotoTaken** (Multicast Delegate):
- Broadcasts when photo taken
- Provides FCameraPhoto data
- Use for UI updates, achievements

**OnFlashFired** (Multicast Delegate):
- Broadcasts when flash fires
- Provides flash intensity
- Use for visual effects, AI detection

**OnPhotoStored** (Multicast Delegate):
- Broadcasts when photo stored
- Provides photo GUID
- Use for save system

### CameraRecordingComponent Events

**OnRecordingStarted** (Multicast Delegate):
- Broadcasts when recording starts
- Use for UI updates

**OnRecordingStopped** (Multicast Delegate):
- Broadcasts when recording stops
- Provides recording data
- Use for UI updates, save system

**OnRecordingFailed** (Multicast Delegate):
- Broadcasts when recording fails
- Provides failure reason
- Use for error messages

### CameraBatteryComponent Events

**OnBatteryLevelChanged** (Multicast Delegate):
- Broadcasts on battery change
- Provides current level
- Use for UI updates

**OnLowBattery** (Multicast Delegate):
- Broadcasts when battery low
- Use for warnings

**OnBatteryDepleted** (Multicast Delegate):
- Broadcasts when battery empty
- Use for death mechanic

**OnChargingStarted** (Multicast Delegate):
- Broadcasts when charging starts
- Use for UI updates

**OnChargingStopped** (Multicast Delegate):
- Broadcasts when charging stops
- Use for UI updates

## Performance Optimization

### Photo System Optimization
- Limit photo resolution (1920x1080 max)
- Compress photo textures
- Generate thumbnails asynchronously
- Limit photo storage (36 photos)
- Clear old photos when full

### Recording System Optimization
- Use lower resolution for recording
- Limit frame rate (30 FPS)
- Compress video data
- Stream to disk instead of memory
- Limit recording duration (5 minutes)

### Battery System Optimization
- Update battery UI at fixed intervals (0.1s)
- Cache battery component references
- Use timers instead of tick for drain
- Batch battery updates

## Troubleshooting

### Photos Not Saving
- Check photo capacity not exceeded
- Verify render target is valid
- Ensure scene capture component initialized
- Check photo texture creation
- Verify storage array is valid

### Flash Not Working
- Check flash enabled
- Verify flash intensity > 0
- Ensure flash light component exists
- Check flash duration > 0
- Verify flash spawn location

### Battery Not Draining
- Check battery component is ticking
- Verify drain rate > 0
- Ensure battery level > 0
- Check battery component is active

### Evidence Not Detected
- Check detection radius (2000 units)
- Verify evidence actors have tags
- Ensure line of sight to evidence
- Check camera forward vector
- Verify evidence in camera view

### Recording Not Working
- Check battery level sufficient
- Verify recording component initialized
- Ensure storage space available
- Check recording duration limit
- Verify frame capture working

## Best Practices

1. **Photo Limit**: Enforce 36 photo limit (film camera feel)
2. **Battery Management**: Make battery a core mechanic
3. **Flash Cost**: Make flash expensive (5x photo cost)
4. **Evidence Detection**: Auto-detect evidence in photos
5. **Visual Feedback**: Clear feedback for all camera actions
6. **Audio Feedback**: Satisfying shutter sound
7. **UI Integration**: Show battery/photo count in HUD
8. **Save System**: Persist photos and battery level
9. **Performance**: Optimize photo capture and storage
10. **Accessibility**: Provide battery warnings

## Testing Checklist

- [ ] Camera components added to player
- [ ] Input bindings configured
- [ ] Photo capture works
- [ ] Flash effect works
- [ ] Photo gallery displays photos
- [ ] Battery drains correctly
- [ ] Battery charging works
- [ ] Recording starts/stops correctly
- [ ] Evidence detection works
- [ ] Photo limit enforced (36)
- [ ] Battery death mechanic works
- [ ] UI updates correctly
- [ ] Save/load preserves photos
- [ ] Performance acceptable
- [ ] No memory leaks

## Next Steps

1. Add camera components to player
2. Configure input bindings
3. Create photo gallery widget
4. Implement battery UI
5. Create flash effect
6. Place charging stations in level
7. Test photo capture
8. Test recording system
9. Implement evidence detection
10. Optimize performance
