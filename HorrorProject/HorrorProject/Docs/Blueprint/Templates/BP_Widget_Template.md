# Blueprint Widget Template

## Template: WBP_Widget_Template

Use this template as a starting point for creating UI widgets.

---

## WBP_BaseWidget Template

**Parent Class**: UserWidget

**Purpose**: Base widget with common functionality.

### Variables

**Configuration**:
- `bAutoShow` (bool, default: false) - Auto-show on construct
- `bAutoHide` (bool, default: false) - Auto-hide after duration
- `AutoHideDuration` (float, default: 3.0) - Auto-hide delay
- `ZOrder` (int32, default: 0) - Widget Z-order

**Animation**:
- `FadeInDuration` (float, default: 0.3) - Fade in time
- `FadeOutDuration` (float, default: 0.5) - Fade out time
- `bUseSlideAnimation` (bool, default: false) - Use slide animation
- `SlideDirection` (Vector2D, default: (100, 0)) - Slide offset

**State**:
- `bIsVisible` (bool, default: false) - Current visibility
- `bIsAnimating` (bool, default: false) - Currently animating

### Functions

#### ShowWidget
```
  → Set bIsVisible = true
  → Play Animation (Anim_FadeIn)
  → Branch (bAutoHide)
    True:
      → Delay (AutoHideDuration)
      → HideWidget
```

#### HideWidget
```
  → Set bIsVisible = false
  → Play Animation (Anim_FadeOut)
  → On Animation Finished:
      → Remove from Parent
```

#### UpdateContent
```
Virtual function - override in child widgets
```

### Event Graph

```
Event Construct
  → Initialize widget
  → Branch (bAutoShow)
    True: ShowWidget

Event Destruct
  → Cleanup references
```

### Animations

**Anim_FadeIn**:
- Opacity: 0 → 1 (FadeInDuration)
- Position: Offset → Center (if bUseSlideAnimation)

**Anim_FadeOut**:
- Opacity: 1 → 0 (FadeOutDuration)

---

## WBP_HUDWidget Template

**Parent Class**: WBP_BaseWidget

**Purpose**: HUD widget that updates in real-time.

### Variables

**Update**:
- `UpdateInterval` (float, default: 0.1) - Update frequency
- `bUseTimer` (bool, default: true) - Use timer instead of tick

**References**:
- `PlayerCharacter` (Character) - Player reference
- `GameState` (GameStateBase) - Game state reference

### Functions

#### UpdateHUD
```
Called by timer or tick
  → Get player data
  → Update UI elements
  → Refresh display
```

#### BindToPlayer
```
Input: Character Player
  → Set PlayerCharacter
  → Bind to player events
  → Initial update
```

### Event Graph

```
Event Construct
  → Parent: Event Construct
  → Get Player Character
  → BindToPlayer
  → Branch (bUseTimer)
    True:
      → Set Timer by Function Name (UpdateHUD, UpdateInterval, true)
    False:
      → Enable Tick
```

---

## WBP_MenuWidget Template

**Parent Class**: WBP_BaseWidget

**Purpose**: Menu widget with navigation.

### Variables

**Navigation**:
- `bUseKeyboardNavigation` (bool, default: true)
- `bUseGamepadNavigation` (bool, default: true)
- `DefaultFocusWidget` (Widget) - Initial focus widget

**Buttons**:
- `MenuButtons` (Array of Button) - All menu buttons

### Functions

#### SetupNavigation
```
  → For Each Button in MenuButtons:
      → Bind OnClicked event
      → Bind OnHovered event
      → Set navigation rules
  → Set focus to DefaultFocusWidget
```

#### OnButtonClicked
```
Input: Button ClickedButton
  → Play click sound
  → Execute button action
```

#### OnButtonHovered
```
Input: Button HoveredButton
  → Play hover sound
  → Update button visual state
```

### Event Graph

```
Event Construct
  → Parent: Event Construct
  → SetupNavigation
  → Set Input Mode (UI Only)

Event Destruct
  → Set Input Mode (Game Only)
```

---

## WBP_ToastNotification Template

**Parent Class**: WBP_BaseWidget

**Purpose**: Toast notification widget.

### Widget Hierarchy
```
Border (Background)
└── HorizontalBox
    ├── Image (Icon)
    └── VerticalBox
        ├── TextBlock (Title)
        └── TextBlock (Message)
```

### Variables

**Content**:
- `ToastTitle` (Text) - Notification title
- `ToastMessage` (Text) - Notification message
- `ToastIcon` (Texture2D) - Notification icon
- `ToastColor` (LinearColor) - Background color

**Timing**:
- `DisplayDuration` (float, default: 3.0) - Display time
- `bAutoHide` (bool, default: true) - Auto-hide after duration

### Functions

#### ShowToast
```
Input: FText Title, FText Message, UTexture2D Icon
  → Set ToastTitle
  → Set ToastMessage
  → Set ToastIcon
  → ShowWidget
  → Branch (bAutoHide)
    True:
      → Delay (DisplayDuration)
      → HideWidget
```

#### SetToastColor
```
Input: FLinearColor Color
  → Set ToastColor
  → Update background color
```

### Animations

**Anim_SlideIn**:
- Position: (100, 0) → (0, 0) (0.3s)
- Opacity: 0 → 1 (0.3s)

**Anim_SlideOut**:
- Position: (0, 0) → (100, 0) (0.5s)
- Opacity: 1 → 0 (0.5s)

