// Copyright Epic Games, Inc. All Rights Reserved.

#include "UILocalization.h"
#include "LocalizationSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

void UUILocalizationComponent::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (UGameInstance* GameInstance = World->GetGameInstance())
	{
		if (ULocalizationSubsystem* LocalizationSubsystem = GameInstance->GetSubsystem<ULocalizationSubsystem>())
		{
			LocalizationSubsystem->OnLanguageChanged.AddDynamic(this, &UUILocalizationComponent::OnLanguageChanged);
			UpdateLocalization();
		}
	}
}

void UUILocalizationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UWorld* World = GetWorld();
	if (World)
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (ULocalizationSubsystem* LocalizationSubsystem = GameInstance->GetSubsystem<ULocalizationSubsystem>())
			{
				LocalizationSubsystem->OnLanguageChanged.RemoveDynamic(this, &UUILocalizationComponent::OnLanguageChanged);
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UUILocalizationComponent::UpdateLocalization()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (UGameInstance* GameInstance = World->GetGameInstance())
	{
		if (ULocalizationSubsystem* LocalizationSubsystem = GameInstance->GetSubsystem<ULocalizationSubsystem>())
		{
			// Update text blocks
			for (const FLocalizedTextWidget& TextWidget : LocalizedTextWidgets)
			{
				if (TextWidget.TextBlock)
				{
					FText LocalizedText = LocalizationSubsystem->GetLocalizedText(TextWidget.LocalizationKey);
					TextWidget.TextBlock->SetText(LocalizedText);
				}
			}

			// Update images
			const FString LanguageCode = LocalizationSubsystem->GetLanguageCode(LocalizationSubsystem->GetCurrentLanguage());
			for (const FLocalizedImageWidget& ImageWidget : LocalizedImageWidgets)
			{
				if (ImageWidget.Image)
				{
					UTexture2D* LocalizedTexture = GetLocalizedTexture(ImageWidget.LocalizationKey, LanguageCode);
					if (LocalizedTexture)
					{
						ImageWidget.Image->SetBrushFromTexture(LocalizedTexture);
					}
				}
			}
		}
	}
}

void UUILocalizationComponent::OnLanguageChanged(ELanguage NewLanguage)
{
	UpdateLocalization();
}

UTexture2D* UUILocalizationComponent::GetLocalizedTexture(const FString& Key, const FString& LanguageCode) const
{
	// In production, load from data asset or asset manager
	return nullptr;
}

void UUILocalizationLibrary::LocalizeTextBlock(UTextBlock* TextBlock, const FString& LocalizationKey)
{
	if (!TextBlock)
	{
		return;
	}

	UWorld* World = TextBlock->GetWorld();
	if (!World)
	{
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	ULocalizationSubsystem* LocalizationSubsystem = GameInstance->GetSubsystem<ULocalizationSubsystem>();
	if (!LocalizationSubsystem)
	{
		return;
	}

	FText LocalizedText = LocalizationSubsystem->GetLocalizedText(LocalizationKey);
	TextBlock->SetText(LocalizedText);
}

void UUILocalizationLibrary::LocalizeImage(UImage* Image, const FString& LocalizationKey)
{
	if (!Image)
	{
		return;
	}

	// In production, load localized texture based on current language
}

FString UUILocalizationLibrary::GetFontPathForLanguage(ELanguage Language)
{
	switch (Language)
	{
	case ELanguage::English:
		return TEXT("/Game/Fonts/Roboto");
	case ELanguage::Chinese:
		return TEXT("/Game/Fonts/NotoSansCJK");
	case ELanguage::Japanese:
		return TEXT("/Game/Fonts/NotoSansCJK");
	case ELanguage::Korean:
		return TEXT("/Game/Fonts/NotoSansCJK");
	case ELanguage::Spanish:
		return TEXT("/Game/Fonts/Roboto");
	default:
		return TEXT("/Game/Fonts/Roboto");
	}
}

float UUILocalizationLibrary::GetTextScaleForLanguage(ELanguage Language)
{
	switch (Language)
	{
	case ELanguage::Chinese:
	case ELanguage::Japanese:
	case ELanguage::Korean:
		return 1.1f; // CJK characters may need slightly larger size
	default:
		return 1.0f;
	}
}
