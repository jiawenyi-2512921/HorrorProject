# WBP_SettingsMenu Blueprint Implementation Guide

## Overview
Comprehensive settings menu with graphics, audio, controls, gameplay, and accessibility options.

## C++ Base Class
**Parent Class:** `USettingsMenuWidget` (inherits from `UUserWidget`)

### Available C++ Functions
```cpp
// Menu Control
void OpenSettings()
void CloseSettings()
void ApplySettings()
void ResetToDefaults()
void SelectCategory(ESettingsCategory Category)

// State Query
bool IsOpen() const
bool HasUnsavedChanges() const
```

### Blueprint Implementable Events
```cpp
BP_SettingsOpened()
BP_SettingsClosed()
BP_SettingsApplied()
BP_CategorySelected(ESettingsCategory Category)
```

### Exposed Properties
- `bIsOpen` (bool, ReadOnly)
- `bHasUnsavedChanges` (bool, ReadOnly)
- `CurrentCategory` (ESettingsCategory, ReadOnly)

### Enums
```cpp
ESettingsCategory:
- Graphics
- Audio
- Controls
- Gameplay
- Accessibility
```

---

## Canvas Layout Design

### Screen Layout (1920x1080 reference)
```
┌─────────────────────────────────────────────────────────────┐
│ [X] SETTINGS                                                │ ← Header
├─────────────┬───────────────────────────────────────────────┤
│             │                                               │
│  GRAPHICS   │  Graphics Settings                            │
│  AUDIO      │                                               │
│  CONTROLS   │  Resolution: [1920x1080 ▼]                    │
│  GAMEPLAY   │  Window Mode: [Fullscreen ▼]                  │
│  ACCESS.    │  Quality: [High ▼]                            │
│             │  VSync: [✓]                                   │
│             │  Frame Limit: [60 ▼]                          │
│             │                                               │
│             │  VHS Effects: [████████░░] 80%                │
│             │  Brightness: [█████░░░░░] 50%                 │
│             │                                               │
├─────────────┴───────────────────────────────────────────────┤
│                    [APPLY]  [RESET]  [CLOSE]                │ ← Footer
└─────────────────────────────────────────────────────────────┘
```

### Widget Hierarchy
```
Canvas Panel (Root)
└── Border_SettingsContainer (Full Screen)
    ├── Image_Background
    ├── VerticalBox_Main
    │   ├── Border_Header (Height: 80)
    │   │   └── HorizontalBox_HeaderContent
    │   │       ├── Button_Close
    │   │       ├── TextBlock_Title
    │   │       └── TextBlock_UnsavedIndicator
    │   ├── HorizontalBox_Content (Fill)
    │   │   ├── Border_CategoryPanel (Width: 200)
    │   │   │   └── VerticalBox_Categories
    │   │   │       ├── Button_Graphics
    │   │   │       ├── Button_Audio
    │   │   │       ├── Button_Controls
    │   │   │       ├── Button_Gameplay
    │   │   │       └── Button_Accessibility
    │   │   └── Border_SettingsPanel (Fill)
    │   │       └── WidgetSwitcher_SettingsPages
    │   │           ├── ScrollBox_GraphicsSettings
    │   │           ├── ScrollBox_AudioSettings
    │   │           ├── ScrollBox_ControlsSettings
    │   │           ├── ScrollBox_GameplaySettings
    │   │           └── ScrollBox_AccessibilitySettings
    │   └── Border_Footer (Height: 80)
    │       └── HorizontalBox_FooterButtons
    │           ├── Button_Apply
    │           ├── Button_Reset
    │           └── Button_Close
    └── Image_ScanlineOverlay
```

---

## Step-by-Step Implementation

### Step 1: Create Widget Blueprint
1. Content Browser → Right-click → User Interface → Widget Blueprint
2. Name: `WBP_SettingsMenu`
3. Open the blueprint
4. Class Settings → Parent Class → Search "SettingsMenuWidget"
5. Compile and Save

