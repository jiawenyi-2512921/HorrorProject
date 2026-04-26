# Code Examples: Accessibility System

Complete code examples for implementing accessibility features.

---

## Example 1: Subtitle System

```cpp
// SubtitleManager.h
#pragma once
#include "CoreMinimal.h"
#include "Accessibility/AccessibilitySubsystem.h"
#include "SubtitleManager.generated.h"

UCLASS()
class HORRORPROJECT_API USubtitleManager : public UObject
{
    GENERATED_BODY()

public:
    static void ShowDialogue(UWorld* World, const FText& Text, const FString& Speaker, float Duration);
    static void ShowEnvironmentalSound(UWorld* World, const FText& SoundDescription, float Duration);
};

// SubtitleManager.cpp
#include "SubtitleManager.h"

void USubtitleManager::ShowDialogue(UWorld* World, const FText& Text, const FString& Speaker, float Duration)
{
    UAccessibilitySubsystem* Accessibility = 
        World->GetGameInstance()->GetSubsystem<UAccessibilitySubsystem>();
    
    if (Accessibility)
    {
        Accessibility->DisplaySubtitle(Text, Duration, Speaker);
    }
}

void USubtitleManager::ShowEnvironmentalSound(UWorld* World, const FText& SoundDescription, float Duration)
{
    UAccessibilitySubsystem* Accessibility = 
        World->GetGameInstance()->GetSubsystem<UAccessibilitySubsystem>();
    
    if (Accessibility)
    {
        Accessibility->DisplaySubtitle(
            SoundDescription, 
            Duration, 
            TEXT("") // No speaker for environmental sounds
        );
    }
}
```

---

## Example 2: Visual Audio Cues

```cpp
// AudioCueManager.h
#pragma once
#include "CoreMinimal.h"
#include "Accessibility/AccessibilitySubsystem.h"
#include "AudioCueManager.generated.h"

UCLASS()
class HORRORPROJECT_API UAudioCueManager : public UObject
{
    GENERATED_BODY()

public:
    static void TriggerFootstepCue(UWorld* World, FVector Location, float Intensity);
    static void TriggerDangerCue(UWorld* World, FVector Location, float Intensity);
    static void TriggerVoiceCue(UWorld* World, FVector Location, float Intensity);
    static void TriggerDoorCue(UWorld* World, FVector Location, float Intensity);
};

// AudioCueManager.cpp
#include "AudioCueManager.h"

void UAudioCueManager::TriggerFootstepCue(UWorld* World, FVector Location, float Intensity)
{
    UAccessibilitySubsystem* Accessibility = 
        World->GetGameInstance()->GetSubsystem<UAccessibilitySubsystem>();
    
    if (Accessibility)
    {
        Accessibility->TriggerVisualAudioCue(Location, TEXT("Footstep"), Intensity);
    }
}

void UAudioCueManager::TriggerDangerCue(UWorld* World, FVector Location, float Intensity)
{
    UAccessibilitySubsystem* Accessibility = 
        World->GetGameInstance()->GetSubsystem<UAccessibilitySubsystem>();
    
    if (Accessibility)
    {
        Accessibility->TriggerVisualAudioCue(Location, TEXT("Danger"), Intensity);
    }
}

void UAudioCueManager::TriggerVoiceCue(UWorld* World, FVector Location, float Intensity)
{
    UAccessibilitySubsystem* Accessibility = 
        World->GetGameInstance()->GetSubsystem<UAccessibilitySubsystem>();
    
    if (Accessibility)
    {
        Accessibility->TriggerVisualAudioCue(Location, TEXT("Voice"), Intensity);
    }
}

void UAudioCueManager::TriggerDoorCue(UWorld* World, FVector Location, float Intensity)
{
    UAccessibilitySubsystem* Accessibility = 
        World->GetGameInstance()->GetSubsystem<UAccessibilitySubsystem>();
    
    if (Accessibility)
    {
        Accessibility->TriggerVisualAudioCue(Location, TEXT("Door"), Intensity);
    }
}
```

---

## Example 3: Accessibility Settings UI

