# Configuration Guide

## Getting Started

### Accessing Settings

#### From C++
```cpp
// Get settings subsystem
UGameInstance* GameInstance = GetGameInstance();
UGameSettingsSubsystem* SettingsSubsystem = GameInstance->GetSubsystem<UGameSettingsSubsystem>();

// Access specific settings
UGraphicsSettings* Graphics = SettingsSubsystem->GetGraphicsSettings();
UAudioSettings* Audio = SettingsSubsystem->GetAudioSettings();
UControlSettings* Controls = SettingsSubsystem->GetControlSettings();
UGameplaySettings* Gameplay = SettingsSubsystem->GetGameplaySettings();
```

#### From Blueprint
```
Get Game Instance -> Get Subsystem (GameSettingsSubsystem) -> Get Graphics Settings
```

### Loading and Saving

#### Automatic Loading
Settings are automatically loaded when the game starts:
```cpp
void UGameSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadSettings(); // Automatic
}
```

#### Manual Save
```cpp
// Save all settings
SettingsSubsystem->SaveSettings();

// Save specific category
SettingsSubsystem->MarkDirty(TEXT("Graphics"));
SettingsSubsystem->SaveSettings();
```

#### Auto-Save on Exit
Settings are automatically saved when the game closes if there are unsaved changes.

## Graphics Configuration

### Resolution and Display

#### Set Resolution
```cpp
FIntPoint NewResolution(2560, 1440);
GraphicsSettings->SetResolution(NewResolution, true);
```

#### Toggle Fullscreen
```cpp
GraphicsSettings->SetFullscreen(true, true);
```

#### Enable VSync
```cpp
GraphicsSettings->bVSync = true;
GraphicsSettings->Apply();
```

### Quality Presets

#### Apply Preset
```cpp
// 0=Low, 1=Medium, 2=High, 3=Ultra, 4=Cinematic
GraphicsSettings->ApplyQualityPreset(2);
```

#### Get Available Presets
```cpp
TArray<FString> Presets = SettingsSubsystem->GetAvailableQualityPresets();
```

### Individual Quality Settings

```cpp
// View distance (0-4)
GraphicsSettings->ViewDistanceQuality = 3;

// Texture quality (0-4)
GraphicsSettings->TextureQuality = 3;

// Effects quality (0-4)
GraphicsSettings->EffectsQuality = 3;

// Shadow quality
GraphicsSettings->ShadowQuality = EShadowQuality::High;

// Apply changes
GraphicsSettings->Apply();
```

### Advanced Features

#### Motion Blur
```cpp
GraphicsSettings->bMotionBlur = true;
GraphicsSettings->MotionBlurAmount = 0.5f;
```

#### Ambient Occlusion
```cpp
GraphicsSettings->bAmbientOcclusion = true;
```

#### Ray Tracing
```cpp
if (GraphicsSettings->IsRayTracingSupported())
{
    GraphicsSettings->bRayTracingEnabled = true;
    GraphicsSettings->bRayTracedShadows = true;
    GraphicsSettings->bRayTracedReflections = true;
}
```

### Auto-Detection

```cpp
// Automatically detect optimal settings
GraphicsSettings->AutoDetect();
```

## Audio Configuration

### Volume Control

#### Master Volume
```cpp
AudioSettings->SetMasterVolume(0.8f, true);
```

#### Category Volumes
```cpp
AudioSettings->SetCategoryVolume(TEXT("Music"), 0.7f, true);
AudioSettings->SetCategoryVolume(TEXT("SFX"), 1.0f, true);
AudioSettings->SetCategoryVolume(TEXT("Dialogue"), 1.0f, true);
AudioSettings->SetCategoryVolume(TEXT("Ambience"), 0.6f, true);
```

### Audio Quality

```cpp
// Audio quality (0-4)
AudioSettings->AudioQuality = 3;

// Enable reverb
AudioSettings->bEnableReverb = true;

// Enable occlusion
AudioSettings->bEnableOcclusion = true;

// Max audio channels
AudioSettings->MaxChannels = 32;
```

### Spatial Audio

```cpp
if (AudioSettings->IsSpatialAudioSupported())
{
    AudioSettings->bEnableSpatialAudio = true;
    AudioSettings->bEnableHRTF = true;
}
```

### Output Device

```cpp
// Get available devices
TArray<FString> Devices = AudioSettings->GetAvailableOutputDevices();

// Set output device
AudioSettings->OutputDevice = TEXT("Default");
```

