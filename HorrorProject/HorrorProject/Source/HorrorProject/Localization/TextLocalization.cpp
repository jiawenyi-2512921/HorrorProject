// Copyright Epic Games, Inc. All Rights Reserved.

#include "TextLocalization.h"
#include "LocalizationSubsystem.h"
#include "HAL/CriticalSection.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ScopeLock.h"

namespace
{
	FCriticalSection DynamicTextsCriticalSection;
	TMap<FString, FText> DynamicTexts;
}

FText UTextLocalizationLibrary::GetLocalizedText(const FString& Namespace, const FString& Key)
{
	FString FullKey = Namespace.IsEmpty() ? Key : FString::Printf(TEXT("%s.%s"), *Namespace, *Key);

	{
		FScopeLock Lock(&DynamicTextsCriticalSection);
		if (const FText* DynamicText = DynamicTexts.Find(FullKey))
		{
			return *DynamicText;
		}
	}

	UWorld* World = GEngine ? GEngine->GetWorld() : nullptr;
	if (World)
	{
		if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(World))
		{
			if (ULocalizationSubsystem* LocalizationSubsystem = GameInstance->GetSubsystem<ULocalizationSubsystem>())
			{
				return LocalizationSubsystem->GetLocalizedText(FullKey);
			}
		}
	}

	return FText::FromString(Key);
}

FText UTextLocalizationLibrary::FormatLocalizedText(const FString& Key, const TArray<FString>& Arguments)
{
	FText BaseText = GetLocalizedText(TEXT(""), Key);

	FFormatOrderedArguments OrderedArguments;
	for (const FString& Argument : Arguments)
	{
		OrderedArguments.Add(FText::FromString(Argument));
	}

	return FText::Format(BaseText, OrderedArguments);
}

FText UTextLocalizationLibrary::GetPluralText(const FString& Key, int32 Count)
{
	FString PluralKey = Count == 1 ? Key + TEXT(".Singular") : Key + TEXT(".Plural");
	FText BaseText = GetLocalizedText(TEXT(""), PluralKey);

	return FText::Format(BaseText, FText::AsNumber(Count));
}

void UTextLocalizationLibrary::RegisterDynamicText(const FString& Key, const FText& Text)
{
	FScopeLock Lock(&DynamicTextsCriticalSection);
	DynamicTexts.Add(Key, Text);
}

FString UTextLocalizationLibrary::SanitizeTextForLocalization(const FString& Text)
{
	FString Sanitized = Text;
	Sanitized = Sanitized.TrimStartAndEnd();
	Sanitized = Sanitized.Replace(TEXT("\r\n"), TEXT("\n"));
	Sanitized = Sanitized.Replace(TEXT("\r"), TEXT("\n"));
	return Sanitized;
}

FText UTextLocalizationData::GetText(const FString& Key, const FString& LanguageCode) const
{
	for (const FLocalizedString& LocalizedString : LocalizedStrings)
	{
		if (LocalizedString.Key == Key)
		{
			if (LanguageCode == TEXT("en"))
				return LocalizedString.English;
			else if (LanguageCode == TEXT("zh-Hans"))
				return LocalizedString.Chinese;
			else if (LanguageCode == TEXT("ja"))
				return LocalizedString.Japanese;
			else if (LanguageCode == TEXT("ko"))
				return LocalizedString.Korean;
			else if (LanguageCode == TEXT("es"))
				return LocalizedString.Spanish;
		}
	}
	return FText::FromString(Key);
}
