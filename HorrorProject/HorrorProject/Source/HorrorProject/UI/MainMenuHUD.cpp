// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/MainMenuHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Save/HorrorSaveSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/GameUserSettings.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "AudioDevice.h"
#include "HorrorProject.h"

AMainMenuHUD::AMainMenuHUD()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	bShowHUD = true;
}

void AMainMenuHUD::BeginPlay()
{
	Super::BeginPlay();

	BackgroundTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *BackgroundImagePath));
	if (!BackgroundTexture)
	{
		UE_LOG(LogHorrorProject, Warning, TEXT("主菜单HUD: 无法加载背景图片 %s"), *BackgroundImagePath);
	}

	bHasSaveGame = CheckHasSaveGame();

	if (UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		SettingFullscreen = UserSettings->GetFullscreenMode() != EWindowMode::Windowed;
	}

	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->bShowMouseCursor = true;
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
	}
}

void AMainMenuHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	InitializeButtons();
	DrawBackground();

	const bool bIsMousePressed = FSlateApplication::IsInitialized()
		&& FSlateApplication::Get().GetPressedMouseButtons().Contains(EKeys::LeftMouseButton);

	switch (CurrentState)
	{
	case EMenuState::Main:
		{
			if (APlayerController* PC = GetOwningPlayerController())
			{
				float MouseX, MouseY;
				PC->GetMousePosition(MouseX, MouseY);
				CheckHoveredButton(FVector2D(MouseX, MouseY));
			}

			if (bIsMousePressed && !bWasMousePressed)
			{
				HandleMouseClick();
			}
			bWasMousePressed = bIsMousePressed;
			DrawButtons();
		}
		break;

	case EMenuState::Settings:
		DrawSettingsScreen();
		break;

	case EMenuState::Credits:
		{
			DrawCreditsScreen();
			if (bIsMousePressed && !bWasMousePressed)
			{
				CurrentState = EMenuState::Main;
			}
			bWasMousePressed = bIsMousePressed;
		}
		break;
	}
}

void AMainMenuHUD::InitializeButtons()
{
	MenuButtons.Empty();

	if (!Canvas)
	{
		return;
	}

	const float CenterX = Canvas->SizeX * 0.5f;
	const float CenterY = Canvas->SizeY * 0.5f;
	const float ButtonWidth = 400.0f;
	const float ButtonHeight = 80.0f;
	const float ButtonSpacing = 100.0f;
	const float TotalHeight = ButtonSpacing * 4;
	const float StartY = CenterY - TotalHeight * 0.5f;

	MenuButtons.Add({ FText::FromString(TEXT("新游戏")),    FVector2D(CenterX - ButtonWidth * 0.5f, StartY),                      FVector2D(ButtonWidth, ButtonHeight), 0 });
	MenuButtons.Add({ FText::FromString(TEXT("继续存档")),   FVector2D(CenterX - ButtonWidth * 0.5f, StartY + ButtonSpacing),       FVector2D(ButtonWidth, ButtonHeight), 1 });
	MenuButtons.Add({ FText::FromString(TEXT("设置")),       FVector2D(CenterX - ButtonWidth * 0.5f, StartY + ButtonSpacing * 2),   FVector2D(ButtonWidth, ButtonHeight), 2 });
	MenuButtons.Add({ FText::FromString(TEXT("制作人员")),   FVector2D(CenterX - ButtonWidth * 0.5f, StartY + ButtonSpacing * 3),   FVector2D(ButtonWidth, ButtonHeight), 3 });
	MenuButtons.Add({ FText::FromString(TEXT("退出游戏")),   FVector2D(CenterX - ButtonWidth * 0.5f, StartY + ButtonSpacing * 4),   FVector2D(ButtonWidth, ButtonHeight), 4 });
}

void AMainMenuHUD::DrawBackground()
{
	if (BackgroundTexture && Canvas)
	{
		FCanvasTileItem TileItem(
			FVector2D(0.0f, 0.0f),
			BackgroundTexture->GetResource(),
			FVector2D(Canvas->SizeX, Canvas->SizeY),
			FLinearColor::White
		);
		TileItem.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(TileItem);
	}
	else if (Canvas)
	{
		FCanvasTileItem TileItem(
			FVector2D(0.0f, 0.0f),
			FVector2D(Canvas->SizeX, Canvas->SizeY),
			FLinearColor::Black
		);
		Canvas->DrawItem(TileItem);
	}
}

