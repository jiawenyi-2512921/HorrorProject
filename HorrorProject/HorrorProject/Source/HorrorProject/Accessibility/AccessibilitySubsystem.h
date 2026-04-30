// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AccessibilitySubsystem.generated.h"

UENUM(BlueprintType, meta=(ScriptName="ColorBlindModeEnum"))
enum class EColorBlindMode : uint8
{
    None UMETA(DisplayName = "无"),
    Protanopia UMETA(DisplayName = "红色弱辅助"),
    Deuteranopia UMETA(DisplayName = "绿色弱辅助"),
    Tritanopia UMETA(DisplayName = "蓝色弱辅助")
};

UENUM(BlueprintType)
enum class ESubtitleSize : uint8
{
    Small UMETA(DisplayName = "小"),
    Medium UMETA(DisplayName = "中"),
    Large UMETA(DisplayName = "大"),
    ExtraLarge UMETA(DisplayName = "特大")
};

USTRUCT(BlueprintType)
struct FAccessibilitySettings
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Accessibility")
    bool bSubtitlesEnabled = true;

    UPROPERTY(BlueprintReadWrite, Category = "Accessibility")
    ESubtitleSize SubtitleSize = ESubtitleSize::Medium;

    UPROPERTY(BlueprintReadWrite, Category = "Accessibility")
    FLinearColor SubtitleColor = FLinearColor::White;

    UPROPERTY(BlueprintReadWrite, Category = "Accessibility")
    FLinearColor SubtitleBackgroundColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.7f);

    UPROPERTY(BlueprintReadWrite, Category = "Accessibility")
    EColorBlindMode ColorBlindMode = EColorBlindMode::None;

    UPROPERTY(BlueprintReadWrite, Category = "Accessibility")
    bool bMotionBlurEnabled = true;

    UPROPERTY(BlueprintReadWrite, Category = "Accessibility")
    float CameraShakeIntensity = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Accessibility")
    bool bFlashWarningsEnabled = true;

    UPROPERTY(BlueprintReadWrite, Category = "Accessibility")
    bool bVisualAudioCuesEnabled = false;

    UPROPERTY(BlueprintReadWrite, Category = "Accessibility")
    bool bScreenReaderEnabled = false;

    UPROPERTY(BlueprintReadWrite, Category = "Accessibility")
    float UIScale = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Accessibility")
    bool bHighContrastMode = false;

    UPROPERTY(BlueprintReadWrite, Category = "Accessibility")
    bool bReduceFlashingLights = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAccessibilitySettingsChanged, const FAccessibilitySettings&, NewSettings);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSubtitleDisplayed, const FText&, SubtitleText, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVisualAudioCue, const FVector&, Location, const FString&, CueType, float, Intensity);

/**
 * Accessibility Subsystem
 * Manages all accessibility features including subtitles, color blind modes, and visual audio cues
 * Compliant with WCAG 2.1 AA standards
 */
UCLASS()
class HORRORPROJECT_API UAccessibilitySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Settings Management
    UFUNCTION(BlueprintCallable, Category = "Accessibility")
    void ApplyAccessibilitySettings(const FAccessibilitySettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "Accessibility")
    FAccessibilitySettings GetAccessibilitySettings() const { return CurrentSettings; }

    UFUNCTION(BlueprintCallable, Category = "Accessibility")
    void SaveAccessibilitySettings();

    UFUNCTION(BlueprintCallable, Category = "Accessibility")
    void LoadAccessibilitySettings();

    // Subtitle System
    UFUNCTION(BlueprintCallable, Category = "Accessibility|Subtitles")
    void DisplaySubtitle(const FText& SubtitleText, float Duration = 3.0f, const FString& SpeakerName = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Accessibility|Subtitles")
    void ClearSubtitles();

    // Color Blind Mode
    UFUNCTION(BlueprintCallable, Category = "Accessibility|ColorBlind")
    void SetColorBlindMode(EColorBlindMode Mode);

    UFUNCTION(BlueprintPure, Category = "Accessibility|ColorBlind")
    EColorBlindMode GetColorBlindMode() const { return CurrentSettings.ColorBlindMode; }

    UFUNCTION(BlueprintPure, Category = "Accessibility|ColorBlind")
    FLinearColor AdjustColorForColorBlindness(const FLinearColor& OriginalColor) const;

    // Visual Audio Cues
    UFUNCTION(BlueprintCallable, Category = "Accessibility|AudioCues")
    void TriggerVisualAudioCue(const FVector& Location, const FString& CueType, float Intensity = 1.0f);

    // Motion and Camera
    UFUNCTION(BlueprintCallable, Category = "Accessibility|Motion")
    void SetMotionBlurEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Accessibility|Motion")
    void SetCameraShakeIntensity(float Intensity);

    UFUNCTION(BlueprintPure, Category = "Accessibility|Motion")
    float GetAdjustedCameraShakeIntensity() const { return CurrentSettings.CameraShakeIntensity; }

    // Screen Reader Support
    UFUNCTION(BlueprintCallable, Category = "Accessibility|ScreenReader")
    void AnnounceToScreenReader(const FText& Message, bool bInterrupt = false);

    // UI Accessibility
    UFUNCTION(BlueprintPure, Category = "Accessibility|UI")
    float GetUIScale() const { return CurrentSettings.UIScale; }

    UFUNCTION(BlueprintCallable, Category = "Accessibility|UI")
    void SetUIScale(float Scale);

    UFUNCTION(BlueprintPure, Category = "Accessibility|UI")
    bool IsHighContrastMode() const { return CurrentSettings.bHighContrastMode; }

    // Flash Warnings
    UFUNCTION(BlueprintCallable, Category = "Accessibility|Flash")
    bool ShouldShowFlashWarning() const;

    UFUNCTION(BlueprintCallable, Category = "Accessibility|Flash")
    void TriggerFlashWarning(const FText& WarningMessage);

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Accessibility")
    FOnAccessibilitySettingsChanged OnAccessibilitySettingsChanged;

    UPROPERTY(BlueprintAssignable, Category = "Accessibility")
    FOnSubtitleDisplayed OnSubtitleDisplayed;

    UPROPERTY(BlueprintAssignable, Category = "Accessibility")
    FOnVisualAudioCue OnVisualAudioCue;

    // Validation
    UFUNCTION(BlueprintPure, Category = "Accessibility")
    bool ValidateWCAGCompliance() const;

private:
    UPROPERTY()
    FAccessibilitySettings CurrentSettings;

    void ApplyColorBlindShader();
    void ApplyMotionSettings();
    void ApplyUIScaling();

    FString GetSettingsSavePath() const;
};
