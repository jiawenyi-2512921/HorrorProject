# WBP_PauseMenu Blueprint Implementation Guide

## Overview
Pause menu with resume, settings, and quit options. Simple, clean design with VHS aesthetic.

## C++ Base Class
**Parent Class:** `UPauseMenuWidget` (inherits from `UUserWidget`)

### Available C++ Functions
```cpp
// Menu Control
void OpenPauseMenu()
void ClosePauseMenu()
void ResumeGame()
void OpenSettings()
void QuitToMainMenu()

// State Query
bool IsOpen() const
```

### Blueprint Implementable Events
```cpp
BP_PauseMenuOpened()
BP_PauseMenuClosed()
BP_SettingsOpened()
```

### Exposed Properties
- `bIsOpen` (bool, ReadOnly)
- `FadeInDuration` (float, EditDefaultsOnly) - Default: 0.2
- `FadeOutDuration` (float, EditDefaultsOnly) - Default: 0.2

---

## Canvas Layout Design

### Screen Layout (1920x1080 reference)
```
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│                                                             │
│                      ┌─────────────┐                        │
│                      │   PAUSED    │                        │
│                      └─────────────┘                        │
│                                                             │
│                      [ RESUME ]                             │
│                      [ SETTINGS ]                           │
│                      [ QUIT TO MENU ]                       │
│                                                             │
│                                                             │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Widget Hierarchy
```
Canvas Panel (Root)
└── Border_PauseContainer (Full Screen)
    ├── Image_BackgroundBlur
    ├── VerticalBox_MenuContent (Center)
    │   ├── TextBlock_Title
    │   ├── Spacer (40px)
    │   ├── Button_Resume
    │   │   └── TextBlock "RESUME"
    │   ├── Spacer (16px)
    │   ├── Button_Settings
    │   │   └── TextBlock "SETTINGS"
    │   ├── Spacer (16px)
    │   └── Button_Quit
    │       └── TextBlock "QUIT TO MENU"
    └── Image_ScanlineOverlay
