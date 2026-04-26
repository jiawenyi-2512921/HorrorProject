# Settings Reference

## Complete Settings Reference

### Graphics Settings

#### Display Settings

| Property | Type | Range | Default | Description |
|----------|------|-------|---------|-------------|
| Resolution | FIntPoint | 640x480 - 7680x4320 | 1920x1080 | Screen resolution |
| bFullscreen | bool | - | true | Fullscreen mode |
| bVSync | bool | - | false | Vertical sync |
| FrameRateLimit | int32 | 0-300 | 0 | FPS cap (0=unlimited) |

#### Quality Settings

| Property | Type | Range | Default | Description |
|----------|------|-------|---------|-------------|
| QualityPreset | EQualityPreset | 0-4 | High | Overall quality preset |
| ViewDistanceQuality | int32 | 0-4 | 3 | View distance quality |
| TextureQuality | int32 | 0-4 | 3 | Texture resolution quality |
| EffectsQuality | int32 | 0-4 | 3 | Visual effects quality |
| PostProcessQuality | int32 | 0-4 | 3 | Post-processing quality |
| FoliageQuality | int32 | 0-4 | 3 | Foliage density and quality |
| ShadingQuality | int32 | 0-4 | 3 | Shading complexity |

#### Shadow Settings

| Property | Type | Range | Default | Description |
|----------|------|-------|---------|-------------|
| ShadowQuality | EShadowQuality | Low-Ultra | High | Shadow quality level |
| bDynamicShadows | bool | - | true | Enable dynamic shadows |
| ShadowDistance | float | 0-10000 | 5000.0 | Shadow draw distance |

#### Anti-Aliasing

| Property | Type | Range | Default | Description |
|----------|------|-------|---------|-------------|
| AntiAliasingMethod | EAntiAliasingMethod | None-MSAA_8x | TAA | AA method |
| AntiAliasingQuality | int32 | 0-4 | 3 | AA quality level |

#### Advanced Settings

| Property | Type | Range | Default | Description |
|----------|------|-------|---------|-------------|
| bMotionBlur | bool | - | true | Enable motion blur |
| MotionBlurAmount | float | 0.0-1.0 | 0.5 | Motion blur intensity |
| bAmbientOcclusion | bool | - | true | Enable SSAO |
| bBloom | bool | - | true | Enable bloom |
| bLensFlares | bool | - | true | Enable lens flares |
| Brightness | float | 0.0-2.0 | 1.0 | Screen brightness |
| Gamma | float | 1.0-3.0 | 2.2 | Gamma correction |

#### Ray Tracing

| Property | Type | Range | Default | Description |
|----------|------|-------|---------|-------------|
| bRayTracingEnabled | bool | - | false | Enable ray tracing |
| bRayTracedShadows | bool | - | false | RT shadows |
| bRayTracedReflections | bool | - | false | RT reflections |
| bRayTracedGlobalIllumination | bool | - | false | RT global illumination |

### Audio Settings

#### Volume Settings

| Property | Type | Range | Default | Description |
|----------|------|-------|---------|-------------|
| MasterVolume | float | 0.0-1.0 | 1.0 | Master volume |
| MusicVolume | float | 0.0-1.0 | 0.8 | Music volume |
| SFXVolume | float | 0.0-1.0 | 1.0 | Sound effects volume |
| DialogueVolume | float | 0.0-1.0 | 1.0 | Dialogue volume |
| AmbienceVolume | float | 0.0-1.0 | 0.7 | Ambient sounds volume |
| UIVolume | float | 0.0-1.0 | 0.9 | UI sounds volume |

#### Quality Settings

| Property | Type | Range | Default | Description |
|----------|------|-------|---------|-------------|
| AudioQuality | int32 | 0-4 | 3 | Audio quality level |
| bEnableReverb | bool | - | true | Enable reverb |
| bEnableOcclusion | bool | - | true | Enable audio occlusion |
| MaxChannels | int32 | 8-128 | 32 | Maximum audio channels |

