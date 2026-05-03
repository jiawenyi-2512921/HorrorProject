// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

/**
 * 主菜单Widget
 */
UCLASS()
class HORRORPROJECT_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category="Horror|MainMenu")
	void OnNewGameClicked();

	UFUNCTION(BlueprintCallable, Category="Horror|MainMenu")
	void OnContinueGameClicked();

	UFUNCTION(BlueprintCallable, Category="Horror|MainMenu")
	void OnSettingsClicked();

	UFUNCTION(BlueprintCallable, Category="Horror|MainMenu")
	void OnCreditsClicked();

	UFUNCTION(BlueprintCallable, Category="Horror|MainMenu")
	void OnQuitGameClicked();

	UFUNCTION(BlueprintPure, Category="Horror|MainMenu")
	bool HasAnySaveGame() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|MainMenu")
	FName FirstChapterMapName = TEXT("/Game/DeepWaterStation/Maps/DemoMap_VerticalSlice_Day1");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|MainMenu")
	TSubclassOf<UUserWidget> SettingsWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> ActiveSettingsWidget;
};