```

---

## Step-by-Step Implementation

### Step 1: Create Widget Blueprint
1. Content Browser → Right-click → User Interface → Widget Blueprint
2. Name: `WBP_PauseMenu`
3. Open the blueprint
4. Class Settings → Parent Class → Search "PauseMenuWidget"
5. Compile and Save

### Step 2: Canvas Root Setup
1. Designer tab → Hierarchy panel
2. Root should be Canvas Panel (default)
3. Canvas Panel settings:
   - Is Variable: False
   - Visibility: Collapsed (initially hidden)

### Step 3: Pause Container

#### Border_PauseContainer
1. Add Border widget to Canvas Panel
2. Rename to `Border_PauseContainer`
3. Anchors: Fill screen (0,0 to 1,1)
4. Position: X=0, Y=0
5. Size: X=0, Y=0
6. Z-Order: 2000
7. Is Variable: True
8. Appearance:
   - Brush Color: Black (0, 0, 0, 0.8)
   - Brush → Draw As: Box
9. Padding: 0 (all sides)

### Step 4: Background Blur

#### Image_BackgroundBlur
1. Add Image widget to Border_PauseContainer (first child)
2. Rename to `Image_BackgroundBlur`
3. Anchors: Fill (0,0 to 1,1)
4. Position: X=0, Y=0
5. Size: X=0, Y=0
6. Z-Order: 0
7. Material: M_BackgroundBlur (optional)
8. Render Opacity: 0.5
9. Hit Test Invisible: True

### Step 5: Menu Content

#### VerticalBox_MenuContent
1. Add Vertical Box to Border_PauseContainer
2. Rename to `VerticalBox_MenuContent`
3. Anchors: Center (0.5, 0.5 to 0.5, 0.5)
4. Position: X=0, Y=0
5. Size: X=400, Y=400
6. Alignment: X=0.5, Y=0.5 (center)
7. Z-Order: 10

### Step 6: Title

#### TextBlock_Title
1. Add Text Block to VerticalBox_MenuContent
2. Rename to `TextBlock_Title`
3. Text: "PAUSED"
4. Font: F_HomeVideo
5. Size: 48
6. Color: Cyan (0, 0.8, 1, 1)
7. Font → Typeface: Bold
8. Justification: Center
9. Is Variable: False

#### Spacer (after title)
- Size: Height=40

### Step 7: Resume Button

#### Button_Resume
1. Add Button to VerticalBox_MenuContent
2. Rename to `Button_Resume`
3. Size: Fill Horizontal, Fixed Height: 60
4. Is Variable: True
5. Style:
   - Normal:
     - Background Color: Dark Gray (0.15, 0.15, 0.15, 1)
     - Outline Width: 2
     - Outline Color: Cyan (0, 0.8, 1, 0.5)
   - Hovered:
     - Background Color: Gray (0.25, 0.25, 0.25, 1)
     - Outline Color: Cyan (0, 0.8, 1, 1)
   - Pressed:
     - Background Color: Cyan (0, 0.8, 1, 0.3)
6. On Clicked: Bind to OnResumeClicked

**TextBlock (Button Content):**
- Text: "RESUME"
- Font: F_HomeVideo
- Size: 24
- Color: White (1, 1, 1, 1)
- Justification: Center

#### Spacer (after Resume)
- Size: Height=16

### Step 8: Settings Button

#### Button_Settings
1. Add Button to VerticalBox_MenuContent
2. Rename to `Button_Settings`
3. Size: Fill Horizontal, Fixed Height: 60
4. Is Variable: True
5. Style: Same as Button_Resume
6. On Clicked: Bind to OnSettingsClicked

**TextBlock (Button Content):**
- Text: "SETTINGS"
- Font: F_HomeVideo
- Size: 24
- Color: White (1, 1, 1, 1)
- Justification: Center

#### Spacer (after Settings)
- Size: Height=16

### Step 9: Quit Button

#### Button_Quit
1. Add Button to VerticalBox_MenuContent
2. Rename to `Button_Quit`
3. Size: Fill Horizontal, Fixed Height: 60
4. Is Variable: True
5. Style: Same as Button_Resume, but:
   - Outline Color: Red (1, 0.2, 0.2, 0.5)
   - Hovered Outline: Red (1, 0.2, 0.2, 1)
6. On Clicked: Bind to OnQuitClicked

**TextBlock (Button Content):**
- Text: "QUIT TO MENU"
- Font: F_HomeVideo
- Size: 24
- Color: Light Red (1, 0.6, 0.6, 1)
- Justification: Center

### Step 10: Scanline Overlay

#### Image_ScanlineOverlay
1. Add Image widget to Border_PauseContainer (last child)
2. Rename to `Image_ScanlineOverlay`
3. Anchors: Fill (0,0 to 1,1)
4. Position: X=0, Y=0
5. Size: X=0, Y=0
6. Z-Order: 100
7. Material: M_Scanlines
8. Render Opacity: 0.1
9. Hit Test Invisible: True

---

## Animation Setup

### Animation 1: FadeIn
**Purpose:** Fade in menu

1. Animations tab → + Animation → Name: "FadeIn"
2. Length: 0.2 seconds
3. Loop: False
4. Tracks:
   - Border_PauseContainer → Render Opacity
     - 0.0s: 0.0
     - 0.2s: 1.0 (Linear curve)
   - VerticalBox_MenuContent → Render Transform → Translation Y
     - 0.0s: -20.0
     - 0.2s: 0.0 (Ease Out curve)

### Animation 2: FadeOut
**Purpose:** Fade out menu

1. Animations tab → + Animation → Name: "FadeOut"
2. Length: 0.2 seconds
3. Loop: False
4. Tracks:
   - Border_PauseContainer → Render Opacity
     - 0.0s: 1.0
     - 0.2s: 0.0 (Linear curve)

### Animation 3: ButtonHoverPulse
**Purpose:** Pulse button on hover

1. Animations tab → + Animation → Name: "ButtonHoverPulse"
2. Length: 0.3 seconds
3. Loop: False
4. Tracks:
   - [Target Button] → Render Transform → Scale
     - 0.0s: (1.0, 1.0)
     - 0.15s: (1.05, 1.05)
     - 0.3s: (1.0, 1.0)

---

## Event Graph Implementation

### Event Construct
```
Event Construct
├── Set Visibility (Collapsed)
├── Set Render Opacity (0.0)
└── Initialize Input Handling
```

**Blueprint Nodes:**
1. Event Construct
2. Set Visibility (Target: Self, Visibility: Collapsed)
3. Set Render Opacity (Target: Border_PauseContainer, Opacity: 0.0)

### BP_PauseMenuOpened Implementation
```
BP_PauseMenuOpened
├── Set Visibility (Visible)
├── Play Animation (FadeIn)
├── Set Game Paused (True)
├── Set Input Mode (UI Only)
├── Show Mouse Cursor (True)
├── Set Focus to Button_Resume
└── Play Sound 2D (SFX_PauseOpen)
```

**Blueprint Nodes:**
1. Event BP_PauseMenuOpened
2. Set Visibility (Target: Self, Visibility: Visible)
3. Play Animation (FadeIn)
4. Set Game Paused (True)
5. Get Player Controller → Set Input Mode UI Only
6. Get Player Controller → Set Show Mouse Cursor (True)
7. Button_Resume → Set User Focus
8. Play Sound 2D (SFX_PauseOpen)

### BP_PauseMenuClosed Implementation
```
BP_PauseMenuClosed
├── Play Animation (FadeOut)
├── Delay (FadeOutDuration)
├── Set Visibility (Collapsed)
├── Set Game Paused (False)
├── Set Input Mode (Game Only)
├── Show Mouse Cursor (False)
└── Play Sound 2D (SFX_PauseClose)
```

**Blueprint Nodes:**
1. Event BP_PauseMenuClosed
2. Play Animation (FadeOut)
3. Delay (Duration: FadeOutDuration)
4. Set Visibility (Target: Self, Visibility: Collapsed)
5. Set Game Paused (False)
6. Get Player Controller → Set Input Mode Game Only
7. Get Player Controller → Set Show Mouse Cursor (False)
8. Play Sound 2D (SFX_PauseClose)

### BP_SettingsOpened Implementation
```
BP_SettingsOpened
├── Hide Pause Menu (keep paused)
├── Get UI Manager
├── Show Widget ("SettingsMenu")
└── Bind to Settings Closed Event
```

**Blueprint Nodes:**
1. Event BP_SettingsOpened
2. Set Visibility (Target: Self, Visibility: Collapsed)
3. Get Game Instance → Get Subsystem (UIManagerSubsystem)
4. Show Widget ("SettingsMenu")
5. Bind to Settings Menu Close Event → OnSettingsClosed

### Button Click Events

#### OnResumeClicked
```
OnResumeClicked
├── Play Sound 2D (SFX_ButtonClick)
└── Call ResumeGame
```

**Blueprint Nodes:**
1. Button_Resume → On Clicked
2. Play Sound 2D (SFX_ButtonClick)
3. Call ResumeGame (C++ function)

#### OnSettingsClicked
```
OnSettingsClicked
├── Play Sound 2D (SFX_ButtonClick)
└── Call OpenSettings
```

**Blueprint Nodes:**
1. Button_Settings → On Clicked
2. Play Sound 2D (SFX_ButtonClick)
3. Call OpenSettings (C++ function)

#### OnQuitClicked
```
OnQuitClicked
├── Play Sound 2D (SFX_ButtonClick)
├── Show Confirmation Dialog
└── Branch (Confirmed)
    ├── True: Call QuitToMainMenu
    └── False: Do nothing
