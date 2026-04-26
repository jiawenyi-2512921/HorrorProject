# Accessibility Implementation Guide

## Architecture

### Subsystem Pattern
The accessibility system uses UE5's GameInstanceSubsystem for persistent, game-wide accessibility features.

```
AccessibilitySubsystem (Core)
├── SubtitleSystem (Subtitle management)
├── ColorBlindMode (Color transformations)
├── ControlRemapping (Input customization)
└── UI Widgets
    ├── AccessibilitySettingsWidget
    ├── SubtitleWidget
    └── VisualAudioCueWidget
```

## Integration Steps

### 1. Add to GameInstance
The subsystem automatically initializes with the GameInstance. No manual setup required.

### 2. Create UI Widgets

#### Subtitle Widget
```cpp
// In your HUD class
UPROPERTY(EditDefaultsOnly, Category = "UI")
TSubclassOf<USubtitleWidget> SubtitleWidgetClass;

UPROPERTY()
USubtitleWidget* SubtitleWidget;

void AMyHUD::BeginPlay()
{
    Super::BeginPlay();
    
    if (SubtitleWidgetClass)
    {
        SubtitleWidget = CreateWidget<USubtitleWidget>(GetWorld(), SubtitleWidgetClass);
        SubtitleWidget->AddToViewport();
    }
}
```

#### Visual Audio Cue Widget
```cpp
UPROPERTY(EditDefaultsOnly, Category = "UI")
TSubclassOf<UVisualAudioCueWidget> AudioCueWidgetClass;

UPROPERTY()
UVisualAudioCueWidget* AudioCueWidget;

void AMyHUD::BeginPlay()
{
    Super::BeginPlay();
    
    if (AudioCueWidgetClass)
    {
        AudioCueWidget = CreateWidget<UVisualAudioCueWidget>(GetWorld(), AudioCueWidgetClass);
        AudioCueWidget->AddToViewport();
    }
}
```

### 3. Integrate with Audio System

#### Trigger Visual Cues for Sounds
```cpp
void AMyAudioManager::PlaySound3D(USoundBase* Sound, FVector Location)
{
    // Play the sound
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        this, Sound, Location
    );
    
    // Trigger visual cue
    UAccessibilitySubsystem* Accessibility = 
        GetGameInstance()->GetSubsystem<UAccessibilitySubsystem>();
    
    if (Accessibility)
    {
        FString CueType = DetermineSoundType(Sound);
        float Intensity = Sound->GetVolume();
        
        Accessibility->TriggerVisualAudioCue(Location, CueType, Intensity);
    }
}

FString AMyAudioManager::DetermineSoundType(USoundBase* Sound)
{
    FString SoundName = Sound->GetName();
    
    if (SoundName.Contains("Footstep")) return "Footstep";
    if (SoundName.Contains("Voice")) return "Voice";
    if (SoundName.Contains("Door")) return "Door";
    if (SoundName.Contains("Monster")) return "Danger";
    
    return "Generic";
}
```

### 4. Integrate with Dialogue System

```cpp
void ADialogueManager::DisplayDialogue(FText DialogueText, AActor* Speaker)
{
    UAccessibilitySubsystem* Accessibility = 
        GetGameInstance()->GetSubsystem<UAccessibilitySubsystem>();
    
    if (Accessibility)
    {
        FString SpeakerName = Speaker ? Speaker->GetName() : "";
        float Duration = CalculateDuration(DialogueText);
        
        Accessibility->DisplaySubtitle(DialogueText, Duration, SpeakerName);
    }
}

float ADialogueManager::CalculateDuration(const FText& Text)
{
    // ~3 seconds per 10 words
    int32 WordCount = CountWords(Text.ToString());
    return FMath::Max(2.0f, WordCount * 0.3f);
}
```

### 5. Apply Camera Shake with Accessibility

```cpp
void AMyPlayerController::ApplyCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass)
{
    UAccessibilitySubsystem* Accessibility = 
        GetGameInstance()->GetSubsystem<UAccessibilitySubsystem>();
    
    float Intensity = 1.0f;
    if (Accessibility)
    {
        Intensity = Accessibility->GetAdjustedCameraShakeIntensity();
    }
    
    if (Intensity > 0.0f)
    {
        ClientStartCameraShake(ShakeClass, Intensity);
    }
}
```

