// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

/**
 * Pause menu with resume, settings, and quit options
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class HORRORPROJECT_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category="Pause|UI")
	void OpenPauseMenu();

	UFUNCTION(BlueprintCallable, Category="Pause|UI")
	void ClosePauseMenu();

	UFUNCTION(BlueprintCallable, Category="Pause|UI")
	void ResumeGame();

	UFUNCTION(BlueprintCallable, Category="Pause|UI")
	void OpenSettings();

	UFUNCTION(BlueprintCallable, Category="Pause|UI")
	void QuitToMainMenu();

	UFUNCTION(BlueprintPure, Category="Pause|UI")
	bool IsOpen() const { return bIsOpen; }

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="Pause|UI", meta=(DisplayName="暂停菜单已打开"))
	void BP_PauseMenuOpened();

	UFUNCTION(BlueprintImplementableEvent, Category="Pause|UI", meta=(DisplayName="暂停菜单已关闭"))
	void BP_PauseMenuClosed();

	UFUNCTION(BlueprintImplementableEvent, Category="Pause|UI", meta=(DisplayName="设置已打开"))
	void BP_SettingsOpened();

	UPROPERTY(BlueprintReadOnly, Category="Pause|UI")
	bool bIsOpen = false;

	UPROPERTY(EditDefaultsOnly, Category="Pause|UI")
	float FadeInDuration = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category="Pause|UI")
	float FadeOutDuration = 0.2f;
};
