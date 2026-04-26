# WBP_BodycamOverlay Blueprint Implementation Guide

## Overview
Bodycam HUD overlay with recording indicator, timestamp, battery level, and VHS effects.

## C++ Base Class
**Parent Class:** `UBodycamOverlayWidget` (inherits from `UUserWidget`)

### Available C++ Functions
```cpp
// State Management
void SetRecordingState(bool bIsRecording)
void SetBatteryLevel(float Percent)  // 0.0 to 1.0
void SetCameraMode(EQuantumCameraMode Mode)
void SetTimestamp(const FDateTime& Time)
void SetVHSIntensity(float Intensity)  // 0.0 to 1.0
void TriggerGlitchEffect(float Duration = 0.5f)

// Getters
bool IsRecording() const
float GetBatteryLevel() const
```

### Blueprint Implementable Events
```cpp
BP_RecordingStateChanged(bool bRecording)
BP_BatteryLevelChanged(float Percent)
BP_CameraModeChanged(EQuantumCameraMode Mode)
BP_TimestampUpdated(const FString& FormattedTime)
BP_VHSIntensityChanged(float Intensity)
BP_GlitchEffectTriggered(float Duration)
```

### Exposed Properties
- `bIsRecording` (bool, ReadOnly)
- `BatteryLevel` (float, ReadOnly)
- `CurrentCameraMode` (EQuantumCameraMode, ReadOnly)
- `VHSIntensity` (float, ReadOnly)
- `TimestampFormat` (string, EditDefaultsOnly) - Default: "{0:02d}:{1:02d}:{2:02d}"
- `RecordingBlinkRate` (float, EditDefaultsOnly) - Default: 1.0

---

## Canvas Layout Design

### Screen Layout (1920x1080 reference)
```
┌─────────────────────────────────────────────────────────────┐
│ ● REC  00:15:32                          [████░░] 80%       │ ← Top Bar
│                                                             │
│                                                             │
│                                                             │
│                                                             │
│                    [GAME VIEW AREA]                         │
│                                                             │
│                                                             │
│                                                             │
│                                                             │
│ [VHS SCANLINES + NOISE OVERLAY - FULL SCREEN]              │
└─────────────────────────────────────────────────────────────┘
```

### Widget Hierarchy
```
Canvas Panel (Root)
├── Image_VHSOverlay (Full Screen)
│   └── Material: M_VHS_Overlay
├── Image_Scanlines (Full Screen)
│   └── Material: M_Scanlines
├── Image_NoiseOverlay (Full Screen)
│   └── Material: M_FilmGrain
├── Overlay_TopBar
│   ├── HorizontalBox_RecordingInfo
│   │   ├── Image_RecDot (16x16)
│   │   ├── Spacer (8px)
│   │   ├── TextBlock_RecLabel
│   │   ├── Spacer (16px)
│   │   └── TextBlock_Timestamp
│   └── HorizontalBox_BatteryInfo
│       ├── ProgressBar_Battery (120x20)
│       ├── Spacer (8px)
│       └── TextBlock_BatteryPercent
└── Image_VignetteEdge (Full Screen)
    └── Material: M_Vignette
```

---

## Step-by-Step Implementation

### Step 1: Create Widget Blueprint
1. Content Browser → Right-click → User Interface → Widget Blueprint
2. Name: `WBP_BodycamOverlay`
3. Open the blueprint
4. Class Settings → Parent Class → Search "BodycamOverlayWidget"
5. Compile and Save

### Step 2: Canvas Root Setup
1. Designer tab → Hierarchy panel
2. Root should be Canvas Panel (default)
3. Canvas Panel settings:
   - Is Variable: False
   - Visibility: Visible

### Step 3: VHS Effect Layers (Bottom to Top)

