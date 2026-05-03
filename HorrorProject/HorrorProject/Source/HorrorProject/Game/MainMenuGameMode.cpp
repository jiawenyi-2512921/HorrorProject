// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/MainMenuGameMode.h"
#include "UI/MainMenuHUD.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "HorrorProject.h"

AMainMenuGameMode::AMainMenuGameMode()
{
	// 设置默认的PlayerController和HUD类
	PlayerControllerClass = APlayerController::StaticClass();
	HUDClass = AMainMenuHUD::StaticClass();
	DefaultPawnClass = nullptr; // 主菜单不需要Pawn
}

void AMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 显示鼠标光标
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;

		FInputModeUIOnly InputMode;
		PC->SetInputMode(InputMode);
	}

	UE_LOG(LogHorrorProject, Log, TEXT("主菜单GameMode已启动"));
}
