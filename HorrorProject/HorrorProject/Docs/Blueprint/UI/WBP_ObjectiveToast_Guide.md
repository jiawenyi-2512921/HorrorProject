# WBP_ObjectiveToast Blueprint Implementation Guide

## Overview
Toast notification for objective updates with typewriter text effect and slide-in animation.

## C++ Base Class
**Parent Class:** `UObjectiveToastWidget` (inherits from `UUserWidget`)

### Available C++ Functions
```cpp
// Display Functions
void ShowObjectiveToast(FGameplayTag EventTag, const FText& ObjectiveText)
void ShowObjectiveWithHint(FGameplayTag EventTag, const FText& ObjectiveText, const FText& HintText)
void DismissToast()

// State Query
bool IsVisible() const
```

### Blueprint Implementable Events
```cpp
BP_ShowToast(FGameplayTag EventTag, const FText& ObjectiveText, const FText& HintText)
BP_DismissToast()
BP_PlayTypewriterEffect(const FText& Text, float Duration)
BP_PlaySlideIn()
BP_PlaySlideOut()
```

### Exposed Properties
- `DisplayDuration` (float, EditDefaultsOnly) - Default: 5.0
- `TypewriterSpeed` (float, EditDefaultsOnly) - Default: 0.05
- `SlideInDuration` (float, EditDefaultsOnly) - Default: 0.4
- `SlideOutDuration` (float, EditDefaultsOnly) - Default: 0.3
- `bIsVisible` (bool, ReadOnly)
- `CurrentEventTag` (FGameplayTag, ReadOnly)

---

## Canvas Layout Design

### Screen Layout (1920x1080 reference)
```
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│  ┌────────────────────────────────────────────────────┐    │
│  │ >> NEW OBJECTIVE                                   │    │ ← Toast
│  │                                                    │    │
│  │ Find the basement key                              │    │
│  │ [Hint: Check the kitchen drawers]                  │    │
│  └────────────────────────────────────────────────────┘    │
│                                                             │
│                                                             │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Widget Hierarchy
```
Canvas Panel (Root)
└── Border_ToastContainer (500x150)
    ├── VerticalBox_Content
    │   ├── HorizontalBox_Header
    │   │   ├── TextBlock_Arrow
    │   │   ├── Spacer (8px)
    │   │   └── TextBlock_Header
    │   ├── Spacer (12px)
    │   ├── TextBlock_ObjectiveText
    │   ├── Spacer (8px)
    │   └── TextBlock_HintText
    └── Image_Background
