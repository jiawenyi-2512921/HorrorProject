# WBP_EvidenceToast Blueprint Implementation Guide

## Overview
Toast notification for evidence collection with fade-in/out animation and typewriter effect.

## C++ Base Class
**Parent Class:** `UEvidenceToastWidget` (inherits from `UUserWidget`)

### Available C++ Functions
```cpp
// Display Functions
void ShowEvidenceToast(FName EvidenceId, const FHorrorEvidenceMetadata& Metadata)
void ShowSimpleToast(FName EvidenceId, const FText& DisplayName)
void DismissToast()

// State Query
bool IsVisible() const
```

### Blueprint Implementable Events
```cpp
BP_ShowToast(FName EvidenceId, const FText& DisplayName, const FText& Description)
BP_DismissToast()
BP_PlayFadeIn()
BP_PlayFadeOut()
```

### Exposed Properties
- `DisplayDuration` (float, EditDefaultsOnly) - Default: 3.0
- `FadeInDuration` (float, EditDefaultsOnly) - Default: 0.3
- `FadeOutDuration` (float, EditDefaultsOnly) - Default: 0.5
- `bIsVisible` (bool, ReadOnly)

---

## Canvas Layout Design

### Screen Layout (1920x1080 reference)
```
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│                                                             │
│                                                             │
│                                                             │
│                                                             │
│                                                             │
│                                                             │
│                                                             │
│  ┌────────────────────────────────────────────────────┐    │
│  │ [!] EVIDENCE COLLECTED                             │    │ ← Toast
│  │ Mysterious Note                                    │    │
│  │ A torn page with cryptic symbols...                │    │
│  └────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

### Widget Hierarchy
```
Canvas Panel (Root)
└── Border_ToastContainer (400x120)
    ├── VerticalBox_Content
    │   ├── HorizontalBox_Header
    │   │   ├── Image_Icon (24x24)
    │   │   ├── Spacer (8px)
    │   │   └── TextBlock_Header
    │   ├── Spacer (8px)
    │   ├── TextBlock_EvidenceName
    │   └── TextBlock_Description
    └── Image_Background (Full)