### Step 2: Canvas Root Setup
1. Designer tab → Hierarchy panel
2. Root should be Canvas Panel (default)
3. Canvas Panel settings:
   - Is Variable: False
   - Visibility: Collapsed (initially hidden)

### Step 3: Settings Container

#### Border_SettingsContainer
1. Add Border widget to Canvas Panel
2. Rename to `Border_SettingsContainer`
3. Anchors: Fill screen (0,0 to 1,1)
4. Position: X=0, Y=0
5. Size: X=0, Y=0
6. Z-Order: 2500
7. Is Variable: True
8. Appearance:
   - Brush Color: Black (0, 0, 0, 0.95)
   - Brush → Draw As: Box
9. Padding: 0

### Step 4: Background

#### Image_Background
1. Add Image widget to Border_SettingsContainer
2. Rename to `Image_Background`
3. Anchors: Fill (0,0 to 1,1)
4. Material: M_FilmGrain
5. Render Opacity: 0.08
6. Hit Test Invisible: True

### Step 5: Main Layout

#### VerticalBox_Main
1. Add Vertical Box to Border_SettingsContainer
2. Rename to `VerticalBox_Main`
3. Anchors: Fill (0,0 to 1,1)

### Step 6: Header

#### Border_Header
1. Add Border to VerticalBox_Main
2. Rename to `Border_Header`
3. Size: Fill Horizontal, Fixed Height: 80
4. Appearance:
   - Brush Color: Dark Gray (0.05, 0.05, 0.05, 1)
   - Outline: Bottom only, 2px, Cyan (0, 0.8, 1, 0.5)
5. Padding: 20

#### HorizontalBox_HeaderContent
**Button_Close:** 40x40, "X" text
**TextBlock_Title:** "SETTINGS", Size 32, Cyan
**Spacer:** Fill
**TextBlock_UnsavedIndicator:** "*Unsaved Changes", Size 16, Yellow, Visibility: Collapsed

### Step 7: Content Section

#### HorizontalBox_Content
1. Add Horizontal Box to VerticalBox_Main
2. Fill Vertical

### Step 8: Category Panel

#### Border_CategoryPanel
1. Add Border to HorizontalBox_Content
2. Size: Fixed Width 200, Fill Vertical
3. Appearance:
   - Brush Color: Dark Gray (0.08, 0.08, 0.08, 1)
   - Outline: Right only, 2px, Cyan (0, 0.8, 1, 0.3)
4. Padding: 10

#### VerticalBox_Categories
1. Add Vertical Box to Border_CategoryPanel
2. Spacing: 8

**Button_Graphics:**
- Size: Fill Horizontal, Fixed Height: 50
- Text: "GRAPHICS"
- Style: Normal (0.15, 0.15, 0.15), Active (0, 0.8, 1, 0.3)
- Is Variable: True
- On Clicked: Bind to OnGraphicsClicked

**Button_Audio:**
- Size: Fill Horizontal, Fixed Height: 50
- Text: "AUDIO"
- Is Variable: True
- On Clicked: Bind to OnAudioClicked

**Button_Controls:**
- Size: Fill Horizontal, Fixed Height: 50
- Text: "CONTROLS"
- Is Variable: True
- On Clicked: Bind to OnControlsClicked

**Button_Gameplay:**
- Size: Fill Horizontal, Fixed Height: 50
- Text: "GAMEPLAY"
- Is Variable: True
- On Clicked: Bind to OnGameplayClicked

**Button_Accessibility:**
- Size: Fill Horizontal, Fixed Height: 50
- Text: "ACCESSIBILITY"
- Is Variable: True
- On Clicked: Bind to OnAccessibilityClicked

### Step 9: Settings Panel

#### Border_SettingsPanel
1. Add Border to HorizontalBox_Content
2. Size: Fill Horizontal, Fill Vertical
3. Appearance:
   - Brush Color: Very Dark Gray (0.05, 0.05, 0.05, 1)
4. Padding: 40

