# UI System - Blueprint Implementation Guide

## Overview
Complete UI system with C++ foundation and Blueprint-ready components for horror game interface.

## Architecture

### C++ Foundation (Complete)
```
Source/HorrorProject/UI/
├── Widgets/
│   ├── BodycamOverlayWidget.h/cpp      ✓ Bodycam HUD
│   ├── EvidenceToastWidget.h/cpp       ✓ Evidence notifications
│   ├── ObjectiveToastWidget.h/cpp      ✓ Objective updates
│   ├── ArchiveMenuWidget.h/cpp         ✓ Archive viewer
│   ├── PauseMenuWidget.h/cpp           ✓ Pause menu
│   └── SettingsMenuWidget.h/cpp        ✓ Settings
├── Components/
│   ├── VHSEffectComponent.h/cpp        ✓ VHS post-process
│   ├── NoiseOverlayComponent.h/cpp     ✓ Film grain
│   └── ScanlineComponent.h/cpp         ✓ CRT scanlines
├── Animation/
│   └── UIAnimationLibrary.h/cpp        ✓ Animation utilities
├── UIManagerSubsystem.h/cpp            ✓ Centralized manager
├── UIEventManager.h/cpp                ✓ Event coordination
└── Tests/
    └── UIWidgetTests.cpp               ✓ Automated tests
```

## Blueprint Implementation Tasks

### 1. Bodycam Overlay Widget (WBP_BodycamOverlay)

**Parent Class:** `UBodycamOverlayWidget`

**Required Elements:**
- Recording indicator (red dot, blinking)
- Timestamp display (HH:MM:SS format)
- Battery level indicator (icon + percentage)
- Camera mode label
- VHS noise overlay
- Scanline effect

**Blueprint Events to Implement:**
- `BP_RecordingStateChanged(bool bRecording)`
- `BP_BatteryLevelChanged(float Percent)`
- `BP_CameraModeChanged(EQuantumCameraMode Mode)`
- `BP_TimestampUpdated(FString FormattedTime)`
- `BP_VHSIntensityChanged(float Intensity)`
- `BP_GlitchEffectTriggered(float Duration)`

**Assets:**
- Font: `/Content/Bodycam_VHS_Effect/UI/UI_Assets/Font/F_HomeVideo_PublicDomainLicense_Font`
- Reference: `/Content/Bodycam_VHS_Effect/UI/WBP_TimeStamp`

**Layout:**
```
┌─────────────────────────────────────┐
│ ● REC  12:34:56        [▓▓▓░] 75%  │ <- Top bar
│                                     │
│                                     │
│         [Game View]                 │
│                                     │
│                                     │
│ BODYCAM MODE: NIGHT VISION          │ <- Bottom label
└─────────────────────────────────────┘
```

### 2. Evidence Toast Widget (WBP_EvidenceToast)

**Parent Class:** `UEvidenceToastWidget`

**Required Elements:**
- Evidence icon/thumbnail
- Evidence name (large text)
- Evidence description (smaller text)
- Fade in/out animation
- VHS glitch effect on appear

**Blueprint Events:**
- `BP_ShowToast(FName EvidenceId, FText DisplayName, FText Description)`
- `BP_DismissToast()`
- `BP_PlayFadeIn()`
- `BP_PlayFadeOut()`

**Animation Timing:**
- Fade In: 0.3s
- Display: 3.0s
- Fade Out: 0.5s

### 3. Objective Toast Widget (WBP_ObjectiveToast)

**Parent Class:** `UObjectiveToastWidget`

**Required Elements:**
- Objective text (typewriter effect)
- Optional hint text
- Slide in from right
- Slide out to right

**Blueprint Events:**
- `BP_ShowToast(FGameplayTag EventTag, FText ObjectiveText, FText HintText)`
- `BP_DismissToast()`
- `BP_PlayTypewriterEffect(FText Text, float Duration)`
- `BP_PlaySlideIn()`
- `BP_PlaySlideOut()`

**Animation:**
- Slide In: 0.4s (ease out)
- Typewriter: 0.05s per character
- Display: 5.0s
- Slide Out: 0.3s (ease in)

### 4. Archive Menu Widget (WBP_ArchiveMenu)

**Parent Class:** `UArchiveMenuWidget`