```

---

## Step-by-Step Implementation

### Step 1: Create Widget Blueprint
1. Content Browser → Right-click → User Interface → Widget Blueprint
2. Name: `WBP_EvidenceToast`
3. Open the blueprint
4. Class Settings → Parent Class → Search "EvidenceToastWidget"
5. Compile and Save

### Step 2: Canvas Root Setup
1. Designer tab → Hierarchy panel
2. Root should be Canvas Panel (default)
3. Canvas Panel settings:
   - Is Variable: False
   - Visibility: Collapsed (initially hidden)

### Step 3: Toast Container

#### Border_ToastContainer
1. Add Border widget to Canvas Panel
2. Rename to `Border_ToastContainer`
3. Anchors: Bottom-Left corner
   - Anchor Min: (0.05, 0.85)
   - Anchor Max: (0.05, 0.85)
4. Position: X=0, Y=0
5. Size: X=450, Y=140
6. Alignment: X=0, Y=0
7. Z-Order: 100
8. Is Variable: True
9. Appearance:
   - Brush Color: Black (0, 0, 0, 0.85)
   - Brush → Draw As: Rounded Box
   - Brush → Rounding → Type: Half Height Radius
   - Brush → Outline Settings → Width: 2.0
   - Brush → Outline Settings → Color: Yellow (1, 0.8, 0, 0.8)
10. Padding: Left=16, Top=12, Right=16, Bottom=12

### Step 4: Background Image (Optional VHS Effect)

#### Image_Background
1. Add Image widget to Border_ToastContainer (as first child)
2. Rename to `Image_Background`
3. Anchors: Fill (0,0 to 1,1)
4. Position: X=0, Y=0
5. Size: X=0, Y=0
6. Z-Order: 0
7. Brush:
   - Tint: White (1, 1, 1, 0.05)
   - Draw As: Image
8. Material: M_FilmGrain (optional)
9. Render Opacity: 0.3

### Step 5: Content Layout

#### VerticalBox_Content
1. Add Vertical Box to Border_ToastContainer
2. Rename to `VerticalBox_Content`
3. Anchors: Fill (0,0 to 1,1)
4. Position: X=0, Y=0
5. Size: X=0, Y=0

### Step 6: Header Section

#### HorizontalBox_Header
1. Add Horizontal Box to VerticalBox_Content
2. Rename to `HorizontalBox_Header`
3. Fill: Horizontal

**Image_Icon:**
- Type: Image
- Size: 24x24 (Fixed)
- Brush → Image: T_EvidenceIcon (or use Material)
- Brush → Tint: Yellow (1, 0.8, 0, 1)
- Brush → Draw As: Image
- Is Variable: False

**Spacer:** Width=8

**TextBlock_Header:**
- Text: "EVIDENCE COLLECTED"
- Font: F_HomeVideo
- Size: 16
- Color: Yellow (1, 0.8, 0, 1)
- Font → Typeface: Bold
- Is Variable: False
- Auto Wrap: False

### Step 7: Evidence Name

#### Spacer (between header and name)
- Size: Height=8

#### TextBlock_EvidenceName
1. Add Text Block to VerticalBox_Content
2. Rename to `TextBlock_EvidenceName`
3. Text: "Evidence Name"
4. Font: F_HomeVideo
5. Size: 20
6. Color: White (1, 1, 1, 1)
7. Font → Typeface: Bold
8. Is Variable: True
9. Auto Wrap: True
10. Justification: Left

### Step 8: Description

#### TextBlock_Description
1. Add Text Block to VerticalBox_Content
2. Rename to `TextBlock_Description`
3. Text: "Description text..."
4. Font: F_HomeVideo
5. Size: 14
6. Color: Light Gray (0.8, 0.8, 0.8, 1)
7. Is Variable: True
8. Auto Wrap: True
9. Justification: Left
10. Max Desired Height: 60

---

## Animation Setup

### Animation 1: FadeIn
**Purpose:** Smooth fade-in with slide up

1. Animations tab → + Animation → Name: "FadeIn"
2. Length: 0.3 seconds
3. Loop: False
4. Tracks:
   - Border_ToastContainer → Render Opacity
     - 0.0s: 0.0
     - 0.3s: 1.0 (Ease Out curve)
   - Border_ToastContainer → Render Transform → Translation Y
     - 0.0s: 30.0
     - 0.3s: 0.0 (Ease Out curve)
   - Border_ToastContainer → Render Transform → Scale
     - 0.0s: (0.95, 0.95)
     - 0.3s: (1.0, 1.0) (Ease Out curve)

### Animation 2: FadeOut
**Purpose:** Smooth fade-out with slide down

1. Animations tab → + Animation → Name: "FadeOut"
2. Length: 0.5 seconds
3. Loop: False
4. Tracks:
   - Border_ToastContainer → Render Opacity
     - 0.0s: 1.0
     - 0.5s: 0.0 (Ease In curve)
   - Border_ToastContainer → Render Transform → Translation Y
     - 0.0s: 0.0
     - 0.5s: 20.0 (Ease In curve)

### Animation 3: IconPulse
**Purpose:** Pulse the evidence icon

1. Animations tab → + Animation → Name: "IconPulse"
2. Length: 1.0 seconds
3. Loop: True
4. Tracks:
   - Image_Icon → Render Transform → Scale
     - 0.0s: (1.0, 1.0)
     - 0.5s: (1.2, 1.2)
     - 1.0s: (1.0, 1.0)
   - Image_Icon → Render Opacity
     - 0.0s: 1.0
     - 0.5s: 0.7
     - 1.0s: 1.0

---

## Event Graph Implementation

### Event Construct
```
Event Construct
├── Set Visibility (Collapsed)
├── Set Render Opacity (0.0)
└── Initialize Material Instances
```

**Blueprint Nodes:**
1. Event Construct
2. Set Visibility (Target: Self, Visibility: Collapsed)
3. Set Render Opacity (Target: Border_ToastContainer, Opacity: 0.0)

### BP_ShowToast Implementation
```
BP_ShowToast (EvidenceId, DisplayName, Description)
├── Set Visibility (Visible)
├── TextBlock_EvidenceName → Set Text (DisplayName)
├── TextBlock_Description → Set Text (Description)
├── Play Animation (FadeIn)
├── Play Animation (IconPulse, Loop)
├── Delay (DisplayDuration)
├── Call BP_DismissToast
```

**Blueprint Nodes:**
1. Event BP_ShowToast (Parameters: EvidenceId, DisplayName, Description)
2. Set Visibility (Target: Self, Visibility: Visible)
3. TextBlock_EvidenceName → Set Text (DisplayName)
4. TextBlock_Description → Set Text (Description)
5. Play Animation (Animation: FadeIn, Play Mode: Forward)
6. Play Animation (Animation: IconPulse, Loop: True)
7. Delay (Duration: DisplayDuration variable)
8. Call Function: BP_DismissToast

### BP_DismissToast Implementation
```
BP_DismissToast
├── Stop Animation (IconPulse)
├── Play Animation (FadeOut)
├── Delay (FadeOutDuration)
└── Set Visibility (Collapsed)
```

**Blueprint Nodes:**
1. Event BP_DismissToast
2. Stop Animation (IconPulse)
3. Play Animation (Animation: FadeOut, Play Mode: Forward)
4. Delay (Duration: FadeOutDuration variable)
5. Set Visibility (Target: Self, Visibility: Collapsed)

### BP_PlayFadeIn Implementation
```
BP_PlayFadeIn
└── Play Animation (FadeIn)
```

### BP_PlayFadeOut Implementation
```
BP_PlayFadeOut
└── Play Animation (FadeOut)
```

### Custom Function: SetToastStyle (Optional)
**Purpose:** Change toast appearance based on evidence type

```
SetToastStyle (EvidenceType)
├── Switch on EvidenceType
│   ├── Document:
│   │   ├── Set Border Color (Yellow)
│   │   └── Set Icon Tint (Yellow)
│   ├── Photo:
│   │   ├── Set Border Color (Cyan)
│   │   └── Set Icon Tint (Cyan)
│   ├── Audio:
│   │   ├── Set Border Color (Green)
│   │   └── Set Icon Tint (Green)
│   └── Physical:
│       ├── Set Border Color (Orange)
│       └── Set Icon Tint (Orange)
```

**Blueprint Nodes:**
1. Function: SetToastStyle (Input: EEvidenceType)
2. Switch on Enum (EvidenceType)
3. Each case sets Border_ToastContainer brush color and Image_Icon tint

---

## Advanced Features

### Typewriter Effect (Optional)
**Purpose:** Animate text appearing character by character

#### Custom Function: PlayTypewriterEffect
```
PlayTypewriterEffect (FullText)
├── Set Text (Empty)
├── For Loop (0 to Text Length)
│   ├── Get Substring (0 to Index)
│   ├── Set Text (Substring)
│   └── Delay (0.05)
```

**Blueprint Nodes:**
1. Function: PlayTypewriterEffect (Input: FText FullText)
2. Convert Text to String
3. Get String Length
4. For Loop (First Index: 0, Last Index: Length)
5. Loop Body:
   - Get Substring (Start: 0, Length: Loop Index)
   - TextBlock_Description → Set Text
   - Delay (0.05 seconds)

### Sound Integration
**Purpose:** Play sound effects on show/dismiss

#### In BP_ShowToast (after Set Visibility)
```
├── Play Sound 2D (SFX_EvidenceCollected)
└── Play Sound 2D (SFX_UINotification)
```

#### In BP_DismissToast (before Play Animation)
```
└── Play Sound 2D (SFX_UIDismiss)
```

---

## Material Setup (Optional)

### M_ToastBackground Material
**Material Type:** User Interface

**Parameters:**
- NoiseAmount (Scalar): 0.05
- ScanlineIntensity (Scalar): 0.1
- TimeOffset (Scalar): 0.0

**Material Graph:**
1. TexCoord → Noise → Multiply (NoiseAmount)
2. Scanline pattern (horizontal lines)
3. Combine and output to Emissive Color
4. Output to Opacity (0.3)

---

## Testing Checklist

### Visual Tests
- [ ] Toast appears in bottom-left corner
- [ ] Border has rounded corners
- [ ] Yellow outline is visible
- [ ] Icon displays correctly
- [ ] Text is readable on dark background
- [ ] Description wraps properly

### Animation Tests
- [ ] FadeIn animation smooth (0.3s)
- [ ] Slide up effect works
- [ ] Scale effect subtle
- [ ] Icon pulse animation loops
- [ ] FadeOut animation smooth (0.5s)
- [ ] Toast disappears after DisplayDuration

### Functional Tests
- [ ] ShowEvidenceToast displays correct info
- [ ] ShowSimpleToast works with minimal data
- [ ] DismissToast hides widget immediately
- [ ] IsVisible returns correct state
- [ ] Multiple toasts queue properly (if implemented)

### Text Tests
- [ ] Evidence name displays correctly
- [ ] Description wraps at container width
- [ ] Long text doesn't overflow
- [ ] Special characters display correctly
- [ ] Typewriter effect smooth (if implemented)

### Performance Tests
- [ ] Animation runs at 60fps
- [ ] No memory leaks from repeated show/hide
- [ ] Material cost minimal
- [ ] Text rendering efficient

---

## Common Issues & Solutions

### Issue: Toast not appearing
**Solution:** Check that Set Visibility is called in BP_ShowToast. Verify Z-Order is high enough (100+).

### Issue: Text overflowing container
**Solution:** Enable Auto Wrap on text blocks. Set Max Desired Height on description. Adjust container size.

### Issue: Animation stuttering
**Solution:** Use Ease Out/In curves instead of Linear. Reduce animation complexity. Check frame rate.

### Issue: Toast appearing in wrong position
**Solution:** Verify anchors are set correctly (0.05, 0.85). Check alignment values. Test at different resolutions.

### Issue: Icon not showing
**Solution:** Verify T_EvidenceIcon texture exists. Check brush tint is not black. Ensure size is not 0.

### Issue: Multiple toasts overlapping
**Solution:** Implement toast queue system. Store pending toasts in array. Show next after current dismisses.

---

## Performance Optimization

### Best Practices
1. Use Collapsed visibility instead of Hidden when not shown
2. Stop animations when widget is hidden
3. Cache material instances
4. Use simple materials for background
5. Limit text length for descriptions
6. Disable tick if not needed

### Recommended Settings
- Update Mode: Tick (only if using typewriter)
- Tick Frequency: Every Frame (only if animating)
- Invalidation: Manual
- Volatility: Not Volatile

---

## Integration with Game

### Triggering from C++
```cpp
// In evidence collection code
UUIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UUIManagerSubsystem>();
UEvidenceToastWidget* Toast = Cast<UEvidenceToastWidget>(
    UIManager->GetWidget("EvidenceToast")
);
if (Toast)
{
    Toast->ShowEvidenceToast(EvidenceId, Metadata);
}
```

### Triggering from Blueprint
1. Get Game Instance → Get Subsystem (UIManagerSubsystem)
2. Get Widget ("EvidenceToast")
3. Cast to EvidenceToastWidget
4. Call ShowEvidenceToast or ShowSimpleToast

### EventBus Integration
```
Event Construct
├── Get UI Event Manager
├── Subscribe to Event (Tag: "Evidence.Collected")
└── Bind to OnEvidenceCollected

