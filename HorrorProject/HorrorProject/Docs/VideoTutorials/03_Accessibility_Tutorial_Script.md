# Video Tutorial Script: Accessibility System

**Duration:** 9-11 minutes  
**Target Audience:** Developers implementing accessibility features

---

## Introduction (0:00 - 0:30)

"Welcome to the HorrorProject Accessibility System tutorial. Learn how to make your game accessible to all players with WCAG 2.1 AA compliant features including subtitles, color blind modes, and visual audio cues."

**Show:** Split screen showing different accessibility features

---

## Part 1: Subtitle System (0:30 - 2:30)

"Let's start with subtitles - essential for deaf and hard-of-hearing players."

**Show:** Game with subtitles appearing

```cpp
UAccessibilitySubsystem* Accessibility = 
    GetGameInstance()->GetSubsystem<UAccessibilitySubsystem>();

Accessibility->DisplaySubtitle(
    FText::FromString("I hear something in the darkness..."),
    3.0f,  // Duration
    TEXT("Player")  // Speaker
);
```

**Show:** Subtitle appearing with speaker name

"Customize subtitle appearance:"

```cpp
FAccessibilitySettings Settings;
Settings.bSubtitlesEnabled = true;
Settings.SubtitleSize = ESubtitleSize::Large;
Settings.SubtitleColor = FLinearColor::White;
Settings.SubtitleBackgroundColor = FLinearColor::Black;
Accessibility->ApplyAccessibilitySettings(Settings);
```

**Show:** Subtitle size and color changing

---

## Part 2: Visual Audio Cues (2:30 - 4:30)

"Visual audio cues help players who can't hear directional sounds."

**Show:** Visual indicators appearing around player

```cpp
Accessibility->TriggerVisualAudioCue(
    SoundLocation,
    TEXT("Footstep"),
    1.0f  // Intensity
);
```

**Show:** Different cue types with colors

"Each sound type has a distinct color:
- Yellow: Footsteps
- Green: Voice
- Red: Danger
- Blue: Doors
- White: Items"

**Show:** Gameplay with multiple cues active

---

## Part 3: Color Blind Modes (4:30 - 6:30)

"Support players with color vision deficiency."

**Show:** Normal view

```cpp
Accessibility->SetColorBlindMode(EColorBlindMode::Protanopia);
```

**Show:** Protanopia filter applied

"Compare the three modes:"

**Show:** Side-by-side comparison
- Normal
- Protanopia (red-blind)
- Deuteranopia (green-blind)
- Tritanopia (blue-blind)

"Adjust severity for individual needs:"

```cpp
Accessibility->SetColorBlindSeverity(0.8f);
```

**Show:** Severity slider in action

---

## Part 4: Motion Settings (6:30 - 7:30)

"Reduce motion for players sensitive to movement."

**Show:** Game with full motion effects

```cpp
Settings.bMotionBlurEnabled = false;
Settings.CameraShakeIntensity = 0.3f;
Settings.bReduceFlashingLights = true;
Accessibility->ApplyAccessibilitySettings(Settings);
```

**Show:** Same scene with reduced motion

"Notice the difference - smoother, less jarring movement."

---

## Part 5: UI Accessibility (7:30 - 8:30)

"Make UI accessible with scaling and high contrast."

**Show:** Normal UI

```cpp
Settings.UIScale = 1.5f;
Settings.bHighContrastMode = true;
Accessibility->ApplyAccessibilitySettings(Settings);
```

**Show:** UI scaling up and high contrast applied

"Keyboard navigation is fully supported:"

**Show:** Navigating UI with keyboard only

---

## Part 6: Control Remapping (8:30 - 9:30)

"Players can remap all controls to their preferences."

**Show:** Control settings menu

"The system supports:
- Full keyboard remapping
- Gamepad customization
- Primary and secondary bindings
- Conflict detection
- Hold duration adjustment"

**Show:** Remapping a key and testing it

---

## Part 7: Complete Settings Example (9:30 - 10:30)

"Here's a complete accessibility setup:"

```cpp
FAccessibilitySettings Settings;
Settings.bSubtitlesEnabled = true;
Settings.SubtitleSize = ESubtitleSize::Large;
Settings.bVisualAudioCuesEnabled = true;
Settings.bMotionBlurEnabled = false;
Settings.CameraShakeIntensity = 0.5f;
Settings.UIScale = 1.5f;
Settings.bHighContrastMode = true;

Accessibility->ApplyAccessibilitySettings(Settings);
Accessibility->SaveAccessibilitySettings();
```

**Show:** All features working together in gameplay

---

## Part 8: WCAG Compliance (10:30 - 11:00)

"This system meets WCAG 2.1 AA standards:"

**Show:** Checklist with checkmarks

"✓ Text alternatives for audio
✓ Captions and subtitles
✓ Keyboard accessible
✓ Color contrast (4.5:1 minimum)
✓ No seizure-inducing content
✓ Navigable interface"

---

## Conclusion (11:00 - 11:30)

"You now know how to:
- Implement subtitles
- Add visual audio cues
- Support color blind players
- Reduce motion effects
- Make UI accessible
- Enable control remapping"

**Show:** Montage of all features

"Making games accessible benefits everyone. Check the documentation for more details. Thanks for watching!"

---

## Visual Notes

- Show before/after comparisons
- Use split screen for feature demonstrations
- Highlight accessibility icons
- Show real players using features
- Use clear, high-contrast visuals
- Include captions on the video itself

## B-Roll Suggestions

- Players using accessibility features
- Settings menu interactions
- Visual cue animations
- Color blind filter comparisons
- Subtitle customization
- Control remapping process