#### Image_VHSOverlay
1. Add Image widget to Canvas Panel
2. Rename to `Image_VHSOverlay`
3. Anchors: Fill screen (0,0 to 1,1)
4. Position: X=0, Y=0
5. Size: X=0, Y=0 (fills to anchors)
6. Z-Order: 0 (bottom layer)
7. Appearance:
   - Brush → Image: None
   - Brush → Tint: White (1,1,1,1)
   - Brush → Draw As: Image
8. Material:
   - Brush → Resource Object → Search "M_VHS_Overlay"
   - If not found, create material (see Material Setup section)
9. Is Variable: True (check box)
10. Render Opacity: 0.3

#### Image_Scanlines
1. Add Image widget to Canvas Panel
2. Rename to `Image_Scanlines`
3. Anchors: Fill screen (0,0 to 1,1)
4. Position: X=0, Y=0
5. Size: X=0, Y=0
6. Z-Order: 1
7. Material: M_Scanlines
8. Is Variable: True
9. Render Opacity: 0.2

#### Image_NoiseOverlay
1. Add Image widget to Canvas Panel
2. Rename to `Image_NoiseOverlay`
3. Anchors: Fill screen (0,0 to 1,1)
4. Position: X=0, Y=0
5. Size: X=0, Y=0
6. Z-Order: 2
7. Material: M_FilmGrain
8. Is Variable: True
9. Render Opacity: 0.15

### Step 4: Top Bar Overlay

#### Overlay_TopBar
1. Add Overlay widget to Canvas Panel
2. Rename to `Overlay_TopBar`
3. Anchors: Top stretch (0,0 to 1,0)
4. Position: X=0, Y=0
5. Size: X=0, Y=60
6. Z-Order: 10
7. Padding: Left=20, Top=10, Right=20, Bottom=10

#### Recording Info (Left Side)
1. Add Horizontal Box to Overlay_TopBar
2. Rename to `HorizontalBox_RecordingInfo`
3. Alignment: Left + Top
4. Add children:

**Image_RecDot:**
- Type: Image
- Size: 16x16
- Brush → Tint: Red (1, 0, 0, 1)
- Brush → Draw As: Rounded Box
- Is Variable: True
- Visibility: Collapsed (initially)

**Spacer:** Width=8

**TextBlock_RecLabel:**
- Text: "REC"
- Font: F_HomeVideo
- Size: 18
- Color: Red (1, 0, 0, 1)
- Is Variable: True
- Visibility: Collapsed (initially)

**Spacer:** Width=16

**TextBlock_Timestamp:**
- Text: "00:00:00"
- Font: F_HomeVideo
- Size: 18
- Color: White (1, 1, 1, 0.9)
- Is Variable: True

#### Battery Info (Right Side)
1. Add Horizontal Box to Overlay_TopBar
2. Rename to `HorizontalBox_BatteryInfo`
3. Alignment: Right + Top
4. Add children:

**ProgressBar_Battery:**
- Size: 120x20
- Fill Type: Left to Right
- Percent: 1.0
- Fill Color: Green (0, 1, 0, 1)
- Background Color: Dark Gray (0.1, 0.1, 0.1, 0.8)
- Is Variable: True

**Spacer:** Width=8

**TextBlock_BatteryPercent:**
- Text: "100%"
- Font: F_HomeVideo
- Size: 16
- Color: White (1, 1, 1, 0.9)
- Is Variable: True

### Step 5: Vignette Edge Effect
1. Add Image widget to Canvas Panel
2. Rename to `Image_VignetteEdge`
3. Anchors: Fill screen (0,0 to 1,1)
4. Position: X=0, Y=0
5. Size: X=0, Y=0
6. Z-Order: 20 (top layer)
7. Material: M_Vignette
8. Render Opacity: 0.4
9. Hit Test Invisible: True

---

## Animation Setup

### Animation 1: RecordingBlink
**Purpose:** Blink the REC indicator

