// Copyright Epic Games, Inc. All Rights Reserved.

#include "GraphicsSettingsWidget.h"
#include "GraphicsSettings.h"
#include "GameSettingsSubsystem.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Kismet/GameplayStatics.h"

void UGraphicsSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ResolveGraphicsSettings();
	PopulateResolutions();
	PopulateQualityPresets();
	BindDisplayControls();
	BindQualityControls();
	BindAdvancedControls();
	RefreshSettings();
}

void UGraphicsSettingsWidget::ResolveGraphicsSettings()
{
	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
	{
		if (UGameSettingsSubsystem* SettingsSubsystem = GameInstance->GetSubsystem<UGameSettingsSubsystem>())
		{
			GraphicsSettings = SettingsSubsystem->GetGraphicsSettings();
		}
	}
}

void UGraphicsSettingsWidget::BindDisplayControls()
{
	if (ResolutionComboBox)
	{
		ResolutionComboBox->OnSelectionChanged.AddDynamic(this, &UGraphicsSettingsWidget::OnResolutionChanged);
	}

	if (FullscreenCheckBox)
	{
		FullscreenCheckBox->OnCheckStateChanged.AddDynamic(this, &UGraphicsSettingsWidget::OnFullscreenChanged);
	}

	if (VSyncCheckBox)
	{
		VSyncCheckBox->OnCheckStateChanged.AddDynamic(this, &UGraphicsSettingsWidget::OnVSyncChanged);
	}
}

void UGraphicsSettingsWidget::BindQualityControls()
{
	if (QualityPresetComboBox)
	{
		QualityPresetComboBox->OnSelectionChanged.AddDynamic(this, &UGraphicsSettingsWidget::OnQualityPresetChanged);
	}

	if (ViewDistanceSlider)
	{
		ViewDistanceSlider->OnValueChanged.AddDynamic(this, &UGraphicsSettingsWidget::OnViewDistanceChanged);
	}

	if (TextureQualitySlider)
	{
		TextureQualitySlider->OnValueChanged.AddDynamic(this, &UGraphicsSettingsWidget::OnTextureQualityChanged);
	}

	if (EffectsQualitySlider)
	{
		EffectsQualitySlider->OnValueChanged.AddDynamic(this, &UGraphicsSettingsWidget::OnEffectsQualityChanged);
	}

	if (ShadowQualitySlider)
	{
		ShadowQualitySlider->OnValueChanged.AddDynamic(this, &UGraphicsSettingsWidget::OnShadowQualityChanged);
	}
}

void UGraphicsSettingsWidget::BindAdvancedControls()
{
	if (MotionBlurCheckBox)
	{
		MotionBlurCheckBox->OnCheckStateChanged.AddDynamic(this, &UGraphicsSettingsWidget::OnMotionBlurChanged);
	}

	if (AmbientOcclusionCheckBox)
	{
		AmbientOcclusionCheckBox->OnCheckStateChanged.AddDynamic(this, &UGraphicsSettingsWidget::OnAmbientOcclusionChanged);
	}

	if (BrightnessSlider)
	{
		BrightnessSlider->OnValueChanged.AddDynamic(this, &UGraphicsSettingsWidget::OnBrightnessChanged);
	}
}

void UGraphicsSettingsWidget::RefreshSettings()
{
	if (!GraphicsSettings)
	{
		return;
	}

	RefreshDisplayControls();
	RefreshQualityControls();
	RefreshAdvancedControls();
}

void UGraphicsSettingsWidget::RefreshDisplayControls()
{
	if (FullscreenCheckBox)
	{
		FullscreenCheckBox->SetIsChecked(GraphicsSettings->bFullscreen);
	}

	if (VSyncCheckBox)
	{
		VSyncCheckBox->SetIsChecked(GraphicsSettings->bVSync);
	}
}

void UGraphicsSettingsWidget::RefreshQualityControls()
{
	if (ViewDistanceSlider)
	{
		ViewDistanceSlider->SetValue(GraphicsSettings->ViewDistanceQuality);
	}

	if (TextureQualitySlider)
	{
		TextureQualitySlider->SetValue(GraphicsSettings->TextureQuality);
	}

	if (EffectsQualitySlider)
	{
		EffectsQualitySlider->SetValue(GraphicsSettings->EffectsQuality);
	}

	if (ShadowQualitySlider)
	{
		ShadowQualitySlider->SetValue(static_cast<int32>(GraphicsSettings->ShadowQuality));
	}
}

