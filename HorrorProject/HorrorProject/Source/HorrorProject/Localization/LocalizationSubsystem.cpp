// Copyright Epic Games, Inc. All Rights Reserved.

#include "LocalizationSubsystem.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"
#include "Kismet/GameplayStatics.h"

void ULocalizationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentLanguage = ELanguage::English;
	LoadLanguageData(CurrentLanguage);
}

void ULocalizationSubsystem::Deinitialize()
{
	LocalizedTexts.Empty();
	Super::Deinitialize();
}

void ULocalizationSubsystem::SetLanguage(ELanguage NewLanguage)
{
	if (CurrentLanguage == NewLanguage)
	{
		return;
	}

	CurrentLanguage = NewLanguage;
	LoadLanguageData(NewLanguage);
	ApplyLanguageSettings();

	OnLanguageChanged.Broadcast(NewLanguage);
}

FString ULocalizationSubsystem::GetLanguageCode(ELanguage Language) const
{
	switch (Language)
	{
	case ELanguage::English:
		return TEXT("en");
	case ELanguage::Chinese:
		return TEXT("zh-Hans");
	case ELanguage::Japanese:
		return TEXT("ja");
	case ELanguage::Korean:
		return TEXT("ko");
	case ELanguage::Spanish:
		return TEXT("es");
	default:
		return TEXT("en");
	}
}

FText ULocalizationSubsystem::GetLocalizedText(const FString& Key) const
{
	if (const FText* FoundText = LocalizedTexts.Find(Key))
	{
		return *FoundText;
	}
	return FText::FromString(Key);
}

bool ULocalizationSubsystem::IsRTLLanguage() const
{
	// None of our supported languages are RTL
	return false;
}

void ULocalizationSubsystem::LoadLanguageData(ELanguage Language)
{
	LocalizedTexts.Empty();

	FString LanguageCode = GetLanguageCode(Language);
	FString LocalizationPath = FPaths::ProjectContentDir() / TEXT("Localization") / LanguageCode;

	// Load localization data from files
	// This is a simplified version - in production, load from data tables or JSON

	// Example entries
	switch (Language)
	{
	case ELanguage::English:
		LocalizedTexts.Add(TEXT("UI.MainMenu.Start"), FText::FromString(TEXT("Start Game")));
		LocalizedTexts.Add(TEXT("UI.MainMenu.Options"), FText::FromString(TEXT("Options")));
		LocalizedTexts.Add(TEXT("UI.MainMenu.Quit"), FText::FromString(TEXT("Quit")));
		break;
	case ELanguage::Chinese:
		LocalizedTexts.Add(TEXT("UI.MainMenu.Start"), FText::FromString(TEXT("开始游戏")));
		LocalizedTexts.Add(TEXT("UI.MainMenu.Options"), FText::FromString(TEXT("选项")));
		LocalizedTexts.Add(TEXT("UI.MainMenu.Quit"), FText::FromString(TEXT("退出")));
		break;
	case ELanguage::Japanese:
		LocalizedTexts.Add(TEXT("UI.MainMenu.Start"), FText::FromString(TEXT("ゲーム開始")));
		LocalizedTexts.Add(TEXT("UI.MainMenu.Options"), FText::FromString(TEXT("オプション")));
		LocalizedTexts.Add(TEXT("UI.MainMenu.Quit"), FText::FromString(TEXT("終了")));
		break;
	case ELanguage::Korean:
		LocalizedTexts.Add(TEXT("UI.MainMenu.Start"), FText::FromString(TEXT("게임 시작")));
		LocalizedTexts.Add(TEXT("UI.MainMenu.Options"), FText::FromString(TEXT("옵션")));
		LocalizedTexts.Add(TEXT("UI.MainMenu.Quit"), FText::FromString(TEXT("종료")));
		break;
	case ELanguage::Spanish:
		LocalizedTexts.Add(TEXT("UI.MainMenu.Start"), FText::FromString(TEXT("Iniciar Juego")));
		LocalizedTexts.Add(TEXT("UI.MainMenu.Options"), FText::FromString(TEXT("Opciones")));
		LocalizedTexts.Add(TEXT("UI.MainMenu.Quit"), FText::FromString(TEXT("Salir")));
		break;
	}
}

void ULocalizationSubsystem::ApplyLanguageSettings()
{
	FString LanguageCode = GetLanguageCode(CurrentLanguage);
	FInternationalization::Get().SetCurrentCulture(LanguageCode);
}