1. Animations tab → + Animation → Name: "RecordingBlink"
2. Length: 1.0 seconds
3. Loop: True
4. Tracks:
   - Image_RecDot → Render Opacity
     - 0.0s: 1.0
     - 0.5s: 0.0
     - 1.0s: 1.0
   - TextBlock_RecLabel → Render Opacity
     - 0.0s: 1.0
     - 0.5s: 0.3
     - 1.0s: 1.0

### Animation 2: GlitchEffect
**Purpose:** VHS glitch distortion

1. Animations tab → + Animation → Name: "GlitchEffect"
2. Length: 0.5 seconds
3. Loop: False
4. Tracks:
   - Image_VHSOverlay → Render Opacity
     - 0.0s: 0.3
     - 0.1s: 0.8
     - 0.2s: 0.2
     - 0.3s: 0.9
     - 0.5s: 0.3
   - Image_NoiseOverlay → Render Opacity
     - 0.0s: 0.15
     - 0.25s: 0.6
     - 0.5s: 0.15

### Animation 3: LowBatteryPulse
**Purpose:** Pulse battery bar when low

1. Animations tab → + Animation → Name: "LowBatteryPulse"
2. Length: 1.0 seconds
3. Loop: True
4. Tracks:
   - ProgressBar_Battery → Fill Color
     - 0.0s: Red (1, 0, 0, 1)
     - 0.5s: Dark Red (0.5, 0, 0, 1)
     - 1.0s: Red (1, 0, 0, 1)

---

## Event Graph Implementation

### Event Construct
```
Event Construct
├── Get Game Instance
├── Get Subsystem (UIManagerSubsystem)
├── Get Event Manager
├── Subscribe to Event (Tag: "Camera.Recording.Started")
│   └── Bind to OnRecordingStarted
├── Subscribe to Event (Tag: "Camera.Recording.Stopped")
│   └── Bind to OnRecordingStopped
├── Subscribe to Event (Tag: "Camera.Battery.Updated")
│   └── Bind to OnBatteryUpdated
└── Set Visibility (Visible)
```

**Blueprint Nodes:**
1. Event Construct
2. Get Game Instance → Cast to HorrorGameInstance
3. Get Subsystem (Class: UIManagerSubsystem)
4. Get Event Manager (from subsystem)
5. Subscribe to Event (Event Manager)
   - Event Tag: Make Gameplay Tag ("Camera.Recording.Started")
   - Create Event → Custom Event "OnRecordingStarted"
6. Repeat for other events

### BP_RecordingStateChanged Implementation
```
BP_RecordingStateChanged (bRecording)
├── Branch (bRecording)
│   ├── True:
│   │   ├── Set Visibility (Image_RecDot → Visible)
│   │   ├── Set Visibility (TextBlock_RecLabel → Visible)
│   │   └── Play Animation (RecordingBlink, Loop)
│   └── False:
│       ├── Set Visibility (Image_RecDot → Collapsed)
│       ├── Set Visibility (TextBlock_RecLabel → Collapsed)
│       └── Stop Animation (RecordingBlink)
```

**Blueprint Nodes:**
1. Event BP_RecordingStateChanged (bRecording parameter)
2. Branch (Condition: bRecording)
3. True path:
   - Image_RecDot → Set Visibility (Visible)
   - TextBlock_RecLabel → Set Visibility (Visible)
   - Play Animation (Animation: RecordingBlink, Loop: True)
4. False path:
   - Image_RecDot → Set Visibility (Collapsed)
   - TextBlock_RecLabel → Set Visibility (Collapsed)
   - Stop Animation (RecordingBlink)

### BP_BatteryLevelChanged Implementation
```
BP_BatteryLevelChanged (Percent)
├── ProgressBar_Battery → Set Percent (Percent)
├── Format Text ("{0}%", Percent * 100)
├── TextBlock_BatteryPercent → Set Text
├── Branch (Percent < 0.2)
│   ├── True:
│   │   ├── Set Fill Color (Red)
│   │   └── Play Animation (LowBatteryPulse, Loop)
│   └── False:
│       ├── Stop Animation (LowBatteryPulse)
│       └── Branch (Percent < 0.5)
│           ├── True: Set Fill Color (Yellow)
│           └── False: Set Fill Color (Green)
```

