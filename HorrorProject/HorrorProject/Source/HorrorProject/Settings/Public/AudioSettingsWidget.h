// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AudioSettingsWidget.generated.h"

class UHorrorAudioSettings;
class USlider;
class UCheckBox;
class UComboBoxString;

/**
 * Audio Settings Widget
 * UI for audio settings
 */
UCLASS()
class HORRORPROJECT_API UHorrorAudioSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void RefreshSettings();

protected:
	// Volume sliders
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> MasterVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> MusicVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> SFXVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> DialogueVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> AmbienceVolumeSlider;

	// Quality settings
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> ReverbCheckBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> OcclusionCheckBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> SpatialAudioCheckBox;

	// Output settings
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> OutputDeviceComboBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> SubtitlesCheckBox;

	// Callbacks
	UFUNCTION()
	void OnMasterVolumeChanged(float Value);

	UFUNCTION()
	void OnMusicVolumeChanged(float Value);

	UFUNCTION()
	void OnSFXVolumeChanged(float Value);

	UFUNCTION()
	void OnDialogueVolumeChanged(float Value);

	UFUNCTION()
	void OnAmbienceVolumeChanged(float Value);

	UFUNCTION()
	void OnReverbChanged(bool bIsChecked);

	UFUNCTION()
	void OnOcclusionChanged(bool bIsChecked);

	UFUNCTION()
	void OnSpatialAudioChanged(bool bIsChecked);

	UFUNCTION()
	void OnOutputDeviceChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnSubtitlesChanged(bool bIsChecked);

private:
	UPROPERTY()
	TObjectPtr<UHorrorAudioSettings> AudioSettings;

	void ResolveAudioSettings();
	void BindVolumeControls();
	void BindOptionControls();
	void RefreshVolumeControls();
	void RefreshOptionControls();
	void PopulateOutputDevices();
};
