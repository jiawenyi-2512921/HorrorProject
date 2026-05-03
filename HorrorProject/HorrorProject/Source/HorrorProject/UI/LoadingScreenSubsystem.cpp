// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/LoadingScreenSubsystem.h"
#include "Misc/CoreDelegates.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Engine/Engine.h"

void ULoadingScreenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	PreLoadHandle = FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &ULoadingScreenSubsystem::HandlePreLoadMap);
	PostLoadHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ULoadingScreenSubsystem::HandlePostLoadMap);
}

void ULoadingScreenSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PreLoadMap.Remove(PreLoadHandle);
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadHandle);
	HideLoadingScreen();
	Super::Deinitialize();
}

void ULoadingScreenSubsystem::HandlePreLoadMap(const FString& MapName)
{
	if (!FSlateApplication::IsInitialized() || !GEngine || !GEngine->GameViewport) return;

	LoadingWidget = SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderBackgroundColor(FLinearColor::Black)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("加载中...")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 36))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0, 20, 0, 0)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("请稍候")))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
						.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
					]
				]
			]
		];

	GEngine->GameViewport->AddViewportWidgetContent(LoadingWidget.ToSharedRef(), 100);
}

void ULoadingScreenSubsystem::HandlePostLoadMap(UWorld* World)
{
	HideLoadingScreen();
}

void ULoadingScreenSubsystem::HideLoadingScreen()
{
	if (LoadingWidget.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(LoadingWidget.ToSharedRef());
		LoadingWidget.Reset();
	}
}