```

**Blueprint Nodes:**
1. Button_Quit → On Clicked
2. Play Sound 2D (SFX_ButtonClick)
3. Create Widget (WBP_ConfirmationDialog)
4. Set Dialog Text ("Are you sure you want to quit?")
5. Add to Viewport
6. Bind to Dialog Result
7. Branch (Result == Confirmed)
8. True: Call QuitToMainMenu (C++ function)

### Button Hover Events

#### OnResumeHovered
```
OnResumeHovered
├── Play Sound 2D (SFX_ButtonHover)
└── Play Animation (ButtonHoverPulse) on Button_Resume
```

#### OnResumeUnhovered
```
OnResumeUnhovered
└── Stop Animation (ButtonHoverPulse)
```

**Repeat for Button_Settings and Button_Quit**

### Custom Event: OnSettingsClosed
```
OnSettingsClosed
├── Set Visibility (Visible)
└── Set Focus to Button_Resume
```

---

## Input Handling

### Keyboard Shortcuts

#### Event Graph
```
Event Construct
├── Get Player Controller
└── Enable Input (Priority: 2)

Override: OnKeyDown
├── Switch on Key
│   ├── Escape:
│   │   └── Call ResumeGame
│   ├── Up Arrow:
│   │   └── Focus Previous Button
│   ├── Down Arrow:
│   │   └── Focus Next Button
│   └── Enter:
│       └── Click Focused Button
```

**Blueprint Nodes:**
1. Override function: OnKeyDown
2. Switch on Key
3. Escape case: Call ResumeGame
4. Up Arrow case: Navigate to previous button
5. Down Arrow case: Navigate to next button
6. Enter case: Trigger click on focused button

### Gamepad Support
```
Override: OnAnalogValueChanged
├── Left Stick Y / D-Pad:
│   └── Navigate Buttons
└── A Button (Xbox) / X Button (PS):
    └── Click Focused Button