```

---

## Step-by-Step Implementation

### Step 1: Create Widget Blueprint
1. Content Browser → Right-click → User Interface → Widget Blueprint
2. Name: `WBP_ObjectiveToast`
3. Open the blueprint
4. Class Settings → Parent Class → Search "ObjectiveToastWidget"
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
3. Anchors: Top-Left corner
   - Anchor Min: (0.05, 0.1)
   - Anchor Max: (0.05, 0.1)
4. Position: X=-550, Y=0 (off-screen left for slide-in)
5. Size: X=500, Y=150
6. Alignment: X=0, Y=0
7. Z-Order: 100
8. Is Variable: True
9. Appearance:
   - Brush Color: Dark Blue (0.05, 0.1, 0.2, 0.9)
   - Brush → Draw As: Rounded Box
   - Brush → Rounding → Type: Half Height Radius
   - Brush → Outline Settings → Width: 2.0
   - Brush → Outline Settings → Color: Cyan (0, 0.8, 1, 0.9)
10. Padding: Left=20, Top=16, Right=20, Bottom=16

### Step 4: Background Image

#### Image_Background
1. Add Image widget to Border_ToastContainer (as first child)
2. Rename to `Image_Background`
3. Anchors: Fill (0,0 to 1,1)
4. Position: X=0, Y=0
5. Size: X=0, Y=0
6. Z-Order: 0
7. Brush:
   - Tint: Cyan (0, 0.8, 1, 0.05)
   - Draw As: Image
8. Material: M_ScanlineSubtle (optional)
9. Render Opacity: 0.2

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

**TextBlock_Arrow:**
- Text: ">>"
- Font: F_HomeVideo
- Size: 20
- Color: Cyan (0, 0.8, 1, 1)
- Font → Typeface: Bold
- Is Variable: True (for animation)
- Auto Wrap: False

**Spacer:** Width=8

**TextBlock_Header:**
- Text: "NEW OBJECTIVE"
- Font: F_HomeVideo
- Size: 18
- Color: Cyan (0, 0.8, 1, 1)
- Font → Typeface: Bold
- Is Variable: False
- Auto Wrap: False

### Step 7: Objective Text

#### Spacer (between header and objective)
- Size: Height=12

#### TextBlock_ObjectiveText
1. Add Text Block to VerticalBox_Content
2. Rename to `TextBlock_ObjectiveText`
3. Text: "Objective text will appear here..."
4. Font: F_HomeVideo
5. Size: 22
6. Color: White (1, 1, 1, 1)
7. Font → Typeface: Bold
8. Is Variable: True
9. Auto Wrap: True
10. Justification: Left

### Step 8: Hint Text

#### Spacer (between objective and hint)
- Size: Height=8

#### TextBlock_HintText
1. Add Text Block to VerticalBox_Content
2. Rename to `TextBlock_HintText`
3. Text: "[Hint text]"
4. Font: F_HomeVideo
5. Size: 14
6. Color: Light Cyan (0.6, 0.9, 1, 0.8)
7. Font → Typeface: Italic (if available)
8. Is Variable: True
9. Auto Wrap: True
10. Justification: Left
11. Visibility: Collapsed (initially)

---

## Animation Setup

### Animation 1: SlideIn
**Purpose:** Slide in from left with bounce

1. Animations tab → + Animation → Name: "SlideIn"
2. Length: 0.4 seconds
3. Loop: False
4. Tracks:
   - Border_ToastContainer → Slot → Position X
     - 0.0s: -550.0
     - 0.4s: 0.0 (Ease Out Bounce curve)
   - Border_ToastContainer → Render Opacity
     - 0.0s: 0.0
     - 0.1s: 1.0 (Ease Out curve)

### Animation 2: SlideOut
**Purpose:** Slide out to left

1. Animations tab → + Animation → Name: "SlideOut"
2. Length: 0.3 seconds
3. Loop: False
4. Tracks:
   - Border_ToastContainer → Slot → Position X
     - 0.0s: 0.0
     - 0.3s: -550.0 (Ease In curve)
   - Border_ToastContainer → Render Opacity
     - 0.0s: 1.0
     - 0.3s: 0.0 (Ease In curve)

### Animation 3: ArrowBlink
**Purpose:** Blink the >> arrow indicator

1. Animations tab → + Animation → Name: "ArrowBlink"
2. Length: 0.8 seconds
3. Loop: True
4. Tracks:
   - TextBlock_Arrow → Render Opacity
     - 0.0s: 1.0
     - 0.4s: 0.3
     - 0.8s: 1.0
   - TextBlock_Arrow → Render Transform → Translation X
     - 0.0s: 0.0
     - 0.4s: 5.0
     - 0.8s: 0.0

### Animation 4: HeaderPulse
**Purpose:** Subtle pulse on header

1. Animations tab → + Animation → Name: "HeaderPulse"
2. Length: 2.0 seconds
3. Loop: True
4. Tracks:
   - TextBlock_Header → Render Transform → Scale
     - 0.0s: (1.0, 1.0)
     - 1.0s: (1.05, 1.05)
     - 2.0s: (1.0, 1.0)

---

## Event Graph Implementation

### Event Construct
```
Event Construct
├── Set Visibility (Collapsed)
├── Set Position X (-550)
├── Get UI Event Manager
├── Subscribe to Event (Tag: "Objective.Updated")
└── Bind to OnObjectiveUpdated
```

**Blueprint Nodes:**
1. Event Construct
2. Set Visibility (Target: Self, Visibility: Collapsed)
3. Border_ToastContainer → Get Slot as Canvas Slot → Set Position (X: -550, Y: 0)
4. Get Game Instance → Get Subsystem (UIManagerSubsystem)
5. Get Event Manager
6. Subscribe to Event (Tag: "Objective.Updated")
7. Create Custom Event: OnObjectiveUpdated

### BP_ShowToast Implementation
```
BP_ShowToast (EventTag, ObjectiveText, HintText)
├── Set Visibility (Visible)
├── Store EventTag in CurrentEventTag
├── Branch (HintText is Empty)
│   ├── True: Hide HintText
│   └── False: Show HintText
├── Call BP_PlaySlideIn
├── Call BP_PlayTypewriterEffect (ObjectiveText)
├── Branch (HintText not Empty)
│   └── True: Call BP_PlayTypewriterEffect (HintText) with delay
├── Play Animation (ArrowBlink, Loop)
├── Play Animation (HeaderPulse, Loop)
├── Delay (DisplayDuration)
└── Call BP_DismissToast
```

**Blueprint Nodes:**
1. Event BP_ShowToast (Parameters: EventTag, ObjectiveText, HintText)
2. Set Visibility (Target: Self, Visibility: Visible)
3. Set CurrentEventTag = EventTag
4. Branch (Is Empty: HintText)
5. True: TextBlock_HintText → Set Visibility (Collapsed)
6. False: TextBlock_HintText → Set Visibility (Visible)
7. Call BP_PlaySlideIn
8. Call BP_PlayTypewriterEffect (Target: TextBlock_ObjectiveText, Text: ObjectiveText)
9. Branch (Not Empty: HintText)
10. True: Delay (TypewriterSpeed * ObjectiveText.Length) → Call BP_PlayTypewriterEffect (Target: TextBlock_HintText, Text: HintText)
11. Play Animation (ArrowBlink, Loop: True)
12. Play Animation (HeaderPulse, Loop: True)
13. Delay (DisplayDuration)
14. Call BP_DismissToast

### BP_DismissToast Implementation
```
BP_DismissToast
├── Stop Animation (ArrowBlink)
├── Stop Animation (HeaderPulse)
├── Call BP_PlaySlideOut
├── Delay (SlideOutDuration)
└── Set Visibility (Collapsed)
```

**Blueprint Nodes:**
1. Event BP_DismissToast
2. Stop Animation (ArrowBlink)
3. Stop Animation (HeaderPulse)
4. Call BP_PlaySlideOut
5. Delay (Duration: SlideOutDuration)
6. Set Visibility (Target: Self, Visibility: Collapsed)

### BP_PlaySlideIn Implementation
```
BP_PlaySlideIn
└── Play Animation (SlideIn)
```

### BP_PlaySlideOut Implementation
```
BP_PlaySlideOut
└── Play Animation (SlideOut)
```

### BP_PlayTypewriterEffect Implementation
```
BP_PlayTypewriterEffect (Text, Duration)
├── Calculate CharDelay (Duration / Text.Length)
├── Set Text to Empty
├── Convert Text to String
├── For Each Character
│   ├── Append Character to Current Text
│   ├── Set Text Block Text
│   └── Delay (CharDelay)
```

**Blueprint Nodes:**
1. Event BP_PlayTypewriterEffect (Parameters: Text, Duration)
2. Convert Text to String
3. Get String Length
4. Divide (Duration / Length) → CharDelay
5. Set Text (Empty)
6. For Loop (0 to Length)
7. Loop Body:
   - Get Substring (Start: 0, Length: Loop Index + 1)
   - Set Text (Substring)
   - Delay (CharDelay)

### Custom Function: PlayTypewriterOnTextBlock
**Purpose:** Reusable typewriter effect for any text block

**Inputs:**
- TargetTextBlock (Text Block reference)
- FullText (FText)
- CharacterDelay (float)

**Implementation:**
```
PlayTypewriterOnTextBlock
├── Set Text (TargetTextBlock, Empty)
├── Convert FullText to String
├── Get String Length
├── For Loop (0 to Length)
│   ├── Get Substring (0 to Index+1)
│   ├── Convert to Text
│   ├── Set Text (TargetTextBlock, Substring)
│   └── Delay (CharacterDelay)
```

---

## Advanced Features

### Sound Integration

#### In BP_ShowToast (after Set Visibility)
```
├── Play Sound 2D (SFX_ObjectiveUpdate)
└── Play Sound 2D (SFX_UISlideIn)
```

#### In BP_PlayTypewriterEffect (in loop)
```
└── Play Sound 2D (SFX_TypewriterClick) with Volume 0.1
```

### Objective Type Styling
**Purpose:** Different colors for different objective types

#### Custom Function: SetObjectiveStyle
**Inputs:**
- EventTag (FGameplayTag)

**Implementation:**
```
SetObjectiveStyle (EventTag)
├── Switch on EventTag
│   ├── "Objective.Main":
│   │   ├── Set Border Color (Cyan)
│   │   ├── Set Header Color (Cyan)
│   │   └── Set Arrow Color (Cyan)
│   ├── "Objective.Optional":
│   │   ├── Set Border Color (Yellow)
│   │   ├── Set Header Color (Yellow)
│   │   └── Set Arrow Color (Yellow)
│   ├── "Objective.Hidden":
│   │   ├── Set Border Color (Purple)
│   │   ├── Set Header Color (Purple)
│   │   └── Set Arrow Color (Purple)
│   └── "Objective.Failed":
│       ├── Set Border Color (Red)
│       ├── Set Header Color (Red)
│       └── Set Arrow Color (Red)
```

**Blueprint Nodes:**
1. Function: SetObjectiveStyle (Input: FGameplayTag EventTag)
2. Switch on Gameplay Tag (EventTag)
3. Each case:
   - Border_ToastContainer → Set Brush Color
   - TextBlock_Header → Set Color and Opacity
   - TextBlock_Arrow → Set Color and Opacity

### Progress Indicator (Optional)
**Purpose:** Show objective progress (e.g., "2/5 items collected")

#### Add to Widget Hierarchy
```
VerticalBox_Content
└── HorizontalBox_Progress (after ObjectiveText)
    ├── ProgressBar_Objective (200x8)
    └── TextBlock_Progress ("2/5")
