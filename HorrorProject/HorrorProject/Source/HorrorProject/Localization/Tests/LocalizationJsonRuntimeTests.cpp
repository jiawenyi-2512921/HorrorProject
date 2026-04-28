// Copyright Epic Games, Inc. All Rights Reserved.

#include "Localization/LocalizationSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Engine/GameInstance.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLocalizationSubsystemDefaultsToChineseJsonTest,
	"HorrorProject.Localization.Json.DefaultsToChineseGameFile",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLocalizationSubsystemDefaultsToChineseJsonTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	TestNotNull(TEXT("Localization JSON test should create a game instance."), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	GameInstance->Init();

	ULocalizationSubsystem* LocalizationSubsystem = GameInstance->GetSubsystem<ULocalizationSubsystem>();
	TestNotNull(TEXT("Localization JSON test should expose the localization subsystem."), LocalizationSubsystem);
	if (!LocalizationSubsystem)
	{
		return false;
	}

	TestEqual(
		TEXT("Chinese projects should boot into Simplified Chinese by default."),
		LocalizationSubsystem->GetCurrentLanguage(),
		ELanguage::Chinese);

	TestEqual(
		TEXT("Default Chinese localization should load keys that only exist in Content/Localization/zh-Hans/Game.json."),
		LocalizationSubsystem->GetLocalizedText(TEXT("UI.Gameplay.Objective")).ToString(),
		FString(TEXT("目标")));
	TestEqual(
		TEXT("Default Chinese localization should load item labels from the JSON file."),
		LocalizationSubsystem->GetLocalizedText(TEXT("Game.Item.Note")).ToString(),
		FString(TEXT("笔记")));
	TestEqual(
		TEXT("Missing localization keys should still fall back to the key."),
		LocalizationSubsystem->GetLocalizedText(TEXT("Missing.Localization.Key")).ToString(),
		FString(TEXT("Missing.Localization.Key")));

	GameInstance->Shutdown();
	return true;
}

#endif