#### Spatial Audio

| Property | Type | Range | Default | Description |
|----------|------|-------|---------|-------------|
| bEnableSpatialAudio | bool | - | true | Enable spatial audio |
| bEnableHRTF | bool | - | false | Enable HRTF |
| bEnableAttenuation | bool | - | true | Enable distance attenuation |

#### Output Settings

| Property | Type | Range | Default | Description |
|----------|------|-------|---------|-------------|
| OutputDevice | FString | - | "Default" | Audio output device |
| bEnableSurround | bool | - | false | Enable surround sound |
| SpeakerConfiguration | int32 | 2-8 | 2 | Speaker setup |

#### Accessibility

| Property | Type | Range | Default | Description |
|----------|------|-------|---------|-------------|
| bEnableSubtitles | bool | - | true | Show subtitles |
| SubtitleSize | float | 0.5-2.0 | 1.0 | Subtitle text size |
| bEnableClosedCaptions | bool | - | false | Show closed captions |
| bEnableAudioDescription | bool | - | false | Audio descriptions |

### Control Settings

#### Mouse Settings

| Property | Type | Range | Default | Description |
|----------|------|-------|---------|-------------|
| MouseSensitivity | float | 0.1-10.0 | 1.0 | Mouse sensitivity |
| bInvertMouseY | bool | - | false | Invert Y axis |
| bInvertMouseX | bool | - | false | Invert X axis |
| MouseSmoothing | float | 0.0-1.0 | 0.0 | Mouse smoothing |
| bEnableMouseAcceleration | bool | - | false | Mouse acceleration |

#### Gamepad Settings

| Property | Type | Range | Default | Description |
|----------|------|-------|---------|-------------|
| GamepadSensitivity | float | 0.1-10.0 | 1.0 | Gamepad sensitivity |
| bInvertGamepadY | bool | - | false | Invert Y axis |
| bInvertGamepadX | bool | - | false | Invert X axis |
| GamepadDeadzone | float | 0.0-1.0 | 0.25 | Analog stick deadzone |
| bEnableGamepadVibration | bool | - | true | Enable vibration |
| VibrationIntensity | float | 0.0-1.0 | 1.0 | Vibration strength |

#### Accessibility

| Property | Type | Range | Default | Description |
|----------|------|-------|---------|-------------|
| bEnableToggleCrouch | bool | - | false | Toggle crouch mode |
| bEnableToggleSprint | bool | - | false | Toggle sprint mode |
| bEnableAutoAim | bool | - | false | Auto-aim assistance |
| HoldButtonDuration | float | 0.1-2.0 | 0.5 | Hold button threshold |

### Gameplay Settings

#### Difficulty Settings

| Property | Type | Range | Default | Description |
|----------|------|-------|---------|-------------|
| Difficulty | EDifficultyLevel | Easy-Nightmare | Normal | Difficulty level |
| bPermadeath | bool | - | false | Permadeath mode |
| DamageMultiplier | float | 0.1-5.0 | 1.0 | Player damage taken |
| EnemyHealthMultiplier | float | 0.1-5.0 | 1.0 | Enemy health |

#### HUD Settings

| Property | Type | Range | Default | Description |
|----------|------|-------|---------|-------------|
| bShowCrosshair | bool | - | true | Show crosshair |
| bShowHealthBar | bool | - | true | Show health bar |
| bShowMinimap | bool | - | true | Show minimap |
| bShowObjectiveMarkers | bool | - | true | Show objective markers |
| HUDScale | float | 0.5-2.0 | 1.0 | HUD size |
| HUDOpacity | float | 0.0-1.0 | 1.0 | HUD transparency |

#### Gameplay Assists