```cpp
// AccessibilitySettingsWidget.h
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Accessibility/AccessibilitySubsystem.h"
#include "AccessibilitySettingsWidget.generated.h"

UCLASS()
class HORRORPROJECT_API UAccessibilitySettingsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Accessibility")
    void LoadSettings();
    
    UFUNCTION(BlueprintCallable, Category = "Accessibility")
    void SaveSettings();
    
    UFUNCTION(BlueprintCallable, Category = "Accessibility")
    void SetSubtitlesEnabled(bool bEnabled);
    
    UFUNCTION(BlueprintCallable, Category = "Accessibility")
    void SetSubtitleSize(ESubtitleSize Size);
    
    UFUNCTION(BlueprintCallable, Category = "Accessibility")
    void SetColorBlindMode(EColorBlindMode Mode);
    
    UFUNCTION(BlueprintCallable, Category = "Accessibility")
    void SetUIScale(float Scale);

private:
    UAccessibilitySubsystem* AccessibilitySubsystem;
    FAccessibilitySettings CurrentSettings;
};

// AccessibilitySettingsWidget.cpp
#include "AccessibilitySettingsWidget.h"

void UAccessibilitySettingsWidget::LoadSettings()
{
    AccessibilitySubsystem = GetGameInstance()->GetSubsystem<UAccessibilitySubsystem>();
    
    if (AccessibilitySubsystem)
    {
        AccessibilitySubsystem->LoadAccessibilitySettings();
        CurrentSettings = AccessibilitySubsystem->GetAccessibilitySettings();
    }
}

void UAccessibilitySettingsWidget::SaveSettings()
{
    if (AccessibilitySubsystem)
    {
        AccessibilitySubsystem->ApplyAccessibilitySettings(CurrentSettings);
        AccessibilitySubsystem->SaveAccessibilitySettings();
    }
}

void UAccessibilitySettingsWidget::SetSubtitlesEnabled(bool bEnabled)
{
    CurrentSettings.bSubtitlesEnabled = bEnabled;
}

void UAccessibilitySettingsWidget::SetSubtitleSize(ESubtitleSize Size)
{
    CurrentSettings.SubtitleSize = Size;
}

void UAccessibilitySettingsWidget::SetColorBlindMode(EColorBlindMode Mode)
{
    if (AccessibilitySubsystem)
    {
        AccessibilitySubsystem->SetColorBlindMode(Mode);
    }
}

void UAccessibilitySettingsWidget::SetUIScale(float Scale)
{
    CurrentSettings.UIScale = FMath::Clamp(Scale, 0.5f, 2.0f);
}
```

---

## Example 4: Audio Integration

```cpp
// AccessibleAudioComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "Accessibility/AccessibilitySubsystem.h"
#include "AccessibleAudioComponent.generated.h"

UCLASS()
class HORRORPROJECT_API UAccessibleAudioComponent : public UAudioComponent
{
    GENERATED_BODY()

public:
    virtual void Play(float StartTime = 0.0f) override;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    FText SubtitleText;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    FString SpeakerName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    FString AudioCueType;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    bool bShowSubtitle;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    bool bShowVisualCue;
};

// AccessibleAudioComponent.cpp
#include "AccessibleAudioComponent.h"

void UAccessibleAudioComponent::Play(float StartTime)
{
    Super::Play(StartTime);
    
    UAccessibilitySubsystem* Accessibility = 
        GetWorld()->GetGameInstance()->GetSubsystem<UAccessibilitySubsystem>();
    
    if (Accessibility)
    {
        // Show subtitle
        if (bShowSubtitle && !SubtitleText.IsEmpty())
        {
            float Duration = Sound ? Sound->GetDuration() : 3.0f;
            Accessibility->DisplaySubtitle(SubtitleText, Duration, SpeakerName);
        }
        
        // Show visual cue
        if (bShowVisualCue && !AudioCueType.IsEmpty())
        {
            FVector Location = GetComponentLocation();
            float Intensity = VolumeMultiplier;
            Accessibility->TriggerVisualAudioCue(Location, AudioCueType, Intensity);
        }
    }
}
```

---

