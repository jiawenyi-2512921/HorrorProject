# UI System Blueprint Implementation Guide

## Overview

The UI system provides 6 Widget classes for the horror game interface:
- **UBodycamOverlayWidget** - Recording HUD with VHS effects
- **UEvidenceToastWidget** - Evidence collection notifications
- **UObjectiveToastWidget** - Objective update notifications
- **UArchiveMenuWidget** - Evidence archive viewer
- **UPauseMenuWidget** - Game pause menu
- **USettingsMenuWidget** - Settings interface

All widgets inherit from `UUserWidget` and support Blueprint implementation.

## C++ Foundation

### Widget Architecture

All UI widgets follow this pattern:
- C++ base class handles logic and data
- Blueprint implementation handles visual design
- Events bridge C++ to Blueprint for UI updates
- `BlueprintImplementableEvent` for designer customization

### Common Widget Features

- Native Construct/Tick for initialization and updates
- Blueprint events for visual updates
- Automatic binding to game systems
- Performance-optimized updates
- Accessibility support

## Blueprint Implementation

### Step 1: Create Widget Blueprints

**Location**: `Content/UI/Widgets/`

#### 1. WBP_BodycamOverlay

**Parent Class**: BodycamOverlayWidget

**Purpose**: Main HUD showing recording status, battery, timestamp, VHS effects.

**Required UI Elements**:
- Recording indicator (red dot, blinking)
- Battery level bar/icon
- Timestamp text (HH:MM:SS format)
- Camera mode indicator
- VHS scanlines overlay
- Noise/grain overlay
- Vignette effect

**Widget Hierarchy**:
```
Canvas Panel
├── Overlay (VHS Effects)
│   ├── Image (Scanlines)
│   ├── Image (Noise)
│   └── Image (Vignette)
├── HorizontalBox (Top Bar)
│   ├── Image (Recording Dot)
│   ├── TextBlock (REC)
│   ├── Spacer
│   ├── TextBlock (Timestamp)
│   └── ProgressBar (Battery)
└── TextBlock (Camera Mode)
```

**Blueprint Events to Implement**:
- `BP_RecordingStateChanged(bool bRecording)` - Update recording indicator
- `BP_BatteryLevelChanged(float Percent)` - Update battery bar
- `BP_CameraModeChanged(EQuantumCameraMode Mode)` - Update mode text
- `BP_TimestampUpdated(FString FormattedTime)` - Update timestamp
- `BP_VHSIntensityChanged(float Intensity)` - Update VHS effects
- `BP_GlitchEffectTriggered(float Duration)` - Play glitch animation

**Properties to Bind**:
- Recording dot visibility (bind to `bIsRecording`)
- Battery bar percent (bind to `BatteryLevel`)
- Timestamp text (bind to formatted time)

#### 2. WBP_EvidenceToast

**Parent Class**: EvidenceToastWidget

**Purpose**: Notification when evidence is collected.

**Required UI Elements**:
- Evidence icon/thumbnail
- Evidence name text
- Evidence type text
- Background panel
- Fade in/out animation

**Widget Hierarchy**:
```
Canvas Panel
└── Border (Background)
    └── HorizontalBox
        ├── Image (Evidence Icon)
        └── VerticalBox
            ├── TextBlock (Evidence Name)
            └── TextBlock (Evidence Type)
```

**Animations**:
- `Anim_FadeIn` (0.3s) - Slide in from right, fade in
- `Anim_FadeOut` (0.5s) - Fade out
- `Anim_Glitch` (0.1s) - VHS glitch effect

**Blueprint Events to Implement**:
- `BP_ShowEvidence(FExtendedEvidenceMetadata Metadata)` - Display evidence
- `BP_HideToast()` - Hide notification

#### 3. WBP_ObjectiveToast

**Parent Class**: ObjectiveToastWidget

**Purpose**: Notification when objectives update.

**Required UI Elements**:
- Objective title text
- Objective description text
- Status icon (new/updated/completed)
- Background panel
- Fade in/out animation

**Widget Hierarchy**:
```
Canvas Panel
└── Border (Background)
    └── HorizontalBox
        ├── Image (Status Icon)
        └── VerticalBox
            ├── TextBlock (Objective Title)
            └── TextBlock (Objective Description)
```

**Animations**:
- `Anim_FadeIn` (0.3s) - Slide in from top
- `Anim_FadeOut` (0.5s) - Fade out
- `Anim_Pulse` (0.5s, looping) - Pulse for important objectives

**Blueprint Events to Implement**:
- `BP_ShowObjective(FText Title, FText Description, EObjectiveStatus Status)` - Display objective
- `BP_HideToast()` - Hide notification