Override: OnKeyDown (Gamepad)
├── B Button (Xbox) / Circle (PS):
│   └── Call ResumeGame
└── Start Button:
    └── Call ResumeGame
```

---

## Advanced Features

### Confirmation Dialog (WBP_ConfirmationDialog)

#### Purpose
Confirm quit action to prevent accidental exits.

#### Widget Hierarchy
```
Border_DialogContainer (400x200)
├── VerticalBox_Content
│   ├── TextBlock_Message
│   ├── Spacer (20px)
│   └── HorizontalBox_Buttons
│       ├── Button_Confirm
│       └── Button_Cancel
```

#### Implementation
1. Create new Widget Blueprint: WBP_ConfirmationDialog
2. Add Border (400x200, centered)
3. Style: Dark background with bright outline
4. Add message text block
5. Add Confirm and Cancel buttons
6. Create Event Dispatcher: OnDialogResult(bool Confirmed)
7. Bind button clicks to dispatcher

### Game State Display (Optional)

#### Add to Menu
```
VerticalBox_MenuContent
├── TextBlock_Title
├── Spacer (20px)
├── TextBlock_GameInfo  ← NEW
│   └── "Chapter 2 - The Basement"
│   └── "Playtime: 1:23:45"
├── Spacer (20px)
├── ... (buttons)
```

#### Implementation
```
BP_PauseMenuOpened (modified)
├── ... (existing code)
├── Get Game State
├── Format Text ("Chapter {0} - {1}", Chapter, Location)
├── Set Text (TextBlock_GameInfo)
├── Get Playtime
├── Format Time ("Playtime: {0:00}:{1:00}:{2:00}")
└── Append to TextBlock_GameInfo
```

### Quick Save/Load (Optional)

#### Add Buttons
```
VerticalBox_MenuContent
├── ... (existing buttons)
├── Spacer (32px)
├── HorizontalBox_SaveLoad
│   ├── Button_QuickSave
│   └── Button_QuickLoad
```

#### Implementation
```
OnQuickSaveClicked
├── Get Save Game Subsystem
├── Quick Save
├── Show Toast ("Game Saved")
└── Play Sound (SFX_SaveGame)

