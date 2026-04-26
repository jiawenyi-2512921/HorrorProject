# Settings System Architecture

## Overview

The Settings System provides a comprehensive configuration management solution for the Horror Project, handling graphics, audio, controls, and gameplay settings with persistence, validation, and migration capabilities.

## Architecture

### Core Components

#### 1. GameSettingsSubsystem
- **Purpose**: Central hub for all game settings
- **Responsibilities**:
  - Manages all settings categories
  - Coordinates loading/saving operations
  - Handles dirty tracking
  - Provides unified API for settings access
  - Broadcasts settings change events

#### 2. Settings Categories

##### GraphicsSettings
- Display settings (resolution, fullscreen, VSync)
- Quality presets (Low, Medium, High, Ultra, Cinematic)
- Individual quality settings (textures, shadows, effects)
- Advanced features (motion blur, AO, ray tracing)
- Auto-detection of hardware capabilities

##### AudioSettings
- Volume controls (master, music, SFX, dialogue, ambience)
- Audio quality settings
- Spatial audio configuration
- Output device selection
- Accessibility features (subtitles, closed captions)

##### ControlSettings
- Mouse settings (sensitivity, inversion, acceleration)
- Gamepad settings (sensitivity, deadzone, vibration)
- Key bindings with conflict detection
- Accessibility options (toggle crouch/sprint)

##### GameplaySettings
- Difficulty levels
- HUD configuration
- Camera settings (FOV, shake, head bob)
- Language and localization
- Accessibility features (colorblind mode, reduced motion)

### Configuration Management

#### ConfigManager
- **File Format**: JSON
- **Storage Location**: `ProjectSaved/Config/`
- **Features**:
  - Load/Save individual categories
  - Import/Export complete settings
  - Backup/Restore functionality
  - Version management

#### ConfigValidator
- Validates all settings before application
- Provides detailed error and warning messages
- Checks for:
  - Value ranges
  - Hardware compatibility
  - Key binding conflicts
  - Logical consistency

#### ConfigMigration
- Handles version upgrades
- Automatic backup before migration
- Incremental migration path
- Rollback capability

#### DefaultConfigs
- Provides sensible defaults
- Hardware-based recommendations
- Quality preset definitions

### UI System

#### SettingsMenuWidget
- Main settings interface
- Tab-based navigation
- Apply/Reset/Back actions
- Dirty state tracking

#### Category Widgets
- **GraphicsSettingsWidget**: Graphics configuration UI
- **AudioSettingsWidget**: Audio configuration UI
- **ControlsSettingsWidget**: Control configuration UI

Each widget:
- Binds to settings objects
- Provides real-time preview
- Validates input
- Updates settings on change

## Data Flow

### Loading Settings
```
Game Start
    ↓
GameSettingsSubsystem::Initialize()
    ↓
ConfigManager::LoadConfig() for each category
    ↓
JSON Deserialization
    ↓
Settings Objects Populated
    ↓
ConfigMigration::MigrateIfNeeded()
    ↓
Settings Ready
```

### Saving Settings
```
User Changes Settings
    ↓
Widget Updates Settings Object
    ↓
MarkDirty(Category)
    ↓
User Clicks Apply
    ↓
ApplySettings()
    ↓
ConfigValidator::Validate()
    ↓
ConfigManager::SaveConfig()
    ↓
JSON Serialization
    ↓
File Written
```

### Quality Preset Application
```
User Selects Preset
    ↓
GraphicsSettings::ApplyQualityPreset()
    ↓
Update All Quality Settings
    ↓
Check Hardware Compatibility
    ↓
Disable Unsupported Features
    ↓
Apply to Engine
    ↓
Notify UI to Refresh
```

## File Structure

### Config Files
```
ProjectSaved/
└── Config/
    ├── Graphics.json
    ├── Audio.json
    ├── Controls.json
    ├── Gameplay.json
    └── Backups/
        ├── PreMigration_v0_2024-01-01/
        └── Manual_Backup_2024-01-02/
```

### JSON Format Example
```json
{
  "ConfigVersion": 1,
  "Resolution": {
    "X": 1920,
    "Y": 1080
  },
  "bFullscreen": true,
  "bVSync": false,
  "QualityPreset": 2,
  "ViewDistanceQuality": 3,
  "TextureQuality": 3
}
```

## Quality Presets

### Low (0)
- Minimum settings for low-end hardware
- FXAA anti-aliasing
- Reduced effects and shadows
- No ray tracing

### Medium (1)
- Balanced settings for mid-range hardware
- TAA anti-aliasing
- Standard effects and shadows
- No ray tracing

### High (2)
- Enhanced visuals for high-end hardware
- TAA anti-aliasing
- High-quality effects and shadows
- No ray tracing

### Ultra (3)
- Maximum quality for enthusiast hardware
- TAA anti-aliasing
- Ultra effects and shadows
- Ray tracing if supported

### Cinematic (4)
- Absolute maximum quality
- All features enabled
- Full ray tracing if supported
- May impact performance

## Validation Rules

### Graphics
- Resolution: 640x480 to 7680x4320
- Quality settings: 0-4
- Brightness: 0.0-2.0
- Gamma: 1.0-3.0
- Frame rate limit: 0-300

### Audio
- All volumes: 0.0-1.0
- Audio quality: 0-4
- Max channels: 8-128

### Controls
- Mouse sensitivity: 0.1-10.0
- Gamepad sensitivity: 0.1-10.0
- Gamepad deadzone: 0.0-1.0
- Vibration intensity: 0.0-1.0

### Gameplay
- FOV: 60.0-120.0
- HUD scale: 0.5-2.0
- HUD opacity: 0.0-1.0
- Text size: 0.5-2.0

## Events

### OnSettingsChanged
- **Trigger**: When settings are applied
- **Parameters**: Category name (or NAME_None for all)
- **Use Cases**:
  - Update UI
  - Refresh game state
  - Apply visual changes

## Best Practices

### For Developers

1. **Always validate before applying**
   ```cpp
   if (Validator->ValidateGraphicsSettings(Settings))
   {
       Settings->Apply();
   }
   ```

2. **Use dirty tracking**
   ```cpp
   SettingsSubsystem->MarkDirty(TEXT("Graphics"));
   ```

3. **Handle hardware limitations**
   ```cpp
   if (Settings->IsRayTracingSupported())
   {
       Settings->bRayTracingEnabled = true;
   }
   ```

4. **Provide feedback**
   ```cpp
   FValidationResult Result = Validator->ValidateAllSettings(...);
   for (const FString& Error : Result.Errors)
   {
       UE_LOG(LogSettings, Error, TEXT("%s"), *Error);
   }
   ```

### For Users

1. **Use quality presets** for quick configuration
2. **Enable auto-detection** for optimal settings
3. **Create backups** before major changes
4. **Export settings** to share configurations

## Performance Considerations

- Settings are loaded once at startup
- Changes are applied immediately but saved on demand
- Validation is lightweight and fast
- JSON serialization is efficient for small config files
- Backup operations are asynchronous

## Future Enhancements

- Cloud settings sync
- Per-level quality overrides
- Dynamic quality adjustment based on performance
- Settings profiles for different scenarios
- Advanced key binding editor with macros
- Settings presets sharing community