### Accessibility

```cpp
// Enable subtitles
AudioSettings->bEnableSubtitles = true;
AudioSettings->SubtitleSize = 1.2f;

// Enable closed captions
AudioSettings->bEnableClosedCaptions = true;
```

## Control Configuration

### Mouse Settings

```cpp
// Sensitivity
ControlSettings->MouseSensitivity = 1.5f;

// Invert axes
ControlSettings->bInvertMouseY = true;
ControlSettings->bInvertMouseX = false;

// Mouse smoothing
ControlSettings->MouseSmoothing = 0.1f;

// Mouse acceleration
ControlSettings->bEnableMouseAcceleration = false;
```

### Gamepad Settings

```cpp
// Sensitivity
ControlSettings->GamepadSensitivity = 1.0f;

// Invert axes
ControlSettings->bInvertGamepadY = false;
ControlSettings->bInvertGamepadX = false;

// Deadzone
ControlSettings->GamepadDeadzone = 0.25f;

// Vibration
ControlSettings->bEnableGamepadVibration = true;
ControlSettings->VibrationIntensity = 1.0f;
```

### Key Bindings

#### Set Key Binding
```cpp
// Set primary key
ControlSettings->SetKeyBinding(TEXT("Jump"), EKeys::SpaceBar, true);

// Set secondary key
ControlSettings->SetKeyBinding(TEXT("Jump"), EKeys::Gamepad_FaceButton_Bottom, false);
```

#### Get Key Binding
```cpp
FKeyBinding Binding = ControlSettings->GetKeyBinding(TEXT("Jump"));
FKey PrimaryKey = Binding.PrimaryKey;
FKey SecondaryKey = Binding.SecondaryKey;
```

#### Check for Conflicts
```cpp
if (ControlSettings->IsKeyConflicting(EKeys::W))
{
    // Handle conflict
}
```

#### Clear Key Binding
```cpp
ControlSettings->ClearKeyBinding(TEXT("Jump"), true);
```

### Accessibility

```cpp
// Toggle crouch instead of hold
ControlSettings->bEnableToggleCrouch = true;

// Toggle sprint instead of hold
ControlSettings->bEnableToggleSprint = true;

// Auto-aim assistance
ControlSettings->bEnableAutoAim = false;

// Hold button duration
ControlSettings->HoldButtonDuration = 0.5f;
```

## Gameplay Configuration

### Difficulty

```cpp
// Set difficulty
GameplaySettings->SetDifficulty(EDifficultyLevel::Hard);

// Manual multipliers
GameplaySettings->DamageMultiplier = 1.5f;
GameplaySettings->EnemyHealthMultiplier = 1.5f;

// Permadeath mode
GameplaySettings->bPermadeath = false;
```

### HUD Settings

```cpp
// HUD elements
GameplaySettings->bShowCrosshair = true;
GameplaySettings->bShowHealthBar = true;
GameplaySettings->bShowMinimap = true;
GameplaySettings->bShowObjectiveMarkers = true;

// HUD appearance
GameplaySettings->HUDScale = 1.0f;
GameplaySettings->HUDOpacity = 0.9f;
```

### Camera Settings

```cpp
// Field of view
GameplaySettings->FieldOfView = 90.0f;

// Camera effects
GameplaySettings->bEnableCameraShake = true;
GameplaySettings->CameraShakeIntensity = 1.0f;
GameplaySettings->bEnableHeadBob = true;
```

### Gameplay Assists

```cpp
// Auto-save
GameplaySettings->bEnableAutoSave = true;
GameplaySettings->AutoSaveInterval = 300.0f; // 5 minutes

// Hints and tutorials
GameplaySettings->bEnableQuickTimeEvents = true;
GameplaySettings->bEnableTutorialHints = true;
GameplaySettings->bEnableObjectiveHints = true;
```

### Language

```cpp
// Get available languages
TArray<FString> Languages = GameplaySettings->GetAvailableLanguages();

// Set language
GameplaySettings->SetLanguage(TEXT("en"));
GameplaySettings->VoiceLanguage = TEXT("en");
```

### Accessibility

```cpp
// Colorblind mode
GameplaySettings->bColorBlindMode = true;
GameplaySettings->ColorBlindType = 1; // Deuteranopia

// Motion sensitivity
GameplaySettings->bReduceFlashing = true;
GameplaySettings->bReduceMotion = true;

// Text size
GameplaySettings->TextSize = 1.2f;
```

