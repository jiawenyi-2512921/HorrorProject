// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorMapChainExit.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Game/HorrorGameModeBase.h"
#include "GameFramework/WorldSettings.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

namespace
{
	bool ContainsPrototypeEndingLanguage(const FString& Text)
	{
		return Text.Contains(TEXT("临时"))
			|| Text.Contains(TEXT("稍后"))
			|| Text.Contains(TEXT("最终视频"))
			|| Text.Contains(TEXT("temporary"), ESearchCase::IgnoreCase)
			|| Text.Contains(TEXT("placeholder"), ESearchCase::IgnoreCase);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorMapChainExitLocksDay1UntilFoundFootageCompleteTest,
	"HorrorProject.Game.MapChain.ExitLocksDay1UntilFoundFootageComplete",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorMapChainExitLocksWhenCampaignChapterInactiveTest,
	"HorrorProject.Game.MapChain.ExitLocksWhenCampaignChapterInactive",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorMapChainExitFinalEndingCopyIsProductionReadyTest,
	"HorrorProject.Game.MapChain.ExitFinalEndingCopyIsProductionReady",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorMapChainExitLocksWhenCampaignChapterInactiveTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for inactive campaign exit gating."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the Horror game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorMapChainExit* MapChainExit = World->SpawnActor<AHorrorMapChainExit>();
	TestNotNull(TEXT("Inactive campaign exit test should expose the game mode."), GameMode);
	TestNotNull(TEXT("Inactive campaign exit test should spawn an exit."), MapChainExit);
	if (!GameMode || !MapChainExit)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FString ErrorMessage;
	GameMode->InitGame(TEXT("/Game/ForestOfSpikes/Levels/Level_ForestOfSpikes_Demo_Night"), TEXT(""), ErrorMessage);
	GameMode->ImportDay1CompleteState(false);
	MapChainExit->ConfigureForMapChain(
		TEXT("/Game/ForestOfSpikes/Levels/Level_ForestOfSpikes_Demo_Night"),
		TEXT("/Game/Scrapopolis/Levels/Level_Scrapopolis_Demo"),
		false);

	TestTrue(TEXT("Inactive campaign exit setup should have no active chapter."), GameMode->GetCurrentCampaignChapterId().IsNone());
	TestFalse(TEXT("Non-Day1 map-chain exit should lock when the campaign chapter is inactive."), MapChainExit->CanUseExitForTests(nullptr));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

bool FHorrorMapChainExitFinalEndingCopyIsProductionReadyTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for final ending copy coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorMapChainExit* MapChainExit = World->SpawnActor<AHorrorMapChainExit>();
	TestNotNull(TEXT("Final ending copy test should spawn an exit."), MapChainExit);
	if (!MapChainExit)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	MapChainExit->ConfigureForMapChain(
		TEXT("/Game/Bodycam_VHS_Effect/Maps/LVL_Showcase_01"),
		TEXT(""),
		true);

	const FString LabelText = MapChainExit->GetLabelTextForTests().ToString();
	const FString EndingMessage = MapChainExit->GetFinalEndingMessageForTests().ToString();
	TestTrue(TEXT("Final exit label should point players to the black-box ending."), LabelText.Contains(TEXT("黑盒")) || LabelText.Contains(TEXT("终章")));
	TestTrue(TEXT("Final ending message should confirm the investigation has been archived."), EndingMessage.Contains(TEXT("黑盒")) && EndingMessage.Contains(TEXT("归档")));
	TestFalse(TEXT("Final exit label should not expose prototype language."), ContainsPrototypeEndingLanguage(LabelText));
	TestFalse(TEXT("Final ending message should not expose prototype language."), ContainsPrototypeEndingLanguage(EndingMessage));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

bool FHorrorMapChainExitLocksDay1UntilFoundFootageCompleteTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for Day1 map-chain exit coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the Horror game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorMapChainExit* MapChainExit = World->SpawnActor<AHorrorMapChainExit>();
	TestNotNull(TEXT("Day1 map-chain exit test should expose the game mode."), GameMode);
	TestNotNull(TEXT("Day1 map-chain exit test should spawn an exit."), MapChainExit);
	if (!GameMode || !MapChainExit)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FString ErrorMessage;
	GameMode->InitGame(TEXT("/Game/DeepWaterStation/Maps/DemoMap_VerticalSlice_Day1"), TEXT(""), ErrorMessage);
	MapChainExit->ConfigureForMapChain(
		TEXT("/Game/DeepWaterStation/Maps/DemoMap_VerticalSlice_Day1"),
		TEXT("/Game/ForestOfSpikes/Levels/Level_ForestOfSpikes_Demo_Night"),
		false);

	TestFalse(TEXT("Day1 map-chain exit should stay locked before found-footage completion."), MapChainExit->CanUseExitForTests(nullptr));

	TestTrue(TEXT("Bodycam acquisition should advance Day1 exit setup."), GameMode->TryAcquireBodycam(TEXT("Evidence.Bodycam"), true));
	TestTrue(TEXT("First note should advance Day1 exit setup."), GameMode->TryCollectFirstNote(TEXT("Note.Intro")));
	TestTrue(TEXT("First anomaly candidate should advance Day1 exit setup."), GameMode->BeginFirstAnomalyCandidate(TEXT("Evidence.Anomaly01")));
	TestTrue(TEXT("First anomaly recording should advance Day1 exit setup."), GameMode->TryRecordFirstAnomaly(true));
	TestTrue(TEXT("Archive review should unlock the Day1 service exit."), GameMode->TryReviewArchive(TEXT("Archive.Terminal")));
	TestTrue(TEXT("Day1 legacy exit should mark the found-footage flow complete."), GameMode->TryCompleteDay1(TEXT("Exit.ServiceDoor")));
	TestTrue(TEXT("Day1 map-chain exit should unlock only after found-footage completion."), MapChainExit->CanUseExitForTests(nullptr));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