void AMainMenuHUD::DrawButtons()
{
	if (!Canvas) return;

	for (const FMenuButton& Button : MenuButtons)
	{
		const bool bIsDisabled = (Button.Index == 1 && !bHasSaveGame);
		const bool bIsHovered = (Button.Index == HoveredButtonIndex && !bIsDisabled);
		DrawButton(Button, bIsHovered);
	}
}

void AMainMenuHUD::DrawButton(const FMenuButton& Button, bool bIsHovered)
{
	if (!Canvas) return;

	const bool bIsDisabled = (Button.Index == 1 && !bHasSaveGame);

	FLinearColor BgColor = bIsDisabled
		? FLinearColor(0.2f, 0.2f, 0.2f, 0.5f)
		: (bIsHovered ? FLinearColor(0.8f, 0.6f, 0.2f, 0.8f) : FLinearColor(0.1f, 0.1f, 0.1f, 0.7f));

	FCanvasTileItem BgItem(Button.Position, Button.Size, BgColor);
	Canvas->DrawItem(BgItem);

	FLinearColor BorderColor = bIsHovered ? FLinearColor::White : FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

	FCanvasLineItem TopLine(Button.Position, Button.Position + FVector2D(Button.Size.X, 0.0f));
	TopLine.SetColor(BorderColor); TopLine.LineThickness = 2.0f;
	Canvas->DrawItem(TopLine);

	FCanvasLineItem BottomLine(Button.Position + FVector2D(0.0f, Button.Size.Y), Button.Position + Button.Size);
	BottomLine.SetColor(BorderColor); BottomLine.LineThickness = 2.0f;
	Canvas->DrawItem(BottomLine);

	FCanvasLineItem LeftLine(Button.Position, Button.Position + FVector2D(0.0f, Button.Size.Y));
	LeftLine.SetColor(BorderColor); LeftLine.LineThickness = 2.0f;
	Canvas->DrawItem(LeftLine);

	FCanvasLineItem RightLine(Button.Position + FVector2D(Button.Size.X, 0.0f), Button.Position + Button.Size);
	RightLine.SetColor(BorderColor); RightLine.LineThickness = 2.0f;
	Canvas->DrawItem(RightLine);

	UFont* Font = GEngine ? GEngine->GetLargeFont() : nullptr;
	if (Font)
	{
		FLinearColor TextColor = bIsDisabled ? FLinearColor(0.4f, 0.4f, 0.4f, 1.0f) : FLinearColor::White;
		float TextWidth, TextHeight;
		Canvas->TextSize(Font, Button.Label.ToString(), TextWidth, TextHeight);

		FVector2D TextPosition(
			Button.Position.X + (Button.Size.X - TextWidth) * 0.5f,
			Button.Position.Y + (Button.Size.Y - TextHeight) * 0.5f
		);

		FCanvasTextItem TextItem(TextPosition, Button.Label, Font, TextColor);
		TextItem.Scale = FVector2D(1.2f);
		Canvas->DrawItem(TextItem);
	}
}

bool AMainMenuHUD::IsPointInButton(const FVector2D& Point, const FMenuButton& Button) const
{
	return Point.X >= Button.Position.X && Point.X <= Button.Position.X + Button.Size.X &&
	       Point.Y >= Button.Position.Y && Point.Y <= Button.Position.Y + Button.Size.Y;
}

void AMainMenuHUD::CheckHoveredButton(const FVector2D& MousePosition)
{
	HoveredButtonIndex = -1;
	for (const FMenuButton& Button : MenuButtons)
	{
		if (IsPointInButton(MousePosition, Button))
		{
			HoveredButtonIndex = Button.Index;
			break;
		}
	}
}

void AMainMenuHUD::HandleMouseClick()
{
	if (!Canvas) return;

	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	float MouseX, MouseY;
	PC->GetMousePosition(MouseX, MouseY);
	FVector2D MousePosition(MouseX, MouseY);

	for (const FMenuButton& Button : MenuButtons)
	{
		if (IsPointInButton(MousePosition, Button))
		{
			if (Button.Index == 1 && !bHasSaveGame)
			{
				continue;
			}
			HandleButtonClick(Button.Index);
			break;
		}
	}
}