#### WidgetSwitcher_SettingsPages
1. Add Widget Switcher to Border_SettingsPanel
2. Rename to `WidgetSwitcher_SettingsPages`
3. Anchors: Fill
4. Is Variable: True
5. Active Widget Index: 0

### Step 10: Graphics Settings Page

#### ScrollBox_GraphicsSettings
1. Add Scroll Box to WidgetSwitcher_SettingsPages (Index 0)
2. Rename to `ScrollBox_GraphicsSettings`
3. Add Vertical Box as child

**Settings Items:**

**ComboBox_Resolution:**
- Label: "Resolution"
- Options: 1920x1080, 2560x1440, 3840x2160
- Is Variable: True
- On Selection Changed: Mark as unsaved

**ComboBox_WindowMode:**
- Label: "Window Mode"
- Options: Fullscreen, Windowed, Borderless
- Is Variable: True

**ComboBox_Quality:**
- Label: "Graphics Quality"
- Options: Low, Medium, High, Ultra
- Is Variable: True

**CheckBox_VSync:**
- Label: "VSync"
- Is Variable: True
- On Check State Changed: Mark as unsaved

**ComboBox_FrameLimit:**
- Label: "Frame Rate Limit"
- Options: 30, 60, 120, 144, Unlimited
- Is Variable: True

**Slider_VHSIntensity:**
- Label: "VHS Effect Intensity"
- Min: 0.0, Max: 1.0, Step: 0.1
- Is Variable: True
- On Value Changed: Mark as unsaved

**Slider_Brightness:**
- Label: "Brightness"
- Min: 0.0, Max: 1.0, Step: 0.05
- Is Variable: True

### Step 11: Audio Settings Page

#### ScrollBox_AudioSettings
1. Add Scroll Box to WidgetSwitcher_SettingsPages (Index 1)
2. Rename to `ScrollBox_AudioSettings`

**Settings Items:**

**Slider_MasterVolume:**
- Label: "Master Volume"
- Min: 0.0, Max: 1.0, Step: 0.01
- Is Variable: True

**Slider_MusicVolume:**
- Label: "Music Volume"
- Min: 0.0, Max: 1.0, Step: 0.01
- Is Variable: True

**Slider_SFXVolume:**
- Label: "Sound Effects Volume"
- Min: 0.0, Max: 1.0, Step: 0.01
- Is Variable: True

**Slider_AmbienceVolume:**
- Label: "Ambience Volume"
- Min: 0.0, Max: 1.0, Step: 0.01
- Is Variable: True

**CheckBox_Subtitles:**
- Label: "Enable Subtitles"
- Is Variable: True

**ComboBox_SubtitleSize:**
- Label: "Subtitle Size"
- Options: Small, Medium, Large
- Is Variable: True

### Step 12: Controls Settings Page

#### ScrollBox_ControlsSettings
1. Add Scroll Box to WidgetSwitcher_SettingsPages (Index 2)
2. Rename to `ScrollBox_ControlsSettings`

**Settings Items:**

**Slider_MouseSensitivity:**
- Label: "Mouse Sensitivity"
- Min: 0.1, Max: 2.0, Step: 0.1
- Is Variable: True

**CheckBox_InvertY:**
- Label: "Invert Y Axis"
- Is Variable: True

**CheckBox_InvertX:**
- Label: "Invert X Axis"
- Is Variable: True

**Slider_GamepadSensitivity:**
- Label: "Gamepad Sensitivity"
- Min: 0.1, Max: 2.0, Step: 0.1
- Is Variable: True

**Key Binding List:**
- Forward, Backward, Left, Right
- Jump, Crouch, Sprint
- Interact, Flashlight, Camera
- Pause, Inventory, Map

### Step 13: Gameplay Settings Page

#### ScrollBox_GameplaySettings
1. Add Scroll Box to WidgetSwitcher_SettingsPages (Index 3)
2. Rename to `ScrollBox_GameplaySettings`

**Settings Items:**

**ComboBox_Difficulty:**
- Label: "Difficulty"
- Options: Easy, Normal, Hard, Nightmare
- Is Variable: True