void UGraphicsSettingsWidget::RefreshAdvancedControls()
{
	if (MotionBlurCheckBox)
	{
		MotionBlurCheckBox->SetIsChecked(GraphicsSettings->bMotionBlur);
	}

	if (AmbientOcclusionCheckBox)
	{
		AmbientOcclusionCheckBox->SetIsChecked(GraphicsSettings->bAmbientOcclusion);
	}

	if (BrightnessSlider)
	{
		BrightnessSlider->SetValue(GraphicsSettings->Brightness);
	}
}

void UGraphicsSettingsWidget::OnResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (!GraphicsSettings)
	{
		return;
	}

	// Parse resolution string (e.g., "1920x1080")
	FString WidthStr, HeightStr;
	if (SelectedItem.Split(TEXT("x"), &WidthStr, &HeightStr))
	{
		FIntPoint NewResolution;
		NewResolution.X = FCString::Atoi(*WidthStr);
		NewResolution.Y = FCString::Atoi(*HeightStr);
		GraphicsSettings->SetResolution(NewResolution, false);
	}
}

void UGraphicsSettingsWidget::OnFullscreenChanged(bool bIsChecked)
{
	if (GraphicsSettings)
	{
		GraphicsSettings->SetFullscreen(bIsChecked, false);
	}
}

void UGraphicsSettingsWidget::OnVSyncChanged(bool bIsChecked)
{
	if (GraphicsSettings)
	{
		GraphicsSettings->bVSync = bIsChecked;
	}
}

void UGraphicsSettingsWidget::OnQualityPresetChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (!GraphicsSettings)
	{
		return;
	}

	int32 PresetIndex = 2; // Default to High

	if (SelectedItem == TEXT("Low"))
	{
		PresetIndex = 0;
	}
	else if (SelectedItem == TEXT("Medium"))
	{
		PresetIndex = 1;
	}
	else if (SelectedItem == TEXT("High"))
	{
		PresetIndex = 2;
	}
	else if (SelectedItem == TEXT("Ultra"))
	{
		PresetIndex = 3;
	}
	else if (SelectedItem == TEXT("Cinematic"))
	{
		PresetIndex = 4;
	}

	GraphicsSettings->ApplyQualityPreset(PresetIndex);
	RefreshSettings();
}

void UGraphicsSettingsWidget::OnViewDistanceChanged(float Value)
{
	if (GraphicsSettings)
	{
		GraphicsSettings->ViewDistanceQuality = FMath::RoundToInt(Value);
	}
}

void UGraphicsSettingsWidget::OnTextureQualityChanged(float Value)
{
	if (GraphicsSettings)
	{
		GraphicsSettings->TextureQuality = FMath::RoundToInt(Value);
	}
}

void UGraphicsSettingsWidget::OnEffectsQualityChanged(float Value)
{
	if (GraphicsSettings)
	{
		GraphicsSettings->EffectsQuality = FMath::RoundToInt(Value);
	}
}

void UGraphicsSettingsWidget::OnShadowQualityChanged(float Value)
{
	if (GraphicsSettings)
	{
		GraphicsSettings->ShadowQuality = static_cast<EShadowQuality>(FMath::RoundToInt(Value));
	}
}

void UGraphicsSettingsWidget::OnMotionBlurChanged(bool bIsChecked)
{
	if (GraphicsSettings)
	{
		GraphicsSettings->bMotionBlur = bIsChecked;
	}
}

void UGraphicsSettingsWidget::OnAmbientOcclusionChanged(bool bIsChecked)
{
	if (GraphicsSettings)
	{
		GraphicsSettings->bAmbientOcclusion = bIsChecked;
	}
}

void UGraphicsSettingsWidget::OnBrightnessChanged(float Value)
{
	if (GraphicsSettings)
	{
		GraphicsSettings->Brightness = Value;
	}
}

void UGraphicsSettingsWidget::PopulateResolutions()
{
	if (!ResolutionComboBox || !GraphicsSettings)
	{
		return;
	}

	ResolutionComboBox->ClearOptions();

	TArray<FIntPoint> Resolutions = GraphicsSettings->GetSupportedResolutions();
	FString ResolutionStr;
	for (const FIntPoint& Resolution : Resolutions)
	{
		ResolutionStr.Reset();
		ResolutionStr.Appendf(TEXT("%dx%d"), Resolution.X, Resolution.Y);
		ResolutionComboBox->AddOption(ResolutionStr);
	}
}

void UGraphicsSettingsWidget::PopulateQualityPresets()
{
	if (!QualityPresetComboBox)
	{
		return;
	}

	QualityPresetComboBox->ClearOptions();
	QualityPresetComboBox->AddOption(TEXT("Low"));
	QualityPresetComboBox->AddOption(TEXT("Medium"));
	QualityPresetComboBox->AddOption(TEXT("High"));
	QualityPresetComboBox->AddOption(TEXT("Ultra"));
	QualityPresetComboBox->AddOption(TEXT("Cinematic"));
}