---

## WBP_ProgressBar Template

**Parent Class**: UserWidget

**Purpose**: Animated progress bar.

### Widget Hierarchy
```
Border (Background)
└── ProgressBar (Bar)
    └── TextBlock (Percentage)
```

### Variables

**Progress**:
- `CurrentProgress` (float, default: 0.0) - Current value (0-1)
- `TargetProgress` (float, default: 0.0) - Target value (0-1)
- `AnimationSpeed` (float, default: 2.0) - Animation speed

**Display**:
- `bShowPercentage` (bool, default: true) - Show percentage text
- `BarColor` (LinearColor, default: Green) - Bar color
- `LowValueColor` (LinearColor, default: Red) - Color when low
- `LowValueThreshold` (float, default: 0.2) - Low value threshold

### Functions

#### SetProgress
```
Input: float Progress
  → Set TargetProgress = Clamp(Progress, 0, 1)
  → Animate to target
```

#### UpdateProgressBar
```
Called on Tick
  → Lerp CurrentProgress to TargetProgress
  → Set ProgressBar Percent
  → Update bar color based on value
  → Branch (bShowPercentage)
    True: Update percentage text
```

#### GetBarColor
```
Return: FLinearColor
  → Branch (CurrentProgress < LowValueThreshold)
    True: Return LowValueColor
    False: Return BarColor
```

### Event Graph

```
Event Tick
  → UpdateProgressBar
```

---

## WBP_ListItem Template

**Parent Class**: UserWidget

**Purpose**: List item widget for scrollable lists.

### Widget Hierarchy
```
Button (ItemButton)
└── HorizontalBox
    ├── Image (ItemIcon)
    └── VerticalBox
        ├── TextBlock (ItemTitle)
        └── TextBlock (ItemDescription)
```

### Variables

**Content**:
- `ItemData` (Object) - Item data reference
- `ItemTitle` (Text) - Item title
- `ItemDescription` (Text) - Item description
- `ItemIcon` (Texture2D) - Item icon

**State**:
- `bIsSelected` (bool, default: false) - Selection state
- `bIsHovered` (bool, default: false) - Hover state

**Colors**:
- `NormalColor` (LinearColor) - Normal background color
- `HoveredColor` (LinearColor) - Hovered background color
- `SelectedColor` (LinearColor) - Selected background color

### Functions

#### SetItemData
```
Input: Object Data
  → Set ItemData
  → Extract display info from data
  → Update UI elements
```

#### SetSelected
```
Input: bool bSelected
  → Set bIsSelected
  → Update background color
  → Play selection animation
```

#### OnItemClicked
```
  → Broadcast OnItemSelected event
  → Play click sound
```

#### OnItemHovered
```
  → Set bIsHovered = true
  → Update background color
  → Play hover sound
```

#### OnItemUnhovered
```
  → Set bIsHovered = false
  → Update background color
```

### Event Graph

```
Event Construct
  → Bind button events
  → Set initial colors

On Button Clicked
  → OnItemClicked

On Button Hovered
  → OnItemHovered

On Button Unhovered
  → OnItemUnhovered
```

---

## Best Practices

### Widget Design
1. Use consistent naming (WBP_ prefix)
2. Organize hierarchy logically
3. Use anchors for responsive design
4. Support multiple resolutions
5. Test on different aspect ratios

### Performance
1. Use Invalidation Boxes for static content
2. Collapse widgets when not visible
3. Limit tick usage (use timers)
4. Cache widget references
5. Pool frequently created widgets

### Styling
1. Use consistent color palette
2. Use consistent fonts and sizes
3. Use consistent spacing
4. Create reusable styles
5. Support themes

### Animation
1. Keep animations short (< 0.5s)
2. Use appropriate easing
3. Don't animate every frame
4. Use material animations for complex effects
5. Provide option to reduce motion

### Accessibility
1. Support keyboard navigation
2. Support gamepad navigation
3. Use readable font sizes (14pt minimum)
4. Use high contrast colors
5. Provide text alternatives for icons

---

## Testing Checklist

- [ ] Widget displays correctly
- [ ] Widget responds to input
- [ ] Widget updates in real-time
- [ ] Widget scales to different resolutions
- [ ] Widget animations play smoothly
- [ ] Widget performs well (no frame drops)
- [ ] Widget supports keyboard navigation
- [ ] Widget supports gamepad navigation
- [ ] Widget is accessible

---

## Common Issues

### Widget Not Showing
- Check Z-order (higher = on top)
- Verify Add to Viewport called
- Check widget visibility
- Ensure parent canvas is visible

### Widget Not Updating
- Check update function is called
- Verify data binding is correct
- Ensure widget is constructed
- Check for null references

### Performance Issues
- Use Widget Reflector to profile
- Check for tick-heavy widgets
- Reduce transparency layers
- Optimize material complexity
- Use Invalidation Boxes

### Input Not Working
- Check Input Mode (UI vs Game)
- Verify button IsEnabled
- Check Z-Order (top widget gets input)
- Ensure widget is focusable

---

## Example Implementation

See these files for reference:
- `/Content/UI/Widgets/WBP_BodycamOverlay`
- `/Content/UI/Widgets/WBP_EvidenceToast`
- `/Content/UI/Widgets/WBP_ObjectiveList`
- `/Content/UI/Widgets/WBP_PauseMenu`
