// Copyright Epic Games, Inc. All Rights Reserved.

#include "LocalizationSubsystem.h"
#include "Dom/JsonObject.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
	const FText& GetMissingKeyFallbackText()
	{
		static const FText MissingKeyFallbackText = FText::FromString(TEXT("文本缺失"));
		return MissingKeyFallbackText;
	}

	void AddFallbackMainMenuTexts(TMap<FString, FText>& LocalizedTexts)
	{
		LocalizedTexts.Add(TEXT("UI.MainMenu.Start"), FText::FromString(TEXT("开始游戏")));
		LocalizedTexts.Add(TEXT("UI.MainMenu.Options"), FText::FromString(TEXT("选项")));
		LocalizedTexts.Add(TEXT("UI.MainMenu.Quit"), FText::FromString(TEXT("退出")));
	}
}

void ULocalizationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentLanguage = ELanguage::Chinese;
	LoadLanguageData(CurrentLanguage);
	ApplyLanguageSettings();
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
	return GetMissingKeyFallbackText();
}

bool ULocalizationSubsystem::HasLocalizedText(const FString& Key) const
{
	return LocalizedTexts.Contains(Key);
}

bool ULocalizationSubsystem::IsRTLLanguage() const
{
	return false;
}

void ULocalizationSubsystem::LoadLanguageData(ELanguage Language)
{
	LocalizedTexts.Empty();

	const FString LanguageCode = GetLanguageCode(Language);
	const FString LocalizationPath = FPaths::ProjectContentDir() / TEXT("Localization") / LanguageCode / TEXT("Game.json");

	FString JsonText;
	if (!FFileHelper::LoadFileToString(JsonText, *LocalizationPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Localization file missing for language '%s': %s"), *LanguageCode, *LocalizationPath);
		AddFallbackMainMenuTexts(LocalizedTexts);
		return;
	}

	TSharedPtr<FJsonObject> JsonObject;
	const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonText);
	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Localization file could not be parsed for language '%s': %s"), *LanguageCode, *LocalizationPath);
		AddFallbackMainMenuTexts(LocalizedTexts);
		return;
	}

	for (const TPair<FString, TSharedPtr<FJsonValue>>& Entry : JsonObject->Values)
	{
		FString LocalizedString;
		if (!Entry.Key.IsEmpty() && Entry.Value.IsValid() && Entry.Value->TryGetString(LocalizedString))
		{
			LocalizedTexts.Add(Entry.Key, FText::FromString(LocalizedString));
		}
	}

	if (LocalizedTexts.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Localization file had no string entries for language '%s': %s"), *LanguageCode, *LocalizationPath);
		AddFallbackMainMenuTexts(LocalizedTexts);
	}
}

void ULocalizationSubsystem::ApplyLanguageSettings()
{
	const FString LanguageCode = GetLanguageCode(CurrentLanguage);
	FInternationalization::Get().SetCurrentCulture(LanguageCode);
}
