// Copyright Epic Games, Inc. All Rights Reserved.

#include "AccessibilitySettingsWidget.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/ComboBoxString.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UAccessibilitySettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
    if (GameInstance)
    {
        AccessibilitySubsystem = GameInstance->GetSubsystem<UAccessibilitySubsystem>();
    }

    InitializeComboBoxes();

    // Bind events
    if (SubtitlesEnabledCheckBox)
        SubtitlesEnabledCheckBox->OnCheckStateChanged.AddDynamic(this, &UAccessibilitySettingsWidget::OnSubtitlesEnabledChanged);

    if (SubtitleSizeComboBox)
        SubtitleSizeComboBox->OnSelectionChanged.AddDynamic(this, &UAccessibilitySettingsWidget::OnSubtitleSizeChanged);

    if (ColorBlindModeComboBox)
        ColorBlindModeComboBox->OnSelectionChanged.AddDynamic(this, &UAccessibilitySettingsWidget::OnColorBlindModeChanged);

    if (MotionBlurCheckBox)
        MotionBlurCheckBox->OnCheckStateChanged.AddDynamic(this, &UAccessibilitySettingsWidget::OnMotionBlurChanged);

    if (CameraShakeSlider)
        CameraShakeSlider->OnValueChanged.AddDynamic(this, &UAccessibilitySettingsWidget::OnCameraShakeChanged);

    if (VisualAudioCuesCheckBox)
        VisualAudioCuesCheckBox->OnCheckStateChanged.AddDynamic(this, &UAccessibilitySettingsWidget::OnVisualAudioCuesChanged);

    if (UIScaleSlider)
        UIScaleSlider->OnValueChanged.AddDynamic(this, &UAccessibilitySettingsWidget::OnUIScaleChanged);

    if (HighContrastCheckBox)
        HighContrastCheckBox->OnCheckStateChanged.AddDynamic(this, &UAccessibilitySettingsWidget::OnHighContrastChanged);

    if (ApplyButton)
        ApplyButton->OnClicked.AddDynamic(this, &UAccessibilitySettingsWidget::OnApplyClicked);

    if (ResetButton)
        ResetButton->OnClicked.AddDynamic(this, &UAccessibilitySettingsWidget::OnResetClicked);

    if (CloseButton)
        CloseButton->OnClicked.AddDynamic(this, &UAccessibilitySettingsWidget::OnCloseClicked);

    LoadCurrentSettings();
}

FReply UAccessibilitySettingsWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    // Keyboard navigation support
    if (InKeyEvent.GetKey() == EKeys::Escape)
    {
        OnCloseClicked();
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UAccessibilitySettingsWidget::LoadCurrentSettings()
{
    if (!AccessibilitySubsystem)
        return;

    CurrentSettings = AccessibilitySubsystem->GetAccessibilitySettings();

    if (SubtitlesEnabledCheckBox)
        SubtitlesEnabledCheckBox->SetIsChecked(CurrentSettings.bSubtitlesEnabled);

    if (SubtitleSizeComboBox)
    {
        int32 SizeIndex = static_cast<int32>(CurrentSettings.SubtitleSize);
        SubtitleSizeComboBox->SetSelectedIndex(SizeIndex);
    }

    if (ColorBlindModeComboBox)
    {
        int32 ModeIndex = static_cast<int32>(CurrentSettings.ColorBlindMode);
        ColorBlindModeComboBox->SetSelectedIndex(ModeIndex);
    }

    if (MotionBlurCheckBox)
        MotionBlurCheckBox->SetIsChecked(CurrentSettings.bMotionBlurEnabled);

    if (CameraShakeSlider)
        CameraShakeSlider->SetValue(CurrentSettings.CameraShakeIntensity);

    if (VisualAudioCuesCheckBox)
        VisualAudioCuesCheckBox->SetIsChecked(CurrentSettings.bVisualAudioCuesEnabled);

    if (UIScaleSlider)
        UIScaleSlider->SetValue(CurrentSettings.UIScale);

    if (HighContrastCheckBox)
        HighContrastCheckBox->SetIsChecked(CurrentSettings.bHighContrastMode);

    if (ScreenReaderCheckBox)
        ScreenReaderCheckBox->SetIsChecked(CurrentSettings.bScreenReaderEnabled);

    if (ReduceFlashingCheckBox)
        ReduceFlashingCheckBox->SetIsChecked(CurrentSettings.bReduceFlashingLights);

    UpdateValueDisplays();
}

void UAccessibilitySettingsWidget::ApplySettings()
{
    if (AccessibilitySubsystem)
    {
        AccessibilitySubsystem->ApplyAccessibilitySettings(CurrentSettings);
        AccessibilitySubsystem->SaveAccessibilitySettings();
        AnnounceChange(FText::FromString(TEXT("Settings applied successfully")));
    }
}

void UAccessibilitySettingsWidget::ResetToDefaults()
{
    CurrentSettings = FAccessibilitySettings();
    LoadCurrentSettings();
    AnnounceChange(FText::FromString(TEXT("Settings reset to defaults")));
}

void UAccessibilitySettingsWidget::OnSubtitlesEnabledChanged(bool bIsChecked)
{
    CurrentSettings.bSubtitlesEnabled = bIsChecked;
    AnnounceChange(FText::FromString(bIsChecked ? TEXT("Subtitles enabled") : TEXT("Subtitles disabled")));
}

void UAccessibilitySettingsWidget::OnSubtitleSizeChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (SubtitleSizeComboBox)
    {
        int32 Index = SubtitleSizeComboBox->GetSelectedIndex();
        CurrentSettings.SubtitleSize = static_cast<ESubtitleSize>(Index);
        AnnounceChange(FText::Format(FText::FromString(TEXT("Subtitle size changed to {0}")), FText::FromString(SelectedItem)));
    }
}