**Blueprint Nodes:**
1. Event BP_BatteryLevelChanged (Percent parameter)
2. ProgressBar_Battery → Set Percent (Percent)
3. Multiply (Percent * 100) → Round → Format Text
4. TextBlock_BatteryPercent → Set Text
5. Branch (Percent < 0.2)
6. Color selection logic with nested branches

### BP_TimestampUpdated Implementation
```
BP_TimestampUpdated (FormattedTime)
└── TextBlock_Timestamp → Set Text (FormattedTime)
```

### BP_VHSIntensityChanged Implementation
```
BP_VHSIntensityChanged (Intensity)
├── Image_VHSOverlay → Get Dynamic Material
├── Set Scalar Parameter Value ("Intensity", Intensity)
├── Image_NoiseOverlay → Get Dynamic Material
└── Set Scalar Parameter Value ("NoiseAmount", Intensity * 0.5)
```

**Blueprint Nodes:**
1. Event BP_VHSIntensityChanged (Intensity parameter)
2. Image_VHSOverlay → Get Dynamic Material Instance
3. Set Scalar Parameter Value (Parameter Name: "Intensity", Value: Intensity)
4. Image_NoiseOverlay → Get Dynamic Material Instance
5. Multiply (Intensity * 0.5)
6. Set Scalar Parameter Value (Parameter Name: "NoiseAmount")

### BP_GlitchEffectTriggered Implementation
```
BP_GlitchEffectTriggered (Duration)
├── Play Animation (GlitchEffect)
├── Delay (Duration)
└── Stop Animation (GlitchEffect)
```

### Event Tick (Optional - for scanline animation)
```
Event Tick (Delta Time)
├── Add (CurrentTime + Delta Time)
├── Image_Scanlines → Get Dynamic Material
└── Set Scalar Parameter Value ("TimeOffset", CurrentTime)
```

---

## Material Setup

### M_VHS_Overlay Material
**Material Type:** User Interface

**Parameters:**
- Intensity (Scalar): 0.3
- ChromaticAberration (Scalar): 0.5
- DistortionAmount (Scalar): 0.2
- TimeOffset (Scalar): 0.0

**Material Graph:**
1. TexCoord → Multiply (by DistortionAmount) → Add (TimeOffset)
2. Sine wave distortion for horizontal lines
3. Chromatic aberration offset (R, G, B channels)
4. Multiply by Intensity
5. Output to Emissive Color

### M_Scanlines Material
**Material Type:** User Interface

**Parameters:**
- ScanlineCount (Scalar): 480.0
- ScanlineIntensity (Scalar): 0.2
- ScanlineSpeed (Scalar): 0.5
- TimeOffset (Scalar): 0.0

**Material Graph:**
1. TexCoord Y → Multiply (ScanlineCount)
2. Add (TimeOffset * ScanlineSpeed)
3. Frac → Step (0.5) → Multiply (ScanlineIntensity)
4. Output to Opacity

### M_FilmGrain Material
**Material Type:** User Interface

**Parameters:**
- NoiseAmount (Scalar): 0.15
- NoiseScale (Scalar): 1.0
- TimeOffset (Scalar): 0.0

**Material Graph:**
1. TexCoord → Multiply (NoiseScale)
2. Add (TimeOffset) → Noise node
3. Multiply (NoiseAmount)
4. Output to Emissive Color + Opacity

### M_Vignette Material
**Material Type:** User Interface

**Parameters:**
- VignetteIntensity (Scalar): 0.4
- VignetteRadius (Scalar): 0.7

**Material Graph:**
1. TexCoord → Subtract (0.5, 0.5) → Length
2. Smoothstep (VignetteRadius, 1.0)
3. Multiply (VignetteIntensity)
4. Output to Opacity

---

## Testing Checklist