```

#### Implementation
1. Add Horizontal Box after TextBlock_ObjectiveText
2. Add Progress Bar (200x8, thin bar)
3. Add Text Block for progress text
4. Create function: UpdateProgress(Current, Total)
5. Calculate percent: Current / Total
6. Update progress bar and text

---

## Testing Checklist

### Visual Tests
- [ ] Toast appears in top-left corner
- [ ] Border has rounded corners and cyan outline
- [ ] Arrow indicator visible and animated
- [ ] Text is readable on dark background
- [ ] Hint text displays when provided
- [ ] Hint text hidden when not provided

### Animation Tests
- [ ] SlideIn animation smooth with bounce (0.4s)
- [ ] SlideOut animation smooth (0.3s)
- [ ] Arrow blink animation loops
- [ ] Header pulse animation subtle
- [ ] Typewriter effect smooth
- [ ] Toast disappears after DisplayDuration

### Functional Tests
- [ ] ShowObjectiveToast displays objective
- [ ] ShowObjectiveWithHint displays both texts
- [ ] DismissToast hides widget immediately
- [ ] IsVisible returns correct state
- [ ] EventTag stored correctly
- [ ] Multiple objectives queue properly

### Typewriter Tests
- [ ] Characters appear one by one
- [ ] Speed controlled by TypewriterSpeed
- [ ] Works with long text
- [ ] Works with special characters
- [ ] Hint typewriter starts after objective

### Performance Tests
- [ ] Animation runs at 60fps
- [ ] Typewriter effect efficient
- [ ] No memory leaks from repeated show/hide
- [ ] Material cost minimal

---

## Common Issues & Solutions

### Issue: Toast not sliding in
**Solution:** Check initial Position X is -550. Verify SlideIn animation targets Slot → Position X, not Render Transform.

### Issue: Typewriter effect too fast/slow
**Solution:** Adjust TypewriterSpeed property (default 0.05). Lower = faster, higher = slower.

### Issue: Text overflowing container
**Solution:** Enable Auto Wrap on text blocks. Increase container height if needed. Limit objective text length.

### Issue: Hint text not showing
**Solution:** Check that TextBlock_HintText visibility is set to Visible in BP_ShowToast when HintText is not empty.

### Issue: Arrow not blinking
**Solution:** Verify ArrowBlink animation is set to Loop. Check that Play Animation is called with Loop=True.

### Issue: Slide animation jerky
**Solution:** Use Ease Out Bounce curve for SlideIn. Ensure animation length matches SlideInDuration property.

---

## Performance Optimization

### Best Practices
1. Use Collapsed visibility when hidden
2. Stop all animations when widget is hidden
3. Cache string conversions in typewriter effect
4. Limit typewriter sound frequency (every 3rd character)
5. Use simple materials for background
6. Disable tick when not animating

### Typewriter Optimization
```
// Instead of delay in loop, use timer
PlayTypewriterOnTextBlock
├── Set Text (Empty)
├── Store FullText in variable
├── Set CurrentCharIndex = 0
└── Set Timer by Function Name ("TypewriterTick", CharacterDelay, Loop)

