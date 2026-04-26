// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GraphicsSettingsWidget.generated.h"

class UGraphicsSettings;
class UComboBoxString;
class USlider;
class UCheckBox;

/**
 * Graphics Settings Widget
 * UI for graphics settings
 */
UCLASS()
class HORRORPROJECT_API UGraphicsSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void RefreshSettings();

protected:
	// Display settings
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> ResolutionComboBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> FullscreenCheckBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> VSyncCheckBox;

	// Quality preset
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> QualityPresetComboBox;

	// Quality sliders
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> ViewDistanceSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> TextureQualitySlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> EffectsQualitySlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> ShadowQualitySlider;

	// Advanced settings
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> MotionBlurCheckBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> AmbientOcclusionCheckBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> BrightnessSlider;

	// Callbacks
	UFUNCTION()
	void OnResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnFullscreenChanged(bool bIsChecked);

	UFUNCTION()
	void OnVSyncChanged(bool bIsChecked);

	UFUNCTION()
	void OnQualityPresetChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnViewDistanceChanged(float Value);

	UFUNCTION()
	void OnTextureQualityChanged(float Value);

	UFUNCTION()
	void OnEffectsQualityChanged(float Value);

	UFUNCTION()
	void OnShadowQualityChanged(float Value);

	UFUNCTION()
	void OnMotionBlurChanged(bool bIsChecked);

	UFUNCTION()
	void OnAmbientOcclusionChanged(bool bIsChecked);

	UFUNCTION()
	void OnBrightnessChanged(float Value);

private:
	UPROPERTY()
	TObjectPtr<UGraphicsSettings> GraphicsSettings;

	void PopulateResolutions();
	void PopulateQualityPresets();
};
