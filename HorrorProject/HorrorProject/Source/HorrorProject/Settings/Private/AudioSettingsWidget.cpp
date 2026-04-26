// Copyright Epic Games, Inc. All Rights Reserved.

#include "AudioSettingsWidget.h"
#include "HorrorAudioSettings.h"
#include "GameSettingsSubsystem.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Kismet/GameplayStatics.h"

void UHorrorAudioSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Get audio settings
	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
	{
		if (UGameSettingsSubsystem* SettingsSubsystem = GameInstance->GetSubsystem<UGameSettingsSubsystem>())
		{
			AudioSettings = SettingsSubsystem->GetAudioSettings();
		}
	}

	// Populate combo boxes
	PopulateOutputDevices();

	// Bind events
	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->OnValueChanged.AddDynamic(this, &UHorrorAudioSettingsWidget::OnMasterVolumeChanged);
	}

	if (MusicVolumeSlider)
	{
		MusicVolumeSlider->OnValueChanged.AddDynamic(this, &UHorrorAudioSettingsWidget::OnMusicVolumeChanged);
	}

	if (SFXVolumeSlider)
	{
		SFXVolumeSlider->OnValueChanged.AddDynamic(this, &UHorrorAudioSettingsWidget::OnSFXVolumeChanged);
	}

	if (DialogueVolumeSlider)
	{
		DialogueVolumeSlider->OnValueChanged.AddDynamic(this, &UHorrorAudioSettingsWidget::OnDialogueVolumeChanged);
	}

	if (AmbienceVolumeSlider)
	{
		AmbienceVolumeSlider->OnValueChanged.AddDynamic(this, &UHorrorAudioSettingsWidget::OnAmbienceVolumeChanged);
	}

	if (ReverbCheckBox)
	{
		ReverbCheckBox->OnCheckStateChanged.AddDynamic(this, &UHorrorAudioSettingsWidget::OnReverbChanged);
	}

	if (OcclusionCheckBox)
	{
		OcclusionCheckBox->OnCheckStateChanged.AddDynamic(this, &UHorrorAudioSettingsWidget::OnOcclusionChanged);
	}

	if (SpatialAudioCheckBox)
	{
		SpatialAudioCheckBox->OnCheckStateChanged.AddDynamic(this, &UHorrorAudioSettingsWidget::OnSpatialAudioChanged);
	}

	if (OutputDeviceComboBox)
	{
		OutputDeviceComboBox->OnSelectionChanged.AddDynamic(this, &UHorrorAudioSettingsWidget::OnOutputDeviceChanged);
	}

	if (SubtitlesCheckBox)
	{
		SubtitlesCheckBox->OnCheckStateChanged.AddDynamic(this, &UHorrorAudioSettingsWidget::OnSubtitlesChanged);
	}

	RefreshSettings();
}

void UHorrorAudioSettingsWidget::RefreshSettings()
{
	if (!AudioSettings)
	{
		return;
	}

	// Update volume sliders
	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->SetValue(AudioSettings->MasterVolume);
	}

	if (MusicVolumeSlider)
	{
		MusicVolumeSlider->SetValue(AudioSettings->MusicVolume);
	}

	if (SFXVolumeSlider)
	{
		SFXVolumeSlider->SetValue(AudioSettings->SFXVolume);
	}

	if (DialogueVolumeSlider)
	{
		DialogueVolumeSlider->SetValue(AudioSettings->DialogueVolume);
	}

	if (AmbienceVolumeSlider)
	{
		AmbienceVolumeSlider->SetValue(AudioSettings->AmbienceVolume);
	}

	// Update quality settings
	if (ReverbCheckBox)
	{
		ReverbCheckBox->SetIsChecked(AudioSettings->bEnableReverb);
	}

	if (OcclusionCheckBox)
	{
		OcclusionCheckBox->SetIsChecked(AudioSettings->bEnableOcclusion);
	}

	if (SpatialAudioCheckBox)
	{
		SpatialAudioCheckBox->SetIsChecked(AudioSettings->bEnableSpatialAudio);
	}

	// Update accessibility settings
	if (SubtitlesCheckBox)
	{
		SubtitlesCheckBox->SetIsChecked(AudioSettings->bEnableSubtitles);
	}
}

void UHorrorAudioSettingsWidget::OnMasterVolumeChanged(float Value)
{
	if (AudioSettings)
	{
		AudioSettings->SetMasterVolume(Value, true);
	}
}

void UHorrorAudioSettingsWidget::OnMusicVolumeChanged(float Value)
{
	if (AudioSettings)
	{
		AudioSettings->SetCategoryVolume(TEXT("Music"), Value, true);
	}
}

void UHorrorAudioSettingsWidget::OnSFXVolumeChanged(float Value)
{
	if (AudioSettings)
	{
		AudioSettings->SetCategoryVolume(TEXT("SFX"), Value, true);
	}
}

void UHorrorAudioSettingsWidget::OnDialogueVolumeChanged(float Value)
{
	if (AudioSettings)
	{
		AudioSettings->SetCategoryVolume(TEXT("Dialogue"), Value, true);
	}
}

void UHorrorAudioSettingsWidget::OnAmbienceVolumeChanged(float Value)
{
	if (AudioSettings)
	{
		AudioSettings->SetCategoryVolume(TEXT("Ambience"), Value, true);
	}
}

void UHorrorAudioSettingsWidget::OnReverbChanged(bool bIsChecked)
{
	if (AudioSettings)
	{
		AudioSettings->bEnableReverb = bIsChecked;
	}
}

void UHorrorAudioSettingsWidget::OnOcclusionChanged(bool bIsChecked)
{
	if (AudioSettings)
	{
		AudioSettings->bEnableOcclusion = bIsChecked;
	}
}

void UHorrorAudioSettingsWidget::OnSpatialAudioChanged(bool bIsChecked)
{
	if (AudioSettings)
	{
		AudioSettings->bEnableSpatialAudio = bIsChecked;
	}
}

void UHorrorAudioSettingsWidget::OnOutputDeviceChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (AudioSettings)
	{
		AudioSettings->OutputDevice = SelectedItem;
	}
}

void UHorrorAudioSettingsWidget::OnSubtitlesChanged(bool bIsChecked)
{
	if (AudioSettings)
	{
		AudioSettings->bEnableSubtitles = bIsChecked;
	}
}

void UHorrorAudioSettingsWidget::PopulateOutputDevices()
{
	if (!OutputDeviceComboBox || !AudioSettings)
	{
		return;
	}

	OutputDeviceComboBox->ClearOptions();

	TArray<FString> Devices = AudioSettings->GetAvailableOutputDevices();
	for (const FString& Device : Devices)
	{
		OutputDeviceComboBox->AddOption(Device);
	}
}
