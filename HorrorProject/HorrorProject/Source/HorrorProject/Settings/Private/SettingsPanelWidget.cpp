// Copyright Epic Games, Inc. All Rights Reserved.

#include "SettingsPanelWidget.h"
#include "GameSettingsSubsystem.h"
#include "GraphicsSettingsWidget.h"
#include "AudioSettingsWidget.h"
#include "ControlsSettingsWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/GameplayStatics.h"

void USettingsPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind button events
	if (GraphicsTabButton)
	{
		GraphicsTabButton->OnClicked.AddDynamic(this, &USettingsPanelWidget::OnGraphicsTabClicked);
	}

	if (AudioTabButton)
	{
		AudioTabButton->OnClicked.AddDynamic(this, &USettingsPanelWidget::OnAudioTabClicked);
	}

	if (ControlsTabButton)
	{
		ControlsTabButton->OnClicked.AddDynamic(this, &USettingsPanelWidget::OnControlsTabClicked);
	}

	if (GameplayTabButton)
	{
		GameplayTabButton->OnClicked.AddDynamic(this, &USettingsPanelWidget::OnGameplayTabClicked);
	}

	if (ApplyButton)
	{
		ApplyButton->OnClicked.AddDynamic(this, &USettingsPanelWidget::OnApplyClicked);
	}

	if (ResetButton)
	{
		ResetButton->OnClicked.AddDynamic(this, &USettingsPanelWidget::OnResetClicked);
	}

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &USettingsPanelWidget::OnBackClicked);
	}

	InitializeSettings();
	SwitchToTab(0);
}

void USettingsPanelWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void USettingsPanelWidget::OnGraphicsTabClicked()
{
	SwitchToTab(0);
}

void USettingsPanelWidget::OnAudioTabClicked()
{
	SwitchToTab(1);
}

void USettingsPanelWidget::OnControlsTabClicked()
{
	SwitchToTab(2);
}

void USettingsPanelWidget::OnGameplayTabClicked()
{
	SwitchToTab(3);
}

void USettingsPanelWidget::OnApplyClicked()
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->ApplySettings();
		SettingsSubsystem->SaveSettings();
	}
}

void USettingsPanelWidget::OnResetClicked()
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->ResetToDefaults();
		RefreshSettings();
	}
}

void USettingsPanelWidget::OnBackClicked()
{
	if (SettingsSubsystem && SettingsSubsystem->HasUnsavedChanges())
	{
		// Show confirmation dialog
	}

	RemoveFromParent();
}

void USettingsPanelWidget::InitializeSettings()
{
	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
	{
		SettingsSubsystem = GameInstance->GetSubsystem<UGameSettingsSubsystem>();
	}

	RefreshSettings();
}

void USettingsPanelWidget::RefreshSettings()
{
	if (GraphicsWidget)
	{
		GraphicsWidget->RefreshSettings();
	}

	if (AudioWidget)
	{
		AudioWidget->RefreshSettings();
	}

	if (ControlsWidget)
	{
		ControlsWidget->RefreshSettings();
	}
}

void USettingsPanelWidget::SwitchToTab(int32 TabIndex)
{
	if (SettingsSwitcher)
	{
		SettingsSwitcher->SetActiveWidgetIndex(TabIndex);
		CurrentTabIndex = TabIndex;
	}
}