**Required Elements:**
- Evidence list (scrollable)
- Notes list (scrollable)
- Filter buttons (All/Evidence/Notes)
- Detail view panel
- Close button

**Blueprint Events:**
- `BP_ArchiveOpened()`
- `BP_ArchiveClosed()`
- `BP_ArchiveDataRefreshed(TArray<FHorrorEvidenceMetadata> Evidence, TArray<FHorrorNoteMetadata> Notes)`
- `BP_EntrySelected(FName EntryId)`
- `BP_FilterApplied(EArchiveEntryType Type)`

### 5. Pause Menu Widget (WBP_PauseMenu)

**Parent Class:** `UPauseMenuWidget`

**Required Elements:**
- Resume button
- Settings button
- Quit to main menu button
- Background blur
- VHS overlay

**Blueprint Events:**
- `BP_PauseMenuOpened()`
- `BP_PauseMenuClosed()`
- `BP_SettingsOpened()`

### 6. Settings Menu Widget (WBP_SettingsMenu)

**Parent Class:** `USettingsMenuWidget`

**Required Elements:**
- Category tabs (Graphics/Audio/Controls/Gameplay/Accessibility)
- Settings panels per category
- Apply/Reset buttons
- Back button

**Blueprint Events:**
- `BP_SettingsOpened()`
- `BP_SettingsClosed()`
- `BP_SettingsApplied()`
- `BP_CategorySelected(ESettingsCategory Category)`

## Material Setup

### VHS Post-Process Material (M_VHS_PostProcess)

**Parameters:**
- `NoiseIntensity` (float, 0-1)
- `ScanlineIntensity` (float, 0-1)
- `ChromaticAberration` (float, 0-1)
- `VignetteIntensity` (float, 0-1)
- `TimeAccumulator` (float, driven by component)

**Material Nodes:**
- Noise texture sampling
- Scanline generation
- RGB channel offset
- Vignette darkening

### UI Material Instance (MI_UI_VHS)

**Parent:** `M_VHS_PostProcess`

**Preset Values:**
- NoiseIntensity: 0.15
- ScanlineIntensity: 0.2
- ChromaticAberration: 0.5
- VignetteIntensity: 0.4

## Animation Blueprint Utilities

### Available Animation Functions (UUIAnimationLibrary)

```cpp
// Fade widget opacity
FadeWidget(Widget, TargetOpacity, Duration, Curve)

// Slide widget position
SlideWidget(Widget, FromPosition, ToPosition, Duration, Curve)

// Scale widget
ScaleWidget(Widget, TargetScale, Duration, Curve)

// Pulse animation (loop)
PulseWidget(Widget, MinScale, MaxScale, Duration, bLoop)

// Typewriter text effect
TypewriterText(Widget, FullText, CharacterDelay)

// Glitch effect
GlitchWidget(Widget, Duration, Intensity)

// Curve evaluation
EvaluateCurve(Alpha, Curve) -> float
```

**Available Curves:**
- Linear
- EaseIn
- EaseOut
- EaseInOut
- Bounce
- Elastic

## UI Manager Usage

### Creating Widgets

```cpp
// Get UI Manager
UUIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UUIManagerSubsystem>();

// Create widget
UUserWidget* Widget = UIManager->CreateWidget(WidgetClass, FName("BodycamHUD"));

// Show widget
UIManager->ShowWidget(FName("BodycamHUD"));

// Hide widget
UIManager->HideWidget(FName("BodycamHUD"));
```

### Event Subscription

```cpp
// Get event manager
UUIEventManager* EventManager = UIManager->GetEventManager();

// Subscribe to events
EventManager->OnUIWidgetOpened.AddDynamic(this, &AMyActor::OnWidgetOpened);
EventManager->OnUIStateChanged.AddDynamic(this, &AMyActor::OnUIStateChanged);
```

### Batch Updates (Performance)

```cpp
// Begin batch update (prevents multiple redraws)
UIManager->BatchUpdateBegin();

// Update multiple widgets
UIManager->ShowWidget(FName("Toast1"));
UIManager->ShowWidget(FName("Toast2"));
UIManager->HideWidget(FName("OldToast"));

// Apply all changes at once
UIManager->BatchUpdateEnd();
```

## Integration with Existing Systems

### HorrorUI Integration

The existing `UHorrorUI` widget already provides:
- Sprint meter updates
- Evidence collection events
- Note recording events
- VHS feedback state
- Objective event bus integration

