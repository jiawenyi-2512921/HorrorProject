// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../AccessibilitySubsystem.h"
#include "AccessibilitySettingsWidget.generated.h"

class UButton;
class USlider;
class UComboBoxString;
class UCheckBox;
class UTextBlock;

/**
 * Accessibility Settings Widget
 * Provides UI for configuring all accessibility options
 * WCAG 2.1 AA compliant with keyboard navigation support
 */
UCLASS()
class HORRORPROJECT_API UAccessibilitySettingsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    UFUNCTION(BlueprintCallable, Category = "Accessibility")
    void LoadCurrentSettings();

    UFUNCTION(BlueprintCallable, Category = "Accessibility")
    void ApplySettings();

    UFUNCTION(BlueprintCallable, Category = "Accessibility")
    void ResetToDefaults();

protected:
    // Subtitle Settings
    UPROPERTY(meta = (BindWidget))
    UCheckBox* SubtitlesEnabledCheckBox;

    UPROPERTY(meta = (BindWidget))
    UComboBoxString* SubtitleSizeComboBox;

    UPROPERTY(meta = (BindWidget))
    UButton* SubtitleColorButton;

    UPROPERTY(meta = (BindWidget))
    UButton* SubtitleBackgroundButton;

    // Color Blind Settings
    UPROPERTY(meta = (BindWidget))
    UComboBoxString* ColorBlindModeComboBox;

    // Motion Settings
    UPROPERTY(meta = (BindWidget))
    UCheckBox* MotionBlurCheckBox;

    UPROPERTY(meta = (BindWidget))
    USlider* CameraShakeSlider;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* CameraShakeValueText;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* ReduceFlashingCheckBox;

    // Audio Settings
    UPROPERTY(meta = (BindWidget))
    UCheckBox* VisualAudioCuesCheckBox;

    // UI Settings
    UPROPERTY(meta = (BindWidget))
    USlider* UIScaleSlider;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* UIScaleValueText;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* HighContrastCheckBox;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* ScreenReaderCheckBox;

    // Buttons
    UPROPERTY(meta = (BindWidget))
    UButton* ApplyButton;

    UPROPERTY(meta = (BindWidget))
    UButton* ResetButton;

    UPROPERTY(meta = (BindWidget))
    UButton* CloseButton;

private:
    UFUNCTION()
    void OnSubtitlesEnabledChanged(bool bIsChecked);

    UFUNCTION()
    void OnSubtitleSizeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void OnColorBlindModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void OnMotionBlurChanged(bool bIsChecked);

    UFUNCTION()
    void OnCameraShakeChanged(float Value);

    UFUNCTION()
    void OnVisualAudioCuesChanged(bool bIsChecked);

    UFUNCTION()
    void OnUIScaleChanged(float Value);

    UFUNCTION()
    void OnHighContrastChanged(bool bIsChecked);

    UFUNCTION()
    void OnApplyClicked();

    UFUNCTION()
    void OnResetClicked();

    UFUNCTION()
    void OnCloseClicked();

    UPROPERTY()
    FAccessibilitySettings CurrentSettings;

    UPROPERTY()
    UAccessibilitySubsystem* AccessibilitySubsystem;

    void InitializeComboBoxes();
    void UpdateValueDisplays();
    void AnnounceChange(const FText& Message);
};