#### 4. WBP_ArchiveMenu

**Parent Class**: ArchiveMenuWidget

**Purpose**: Full-screen evidence archive viewer.

**Required UI Elements**:
- Evidence list (scrollable)
- Evidence detail panel
- Evidence image viewer
- Category filter buttons
- Search box
- Close button

**Widget Hierarchy**:
```
Canvas Panel
├── Image (Background Blur)
└── Border (Main Panel)
    ├── VerticalBox
    │   ├── HorizontalBox (Header)
    │   │   ├── TextBlock (Title: "Evidence Archive")
    │   │   └── Button (Close)
    │   ├── HorizontalBox (Filters)
    │   │   ├── Button (All)
    │   │   ├── Button (Documents)
    │   │   ├── Button (Photos)
    │   │   └── Button (Audio)
    │   ├── EditableTextBox (Search)
    │   └── HorizontalBox (Content)
    │       ├── ScrollBox (Evidence List)
    │       │   └── WBP_EvidenceListItem (repeated)
    │       └── VerticalBox (Detail Panel)
    │           ├── Image (Evidence Image)
    │           ├── TextBlock (Evidence Name)
    │           ├── TextBlock (Evidence Description)
    │           └── TextBlock (Collection Info)
```

**Blueprint Events to Implement**:
- `BP_PopulateEvidenceList(TArray<FExtendedEvidenceMetadata> Evidence)` - Fill list
- `BP_SelectEvidence(FExtendedEvidenceMetadata Metadata)` - Show details
- `BP_FilterByCategory(EEvidenceCategory Category)` - Filter list
- `BP_SearchEvidence(FString SearchTerm)` - Search functionality

#### 5. WBP_PauseMenu

**Parent Class**: PauseMenuWidget

**Purpose**: Pause menu with resume, settings, quit options.

**Required UI Elements**:
- Resume button
- Settings button
- Archive button
- Main menu button
- Quit button
- Background blur

**Widget Hierarchy**:
```
Canvas Panel
├── Image (Background Blur)
└── VerticalBox (Menu)
    ├── TextBlock (Title: "PAUSED")
    ├── Button (Resume)
    ├── Button (Settings)
    ├── Button (Evidence Archive)
    ├── Button (Main Menu)
    └── Button (Quit Game)
```

**Blueprint Events to Implement**:
- `BP_OnResumeClicked()` - Resume game
- `BP_OnSettingsClicked()` - Open settings
- `BP_OnArchiveClicked()` - Open archive
- `BP_OnMainMenuClicked()` - Return to main menu
- `BP_OnQuitClicked()` - Quit game

#### 6. WBP_SettingsMenu

**Parent Class**: SettingsMenuWidget

**Purpose**: Settings interface for graphics, audio, controls.

**Required UI Elements**:
- Tab buttons (Graphics, Audio, Controls, Gameplay)
- Settings panels (one per tab)
- Sliders for volume, sensitivity
- Dropdowns for quality settings
- Checkboxes for toggles
- Apply/Reset buttons
- Back button

**Widget Hierarchy**:
```
Canvas Panel
└── Border (Main Panel)
    ├── VerticalBox
    │   ├── HorizontalBox (Header)
    │   │   ├── TextBlock (Title: "Settings")
    │   │   └── Button (Close)
    │   ├── HorizontalBox (Tabs)
    │   │   ├── Button (Graphics)
    │   │   ├── Button (Audio)
    │   │   ├── Button (Controls)
    │   │   └── Button (Gameplay)
    │   ├── WidgetSwitcher (Content)
    │   │   ├── ScrollBox (Graphics Settings)
    │   │   ├── ScrollBox (Audio Settings)
    │   │   ├── ScrollBox (Controls Settings)
    │   │   └── ScrollBox (Gameplay Settings)
    │   └── HorizontalBox (Footer)
    │       ├── Button (Apply)
    │       ├── Button (Reset)
    │       └── Button (Back)
```

**Blueprint Events to Implement**:
- `BP_OnGraphicsChanged(FGraphicsSettings Settings)` - Apply graphics
- `BP_OnAudioChanged(FAudioSettings Settings)` - Apply audio
- `BP_OnControlsChanged(FControlSettings Settings)` - Apply controls
- `BP_OnGameplayChanged(FGameplaySettings Settings)` - Apply gameplay
- `BP_LoadSettings()` - Load saved settings
- `BP_SaveSettings()` - Save settings
- `BP_ResetToDefaults()` - Reset all settings