### Visual Tests
- [ ] Widget displays correctly at 1920x1080
- [ ] Widget scales correctly at different resolutions
- [ ] VHS effects are visible but not overwhelming
- [ ] Scanlines animate smoothly
- [ ] Noise overlay is subtle
- [ ] Vignette darkens edges appropriately

### Functional Tests
- [ ] REC indicator blinks when recording
- [ ] REC indicator hidden when not recording
- [ ] Timestamp updates every second
- [ ] Battery bar updates correctly
- [ ] Battery color changes: Green > Yellow > Red
- [ ] Low battery pulse animation triggers at <20%
- [ ] Glitch effect plays on trigger
- [ ] VHS intensity adjusts dynamically

### Event Tests
- [ ] EventBus subscription works on construct
- [ ] Recording events trigger correctly
- [ ] Battery events trigger correctly
- [ ] Camera mode changes handled
- [ ] No memory leaks from event subscriptions

### Performance Tests
- [ ] Widget tick cost < 0.1ms
- [ ] Material instructions < 100
- [ ] No unnecessary redraws
- [ ] Animations run at 60fps

---

## Common Issues & Solutions

### Issue: REC indicator not blinking
**Solution:** Check that RecordingBlink animation is set to Loop and Play Animation is called with Loop=True.

### Issue: VHS effects too strong
**Solution:** Adjust material parameter Intensity values. Recommended: VHS=0.3, Scanlines=0.2, Noise=0.15.

### Issue: Battery bar not updating
**Solution:** Verify EventBus subscription to "Camera.Battery.Updated" tag. Check that SetBatteryLevel is called from C++.

### Issue: Timestamp not updating
**Solution:** Ensure NativeTick is enabled in C++ and BP_TimestampUpdated is implemented.

### Issue: Materials not showing
**Solution:** Check that materials are set to User Interface domain. Verify material instances are created.

### Issue: Performance issues
**Solution:** Disable Event Tick if not needed. Use material parameters instead of recreating materials. Cache dynamic material instances.

---

## Performance Optimization

### Best Practices
1. Cache dynamic material instances in Event Construct
2. Only update materials when values change
3. Use Invalidation Box for static elements
4. Disable tick when widget is hidden
5. Use material parameters instead of texture swaps
6. Batch material parameter updates

### Recommended Settings
- Update Mode: Tick
- Tick Frequency: Every Frame (only if needed)
- Invalidation: Manual (for static elements)
- Volatility: Not Volatile (unless animating)

---

## Integration with Game

### Adding to Player HUD
```cpp
// In Player Controller or HUD class
UUIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UUIManagerSubsystem>();
UUserWidget* BodycamWidget = UIManager->CreateWidget(WBP_BodycamOverlay, "BodycamOverlay");
UIManager->ShowWidget("BodycamOverlay");
```

### Blueprint Integration
1. Open Player Controller blueprint
2. Event BeginPlay
3. Get Game Instance → Get Subsystem (UIManagerSubsystem)
4. Create Widget (Class: WBP_BodycamOverlay, Name: "BodycamOverlay")
5. Show Widget ("BodycamOverlay")

### Updating from Gameplay
```cpp
// From any gameplay code
UBodycamOverlayWidget* Overlay = Cast<UBodycamOverlayWidget>(
    UIManager->GetWidget("BodycamOverlay")
);
if (Overlay)
{
    Overlay->SetRecordingState(true);
    Overlay->SetBatteryLevel(0.75f);
}
```

---

## Asset Requirements

### Fonts
- F_HomeVideo (VHS-style monospace font)

### Materials
- M_VHS_Overlay
- M_Scanlines
- M_FilmGrain
- M_Vignette

### Textures
- T_RecDot (optional, can use solid color)
- T_BatteryIcon (optional)

### Sounds (for future integration)
- SFX_RecordingStart
- SFX_RecordingStop
- SFX_LowBattery
- SFX_VHSGlitch