void AMainMenuHUD::HandleButtonClick(int32 ButtonIndex)
{
	switch (ButtonIndex)
	{
	case 0: // 新游戏
		{
			if (UWorld* World = GetWorld())
			{
				if (UGameInstance* GameInstance = World->GetGameInstance())
				{
					if (UHorrorSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UHorrorSaveSubsystem>())
					{
						SaveSubsystem->ClearCachedSaveOnly();
					}
				}
				UGameplayStatics::SetGamePaused(World, false);
			}
			UGameplayStatics::OpenLevel(this, FirstChapterMapName);
		}
		break;

	case 1: // 继续存档
		{
			if (UWorld* World = GetWorld())
			{
				if (UGameInstance* GameInstance = World->GetGameInstance())
				{
					if (UHorrorSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UHorrorSaveSubsystem>())
					{
						UGameplayStatics::SetGamePaused(World, false);
						SaveSubsystem->LoadCheckpoint(World);
					}
				}
			}
		}
		break;

	case 2: // 设置
		CurrentState = EMenuState::Settings;
		DraggingSliderIndex = -1;
		break;

	case 3: // 制作人员
		CurrentState = EMenuState::Credits;
		break;

	case 4: // 退出游戏
		if (APlayerController* PC = GetOwningPlayerController())
		{
			UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
		}
		break;
	}
}

// ============== 设置界面 ==============