| Property | Type | Range | Default | Description |
|----------|------|-------|---------|-------------|
| bEnableAutoSave | bool | - | true | Enable auto-save |
| AutoSaveInterval | float | 60-600 | 300.0 | Auto-save interval (seconds) |
| bEnableQuickTimeEvents | bool | - | true | Enable QTEs |
| bEnableTutorialHints | bool | - | true | Show tutorial hints |
| bEnableObjectiveHints | bool | - | true | Show objective hints |

#### Camera Settings

| Property | Type | Range | Default | Description |
|----------|------|-------|---------|-------------|
| FieldOfView | float | 60.0-120.0 | 90.0 | Field of view |
| bEnableCameraShake | bool | - | true | Enable camera shake |
| CameraShakeIntensity | float | 0.0-1.0 | 1.0 | Camera shake strength |
| bEnableHeadBob | bool | - | true | Enable head bobbing |

#### Language Settings

| Property | Type | Range | Default | Description |
|----------|------|-------|---------|-------------|
| Language | FString | - | "en" | UI language |
| VoiceLanguage | FString | - | "en" | Voice language |

#### Accessibility

| Property | Type | Range | Default | Description |
|----------|------|-------|---------|-------------|
| bColorBlindMode | bool | - | false | Colorblind mode |
| ColorBlindType | int32 | 0-3 | 0 | Colorblind type |
| bReduceFlashing | bool | - | false | Reduce flashing |
| bReduceMotion | bool | - | false | Reduce motion |
| TextSize | float | 0.5-2.0 | 1.0 | Text size multiplier |

## Enumerations

### EQualityPreset
```cpp
enum class EQualityPreset : uint8
{
    Low = 0,        // Minimum quality
    Medium = 1,     // Balanced quality
    High = 2,       // Enhanced quality
    Ultra = 3,      // Maximum quality
    Cinematic = 4,  // Absolute maximum
    Custom = 255    // User-defined
};
```

### EAntiAliasingMethod
```cpp
enum class EAntiAliasingMethod : uint8
{
    None,           // No anti-aliasing
    FXAA,          // Fast approximate AA
    TAA,           // Temporal AA
    MSAA_2x,       // 2x multi-sample AA
    MSAA_4x,       // 4x multi-sample AA
    MSAA_8x        // 8x multi-sample AA
};
```

### EShadowQuality
```cpp
enum class EShadowQuality : uint8
{
    Low,    // Low shadow quality
    Medium, // Medium shadow quality
    High,   // High shadow quality
    Ultra   // Ultra shadow quality
};
```

### EDifficultyLevel
```cpp
enum class EDifficultyLevel : uint8
{
    Easy,      // Easy difficulty
    Normal,    // Normal difficulty
    Hard,      // Hard difficulty
    Nightmare  // Nightmare difficulty
};
```

## Structures

### FKeyBinding
```cpp
struct FKeyBinding
{
    FName ActionName;        // Action identifier
    FKey PrimaryKey;         // Primary key
    FKey SecondaryKey;       // Secondary key
    bool bShift;            // Requires Shift
    bool bCtrl;             // Requires Ctrl
    bool bAlt;              // Requires Alt
};
```

### FValidationResult
```cpp
struct FValidationResult
{
    bool bIsValid;              // Overall validity
    TArray<FString> Errors;     // Error messages
    TArray<FString> Warnings;   // Warning messages
};
```

## API Reference

### GameSettingsSubsystem

#### Methods
```cpp
// Settings access
UGraphicsSettings* GetGraphicsSettings() const;
UAudioSettings* GetAudioSettings() const;
UControlSettings* GetControlSettings() const;
UGameplaySettings* GetGameplaySettings() const;

// Operations
void LoadSettings();
void SaveSettings();
void ResetToDefaults(FName Category = NAME_None);
void ApplySettings(FName Category = NAME_None);

// Import/Export
bool ExportSettings(const FString& FilePath);
bool ImportSettings(const FString& FilePath);

// Validation
bool ValidateSettings(FName Category = NAME_None);

// Auto-detection
void AutoDetectSettings();

// Quality presets
void ApplyQualityPreset(int32 PresetLevel);
TArray<FString> GetAvailableQualityPresets() const;

// State
bool HasUnsavedChanges() const;
void MarkDirty(FName Category);
```

