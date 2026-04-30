// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorCampaign.h"
#include "Game/HorrorCampaignBossActor.h"
#include "Game/HorrorCampaignObjectiveActor.h"
#include "Game/HorrorGameModeBase.h"
#include "Game/HorrorMapChain.h"
#include "Player/Components/FearComponent.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/LightComponent.h"
#include "Components/LocalLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PointLight.h"
#include "Engine/PostProcessVolume.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Tests/AutomationCommon.h"

namespace
{
bool ContainsCampaignMapFragment(const FHorrorCampaignChapterDefinition& Chapter, const TCHAR* Fragment)
{
	return Chapter.MapPackageName.Contains(Fragment);
}

bool ContainsMojibakeMarker(const FString& Text)
{
	static const TCHAR* Markers[] = {
		TEXT("锛"),
		TEXT("鍚"),
		TEXT("绗"),
		TEXT("€"),
		TEXT("鎴"),
		TEXT("鐭"),
		TEXT("?")
	};

	for (const TCHAR* Marker : Markers)
	{
		if (Text.Contains(Marker))
		{
			return true;
		}
	}

	return false;
}

bool ContainsLatinLetter(const FString& Text)
{
	for (const TCHAR Character : Text)
	{
		if ((Character >= TEXT('A') && Character <= TEXT('Z')) || (Character >= TEXT('a') && Character <= TEXT('z')))
		{
			return true;
		}
	}

	return false;
}

int32 CountAwakeCampaignBossActors(UWorld* World)
{
	int32 AwakeCount = 0;
	if (!World)
	{
		return AwakeCount;
	}

	for (TActorIterator<AHorrorCampaignBossActor> It(World); It; ++It)
	{
		const AHorrorCampaignBossActor* BossActor = *It;
		if (BossActor && BossActor->IsBossAwake())
		{
			++AwakeCount;
		}
	}

	return AwakeCount;
}

AHorrorPlayerCharacter* SpawnControlledCampaignPlayer(FAutomationTestBase& Test, UWorld* World, const FVector& Location)
{
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	APlayerController* PlayerController = World->SpawnActor<APlayerController>(SpawnParameters);
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(Location, FRotator::ZeroRotator, SpawnParameters);
	Test.TestNotNull(TEXT("Campaign test should spawn a player controller."), PlayerController);
	Test.TestNotNull(TEXT("Campaign test should spawn a controlled horror player character."), PlayerCharacter);
	if (PlayerController)
	{
		PlayerController->PlayerState = World->SpawnActor<APlayerState>(SpawnParameters);
		World->AddController(PlayerController);
	}
	if (PlayerController && PlayerCharacter)
	{
		PlayerController->Possess(PlayerCharacter);
	}
	return PlayerCharacter;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignDefinesPlayableStoryOrderTest,
	"HorrorProject.Game.Campaign.DefinesPlayableStoryOrder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignDefinesPlayableStoryOrderTest::RunTest(const FString& Parameters)
{
	const TArray<FHorrorCampaignChapterDefinition>& Chapters = FHorrorCampaign::GetChapters();
	const TArray<FHorrorMapChainEntry>& MapEntries = FHorrorMapChain::GetEntries();

	TestEqual(TEXT("The campaign and travel chain should describe the same playable maps."), Chapters.Num(), MapEntries.Num());
	TestTrue(TEXT("The campaign should contain enough chapters for the current multi-map prototype."), Chapters.Num() >= 8);
	if (Chapters.IsEmpty() || MapEntries.IsEmpty())
	{
		return false;
	}

	TestEqual(
		TEXT("The campaign should begin in Deep Water Station as the investigation opening."),
		Chapters[0].MapPackageName,
		FString(TEXT("/Game/DeepWaterStation/Maps/DemoMap_VerticalSlice_Day1")));
	TestEqual(
		TEXT("The final campaign chapter should be the Bodycam VHS black-box ending."),
		Chapters.Last().MapPackageName,
		FString(TEXT("/Game/Bodycam_VHS_Effect/Maps/LVL_Showcase_01")));
	TestTrue(TEXT("The final map chain entry should also be the Bodycam VHS black-box ending."), FHorrorMapChain::IsFinalMap(Chapters.Last().MapPackageName));
	TestTrue(
		TEXT("The opening Deep Water Station chapter should include a pressure beat so the first playable map is more than location finding."),
		FHorrorCampaign::CountObjectivesOfType(Chapters[0], EHorrorCampaignObjectiveType::SurviveAmbush) >= 1);

	int32 BossChapterCount = 0;
	int32 FinalChapterCount = 0;
	int32 SurvivalObjectiveCount = 0;
	for (int32 ChapterIndex = 0; ChapterIndex < Chapters.Num(); ++ChapterIndex)
	{
		const FHorrorCampaignChapterDefinition& Chapter = Chapters[ChapterIndex];
		TestEqual(
			FString::Printf(TEXT("Campaign chapter %d should match map-chain entry."), ChapterIndex),
			Chapter.MapPackageName,
			MapEntries[ChapterIndex].MapPackageName);

		TestFalse(
			FString::Printf(TEXT("Campaign should not include non-playable Advanced Universal Spawner showcase maps: %s."), *Chapter.MapPackageName),
			ContainsCampaignMapFragment(Chapter, TEXT("AdvancedUniversalSpawner")));
		TestFalse(
			FString::Printf(TEXT("Campaign should not include pipe asset-library maps: %s."), *Chapter.MapPackageName),
			ContainsCampaignMapFragment(Chapter, TEXT("IndustrialPipesM")));
		TestFalse(
			FString::Printf(TEXT("Campaign should not include Stone Golem model showcase maps: %s."), *Chapter.MapPackageName),
			ContainsCampaignMapFragment(Chapter, TEXT("Stone_Golem/map")));
		TestFalse(
			FString::Printf(TEXT("Campaign should not include static asset-library maps: %s."), *Chapter.MapPackageName),
			ContainsCampaignMapFragment(Chapter, TEXT("Asset_Library")));

		TestFalse(
			FString::Printf(TEXT("Chapter %s should have a readable title."), *Chapter.ChapterId.ToString()),
			Chapter.Title.IsEmpty());
		TestFalse(
			FString::Printf(TEXT("Chapter %s should have a readable story brief."), *Chapter.ChapterId.ToString()),
			Chapter.StoryBrief.IsEmpty());
		TestFalse(
			FString::Printf(TEXT("Chapter %s should have a readable completion bridge."), *Chapter.ChapterId.ToString()),
			Chapter.CompletionBridgeText.IsEmpty());
		TestTrue(
			FString::Printf(TEXT("Chapter %s should expose at least one playable objective."), *Chapter.ChapterId.ToString()),
			Chapter.Objectives.Num() > 0);

		if (Chapter.bRequiresBoss)
		{
			++BossChapterCount;
			TestTrue(TEXT("The boss chapter should use the dungeon boss-room map."), ContainsCampaignMapFragment(Chapter, TEXT("map_dungeon_level_5_bossroom")));
			TestTrue(TEXT("The boss chapter should contain boss weak-point objectives."), FHorrorCampaign::CountObjectivesOfType(Chapter, EHorrorCampaignObjectiveType::BossWeakPoint) >= 3);
		}

		if (Chapter.bIsFinalChapter)
		{
			++FinalChapterCount;
			TestTrue(TEXT("The final chapter should be Bodycam VHS."), ContainsCampaignMapFragment(Chapter, TEXT("Bodycam_VHS_Effect")));
			TestTrue(TEXT("The final chapter should end at a final terminal objective."), FHorrorCampaign::CountObjectivesOfType(Chapter, EHorrorCampaignObjectiveType::FinalTerminal) >= 1);
		}

		SurvivalObjectiveCount += FHorrorCampaign::CountObjectivesOfType(Chapter, EHorrorCampaignObjectiveType::SurviveAmbush);

		TestTrue(
			FString::Printf(TEXT("Campaign package should exist: %s."), *Chapter.MapPackageName),
			FPackageName::DoesPackageExist(Chapter.MapPackageName));
	}

	TestEqual(TEXT("The campaign should have exactly one boss chapter for the current prototype."), BossChapterCount, 1);
	TestEqual(TEXT("The campaign should have exactly one final chapter."), FinalChapterCount, 1);
	TestTrue(TEXT("The campaign should include at least one timed survival pressure objective."), SurvivalObjectiveCount >= 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignUsesReadableChineseTextTest,
	"HorrorProject.Game.Campaign.UsesReadableChineseText",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignUsesReadableChineseTextTest::RunTest(const FString& Parameters)
{
	int32 BlackBoxTextCount = 0;
	int32 DeepWaterTextCount = 0;
	int32 GolemTextCount = 0;
	int32 TapeTextCount = 0;

	for (const FHorrorCampaignChapterDefinition& Chapter : FHorrorCampaign::GetChapters())
	{
		const FString ChapterVisibleText = FString::Printf(
			TEXT("%s %s %s"),
			*Chapter.Title.ToString(),
			*Chapter.StoryBrief.ToString(),
			*Chapter.CompletionBridgeText.ToString());

		TestFalse(
			FString::Printf(TEXT("Chapter title should not contain mojibake markers: %s."), *Chapter.ChapterId.ToString()),
			ContainsMojibakeMarker(Chapter.Title.ToString()));
		TestFalse(
			FString::Printf(TEXT("Chapter story should not contain mojibake markers: %s."), *Chapter.ChapterId.ToString()),
			ContainsMojibakeMarker(Chapter.StoryBrief.ToString()));
		TestFalse(
			FString::Printf(TEXT("Chapter completion bridge should not contain mojibake markers: %s."), *Chapter.ChapterId.ToString()),
			ContainsMojibakeMarker(Chapter.CompletionBridgeText.ToString()));
		TestFalse(
			FString::Printf(TEXT("Chapter visible text should not expose English letters: %s."), *Chapter.ChapterId.ToString()),
			ContainsLatinLetter(ChapterVisibleText));

		for (const FHorrorCampaignObjectiveDefinition& Objective : Chapter.Objectives)
		{
			const FString ObjectiveVisibleText = FString::Printf(
				TEXT("%s %s"),
				*Objective.PromptText.ToString(),
				*Objective.CompletionText.ToString());

			TestFalse(
				FString::Printf(TEXT("Objective prompt should not contain mojibake markers: %s."), *Objective.ObjectiveId.ToString()),
				ContainsMojibakeMarker(Objective.PromptText.ToString()));
			TestFalse(
				FString::Printf(TEXT("Objective completion should not contain mojibake markers: %s."), *Objective.ObjectiveId.ToString()),
				ContainsMojibakeMarker(Objective.CompletionText.ToString()));
			TestFalse(
				FString::Printf(TEXT("Objective visible text should not expose English letters: %s."), *Objective.ObjectiveId.ToString()),
				ContainsLatinLetter(ObjectiveVisibleText));
		}

		const FString NarrativeText = ChapterVisibleText;
		BlackBoxTextCount += NarrativeText.Contains(TEXT("黑盒")) ? 1 : 0;
		DeepWaterTextCount += NarrativeText.Contains(TEXT("深水站")) ? 1 : 0;
		GolemTextCount += NarrativeText.Contains(TEXT("巨人")) ? 1 : 0;
		TapeTextCount += NarrativeText.Contains(TEXT("录像带")) ? 1 : 0;
	}

	TestTrue(TEXT("AAA campaign story should repeatedly anchor the black-box mystery."), BlackBoxTextCount >= 3);
	TestTrue(TEXT("AAA campaign story should repeatedly anchor Deep Water Station."), DeepWaterTextCount >= 2);
	TestTrue(TEXT("AAA campaign story should repeatedly anchor the stone giant thread."), GolemTextCount >= 4);
	TestTrue(TEXT("AAA campaign story should repeatedly anchor the final tape reveal."), TapeTextCount >= 2);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignTempleKeyFragmentIsFindableTest,
	"HorrorProject.Game.Campaign.TempleKeyFragmentIsFindable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignTempleKeyFragmentIsFindableTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* TempleChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.DungeonTemple"));
	TestNotNull(TEXT("Temple chapter should exist for key-fragment coverage."), TempleChapter);
	if (!TempleChapter)
	{
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* AltarObjective =
		FHorrorCampaign::FindObjectiveById(*TempleChapter, TEXT("Temple.ChargeAltar"));
	const FHorrorCampaignObjectiveDefinition* KeyFragmentObjective =
		FHorrorCampaign::FindObjectiveById(*TempleChapter, TEXT("Temple.RecoverKeySigil"));
	TestNotNull(TEXT("Temple should expose the altar objective."), AltarObjective);
	TestNotNull(TEXT("Temple should expose the key-fragment objective."), KeyFragmentObjective);
	if (!AltarObjective || !KeyFragmentObjective)
	{
		return false;
	}

	TestTrue(
		TEXT("The key fragment should explicitly say key fragment so players can identify it."),
		KeyFragmentObjective->PromptText.ToString().Contains(TEXT("钥匙碎片")));
	TestTrue(
		TEXT("The key fragment should use the dungeon key mesh instead of a generic relic prop."),
		KeyFragmentObjective->VisualMeshPath.ToString().Contains(TEXT("SM_PROP_key_dungeon_01")));
	TestTrue(
		TEXT("The key fragment should stay close enough to the charged altar to be discoverable."),
		FVector::Dist2D(AltarObjective->RelativeLocation, KeyFragmentObjective->RelativeLocation) <= 850.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignAddsGolemWorkAcrossLateChaptersTest,
	"HorrorProject.Game.Campaign.AddsGolemWorkAcrossLateChapters",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignAddsGolemWorkAcrossLateChaptersTest::RunTest(const FString& Parameters)
{
	const FName LateChapterIds[] = {
		TEXT("Chapter.Scrapopolis"),
		TEXT("Chapter.DungeonEntrance"),
		TEXT("Chapter.DungeonDepths"),
		TEXT("Chapter.DungeonHall"),
		TEXT("Chapter.DungeonTemple"),
		TEXT("Chapter.StoneGolemBoss"),
		TEXT("Chapter.SignalCalibration")
	};

	int32 NonBossGolemWorkObjectiveCount = 0;
	for (const FName ChapterId : LateChapterIds)
	{
		const FHorrorCampaignChapterDefinition* Chapter = FHorrorCampaign::FindChapterById(ChapterId);
		TestNotNull(FString::Printf(TEXT("Late chapter should exist: %s."), *ChapterId.ToString()), Chapter);
		if (!Chapter)
		{
			return false;
		}

		bool bHasGolemObjective = false;
		for (const FHorrorCampaignObjectiveDefinition& Objective : Chapter->Objectives)
		{
			const FString ObjectiveText = FString::Printf(
				TEXT("%s %s %s"),
				*Objective.ObjectiveId.ToString(),
				*Objective.PromptText.ToString(),
				*Objective.CompletionText.ToString());
			if (ObjectiveText.Contains(TEXT("巨人")) || ObjectiveText.Contains(TEXT("Golem")))
			{
				bHasGolemObjective = true;
				if (!Chapter->bRequiresBoss && Objective.ObjectiveType != EHorrorCampaignObjectiveType::BossWeakPoint)
				{
					++NonBossGolemWorkObjectiveCount;
				}
			}
		}

		TestTrue(
			FString::Printf(TEXT("Late chapter should include at least one golem-related objective: %s."), *ChapterId.ToString()),
			bHasGolemObjective);
	}

	TestTrue(TEXT("The late campaign should include several non-boss golem repair/calibration tasks."), NonBossGolemWorkObjectiveCount >= 5);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignScrapopolisGolemJointUsesGeneratorTerminalTest,
	"HorrorProject.Game.Campaign.ScrapopolisGolemJointUsesGeneratorTerminal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignScrapopolisGolemJointUsesGeneratorTerminalTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* ScrapopolisChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.Scrapopolis"));
	TestNotNull(TEXT("Scrapopolis chapter should exist for golem-joint reachability coverage."), ScrapopolisChapter);
	if (!ScrapopolisChapter)
	{
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* GeneratorObjective =
		FHorrorCampaign::FindObjectiveById(*ScrapopolisChapter, TEXT("Scrapopolis.RestoreGenerator"));
	TestNotNull(TEXT("Scrapopolis should expose the generator objective."), GeneratorObjective);
	if (!GeneratorObjective)
	{
		return false;
	}

	TestTrue(
		TEXT("The golem knee-joint repair should be merged into the already reachable generator terminal."),
		GeneratorObjective->PromptText.ToString().Contains(TEXT("巨人膝关节驱动环")));
	TestNull(
		TEXT("Scrapopolis should not spawn a separate golem knee-joint marker that can land inside tower geometry."),
		FHorrorCampaign::FindObjectiveById(*ScrapopolisChapter, TEXT("Scrapopolis.ForgeGolemJoint")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignDepthsGolemMemoryUsesArchiveReaderTest,
	"HorrorProject.Game.Campaign.DepthsGolemMemoryUsesArchiveReader",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignDepthsGolemMemoryUsesArchiveReaderTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* DepthsChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.DungeonDepths"));
	TestNotNull(TEXT("Dungeon depths chapter should exist for golem-memory reachability coverage."), DepthsChapter);
	if (!DepthsChapter)
	{
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* ArchiveReaderObjective =
		FHorrorCampaign::FindObjectiveById(*DepthsChapter, TEXT("Depths.PowerArchiveReader"));
	TestNotNull(TEXT("Dungeon depths should expose the archive reader objective."), ArchiveReaderObjective);
	if (!ArchiveReaderObjective)
	{
		return false;
	}

	TestTrue(
		TEXT("The golem memory plate repair should be merged into the already reachable archive reader terminal."),
		ArchiveReaderObjective->PromptText.ToString().Contains(TEXT("巨人记忆石板")));
	TestNull(
		TEXT("Dungeon depths should not spawn a separate golem memory plate marker that can land inside unreachable dungeon geometry."),
		FHorrorCampaign::FindObjectiveById(*DepthsChapter, TEXT("Depths.RepairGolemMemoryPlate")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignBossActorTracksWeakPointProgressTest,
	"HorrorProject.Game.Campaign.BossActorTracksWeakPointProgress",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignBossActorTracksWeakPointProgressTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for boss actor coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorCampaignBossActor* BossActor = World->SpawnActor<AHorrorCampaignBossActor>();
	TestNotNull(TEXT("Boss actor should spawn for weak-point progress coverage."), BossActor);
	if (!BossActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	BossActor->ConfigureBoss(TEXT("Chapter.StoneGolemBoss"), FText::FromString(TEXT("Stone Golem")), 3);
	TestEqual(TEXT("Configured boss should remember required weak points."), BossActor->GetRequiredWeakPointCount(), 3);
	TestEqual(TEXT("Configured boss should start with no resolved weak points."), BossActor->GetResolvedWeakPointCount(), 0);
	TestFalse(TEXT("Configured boss should start undefeated."), BossActor->IsBossDefeated());

	TestTrue(TEXT("First weak point should register."), BossActor->RegisterWeakPointResolved());
	TestEqual(TEXT("First weak point should update progress."), BossActor->GetResolvedWeakPointCount(), 1);
	TestFalse(TEXT("Boss should survive after one weak point."), BossActor->IsBossDefeated());

	TestTrue(TEXT("Second weak point should register."), BossActor->RegisterWeakPointResolved());
	TestEqual(TEXT("Second weak point should update progress."), BossActor->GetResolvedWeakPointCount(), 2);
	TestFalse(TEXT("Boss should survive until all weak points resolve."), BossActor->IsBossDefeated());

	TestTrue(TEXT("Third weak point should register."), BossActor->RegisterWeakPointResolved());
	TestEqual(TEXT("Third weak point should complete progress."), BossActor->GetResolvedWeakPointCount(), 3);
	TestTrue(TEXT("Boss should be defeated after all weak points resolve."), BossActor->IsBossDefeated());
	TestFalse(TEXT("Extra weak points should be ignored after defeat."), BossActor->RegisterWeakPointResolved());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveActorsExposeCurrentObjectiveAvailabilityTest,
	"HorrorProject.Game.Campaign.ObjectiveActorsExposeCurrentObjectiveAvailability",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveActorsExposeCurrentObjectiveAvailabilityTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* ForestChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.ForestOfSpikes"));
	TestNotNull(TEXT("Forest chapter should exist for objective availability coverage."), ForestChapter);
	if (!ForestChapter || ForestChapter->Objectives.Num() < 2)
	{
		return false;
	}

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for objective availability coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the campaign game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	TestNotNull(TEXT("Objective availability test should expose the campaign game mode."), GameMode);
	if (!GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(ForestChapter->ChapterId);

	AHorrorCampaignObjectiveActor* FirstObjective = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	AHorrorCampaignObjectiveActor* SecondObjective = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("First campaign objective actor should spawn."), FirstObjective);
	TestNotNull(TEXT("Second campaign objective actor should spawn."), SecondObjective);
	if (!FirstObjective || !SecondObjective)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FirstObjective->ConfigureObjective(ForestChapter->ChapterId, ForestChapter->Objectives[0]);
	SecondObjective->ConfigureObjective(ForestChapter->ChapterId, ForestChapter->Objectives[1]);

	TestTrue(TEXT("The first objective actor should be available at chapter start."), FirstObjective->IsAvailableForInteraction());
	TestFalse(TEXT("The second objective actor should wait for the first objective."), SecondObjective->IsAvailableForInteraction());
	const FString LockedPrompt = SecondObjective->GetInteractionPromptText().ToString();
	TestTrue(TEXT("Locked campaign objectives should tell the player which current objective gates them."), LockedPrompt.Contains(ForestChapter->Objectives[0].PromptText.ToString()));

	TestTrue(
		TEXT("Completing the first objective through the game mode should advance campaign progress."),
		GameMode->TryCompleteCampaignObjective(ForestChapter->ChapterId, ForestChapter->Objectives[0].ObjectiveId, nullptr));
	SecondObjective->RefreshObjectiveState();
	TestTrue(TEXT("The second objective actor should become available after progress advances."), SecondObjective->IsAvailableForInteraction());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveActorRequiresMultiStepInteractionsTest,
	"HorrorProject.Game.Campaign.ObjectiveActorRequiresMultiStepInteractions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveActorRequiresMultiStepInteractionsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for multi-step objective coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Multi-step objective test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Multi-step objective test should spawn an objective actor."), ObjectiveActor);
	if (!InstigatorActor || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.RestorePower");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::RestorePower;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::MultiStep;
	Objective.PromptText = FText::FromString(TEXT("恢复测试电源"));
	Objective.CompletionText = FText::FromString(TEXT("测试电源恢复。"));

	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);
	TestEqual(TEXT("Explicit multi-step objectives should require multiple interaction steps."), ObjectiveActor->GetRequiredInteractionCount(), 3);
	TestEqual(TEXT("Multi-step objectives should begin with zero progress."), ObjectiveActor->GetInteractionProgressCount(), 0);

	const FHitResult EmptyHit;
	TestTrue(TEXT("First interaction step should be accepted."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	TestEqual(TEXT("First interaction should advance progress."), ObjectiveActor->GetInteractionProgressCount(), 1);
	TestFalse(TEXT("First interaction should not complete a multi-step objective."), ObjectiveActor->IsCompleted());

	TestTrue(TEXT("Second interaction step should be accepted."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	TestEqual(TEXT("Second interaction should advance progress."), ObjectiveActor->GetInteractionProgressCount(), 2);
	TestFalse(TEXT("Second interaction should not complete a three-step objective."), ObjectiveActor->IsCompleted());

	TestTrue(TEXT("Final interaction step should be accepted."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	TestEqual(TEXT("Final interaction should complete progress."), ObjectiveActor->GetInteractionProgressCount(), 3);
	TestTrue(TEXT("Final interaction should complete the objective."), ObjectiveActor->IsCompleted());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignAdvancedObjectivesUseDedicatedInteractionModesTest,
	"HorrorProject.Game.Campaign.AdvancedObjectivesUseDedicatedInteractionModes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignAdvancedObjectivesUseDedicatedInteractionModesTest::RunTest(const FString& Parameters)
{
	int32 CircuitObjectiveCount = 0;
	int32 GearObjectiveCount = 0;
	int32 TimedPursuitObjectiveCount = 0;

	for (const FHorrorCampaignChapterDefinition& Chapter : FHorrorCampaign::GetChapters())
	{
		for (const FHorrorCampaignObjectiveDefinition& Objective : Chapter.Objectives)
		{
			if (Objective.ObjectiveType == EHorrorCampaignObjectiveType::RestorePower)
			{
				TestEqual(
					FString::Printf(TEXT("RestorePower should use the circuit wiring mode: %s."), *Objective.ObjectiveId.ToString()),
					Objective.InteractionMode,
					EHorrorCampaignInteractionMode::CircuitWiring);
				++CircuitObjectiveCount;
			}
			else if (Objective.ObjectiveType == EHorrorCampaignObjectiveType::DisableSeal)
			{
				TestEqual(
					FString::Printf(TEXT("DisableSeal should use the gear calibration mode: %s."), *Objective.ObjectiveId.ToString()),
					Objective.InteractionMode,
					EHorrorCampaignInteractionMode::GearCalibration);
				++GearObjectiveCount;
			}
			else if (Objective.ObjectiveType == EHorrorCampaignObjectiveType::SurviveAmbush)
			{
				TestEqual(
					FString::Printf(TEXT("SurviveAmbush should remain the simple timed pursuit mode: %s."), *Objective.ObjectiveId.ToString()),
					Objective.InteractionMode,
					EHorrorCampaignInteractionMode::TimedPursuit);
				++TimedPursuitObjectiveCount;
			}
		}
	}

	TestTrue(TEXT("The campaign should include multiple circuit wiring tasks."), CircuitObjectiveCount >= 4);
	TestTrue(TEXT("The campaign should include multiple gear calibration tasks."), GearObjectiveCount >= 3);
	TestTrue(TEXT("The campaign should still include timed pursuit tasks."), TimedPursuitObjectiveCount >= 2);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveActorRunsCircuitWiringInteractionTest,
	"HorrorProject.Game.Campaign.ObjectiveActorRunsCircuitWiringInteraction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveActorRunsCircuitWiringInteractionTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for circuit wiring coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Circuit wiring test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Circuit wiring test should spawn an objective actor."), ObjectiveActor);
	if (!InstigatorActor || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.CircuitWiring");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::RestorePower;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::CircuitWiring;
	Objective.PromptText = FText::FromString(TEXT("修复测试电路"));
	Objective.CompletionText = FText::FromString(TEXT("测试电路恢复。"));

	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);
	TestEqual(TEXT("Circuit wiring objectives should expose the dedicated mode."), ObjectiveActor->GetInteractionMode(), EHorrorCampaignInteractionMode::CircuitWiring);
	TestFalse(TEXT("Circuit wiring should not be active before interaction."), ObjectiveActor->IsAdvancedInteractionActive());

	const FHitResult EmptyHit;
	TestTrue(TEXT("First interaction should open the circuit wiring state."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	TestTrue(TEXT("Circuit wiring should become active after interaction."), ObjectiveActor->IsAdvancedInteractionActive());
	TestTrue(TEXT("Circuit prompt should describe wiring in Chinese."), ObjectiveActor->GetInteractionPromptText().ToString().Contains(TEXT("接线")));

	const FHorrorAdvancedInteractionHUDState OpenCircuitPanel = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestTrue(TEXT("Circuit wiring should expose a visible animated HUD device state."), OpenCircuitPanel.bVisible);
	TestEqual(TEXT("Circuit wiring HUD state should report the wiring mode."), OpenCircuitPanel.Mode, EHorrorCampaignInteractionMode::CircuitWiring);
	TestTrue(TEXT("Circuit wiring HUD state should carry the visible objective title."), OpenCircuitPanel.Title.ToString().Contains(TEXT("修复测试电路")));
	TestEqual(TEXT("Circuit wiring HUD state should start at the first step."), OpenCircuitPanel.StepIndex, 0);
	TestEqual(TEXT("Circuit wiring HUD state should expose the required step count."), OpenCircuitPanel.RequiredStepCount, 3);

	const FName ExpectedFirstInput = ObjectiveActor->GetExpectedAdvancedInputId();
	TestTrue(TEXT("Circuit wiring should expose an expected input id."), !ExpectedFirstInput.IsNone());
	TestTrue(TEXT("Premature wire input should be treated as a timing failure event."), ObjectiveActor->SubmitAdvancedInteractionInput(ExpectedFirstInput, InstigatorActor));
	TestTrue(TEXT("Premature wire input should warn the player about the timing window."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("时机")));
	TestEqual(TEXT("Premature wire input should not advance circuit progress."), ObjectiveActor->GetAdvancedInteractionProgressFraction(), 0.0f);

	ObjectiveActor->Tick(0.65f);
	TestTrue(TEXT("Wrong wire should be accepted as a failure event."), ObjectiveActor->SubmitAdvancedInteractionInput(TEXT("错误端子"), InstigatorActor));
	TestTrue(TEXT("Wrong wire should create red-spark feedback."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("红色火花")));
	TestTrue(TEXT("Wrong wire should keep progress below completion."), ObjectiveActor->GetAdvancedInteractionProgressFraction() < 0.5f);

	ObjectiveActor->Tick(0.65f);
	TestTrue(TEXT("Correct first wire should advance progress."), ObjectiveActor->SubmitAdvancedInteractionInput(ExpectedFirstInput, InstigatorActor));
	const float ProgressAfterFirstWire = ObjectiveActor->GetAdvancedInteractionProgressFraction();
	TestTrue(TEXT("Correct wire should advance circuit progress."), ProgressAfterFirstWire > 0.0f);

	for (int32 StepIndex = 0; StepIndex < 4 && !ObjectiveActor->IsCompleted(); ++StepIndex)
	{
		ObjectiveActor->Tick(0.65f);
		TestTrue(
			FString::Printf(TEXT("Correct wire step should advance or complete the circuit: %d."), StepIndex),
			ObjectiveActor->SubmitAdvancedInteractionInput(ObjectiveActor->GetExpectedAdvancedInputId(), InstigatorActor));
	}

	TestTrue(TEXT("Circuit wiring should complete after the required correct wire inputs."), ObjectiveActor->IsCompleted());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveActorRunsGearCalibrationInteractionTest,
	"HorrorProject.Game.Campaign.ObjectiveActorRunsGearCalibrationInteraction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveActorRunsGearCalibrationInteractionTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for gear calibration coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Gear calibration test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Gear calibration test should spawn an objective actor."), ObjectiveActor);
	if (!InstigatorActor || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.GearCalibration");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::DisableSeal;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::GearCalibration;
	Objective.PromptText = FText::FromString(TEXT("校准测试齿轮"));
	Objective.CompletionText = FText::FromString(TEXT("测试齿轮咬合。"));

	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);
	TestEqual(TEXT("Gear calibration objectives should expose the dedicated mode."), ObjectiveActor->GetInteractionMode(), EHorrorCampaignInteractionMode::GearCalibration);

	const FHitResult EmptyHit;
	TestTrue(TEXT("First interaction should open the gear calibration state."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	TestTrue(TEXT("Gear calibration should become active after interaction."), ObjectiveActor->IsAdvancedInteractionActive());
	TestTrue(TEXT("Gear prompt should describe stopped gears in Chinese."), ObjectiveActor->GetInteractionPromptText().ToString().Contains(TEXT("齿轮")));

	const FHorrorAdvancedInteractionHUDState OpenGearPanel = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestTrue(TEXT("Gear calibration should expose a visible animated HUD device state."), OpenGearPanel.bVisible);
	TestEqual(TEXT("Gear calibration HUD state should report the gear mode."), OpenGearPanel.Mode, EHorrorCampaignInteractionMode::GearCalibration);
	TestTrue(TEXT("Gear calibration HUD state should carry the visible objective title."), OpenGearPanel.Title.ToString().Contains(TEXT("校准测试齿轮")));
	TestEqual(TEXT("Gear calibration HUD state should start at the first step."), OpenGearPanel.StepIndex, 0);
	TestEqual(TEXT("Gear calibration HUD state should expose the required step count."), OpenGearPanel.RequiredStepCount, 3);

	const FName ExpectedFirstGear = ObjectiveActor->GetExpectedAdvancedInputId();
	TestTrue(TEXT("Premature gear input should be treated as a timing failure event."), ObjectiveActor->SubmitAdvancedInteractionInput(ExpectedFirstGear, InstigatorActor));
	TestTrue(TEXT("Premature gear input should warn the player about the timing window."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("时机")));
	TestTrue(TEXT("Premature gear input should pause calibration."), ObjectiveActor->GetAdvancedInteractionPauseRemainingSeconds() > 0.0f);
	ObjectiveActor->Tick(3.0f);

	ObjectiveActor->Tick(0.65f);
	TestTrue(TEXT("Wrong gear should trigger the jam pause."), ObjectiveActor->SubmitAdvancedInteractionInput(TEXT("齿轮9"), InstigatorActor));
	TestTrue(TEXT("Wrong gear should pause calibration."), ObjectiveActor->GetAdvancedInteractionPauseRemainingSeconds() > 0.0f);
	TestTrue(TEXT("Wrong gear should create click feedback."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("咔哒")));

	const float PausedProgress = ObjectiveActor->GetAdvancedInteractionProgressFraction();
	ObjectiveActor->Tick(1.0f);
	TestEqual(TEXT("Gear progress should stay paused during a jam."), ObjectiveActor->GetAdvancedInteractionProgressFraction(), PausedProgress);
	ObjectiveActor->Tick(3.0f);
	TestEqual(TEXT("Gear pause should clear after the jam duration."), ObjectiveActor->GetAdvancedInteractionPauseRemainingSeconds(), 0.0f);

	for (int32 StepIndex = 0; StepIndex < 4 && !ObjectiveActor->IsCompleted(); ++StepIndex)
	{
		ObjectiveActor->Tick(0.65f);
		TestTrue(
			FString::Printf(TEXT("Correct gear step should advance or complete calibration: %d."), StepIndex),
			ObjectiveActor->SubmitAdvancedInteractionInput(ObjectiveActor->GetExpectedAdvancedInputId(), InstigatorActor));
	}

	TestTrue(TEXT("Gear calibration should complete after the required stopped-gear corrections."), ObjectiveActor->IsCompleted());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveActorRunsTimedSurvivalObjectivesTest,
	"HorrorProject.Game.Campaign.ObjectiveActorRunsTimedSurvivalObjectives",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveActorRunsTimedSurvivalObjectivesTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for survival objective coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Survival objective test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Survival objective test should spawn an objective actor."), ObjectiveActor);
	if (!InstigatorActor || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.SurviveAmbush");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::SurviveAmbush;
	Objective.PromptText = FText::FromString(TEXT("守住测试阵地"));
	Objective.CompletionText = FText::FromString(TEXT("测试遭遇已撑过。"));

	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);
	TestTrue(TEXT("Survival objectives should expose a timed duration."), ObjectiveActor->GetTimedObjectiveDurationSeconds() > 0.0f);
	TestFalse(TEXT("Survival objectives should not be active before interaction."), ObjectiveActor->IsTimedObjectiveActive());

	const FHitResult EmptyHit;
	TestTrue(TEXT("Interacting with a survival objective should start the timer."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	TestTrue(TEXT("Survival objective should become active after interaction."), ObjectiveActor->IsTimedObjectiveActive());
	TestFalse(TEXT("Survival objective should not complete immediately."), ObjectiveActor->IsCompleted());

	const float DurationSeconds = ObjectiveActor->GetTimedObjectiveDurationSeconds();
	ObjectiveActor->Tick(DurationSeconds * 0.5f);
	TestTrue(TEXT("Survival objective should remain active before the timer expires."), ObjectiveActor->IsTimedObjectiveActive());
	TestFalse(TEXT("Survival objective should remain incomplete before the timer expires."), ObjectiveActor->IsCompleted());

	ObjectiveActor->Tick(DurationSeconds);
	TestFalse(TEXT("Survival objective should stop running once the timer expires."), ObjectiveActor->IsTimedObjectiveActive());
	TestTrue(TEXT("Survival objective should complete after the player survives the timer."), ObjectiveActor->IsCompleted());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveActorRequiresEscapeDestinationForPursuitTest,
	"HorrorProject.Game.Campaign.ObjectiveActorRequiresEscapeDestinationForPursuit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveActorRequiresEscapeDestinationForPursuitTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for escape-pursuit objective coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorCampaignObjectiveActor* InstigatorActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	TestNotNull(TEXT("Escape-pursuit objective test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Escape-pursuit objective test should spawn an objective actor."), ObjectiveActor);
	if (!InstigatorActor || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.EscapePursuit");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::SurviveAmbush;
	Objective.PromptText = FText::FromString(TEXT("逃向测试汇合点"));
	Objective.CompletionText = FText::FromString(TEXT("测试追逐已甩开。"));
	Objective.EscapeDestinationOffset = FVector(3000.0f, 0.0f, 0.0f);
	Objective.EscapeCompletionRadius = 220.0f;

	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

	const FHitResult EmptyHit;
	TestTrue(TEXT("Interacting with an escape-pursuit objective should start the timer."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	TestTrue(TEXT("Escape-pursuit objective should become active after interaction."), ObjectiveActor->IsTimedObjectiveActive());
	TestTrue(TEXT("Escape-pursuit objective should expose an active escape destination."), ObjectiveActor->HasActiveEscapeDestination());
	TestEqual(TEXT("The active objective marker should move to the escape destination."), ObjectiveActor->GetActorLocation(), FVector(3000.0f, 0.0f, 0.0f));

	ObjectiveActor->Tick(ObjectiveActor->GetTimedObjectiveDurationSeconds() + 0.1f);
	TestTrue(TEXT("Escape-pursuit objective should stay active if the player has not reached the destination."), ObjectiveActor->IsTimedObjectiveActive());
	TestFalse(TEXT("Escape-pursuit objective should not complete while the player remains at the start."), ObjectiveActor->IsCompleted());

	InstigatorActor->SetActorLocation(FVector(2920.0f, 0.0f, 0.0f));
	ObjectiveActor->Tick(0.1f);
	TestFalse(TEXT("Escape-pursuit objective should stop once the player reaches the destination after the timer."), ObjectiveActor->IsTimedObjectiveActive());
	TestTrue(TEXT("Escape-pursuit objective should complete after both chase time and destination arrival are satisfied."), ObjectiveActor->IsCompleted());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignOpeningSurvivalObjectivesUseFarEscapeDestinationsTest,
	"HorrorProject.Game.Campaign.OpeningSurvivalObjectivesUseFarEscapeDestinations",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignOpeningSurvivalObjectivesUseFarEscapeDestinationsTest::RunTest(const FString& Parameters)
{
	const FName OpeningChapters[] = {
		TEXT("Chapter.DeepWaterStationFinale"),
		TEXT("Chapter.ForestOfSpikes")
	};

	for (const FName ChapterId : OpeningChapters)
	{
		const FHorrorCampaignChapterDefinition* Chapter = FHorrorCampaign::FindChapterById(ChapterId);
		TestNotNull(FString::Printf(TEXT("Opening chapter should exist: %s."), *ChapterId.ToString()), Chapter);
		if (!Chapter)
		{
			return false;
		}

		const FHorrorCampaignObjectiveDefinition* SurvivalObjective = nullptr;
		for (const FHorrorCampaignObjectiveDefinition& Objective : Chapter->Objectives)
		{
			if (Objective.ObjectiveType == EHorrorCampaignObjectiveType::SurviveAmbush)
			{
				SurvivalObjective = &Objective;
				break;
			}
		}

		TestNotNull(FString::Printf(TEXT("Opening chapter should include a pursuit objective: %s."), *ChapterId.ToString()), SurvivalObjective);
		if (!SurvivalObjective)
		{
			return false;
		}

		TestTrue(
			FString::Printf(TEXT("Opening pursuit objective should send the player to a distant destination: %s."), *SurvivalObjective->ObjectiveId.ToString()),
			SurvivalObjective->EscapeDestinationOffset.Size2D() >= 2200.0f);
		TestTrue(
			FString::Printf(TEXT("Opening pursuit objective should use a reachable completion radius: %s."), *SurvivalObjective->ObjectiveId.ToString()),
			SurvivalObjective->EscapeCompletionRadius >= 180.0f && SurvivalObjective->EscapeCompletionRadius <= 360.0f);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignSurvivalObjectiveSpawnsAmbushThreatTest,
	"HorrorProject.Game.Campaign.SurvivalObjectiveSpawnsAmbushThreat",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignSurvivalObjectiveSpawnsAmbushThreatTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* ForestChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.ForestOfSpikes"));
	TestNotNull(TEXT("Forest chapter should exist for ambush threat coverage."), ForestChapter);
	if (!ForestChapter)
	{
		return false;
	}

	const int32 SurvivalObjectiveIndex = ForestChapter->Objectives.IndexOfByPredicate(
		[](const FHorrorCampaignObjectiveDefinition& Objective)
		{
			return Objective.ObjectiveType == EHorrorCampaignObjectiveType::SurviveAmbush;
		});
	TestTrue(TEXT("Forest chapter should contain a survival objective."), SurvivalObjectiveIndex != INDEX_NONE);
	if (SurvivalObjectiveIndex == INDEX_NONE)
	{
		return false;
	}

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for ambush threat coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the campaign game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	TestNotNull(TEXT("Ambush threat test should expose the campaign game mode."), GameMode);
	if (!GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(ForestChapter->ChapterId);
	for (int32 ObjectiveIndex = 0; ObjectiveIndex < SurvivalObjectiveIndex; ++ObjectiveIndex)
	{
		TestTrue(
			FString::Printf(TEXT("Setup should complete prerequisite objective %s."), *ForestChapter->Objectives[ObjectiveIndex].ObjectiveId.ToString()),
			GameMode->TryCompleteCampaignObjective(ForestChapter->ChapterId, ForestChapter->Objectives[ObjectiveIndex].ObjectiveId, nullptr));
	}

	AHorrorCampaignObjectiveActor* InstigatorActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(
		FVector(900.0f, 0.0f, 0.0f),
		FRotator::ZeroRotator);
	TestNotNull(TEXT("Ambush objective test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Ambush objective test should spawn a survival objective actor."), ObjectiveActor);
	if (!InstigatorActor || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	ObjectiveActor->ConfigureObjective(ForestChapter->ChapterId, ForestChapter->Objectives[SurvivalObjectiveIndex]);
	const FHitResult EmptyHit;
	TestTrue(TEXT("Interacting with the forest survival objective should start the ambush."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	TestTrue(TEXT("Starting a survival objective should wake a chase threat."), CountAwakeCampaignBossActors(World) >= 1);

	ObjectiveActor->Tick(ObjectiveActor->GetTimedObjectiveDurationSeconds() + 0.1f);
	TestTrue(TEXT("The temporary chase threat should keep pursuing until the player reaches the escape marker."), CountAwakeCampaignBossActors(World) >= 1);
	InstigatorActor->SetActorLocation(ObjectiveActor->GetActorLocation());
	ObjectiveActor->Tick(0.1f);
	TestFalse(TEXT("Reaching the escape marker after the survival timer should clear the temporary chase threat."), CountAwakeCampaignBossActors(World) >= 1);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignAmbushThreatTuningScalesByMapTest,
	"HorrorProject.Game.Campaign.AmbushThreatTuningScalesByMap",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignAmbushThreatTuningScalesByMapTest::RunTest(const FString& Parameters)
{
	AHorrorGameModeBase* GameMode = NewObject<AHorrorGameModeBase>();
	TestNotNull(TEXT("Ambush tuning test should create a game mode."), GameMode);
	if (!GameMode)
	{
		return false;
	}

	const FHorrorCampaignAmbushThreatTuning BodycamTuning =
		GameMode->ResolveCampaignAmbushThreatTuningForMapForTests(TEXT("/Game/Bodycam_VHS_Effect/Maps/LVL_Showcase_01"));
	const FHorrorCampaignAmbushThreatTuning ForestTuning =
		GameMode->ResolveCampaignAmbushThreatTuningForMapForTests(TEXT("/Game/ForestOfSpikes/Levels/Level_ForestOfSpikes_Demo_Night"));
	const FHorrorCampaignAmbushThreatTuning DungeonHallTuning =
		GameMode->ResolveCampaignAmbushThreatTuningForMapForTests(TEXT("/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_3_hall"));
	const FHorrorCampaignAmbushThreatTuning DungeonBossRoomTuning =
		GameMode->ResolveCampaignAmbushThreatTuningForMapForTests(TEXT("/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_5_bossroom"));
	const FHorrorCampaignAmbushThreatTuning DefaultTuning;

	TestTrue(TEXT("Bodycam corridors should use a smaller pursuer scale than the forest."), BodycamTuning.ActorScale < ForestTuning.ActorScale);
	TestTrue(TEXT("The forest chase should use a larger engagement radius than the tight prologue map."), ForestTuning.EngageRadius > BodycamTuning.EngageRadius);
	TestTrue(TEXT("Default ambush tuning should be slower than player walking speed for the prototype escape loop."), DefaultTuning.MoveSpeed < 250.0f);
	TestTrue(TEXT("Bodycam chase should be slower than walking so the opening escape is learnable."), BodycamTuning.MoveSpeed <= 170.0f);
	TestTrue(TEXT("Forest chase should be faster than the prologue but still below player walking speed."), ForestTuning.MoveSpeed > BodycamTuning.MoveSpeed && ForestTuning.MoveSpeed <= 200.0f);
	TestTrue(TEXT("Castle/dungeon chase should be slower so the player can outrun it."), DungeonHallTuning.MoveSpeed <= 135.0f && DungeonBossRoomTuning.MoveSpeed <= 135.0f);
	TestTrue(TEXT("Castle/dungeon attack range should be forgiving during tight turns."), DungeonHallTuning.AttackRadius <= 110.0f && DungeonBossRoomTuning.AttackRadius <= 110.0f);
	TestTrue(TEXT("Castle/dungeon fear pressure should warn without instantly exhausting the player."), DungeonHallTuning.FearPressureRadius <= 1350.0f && DungeonBossRoomTuning.FearPressureRadius <= 1350.0f);
	TestTrue(TEXT("Opening chase attack radii should be forgiving while players learn the route."), BodycamTuning.AttackRadius <= 100.0f && ForestTuning.AttackRadius <= 140.0f);
	TestTrue(TEXT("Opening chase fear pressure should signal danger without exhausting the player immediately."), BodycamTuning.FearPressureRadius <= 1250.0f && ForestTuning.FearPressureRadius <= 1650.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignGameModeExposesCurrentObjectiveNavigationTargetTest,
	"HorrorProject.Game.Campaign.GameModeExposesCurrentObjectiveNavigationTarget",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignGameModeExposesCurrentObjectiveNavigationTargetTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* ForestChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.ForestOfSpikes"));
	TestNotNull(TEXT("Forest chapter should exist for campaign navigation coverage."), ForestChapter);
	if (!ForestChapter || ForestChapter->Objectives.Num() < 2)
	{
		return false;
	}

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for campaign navigation coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the campaign game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	TestNotNull(TEXT("Campaign navigation test should expose the campaign game mode."), GameMode);
	if (!GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(ForestChapter->ChapterId);

	const FVector FirstLocation(250.0f, -80.0f, 30.0f);
	const FVector SecondLocation(900.0f, 120.0f, 30.0f);
	AHorrorCampaignObjectiveActor* FirstObjective = World->SpawnActor<AHorrorCampaignObjectiveActor>(FirstLocation, FRotator::ZeroRotator);
	AHorrorCampaignObjectiveActor* SecondObjective = World->SpawnActor<AHorrorCampaignObjectiveActor>(SecondLocation, FRotator::ZeroRotator);
	TestNotNull(TEXT("First campaign objective actor should spawn for navigation."), FirstObjective);
	TestNotNull(TEXT("Second campaign objective actor should spawn for navigation."), SecondObjective);
	if (!FirstObjective || !SecondObjective)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FirstObjective->ConfigureObjective(ForestChapter->ChapterId, ForestChapter->Objectives[0]);
	SecondObjective->ConfigureObjective(ForestChapter->ChapterId, ForestChapter->Objectives[1]);

	FVector NavigationLocation = FVector::ZeroVector;
	TestTrue(TEXT("GameMode should expose the current campaign objective location."), GameMode->TryGetCurrentCampaignObjectiveWorldLocation(NavigationLocation));
	TestEqual(TEXT("Campaign navigation should begin at the first objective actor."), NavigationLocation, FirstLocation);
	TestTrue(TEXT("Campaign navigation should expose the current objective action label."), !GameMode->GetCurrentCampaignObjectiveActionText().IsEmpty());

	TestTrue(
		TEXT("Completing the first objective should advance the campaign navigation target."),
		GameMode->TryCompleteCampaignObjective(ForestChapter->ChapterId, ForestChapter->Objectives[0].ObjectiveId, nullptr));
	TestTrue(TEXT("GameMode should still expose a campaign navigation location after progress advances."), GameMode->TryGetCurrentCampaignObjectiveWorldLocation(NavigationLocation));
	TestEqual(TEXT("Campaign navigation should move to the second objective actor."), NavigationLocation, SecondLocation);
	const FString AdvancedActionText = GameMode->GetCurrentCampaignObjectiveActionText().ToString();
	TestTrue(TEXT("Advanced campaign objectives should name their action mode for HUD navigation."), AdvancedActionText.Contains(TEXT("信标"))
		|| AdvancedActionText.Contains(TEXT("多段")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignGameModeAppliesHorrorAtmosphereOnceTest,
	"HorrorProject.Game.Campaign.GameModeAppliesHorrorAtmosphereOnce",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignGameModeAppliesHorrorAtmosphereOnceTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for campaign atmosphere coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorGameModeBase* GameMode = World->SpawnActor<AHorrorGameModeBase>();
	ADirectionalLight* DirectionalLight = World->SpawnActor<ADirectionalLight>();
	TestNotNull(TEXT("Campaign atmosphere test should spawn a game mode."), GameMode);
	TestNotNull(TEXT("Campaign atmosphere test should spawn a dimmable light."), DirectionalLight);
	if (!GameMode || !DirectionalLight || !DirectionalLight->GetLightComponent())
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	ULightComponent* LightComponent = DirectionalLight->GetLightComponent();
	LightComponent->SetIntensity(10.0f);

	const int32 FirstPassCount = GameMode->ApplyCampaignHorrorAtmosphere();
	const float DimmedIntensity = LightComponent->Intensity;
	TestTrue(TEXT("Campaign atmosphere should affect at least one placed light."), FirstPassCount >= 1);
	TestTrue(TEXT("Campaign atmosphere should dim placed map lights."), DimmedIntensity < 10.0f);

	APostProcessVolume* RuntimePostProcess = nullptr;
	for (TActorIterator<APostProcessVolume> It(World); It; ++It)
	{
		RuntimePostProcess = *It;
		break;
	}
	TestNotNull(TEXT("Campaign atmosphere should add a runtime post-process volume."), RuntimePostProcess);
	TestTrue(TEXT("Runtime atmosphere post-process should affect the whole level."), RuntimePostProcess && RuntimePostProcess->bUnbound);

	const int32 SecondPassCount = GameMode->ApplyCampaignHorrorAtmosphere();
	TestEqual(TEXT("Campaign atmosphere should not dim the same lights twice."), SecondPassCount, 0);
	TestEqual(TEXT("Second atmosphere pass should preserve dimmed light intensity."), LightComponent->Intensity, DimmedIntensity);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignScrapopolisSanitizesImportedVisualObstructionsTest,
	"HorrorProject.Game.Campaign.ScrapopolisSanitizesImportedVisualObstructions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignScrapopolisSanitizesImportedVisualObstructionsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for Scrapopolis visual sanitation coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorGameModeBase* GameMode = World->SpawnActor<AHorrorGameModeBase>();
	APostProcessVolume* ImportedPostProcess = World->SpawnActor<APostProcessVolume>();
	AExponentialHeightFog* ImportedFog = World->SpawnActor<AExponentialHeightFog>();
	TestNotNull(TEXT("Scrapopolis visual sanitation test should spawn a game mode."), GameMode);
	TestNotNull(TEXT("Scrapopolis visual sanitation test should spawn an imported post-process volume."), ImportedPostProcess);
	TestNotNull(TEXT("Scrapopolis visual sanitation test should spawn imported fog."), ImportedFog);
	if (!GameMode || !ImportedPostProcess || !ImportedFog || !ImportedFog->GetComponent())
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	ImportedPostProcess->bEnabled = true;
	ImportedPostProcess->bUnbound = true;
	ImportedPostProcess->BlendWeight = 1.0f;
	ImportedPostProcess->Settings.bOverride_AutoExposureBias = true;
	ImportedPostProcess->Settings.AutoExposureBias = -10.0f;
	ImportedPostProcess->Settings.bOverride_ColorGain = true;
	ImportedPostProcess->Settings.ColorGain = FVector4(0.0f, 0.0f, 0.0f, 0.0f);
	ImportedPostProcess->Settings.bOverride_ColorGamma = true;
	ImportedPostProcess->Settings.ColorGamma = FVector4(0.0f, 0.0f, 0.0f, 0.0f);

	UExponentialHeightFogComponent* FogComponent = ImportedFog->GetComponent();
	FogComponent->SetFogDensity(3.0f);
	FogComponent->SetFogMaxOpacity(1.0f);
	FogComponent->SetVolumetricFog(true);
	FogComponent->SetVolumetricFogExtinctionScale(30.0f);

	const int32 SanitationCount =
		GameMode->SanitizeImportedMapVisualObstructionsForTests(TEXT("/Game/Scrapopolis/Levels/Level_Scrapopolis_Demo"));
	TestTrue(TEXT("Scrapopolis sanitation should touch the blocking imported visuals and add a rescue light."), SanitationCount >= 3);
	TestFalse(TEXT("Imported unbound post-process should be disabled so it cannot black out the playable camera."), ImportedPostProcess->bEnabled);
	TestEqual(TEXT("Imported unbound post-process blend should be neutralized."), ImportedPostProcess->BlendWeight, 0.0f);
	TestFalse(TEXT("Imported fog should have volumetric fog disabled for the black-screen regression."), FogComponent->bEnableVolumetricFog);
	TestTrue(TEXT("Imported fog density should be clamped to a readable value."), FogComponent->FogDensity <= 0.04f);
	TestTrue(TEXT("Imported fog opacity should be clamped to a readable value."), FogComponent->FogMaxOpacity <= 0.35f);

	APointLight* RescueLight = nullptr;
	for (TActorIterator<APointLight> It(World); It; ++It)
	{
		RescueLight = *It;
		break;
	}

	TestNotNull(TEXT("Scrapopolis sanitation should add a rescue point light near the player start."), RescueLight);
	if (RescueLight)
	{
		TestTrue(TEXT("Rescue light should be bright enough to make the spawn readable."), RescueLight->GetLightComponent()->Intensity >= 3500.0f);
		if (const ULocalLightComponent* LocalLight = Cast<ULocalLightComponent>(RescueLight->GetLightComponent()))
		{
			TestTrue(TEXT("Rescue light should cover a useful radius around the spawn."), LocalLight->AttenuationRadius >= 1800.0f);
		}
	}

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignScrapopolisUsesReadableAtmosphereTuningTest,
	"HorrorProject.Game.Campaign.ScrapopolisUsesReadableAtmosphereTuning",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignScrapopolisUsesReadableAtmosphereTuningTest::RunTest(const FString& Parameters)
{
	AHorrorGameModeBase* GameMode = NewObject<AHorrorGameModeBase>();
	TestNotNull(TEXT("Atmosphere tuning test should create a game mode."), GameMode);
	if (!GameMode)
	{
		return false;
	}

	const FHorrorCampaignAtmosphereTuning DefaultTuning =
		GameMode->ResolveCampaignAtmosphereTuningForMapForTests(TEXT("/Game/ForestOfSpikes/Levels/Level_ForestOfSpikes_Demo_Night"));
	const FHorrorCampaignAtmosphereTuning ScrapopolisTuning =
		GameMode->ResolveCampaignAtmosphereTuningForMapForTests(TEXT("/Game/Scrapopolis/Levels/Level_Scrapopolis_Demo"));

	TestTrue(TEXT("Scrapopolis should keep more map light than the default horror profile."), ScrapopolisTuning.LightScale > DefaultTuning.LightScale);
	TestTrue(TEXT("Scrapopolis should use a less negative exposure bias to avoid the black-screen failure."), ScrapopolisTuning.AutoExposureBias > DefaultTuning.AutoExposureBias);
	TestTrue(TEXT("Scrapopolis should use a lighter vignette than the default horror profile."), ScrapopolisTuning.VignetteIntensity < DefaultTuning.VignetteIntensity);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignImportedMapCameraFadeResetTest,
	"HorrorProject.Game.Campaign.ImportedMapCameraFadeReset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignImportedMapCameraFadeResetTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for camera fade reset coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	APlayerController* PlayerController = World->SpawnActor<APlayerController>();
	APlayerCameraManager* CameraManager = World->SpawnActor<APlayerCameraManager>();
	TestNotNull(TEXT("Camera fade reset test should spawn a player controller."), PlayerController);
	TestNotNull(TEXT("Camera fade reset test should spawn a camera manager."), CameraManager);
	if (!PlayerController || !CameraManager)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerCameraManager = CameraManager;
	CameraManager->SetManualCameraFade(1.0f, FLinearColor::Black, false);
	TestTrue(TEXT("The imported showcase map can leave a held full-screen fade."), CameraManager->bEnableFading);
	TestEqual(TEXT("The held showcase fade should start fully opaque for this regression."), CameraManager->FadeAmount, 1.0f);

	TestTrue(TEXT("GameMode camera sanitation should clear a held imported-map fade."), AHorrorGameModeBase::ClearImportedMapCameraFade(PlayerController));
	TestFalse(TEXT("Camera fade should be disabled after imported-map sanitation."), CameraManager->bEnableFading);
	TestEqual(TEXT("Camera fade amount should be reset after imported-map sanitation."), CameraManager->FadeAmount, 0.0f);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignBossActorAppliesFearPressureTest,
	"HorrorProject.Game.Campaign.BossActorAppliesFearPressure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignBossActorAppliesFearPressureTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for boss pressure coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorCampaignBossActor* BossActor = World->SpawnActor<AHorrorCampaignBossActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	AActor* TargetActor = World->SpawnActor<AActor>(FVector(500.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Boss actor should spawn for fear pressure coverage."), BossActor);
	TestNotNull(TEXT("Fear pressure target should spawn."), TargetActor);
	if (!BossActor || !TargetActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UFearComponent* FearComponent = NewObject<UFearComponent>(TargetActor);
	TargetActor->AddInstanceComponent(FearComponent);
	FearComponent->RegisterComponent();

	BossActor->ConfigureBoss(TEXT("Chapter.StoneGolemBoss"), FText::FromString(TEXT("Stone Golem")), 3);
	BossActor->SetBossAwake(true);
	TestTrue(TEXT("Living boss should apply fear pressure inside its aura."), BossActor->ApplyBossPressureToActor(TargetActor, 1.0f));
	const float InitialFear = FearComponent->GetFearValue();
	TestTrue(TEXT("Boss pressure should raise the target fear value."), InitialFear > 0.0f);

	BossActor->SetBossDefeated(true);
	TestFalse(TEXT("Defeated boss should stop fear pressure."), BossActor->ApplyBossPressureToActor(TargetActor, 1.0f));
	TestEqual(TEXT("Defeated boss pressure should leave fear unchanged."), FearComponent->GetFearValue(), InitialFear);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignBossActorMovesAndAttacksTest,
	"HorrorProject.Game.Campaign.BossActorMovesAndAttacks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignBossActorMovesAndAttacksTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for boss movement coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorCampaignBossActor* BossActor = World->SpawnActor<AHorrorCampaignBossActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHorrorCampaignObjectiveActor* TargetActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector(1000.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Boss movement test should spawn a boss actor."), BossActor);
	TestNotNull(TEXT("Boss movement test should spawn a target actor."), TargetActor);
	if (!BossActor || !TargetActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	BossActor->ConfigureBoss(TEXT("Chapter.StoneGolemBoss"), FText::FromString(TEXT("Stone Golem")), 3);
	const float InitialDistance = FVector::Dist(BossActor->GetActorLocation(), TargetActor->GetActorLocation());
	TestFalse(TEXT("Configured boss should wait dormant until the arena altar wakes it."), BossActor->IsBossAwake());
	TestFalse(TEXT("Dormant boss should not move before its wake objective resolves."), BossActor->MoveBossTowardActor(TargetActor, 1.0f));

	BossActor->SetBossAwake(true);
	TestTrue(TEXT("Boss should report awake after the arena altar resolves."), BossActor->IsBossAwake());
	TestTrue(TEXT("Living boss should move toward an engageable target."), BossActor->MoveBossTowardActor(TargetActor, 1.0f));
	TestTrue(
		TEXT("Boss movement should reduce the distance to its target."),
		FVector::Dist(BossActor->GetActorLocation(), TargetActor->GetActorLocation()) < InitialDistance);

	TargetActor->SetActorLocation(BossActor->GetActorLocation() + FVector(120.0f, 0.0f, 0.0f));
	TestTrue(TEXT("Boss should recognize a target inside attack range."), BossActor->CanBossAttackActor(TargetActor));
	TestTrue(TEXT("Boss should trigger an attack on a nearby target."), BossActor->TryTriggerBossAttack(TargetActor));
	TestFalse(TEXT("Boss attack cooldown should block immediate repeat attacks."), BossActor->TryTriggerBossAttack(TargetActor));

	BossActor->SetBossDefeated(true);
	TargetActor->SetActorLocation(BossActor->GetActorLocation() + FVector(1000.0f, 0.0f, 0.0f));
	TestFalse(TEXT("Defeated boss should stop moving."), BossActor->MoveBossTowardActor(TargetActor, 1.0f));
	TestFalse(TEXT("Defeated boss should stop attacking."), BossActor->CanBossAttackActor(TargetActor));
	TestFalse(TEXT("Defeated boss should no longer report awake."), BossActor->IsBossAwake());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignBossAttackUsesArenaRecoveryInsteadOfDay1CheckpointTest,
	"HorrorProject.Game.Campaign.BossAttackUsesArenaRecoveryInsteadOfDay1Checkpoint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignBossAttackUsesArenaRecoveryInsteadOfDay1CheckpointTest::RunTest(const FString& Parameters)
{
	const FString AutosaveSlotName(TEXT("SM13_Day1_Autosave"));
	constexpr int32 AutosaveUserIndex = 0;
	UGameplayStatics::DeleteGameInSlot(AutosaveSlotName, AutosaveUserIndex);

	const FHorrorCampaignChapterDefinition* BossChapter = FHorrorCampaign::FindBossChapter();
	TestNotNull(TEXT("Boss chapter should exist for boss attack recovery coverage."), BossChapter);
	if (!BossChapter || BossChapter->Objectives.Num() < 2)
	{
		UGameplayStatics::DeleteGameInSlot(AutosaveSlotName, AutosaveUserIndex);
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* LeftShoulderObjective =
		FHorrorCampaign::FindObjectiveById(*BossChapter, TEXT("Boss.WeakPoint.LeftShoulder"));
	TestNotNull(TEXT("Boss chapter should expose the left shoulder weak point objective."), LeftShoulderObjective);
	if (!LeftShoulderObjective)
	{
		UGameplayStatics::DeleteGameInSlot(AutosaveSlotName, AutosaveUserIndex);
		return false;
	}

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for boss attack recovery coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		UGameplayStatics::DeleteGameInSlot(AutosaveSlotName, AutosaveUserIndex);
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the campaign recovery game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerCharacter* PlayerCharacter = SpawnControlledCampaignPlayer(*this, World, FVector::ZeroVector);
	TestNotNull(TEXT("Boss attack recovery should expose the game mode."), GameMode);
	TestNotNull(TEXT("Boss attack recovery should spawn a controlled player."), PlayerCharacter);
	if (!GameMode || !PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		UGameplayStatics::DeleteGameInSlot(AutosaveSlotName, AutosaveUserIndex);
		return false;
	}

	const FVector OldDay1CheckpointLocation(90000.0f, -90000.0f, 5000.0f);
	PlayerCharacter->SetActorLocation(OldDay1CheckpointLocation);
	TestTrue(TEXT("Boss attack recovery test should save a deliberately stale Day1 checkpoint first."), GameMode->SaveDay1Checkpoint(TEXT("Checkpoint.Day1.StaleBeforeBoss")));

	GameMode->ResetCampaignProgressForChapterForTests(BossChapter->ChapterId);
	AHorrorCampaignBossActor* BossActor = World->SpawnActor<AHorrorCampaignBossActor>(FVector(1450.0f, 0.0f, 100.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Boss attack recovery should spawn an arena boss for awake-state coverage."), BossActor);
	if (!BossActor)
	{
		TestWorld.DestroyTestWorld(false);
		UGameplayStatics::DeleteGameInSlot(AutosaveSlotName, AutosaveUserIndex);
		return false;
	}
	BossActor->ConfigureBoss(BossChapter->ChapterId, FText::FromString(TEXT("石像巨人")), 3);
	BossActor->SetBossAwake(true);
	TestTrue(
		TEXT("Completing the arena altar should make the left shoulder the current boss objective."),
		GameMode->TryCompleteCampaignObjective(BossChapter->ChapterId, BossChapter->Objectives[0].ObjectiveId, PlayerCharacter));
	TestEqual(TEXT("Boss chapter should now point at the left shoulder objective."), GameMode->GetCurrentCampaignObjectivePromptText().ToString(), LeftShoulderObjective->PromptText.ToString());

	PlayerCharacter->SetActorLocation(FVector(1450.0f, 0.0f, 100.0f));
	TestTrue(
		TEXT("Boss attack failure should recover the player inside the current arena instead of loading stale Day1 coordinates."),
		GameMode->RequestPlayerFailure(TEXT("Failure.Boss.StoneGolemAttack"), FText::FromString(TEXT("巨人抓住了你。"))));

	const FVector RecoveredLocation = PlayerCharacter->GetActorLocation();
	TestTrue(
		TEXT("Boss attack recovery should not use the stale Day1 checkpoint outside the boss map."),
		FVector::Dist(RecoveredLocation, OldDay1CheckpointLocation) > 10000.0f);
	TestTrue(
		TEXT("Boss attack recovery should place the player close enough to retry the left shoulder weak point."),
		FVector::Dist2D(RecoveredLocation, LeftShoulderObjective->RelativeLocation) <= 900.0f);
	TestTrue(TEXT("Boss attack recovery should keep the arena boss awake after the safe reset."), CountAwakeCampaignBossActors(World) >= 1);

	UGameplayStatics::DeleteGameInSlot(AutosaveSlotName, AutosaveUserIndex);
	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignGameModeLocksMapExitUntilChapterCompleteTest,
	"HorrorProject.Game.Campaign.GameModeLocksMapExitUntilChapterComplete",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignGameModeLocksMapExitUntilChapterCompleteTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* ForestChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.ForestOfSpikes"));
	TestNotNull(TEXT("Forest chapter should exist."), ForestChapter);
	if (!ForestChapter)
	{
		return false;
	}

	AHorrorGameModeBase* GameMode = NewObject<AHorrorGameModeBase>();
	TestNotNull(TEXT("Campaign GameMode test should create a game mode."), GameMode);
	if (!GameMode)
	{
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(ForestChapter->ChapterId);
	TestFalse(TEXT("Map-chain exit should be locked when a campaign chapter starts."), GameMode->IsCurrentCampaignChapterComplete());

	if (ForestChapter->Objectives.Num() >= 2)
	{
		TestTrue(
			TEXT("GameMode should allow the next campaign objective."),
			GameMode->CanCompleteCampaignObjective(ForestChapter->ChapterId, ForestChapter->Objectives[0].ObjectiveId));
		TestFalse(
			TEXT("GameMode should reject later objectives until the current one is complete."),
			GameMode->CanCompleteCampaignObjective(ForestChapter->ChapterId, ForestChapter->Objectives[1].ObjectiveId));
	}

	for (const FHorrorCampaignObjectiveDefinition& Objective : ForestChapter->Objectives)
	{
		TestTrue(
			FString::Printf(TEXT("GameMode should complete campaign objective: %s."), *Objective.ObjectiveId.ToString()),
			GameMode->TryCompleteCampaignObjective(ForestChapter->ChapterId, Objective.ObjectiveId, nullptr));
	}

	TestTrue(TEXT("Map-chain exit should unlock after all chapter objectives complete."), GameMode->IsCurrentCampaignChapterComplete());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignProgressionLocksExitUntilObjectivesCompleteTest,
	"HorrorProject.Game.Campaign.ProgressionLocksExitUntilObjectivesComplete",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignProgressionLocksExitUntilObjectivesCompleteTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* ForestChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.ForestOfSpikes"));
	TestNotNull(TEXT("Forest chapter should exist."), ForestChapter);
	if (!ForestChapter)
	{
		return false;
	}

	FHorrorCampaignProgress Progress;
	Progress.ResetForChapter(*ForestChapter);

	TestFalse(TEXT("A new chapter should start incomplete."), Progress.IsChapterComplete());
	TestEqual(TEXT("No objectives should be completed at chapter start."), Progress.GetCompletedObjectiveCount(), 0);

	if (ForestChapter->Objectives.Num() >= 2)
	{
		TestFalse(
			TEXT("Campaign progression should reject out-of-order objective completion."),
			Progress.TryCompleteObjective(ForestChapter->Objectives[1].ObjectiveId));
		TestEqual(TEXT("Rejected out-of-order objectives should not change progress."), Progress.GetCompletedObjectiveCount(), 0);
	}

	for (const FHorrorCampaignObjectiveDefinition& Objective : ForestChapter->Objectives)
	{
		TestTrue(
			FString::Printf(TEXT("Objective should complete in sequence: %s."), *Objective.ObjectiveId.ToString()),
			Progress.TryCompleteObjective(Objective.ObjectiveId));
	}

	TestEqual(TEXT("All forest objectives should be counted."), Progress.GetCompletedObjectiveCount(), ForestChapter->Objectives.Num());
	TestTrue(TEXT("A non-boss chapter should unlock after all required objectives complete."), Progress.IsChapterComplete());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignBossChapterRequiresWeakPointResolutionTest,
	"HorrorProject.Game.Campaign.BossChapterRequiresWeakPointResolution",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignBossChapterRequiresWeakPointResolutionTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* BossChapter = FHorrorCampaign::FindBossChapter();
	TestNotNull(TEXT("A boss chapter should be configured."), BossChapter);
	if (!BossChapter)
	{
		return false;
	}

	FHorrorCampaignProgress Progress;
	Progress.ResetForChapter(*BossChapter);

	TestFalse(TEXT("Boss should start undefeated."), Progress.IsBossDefeated());
	TestFalse(TEXT("Boss chapter should not start complete."), Progress.IsChapterComplete());

	int32 CompletedWeakPoints = 0;
	for (const FHorrorCampaignObjectiveDefinition& Objective : BossChapter->Objectives)
	{
		TestTrue(
			FString::Printf(TEXT("Boss objective should complete in sequence: %s."), *Objective.ObjectiveId.ToString()),
			Progress.TryCompleteObjective(Objective.ObjectiveId));
		if (Objective.ObjectiveType == EHorrorCampaignObjectiveType::BossWeakPoint)
		{
			++CompletedWeakPoints;
			if (CompletedWeakPoints < FHorrorCampaign::CountObjectivesOfType(*BossChapter, EHorrorCampaignObjectiveType::BossWeakPoint))
			{
				TestFalse(TEXT("Boss should remain active until every weak point is resolved."), Progress.IsBossDefeated());
			}
		}
	}

	TestTrue(TEXT("Resolving every configured boss weak point should defeat the boss."), Progress.IsBossDefeated());
	TestTrue(TEXT("The boss chapter should unlock after objectives and boss resolution."), Progress.IsChapterComplete());

	return true;
}

#endif