TypewriterTick
├── Increment CurrentCharIndex
├── Get Substring (0 to CurrentCharIndex)
├── Set Text
├── Branch (CurrentCharIndex >= Length)
│   └── True: Clear Timer
```

---

## Integration with Game

### Triggering from C++
```cpp
// In objective system
UUIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UUIManagerSubsystem>();
UObjectiveToastWidget* Toast = Cast<UObjectiveToastWidget>(
    UIManager->GetWidget("ObjectiveToast")
);
if (Toast)
{
    FGameplayTag EventTag = FGameplayTag::RequestGameplayTag("Objective.Main");
    Toast->ShowObjectiveToast(EventTag, FText::FromString("Find the basement key"));
}
```

### Triggering from Blueprint
1. Get Game Instance → Get Subsystem (UIManagerSubsystem)
2. Get Widget ("ObjectiveToast")
3. Cast to ObjectiveToastWidget
4. Call ShowObjectiveToast or ShowObjectiveWithHint

### EventBus Integration
```
Event Construct
├── Get UI Event Manager
├── Subscribe to Event (Tag: "Objective.Updated")
└── Bind to OnObjectiveUpdated

OnObjectiveUpdated (EventData)
├── Extract Objective Text from EventData
├── Extract Hint Text (optional)
└── Call ShowObjectiveToast
```

---

## Objective Queue System

### Purpose
Handle multiple objective updates in quick succession.

### Implementation
1. Create TArray variable: `ObjectiveQueue`
2. Create struct: `FObjectiveData` (EventTag, ObjectiveText, HintText)
3. Modify ShowObjectiveToast:
   - If currently visible, add to queue
   - If not visible, show immediately
4. Modify BP_DismissToast:
   - After hiding, check queue
   - If queue has items, show next objective

### Blueprint Logic
```
ShowObjectiveToast
├── Branch (IsVisible)
│   ├── True: Add to ObjectiveQueue
│   └── False: Call BP_ShowToast