void UAccessibilitySettingsWidget::OnColorBlindModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (ColorBlindModeComboBox)
    {
        int32 Index = ColorBlindModeComboBox->GetSelectedIndex();
        CurrentSettings.ColorBlindMode = static_cast<EColorBlindMode>(Index);
        AnnounceChange(FText::Format(FText::FromString(TEXT("Color blind mode changed to {0}")), FText::FromString(SelectedItem)));
    }
}

void UAccessibilitySettingsWidget::OnMotionBlurChanged(bool bIsChecked)
{
    CurrentSettings.bMotionBlurEnabled = bIsChecked;
    AnnounceChange(FText::FromString(bIsChecked ? TEXT("Motion blur enabled") : TEXT("Motion blur disabled")));
}

void UAccessibilitySettingsWidget::OnCameraShakeChanged(float Value)
{
    CurrentSettings.CameraShakeIntensity = Value;
    UpdateValueDisplays();
    AnnounceChange(FText::Format(FText::FromString(TEXT("Camera shake intensity set to {0}%")),
        FText::AsNumber(FMath::RoundToInt(Value * 100))));
}

void UAccessibilitySettingsWidget::OnVisualAudioCuesChanged(bool bIsChecked)
{
    CurrentSettings.bVisualAudioCuesEnabled = bIsChecked;
    AnnounceChange(FText::FromString(bIsChecked ? TEXT("Visual audio cues enabled") : TEXT("Visual audio cues disabled")));
}

void UAccessibilitySettingsWidget::OnUIScaleChanged(float Value)
{
    CurrentSettings.UIScale = Value;
    UpdateValueDisplays();
    AnnounceChange(FText::Format(FText::FromString(TEXT("UI scale set to {0}%")),
        FText::AsNumber(FMath::RoundToInt(Value * 100))));
}

void UAccessibilitySettingsWidget::OnHighContrastChanged(bool bIsChecked)
{
    CurrentSettings.bHighContrastMode = bIsChecked;
    AnnounceChange(FText::FromString(bIsChecked ? TEXT("High contrast mode enabled") : TEXT("High contrast mode disabled")));
}

void UAccessibilitySettingsWidget::OnApplyClicked()
{
    ApplySettings();
}

void UAccessibilitySettingsWidget::OnResetClicked()
{
    ResetToDefaults();
}

void UAccessibilitySettingsWidget::OnCloseClicked()
{
    RemoveFromParent();
}

void UAccessibilitySettingsWidget::InitializeComboBoxes()
{
    if (SubtitleSizeComboBox)
    {
        SubtitleSizeComboBox->ClearOptions();
        SubtitleSizeComboBox->AddOption(TEXT("Small"));
        SubtitleSizeComboBox->AddOption(TEXT("Medium"));
        SubtitleSizeComboBox->AddOption(TEXT("Large"));
        SubtitleSizeComboBox->AddOption(TEXT("Extra Large"));
    }

    if (ColorBlindModeComboBox)
    {
        ColorBlindModeComboBox->ClearOptions();
        ColorBlindModeComboBox->AddOption(TEXT("None"));
        ColorBlindModeComboBox->AddOption(TEXT("Protanopia (Red-Blind)"));
        ColorBlindModeComboBox->AddOption(TEXT("Deuteranopia (Green-Blind)"));
        ColorBlindModeComboBox->AddOption(TEXT("Tritanopia (Blue-Blind)"));
    }
}

void UAccessibilitySettingsWidget::UpdateValueDisplays()
{
    if (CameraShakeValueText)
    {
        FString ValueText = FString::Printf(TEXT("%d%%"), FMath::RoundToInt(CurrentSettings.CameraShakeIntensity * 100));
        CameraShakeValueText->SetText(FText::FromString(ValueText));
    }

    if (UIScaleValueText)
    {
        FString ValueText = FString::Printf(TEXT("%d%%"), FMath::RoundToInt(CurrentSettings.UIScale * 100));
        UIScaleValueText->SetText(FText::FromString(ValueText));
    }
}

void UAccessibilitySettingsWidget::AnnounceChange(const FText& Message)
{
    if (AccessibilitySubsystem && CurrentSettings.bScreenReaderEnabled)
    {
        AccessibilitySubsystem->AnnounceToScreenReader(Message, false);
    }
}
