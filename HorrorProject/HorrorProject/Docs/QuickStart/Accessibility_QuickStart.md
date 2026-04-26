# Accessibility - Quick Start Guide

Make your game accessible in 5 minutes.

## Step 1: Get Accessibility Subsystem (30 sec)

```cpp
UAccessibilitySubsystem* Accessibility = 
    GetGameInstance()->GetSubsystem<UAccessibilitySubsystem>();
```

## Step 2: Enable Subtitles (1 min)

```cpp
// Display subtitle
Accessibility->DisplaySubtitle(
    FText::FromString("I hear something in the darkness..."),
    3.0f,  // Duration
    TEXT("Player")  // Speaker
);

// Enable subtitles
FAccessibilitySettings Settings;
Settings.bSubtitlesEnabled = true;
Settings.SubtitleSize = ESubtitleSize::Large;
Accessibility->ApplyAccessibilitySettings(Settings);
```

## Step 3: Add Visual Audio Cues (1 min)

```cpp
// Show sound indicator
Accessibility->TriggerVisualAudioCue(
    SoundLocation,
    TEXT("Footstep"),  // Sound type
    1.0f  // Intensity
);

// Enable visual cues
Settings.bVisualAudioCuesEnabled = true;
Accessibility->ApplyAccessibilitySettings(Settings);
```

## Step 4: Color Blind Support (1 min)

```cpp
// Apply color blind mode
Accessibility->SetColorBlindMode(EColorBlindMode::Protanopia);  // Red-blind
Accessibility->SetColorBlindMode(EColorBlindMode::Deuteranopia);  // Green-blind
Accessibility->SetColorBlindMode(EColorBlindMode::Tritanopia);  // Blue-blind

// Adjust severity
Accessibility->SetColorBlindSeverity(0.8f);  // 0.0 to 1.0
```

## Step 5: Motion Settings (1 min)

```cpp
// Reduce motion
Settings.bMotionBlurEnabled = false;
Settings.CameraShakeIntensity = 0.3f;  // 30% intensity
Settings.bReduceFlashingLights = true;
Accessibility->ApplyAccessibilitySettings(Settings);
```

## Blueprint Quick Start

1. Get Accessibility Subsystem
2. Display Subtitle (Text: "Hello", Duration: 3.0, Speaker: "NPC")
3. Trigger Visual Audio Cue (Location, Type: "Footstep", Intensity: 1.0)
4. Set Color Blind Mode (Protanopia)

## Complete Settings Example

```cpp
FAccessibilitySettings Settings;

// Subtitles
Settings.bSubtitlesEnabled = true;
Settings.SubtitleSize = ESubtitleSize::Large;
Settings.SubtitleColor = FLinearColor::White;
Settings.SubtitleBackgroundColor = FLinearColor::Black;

// Visual cues
Settings.bVisualAudioCuesEnabled = true;

// Motion
Settings.bMotionBlurEnabled = false;
Settings.CameraShakeIntensity = 0.5f;
Settings.bReduceFlashingLights = true;

// UI
Settings.UIScale = 1.5f;
Settings.bHighContrastMode = true;

// Apply all
Accessibility->ApplyAccessibilitySettings(Settings);
Accessibility->SaveAccessibilitySettings();
```

## Visual Audio Cue Types

- `Footstep` - Yellow
- `Voice` - Green
- `Danger` - Red
- `Door` - Blue
- `Item` - White

## WCAG 2.1 AA Compliance

This system is compliant with WCAG 2.1 AA standards:
- Subtitles for all audio
- Keyboard navigation
- Color blind support
- Motion reduction
- High contrast mode

## Testing

```cpp
// Load saved settings
Accessibility->LoadAccessibilitySettings();

// Get current settings
FAccessibilitySettings Current = Accessibility->GetAccessibilitySettings();
```

## Next Steps

- Customize subtitle appearance
- Add more visual cue types
- Implement screen reader support
- Test with assistive technologies
