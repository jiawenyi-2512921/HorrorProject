// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MainMenuSubsystem.generated.h"

/**
 * 主菜单子系统 - 管理主菜单的显示和隐藏
 */
UCLASS()
class HORRORPROJECT_API UMainMenuSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category="Horror|MainMenu")
	void ShowMainMenu();

	UFUNCTION(BlueprintCallable, Category="Horror|MainMenu")
	void HideMainMenu();

	UFUNCTION(BlueprintPure, Category="Horror|MainMenu")
	bool IsMainMenuVisible() const { return bMainMenuVisible; }

private:
	void OnWorldBeginPlay(UWorld* World);

	bool bMainMenuVisible = false;
	bool bFirstLaunch = true;
};