### 6. Flash Warning System

```cpp
void AEffectManager::TriggerIntenseFlash()
{
    UAccessibilitySubsystem* Accessibility = 
        GetGameInstance()->GetSubsystem<UAccessibilitySubsystem>();
    
    if (Accessibility && Accessibility->ShouldShowFlashWarning())
    {
        Accessibility->TriggerFlashWarning(
            FText::FromString("Warning: Intense flashing lights ahead")
        );
        
        // Delay the flash effect
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,
            this,
            &AEffectManager::ExecuteFlash,
            2.0f,
            false
        );
    }
    else
    {
        ExecuteFlash();
    }
}
```

## Color Blind Mode Integration

### Material Setup
Create a post-process material with color blind correction:

1. Create Material with domain: Post Process
2. Add material parameter: ColorBlindMode (Scalar)
3. Implement color transformation logic
4. Apply via post-process volume

### Blueprint Implementation
```cpp
void UAccessibilitySubsystem::ApplyColorBlindShader()
{
    APostProcessVolume* PPVolume = FindPostProcessVolume();
    if (PPVolume && ColorBlindMaterial)
    {
        UMaterialInstanceDynamic* DynMaterial = 
            UMaterialInstanceDynamic::Create(ColorBlindMaterial, this);
        
        DynMaterial->SetScalarParameterValue(
            "ColorBlindMode",
            static_cast<float>(CurrentSettings.ColorBlindMode)
        );
        
        PPVolume->AddOrUpdateBlendable(DynMaterial);
    }
}
```

## Control Remapping Integration

### Input Component Setup
```cpp
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    UControlRemapping* ControlRemapping = GetControlRemappingSystem();
    
    if (ControlRemapping)
    {
        // Bind with remapped keys
        FKey JumpKey = ControlRemapping->GetMappedKey("Jump", true);
        PlayerInputComponent->BindKey(JumpKey, IE_Pressed, this, &AMyCharacter::Jump);
        
        FKey InteractKey = ControlRemapping->GetMappedKey("Interact", true);
        PlayerInputComponent->BindKey(InteractKey, IE_Pressed, this, &AMyCharacter::Interact);
    }
}
```

## Performance Considerations

### Optimization Tips
1. **Subtitle Updates**: Only update when subtitles are active
2. **Visual Audio Cues**: Limit to nearby sounds (MaxCueDistance)
3. **Color Blind Shader**: Use single post-process material
4. **Settings Caching**: Cache settings locally, update on change

### Memory Management
- Subsystem persists for game lifetime
- UI widgets created/destroyed as needed
- Settings saved to disk, not kept in memory

## Debugging

### Console Commands
```cpp
// Enable accessibility debug logging
UFUNCTION(Exec)
void DebugAccessibility(bool bEnabled);

// Test subtitle display
UFUNCTION(Exec)
void TestSubtitle(FString Text);

// Test visual audio cue
UFUNCTION(Exec)
void TestAudioCue(FString Type);

// Validate WCAG compliance
UFUNCTION(Exec)
void ValidateWCAG();
```

### Debug Visualization
```cpp
// Draw visual audio cue locations
void UVisualAudioCueWidget::DebugDrawCues()
{
    for (const FVisualCue& Cue : ActiveCues)
    {
        DrawDebugSphere(
            GetWorld(),
            ScreenToWorld(Cue.ScreenPosition),
            50.0f,
            12,
            Cue.Color.ToFColor(true),
            false,
            2.0f
        );
    }
}
```

## Best Practices

1. **Always provide subtitles** for all dialogue and important sounds
2. **Test with all color blind modes** to ensure UI remains usable
3. **Respect user settings** - never override accessibility preferences
4. **Provide visual alternatives** for all audio cues
5. **Support full keyboard navigation** in all menus
6. **Test with screen readers** if implementing screen reader support
7. **Maintain high contrast ratios** (minimum 4.5:1 for normal text)
8. **Allow sufficient time** for reading subtitles
9. **Avoid rapid flashing** (no more than 3 flashes per second)
10. **Document all accessibility features** in-game
