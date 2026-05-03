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
				// 完全清除存档缓存和磁盘文件
				SaveSubsystem->ClearCachedSaveOnly();

				// 删除磁盘上的存档文件
				const FString SlotName = TEXT("SM13_Day1_Autosave");
				if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
				{
					UGameplayStatics::DeleteGameInSlot(SlotName, 0);
					UE_LOG(LogHorrorProject, Log, TEXT("主菜单：已删除旧存档，开始全新游戏"));
				}
			}
		}
	}

	// 加载第一章 - 使用绝对路径确保正确加载
	const FString FirstChapterPath = FirstChapterMapName.ToString();
	UE_LOG(LogHorrorProject, Log, TEXT("主菜单：正在加载第一章地图：%s"), *FirstChapterPath);
	UGameplayStatics::OpenLevel(this, FName(*FirstChapterPath), true);
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