**CheckBox_AutoSave:**
- Label: "Auto Save"
- Is Variable: True

**Slider_AutoSaveInterval:**
- Label: "Auto Save Interval (minutes)"
- Min: 1, Max: 30, Step: 1
- Is Variable: True

**CheckBox_Hints:**
- Label: "Show Hints"
- Is Variable: True

**CheckBox_Crosshair:**
- Label: "Show Crosshair"
- Is Variable: True

### Step 14: Accessibility Settings Page

#### ScrollBox_AccessibilitySettings
1. Add Scroll Box to WidgetSwitcher_SettingsPages (Index 4)
2. Rename to `ScrollBox_AccessibilitySettings`

**Settings Items:**

**CheckBox_ColorblindMode:**
- Label: "Colorblind Mode"
- Is Variable: True

**ComboBox_ColorblindType:**
- Label: "Colorblind Type"
- Options: Protanopia, Deuteranopia, Tritanopia
- Is Variable: True

**Slider_UIScale:**
- Label: "UI Scale"
- Min: 0.5, Max: 2.0, Step: 0.1
- Is Variable: True

**CheckBox_ReduceMotion:**
- Label: "Reduce Motion"
- Is Variable: True

**CheckBox_HighContrast:**
- Label: "High Contrast Mode"
- Is Variable: True

**Slider_TextSize:**
- Label: "Text Size"
- Min: 0.8, Max: 1.5, Step: 0.1
- Is Variable: True

### Step 15: Footer

#### Border_Footer
1. Add Border to VerticalBox_Main
2. Size: Fill Horizontal, Fixed Height: 80
3. Appearance:
   - Brush Color: Dark Gray (0.05, 0.05, 0.05, 1)
   - Outline: Top only, 2px, Cyan (0, 0.8, 1, 0.5)
4. Padding: 20

#### HorizontalBox_FooterButtons
1. Add Horizontal Box to Border_Footer
2. Alignment: Center
3. Spacing: 16

**Button_Apply:**
- Size: 150x50
- Text: "APPLY"
- Style: Cyan highlight when has unsaved changes
- Is Variable: True
- Is Enabled: Bind to HasUnsavedChanges
- On Clicked: Bind to OnApplyClicked

**Button_Reset:**
- Size: 150x50
- Text: "RESET"
- Style: Red outline
- Is Variable: True
- On Clicked: Bind to OnResetClicked

**Button_CloseFooter:**
- Size: 150x50
- Text: "CLOSE"
- Is Variable: True
- On Clicked: Bind to OnCloseFooterClicked

---

## Animation Setup

### Animation 1: OpenSettings
1. Length: 0.3 seconds
2. Tracks:
   - Border_SettingsContainer → Render Opacity: 0.0 to 1.0
   - Border_SettingsContainer → Render Transform → Scale: 0.95 to 1.0

### Animation 2: CloseSettings
1. Length: 0.2 seconds
2. Tracks:
   - Border_SettingsContainer → Render Opacity: 1.0 to 0.0

### Animation 3: CategoryHighlight
1. Length: 0.2 seconds
2. Tracks:
   - [Target Button] → Background Color: Gray to Cyan

---

## Event Graph Implementation

### Event Construct
```
Event Construct
├── Set Visibility (Collapsed)
├── Load Current Settings
└── Initialize All Controls
```

### BP_SettingsOpened
```
BP_SettingsOpened
├── Set Visibility (Visible)
├── Play Animation (OpenSettings)
├── Set Input Mode (UI Only)
├── Show Mouse Cursor (True)
├── Load Current Settings
├── Select Category (Graphics)
└── Play Sound (SFX_MenuOpen)
```

### BP_SettingsClosed
```
BP_SettingsClosed
├── Branch (HasUnsavedChanges)
│   ├── True: Show Confirmation Dialog
│   └── False: Close Immediately
├── Play Animation (CloseSettings)
├── Delay (0.2s)
├── Set Visibility (Collapsed)
├── Set Input Mode (Game Only)
└── Show Mouse Cursor (False)
```