**Connect to new widgets:**
```cpp
// In Blueprint, implement BP_EvidenceCollected
void BP_EvidenceCollected_Implementation(FName EvidenceId, int32 TotalCount)
{
    // Get evidence metadata
    UInventoryComponent* Inventory = GetInventoryComponent();
    FHorrorEvidenceMetadata Metadata = Inventory->GetEvidenceMetadata(EvidenceId);
    
    // Show toast
    UEvidenceToastWidget* Toast = GetWidget<UEvidenceToastWidget>(FName("EvidenceToast"));
    Toast->ShowEvidenceToast(EvidenceId, Metadata);
}
```

### EventBus Integration

```cpp
// Subscribe to objective events
void UHorrorUI::OnObjectiveEventPublished(const FHorrorEventMessage& Message)
{
    // Show objective toast
    UObjectiveToastWidget* Toast = GetWidget<UObjectiveToastWidget>(FName("ObjectiveToast"));
    Toast->ShowObjectiveWithHint(Message.EventTag, Message.ObjectiveHint, FText::GetEmpty());
}
```

## Performance Guidelines

### Frame Budget
- UI updates: <1ms per frame
- Animation ticks: <0.5ms per frame
- Event broadcasts: <0.1ms per frame

### Optimization Techniques
1. **Batch Updates:** Use `BatchUpdateBegin/End` for multiple changes
2. **Widget Pooling:** Reuse toast widgets instead of creating new ones
3. **Lazy Loading:** Create menus only when first opened
4. **Tick Optimization:** Disable tick when widgets are hidden
5. **Material Instances:** Use material instances for VHS effects

### Memory Management
- Maximum active widgets: 10
- Toast widget pool size: 3
- Menu widget caching: Enabled
- Automatic cleanup on level transition

## Accessibility Features

### Required Support
- **Colorblind Modes:** Alternative colors for UI elements
- **Text Scaling:** Adjustable font sizes (100%-200%)
- **High Contrast:** Optional high contrast mode
- **Screen Reader:** Descriptive labels for all interactive elements
- **Input Remapping:** Full keyboard/gamepad customization

### Implementation
```cpp
// In SettingsMenuWidget
void ApplyAccessibilitySettings()
{
    // Text scaling
    SetGlobalTextScale(TextScalePercent / 100.0f);
    
    // High contrast
    SetHighContrastMode(bHighContrastEnabled);
    
    // Colorblind mode
    SetColorblindMode(ColorblindType);
}
```

## Testing

### Automated Tests (Complete)
- Widget creation tests
- Component functionality tests
- State management tests
- Event broadcasting tests

**Run tests:**
```
Session Frontend -> Automation -> HorrorProject.UI.*
```

### Manual Testing Checklist
- [ ] Bodycam overlay displays correctly
- [ ] Evidence toasts appear and dismiss
- [ ] Objective toasts slide in/out
- [ ] Archive menu loads all entries
- [ ] Pause menu pauses game
- [ ] Settings persist across sessions
- [ ] VHS effects perform at 60fps
- [ ] All animations are smooth
- [ ] Input works with keyboard and gamepad
- [ ] Accessibility features function

## Next Steps

1. **Create Blueprint Widgets:** Implement WBP_ versions of all widgets
2. **Setup Materials:** Create VHS post-process materials
3. **Configure Animations:** Setup widget animations in UMG
4. **Test Integration:** Connect to existing game systems
5. **Polish:** Fine-tune timings and visual effects
6. **Accessibility:** Implement accessibility features
7. **Performance:** Profile and optimize

## Asset Locations

**Fonts:**
- `/Content/Bodycam_VHS_Effect/UI/UI_Assets/Font/F_HomeVideo_PublicDomainLicense_Font`

**Reference Widgets:**
- `/Content/Bodycam_VHS_Effect/UI/WBP_TimeStamp`
- `/Content/Bodycam_VHS_Effect/Blueprints/Widgets/WBP_Screen_Blur`

**Materials:**
- Create in `/Content/Horror/UI/Materials/`

**Widgets:**
- Create in `/Content/Horror/UI/Widgets/`

## Support

All C++ classes are fully documented with Blueprint-exposed functions. Use the UE5 Blueprint editor's context menu to see available functions and events.