void AMainMenuHUD::DrawSettingsScreen()
{
	if (!Canvas) return;

	// 半透明背景
	FCanvasTileItem BgItem(FVector2D(0, 0), FVector2D(Canvas->SizeX, Canvas->SizeY), FLinearColor(0, 0, 0, 0.92f));
	Canvas->DrawItem(BgItem);

	UFont* TitleFont = GEngine ? GEngine->GetLargeFont() : nullptr;
	UFont* Font = GEngine ? GEngine->GetMediumFont() : nullptr;
	if (!Font || !TitleFont) return;

	const float CenterX = Canvas->SizeX * 0.5f;
	const float SliderWidth = 400.0f;
	const float SliderHeight = 12.0f;
	const float ItemSpacing = 70.0f;
	const float LabelScale = 1.4f;
	const float StartY = Canvas->SizeY * 0.15f;
	const float SliderX = CenterX - SliderWidth * 0.5f;

	// 标题
	{
		float TW, TH;
		Canvas->TextSize(TitleFont, TEXT("设置"), TW, TH);
		FCanvasTextItem Title(FVector2D(CenterX - TW, StartY), FText::FromString(TEXT("设置")), TitleFont, FLinearColor::White);
		Title.Scale = FVector2D(2.0f);
		Canvas->DrawItem(Title);
	}

	const float ContentStartY = StartY + 100.0f;

	// 构建滑块数据
	SettingsSliders.Empty();
	SettingsSliders.Add({ TEXT("主音量"), &SettingMasterVolume, FVector2D(SliderX, ContentStartY), FVector2D(SliderWidth, SliderHeight) });
	SettingsSliders.Add({ TEXT("音乐音量"), &SettingMusicVolume, FVector2D(SliderX, ContentStartY + ItemSpacing), FVector2D(SliderWidth, SliderHeight) });
	SettingsSliders.Add({ TEXT("音效音量"), &SettingSFXVolume, FVector2D(SliderX, ContentStartY + ItemSpacing * 2), FVector2D(SliderWidth, SliderHeight) });

	// 获取鼠标状态
	APlayerController* PC = GetOwningPlayerController();
	FVector2D MousePos(0, 0);
	if (PC)
	{
		float MX, MY;
		PC->GetMousePosition(MX, MY);
		MousePos = FVector2D(MX, MY);
	}

	const bool bIsMousePressed = FSlateApplication::IsInitialized()
		&& FSlateApplication::Get().GetPressedMouseButtons().Contains(EKeys::LeftMouseButton);

	// 处理滑块拖拽
	if (bIsMousePressed)
	{
		if (!bWasMousePressed)
		{
			// 按下瞬间：检查是否点击在某个滑块区域
			for (int32 i = 0; i < SettingsSliders.Num(); ++i)
			{
				const auto& S = SettingsSliders[i];
				const float HitPadding = 20.0f;
				if (MousePos.X >= S.BarPosition.X - 10.0f && MousePos.X <= S.BarPosition.X + S.BarSize.X + 10.0f &&
				    MousePos.Y >= S.BarPosition.Y - HitPadding && MousePos.Y <= S.BarPosition.Y + S.BarSize.Y + HitPadding)
				{
					DraggingSliderIndex = i;
					break;
				}
			}
		}

		if (DraggingSliderIndex >= 0 && DraggingSliderIndex < SettingsSliders.Num())
		{
			const auto& S = SettingsSliders[DraggingSliderIndex];
			float NewVal = (MousePos.X - S.BarPosition.X) / S.BarSize.X;
			*S.Value = FMath::Clamp(NewVal, 0.0f, 1.0f);
		}
	}
	else
	{
		if (DraggingSliderIndex >= 0)
		{
			ApplySettings();
		}
		DraggingSliderIndex = -1;
	}

	// 绘制滑块
	for (int32 i = 0; i < SettingsSliders.Num(); ++i)
	{
		const auto& S = SettingsSliders[i];

		// 标签
		float LW, LH;
		Canvas->TextSize(Font, *S.Label, LW, LH);
		FCanvasTextItem LabelItem(FVector2D(S.BarPosition.X, S.BarPosition.Y - 30.0f), FText::FromString(S.Label), Font, FLinearColor::White);
		LabelItem.Scale = FVector2D(LabelScale);
		Canvas->DrawItem(LabelItem);

		// 百分比
		FString PctStr = FString::Printf(TEXT("%d%%"), FMath::RoundToInt(*S.Value * 100.0f));
		float PW, PH;
		Canvas->TextSize(Font, *PctStr, PW, PH);
		FCanvasTextItem PctItem(FVector2D(S.BarPosition.X + S.BarSize.X + 15.0f, S.BarPosition.Y - 5.0f), FText::FromString(PctStr), Font, FLinearColor::White);
		PctItem.Scale = FVector2D(LabelScale);
		Canvas->DrawItem(PctItem);

		// 背景条
		FCanvasTileItem BarBg(S.BarPosition, S.BarSize, FLinearColor(0.3f, 0.3f, 0.3f, 0.8f));
		Canvas->DrawItem(BarBg);

		// 填充条
		float FillWidth = S.BarSize.X * (*S.Value);
		if (FillWidth > 0.0f)
		{
			FCanvasTileItem BarFill(S.BarPosition, FVector2D(FillWidth, S.BarSize.Y), FLinearColor(0.8f, 0.6f, 0.2f, 1.0f));
			Canvas->DrawItem(BarFill);
		}

		// 滑块手柄
		float HandleX = S.BarPosition.X + FillWidth - 6.0f;
		FCanvasTileItem Handle(FVector2D(HandleX, S.BarPosition.Y - 4.0f), FVector2D(12.0f, S.BarSize.Y + 8.0f), FLinearColor::White);
		Canvas->DrawItem(Handle);
	}

	// 全屏切换
	const float ToggleY = ContentStartY + ItemSpacing * 3;
	{
		FCanvasTextItem FullscreenLabel(FVector2D(SliderX, ToggleY), FText::FromString(TEXT("全屏模式")), Font, FLinearColor::White);
		FullscreenLabel.Scale = FVector2D(LabelScale);
		Canvas->DrawItem(FullscreenLabel);

		FString ToggleStr = SettingFullscreen ? TEXT("[开]") : TEXT("[关]");
		FLinearColor ToggleColor = SettingFullscreen ? FLinearColor(0.8f, 0.6f, 0.2f, 1.0f) : FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
		float TW, TH;
		Canvas->TextSize(Font, *ToggleStr, TW, TH);
		FVector2D TogglePos(SliderX + 200.0f, ToggleY);
		FCanvasTextItem ToggleItem(TogglePos, FText::FromString(ToggleStr), Font, ToggleColor);
		ToggleItem.Scale = FVector2D(LabelScale);
		Canvas->DrawItem(ToggleItem);

		// 点击切换
		if (bIsMousePressed && !bWasMousePressed)
		{
			if (MousePos.X >= TogglePos.X && MousePos.X <= TogglePos.X + TW * LabelScale + 30.0f &&
			    MousePos.Y >= ToggleY - 5.0f && MousePos.Y <= ToggleY + TH * LabelScale + 5.0f)
			{
				SettingFullscreen = !SettingFullscreen;
				ApplySettings();
			}
		}
	}

	// 返回按钮
	const float BackY = Canvas->SizeY - 120.0f;
	const float BackBtnW = 250.0f;
	const float BackBtnH = 60.0f;
	const FVector2D BackPos(CenterX - BackBtnW * 0.5f, BackY);
	const FVector2D BackSize(BackBtnW, BackBtnH);

	bool bBackHovered = MousePos.X >= BackPos.X && MousePos.X <= BackPos.X + BackSize.X &&
	                    MousePos.Y >= BackPos.Y && MousePos.Y <= BackPos.Y + BackSize.Y;

	FLinearColor BackBg = bBackHovered ? FLinearColor(0.8f, 0.6f, 0.2f, 0.8f) : FLinearColor(0.1f, 0.1f, 0.1f, 0.7f);
	FCanvasTileItem BackBgItem(BackPos, BackSize, BackBg);
	Canvas->DrawItem(BackBgItem);

	FLinearColor BackBorder = bBackHovered ? FLinearColor::White : FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
	FCanvasLineItem BT(BackPos, BackPos + FVector2D(BackSize.X, 0)); BT.SetColor(BackBorder); BT.LineThickness = 2; Canvas->DrawItem(BT);
	FCanvasLineItem BB(BackPos + FVector2D(0, BackSize.Y), BackPos + BackSize); BB.SetColor(BackBorder); BB.LineThickness = 2; Canvas->DrawItem(BB);
	FCanvasLineItem BL(BackPos, BackPos + FVector2D(0, BackSize.Y)); BL.SetColor(BackBorder); BL.LineThickness = 2; Canvas->DrawItem(BL);
	FCanvasLineItem BR(BackPos + FVector2D(BackSize.X, 0), BackPos + BackSize); BR.SetColor(BackBorder); BR.LineThickness = 2; Canvas->DrawItem(BR);

	{
		float TW, TH;
		Canvas->TextSize(Font, TEXT("返回"), TW, TH);
		FCanvasTextItem BackText(FVector2D(BackPos.X + (BackSize.X - TW) * 0.5f, BackPos.Y + (BackSize.Y - TH) * 0.5f),
			FText::FromString(TEXT("返回")), Font, FLinearColor::White);
		BackText.Scale = FVector2D(LabelScale);
		Canvas->DrawItem(BackText);
	}

	if (bIsMousePressed && !bWasMousePressed && bBackHovered)
	{
		ApplySettings();
		CurrentState = EMenuState::Main;
	}

	bWasMousePressed = bIsMousePressed;
}