#### Events
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSettingsChanged, FName, SettingsCategory);
FOnSettingsChanged OnSettingsChanged;
```

### ConfigManager

#### Methods
```cpp
// Config operations
bool LoadConfig(const FString& Category, UObject* SettingsObject);
bool SaveConfig(const FString& Category, UObject* SettingsObject);
bool DeleteConfig(const FString& Category);
bool ConfigExists(const FString& Category) const;

// Import/Export
bool ExportSettings(const FString& FilePath, ...);
bool ImportSettings(const FString& FilePath, ...);

// Backup/Restore
bool CreateBackup(const FString& BackupName);
bool RestoreBackup(const FString& BackupName);
TArray<FString> GetAvailableBackups() const;

// Version
int32 GetConfigVersion() const;
void SetConfigVersion(int32 Version);

// Paths
FString GetConfigDirectory() const;
FString GetBackupDirectory() const;
```

### ConfigValidator

#### Methods
```cpp
// Validation
bool ValidateGraphicsSettings(UGraphicsSettings* Settings);
bool ValidateAudioSettings(UAudioSettings* Settings);
bool ValidateControlSettings(UControlSettings* Settings);
bool ValidateGameplaySettings(UGameplaySettings* Settings);
FValidationResult ValidateAllSettings(...);

// Results
FValidationResult GetLastValidationResult() const;
```

### DefaultConfigs

#### Methods
```cpp
// Apply defaults
static void ApplyDefaultGraphicsSettings(UGraphicsSettings* Settings);
static void ApplyDefaultAudioSettings(UAudioSettings* Settings);
static void ApplyDefaultControlSettings(UControlSettings* Settings);
static void ApplyDefaultGameplaySettings(UGameplaySettings* Settings);

// Recommended settings
static void ApplyRecommendedGraphicsSettings(UGraphicsSettings* Settings);
static void ApplyRecommendedAudioSettings(UAudioSettings* Settings);
```

## Configuration Files

### File Locations
- **Config Directory**: `[ProjectSaved]/Config/`
- **Graphics Config**: `[ProjectSaved]/Config/Graphics.json`
- **Audio Config**: `[ProjectSaved]/Config/Audio.json`
- **Controls Config**: `[ProjectSaved]/Config/Controls.json`
- **Gameplay Config**: `[ProjectSaved]/Config/Gameplay.json`
- **Backups**: `[ProjectSaved]/Config/Backups/`

### File Format
All configuration files use JSON format with UTF-8 encoding.

### Version Management
Each config file includes a `ConfigVersion` field for migration support.

## Supported Languages

| Code | Language |
|------|----------|
| en | English |
| zh | Chinese |
| ja | Japanese |
| ko | Korean |
| es | Spanish |
| fr | French |
| de | German |
| ru | Russian |

## Colorblind Types

| Type | Description |
|------|-------------|
| 0 | None |
| 1 | Protanopia (Red-blind) |
| 2 | Deuteranopia (Green-blind) |
| 3 | Tritanopia (Blue-blind) |

## Default Key Bindings

| Action | Primary Key | Secondary Key |
|--------|-------------|---------------|
| MoveForward | W | Up Arrow |
| MoveBackward | S | Down Arrow |
| MoveLeft | A | Left Arrow |
| MoveRight | D | Right Arrow |
| Jump | Space | - |
| Crouch | Left Ctrl | C |
| Sprint | Left Shift | - |
| Interact | E | - |
| Fire | Left Mouse | - |
| Aim | Right Mouse | - |
| Reload | R | - |
| Inventory | Tab | I |
| Pause | Escape | - |
