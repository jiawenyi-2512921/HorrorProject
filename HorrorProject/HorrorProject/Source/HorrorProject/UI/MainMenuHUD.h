// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MainMenuHUD.generated.h"

UCLASS()
class HORRORPROJECT_API AMainMenuHUD : public AHUD
{
	GENERATED_BODY()

public:
	AMainMenuHUD();

	virtual void DrawHUD() override;
	virtual void BeginPlay() override;

	void HandleMouseClick();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|MainMenu")
	FString BackgroundImagePath = TEXT("/Game/UI/MainMenu/MainMenuBackground");

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> BackgroundTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|MainMenu")
	FName FirstChapterMapName = TEXT("LVL_Showcase_01");

private:
	struct FMenuButton
	{
		FText Label;
		FVector2D Position;
		FVector2D Size;
		int32 Index;
	};

	enum class EMenuState : uint8
	{
		Main,
		Settings,
		Credits
	};

	TArray<FMenuButton> MenuButtons;
	int32 HoveredButtonIndex = -1;
	bool bHasSaveGame = false;
	bool bWasMousePressed = false;
	EMenuState CurrentState = EMenuState::Main;

	// 设置项
	float SettingMasterVolume = 1.0f;
	float SettingMusicVolume = 1.0f;
	float SettingSFXVolume = 1.0f;
	int32 SettingSelectedResolution = 0;
	bool SettingFullscreen = false;
	int32 SettingsHoveredItem = -1;

	struct FSettingsSlider
	{
		FString Label;
		float* Value;
		FVector2D BarPosition;
		FVector2D BarSize;
	};
	TArray<FSettingsSlider> SettingsSliders;
	int32 DraggingSliderIndex = -1;

	void InitializeButtons();
	void DrawBackground();
	void DrawButtons();
	void DrawButton(const FMenuButton& Button, bool bIsHovered);
	void DrawCreditsScreen();
	void DrawSettingsScreen();
	void ApplySettings();
	bool IsPointInButton(const FVector2D& Point, const FMenuButton& Button) const;
	void HandleButtonClick(int32 ButtonIndex);
	void CheckHoveredButton(const FVector2D& MousePosition);
	bool CheckHasSaveGame() const;
};