void AMainMenuHUD::ApplySettings()
{
	if (UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		EWindowMode::Type DesiredMode = SettingFullscreen ? EWindowMode::Fullscreen : EWindowMode::Windowed;
		if (UserSettings->GetFullscreenMode() != DesiredMode)
		{
			UserSettings->SetFullscreenMode(DesiredMode);
			UserSettings->ApplySettings(false);
		}
	}

	if (GEngine)
	{
		FAudioDeviceHandle AudioDevice = GEngine->GetMainAudioDevice();
		if (AudioDevice)
		{
			AudioDevice->SetTransientPrimaryVolume(SettingMasterVolume);
		}
	}
}

// ============== 制作人员界面 ==============

void AMainMenuHUD::DrawCreditsScreen()
{
	if (!Canvas) return;

	FCanvasTileItem BgItem(FVector2D(0, 0), FVector2D(Canvas->SizeX, Canvas->SizeY), FLinearColor(0, 0, 0, 0.9f));
	Canvas->DrawItem(BgItem);

	UFont* TitleFont = GEngine ? GEngine->GetLargeFont() : nullptr;
	UFont* Font = GEngine ? GEngine->GetMediumFont() : nullptr;
	if (!Font || !TitleFont) return;

	const float CenterX = Canvas->SizeX * 0.5f;
	const float CenterY = Canvas->SizeY * 0.5f;
	const float LineHeight = 40.0f;
	const FLinearColor TextColor = FLinearColor::White;

	TArray<FString> CreditLines = {
		TEXT("制作人员"),
		TEXT(""),
		TEXT("南开大学"),
		TEXT("2512921 贾文一"),
		TEXT(""),
		TEXT("借助AI:"),
		TEXT("GPT / Claude / 即梦"),
		TEXT(""),
		TEXT(""),
		TEXT("点击任意位置返回")
	};

	float TotalHeight = CreditLines.Num() * LineHeight;
	float StartY = CenterY - TotalHeight * 0.5f;
	float CurrentY = StartY;

	for (int32 i = 0; i < CreditLines.Num(); ++i)
	{
		const FString& Line = CreditLines[i];
		if (!Line.IsEmpty())
		{
			UFont* CurrentFont = (i == 0) ? TitleFont : Font;
			float TextWidth, TextHeight;
			Canvas->TextSize(CurrentFont, *Line, TextWidth, TextHeight);

			float Scale = (i == 0) ? 2.5f : 1.8f;
			FVector2D TextPosition(CenterX - TextWidth * Scale * 0.5f, CurrentY);

			FCanvasTextItem TextItem(TextPosition, FText::FromString(Line), CurrentFont, TextColor);
			TextItem.Scale = FVector2D(Scale);
			Canvas->DrawItem(TextItem);
		}
		CurrentY += LineHeight;
	}
}

// ============== 其他 ==============

bool AMainMenuHUD::CheckHasSaveGame() const
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