### BP_SettingsApplied
```
BP_SettingsApplied
├── Save All Settings
├── Apply Graphics Settings
├── Apply Audio Settings
├── Apply Control Settings
├── Apply Gameplay Settings
├── Apply Accessibility Settings
├── Set HasUnsavedChanges (False)
├── Update Unsaved Indicator
└── Play Sound (SFX_SettingsApplied)
```

### BP_CategorySelected
```
BP_CategorySelected (Category)
├── Update Category Button States
├── Switch on Category
│   ├── Graphics: Set Active Widget Index (0)
│   ├── Audio: Set Active Widget Index (1)
│   ├── Controls: Set Active Widget Index (2)
│   ├── Gameplay: Set Active Widget Index (3)
│   └── Accessibility: Set Active Widget Index (4)
└── Play Sound (SFX_CategoryChange)
```

### Custom Function: MarkAsUnsaved
```
MarkAsUnsaved
├── Set HasUnsavedChanges (True)
├── Set Visibility (TextBlock_UnsavedIndicator, Visible)
└── Enable Button_Apply
```

### Custom Function: LoadCurrentSettings
```
LoadCurrentSettings
├── Get Game User Settings
├── Load Graphics Settings
│   ├── Set Resolution ComboBox
│   ├── Set Window Mode ComboBox
│   ├── Set Quality ComboBox
│   ├── Set VSync CheckBox
│   └── Set Frame Limit ComboBox
├── Load Audio Settings
│   ├── Set Master Volume Slider
│   ├── Set Music Volume Slider
│   └── Set SFX Volume Slider
├── Load Control Settings
│   ├── Set Mouse Sensitivity Slider
│   └── Set Invert Y CheckBox
├── Load Gameplay Settings
│   └── Set Difficulty ComboBox
└── Load Accessibility Settings
    └── Set UI Scale Slider
```

### Custom Function: SaveAllSettings
```
SaveAllSettings
├── Get Game User Settings
├── Save Graphics Settings
├── Save Audio Settings
├── Save Control Settings
├── Save Gameplay Settings
├── Save Accessibility Settings
└── Apply Settings to Game User Settings
```

---

## Testing Checklist

### Visual Tests
- [ ] Settings menu fills screen
- [ ] All categories visible
- [ ] All settings controls display correctly
- [ ] Unsaved indicator shows when changed
- [ ] Active category highlighted

### Functional Tests
- [ ] OpenSettings shows menu
- [ ] CloseSettings hides menu
- [ ] ApplySettings saves changes
- [ ] ResetToDefaults restores defaults
- [ ] SelectCategory switches pages
- [ ] All sliders work
- [ ] All checkboxes work
- [ ] All combo boxes work

### Category Tests
- [ ] Graphics category shows graphics settings
- [ ] Audio category shows audio settings
- [ ] Controls category shows control settings
- [ ] Gameplay category shows gameplay settings
- [ ] Accessibility category shows accessibility settings

### Save/Load Tests
- [ ] Settings persist after closing
- [ ] Settings load correctly on open
- [ ] Apply button saves changes
- [ ] Reset button restores defaults
- [ ] Unsaved changes detected

### Performance Tests
- [ ] Menu opens smoothly
- [ ] Category switching instant
- [ ] Slider updates smooth
- [ ] No memory leaks

---

## Asset Requirements

### Fonts
- F_HomeVideo

### Sounds
- SFX_MenuOpen
- SFX_MenuClose
- SFX_SettingsApplied
- SFX_CategoryChange
- SFX_ButtonClick
- SFX_SliderChange

### Colors
- Primary: Cyan (0.0, 0.8, 1.0, 1.0)
- Background: Black (0.0, 0.0, 0.0, 0.95)
- Panel: Dark Gray (0.05, 0.05, 0.05, 1.0)
- Text: White (1.0, 1.0, 1.0, 1.0)
- Warning: Yellow (1.0, 0.8, 0.0, 1.0)
- Danger: Red (1.0, 0.2, 0.2, 1.0)