OnEvidenceCollected (EventData)
├── Extract Evidence ID and Metadata
└── Call ShowEvidenceToast
```

---

## Toast Queue System (Advanced)

### Purpose
Handle multiple evidence collections in quick succession.

### Implementation
1. Create TArray variable: `ToastQueue`
2. Create struct: `FToastData` (EvidenceId, DisplayName, Description)
3. Modify BP_ShowToast:
   - If currently visible, add to queue instead
   - If not visible, show immediately
4. Modify BP_DismissToast:
   - After hiding, check if queue has items
   - If yes, show next toast from queue

### Blueprint Logic
```
ShowEvidenceToast
├── Branch (IsVisible)
│   ├── True: Add to ToastQueue
│   └── False: Call BP_ShowToast

BP_DismissToast (modified)
├── Stop Animation (IconPulse)
├── Play Animation (FadeOut)
├── Delay (FadeOutDuration)
├── Set Visibility (Collapsed)
├── Branch (ToastQueue.Length > 0)
│   ├── True:
│   │   ├── Get First Element from Queue
│   │   ├── Remove from Queue
│   │   ├── Delay (0.5)
│   │   └── Call BP_ShowToast with queued data
│   └── False: Do nothing
```

---

## Asset Requirements

### Fonts
- F_HomeVideo (VHS-style monospace font)

### Textures
- T_EvidenceIcon (24x24, yellow exclamation mark or document icon)

### Materials (Optional)
- M_ToastBackground (subtle noise/scanline effect)
- M_FilmGrain

### Sounds
- SFX_EvidenceCollected (notification sound)
- SFX_UINotification (UI feedback)
- SFX_UIDismiss (dismiss sound)

### Colors
- Primary: Yellow (1.0, 0.8, 0.0, 1.0)
- Background: Black (0.0, 0.0, 0.0, 0.85)
- Text Primary: White (1.0, 1.0, 1.0, 1.0)
- Text Secondary: Light Gray (0.8, 0.8, 0.8, 1.0)
