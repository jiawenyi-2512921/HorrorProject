// Copyright Epic Games, Inc. All Rights Reserved.

#include "AccessibilitySubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/GameUserSettings.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "JsonObjectConverter.h"

void UAccessibilitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    LoadAccessibilitySettings();
    ApplyAccessibilitySettings(CurrentSettings);

    UE_LOG(LogTemp, Log, TEXT("AccessibilitySubsystem initialized"));
}

void UAccessibilitySubsystem::Deinitialize()
{
    SaveAccessibilitySettings();
    Super::Deinitialize();
}

void UAccessibilitySubsystem::ApplyAccessibilitySettings(const FAccessibilitySettings& NewSettings)
{
    CurrentSettings = NewSettings;

    ApplyColorBlindShader();
    ApplyMotionSettings();
    ApplyUIScaling();

    OnAccessibilitySettingsChanged.Broadcast(CurrentSettings);

    UE_LOG(LogTemp, Log, TEXT("Accessibility settings applied"));
}

void UAccessibilitySubsystem::SaveAccessibilitySettings()
{
    FString JsonString;
    if (FJsonObjectConverter::UStructToJsonObjectString(CurrentSettings, JsonString))
    {
        FString SavePath = GetSettingsSavePath();
        if (FFileHelper::SaveStringToFile(JsonString, *SavePath))
        {
            UE_LOG(LogTemp, Log, TEXT("Accessibility settings saved to: %s"), *SavePath);
        }
    }
}

void UAccessibilitySubsystem::LoadAccessibilitySettings()
{
    FString SavePath = GetSettingsSavePath();
    FString JsonString;

    if (FFileHelper::LoadFileToString(JsonString, *SavePath))
    {
        FAccessibilitySettings LoadedSettings;
        if (FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &LoadedSettings))
        {
            CurrentSettings = LoadedSettings;
            UE_LOG(LogTemp, Log, TEXT("Accessibility settings loaded from: %s"), *SavePath);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No saved accessibility settings found, using defaults"));
    }
}

void UAccessibilitySubsystem::DisplaySubtitle(const FText& SubtitleText, float Duration, const FString& SpeakerName)
{
    if (!CurrentSettings.bSubtitlesEnabled)
    {
        return;
    }

    FText DisplayText = SubtitleText;
    if (!SpeakerName.IsEmpty())
    {
        DisplayText = FText::Format(FText::FromString("{0}: {1}"),
            FText::FromString(SpeakerName), SubtitleText);
    }

    OnSubtitleDisplayed.Broadcast(DisplayText, Duration);

    if (CurrentSettings.bScreenReaderEnabled)
    {
        AnnounceToScreenReader(DisplayText, false);
    }
}

void UAccessibilitySubsystem::ClearSubtitles()
{
    OnSubtitleDisplayed.Broadcast(FText::GetEmpty(), 0.0f);
}

void UAccessibilitySubsystem::SetColorBlindMode(EColorBlindMode Mode)
{
    CurrentSettings.ColorBlindMode = Mode;
    ApplyColorBlindShader();
    OnAccessibilitySettingsChanged.Broadcast(CurrentSettings);
}

FLinearColor UAccessibilitySubsystem::AdjustColorForColorBlindness(const FLinearColor& OriginalColor) const
{
    if (CurrentSettings.ColorBlindMode == EColorBlindMode::None)
    {
        return OriginalColor;
    }

    FLinearColor AdjustedColor = OriginalColor;

    switch (CurrentSettings.ColorBlindMode)
    {
        case EColorBlindMode::Protanopia: // Red-blind
            AdjustedColor.R = 0.567f * OriginalColor.R + 0.433f * OriginalColor.G;
            AdjustedColor.G = 0.558f * OriginalColor.R + 0.442f * OriginalColor.G;
            break;

        case EColorBlindMode::Deuteranopia: // Green-blind
            AdjustedColor.R = 0.625f * OriginalColor.R + 0.375f * OriginalColor.G;
            AdjustedColor.G = 0.7f * OriginalColor.R + 0.3f * OriginalColor.G;
            break;

        case EColorBlindMode::Tritanopia: // Blue-blind
            AdjustedColor.G = 0.95f * OriginalColor.G + 0.05f * OriginalColor.B;
            AdjustedColor.B = 0.433f * OriginalColor.G + 0.567f * OriginalColor.B;
            break;
    }

    return AdjustedColor;
}

