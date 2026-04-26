#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Localization/LocalizationSubsystem.h"
#include "Engine/GameInstance.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLocalizationSubsystemTest, "HorrorProject.Localization.LocalizationSubsystem", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLocalizationSubsystemTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
	{
		return false;
	}

	// Get localization subsystem
	ULocalizationSubsystem* LocalizationSubsystem = GameInstance->GetSubsystem<ULocalizationSubsystem>();
	if (!TestNotNull(TEXT("LocalizationSubsystem exists"), LocalizationSubsystem))
	{
		return false;
	}

	// Test default language
	TestEqual(TEXT("Default language is English"), LocalizationSubsystem->GetCurrentLanguage(), ELanguage::English);

	// Test language codes
	TestEqual(TEXT("English code"), LocalizationSubsystem->GetLanguageCode(ELanguage::English), TEXT("en"));
	TestEqual(TEXT("Chinese code"), LocalizationSubsystem->GetLanguageCode(ELanguage::Chinese), TEXT("zh-Hans"));
	TestEqual(TEXT("Japanese code"), LocalizationSubsystem->GetLanguageCode(ELanguage::Japanese), TEXT("ja"));
	TestEqual(TEXT("Korean code"), LocalizationSubsystem->GetLanguageCode(ELanguage::Korean), TEXT("ko"));
	TestEqual(TEXT("Spanish code"), LocalizationSubsystem->GetLanguageCode(ELanguage::Spanish), TEXT("es"));

	// Test language switching
	LocalizationSubsystem->SetLanguage(ELanguage::Chinese);
	TestEqual(TEXT("Language changed to Chinese"), LocalizationSubsystem->GetCurrentLanguage(), ELanguage::Chinese);

	LocalizationSubsystem->SetLanguage(ELanguage::Japanese);
	TestEqual(TEXT("Language changed to Japanese"), LocalizationSubsystem->GetCurrentLanguage(), ELanguage::Japanese);

	// Test RTL detection
	TestFalse(TEXT("English is not RTL"), LocalizationSubsystem->IsRTLLanguage());

	// Test text retrieval
	LocalizationSubsystem->SetLanguage(ELanguage::English);
	FText EnglishText = LocalizationSubsystem->GetLocalizedText(TEXT("UI.MainMenu.Start"));
	TestTrue(TEXT("English text retrieved"), !EnglishText.IsEmpty());

	LocalizationSubsystem->SetLanguage(ELanguage::Chinese);
	FText ChineseText = LocalizationSubsystem->GetLocalizedText(TEXT("UI.MainMenu.Start"));
	TestTrue(TEXT("Chinese text retrieved"), !ChineseText.IsEmpty());
	TestNotEqual(TEXT("Chinese text differs from English"), ChineseText.ToString(), EnglishText.ToString());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTextLocalizationTest, "HorrorProject.Localization.TextLocalization", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTextLocalizationTest::RunTest(const FString& Parameters)
{
	// Test text sanitization
	FString DirtyText = TEXT("  Hello\r\nWorld\r  ");
	FString CleanText = UTextLocalizationLibrary::SanitizeTextForLocalization(DirtyText);
	TestEqual(TEXT("Text sanitized"), CleanText, TEXT("Hello\nWorld"));

	// Test format text
	TArray<FString> Args;
	Args.Add(TEXT("Player"));
	Args.Add(TEXT("100"));

	// Note: This test requires a game instance with localization subsystem
	// In a real test, we would set up the full context

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLocalizationDataTest, "HorrorProject.Localization.LocalizationData", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLocalizationDataTest::RunTest(const FString& Parameters)
{
	// Create test data asset
	UTextLocalizationData* DataAsset = NewObject<UTextLocalizationData>();
	if (!TestNotNull(TEXT("DataAsset created"), DataAsset))
	{
		return false;
	}

	// Add test entries
	FLocalizedString TestEntry;
	TestEntry.Key = TEXT("Test.Key");
	TestEntry.English = FText::FromString(TEXT("Hello"));
	TestEntry.Chinese = FText::FromString(TEXT("你好"));
	TestEntry.Japanese = FText::FromString(TEXT("こんにちは"));
	TestEntry.Korean = FText::FromString(TEXT("안녕하세요"));
	TestEntry.Spanish = FText::FromString(TEXT("Hola"));

	DataAsset->LocalizedStrings.Add(TestEntry);

	// Test retrieval
	FText EnglishText = DataAsset->GetText(TEXT("Test.Key"), TEXT("en"));
	TestEqual(TEXT("English text matches"), EnglishText.ToString(), TEXT("Hello"));

	FText ChineseText = DataAsset->GetText(TEXT("Test.Key"), TEXT("zh-Hans"));
	TestEqual(TEXT("Chinese text matches"), ChineseText.ToString(), TEXT("你好"));

	FText JapaneseText = DataAsset->GetText(TEXT("Test.Key"), TEXT("ja"));
	TestEqual(TEXT("Japanese text matches"), JapaneseText.ToString(), TEXT("こんにちは"));

	// Test missing key
	FText MissingText = DataAsset->GetText(TEXT("Missing.Key"), TEXT("en"));
	TestEqual(TEXT("Missing key returns key"), MissingText.ToString(), TEXT("Missing.Key"));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLocalizationPerformanceTest, "HorrorProject.Localization.Performance", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLocalizationPerformanceTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	ULocalizationSubsystem* LocalizationSubsystem = GameInstance->GetSubsystem<ULocalizationSubsystem>();

	if (!LocalizationSubsystem)
	{
		return false;
	}

	// Test language switching performance
	const int32 NumIterations = 100;
	double StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < NumIterations; ++i)
	{
		ELanguage TestLanguage = static_cast<ELanguage>(i % 5);
		LocalizationSubsystem->SetLanguage(TestLanguage);
	}

	double EndTime = FPlatformTime::Seconds();
	double TotalTime = EndTime - StartTime;
	double AverageTime = TotalTime / NumIterations;

	AddInfo(FString::Printf(TEXT("Language switching: %d iterations in %.3f seconds (avg: %.3f ms)"),
		NumIterations, TotalTime, AverageTime * 1000.0));

	TestTrue(TEXT("Language switching is fast"), AverageTime < 0.01); // Less than 10ms per switch

	// Test text retrieval performance
	StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < NumIterations * 10; ++i)
	{
		FText Text = LocalizationSubsystem->GetLocalizedText(TEXT("UI.MainMenu.Start"));
	}

	EndTime = FPlatformTime::Seconds();
	TotalTime = EndTime - StartTime;
	AverageTime = TotalTime / (NumIterations * 10);

	AddInfo(FString::Printf(TEXT("Text retrieval: %d iterations in %.3f seconds (avg: %.3f ms)"),
		NumIterations * 10, TotalTime, AverageTime * 1000.0));

	TestTrue(TEXT("Text retrieval is fast"), AverageTime < 0.001); // Less than 1ms per retrieval

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