## Import/Export

### Export Settings

```cpp
FString ExportPath = FPaths::ProjectSavedDir() / TEXT("MySettings.json");
bool bSuccess = SettingsSubsystem->ExportSettings(ExportPath);
```

### Import Settings

```cpp
FString ImportPath = FPaths::ProjectSavedDir() / TEXT("MySettings.json");
bool bSuccess = SettingsSubsystem->ImportSettings(ImportPath);

if (bSuccess)
{
    SettingsSubsystem->ApplySettings();
}
```

## Backup and Restore

### Create Backup

```cpp
UConfigManager* ConfigManager = SettingsSubsystem->GetConfigManager();
ConfigManager->CreateBackup(TEXT("BeforeChanges"));
```

### List Backups

```cpp
TArray<FString> Backups = ConfigManager->GetAvailableBackups();
for (const FString& Backup : Backups)
{
    UE_LOG(LogTemp, Log, TEXT("Backup: %s"), *Backup);
}
```

### Restore Backup

```cpp
ConfigManager->RestoreBackup(TEXT("BeforeChanges"));
SettingsSubsystem->LoadSettings();
SettingsSubsystem->ApplySettings();
```

## Validation

### Validate Settings

```cpp
// Validate specific category
bool bValid = SettingsSubsystem->ValidateSettings(TEXT("Graphics"));

// Validate all settings
bool bAllValid = SettingsSubsystem->ValidateSettings();
```

### Get Validation Results

```cpp
UConfigValidator* Validator = NewObject<UConfigValidator>();
FValidationResult Result = Validator->ValidateAllSettings(
    Graphics, Audio, Controls, Gameplay
);

if (!Result.bIsValid)
{
    for (const FString& Error : Result.Errors)
    {
        UE_LOG(LogTemp, Error, TEXT("Validation Error: %s"), *Error);
    }
}

for (const FString& Warning : Result.Warnings)
{
    UE_LOG(LogTemp, Warning, TEXT("Validation Warning: %s"), *Warning);
}
```

## Events

### Listen for Settings Changes

```cpp
SettingsSubsystem->OnSettingsChanged.AddDynamic(this, &AMyActor::OnSettingsChanged);

void AMyActor::OnSettingsChanged(FName Category)
{
    if (Category == TEXT("Graphics") || Category == NAME_None)
    {
        // Refresh graphics
    }
    
    if (Category == TEXT("Audio") || Category == NAME_None)
    {
        // Refresh audio
    }
}
```

## Best Practices

### 1. Always Apply After Changes
```cpp
GraphicsSettings->ViewDistanceQuality = 3;
GraphicsSettings->TextureQuality = 3;
GraphicsSettings->Apply(); // Don't forget!
```

### 2. Validate Before Applying
```cpp
if (SettingsSubsystem->ValidateSettings(TEXT("Graphics")))
{
    SettingsSubsystem->ApplySettings(TEXT("Graphics"));
}
```

### 3. Use Presets for Quick Setup
```cpp
// Instead of setting each value individually
GraphicsSettings->ApplyQualityPreset(2); // High preset
```

### 4. Check Hardware Support
```cpp
if (GraphicsSettings->IsRayTracingSupported())
{
    GraphicsSettings->bRayTracingEnabled = true;
}
```

### 5. Provide User Feedback
```cpp
FValidationResult Result = Validator->ValidateGraphicsSettings(Graphics);
if (!Result.bIsValid)
{
    // Show error message to user
    ShowErrorDialog(Result.Errors[0]);
}
```

### 6. Save Periodically
```cpp
// Save after important changes
SettingsSubsystem->SaveSettings();
```

### 7. Use Auto-Detection
```cpp
// Let the system detect optimal settings
SettingsSubsystem->AutoDetectSettings();
```

## Troubleshooting

### Settings Not Persisting
- Check file permissions in `ProjectSaved/Config/`
- Verify `SaveSettings()` is called
- Check for validation errors

### Settings Not Applying
- Call `Apply()` after changes
- Check validation results
- Verify hardware support for features

### Performance Issues
- Use lower quality presets
- Disable expensive features (ray tracing, high shadows)
- Reduce resolution

### Key Binding Conflicts
- Use `IsKeyConflicting()` before setting
- Clear conflicting bindings first
- Provide UI feedback for conflicts