## Example 5: Complete Accessibility Setup

```cpp
// AccessibilityManager.h
#pragma once
#include "CoreMinimal.h"
#include "Accessibility/AccessibilitySubsystem.h"
#include "AccessibilityManager.generated.h"

UCLASS()
class HORRORPROJECT_API UAccessibilityManager : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(UWorld* World);
    void ApplyRecommendedSettings();
    void ApplyMinimalMotionSettings();
    void ApplyHighContrastSettings();
    void ApplyColorBlindSettings(EColorBlindMode Mode);
};

// AccessibilityManager.cpp
#include "AccessibilityManager.h"

void UAccessibilityManager::Initialize(UWorld* World)
{
    UAccessibilitySubsystem* Accessibility = 
        World->GetGameInstance()->GetSubsystem<UAccessibilitySubsystem>();
    
    if (Accessibility)
    {
        Accessibility->LoadAccessibilitySettings();
    }
}

void UAccessibilityManager::ApplyRecommendedSettings()
{
    UAccessibilitySubsystem* Accessibility = 
        GetWorld()->GetGameInstance()->GetSubsystem<UAccessibilitySubsystem>();
    
    if (Accessibility)
    {
        FAccessibilitySettings Settings;
        Settings.bSubtitlesEnabled = true;
        Settings.SubtitleSize = ESubtitleSize::Medium;
        Settings.bVisualAudioCuesEnabled = true;
        Settings.bMotionBlurEnabled = true;
        Settings.CameraShakeIntensity = 1.0f;
        Settings.UIScale = 1.0f;
        
        Accessibility->ApplyAccessibilitySettings(Settings);
    }
}

void UAccessibilityManager::ApplyMinimalMotionSettings()
{
    UAccessibilitySubsystem* Accessibility = 
        GetWorld()->GetGameInstance()->GetSubsystem<UAccessibilitySubsystem>();
    
    if (Accessibility)
    {
        FAccessibilitySettings Settings = Accessibility->GetAccessibilitySettings();
        Settings.bMotionBlurEnabled = false;
        Settings.CameraShakeIntensity = 0.2f;
        Settings.bReduceFlashingLights = true;
        
        Accessibility->ApplyAccessibilitySettings(Settings);
    }
}

void UAccessibilityManager::ApplyHighContrastSettings()
{
    UAccessibilitySubsystem* Accessibility = 
        GetWorld()->GetGameInstance()->GetSubsystem<UAccessibilitySubsystem>();
    
    if (Accessibility)
    {
        FAccessibilitySettings Settings = Accessibility->GetAccessibilitySettings();
        Settings.bHighContrastMode = true;
        Settings.UIScale = 1.2f;
        
        Accessibility->ApplyAccessibilitySettings(Settings);
    }
}

void UAccessibilityManager::ApplyColorBlindSettings(EColorBlindMode Mode)
{
    UAccessibilitySubsystem* Accessibility = 
        GetWorld()->GetGameInstance()->GetSubsystem<UAccessibilitySubsystem>();
    
    if (Accessibility)
    {
        Accessibility->SetColorBlindMode(Mode);
        Accessibility->SetColorBlindSeverity(0.8f);
    }
}
```

---

## Blueprint Examples

### Display Subtitle
```
On Dialogue Start
  → Get Accessibility Subsystem
  → Display Subtitle (Text: "Hello", Duration: 3.0, Speaker: "NPC")
```

### Trigger Visual Cue
```
On Play Sound
  → Get Accessibility Subsystem
  → Trigger Visual Audio Cue (Location, Type: "Footstep", Intensity: 1.0)
```

### Apply Settings
```
On Settings Changed
  → Get Accessibility Subsystem
  → Make Accessibility Settings
    → Subtitles Enabled: true
    → Subtitle Size: Large
    → Visual Cues: true
    → Motion Blur: false
  → Apply Accessibility Settings
  → Save Accessibility Settings
```

### Color Blind Mode
```
On Color Blind Dropdown Changed
  → Get Accessibility Subsystem
  → Set Color Blind Mode (Selected Mode)
  → Set Color Blind Severity (0.8)
```
