// Copyright Epic Games, Inc. All Rights Reserved.

#include "SettingsMenuWidget.h"
#include "GameFramework/GameUserSettings.h"

void USettingsMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void USettingsMenuWidget::OpenSettings()
{
	if (bIsOpen)
	{
		return;
	}

	bIsOpen = true;
	bHasUnsavedChanges = false;
	BP_SettingsOpened();

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeUIOnly());
		PC->bShowMouseCursor = true;
	}
}

void USettingsMenuWidget::CloseSettings()
{
	if (!bIsOpen)
	{
		return;
	}

	bIsOpen = false;
	BP_SettingsClosed();

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}
}

void USettingsMenuWidget::ApplySettings()
{
	if (UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
	{
		Settings->ApplySettings(false);
	}

	bHasUnsavedChanges = false;
	BP_SettingsApplied();
}

void USettingsMenuWidget::ResetToDefaults()
{
	if (UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
	{
		Settings->SetToDefaults();
	}

	bHasUnsavedChanges = true;
}

void USettingsMenuWidget::SelectCategory(ESettingsCategory Category)
{
	CurrentCategory = Category;
	BP_CategorySelected(Category);
}
