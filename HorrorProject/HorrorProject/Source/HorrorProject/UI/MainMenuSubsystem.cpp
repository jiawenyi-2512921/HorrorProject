// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/MainMenuSubsystem.h"
#include "UI/MainMenuHUD.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"
#include "HorrorProject.h"

void UMainMenuSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogHorrorProject, Log, TEXT("主菜单子系统已初始化"));

	// 延迟显示主菜单
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		FTimerHandle TimerHandle;
		GameInstance->GetTimerManager().SetTimerForNextTick([this]()
		{
			if (bFirstLaunch)
			{
				ShowMainMenu();
			}
		});
	}
}

void UMainMenuSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UMainMenuSubsystem::OnWorldBeginPlay(UWorld* World)
{
	// 不再使用
}

void UMainMenuSubsystem::ShowMainMenu()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC)
	{
		return;
	}

	// 创建或获取MainMenuHUD
	AMainMenuHUD* MenuHUD = Cast<AMainMenuHUD>(PC->GetHUD());
	if (!MenuHUD)
	{
		// 如果当前HUD不是MainMenuHUD，则替换它
		if (PC->MyHUD)
		{
			PC->MyHUD->Destroy();
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = PC;
		MenuHUD = World->SpawnActor<AMainMenuHUD>(AMainMenuHUD::StaticClass(), SpawnParams);
		PC->MyHUD = MenuHUD;
	}

	if (MenuHUD)
	{
		MenuHUD->SetActorHiddenInGame(false);
		MenuHUD->SetActorTickEnabled(true);
		bMainMenuVisible = true;
		bFirstLaunch = false;

		PC->bShowMouseCursor = true;

		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);

		// 暂停游戏
		UGameplayStatics::SetGamePaused(World, true);

		UE_LOG(LogHorrorProject, Log, TEXT("主菜单已显示"));
	}
}

void UMainMenuSubsystem::HideMainMenu()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC)
	{
		return;
	}

	AMainMenuHUD* MenuHUD = Cast<AMainMenuHUD>(PC->GetHUD());
	if (MenuHUD)
	{
		MenuHUD->SetActorHiddenInGame(true);
		MenuHUD->SetActorTickEnabled(false);
		bMainMenuVisible = false;

		// 恢复游戏输入模式
		PC->bShowMouseCursor = false;
		PC->bEnableClickEvents = false;
		PC->bEnableMouseOverEvents = false;

		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);

		// 取消暂停
		UGameplayStatics::SetGamePaused(World, false);

		UE_LOG(LogHorrorProject, Log, TEXT("主菜单已隐藏"));
	}
}