### Step 2: Create UI Manager Blueprint

**Purpose**: Central UI management for showing/hiding widgets.

**Creation Steps**:
1. Content Browser → Blueprint Class → Actor
2. Name: `BP_UIManager`
3. Implements: No components needed (pure logic)

**Variables**:
- `BodycamOverlay` (WBP_BodycamOverlay reference)
- `PauseMenu` (WBP_PauseMenu reference)
- `ArchiveMenu` (WBP_ArchiveMenu reference)
- `SettingsMenu` (WBP_SettingsMenu reference)
- `ActiveToasts` (Array of UserWidget)

**Functions**:

#### ShowBodycamOverlay
```
Create Widget (WBP_BodycamOverlay)
  → Add to Viewport (Z-Order: 0)
  → Set BodycamOverlay variable
```

#### ShowEvidenceToast
```
Input: FExtendedEvidenceMetadata
Create Widget (WBP_EvidenceToast)
  → Call BP_ShowEvidence (Metadata)
  → Add to Viewport (Z-Order: 10)
  → Add to ActiveToasts array
  → Delay (3.0)
  → Remove from Viewport
  → Remove from ActiveToasts
```

#### ShowObjectiveToast
```
Input: FText Title, FText Description, EObjectiveStatus Status
Create Widget (WBP_ObjectiveToast)
  → Call BP_ShowObjective (Title, Description, Status)
  → Add to Viewport (Z-Order: 10)
  → Add to ActiveToasts array
  → Delay (4.0)
  → Remove from Viewport
  → Remove from ActiveToasts
```

#### TogglePauseMenu
```
Branch (Is Valid: PauseMenu)
  True:
    → Remove from Viewport
    → Set PauseMenu = null
    → Set Game Paused (false)
  False:
    → Create Widget (WBP_PauseMenu)
    → Add to Viewport (Z-Order: 100)
    → Set PauseMenu variable
    → Set Game Paused (true)
    → Set Input Mode UI Only
```

#### ShowArchiveMenu
```
Create Widget (WBP_ArchiveMenu)
  → Get All Evidence (from ArchiveSubsystem)
  → Call BP_PopulateEvidenceList
  → Add to Viewport (Z-Order: 100)
  → Set ArchiveMenu variable
  → Set Game Paused (true)
  → Set Input Mode UI Only
```

#### ShowSettingsMenu
```
Create Widget (WBP_SettingsMenu)
  → Call BP_LoadSettings
  → Add to Viewport (Z-Order: 100)
  → Set SettingsMenu variable
  → Set Input Mode UI Only
```

### Step 3: Integrate with Player Character

**In BP_PlayerCharacter**:

**Variables**:
- `UIManager` (BP_UIManager reference)

**Event BeginPlay**:
```
Event BeginPlay
  → Get Actor of Class (BP_UIManager)
  → Set UIManager variable
  → Call ShowBodycamOverlay
```

**Input Binding** (Pause):
```
Input Action (Pause)
  → UIManager → TogglePauseMenu
```

**Input Binding** (Archive):
```
Input Action (Archive)
  → UIManager → ShowArchiveMenu
```

### Step 4: Bind to Game Systems

#### Evidence Collection Integration

**In EvidenceCollectionComponent**:
```
On Evidence Collected Event
  → Get UIManager
  → ShowEvidenceToast (Evidence Metadata)
```

#### Objective System Integration

**In HorrorObjectiveManager**:
```
On Objective Updated Event
  → Get UIManager
  → ShowObjectiveToast (Title, Description, Status)
```

#### Camera System Integration

**In CameraPhotoComponent**:
```
On Photo Taken Event
  → Get BodycamOverlay
  → Trigger Flash Effect
```

**In CameraBatteryComponent**:
```
Event Tick
  → Get BodycamOverlay
  → Set Battery Level (Current Battery Percent)
```

**In CameraRecordingComponent**:
```
On Recording Started Event
  → Get BodycamOverlay
  → Set Recording State (true)

On Recording Stopped Event
  → Get BodycamOverlay
  → Set Recording State (false)
```

## Styling and Theming

### Color Palette

**Primary Colors**:
- Background: `#0A0A0A` (near black)
- Panel: `#1A1A1A` (dark gray)
- Text: `#E0E0E0` (light gray)
- Accent: `#FF3333` (red)
- Warning: `#FFAA00` (orange)
- Success: `#33FF33` (green)

**VHS Theme**:
- Scanline color: `#00FF00` with 10% opacity
- Noise: White with 5% opacity
- Vignette: Black with 30% opacity
- Glitch color: `#FF00FF` (magenta)

