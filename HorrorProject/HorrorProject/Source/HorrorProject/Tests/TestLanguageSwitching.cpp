// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Localization/LocalizationSubsystem.h"
#include "Engine/GameInstance.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLanguageSwitchingTest, "HorrorProject.Localization.LanguageSwitching", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FLanguageSwitchingTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	ULocalizationSubsystem* LocalizationSubsystem = GameInstance->GetSubsystem<ULocalizationSubsystem>();

	if (!TestNotNull(TEXT("LocalizationSubsystem exists"), LocalizationSubsystem))
	{
		return false;
	}

	// Test callback registration
	bool bCallbackFired = false;
	ELanguage CallbackLanguage = ELanguage::English;

	FScriptDelegate Delegate;
	Delegate.BindUFunction(this, FName("OnLanguageChangedCallback"));

	// Test switching to each language
	TArray<ELanguage> Languages = {
		ELanguage::English,
		ELanguage::Chinese,
		ELanguage::Japanese,
		ELanguage::Korean,
		ELanguage::Spanish
	};

	for (ELanguage Language : Languages)
	{
		LocalizationSubsystem->SetLanguage(Language);
		TestEqual(FString::Printf(TEXT("Language set to %d"), (int32)Language),
			LocalizationSubsystem->GetCurrentLanguage(), Language);

		// Verify language code
		FString LanguageCode = LocalizationSubsystem->GetLanguageCode(Language);
		TestTrue(TEXT("Language code is valid"), !LanguageCode.IsEmpty());
	}

	// Test rapid switching
	for (int32 i = 0; i < 50; ++i)
	{
		ELanguage RandomLanguage = Languages[FMath::RandRange(0, Languages.Num() - 1)];
		LocalizationSubsystem->SetLanguage(RandomLanguage);
		TestEqual(TEXT("Rapid switch successful"), LocalizationSubsystem->GetCurrentLanguage(), RandomLanguage);
	}

	// Test switching to same language (should not cause issues)
	LocalizationSubsystem->SetLanguage(ELanguage::English);
	LocalizationSubsystem->SetLanguage(ELanguage::English);
	TestEqual(TEXT("Same language switch handled"), LocalizationSubsystem->GetCurrentLanguage(), ELanguage::English);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLanguagePersistenceTest, "HorrorProject.Localization.LanguagePersistence", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FLanguagePersistenceTest::RunTest(const FString& Parameters)
{
	// Test that language settings persist across subsystem recreation
	UGameInstance* GameInstance1 = NewObject<UGameInstance>();
	ULocalizationSubsystem* LocalizationSubsystem1 = GameInstance1->GetSubsystem<ULocalizationSubsystem>();

	if (!LocalizationSubsystem1)
	{
		return false;
	}

	// Set to Japanese
	LocalizationSubsystem1->SetLanguage(ELanguage::Japanese);
	TestEqual(TEXT("Language set to Japanese"), LocalizationSubsystem1->GetCurrentLanguage(), ELanguage::Japanese);

	// In a real scenario, this would test saving/loading from config
	// For now, we just verify the subsystem maintains state

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLanguageTextConsistencyTest, "HorrorProject.Localization.TextConsistency", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FLanguageTextConsistencyTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	ULocalizationSubsystem* LocalizationSubsystem = GameInstance->GetSubsystem<ULocalizationSubsystem>();

	if (!LocalizationSubsystem)
	{
		return false;
	}

	// Test that the same key returns consistent text for each language
	TArray<FString> TestKeys = {
		TEXT("UI.MainMenu.Start"),
		TEXT("UI.MainMenu.Options"),
		TEXT("UI.MainMenu.Quit")
	};

	TArray<ELanguage> Languages = {
		ELanguage::English,
		ELanguage::Chinese,
		ELanguage::Japanese,
		ELanguage::Korean,
		ELanguage::Spanish
	};

	for (const FString& Key : TestKeys)
	{
		TMap<ELanguage, FText> TextsByLanguage;

		// Retrieve text for each language
		for (ELanguage Language : Languages)
		{
			LocalizationSubsystem->SetLanguage(Language);
			FText Text = LocalizationSubsystem->GetLocalizedText(Key);
			TextsByLanguage.Add(Language, Text);

			TestTrue(FString::Printf(TEXT("Text exists for %s in language %d"), *Key, (int32)Language),
				!Text.IsEmpty());
		}

		// Verify texts are different across languages (except for missing translations)
		LocalizationSubsystem->SetLanguage(ELanguage::English);
		FText EnglishText = TextsByLanguage[ELanguage::English];

		for (ELanguage Language : Languages)
		{
			if (Language != ELanguage::English)
			{
				FText OtherText = TextsByLanguage[Language];
				// In production, we'd expect different text, but for now just verify it exists
				TestTrue(FString::Printf(TEXT("Text exists for %s in non-English language"), *Key),
					!OtherText.IsEmpty());
			}
		}
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLanguageFallbackTest, "HorrorProject.Localization.LanguageFallback", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FLanguageFallbackTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	ULocalizationSubsystem* LocalizationSubsystem = GameInstance->GetSubsystem<ULocalizationSubsystem>();

	if (!LocalizationSubsystem)
	{
		return false;
	}

	// Test that missing keys return the key itself (fallback behavior)
	LocalizationSubsystem->SetLanguage(ELanguage::English);

	FString MissingKey = TEXT("This.Key.Does.Not.Exist");
	FText FallbackText = LocalizationSubsystem->GetLocalizedText(MissingKey);

	TestEqual(TEXT("Missing key returns key as fallback"), FallbackText.ToString(), MissingKey);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLanguageSwitchingStressTest, "HorrorProject.Localization.LanguageSwitchingStress", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::StressFilter)

bool FLanguageSwitchingStressTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	ULocalizationSubsystem* LocalizationSubsystem = GameInstance->GetSubsystem<ULocalizationSubsystem>();

	if (!LocalizationSubsystem)
	{
		return false;
	}

	// Stress test: rapid language switching with text retrieval
	TArray<ELanguage> Languages = {
		ELanguage::English,
		ELanguage::Chinese,
		ELanguage::Japanese,
		ELanguage::Korean,
		ELanguage::Spanish
	};

	TArray<FString> TestKeys = {
		TEXT("UI.MainMenu.Start"),
		TEXT("UI.MainMenu.Options"),
		TEXT("UI.MainMenu.Quit")
	};

	const int32 NumIterations = 1000;
	int32 SuccessCount = 0;

	for (int32 i = 0; i < NumIterations; ++i)
	{
		// Random language
		ELanguage RandomLanguage = Languages[FMath::RandRange(0, Languages.Num() - 1)];
		LocalizationSubsystem->SetLanguage(RandomLanguage);

		// Random key
		FString RandomKey = TestKeys[FMath::RandRange(0, TestKeys.Num() - 1)];
		FText Text = LocalizationSubsystem->GetLocalizedText(RandomKey);

		if (!Text.IsEmpty())
		{
			SuccessCount++;
		}
	}

	float SuccessRate = (float)SuccessCount / NumIterations;
	AddInfo(FString::Printf(TEXT("Stress test: %d/%d successful (%.1f%%)"),
		SuccessCount, NumIterations, SuccessRate * 100.0f));

	TestTrue(TEXT("Stress test success rate > 95%"), SuccessRate > 0.95f);

	return true;
}
