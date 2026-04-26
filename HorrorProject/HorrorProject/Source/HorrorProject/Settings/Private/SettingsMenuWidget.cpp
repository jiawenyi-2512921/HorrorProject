// Copyright Epic Games, Inc. All Rights Reserved.

#include "SettingsMenuWidget.h"
#include "GameSettingsSubsystem.h"
#include "GraphicsSettingsWidget.h"
#include "AudioSettingsWidget.h"
#include "ControlsSettingsWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/GameplayStatics.h"

void USettingsMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind button events
	if (GraphicsTabButton)
	{
		GraphicsTabButton->OnClicked.AddDynamic(this, &USettingsMenuWidget::OnGraphicsTabClicked);
	}

	if (AudioTabButton)
	{
		AudioTabButton->OnClicked.AddDynamic(this, &USettingsMenuWidget::OnAudioTabClicked);
	}

	if (ControlsTabButton)
	{
		ControlsTabButton->OnClicked.AddDynamic(this, &USettingsMenuWidget::OnControlsTabClicked);
	}

	if (GameplayTabButton)
	{
		GameplayTabButton->OnClicked.AddDynamic(this, &USettingsMenuWidget::OnGameplayTabClicked);
	}

	if (ApplyButton)
	{
		ApplyButton->OnClicked.AddDynamic(this, &USettingsMenuWidget::OnApplyClicked);
	}

	if (ResetButton)
	{
		ResetButton->OnClicked.AddDynamic(this, &USettingsMenuWidget::OnResetClicked);
	}

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &USettingsMenuWidget::OnBackClicked);
	}

	InitializeSettings();
	SwitchToTab(0);
}

void USettingsMenuWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void USettingsMenuWidget::OnGraphicsTabClicked()
{
	SwitchToTab(0);
}

void USettingsMenuWidget::OnAudioTabClicked()
{
	SwitchToTab(1);
}

void USettingsMenuWidget::OnControlsTabClicked()
{
	SwitchToTab(2);
}

void USettingsMenuWidget::OnGameplayTabClicked()
{
	SwitchToTab(3);
}

void USettingsMenuWidget::OnApplyClicked()
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->ApplySettings();
		SettingsSubsystem->SaveSettings();
	}
}

void USettingsMenuWidget::OnResetClicked()
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->ResetToDefaults();
		RefreshSettings();
	}
}

void USettingsMenuWidget::OnBackClicked()
{
	if (SettingsSubsystem && SettingsSubsystem->HasUnsavedChanges())
	{
		// Show confirmation dialog
	}

	RemoveFromParent();
}

void USettingsMenuWidget::InitializeSettings()
{
	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
	{
		SettingsSubsystem = GameInstance->GetSubsystem<UGameSettingsSubsystem>();
	}

	RefreshSettings();
}

void USettingsMenuWidget::RefreshSettings()
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

void USettingsMenuWidget::SwitchToTab(int32 TabIndex)
{
	if (SettingsSwitcher)
	{
		SettingsSwitcher->SetActiveWidgetIndex(TabIndex);
		CurrentTabIndex = TabIndex;
	}
}