### Typography

**Fonts**:
- Primary: VCR OSD Mono (or similar monospace)
- Secondary: Roboto Mono
- Size: 14-24pt for body, 32-48pt for headers

**Text Effects**:
- Slight glow for readability
- Scanline overlay
- Occasional glitch effect

### Widget Styling

**Buttons**:
- Normal: Dark background, light text
- Hovered: Slight glow, color shift
- Pressed: Darker, scale down 95%
- Disabled: 50% opacity

**Panels**:
- Background: Semi-transparent dark
- Border: 1px light gray
- Blur background when fullscreen

**Progress Bars**:
- Background: Dark gray
- Fill: Gradient (green → yellow → red based on value)
- Border: 1px light gray

## Animation Guidelines

### Timing
- Fast transitions: 0.1-0.2s (button feedback)
- Normal transitions: 0.3-0.5s (widget show/hide)
- Slow transitions: 0.8-1.0s (dramatic effects)

### Easing
- UI show: Ease Out (Cubic)
- UI hide: Ease In (Cubic)
- Loops: Linear
- Bounce: Ease Out (Elastic)

### Common Animations

**Fade In**:
- Opacity: 0 → 1 (0.3s)
- Position: Offset → Center (0.3s)

**Fade Out**:
- Opacity: 1 → 0 (0.5s)

**Glitch**:
- Position: Random offset (0.05s)
- Color: Shift to magenta (0.05s)
- Repeat 3-5 times

**Pulse**:
- Scale: 1.0 → 1.05 → 1.0 (0.5s loop)
- Opacity: 1.0 → 0.8 → 1.0 (0.5s loop)

## Performance Optimization

### Widget Optimization
- Use Invalidation Boxes for static content
- Collapse unused widgets (don't just hide)
- Limit tick updates (use timers instead)
- Cache widget references
- Use object pooling for toasts

### Rendering Optimization
- Minimize overdraw (transparent layers)
- Use texture atlases for icons
- Optimize material complexity
- Disable widgets when not visible
- Use LOD for complex widgets

### Memory Optimization
- Unload unused widgets
- Clear widget references
- Use weak references where possible
- Limit widget creation frequency

## Accessibility

### Visual Accessibility
- High contrast mode option
- Adjustable text size
- Colorblind-friendly palette
- Reduce motion option
- Screen reader support (future)

### Input Accessibility
- Keyboard navigation
- Gamepad support
- Rebindable controls
- Hold vs toggle options
- Adjustable timing

## Testing Checklist

- [ ] All widget blueprints created
- [ ] UI Manager implemented
- [ ] Widgets show/hide correctly
- [ ] Animations play smoothly
- [ ] Input handling works
- [ ] Pause menu functions correctly
- [ ] Archive menu displays evidence
- [ ] Settings menu saves/loads
- [ ] Toasts appear and disappear
- [ ] Bodycam overlay updates in real-time
- [ ] Performance acceptable (60 FPS)
- [ ] No memory leaks
- [ ] Gamepad navigation works
- [ ] Keyboard navigation works
- [ ] Accessibility options work

## Troubleshooting

### Widget Not Showing
- Check Z-Order (higher = on top)
- Verify Add to Viewport called
- Check widget visibility
- Ensure parent canvas is visible

### Events Not Firing
- Verify event binding in Blueprint
- Check C++ event is called
- Ensure widget is constructed
- Check event dispatcher connections

### Performance Issues
- Profile with Widget Reflector
- Check for tick-heavy widgets
- Reduce transparency layers
- Optimize material complexity
- Use Invalidation Boxes

### Input Not Working
- Check Input Mode (UI vs Game)
- Verify button IsEnabled
- Check Z-Order (top widget gets input)
- Ensure widget is focusable

## Best Practices

1. **Separate Logic and Visuals**: C++ for logic, Blueprint for visuals
2. **Use Events**: Bridge C++ to Blueprint with events
3. **Cache References**: Don't search for widgets every frame
4. **Pool Widgets**: Reuse toast widgets instead of creating new
5. **Invalidation**: Use Invalidation Boxes for static content
6. **Consistent Styling**: Use shared styles and themes
7. **Responsive Design**: Support multiple resolutions
8. **Test Early**: Test UI on target hardware
9. **Accessibility**: Consider all players
10. **Document**: Comment complex UI logic

## Next Steps

1. Create all widget blueprints
2. Implement UI Manager
3. Style widgets with VHS theme
4. Create animations
5. Integrate with game systems
6. Test on multiple resolutions
7. Optimize performance
8. Add accessibility options