OnQuickLoadClicked
├── Show Confirmation Dialog
├── Branch (Confirmed)
│   ├── True:
│   │   ├── Get Save Game Subsystem
│   │   ├── Quick Load
│   │   └── Close Pause Menu
│   └── False: Do nothing
```

---

## Testing Checklist

### Visual Tests
- [ ] Menu fills entire screen
- [ ] Background darkens game view
- [ ] Title displays correctly
- [ ] All buttons visible and aligned
- [ ] Scanline overlay subtle
- [ ] Button hover states work

### Animation Tests
- [ ] FadeIn animation smooth (0.2s)
- [ ] FadeOut animation smooth (0.2s)
- [ ] Menu slides down slightly on open
- [ ] Button hover pulse works
- [ ] No animation stuttering

### Functional Tests
- [ ] OpenPauseMenu shows menu
- [ ] ClosePauseMenu hides menu
- [ ] ResumeGame unpauses and closes
- [ ] OpenSettings opens settings menu
- [ ] QuitToMainMenu returns to main menu
- [ ] Game pauses when menu open
- [ ] Mouse cursor shows when open

### Input Tests
- [ ] ESC key opens/closes menu
- [ ] ESC key resumes game when open
- [ ] Up/Down arrows navigate buttons
- [ ] Enter key activates focused button
- [ ] Mouse clicks work on all buttons
- [ ] Gamepad navigation works
- [ ] Gamepad B button resumes

### Button Tests
- [ ] Resume button closes menu
- [ ] Settings button opens settings
- [ ] Quit button shows confirmation
- [ ] Confirmation dialog works
- [ ] Button hover sounds play
- [ ] Button click sounds play

### State Tests
- [ ] Game pauses when menu opens
- [ ] Game resumes when menu closes
- [ ] Input mode switches correctly
- [ ] Mouse cursor visibility correct
- [ ] Focus set to Resume on open
- [ ] Settings menu returns to pause menu

### Performance Tests
- [ ] Menu opens instantly
- [ ] No frame drops when opening
- [ ] No memory leaks from repeated open/close
- [ ] Background blur efficient (if used)

---

## Common Issues & Solutions

### Issue: Menu not pausing game
**Solution:** Check that Set Game Paused (True) is called in BP_PauseMenuOpened. Verify game mode allows pausing.

### Issue: Can't resume with ESC key
**Solution:** Verify OnKeyDown override is implemented. Check that input priority is high enough (2+). Ensure Enable Input is called.

### Issue: Mouse cursor not showing
**Solution:** Check that Set Show Mouse Cursor (True) is called in BP_PauseMenuOpened. Verify Set Input Mode UI Only is called.

### Issue: Buttons not clickable
**Solution:** Verify button Is Enabled is True. Check that Border_PauseContainer is not blocking input. Ensure Z-Order is correct.

### Issue: Settings menu doesn't return to pause menu
**Solution:** Bind to Settings Menu Close event. Implement OnSettingsClosed to show pause menu again.

### Issue: Quit confirmation not showing
**Solution:** Verify WBP_ConfirmationDialog widget exists. Check that Add to Viewport is called. Ensure dialog Z-Order is higher than pause menu.

### Issue: Background too dark/light
**Solution:** Adjust Border_PauseContainer brush color alpha. Recommended: (0, 0, 0, 0.8). Test in different lighting conditions.

---

## Performance Optimization

### Best Practices
1. Use Collapsed visibility when hidden
2. Stop animations when menu closes
3. Cache player controller reference
4. Use simple materials for background
5. Disable tick if not needed
6. Minimize widget complexity

### Recommended Settings
- Update Mode: Tick (only if animating)
- Tick Frequency: Every Frame (only if needed)
- Invalidation: Manual
- Volatility: Not Volatile

---

## Integration with Game

### Opening from Player Input
```cpp
// In player controller or input component
void AHorrorPlayerController::OnPausePressed()
{
    UUIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UUIManagerSubsystem>();
    UPauseMenuWidget* PauseMenu = Cast<UPauseMenuWidget>(
        UIManager->GetWidget("PauseMenu")
    );
    if (PauseMenu)
    {
        if (PauseMenu->IsOpen())
        {
            PauseMenu->ClosePauseMenu();
        }
        else
        {
            PauseMenu->OpenPauseMenu();
        }
    }
}
```

### Blueprint Integration
```
Input Action "Pause" (ESC key)
├── Get Game Instance
├── Get Subsystem (UIManagerSubsystem)
├── Get Widget ("PauseMenu")
├── Cast to PauseMenuWidget
├── Branch (IsOpen)
│   ├── True: Call ClosePauseMenu
│   └── False: Call OpenPauseMenu
```

### Creating at Game Start
```
Game Mode: BeginPlay
├── Get Game Instance
├── Get Subsystem (UIManagerSubsystem)
├── Create Widget (WBP_PauseMenu, "PauseMenu")
└── Add to Viewport (Hidden)
```

---

## Asset Requirements

### Fonts
- F_HomeVideo (VHS-style monospace font)

### Materials (Optional)
- M_BackgroundBlur (blur effect for background)
- M_Scanlines (scanline overlay)

### Sounds
- SFX_PauseOpen (menu open sound)
- SFX_PauseClose (menu close sound)
- SFX_ButtonClick (button click sound)
- SFX_ButtonHover (button hover sound)

### Colors
- Primary: Cyan (0.0, 0.8, 1.0, 1.0)
- Background: Black (0.0, 0.0, 0.0, 0.8)
- Button Normal: Dark Gray (0.15, 0.15, 0.15, 1.0)
- Button Hovered: Gray (0.25, 0.25, 0.25, 1.0)
- Button Pressed: Cyan Tint (0.0, 0.8, 1.0, 0.3)
- Text Primary: White (1.0, 1.0, 1.0, 1.0)
- Quit Button: Light Red (1.0, 0.6, 0.6, 1.0)
- Outline: Cyan (0.0, 0.8, 1.0, 0.5)
- Outline Hovered: Cyan (0.0, 0.8, 1.0, 1.0)

---

## Accessibility Considerations

### Focus Management
- Set focus to Resume button on open
- Support keyboard navigation (Tab, Arrow keys)
- Visual focus indicator on buttons
- Announce menu state to screen readers

### Color Contrast
- Ensure text has sufficient contrast (4.5:1 minimum)
- Don't rely solely on color for information
- Provide alternative visual cues (outlines, icons)

### Input Flexibility
- Support keyboard, mouse, and gamepad
- Allow remapping of pause key
- Provide multiple ways to resume (ESC, button, gamepad)

### Text Scaling
- Support dynamic text scaling
- Test at different UI scales (0.5x to 2x)
- Ensure buttons don't overflow at large scales
