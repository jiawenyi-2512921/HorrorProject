// Copyright Epic Games, Inc. All Rights Reserved.

#include "AudioSettingsWidget.h"
#include "AudioSettings.h"
#include "GameSettingsSubsystem.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Kismet/GameplayStatics.h"

void UAudioSettingsWidget::NativeConstruct()
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
		MasterVolumeSlider->OnValueChanged.AddDynamic(this, &UAudioSettingsWidget::OnMasterVolumeChanged);
	}

	if (MusicVolumeSlider)
	{
		MusicVolumeSlider->OnValueChanged.AddDynamic(this, &UAudioSettingsWidget::OnMusicVolumeChanged);
	}

	if (SFXVolumeSlider)
	{
		SFXVolumeSlider->OnValueChanged.AddDynamic(this, &UAudioSettingsWidget::OnSFXVolumeChanged);
	}

	if (DialogueVolumeSlider)
	{
		DialogueVolumeSlider->OnValueChanged.AddDynamic(this, &UAudioSettingsWidget::OnDialogueVolumeChanged);
	}

	if (AmbienceVolumeSlider)
	{
		AmbienceVolumeSlider->OnValueChanged.AddDynamic(this, &UAudioSettingsWidget::OnAmbienceVolumeChanged);
	}

	if (ReverbCheckBox)
	{
		ReverbCheckBox->OnCheckStateChanged.AddDynamic(this, &UAudioSettingsWidget::OnReverbChanged);
	}

	if (OcclusionCheckBox)
	{
		OcclusionCheckBox->OnCheckStateChanged.AddDynamic(this, &UAudioSettingsWidget::OnOcclusionChanged);
	}

	if (SpatialAudioCheckBox)
	{
		SpatialAudioCheckBox->OnCheckStateChanged.AddDynamic(this, &UAudioSettingsWidget::OnSpatialAudioChanged);
	}

	if (OutputDeviceComboBox)
	{
		OutputDeviceComboBox->OnSelectionChanged.AddDynamic(this, &UAudioSettingsWidget::OnOutputDeviceChanged);
	}

	if (SubtitlesCheckBox)
	{
		SubtitlesCheckBox->OnCheckStateChanged.AddDynamic(this, &UAudioSettingsWidget::OnSubtitlesChanged);
	}

	RefreshSettings();
}

void UAudioSettingsWidget::RefreshSettings()
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

void UAudioSettingsWidget::OnMasterVolumeChanged(float Value)
{
	if (AudioSettings)
	{
		AudioSettings->SetMasterVolume(Value, true);
	}
}

void UAudioSettingsWidget::OnMusicVolumeChanged(float Value)
{
	if (AudioSettings)
	{
		AudioSettings->SetCategoryVolume(TEXT("Music"), Value, true);
	}
}

void UAudioSettingsWidget::OnSFXVolumeChanged(float Value)
{
	if (AudioSettings)
	{
		AudioSettings->SetCategoryVolume(TEXT("SFX"), Value, true);
	}
}

void UAudioSettingsWidget::OnDialogueVolumeChanged(float Value)
{
	if (AudioSettings)
	{
		AudioSettings->SetCategoryVolume(TEXT("Dialogue"), Value, true);
	}
}

void UAudioSettingsWidget::OnAmbienceVolumeChanged(float Value)
{
	if (AudioSettings)
	{
		AudioSettings->SetCategoryVolume(TEXT("Ambience"), Value, true);
	}
}

void UAudioSettingsWidget::OnReverbChanged(bool bIsChecked)
{
	if (AudioSettings)
	{
		AudioSettings->bEnableReverb = bIsChecked;
	}
}

void UAudioSettingsWidget::OnOcclusionChanged(bool bIsChecked)
{
	if (AudioSettings)
	{
		AudioSettings->bEnableOcclusion = bIsChecked;
	}
}

void UAudioSettingsWidget::OnSpatialAudioChanged(bool bIsChecked)
{
	if (AudioSettings)
	{
		AudioSettings->bEnableSpatialAudio = bIsChecked;
	}
}

void UAudioSettingsWidget::OnOutputDeviceChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (AudioSettings)
	{
		AudioSettings->OutputDevice = SelectedItem;
	}
}

void UAudioSettingsWidget::OnSubtitlesChanged(bool bIsChecked)
{
	if (AudioSettings)
	{
		AudioSettings->bEnableSubtitles = bIsChecked;
	}
}

void UAudioSettingsWidget::PopulateOutputDevices()
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