BP_DismissToast (modified)
├── Stop Animations
├── Play SlideOut
├── Delay (SlideOutDuration)
├── Set Visibility (Collapsed)
├── Branch (ObjectiveQueue.Length > 0)
│   ├── True:
│   │   ├── Get First Element
│   │   ├── Remove from Queue
│   │   ├── Delay (0.5)
│   │   └── Call BP_ShowToast
│   └── False: Do nothing
```

---

## Asset Requirements

### Fonts
- F_HomeVideo (VHS-style monospace font)

### Materials (Optional)
- M_ScanlineSubtle (subtle scanline effect for background)

### Sounds
- SFX_ObjectiveUpdate (notification sound)
- SFX_UISlideIn (slide-in whoosh)
- SFX_TypewriterClick (typewriter character sound)
- SFX_UISlideOut (slide-out whoosh)

### Colors
- Primary: Cyan (0.0, 0.8, 1.0, 1.0)
- Background: Dark Blue (0.05, 0.1, 0.2, 0.9)
- Text Primary: White (1.0, 1.0, 1.0, 1.0)
- Text Secondary: Light Cyan (0.6, 0.9, 1.0, 0.8)
- Optional: Yellow (1.0, 0.8, 0.0, 1.0)
- Hidden: Purple (0.8, 0.2, 1.0, 1.0)
- Failed: Red (1.0, 0.2, 0.2, 1.0)