void UAccessibilitySubsystem::TriggerVisualAudioCue(const FVector& Location, const FString& CueType, float Intensity)
{
    if (!CurrentSettings.bVisualAudioCuesEnabled)
    {
        return;
    }

    OnVisualAudioCue.Broadcast(Location, CueType, Intensity);
}

void UAccessibilitySubsystem::SetMotionBlurEnabled(bool bEnabled)
{
    CurrentSettings.bMotionBlurEnabled = bEnabled;
    ApplyMotionSettings();
    OnAccessibilitySettingsChanged.Broadcast(CurrentSettings);
}

void UAccessibilitySubsystem::SetCameraShakeIntensity(float Intensity)
{
    CurrentSettings.CameraShakeIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    OnAccessibilitySettingsChanged.Broadcast(CurrentSettings);
}

void UAccessibilitySubsystem::AnnounceToScreenReader(const FText& Message, bool bInterrupt)
{
    if (!CurrentSettings.bScreenReaderEnabled)
    {
        return;
    }

    // Platform-specific screen reader implementation would go here
    UE_LOG(LogTemp, Log, TEXT("Screen Reader: %s"), *Message.ToString());
}

void UAccessibilitySubsystem::SetUIScale(float Scale)
{
    CurrentSettings.UIScale = FMath::Clamp(Scale, 0.5f, 2.0f);
    ApplyUIScaling();
    OnAccessibilitySettingsChanged.Broadcast(CurrentSettings);
}

bool UAccessibilitySubsystem::ShouldShowFlashWarning() const
{
    return CurrentSettings.bFlashWarningsEnabled;
}

void UAccessibilitySubsystem::TriggerFlashWarning(const FText& WarningMessage)
{
    if (ShouldShowFlashWarning())
    {
        DisplaySubtitle(WarningMessage, 5.0f, TEXT("Warning"));
        AnnounceToScreenReader(WarningMessage, true);
    }
}

bool UAccessibilitySubsystem::ValidateWCAGCompliance() const
{
    // WCAG 2.1 AA Compliance Checks
    bool bCompliant = true;

    // Check contrast ratios (minimum 4.5:1 for normal text)
    float SubtitleLuminance = CurrentSettings.SubtitleColor.GetLuminance();
    float BackgroundLuminance = CurrentSettings.SubtitleBackgroundColor.GetLuminance();
    float ContrastRatio = (FMath::Max(SubtitleLuminance, BackgroundLuminance) + 0.05f) /
                          (FMath::Min(SubtitleLuminance, BackgroundLuminance) + 0.05f);

    if (ContrastRatio < 4.5f)
    {
        UE_LOG(LogTemp, Warning, TEXT("WCAG Compliance: Subtitle contrast ratio too low: %f"), ContrastRatio);
        bCompliant = false;
    }

    // Check UI scale is within reasonable bounds
    if (CurrentSettings.UIScale < 0.5f || CurrentSettings.UIScale > 2.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("WCAG Compliance: UI scale out of bounds"));
        bCompliant = false;
    }

    return bCompliant;
}

void UAccessibilitySubsystem::ApplyColorBlindShader()
{
    UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
    if (Settings)
    {
        // Apply color blind mode through post-process settings
        int32 ColorBlindMode = static_cast<int32>(CurrentSettings.ColorBlindMode);
        Settings->SetColorBlindMode(ColorBlindMode);
        Settings->ApplySettings(false);
    }
}

void UAccessibilitySubsystem::ApplyMotionSettings()
{
    UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
    if (Settings)
    {
        Settings->SetMotionBlurQuality(CurrentSettings.bMotionBlurEnabled ? 3 : 0);
        Settings->ApplySettings(false);
    }
}

void UAccessibilitySubsystem::ApplyUIScaling()
{
    // UI scaling would be applied through UMG widgets
    UE_LOG(LogTemp, Log, TEXT("UI Scale applied: %f"), CurrentSettings.UIScale);
}

FString UAccessibilitySubsystem::GetSettingsSavePath() const
{
    return FPaths::ProjectSavedDir() / TEXT("Config") / TEXT("AccessibilitySettings.json");
}
