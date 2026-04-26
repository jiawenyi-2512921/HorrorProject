# Accessibility Features

## Overview
Complete accessibility system for HorrorProject, compliant with WCAG 2.1 AA standards.

## Core Features

### 1. Subtitle System
- **Customizable Size**: Small, Medium, Large, Extra Large
- **Color Customization**: Full color picker for text and background
- **Speaker Identification**: Shows who is speaking
- **Priority Queue**: Manages multiple simultaneous subtitles
- **Environmental Sounds**: Displays non-dialogue audio cues
- **Auto-timing**: Subtitles display for appropriate duration

### 2. Color Blind Modes
- **Protanopia (Red-Blind)**: Adjusts red color perception
- **Deuteranopia (Green-Blind)**: Adjusts green color perception
- **Tritanopia (Blue-Blind)**: Adjusts blue color perception
- **Severity Control**: Adjustable transformation intensity
- **Real-time Application**: Applied via post-process effects

### 3. Motion Settings
- **Motion Blur Toggle**: Enable/disable motion blur
- **Camera Shake Intensity**: 0-100% adjustable
- **Reduce Flashing Lights**: Minimizes rapid light changes
- **Flash Warnings**: Alerts before intense visual effects

### 4. Visual Audio Cues
- **Directional Indicators**: Shows sound source location
- **Sound Type Classification**: Different colors for different sounds
  - Yellow: Footsteps
  - Green: Voice/Dialogue
  - Red: Danger/Threats
  - Blue: Doors/Interactions
  - White: Items/Pickups
- **Distance-based**: Only shows nearby sounds
- **Intensity Visualization**: Louder sounds = larger indicators

### 5. Control Remapping
- **Full Keyboard Remapping**: All actions can be rebound
- **Gamepad Support**: Complete controller customization
- **Conflict Detection**: Warns about duplicate bindings
- **Primary/Secondary Keys**: Two keys per action
- **Hold Duration**: Adjustable hold time for actions
- **Double-tap Window**: Configurable double-tap timing

### 6. UI Accessibility
- **UI Scaling**: 50-200% size adjustment
- **High Contrast Mode**: Enhanced visibility
- **Keyboard Navigation**: Full keyboard support
- **Screen Reader Support**: Text-to-speech integration
- **Focus Indicators**: Clear visual focus states

## WCAG 2.1 AA Compliance

### Perceivable
- ✓ Text alternatives for non-text content
- ✓ Captions for audio content
- ✓ Adaptable content presentation
- ✓ Distinguishable visual elements (4.5:1 contrast minimum)

### Operable
- ✓ Keyboard accessible
- ✓ Enough time to read content
- ✓ No seizure-inducing content
- ✓ Navigable interface

### Understandable
- ✓ Readable text
- ✓ Predictable operation
- ✓ Input assistance

### Robust
- ✓ Compatible with assistive technologies
- ✓ Standards-compliant implementation

## Usage Examples

### Blueprint Usage
```cpp
// Get accessibility subsystem
UAccessibilitySubsystem* Accessibility = GetGameInstance()->GetSubsystem<UAccessibilitySubsystem>();

// Display subtitle
Accessibility->DisplaySubtitle(
    FText::FromString("I hear something..."),
    3.0f,
    "Player"
);

// Trigger visual audio cue
Accessibility->TriggerVisualAudioCue(
    SoundLocation,
    "Footstep",
    1.0f
);

// Change color blind mode
Accessibility->SetColorBlindMode(EColorBlindMode::Protanopia);
```

### C++ Usage
```cpp
// Apply custom settings
FAccessibilitySettings Settings;
Settings.bSubtitlesEnabled = true;
Settings.SubtitleSize = ESubtitleSize::Large;
Settings.bVisualAudioCuesEnabled = true;
Settings.UIScale = 1.5f;

AccessibilitySubsystem->ApplyAccessibilitySettings(Settings);
AccessibilitySubsystem->SaveAccessibilitySettings();
```

## Configuration Files
- **AccessibilitySettings.json**: Saved in `ProjectSaved/Config/`
- **ControlMappings.json**: Saved in `ProjectSaved/Config/`

## Testing
All features include comprehensive automated tests:
- TestAccessibilitySystem.cpp
- TestColorBlindMode.cpp
- TestSubtitleSystem.cpp

Run tests via: `Session Frontend > Automation > HorrorProject.Accessibility`
