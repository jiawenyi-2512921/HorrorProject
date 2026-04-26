// Copyright Epic Games, Inc. All Rights Reserved.

#include "PauseMenuWidget.h"
#include "Kismet/GameplayStatics.h"

void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPauseMenuWidget::OpenPauseMenu()
{
	if (bIsOpen)
	{
		return;
	}

	bIsOpen = true;
	BP_PauseMenuOpened();

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeUIOnly());
		PC->bShowMouseCursor = true;
	}

	UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void UPauseMenuWidget::ClosePauseMenu()
{
	if (!bIsOpen)
	{
		return;
	}

	bIsOpen = false;
	BP_PauseMenuClosed();

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}

	UGameplayStatics::SetGamePaused(GetWorld(), false);
}

void UPauseMenuWidget::ResumeGame()
{
	ClosePauseMenu();
}

void UPauseMenuWidget::OpenSettings()
{
	BP_SettingsOpened();
}

void UPauseMenuWidget::QuitToMainMenu()
{
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("MainMenu")));
}
