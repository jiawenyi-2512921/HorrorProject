// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/MainMenuWidget.h"
#include "Save/HorrorSaveSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "HorrorProject.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UMainMenuWidget::OnNewGameClicked()
{
	UE_LOG(LogHorrorProject, Log, TEXT("主菜单：开始新游戏"));

	// 清除现有存档，开始新游戏
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UHorrorSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UHorrorSaveSubsystem>())
			{
				SaveSubsystem->ClearCachedSaveOnly();
			}
		}
	}

	// 加载第一章
	UGameplayStatics::OpenLevel(this, FName(FirstChapterMapName));
}

void UMainMenuWidget::OnContinueGameClicked()
{
	UE_LOG(LogHorrorProject, Log, TEXT("主菜单：继续游戏"));

	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UHorrorSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UHorrorSaveSubsystem>())
			{
				// 加载存档
				if (SaveSubsystem->LoadCheckpoint(World))
				{
					UE_LOG(LogHorrorProject, Log, TEXT("主菜单：存档加载成功"));
				}
				else
				{
					UE_LOG(LogHorrorProject, Warning, TEXT("主菜单：存档加载失败，开始新游戏"));
					OnNewGameClicked();
				}
			}
		}
	}
}

void UMainMenuWidget::OnSettingsClicked()
{
	UE_LOG(LogHorrorProject, Log, TEXT("主菜单：打开设置"));

	if (SettingsWidgetClass)
	{
		if (!ActiveSettingsWidget)
		{
			ActiveSettingsWidget = CreateWidget<UUserWidget>(this, SettingsWidgetClass);
			if (ActiveSettingsWidget)
			{
				ActiveSettingsWidget->AddToViewport(100);
			}
		}
	}
}

void UMainMenuWidget::OnCreditsClicked()
{
	UE_LOG(LogHorrorProject, Log, TEXT("主菜单：制作人员名单"));
	// TODO: 显示制作人员名单
}

void UMainMenuWidget::OnQuitGameClicked()
{
	UE_LOG(LogHorrorProject, Log, TEXT("主菜单：退出游戏"));

	if (APlayerController* PC = GetOwningPlayer())
	{
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
	}
}

bool UMainMenuWidget::HasAnySaveGame() const
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UHorrorSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UHorrorSaveSubsystem>())
			{
				return SaveSubsystem->HasSave();
			}
		}
	}
	return false;
}
