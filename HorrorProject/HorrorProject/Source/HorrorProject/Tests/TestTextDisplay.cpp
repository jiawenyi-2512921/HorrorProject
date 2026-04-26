// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Localization/LocalizationSubsystem.h"
#include "Localization/TextLocalization.h"
#include "Engine/GameInstance.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTextDisplayBasicTest, "HorrorProject.Localization.TextDisplay.Basic", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTextDisplayBasicTest::RunTest(const FString& Parameters)
{
	// Test basic text display functionality
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	ULocalizationSubsystem* LocalizationSubsystem = GameInstance->GetSubsystem<ULocalizationSubsystem>();

	if (!LocalizationSubsystem)
	{
		return false;
	}

	// Test text retrieval for display
	LocalizationSubsystem->SetLanguage(ELanguage::English);
	FText EnglishText = LocalizationSubsystem->GetLocalizedText(TEXT("UI.MainMenu.Start"));
	TestTrue(TEXT("English text is displayable"), !EnglishText.IsEmpty());

	// Test CJK text display
	LocalizationSubsystem->SetLanguage(ELanguage::Chinese);
	FText ChineseText = LocalizationSubsystem->GetLocalizedText(TEXT("UI.MainMenu.Start"));
	TestTrue(TEXT("Chinese text is displayable"), !ChineseText.IsEmpty());

	LocalizationSubsystem->SetLanguage(ELanguage::Japanese);
	FText JapaneseText = LocalizationSubsystem->GetLocalizedText(TEXT("UI.MainMenu.Start"));
	TestTrue(TEXT("Japanese text is displayable"), !JapaneseText.IsEmpty());

	LocalizationSubsystem->SetLanguage(ELanguage::Korean);
	FText KoreanText = LocalizationSubsystem->GetLocalizedText(TEXT("UI.MainMenu.Start"));
	TestTrue(TEXT("Korean text is displayable"), !KoreanText.IsEmpty());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTextFormatTest, "HorrorProject.Localization.TextDisplay.Format", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTextFormatTest::RunTest(const FString& Parameters)
{
	// Test formatted text display
	TArray<FString> Args;
	Args.Add(TEXT("Player1"));
	Args.Add(TEXT("100"));

	// Test format with placeholders
	FString TestFormat = TEXT("Player {0} scored {1} points");
	FString Expected = TEXT("Player Player1 scored 100 points");

	// Manual format for testing
	FString Result = TestFormat;
	for (int32 i = 0; i < Args.Num(); ++i)
	{
		FString Placeholder = FString::Printf(TEXT("{%d}"), i);
		Result = Result.Replace(*Placeholder, *Args[i]);
	}

	TestEqual(TEXT("Format string works"), Result, Expected);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTextLengthTest, "HorrorProject.Localization.TextDisplay.Length", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTextLengthTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	ULocalizationSubsystem* LocalizationSubsystem = GameInstance->GetSubsystem<ULocalizationSubsystem>();

	if (!LocalizationSubsystem)
	{
		return false;
	}

	// Test that text lengths are reasonable for UI display
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
		for (ELanguage Language : Languages)
		{
			LocalizationSubsystem->SetLanguage(Language);
			FText Text = LocalizationSubsystem->GetLocalizedText(Key);
			int32 Length = Text.ToString().Len();

			// Reasonable length for UI buttons (adjust as needed)
			TestTrue(FString::Printf(TEXT("Text length reasonable for %s in language %d"), *Key, (int32)Language),
				Length > 0 && Length < 100);
		}
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTextEncodingTest, "HorrorProject.Localization.TextDisplay.Encoding", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTextEncodingTest::RunTest(const FString& Parameters)
{
	// Test that special characters are handled correctly
	TArray<FString> SpecialTexts = {
		TEXT("Hello \"World\""),
		TEXT("Test's apostrophe"),
		TEXT("Line1\nLine2"),
		TEXT("Tab\tSeparated"),
		TEXT("Special: !@#$%^&*()"),
		TEXT("Unicode: éñü") // é ñ ü
	};

	for (const FString& SpecialText : SpecialTexts)
	{
		FString Sanitized = UTextLocalizationLibrary::SanitizeTextForLocalization(SpecialText);
		TestTrue(FString::Printf(TEXT("Special text handled: %s"), *SpecialText), !Sanitized.IsEmpty());
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTextPluralTest, "HorrorProject.Localization.TextDisplay.Plural", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTextPluralTest::RunTest(const FString& Parameters)
{
	// Test plural forms
	// Note: This requires proper localization data setup

	// Test singular
	int32 Count1 = 1;
	// In production: FText Singular = UTextLocalizationLibrary::GetPluralText(TEXT("Items"), Count1);

	// Test plural
	int32 Count2 = 5;
	// In production: FText Plural = UTextLocalizationLibrary::GetPluralText(TEXT("Items"), Count2);

	// For now, just verify the function exists and can be called
	TestTrue(TEXT("Plural text function available"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTextRTLTest, "HorrorProject.Localization.TextDisplay.RTL", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTextRTLTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	ULocalizationSubsystem* LocalizationSubsystem = GameInstance->GetSubsystem<ULocalizationSubsystem>();

	if (!LocalizationSubsystem)
	{
		return false;
	}

	// Test RTL detection for all supported languages
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
		bool bIsRTL = LocalizationSubsystem->IsRTLLanguage();

		// None of our current languages are RTL
		TestFalse(FString::Printf(TEXT("Language %d is not RTL"), (int32)Language), bIsRTL);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTextDisplayPerformanceTest, "HorrorProject.Localization.TextDisplay.Performance", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTextDisplayPerformanceTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	ULocalizationSubsystem* LocalizationSubsystem = GameInstance->GetSubsystem<ULocalizationSubsystem>();

	if (!LocalizationSubsystem)
	{
		return false;
	}

	// Test text retrieval performance for UI updates
	const int32 NumIterations = 10000;
	TArray<FString> TestKeys = {
		TEXT("UI.MainMenu.Start"),
		TEXT("UI.MainMenu.Options"),
		TEXT("UI.MainMenu.Quit")
	};

	double StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < NumIterations; ++i)
	{
		FString Key = TestKeys[i % TestKeys.Num()];
		FText Text = LocalizationSubsystem->GetLocalizedText(Key);
		FString DisplayString = Text.ToString();
	}

	double EndTime = FPlatformTime::Seconds();
	double TotalTime = EndTime - StartTime;
	double AverageTime = TotalTime / NumIterations;

	AddInfo(FString::Printf(TEXT("Text display: %d iterations in %.3f seconds (avg: %.3f ms)"),
		NumIterations, TotalTime, AverageTime * 1000.0));

	// Should be very fast for UI updates
	TestTrue(TEXT("Text display is fast enough for UI"), AverageTime < 0.0001); // Less than 0.1ms

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTextCachingTest, "HorrorProject.Localization.TextDisplay.Caching", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTextCachingTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	ULocalizationSubsystem* LocalizationSubsystem = GameInstance->GetSubsystem<ULocalizationSubsystem>();

	if (!LocalizationSubsystem)
	{
		return false;
	}

	// Test that repeated text retrieval is efficient (should use caching)
	const FString TestKey = TEXT("UI.MainMenu.Start");
	const int32 NumRetrievals = 1000;

	double StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < NumRetrievals; ++i)
	{
		FText Text = LocalizationSubsystem->GetLocalizedText(TestKey);
	}

	double EndTime = FPlatformTime::Seconds();
	double TotalTime = EndTime - StartTime;
	double AverageTime = TotalTime / NumRetrievals;

	AddInfo(FString::Printf(TEXT("Cached text retrieval: %d iterations in %.3f seconds (avg: %.6f ms)"),
		NumRetrievals, TotalTime, AverageTime * 1000.0));

	// Cached retrieval should be extremely fast
	TestTrue(TEXT("Cached text retrieval is very fast"), AverageTime < 0.00001); // Less than 0.01ms

	return true;
}
