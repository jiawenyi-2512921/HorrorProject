// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorCampaign.h"
#include "Game/HorrorCampaignBossActor.h"
#include "Game/HorrorCampaignObjectiveActor.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Game/HorrorGameModeBase.h"
#include "Game/HorrorMapChain.h"
#include "Player/Components/CameraBatteryComponent.h"
#include "Player/Components/FearComponent.h"
#include "Player/Components/InventoryComponent.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "Animation/AnimationAsset.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/LightComponent.h"
#include "Components/LocalLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PointLight.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
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

bool ContainsDisallowedEnglishText(const FString& Text)
{
	for (int32 CharacterIndex = 0; CharacterIndex < Text.Len(); ++CharacterIndex)
	{
		const TCHAR Character = Text[CharacterIndex];
		if ((Character >= TEXT('A') && Character <= TEXT('Z')) || (Character >= TEXT('a') && Character <= TEXT('z')))
		{
			if ((Character == TEXT('A') || Character == TEXT('D') || Character == TEXT('S'))
				&& (CharacterIndex == 0 || !FChar::IsAlpha(Text[CharacterIndex - 1]))
				&& (CharacterIndex == Text.Len() - 1 || !FChar::IsAlpha(Text[CharacterIndex + 1])))
			{
				continue;
			}
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

AHorrorCampaignBossActor* FindFirstAwakeCampaignBossActor(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AHorrorCampaignBossActor> It(World); It; ++It)
	{
		AHorrorCampaignBossActor* BossActor = *It;
		if (BossActor && BossActor->IsBossAwake())
		{
			return BossActor;
		}
	}

	return nullptr;
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

bool IsFiniteVector(const FVector& Location)
{
	return FMath::IsFinite(Location.X)
		&& FMath::IsFinite(Location.Y)
		&& FMath::IsFinite(Location.Z)
		&& !Location.ContainsNaN();
}

void TestAdvancedInteractionDiagnosticsAreLocalized(
	FAutomationTestBase& Test,
	const FHorrorAdvancedInteractionHUDState& State,
	const TCHAR* Context)
{
	Test.TestFalse(
		FString::Printf(TEXT("%s should expose a device diagnostic label."), Context),
		State.DeviceStatusLabel.IsEmpty());
	Test.TestFalse(
		FString::Printf(TEXT("%s should expose a player-readable risk label."), Context),
		State.RiskLabel.IsEmpty());
	Test.TestFalse(
		FString::Printf(TEXT("%s should expose a timing rhythm label."), Context),
		State.RhythmLabel.IsEmpty());
	Test.TestFalse(
		FString::Printf(TEXT("%s should expose a next-action operator cue."), Context),
		State.NextActionLabel.IsEmpty());
	Test.TestTrue(
		FString::Printf(TEXT("%s should expose a bounded performance grade."), Context),
		State.PerformanceGradeFraction >= 0.0f && State.PerformanceGradeFraction <= 1.0f);
	Test.TestFalse(
		FString::Printf(TEXT("%s diagnostic label should not expose English letters."), Context),
		ContainsLatinLetter(State.DeviceStatusLabel.ToString()));
	Test.TestFalse(
		FString::Printf(TEXT("%s risk label should not expose English letters."), Context),
		ContainsLatinLetter(State.RiskLabel.ToString()));
	Test.TestFalse(
		FString::Printf(TEXT("%s rhythm label should not expose English letters."), Context),
		ContainsLatinLetter(State.RhythmLabel.ToString()));
	Test.TestFalse(
		FString::Printf(TEXT("%s next-action cue should not expose English letters."), Context),
		ContainsDisallowedEnglishText(State.NextActionLabel.ToString()));
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
	const FHorrorCampaignObjectiveDefinition* RootGlyphObjective = ForestChapter
		? FHorrorCampaign::FindObjectiveById(*ForestChapter, TEXT("Forest.ReadRootGlyph"))
		: nullptr;
	const FHorrorCampaignObjectiveDefinition* SpikeBeaconObjective = ForestChapter
		? FHorrorCampaign::FindObjectiveById(*ForestChapter, TEXT("Forest.AlignSpikeBeacon"))
		: nullptr;
	const FHorrorCampaignObjectiveDefinition* PursuitObjective = ForestChapter
		? FHorrorCampaign::FindObjectiveById(*ForestChapter, TEXT("Forest.HoldSpikeCircle"))
		: nullptr;
	TestNotNull(TEXT("Forest root glyph objective should exist."), RootGlyphObjective);
	TestNotNull(TEXT("Forest spike beacon objective should exist."), SpikeBeaconObjective);
	TestNotNull(TEXT("Forest pursuit objective should exist."), PursuitObjective);
	if (!ForestChapter || !RootGlyphObjective || !SpikeBeaconObjective || !PursuitObjective)
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
	AHorrorCampaignObjectiveActor* LockedObjective = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("First campaign objective actor should spawn."), FirstObjective);
	TestNotNull(TEXT("Second campaign objective actor should spawn."), SecondObjective);
	TestNotNull(TEXT("Locked campaign objective actor should spawn."), LockedObjective);
	if (!FirstObjective || !SecondObjective || !LockedObjective)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FirstObjective->ConfigureObjective(ForestChapter->ChapterId, *RootGlyphObjective);
	SecondObjective->ConfigureObjective(ForestChapter->ChapterId, *SpikeBeaconObjective);
	LockedObjective->ConfigureObjective(ForestChapter->ChapterId, *PursuitObjective);

	TestTrue(TEXT("The first objective actor should be available at chapter start."), FirstObjective->IsAvailableForInteraction());
	TestTrue(TEXT("The second dependency-free objective actor should be available at chapter start."), SecondObjective->IsAvailableForInteraction());
	TestFalse(TEXT("The dependent pursuit objective should wait for its beacon prerequisite."), LockedObjective->IsAvailableForInteraction());
	const FString LockedPrompt = LockedObjective->GetInteractionPromptText().ToString();
	TestTrue(TEXT("Locked campaign objectives should name the missing prerequisite objective."), LockedPrompt.Contains(SpikeBeaconObjective->PromptText.ToString()));

	TestTrue(
		TEXT("Completing the beacon prerequisite through the game mode should advance campaign progress."),
		GameMode->TryCompleteCampaignObjective(ForestChapter->ChapterId, SpikeBeaconObjective->ObjectiveId, nullptr));
	LockedObjective->RefreshObjectiveState();
	TestTrue(TEXT("The dependent pursuit objective should become available after its prerequisite."), LockedObjective->IsAvailableForInteraction());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignLockedObjectivePromptListsAllMissingPrerequisitesTest,
	"HorrorProject.Game.Campaign.LockedObjectivePromptListsAllMissingPrerequisites",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignLockedObjectivePromptListsAllMissingPrerequisitesTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* EntranceChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.DungeonEntrance"));
	TestNotNull(TEXT("Dungeon entrance chapter should exist for locked prompt coverage."), EntranceChapter);
	if (!EntranceChapter)
	{
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* FogDoorObjective =
		FHorrorCampaign::FindObjectiveById(*EntranceChapter, TEXT("Entrance.ScanFogDoor"));
	const FHorrorCampaignObjectiveDefinition* GolemFingerObjective =
		FHorrorCampaign::FindObjectiveById(*EntranceChapter, TEXT("Entrance.RecoverGolemFinger"));
	const FHorrorCampaignObjectiveDefinition* OuterSealObjective =
		FHorrorCampaign::FindObjectiveById(*EntranceChapter, TEXT("Entrance.DisableOuterSeal"));
	TestNotNull(TEXT("Entrance fog-door prerequisite should exist."), FogDoorObjective);
	TestNotNull(TEXT("Entrance golem-finger prerequisite should exist."), GolemFingerObjective);
	TestNotNull(TEXT("Entrance outer-seal merge objective should exist."), OuterSealObjective);
	if (!FogDoorObjective || !GolemFingerObjective || !OuterSealObjective)
	{
		return false;
	}

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for locked prerequisite prompt coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the campaign game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	TestNotNull(TEXT("Locked prerequisite prompt test should expose the campaign game mode."), GameMode);
	if (!GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(EntranceChapter->ChapterId);

	AHorrorCampaignObjectiveActor* LockedObjective = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Locked outer-seal objective actor should spawn."), LockedObjective);
	if (!LockedObjective)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	LockedObjective->ConfigureObjective(EntranceChapter->ChapterId, *OuterSealObjective);

	const FString LockedPrompt = LockedObjective->GetInteractionPromptText().ToString();
	TestTrue(TEXT("Locked prompt should explain that prerequisite work is missing."), LockedPrompt.Contains(TEXT("先完成")));
	TestTrue(TEXT("Locked prompt should list the missing fog-door scan."), LockedPrompt.Contains(FogDoorObjective->PromptText.ToString()));
	TestTrue(TEXT("Locked prompt should list the missing golem-finger relic."), LockedPrompt.Contains(GolemFingerObjective->PromptText.ToString()));

	TestTrue(TEXT("Completing one prerequisite should be allowed."), GameMode->TryCompleteCampaignObjective(EntranceChapter->ChapterId, FogDoorObjective->ObjectiveId, nullptr));
	LockedObjective->RefreshObjectiveState();

	const FString PartiallyLockedPrompt = LockedObjective->GetInteractionPromptText().ToString();
	TestFalse(TEXT("Completed prerequisites should drop out of the locked prompt."), PartiallyLockedPrompt.Contains(FogDoorObjective->PromptText.ToString()));
	TestTrue(TEXT("Remaining missing prerequisites should stay visible."), PartiallyLockedPrompt.Contains(GolemFingerObjective->PromptText.ToString()));

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
	FHorrorCampaignObjectiveRuntimeState RuntimeState = ObjectiveActor->BuildObjectiveRuntimeState();
	TestTrue(TEXT("Multi-step runtime should describe the active beat after each step."), RuntimeState.PhaseText.ToString().Contains(TEXT("阶段")));
	TestTrue(TEXT("Multi-step runtime should expose normalized progress after the first step."), RuntimeState.ProgressFraction > 0.0f && RuntimeState.ProgressFraction < 1.0f);

	TestTrue(TEXT("Second interaction step should be accepted."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	TestEqual(TEXT("Second interaction should advance progress."), ObjectiveActor->GetInteractionProgressCount(), 2);
	TestFalse(TEXT("Second interaction should not complete a three-step objective."), ObjectiveActor->IsCompleted());
	RuntimeState = ObjectiveActor->BuildObjectiveRuntimeState();
	TestTrue(TEXT("Multi-step runtime should continue to show a localized stage."), RuntimeState.PhaseText.ToString().Contains(TEXT("阶段")));
	TestFalse(TEXT("Multi-step runtime phase should not expose English text."), ContainsLatinLetter(RuntimeState.PhaseText.ToString()));

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
	int32 SpectralScanObjectiveCount = 0;
	int32 SignalTuningObjectiveCount = 0;
	int32 BeaconTuningObjectiveCount = 0;
	int32 BossWeakPointScanObjectiveCount = 0;
	int32 UpgradedRelicObjectiveCount = 0;

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
					FString::Printf(TEXT("SurviveAmbush should use the giant pursuit mode: %s."), *Objective.ObjectiveId.ToString()),
					Objective.InteractionMode,
					EHorrorCampaignInteractionMode::TimedPursuit);
				++TimedPursuitObjectiveCount;
			}
			else if (Objective.ObjectiveType == EHorrorCampaignObjectiveType::PlantBeacon)
			{
				TestEqual(
					FString::Printf(TEXT("PlantBeacon should use a signal tuning anchor window instead of plain multi-step input: %s."), *Objective.ObjectiveId.ToString()),
					Objective.InteractionMode,
					EHorrorCampaignInteractionMode::SignalTuning);
				++SignalTuningObjectiveCount;
				++BeaconTuningObjectiveCount;
			}
			else if (Objective.ObjectiveType == EHorrorCampaignObjectiveType::BossWeakPoint)
			{
				TestEqual(
					FString::Printf(TEXT("BossWeakPoint should use a spectral resonance window instead of plain multi-step input: %s."), *Objective.ObjectiveId.ToString()),
					Objective.InteractionMode,
					EHorrorCampaignInteractionMode::SpectralScan);
				++SpectralScanObjectiveCount;
				++BossWeakPointScanObjectiveCount;
			}
			else if (Objective.ObjectiveType == EHorrorCampaignObjectiveType::ScanAnomaly)
			{
				TestEqual(
					FString::Printf(TEXT("ScanAnomaly should open the spectral scan window: %s."), *Objective.ObjectiveId.ToString()),
					Objective.InteractionMode,
					EHorrorCampaignInteractionMode::SpectralScan);
				++SpectralScanObjectiveCount;
			}
			else if (Objective.ObjectiveType == EHorrorCampaignObjectiveType::AcquireSignal)
			{
				TestEqual(
					FString::Printf(TEXT("AcquireSignal should open the signal tuning window: %s."), *Objective.ObjectiveId.ToString()),
					Objective.InteractionMode,
					EHorrorCampaignInteractionMode::SignalTuning);
				++SignalTuningObjectiveCount;
			}
			else if (Objective.ObjectiveType == EHorrorCampaignObjectiveType::RecoverRelic)
			{
				TestNotEqual(
					FString::Printf(TEXT("Mainline relic recovery should not be a one-click instant pickup: %s."), *Objective.ObjectiveId.ToString()),
					Objective.InteractionMode,
					EHorrorCampaignInteractionMode::Instant);
				TestTrue(
					FString::Printf(TEXT("Mainline relic recovery should open an authored interaction window or multi-step recovery: %s."), *Objective.ObjectiveId.ToString()),
					Objective.Presentation.bOpensInteractionPanel
						|| Objective.InteractionMode == EHorrorCampaignInteractionMode::MultiStep);
				++UpgradedRelicObjectiveCount;
			}
		}
	}

	TestTrue(TEXT("The campaign should include multiple circuit wiring tasks."), CircuitObjectiveCount >= 4);
	TestTrue(TEXT("The campaign should include multiple gear calibration tasks."), GearObjectiveCount >= 3);
	TestTrue(TEXT("The campaign should include several giant pursuit tasks."), TimedPursuitObjectiveCount >= 2);
	TestTrue(TEXT("The campaign should include multiple spectral scan tasks."), SpectralScanObjectiveCount >= 6);
	TestTrue(TEXT("The campaign should include multiple signal tuning tasks."), SignalTuningObjectiveCount >= 4);
	TestTrue(TEXT("The campaign should include several signal-tuned beacon anchor tasks."), BeaconTuningObjectiveCount >= 3);
	TestTrue(TEXT("The campaign should include several spectral boss weak-point pressure tasks."), BossWeakPointScanObjectiveCount >= 3);
	TestTrue(TEXT("The campaign should upgrade several key relic recoveries beyond one-click pickup."), UpgradedRelicObjectiveCount >= 5);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignChaptersMaintainGameplayVarietyTest,
	"HorrorProject.Game.Campaign.ChaptersMaintainGameplayVariety",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignChaptersMaintainGameplayVarietyTest::RunTest(const FString& Parameters)
{
	int32 ChapterCount = 0;
	for (const FHorrorCampaignChapterDefinition& Chapter : FHorrorCampaign::GetChapters())
	{
		++ChapterCount;
		TSet<EHorrorCampaignInteractionMode> InteractionModes;
		int32 AdvancedPanelObjectiveCount = 0;
		int32 RequiredAdvancedPanelObjectiveCount = 0;
		int32 OptionalInvestigationCount = 0;
		int32 PressureObjectiveCount = 0;

		for (const FHorrorCampaignObjectiveDefinition& Objective : Chapter.Objectives)
		{
			InteractionModes.Add(Objective.InteractionMode);
			const bool bAdvancedPanelObjective =
				Objective.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring
				|| Objective.InteractionMode == EHorrorCampaignInteractionMode::GearCalibration
				|| Objective.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan
				|| Objective.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning;
			AdvancedPanelObjectiveCount += bAdvancedPanelObjective ? 1 : 0;
			RequiredAdvancedPanelObjectiveCount += bAdvancedPanelObjective && Objective.bRequiredForChapterCompletion ? 1 : 0;
			OptionalInvestigationCount += (Objective.bOptional || !Objective.bRequiredForChapterCompletion) ? 1 : 0;
			PressureObjectiveCount += (Objective.ObjectiveType == EHorrorCampaignObjectiveType::SurviveAmbush
				|| Objective.ObjectiveType == EHorrorCampaignObjectiveType::BossWeakPoint
				|| Objective.InteractionMode == EHorrorCampaignInteractionMode::TimedPursuit)
				? 1
				: 0;
		}

		TestTrue(
			FString::Printf(TEXT("Chapter should mix at least three interaction modes instead of repeating one mechanic: %s."), *Chapter.ChapterId.ToString()),
			InteractionModes.Num() >= 3);
		TestTrue(
			FString::Printf(TEXT("Chapter should contain at least two authored advanced panel objectives: %s."), *Chapter.ChapterId.ToString()),
			AdvancedPanelObjectiveCount >= 2);
		TestTrue(
			FString::Printf(TEXT("Chapter should keep at least one required advanced panel objective on the main route: %s."), *Chapter.ChapterId.ToString()),
			RequiredAdvancedPanelObjectiveCount >= 1);
		TestTrue(
			FString::Printf(TEXT("Chapter should preserve optional investigation so players can choose a side read: %s."), *Chapter.ChapterId.ToString()),
			OptionalInvestigationCount >= 1);

		if (Chapter.bRequiresBoss || Chapter.bIsFinalChapter)
		{
			TestTrue(
				FString::Printf(TEXT("Boss and finale chapters should include pressure gameplay beyond quiet investigation: %s."), *Chapter.ChapterId.ToString()),
				PressureObjectiveCount >= 1);
		}
	}

	TestTrue(TEXT("Campaign should validate gameplay variety across all playable chapters."), ChapterCount >= 8);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignPressureObjectivesOpenAdvancedWindowsTest,
	"HorrorProject.Game.Campaign.PressureObjectivesOpenAdvancedWindows",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignPressureObjectivesOpenAdvancedWindowsTest::RunTest(const FString& Parameters)
{
	int32 CheckedPressureObjectiveCount = 0;
	for (const FHorrorCampaignChapterDefinition& Chapter : FHorrorCampaign::GetChapters())
	{
		for (const FHorrorCampaignObjectiveDefinition& Objective : Chapter.Objectives)
		{
			if (Objective.ObjectiveType != EHorrorCampaignObjectiveType::PlantBeacon
				&& Objective.ObjectiveType != EHorrorCampaignObjectiveType::BossWeakPoint)
			{
				continue;
			}

			++CheckedPressureObjectiveCount;
			TestTrue(
				FString::Printf(TEXT("Pressure objective should open a focused interaction panel: %s."), *Objective.ObjectiveId.ToString()),
				Objective.Presentation.bOpensInteractionPanel);
			TestFalse(
				FString::Printf(TEXT("Pressure objective should not fall back to plain multi-step gameplay: %s."), *Objective.ObjectiveId.ToString()),
				Objective.InteractionMode == EHorrorCampaignInteractionMode::MultiStep);
			TestTrue(
				FString::Printf(TEXT("Pressure objective should advertise a localized window mechanic: %s."), *Objective.ObjectiveId.ToString()),
				Objective.Presentation.MechanicLabel.ToString().Contains(TEXT("窗口"))
					|| Objective.Presentation.MechanicLabel.ToString().Contains(TEXT("调谐"))
					|| Objective.Presentation.MechanicLabel.ToString().Contains(TEXT("频谱")));
			TestTrue(
				FString::Printf(TEXT("Pressure objective should include a retryable advanced panel beat: %s."), *Objective.ObjectiveId.ToString()),
				Objective.ObjectiveBeats.ContainsByPredicate(
					[](const FHorrorCampaignObjectiveBeat& Beat)
					{
						return Beat.CompletionRule == EHorrorCampaignObjectiveBeatCompletionRule::AdvancedWindow
							&& Beat.bOpensInteractionPanel
							&& Beat.FailurePolicy == EHorrorCampaignObjectiveBeatFailurePolicy::RetryBeat;
					}));
		}
	}

	TestTrue(TEXT("Campaign should validate several upgraded pressure objectives."), CheckedPressureObjectiveCount >= 6);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveActorRunsPressureWindowsTest,
	"HorrorProject.Game.Campaign.ObjectiveActorRunsPressureWindows",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveActorRunsPressureWindowsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for pressure window runtime coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	TestNotNull(TEXT("Pressure window runtime test should spawn an instigator."), InstigatorActor);
	if (!InstigatorActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	struct FPressureRuntimeCase
	{
		EHorrorCampaignObjectiveType ObjectiveType = EHorrorCampaignObjectiveType::PlantBeacon;
		FName ObjectiveId = NAME_None;
		const TCHAR* PromptText = TEXT("");
		EHorrorCampaignInteractionMode ExpectedMode = EHorrorCampaignInteractionMode::Instant;
		const TCHAR* ExpectedDeviceFragment = TEXT("");
	};

	const FPressureRuntimeCase Cases[] = {
		{
			EHorrorCampaignObjectiveType::PlantBeacon,
			TEXT("Test.RuntimeBeaconWindow"),
			TEXT("锚定测试信标"),
			EHorrorCampaignInteractionMode::SignalTuning,
			TEXT("声像")
		},
		{
			EHorrorCampaignObjectiveType::BossWeakPoint,
			TEXT("Test.RuntimeWeakPointWindow"),
			TEXT("压制测试弱点"),
			EHorrorCampaignInteractionMode::SpectralScan,
			TEXT("扫描")
		}
	};

	for (const FPressureRuntimeCase& RuntimeCase : Cases)
	{
		AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
		TestNotNull(
			FString::Printf(TEXT("Pressure window runtime test should spawn objective actor for %s."), *RuntimeCase.ObjectiveId.ToString()),
			ObjectiveActor);
		if (!ObjectiveActor)
		{
			TestWorld.DestroyTestWorld(false);
			return false;
		}

		FHorrorCampaignObjectiveDefinition Objective;
		Objective.ObjectiveId = RuntimeCase.ObjectiveId;
		Objective.ObjectiveType = RuntimeCase.ObjectiveType;
		Objective.PromptText = FText::FromString(RuntimeCase.PromptText);
		Objective.AdvancedInteractionTuning.RequiredStepCount = 3;
		Objective.AdvancedInteractionTuning.SuccessProgress = 0.34f;

		ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);
		TestEqual(
			FString::Printf(TEXT("Pressure objective should resolve to an advanced runtime mode for %s."), *RuntimeCase.ObjectiveId.ToString()),
			ObjectiveActor->GetInteractionMode(),
			RuntimeCase.ExpectedMode);
		TestTrue(
			FString::Printf(TEXT("Pressure objective should open its advanced window for %s."), *RuntimeCase.ObjectiveId.ToString()),
			ObjectiveActor->Interact_Implementation(InstigatorActor, FHitResult()));
		ObjectiveActor->Tick(0.9f);

		const FHorrorCampaignObjectiveRuntimeState RuntimeState = ObjectiveActor->BuildObjectiveRuntimeState();
		TestTrue(
			FString::Printf(TEXT("Pressure runtime should mark an active advanced panel for %s."), *RuntimeCase.ObjectiveId.ToString()),
			RuntimeState.bAdvancedInteractionActive);
		TestEqual(
			FString::Printf(TEXT("Pressure runtime should expose the expected mode for %s."), *RuntimeCase.ObjectiveId.ToString()),
			RuntimeState.AdvancedInteraction.Mode,
			RuntimeCase.ExpectedMode);
		TestTrue(
			FString::Printf(TEXT("Pressure runtime should expose three modal inputs for %s."), *RuntimeCase.ObjectiveId.ToString()),
			RuntimeState.AdvancedInteraction.InputOptions.Num() == 3);
		TestTrue(
			FString::Printf(TEXT("Pressure runtime should expose localized diagnostics for %s."), *RuntimeCase.ObjectiveId.ToString()),
			RuntimeState.AdvancedInteraction.DeviceStatusLabel.ToString().Contains(RuntimeCase.ExpectedDeviceFragment));
		TestFalse(
			FString::Printf(TEXT("Pressure runtime diagnostics should remain localized for %s."), *RuntimeCase.ObjectiveId.ToString()),
			ContainsLatinLetter(RuntimeState.AdvancedInteraction.DeviceStatusLabel.ToString()));
	}

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignDeepWaterBlackBoxScanUsesPlayableScanWindowTest,
	"HorrorProject.Game.Campaign.DeepWaterBlackBoxScanUsesPlayableScanWindow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignDeepWaterBlackBoxScanUsesPlayableScanWindowTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* DeepWaterChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.DeepWaterStationFinale"));
	TestNotNull(TEXT("Deep Water chapter should exist for black-box scan coverage."), DeepWaterChapter);
	if (!DeepWaterChapter)
	{
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* BootObjective =
		FHorrorCampaign::FindObjectiveById(*DeepWaterChapter, TEXT("DeepWater.BootDryDock"));
	const FHorrorCampaignObjectiveDefinition* BlackBoxScanObjective =
		FHorrorCampaign::FindObjectiveById(*DeepWaterChapter, TEXT("DeepWater.DecodeHeartbeat"));
	TestNotNull(TEXT("Deep Water should expose the boot objective."), BootObjective);
	TestNotNull(TEXT("Deep Water should expose the black-box heartbeat scan objective."), BlackBoxScanObjective);
	if (!BootObjective || !BlackBoxScanObjective)
	{
		return false;
	}

	TestTrue(
		TEXT("The black-box scan should stay near the boot terminal so players can find it after restoring power."),
		FVector::Dist2D(BootObjective->RelativeLocation, BlackBoxScanObjective->RelativeLocation) <= 520.0f);
	TestEqual(
		TEXT("The black-box scan should use the spectral scan window instead of a plain one-tap interaction."),
		BlackBoxScanObjective->InteractionMode,
		EHorrorCampaignInteractionMode::SpectralScan);
	TestTrue(
		TEXT("The black-box scan presentation should explicitly open a focused interaction panel."),
		BlackBoxScanObjective->Presentation.bOpensInteractionPanel);
	TestTrue(
		TEXT("The black-box scan should advertise a scanning mechanic in Chinese."),
		BlackBoxScanObjective->Presentation.MechanicLabel.ToString().Contains(TEXT("扫描")));
	TestTrue(
		TEXT("The black-box scan should include a panel beat so HUD and gameplay agree."),
		BlackBoxScanObjective->ObjectiveBeats.ContainsByPredicate(
			[](const FHorrorCampaignObjectiveBeat& Beat)
			{
				return Beat.CompletionRule == EHorrorCampaignObjectiveBeatCompletionRule::AdvancedWindow
					&& Beat.bOpensInteractionPanel;
			}));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectivesExposeCompositeBeatMetadataTest,
	"HorrorProject.Game.Campaign.ObjectivesExposeCompositeBeatMetadata",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectivesExposeCompositeBeatMetadataTest::RunTest(const FString& Parameters)
{
	int32 CompositeObjectiveCount = 0;
	int32 UrgentObjectiveCount = 0;

	for (const FHorrorCampaignChapterDefinition& Chapter : FHorrorCampaign::GetChapters())
	{
		for (const FHorrorCampaignObjectiveDefinition& Objective : Chapter.Objectives)
		{
			if (!Objective.bRequiredForChapterCompletion)
			{
				continue;
			}

			TestTrue(
				FString::Printf(TEXT("Required campaign objective should expose at least three authored beats: %s."), *Objective.ObjectiveId.ToString()),
				Objective.ObjectiveBeats.Num() >= 3);
			if (Objective.ObjectiveBeats.Num() < 3)
			{
				continue;
			}

			++CompositeObjectiveCount;
			for (int32 BeatIndex = 0; BeatIndex < Objective.ObjectiveBeats.Num(); ++BeatIndex)
			{
				const FHorrorCampaignObjectiveBeat& Beat = Objective.ObjectiveBeats[BeatIndex];
				const FString BeatVisibleText = FString::Printf(
					TEXT("%s %s"),
					*Beat.Label.ToString(),
					*Beat.Detail.ToString());
				TestFalse(
					FString::Printf(TEXT("Objective beat label should be readable: %s[%d]."), *Objective.ObjectiveId.ToString(), BeatIndex),
					Beat.Label.IsEmpty());
				TestFalse(
					FString::Printf(TEXT("Objective beat text should not contain mojibake markers: %s[%d]."), *Objective.ObjectiveId.ToString(), BeatIndex),
					ContainsMojibakeMarker(BeatVisibleText));
				TestFalse(
					FString::Printf(TEXT("Objective beat text should stay localized in Chinese: %s[%d]."), *Objective.ObjectiveId.ToString(), BeatIndex),
					ContainsLatinLetter(BeatVisibleText));
				TestFalse(
					FString::Printf(TEXT("Objective beat should carry a stable beat id: %s[%d]."), *Objective.ObjectiveId.ToString(), BeatIndex),
					Beat.BeatId.IsNone());
				TestTrue(
					FString::Printf(TEXT("Objective beat should expose a concrete completion rule: %s[%d]."), *Objective.ObjectiveId.ToString(), BeatIndex),
					Beat.CompletionRule != EHorrorCampaignObjectiveBeatCompletionRule::None);
				TestTrue(
					FString::Printf(TEXT("Objective beat should expose a navigation role: %s[%d]."), *Objective.ObjectiveId.ToString(), BeatIndex),
					Beat.NavigationRole != EHorrorCampaignObjectiveBeatNavigationRole::None);
			}

			if (Objective.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring
				|| Objective.InteractionMode == EHorrorCampaignInteractionMode::GearCalibration
				|| Objective.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan
				|| Objective.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning)
			{
				TestTrue(
					FString::Printf(TEXT("Advanced interaction objective should mark at least one panel beat: %s."), *Objective.ObjectiveId.ToString()),
					Objective.ObjectiveBeats.ContainsByPredicate(
						[](const FHorrorCampaignObjectiveBeat& Beat)
						{
							return Beat.CompletionRule == EHorrorCampaignObjectiveBeatCompletionRule::AdvancedWindow
								&& Beat.bOpensInteractionPanel
							&& Beat.FailurePolicy == EHorrorCampaignObjectiveBeatFailurePolicy::RetryBeat;
						}));
			}

			if (Objective.ObjectiveType == EHorrorCampaignObjectiveType::FinalTerminal)
			{
				TestEqual(
					FString::Printf(TEXT("Final terminal objectives should use a signal tuning finale instead of a single key press: %s."), *Objective.ObjectiveId.ToString()),
					Objective.InteractionMode,
					EHorrorCampaignInteractionMode::SignalTuning);
				TestTrue(
					FString::Printf(TEXT("Final terminal objectives should open a focused finale panel: %s."), *Objective.ObjectiveId.ToString()),
					Objective.Presentation.bOpensInteractionPanel);
			}

			if (Objective.InteractionMode == EHorrorCampaignInteractionMode::TimedPursuit)
			{
				TestTrue(
					FString::Printf(TEXT("Timed pursuit objective should mark its escape navigation beat: %s."), *Objective.ObjectiveId.ToString()),
					Objective.ObjectiveBeats.ContainsByPredicate(
						[](const FHorrorCampaignObjectiveBeat& Beat)
						{
							return Beat.CompletionRule == EHorrorCampaignObjectiveBeatCompletionRule::ReachEscapePoint
								&& Beat.NavigationRole == EHorrorCampaignObjectiveBeatNavigationRole::EscapeDestination
								&& Beat.FailurePolicy == EHorrorCampaignObjectiveBeatFailurePolicy::CampaignRecovery;
						}));
				TestTrue(
					FString::Printf(TEXT("Timed pursuit objective should read like a giant chase: %s."), *Objective.ObjectiveId.ToString()),
					Objective.PromptText.ToString().Contains(TEXT("巨人"))
						|| Objective.CompletionText.ToString().Contains(TEXT("巨人"))
						|| Objective.ObjectiveBeats.ContainsByPredicate(
							[](const FHorrorCampaignObjectiveBeat& Beat)
							{
								return Beat.Label.ToString().Contains(TEXT("巨人"))
									|| Beat.Detail.ToString().Contains(TEXT("巨人"));
							}));
				TestTrue(
					FString::Printf(TEXT("Timed pursuit objective should allow a fair chase window: %s."), *Objective.ObjectiveId.ToString()),
					Objective.TimedObjectiveDurationSeconds >= 18.0f);
			}

			if (Objective.ObjectiveType == EHorrorCampaignObjectiveType::SurviveAmbush
				|| Objective.ObjectiveType == EHorrorCampaignObjectiveType::BossWeakPoint
				|| Objective.InteractionMode == EHorrorCampaignInteractionMode::TimedPursuit)
			{
				const bool bHasUrgentBeat = Objective.ObjectiveBeats.ContainsByPredicate(
					[](const FHorrorCampaignObjectiveBeat& Beat)
					{
						return Beat.bUrgent;
					});
				TestTrue(
					FString::Printf(TEXT("Pressure objectives should mark at least one urgent beat: %s."), *Objective.ObjectiveId.ToString()),
					bHasUrgentBeat);
				UrgentObjectiveCount += bHasUrgentBeat ? 1 : 0;
			}
		}
	}

	TestTrue(TEXT("The campaign should expose composite beats across the full playable route."), CompositeObjectiveCount >= 20);
	TestTrue(TEXT("The campaign should expose several urgent beats for chase/boss pressure."), UrgentObjectiveCount >= 4);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectivesUseBranchingAndOptionalInvestigationTest,
	"HorrorProject.Game.Campaign.ObjectivesUseBranchingAndOptionalInvestigation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectivesUseBranchingAndOptionalInvestigationTest::RunTest(const FString& Parameters)
{
	int32 OptionalObjectiveCount = 0;
	int32 ChapterWithDependencyGraphCount = 0;
	int32 ChapterWithParallelStartsCount = 0;
	int32 ChapterWithOptionalInvestigationCount = 0;

	for (const FHorrorCampaignChapterDefinition& Chapter : FHorrorCampaign::GetChapters())
	{
		int32 ObjectivesWithPrerequisites = 0;
		int32 DependencyFreeRequiredObjectives = 0;
		bool bHasOptionalObjective = false;

		for (const FHorrorCampaignObjectiveDefinition& Objective : Chapter.Objectives)
		{
			if (!Objective.PrerequisiteObjectiveIds.IsEmpty())
			{
				++ObjectivesWithPrerequisites;
			}

			if (Objective.bRequiredForChapterCompletion && Objective.PrerequisiteObjectiveIds.IsEmpty())
			{
				++DependencyFreeRequiredObjectives;
			}

			if (Objective.bOptional || !Objective.bRequiredForChapterCompletion)
			{
				++OptionalObjectiveCount;
				bHasOptionalObjective = true;
				TestFalse(
					FString::Printf(TEXT("Optional objective should still carry readable Chinese text: %s."), *Objective.ObjectiveId.ToString()),
					Objective.PromptText.IsEmpty());
				TestTrue(
					FString::Printf(TEXT("Optional objective should not block required chapter completion: %s."), *Objective.ObjectiveId.ToString()),
					!Objective.bRequiredForChapterCompletion);
			}
		}

		ChapterWithDependencyGraphCount += ObjectivesWithPrerequisites >= 2 ? 1 : 0;
		ChapterWithParallelStartsCount += DependencyFreeRequiredObjectives >= 2 ? 1 : 0;
		ChapterWithOptionalInvestigationCount += bHasOptionalObjective ? 1 : 0;
	}

	TestTrue(TEXT("The campaign should include optional investigation objectives across several chapters."), OptionalObjectiveCount >= 4);
	TestTrue(TEXT("Multiple chapters should contain dependency graph objectives."), ChapterWithDependencyGraphCount >= 5);
	TestTrue(TEXT("Several chapters should open with parallel required objectives."), ChapterWithParallelStartsCount >= 3);
	TestTrue(TEXT("Optional investigation should appear in more than one chapter."), ChapterWithOptionalInvestigationCount >= 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveTransformsStayWithinPlayableBoundsTest,
	"HorrorProject.Game.Campaign.ObjectiveTransformsStayWithinPlayableBounds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveTransformsStayWithinPlayableBoundsTest::RunTest(const FString& Parameters)
{
	constexpr float MaxAuthoredObjectiveDistanceCm = 3600.0f;
	constexpr float MaxAuthoredObjectiveAbsZCm = 450.0f;
	constexpr float MaxEscapeDestinationDistanceCm = 3600.0f;
	constexpr float MinConfiguredEscapeDestinationDistanceCm = 2200.0f;
	constexpr float MinTightDungeonEscapeDestinationDistanceCm = 650.0f;
	constexpr float MaxTightDungeonEscapeDestinationDistanceCm = 1500.0f;
	constexpr float MaxEscapeDestinationAbsZCm = 450.0f;

	int32 CheckedObjectiveCount = 0;
	int32 CheckedEscapeDestinationCount = 0;

	for (const FHorrorCampaignChapterDefinition& Chapter : FHorrorCampaign::GetChapters())
	{
		for (const FHorrorCampaignObjectiveDefinition& Objective : Chapter.Objectives)
		{
			++CheckedObjectiveCount;
			TestTrue(
				FString::Printf(TEXT("Campaign objective should use finite authored coordinates: %s."), *Objective.ObjectiveId.ToString()),
				IsFiniteVector(Objective.RelativeLocation));
			TestTrue(
				FString::Printf(TEXT("Campaign objective should stay inside the authored playable radius: %s."), *Objective.ObjectiveId.ToString()),
				Objective.RelativeLocation.Size2D() <= MaxAuthoredObjectiveDistanceCm);
			TestTrue(
				FString::Printf(TEXT("Campaign objective should not be authored far above or below the playable floor: %s."), *Objective.ObjectiveId.ToString()),
				FMath::Abs(Objective.RelativeLocation.Z) <= MaxAuthoredObjectiveAbsZCm);

			if (Objective.ObjectiveType == EHorrorCampaignObjectiveType::SurviveAmbush)
			{
				++CheckedEscapeDestinationCount;
				TestTrue(
					FString::Printf(TEXT("Survival objective should use finite escape destination coordinates: %s."), *Objective.ObjectiveId.ToString()),
					IsFiniteVector(Objective.EscapeDestinationOffset));

				if (!Objective.EscapeDestinationOffset.IsNearlyZero())
				{
					const bool bTightDungeonHallChase =
						Chapter.ChapterId == FName(TEXT("Chapter.DungeonHall"))
						&& Objective.ObjectiveId == FName(TEXT("Hall.SurviveEchoPulse"));
					const float EscapeDestinationDistanceCm = Objective.EscapeDestinationOffset.Size2D();
					TestTrue(
						FString::Printf(TEXT("Survival objective should send the player far enough for its map scale: %s."), *Objective.ObjectiveId.ToString()),
						bTightDungeonHallChase
							? EscapeDestinationDistanceCm >= MinTightDungeonEscapeDestinationDistanceCm
							: EscapeDestinationDistanceCm >= MinConfiguredEscapeDestinationDistanceCm);
					TestTrue(
						FString::Printf(TEXT("Survival objective escape destination should stay inside the playable radius: %s."), *Objective.ObjectiveId.ToString()),
						bTightDungeonHallChase
							? EscapeDestinationDistanceCm <= MaxTightDungeonEscapeDestinationDistanceCm
							: EscapeDestinationDistanceCm <= MaxEscapeDestinationDistanceCm);
				}

				TestTrue(
					FString::Printf(TEXT("Survival objective escape destination should not move vertically outside the playable floor band: %s."), *Objective.ObjectiveId.ToString()),
					FMath::Abs(Objective.EscapeDestinationOffset.Z) <= MaxEscapeDestinationAbsZCm);
			}
		}
	}

	TestTrue(TEXT("Campaign should validate several authored objective transforms."), CheckedObjectiveCount >= 30);
	TestTrue(TEXT("Campaign should validate several survival escape destinations."), CheckedEscapeDestinationCount >= 4);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveActorTracksCompositeBeatStateTest,
	"HorrorProject.Game.Campaign.ObjectiveActorTracksCompositeBeatState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveActorTracksCompositeBeatStateTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for composite objective beat coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Composite beat test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Composite beat test should spawn an objective actor."), ObjectiveActor);
	if (!InstigatorActor || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.CompositeSignal");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::PlantBeacon;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::MultiStep;
	Objective.PromptText = FText::FromString(TEXT("部署测试信标"));
	Objective.CompletionText = FText::FromString(TEXT("测试信标已稳定。"));
	FHorrorCampaignObjectiveBeat LocateBeat(FText::FromString(TEXT("定位")), FText::FromString(TEXT("确认地面裂隙方向。")));
	LocateBeat.CompletionRule = EHorrorCampaignObjectiveBeatCompletionRule::InteractOnce;
	FHorrorCampaignObjectiveBeat CalibrateBeat(FText::FromString(TEXT("校准")), FText::FromString(TEXT("压住信标直到频率对齐。")));
	CalibrateBeat.CompletionRule = EHorrorCampaignObjectiveBeatCompletionRule::HoldInteract;
	CalibrateBeat.FailurePolicy = EHorrorCampaignObjectiveBeatFailurePolicy::RetryBeat;
	FHorrorCampaignObjectiveBeat ConfirmBeat(FText::FromString(TEXT("确认")), FText::FromString(TEXT("观察回声是否退潮。")));
	ConfirmBeat.CompletionRule = EHorrorCampaignObjectiveBeatCompletionRule::ConfirmResult;
	Objective.ObjectiveBeats.Add(LocateBeat);
	Objective.ObjectiveBeats.Add(CalibrateBeat);
	Objective.ObjectiveBeats.Add(ConfirmBeat);

	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);
	TestEqual(TEXT("Objective actor should expose the configured beat count."), ObjectiveActor->GetObjectiveBeatCount(), 3);
	TestEqual(TEXT("Objective actor should begin on the first beat."), ObjectiveActor->GetCurrentObjectiveBeatLabel().ToString(), FString(TEXT("定位")));
	TestEqual(TEXT("Objective actor should expose the first beat detail."), ObjectiveActor->GetCurrentObjectiveBeatDetail().ToString(), FString(TEXT("确认地面裂隙方向。")));
	TestTrue(TEXT("Interaction prompt should include the active beat label."), ObjectiveActor->GetInteractionPromptText().ToString().Contains(TEXT("定位")));

	const FHitResult EmptyHit;
	TestTrue(TEXT("First composite interaction should advance the objective."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	TestEqual(TEXT("Objective actor should move to the next beat after progress."), ObjectiveActor->GetCurrentObjectiveBeatLabel().ToString(), FString(TEXT("校准")));
	TestTrue(TEXT("Updated prompt should include the next beat label."), ObjectiveActor->GetInteractionPromptText().ToString().Contains(TEXT("校准")));
	FHorrorCampaignObjectiveRuntimeState RuntimeState = ObjectiveActor->BuildObjectiveRuntimeState();
	TestTrue(TEXT("Composite runtime phase should include the active beat label."), RuntimeState.PhaseText.ToString().Contains(TEXT("校准")));
	TestTrue(TEXT("Composite runtime phase should include the active beat detail."), RuntimeState.PhaseText.ToString().Contains(TEXT("频率")));
	TestTrue(TEXT("Composite runtime should tell the player to hold interaction for standard beats."), RuntimeState.NextActionLabel.ToString().Contains(TEXT("长按")));
	TestFalse(TEXT("Composite runtime next action should remain localized."), ContainsLatinLetter(RuntimeState.NextActionLabel.ToString()));

	TestTrue(TEXT("Second composite interaction should advance to the confirmation beat."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	TestEqual(TEXT("Objective actor should expose the final beat before completion."), ObjectiveActor->GetCurrentObjectiveBeatLabel().ToString(), FString(TEXT("确认")));
	RuntimeState = ObjectiveActor->BuildObjectiveRuntimeState();
	TestTrue(TEXT("Composite runtime phase should update to the final beat."), RuntimeState.PhaseText.ToString().Contains(TEXT("确认")));
	TestTrue(TEXT("Composite runtime should tell the player to confirm the result on the final beat."), RuntimeState.NextActionLabel.ToString().Contains(TEXT("确认")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignTrackerUsesChapterProgressAndObjectiveBeatsTest,
	"HorrorProject.Game.Campaign.TrackerUsesChapterProgressAndObjectiveBeats",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignTrackerUsesChapterProgressAndObjectiveBeatsTest::RunTest(const FString& Parameters)
{
	AHorrorGameModeBase* GameMode = NewObject<AHorrorGameModeBase>();
	TestNotNull(TEXT("Campaign tracker test should create a game mode."), GameMode);
	if (!GameMode)
	{
		return false;
	}

	const FHorrorCampaignChapterDefinition* ForestChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.ForestOfSpikes"));
	TestNotNull(TEXT("Forest chapter should exist for tracker coverage."), ForestChapter);
	if (!ForestChapter)
	{
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(ForestChapter->ChapterId);
	FHorrorObjectiveTrackerSnapshot Tracker = GameMode->BuildObjectiveTrackerSnapshot();

	TestEqual(TEXT("Campaign tracker should expose campaign objective stage."), Tracker.Stage, EHorrorObjectiveTrackerStage::CampaignObjective);
	TestTrue(TEXT("Campaign tracker title should include the active chapter title."), Tracker.Title.ToString().Contains(TEXT("尖刺林地")));
	TestTrue(TEXT("Campaign tracker should expose chapter progress instead of Day1 evidence progress."), Tracker.ProgressLabel.ToString().Contains(TEXT("章节任务")));
	TestEqual(TEXT("Campaign tracker should expose completed campaign objectives."), Tracker.CompletedMilestoneCount, 0);
	TestEqual(TEXT("Campaign tracker should expose required campaign objectives."), Tracker.RequiredMilestoneCount, FHorrorCampaign::CountRequiredObjectives(*ForestChapter));
	TestTrue(TEXT("Campaign tracker should include the current objective beat checklist."), Tracker.ChecklistItems.Num() >= 3);
	TestEqual(TEXT("The first campaign beat should be active."), Tracker.ChecklistItems[0].bActive, true);
	TestFalse(TEXT("The first campaign beat should not be complete before progress."), Tracker.ChecklistItems[0].bComplete);
	TestFalse(TEXT("Campaign beat labels should not be empty."), Tracker.ChecklistItems[0].Label.IsEmpty());
	TestFalse(TEXT("Campaign tracker should carry standard beat next-action guidance."), Tracker.ChecklistItems[0].NextActionLabel.IsEmpty());
	TestFalse(TEXT("Campaign tracker should carry standard beat recovery guidance."), Tracker.ChecklistItems[0].FailureRecoveryLabel.IsEmpty());
	TestFalse(TEXT("Campaign tracker standard beat guidance should remain localized."), ContainsLatinLetter(Tracker.ChecklistItems[0].NextActionLabel.ToString() + Tracker.ChecklistItems[0].FailureRecoveryLabel.ToString()));
	TestFalse(TEXT("Campaign tracker should not show the Day1 bodycam checklist during campaign objectives."), Tracker.ChecklistItems[0].Label.ToString().Contains(TEXT("随身摄像机")));
	TestEqual(TEXT("Campaign tracker primary checklist should only expose the active objective beats."), Tracker.ChecklistItems.Num(), ForestChapter->Objectives[0].ObjectiveBeats.Num());
	TestFalse(TEXT("Campaign tracker primary checklist should not leak parallel objectives into the active task list."), Tracker.ChecklistItems.ContainsByPredicate(
		[](const FHorrorObjectiveChecklistItem& Item)
		{
			return Item.Label.ToString().Contains(TEXT("放置信标稳定林地路径"));
		}));
	TestTrue(TEXT("Campaign tracker should expose a focused chapter objective graph."), Tracker.ObjectiveGraphItems.Num() > 0);
	TestNull(TEXT("Campaign tracker graph should not advertise parallel spike beacon as the current executable story objective."), Tracker.ObjectiveGraphItems.FindByPredicate(
		[](const FHorrorObjectiveChecklistItem& Item)
		{
			return Item.ObjectiveId == FName(TEXT("Forest.AlignSpikeBeacon"))
				&& Item.bActive
				&& Item.StatusText.ToString().Contains(TEXT("可执行"));
		}));
	TestNotNull(TEXT("Campaign tracker graph should show parallel mainline objectives without stealing the active beat list."), Tracker.ObjectiveGraphItems.FindByPredicate(
		[](const FHorrorObjectiveChecklistItem& Item)
		{
			return Item.ObjectiveId == FName(TEXT("Forest.AlignSpikeBeacon"))
				&& Item.bMainline
				&& !Item.bActive
				&& !Item.bBlocked
				&& Item.StatusText.ToString().Contains(TEXT("并行主线"));
		}));
	TestNull(TEXT("Campaign tracker should not mark a navigation focus before the player cycles objectives."), Tracker.ObjectiveGraphItems.FindByPredicate(
		[](const FHorrorObjectiveChecklistItem& Item)
		{
			return Item.bNavigationFocused;
		}));
	TestTrue(
		TEXT("Campaign tracker should accept a manual navigation focus for the parallel spike beacon."),
		GameMode->SetCampaignNavigationFocusObjective(ForestChapter->ChapterId, TEXT("Forest.AlignSpikeBeacon")));
	Tracker = GameMode->BuildObjectiveTrackerSnapshot();
	TestNotNull(TEXT("Campaign tracker graph should flag the manually focused navigation objective."), Tracker.ObjectiveGraphItems.FindByPredicate(
		[](const FHorrorObjectiveChecklistItem& Item)
		{
			return Item.ObjectiveId == FName(TEXT("Forest.AlignSpikeBeacon"))
				&& Item.bNavigationFocused
				&& !Item.bActive
				&& Item.StatusText.ToString().Contains(TEXT("导航锁定"));
		}));
	TestTrue(TEXT("Clearing the navigation focus should be explicit for subsequent tracker assertions."), GameMode->ClearCampaignNavigationFocusObjective());
	Tracker = GameMode->BuildObjectiveTrackerSnapshot();
	TestNotNull(TEXT("Campaign tracker graph should show available optional investigations without selecting them as the current story objective."), Tracker.ObjectiveGraphItems.FindByPredicate(
		[](const FHorrorObjectiveChecklistItem& Item)
		{
			return Item.ObjectiveId == FName(TEXT("Forest.RecordHangingTape"))
				&& Item.bOptional
				&& !Item.bActive
				&& !Item.bBlocked
				&& Item.StatusText.ToString().Contains(TEXT("可选"))
				&& !Item.StatusText.ToString().Contains(TEXT("可执行"));
		}));
	TestNotNull(TEXT("Campaign tracker graph should include locked convergence objectives with lock reasons."), Tracker.ObjectiveGraphItems.FindByPredicate(
		[](const FHorrorObjectiveChecklistItem& Item)
		{
			return Item.ObjectiveId == FName(TEXT("Forest.ExtractBlackSeed"))
				&& Item.bBlocked
				&& !Item.LockReason.IsEmpty();
		}));

	TestTrue(
		TEXT("Completing the first forest objective should advance chapter progress."),
		GameMode->TryCompleteCampaignObjective(ForestChapter->ChapterId, ForestChapter->Objectives[0].ObjectiveId, nullptr));
	Tracker = GameMode->BuildObjectiveTrackerSnapshot();
	TestEqual(TEXT("Campaign tracker should update completed objective count."), Tracker.CompletedMilestoneCount, 1);
	TestTrue(TEXT("Campaign progress fraction should increase after objective completion."), Tracker.ProgressFraction > 0.0f);
	TestFalse(TEXT("Campaign tracker primary instruction should remain useful after graph progress."), Tracker.PrimaryInstruction.IsEmpty());
	TestFalse(TEXT("Campaign tracker primary checklist should not keep completed previous objectives in the active task list."), Tracker.ChecklistItems.ContainsByPredicate(
		[](const FHorrorObjectiveChecklistItem& Item)
		{
			return Item.Label.ToString().Contains(TEXT("读取荆棘根系上的符号"));
		}));
	TestNotNull(TEXT("Campaign tracker graph should keep completed objectives visible as progress context."), Tracker.ObjectiveGraphItems.FindByPredicate(
		[](const FHorrorObjectiveChecklistItem& Item)
		{
			return Item.ObjectiveId == FName(TEXT("Forest.ReadRootGlyph"))
				&& Item.bComplete;
		}));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveGraphExposesParallelAndLockedObjectivesTest,
	"HorrorProject.Game.Campaign.ObjectiveGraphExposesParallelAndLockedObjectives",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignGameModeExposesOnlyCurrentStoryObjectiveTest,
	"HorrorProject.Game.Campaign.GameModeExposesOnlyCurrentStoryObjective",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignTrackerShowsAvailableOptionalInvestigationsTest,
	"HorrorProject.Game.Objectives.TrackerShowsAvailableOptionalInvestigations",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignTrackerShowsAvailableOptionalInvestigationsTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* DeepWaterChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.DeepWaterStationFinale"));
	TestNotNull(TEXT("Deep Water chapter should exist for optional investigation tracker coverage."), DeepWaterChapter);
	if (!DeepWaterChapter)
	{
		return false;
	}

	AHorrorGameModeBase* GameMode = NewObject<AHorrorGameModeBase>();
	TestNotNull(TEXT("Optional investigation tracker test should create a campaign game mode."), GameMode);
	if (!GameMode)
	{
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(DeepWaterChapter->ChapterId);
	const FHorrorObjectiveTrackerSnapshot Tracker = GameMode->BuildObjectiveTrackerSnapshot();
	TestEqual(TEXT("Deep Water tracker should keep the main story objective active."), Tracker.ActiveObjectiveId, FName(TEXT("DeepWater.BootDryDock")));

	const FHorrorObjectiveChecklistItem* OptionalInvestigation = Tracker.ObjectiveGraphItems.FindByPredicate(
		[](const FHorrorObjectiveChecklistItem& Item)
		{
			return Item.ObjectiveId == FName(TEXT("DeepWater.ReviewFloodLog"));
		});
	TestNotNull(TEXT("Tracker graph should show available optional investigations instead of hiding them."), OptionalInvestigation);
	if (OptionalInvestigation)
	{
		TestTrue(TEXT("Flood log should be marked as optional investigation context."), OptionalInvestigation->bOptional);
		TestFalse(TEXT("Optional investigation should not steal the active story objective slot."), OptionalInvestigation->bActive);
		TestFalse(TEXT("Available optional investigation should not look blocked."), OptionalInvestigation->bBlocked);
		TestTrue(TEXT("Available optional investigation should be labelled as optional context, not a world-exposed task."), OptionalInvestigation->StatusText.ToString().Contains(TEXT("可选")));
		TestFalse(TEXT("Available optional investigation should not claim it is directly executable from the world prompt."), OptionalInvestigation->StatusText.ToString().Contains(TEXT("可执行")));
		TestTrue(TEXT("Available optional investigation should not show a lock reason."), OptionalInvestigation->LockReason.IsEmpty());
		TestFalse(TEXT("Optional investigation tracker text should remain localized."), ContainsLatinLetter(OptionalInvestigation->StatusText.ToString() + OptionalInvestigation->LockReason.ToString()));
	}

	TestFalse(TEXT("Optional investigation should still not expose world prompts outside the current story objective."), GameMode->CanExposeCampaignObjective(DeepWaterChapter->ChapterId, TEXT("DeepWater.ReviewFloodLog")));
	return true;
}

bool FHorrorCampaignGameModeExposesOnlyCurrentStoryObjectiveTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* DeepWaterChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.DeepWaterStationFinale"));
	TestNotNull(TEXT("Deep Water chapter should exist for story visibility coverage."), DeepWaterChapter);
	if (!DeepWaterChapter)
	{
		return false;
	}

	AHorrorGameModeBase* GameMode = NewObject<AHorrorGameModeBase>();
	TestNotNull(TEXT("Story visibility test should create a campaign game mode."), GameMode);
	if (!GameMode)
	{
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(DeepWaterChapter->ChapterId);
	TestTrue(
		TEXT("The current required story objective should be visible to world actors."),
		GameMode->CanExposeCampaignObjective(DeepWaterChapter->ChapterId, TEXT("DeepWater.BootDryDock")));
	TestTrue(
		TEXT("Optional investigations should remain completable for reward/debug flows."),
		GameMode->CanCompleteCampaignObjective(DeepWaterChapter->ChapterId, TEXT("DeepWater.ReviewFloodLog")));
	TestFalse(
		TEXT("Optional investigations should not expose world prompts outside the current story objective."),
		GameMode->CanExposeCampaignObjective(DeepWaterChapter->ChapterId, TEXT("DeepWater.ReviewFloodLog")));
	TestFalse(
		TEXT("Locked future objectives should not expose world prompts."),
		GameMode->CanExposeCampaignObjective(DeepWaterChapter->ChapterId, TEXT("DeepWater.DecodeHeartbeat")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignGameModeExposesParallelRequiredMainlineObjectivesTest,
	"HorrorProject.Game.Campaign.GameModeExposesParallelRequiredMainlineObjectives",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignGameModeExposesParallelRequiredMainlineObjectivesTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* ForestChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.ForestOfSpikes"));
	TestNotNull(TEXT("Forest chapter should exist for parallel mainline visibility coverage."), ForestChapter);
	if (!ForestChapter)
	{
		return false;
	}

	AHorrorGameModeBase* GameMode = NewObject<AHorrorGameModeBase>();
	TestNotNull(TEXT("Parallel mainline visibility test should create a campaign game mode."), GameMode);
	if (!GameMode)
	{
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(ForestChapter->ChapterId);
	TestTrue(
		TEXT("The first dependency-free mainline objective should expose its world prompt."),
		GameMode->CanExposeCampaignObjective(ForestChapter->ChapterId, TEXT("Forest.ReadRootGlyph")));
	TestTrue(
		TEXT("Parallel dependency-free mainline objectives should also expose world prompts so players can recover from blocked routes."),
		GameMode->CanExposeCampaignObjective(ForestChapter->ChapterId, TEXT("Forest.AlignSpikeBeacon")));
	TestFalse(
		TEXT("Optional investigations should stay tracker-only unless promoted by story flow."),
		GameMode->CanExposeCampaignObjective(ForestChapter->ChapterId, TEXT("Forest.RecordHangingTape")));

	return true;
}

bool FHorrorCampaignObjectiveGraphExposesParallelAndLockedObjectivesTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* ForestChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.ForestOfSpikes"));
	TestNotNull(TEXT("Forest chapter should exist for objective graph coverage."), ForestChapter);
	if (!ForestChapter)
	{
		return false;
	}

	FHorrorCampaignProgress Progress;
	Progress.ResetForChapter(*ForestChapter);

	TArray<const FHorrorCampaignObjectiveDefinition*> AvailableObjectives = Progress.GetAvailableObjectives();
	TestTrue(TEXT("Objective graph should expose more than one available forest objective at chapter start."), AvailableObjectives.Num() >= 3);
	TestTrue(TEXT("Root glyph should be available at chapter start."), AvailableObjectives.ContainsByPredicate(
		[](const FHorrorCampaignObjectiveDefinition* Objective)
		{
			return Objective && Objective->ObjectiveId == FName(TEXT("Forest.ReadRootGlyph"));
		}));
	TestTrue(TEXT("Spike beacon should be available in parallel at chapter start."), AvailableObjectives.ContainsByPredicate(
		[](const FHorrorCampaignObjectiveDefinition* Objective)
		{
			return Objective && Objective->ObjectiveId == FName(TEXT("Forest.AlignSpikeBeacon"));
		}));
	TestTrue(TEXT("Optional hanging tape should be available without blocking mainline progress."), AvailableObjectives.ContainsByPredicate(
		[](const FHorrorCampaignObjectiveDefinition* Objective)
		{
			return Objective && Objective->ObjectiveId == FName(TEXT("Forest.RecordHangingTape")) && Objective->bOptional;
		}));

	TArray<FHorrorCampaignObjectiveGraphNode> Graph = Progress.BuildObjectiveGraph();
	TestEqual(TEXT("Objective graph should include every authored objective in the chapter."), Graph.Num(), ForestChapter->Objectives.Num());
	const FHorrorCampaignObjectiveGraphNode* LockedSeedNode = Graph.FindByPredicate(
		[](const FHorrorCampaignObjectiveGraphNode& Node)
		{
			return Node.ObjectiveId == FName(TEXT("Forest.ExtractBlackSeed"));
		});
	TestNotNull(TEXT("Objective graph should include the locked black seed convergence objective."), LockedSeedNode);
	if (LockedSeedNode)
	{
		TestEqual(TEXT("Black seed objective should start locked."), LockedSeedNode->Status, EHorrorCampaignObjectiveGraphStatus::Locked);
		TestTrue(TEXT("Black seed objective should explain the missing glyph prerequisite."), LockedSeedNode->MissingPrerequisiteObjectiveIds.Contains(FName(TEXT("Forest.ReadRootGlyph"))));
		TestTrue(TEXT("Black seed objective should explain the missing pursuit prerequisite."), LockedSeedNode->MissingPrerequisiteObjectiveIds.Contains(FName(TEXT("Forest.HoldSpikeCircle"))));
		TestTrue(TEXT("Black seed objective should be tagged as mainline."), LockedSeedNode->bMainline);
	}

	TestTrue(TEXT("Completing the beacon should unlock the pursuit objective."), Progress.TryCompleteObjective(TEXT("Forest.AlignSpikeBeacon")));
	Graph = Progress.BuildObjectiveGraph();
	const FHorrorCampaignObjectiveGraphNode* PursuitNode = Graph.FindByPredicate(
		[](const FHorrorCampaignObjectiveGraphNode& Node)
		{
			return Node.ObjectiveId == FName(TEXT("Forest.HoldSpikeCircle"));
		});
	TestNotNull(TEXT("Objective graph should include the pursuit node after beacon completion."), PursuitNode);
	if (PursuitNode)
	{
		TestEqual(TEXT("Pursuit objective should become available after beacon completion."), PursuitNode->Status, EHorrorCampaignObjectiveGraphStatus::Available);
		TestTrue(TEXT("Pursuit objective should have no missing prerequisites once beacon is complete."), PursuitNode->MissingPrerequisiteObjectiveIds.IsEmpty());
	}

	TestTrue(TEXT("Completing the root glyph should preserve convergence locking until pursuit is done."), Progress.TryCompleteObjective(TEXT("Forest.ReadRootGlyph")));
	Graph = Progress.BuildObjectiveGraph();
	LockedSeedNode = Graph.FindByPredicate(
		[](const FHorrorCampaignObjectiveGraphNode& Node)
		{
			return Node.ObjectiveId == FName(TEXT("Forest.ExtractBlackSeed"));
		});
	TestNotNull(TEXT("Objective graph should keep the black seed node visible while still locked."), LockedSeedNode);
	if (LockedSeedNode)
	{
		TestEqual(TEXT("Black seed objective should remain locked until the pursuit objective is complete."), LockedSeedNode->Status, EHorrorCampaignObjectiveGraphStatus::Locked);
		TestFalse(TEXT("Completed glyph prerequisite should be removed from the missing list."), LockedSeedNode->MissingPrerequisiteObjectiveIds.Contains(FName(TEXT("Forest.ReadRootGlyph"))));
		TestTrue(TEXT("Remaining pursuit prerequisite should still be named."), LockedSeedNode->MissingPrerequisiteObjectiveIds.Contains(FName(TEXT("Forest.HoldSpikeCircle"))));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignDefinitionsValidateAsPlayableGraphTest,
	"HorrorProject.Game.Campaign.DefinitionsValidateAsPlayableGraph",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignValidationRejectsRequiredInstantMainlineObjectivesTest,
	"HorrorProject.Game.Campaign.ValidationRejectsRequiredInstantMainlineObjectives",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignValidationRejectsEnglishPresentationTextTest,
	"HorrorProject.Game.Campaign.ValidationRejectsEnglishPresentationText",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignValidationRejectsUnplayableObjectiveLocationsTest,
	"HorrorProject.Game.Campaign.ValidationRejectsUnplayableObjectiveLocations",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignDefinitionsValidateAsPlayableGraphTest::RunTest(const FString& Parameters)
{
	const TArray<FHorrorCampaignValidationIssue> Issues = FHorrorCampaign::ValidateCampaignChapters();
	for (const FHorrorCampaignValidationIssue& Issue : Issues)
	{
		AddError(FString::Printf(
			TEXT("[%s] %s / %s: %s"),
			*Issue.IssueCode.ToString(),
			*Issue.ChapterId.ToString(),
			*Issue.ObjectiveId.ToString(),
			*Issue.Message.ToString()));
	}

	TestEqual(TEXT("Campaign definitions should validate as a playable objective graph."), Issues.Num(), 0);
	return Issues.IsEmpty();
}

bool FHorrorCampaignValidationRejectsRequiredInstantMainlineObjectivesTest::RunTest(const FString& Parameters)
{
	FHorrorCampaignChapterDefinition Chapter;
	Chapter.ChapterId = TEXT("Chapter.InstantRegression");
	Chapter.Title = FText::FromString(TEXT("即时互动回归测试"));
	Chapter.StoryBrief = FText::FromString(TEXT("验证必做主线不能退回一键互动。"));
	Chapter.CompletionBridgeText = FText::FromString(TEXT("验证结束。"));

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("InstantRegression.RequiredRelic");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::RecoverRelic;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::Instant;
	Objective.PromptText = FText::FromString(TEXT("一键取回关键遗物"));
	Objective.CompletionText = FText::FromString(TEXT("关键遗物已取回。"));
	Objective.bRequiredForChapterCompletion = true;
	Objective.Presentation.bOpensInteractionPanel = false;
	Chapter.Objectives.Add(Objective);

	const TArray<FHorrorCampaignValidationIssue> Issues = FHorrorCampaign::ValidateChapterDefinitionForTests(Chapter);
	TestTrue(TEXT("Validation should reject required mainline objectives that regress to one-click instant interaction."), Issues.ContainsByPredicate(
		[](const FHorrorCampaignValidationIssue& Issue)
		{
			return Issue.IssueCode == FName(TEXT("Campaign.Objective.RequiredInstantMainline"));
		}));

	return true;
}

bool FHorrorCampaignValidationRejectsEnglishPresentationTextTest::RunTest(const FString& Parameters)
{
	FHorrorCampaignChapterDefinition Chapter;
	Chapter.ChapterId = TEXT("Chapter.EnglishPresentationRegression");
	Chapter.Title = FText::FromString(TEXT("英文界面回归测试"));
	Chapter.StoryBrief = FText::FromString(TEXT("验证任务窗口不会混入英文。"));
	Chapter.CompletionBridgeText = FText::FromString(TEXT("验证结束。"));

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("EnglishPresentation.RequiredPanel");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::RestorePower;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::CircuitWiring;
	Objective.PromptText = FText::FromString(TEXT("修复测试电路"));
	Objective.CompletionText = FText::FromString(TEXT("测试电路已修复。"));
	Objective.bRequiredForChapterCompletion = true;
	Objective.Presentation.bOpensInteractionPanel = true;
	Objective.Presentation.bUsesFocusedInteraction = true;
	Objective.Presentation.MechanicLabel = FText::FromString(TEXT("Debug Panel"));
	Objective.Presentation.InputHint = FText::FromString(TEXT("Press E to repair"));
	Objective.Presentation.MissionContext = FText::FromString(TEXT("Debug mission"));
	Objective.Presentation.FailureStakes = FText::FromString(TEXT("Failure state"));
	FHorrorCampaignObjectiveBeat PanelBeat(
		FText::FromString(TEXT("接线同步")),
		FText::FromString(TEXT("在窗口内完成接线。")));
	PanelBeat.BeatId = TEXT("接线同步");
	PanelBeat.CompletionRule = EHorrorCampaignObjectiveBeatCompletionRule::AdvancedWindow;
	PanelBeat.FailurePolicy = EHorrorCampaignObjectiveBeatFailurePolicy::RetryBeat;
	PanelBeat.NavigationRole = EHorrorCampaignObjectiveBeatNavigationRole::ObjectiveActor;
	PanelBeat.bOpensInteractionPanel = true;
	Objective.ObjectiveBeats.Add(PanelBeat);
	Chapter.Objectives.Add(Objective);

	const TArray<FHorrorCampaignValidationIssue> Issues = FHorrorCampaign::ValidateChapterDefinitionForTests(Chapter);
	TestTrue(TEXT("Validation should reject English text in player-facing campaign presentation metadata."), Issues.ContainsByPredicate(
		[](const FHorrorCampaignValidationIssue& Issue)
		{
			return Issue.IssueCode == FName(TEXT("Campaign.Text.NonLocalized"));
		}));

	return true;
}

bool FHorrorCampaignValidationRejectsUnplayableObjectiveLocationsTest::RunTest(const FString& Parameters)
{
	FHorrorCampaignChapterDefinition Chapter;
	Chapter.ChapterId = TEXT("Chapter.LocationRegression");
	Chapter.Title = FText::FromString(TEXT("坐标回归测试"));
	Chapter.StoryBrief = FText::FromString(TEXT("验证任务不会被配置到地图外。"));
	Chapter.CompletionBridgeText = FText::FromString(TEXT("验证结束。"));

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("LocationRegression.OutsideMap");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::AcquireSignal;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::Instant;
	Objective.PromptText = FText::FromString(TEXT("检查异常坐标"));
	Objective.CompletionText = FText::FromString(TEXT("异常坐标已检查。"));
	Objective.RelativeLocation = FVector(50000.0f, -42000.0f, 9000.0f);
	Objective.bRequiredForChapterCompletion = false;
	Objective.bOptional = true;
	Chapter.Objectives.Add(Objective);

	const TArray<FHorrorCampaignValidationIssue> Issues = FHorrorCampaign::ValidateChapterDefinitionForTests(Chapter);
	TestTrue(TEXT("Validation should reject objectives authored outside the playable campaign bounds."), Issues.ContainsByPredicate(
		[](const FHorrorCampaignValidationIssue& Issue)
		{
			return Issue.IssueCode == FName(TEXT("Campaign.Objective.LocationOutOfBounds"));
		}));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignTrackerUsesRuntimeCompositeBeatStateTest,
	"HorrorProject.Game.Campaign.TrackerUsesRuntimeCompositeBeatState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignTrackerUsesRuntimeCompositeBeatStateTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* ForestChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.ForestOfSpikes"));
	TestNotNull(TEXT("Forest chapter should exist for runtime composite tracker coverage."), ForestChapter);
	if (!ForestChapter || ForestChapter->Objectives.IsEmpty())
	{
		return false;
	}

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for runtime composite tracker coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the campaign game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Runtime composite tracker test should expose the game mode."), GameMode);
	TestNotNull(TEXT("Runtime composite tracker test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Runtime composite tracker test should spawn an objective actor."), ObjectiveActor);
	if (!GameMode || !InstigatorActor || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* RootObjective = FHorrorCampaign::FindObjectiveById(
		*ForestChapter,
		TEXT("Forest.AlignSpikeBeacon"));
	TestNotNull(TEXT("Forest chapter should expose a multi-step beacon objective for runtime tracker coverage."), RootObjective);
	if (!RootObjective)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(ForestChapter->ChapterId);
	TestTrue(
		TEXT("Runtime composite tracker setup should complete the earlier forest scan objective first."),
		GameMode->TryCompleteCampaignObjective(ForestChapter->ChapterId, TEXT("Forest.ReadRootGlyph"), InstigatorActor));
	ObjectiveActor->ConfigureObjective(ForestChapter->ChapterId, *RootObjective);
	GameMode->ResetRuntimeCampaignObjectiveViewsForTests();
	GameMode->AddRuntimeCampaignObjectiveForTests(ObjectiveActor);

	FHorrorObjectiveTrackerSnapshot Tracker = GameMode->BuildObjectiveTrackerSnapshot();
	TestTrue(TEXT("Runtime tracker should expose one checklist row per current objective beat."), Tracker.ChecklistItems.Num() >= RootObjective->ObjectiveBeats.Num());
	TestEqual(TEXT("Initial tracker beat should carry the authored beat id."), Tracker.ChecklistItems[0].BeatId, RootObjective->ObjectiveBeats[0].BeatId);
	TestEqual(TEXT("Initial tracker beat should carry the beat index."), Tracker.ChecklistItems[0].BeatIndex, 0);
	TestEqual(TEXT("Initial tracker beat runtime status should be available."), Tracker.ChecklistItems[0].RuntimeStatus, EHorrorCampaignObjectiveRuntimeStatus::Available);
	TestTrue(TEXT("Initial tracker beat should be active."), Tracker.ChecklistItems[0].bActive);
	TestFalse(TEXT("Initial tracker beat should not be complete."), Tracker.ChecklistItems[0].bComplete);

	const FHitResult Hit;
	TestTrue(TEXT("First interaction should open the beacon tuning panel."), ObjectiveActor->Interact_Implementation(InstigatorActor, Hit));
	TestTrue(TEXT("Beacon objective should now be running its advanced interaction window."), ObjectiveActor->IsAdvancedInteractionActive());
	Tracker = GameMode->BuildObjectiveTrackerSnapshot();

	TestEqual(TEXT("Runtime tracker should keep the first beat active while the tuning panel is open."), Tracker.ChecklistItems[0].bActive, true);
	TestFalse(TEXT("Runtime tracker should not complete the first beat until tuning succeeds."), Tracker.ChecklistItems[0].bComplete);
	TestFalse(TEXT("Runtime tracker should not activate the second beat while the tuning panel is unresolved."), Tracker.ChecklistItems[1].bActive);
	TestFalse(TEXT("Runtime tracker should not mark the second beat complete yet."), Tracker.ChecklistItems[1].bComplete);
	TestEqual(TEXT("Active beat runtime status should report advanced interaction."), Tracker.ChecklistItems[0].RuntimeStatus, EHorrorCampaignObjectiveRuntimeStatus::AdvancedInteractionActive);
	TestTrue(TEXT("Active beat should advertise that it opens an interaction panel."), Tracker.ChecklistItems[0].bOpensInteractionPanel);
	TestEqual(TEXT("Active beat should expose the signal tuning mode."), Tracker.ChecklistItems[0].InteractionMode, EHorrorCampaignInteractionMode::SignalTuning);
	TestFalse(TEXT("Active beat should carry device diagnostics while tuning is open."), Tracker.ChecklistItems[0].DeviceStatusLabel.IsEmpty());
	TestFalse(TEXT("Active beat should carry the next tuning action while tuning is open."), Tracker.ChecklistItems[0].NextActionLabel.IsEmpty());
	TestTrue(TEXT("Active beat progress should remain at the start until the player solves the window."), Tracker.ChecklistItems[0].RuntimeProgressFraction <= KINDA_SMALL_NUMBER);
	TestEqual(TEXT("Second beat should carry the authored beat id."), Tracker.ChecklistItems[1].BeatId, RootObjective->ObjectiveBeats[1].BeatId);
	TestEqual(TEXT("Second beat should carry the beat index."), Tracker.ChecklistItems[1].BeatIndex, 1);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignAdvancedInteractionFailurePublishesRetryLoopTest,
	"HorrorProject.Game.Campaign.AdvancedInteractionFailurePublishesRetryLoop",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignAdvancedInteractionFailurePublishesRetryLoopTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for advanced interaction retry-loop coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	TestNotNull(TEXT("Advanced interaction failure should publish through the event bus."), EventBus);
	if (!EventBus)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Retry-loop test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Retry-loop test should spawn an objective actor."), ObjectiveActor);
	if (!InstigatorActor || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.RetryCircuit");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::RestorePower;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::CircuitWiring;
	Objective.PromptText = FText::FromString(TEXT("修复会过载的测试电路"));
	Objective.CompletionText = FText::FromString(TEXT("测试电路恢复。"));

	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);
	const FHitResult Hit;
	TestTrue(TEXT("Starting circuit retry-loop objective should open the advanced panel."), ObjectiveActor->Interact_Implementation(InstigatorActor, Hit));
	TestTrue(TEXT("Retry-loop objective should start active."), ObjectiveActor->IsAdvancedInteractionActive());

	for (int32 FailureIndex = 0; FailureIndex < 6 && ObjectiveActor->IsAdvancedInteractionActive(); ++FailureIndex)
	{
		TestTrue(
			FString::Printf(TEXT("Failure %d should be accepted as a retry-loop input."), FailureIndex + 1),
			ObjectiveActor->SubmitAdvancedInteractionInput(TEXT("错误端子"), InstigatorActor));
	}

	TestFalse(TEXT("Stability exhaustion should close the advanced interaction panel."), ObjectiveActor->IsAdvancedInteractionActive());
	TestFalse(TEXT("Stability exhaustion should not complete the objective."), ObjectiveActor->IsCompleted());
	TestTrue(TEXT("Failed advanced objectives should remain available for retry."), ObjectiveActor->IsAvailableForInteraction());
	TestEqual(TEXT("Failed advanced objectives should reset interaction progress for a clean retry."), ObjectiveActor->GetInteractionProgressCount(), 0);
	TestEqual(TEXT("Failed advanced objectives should reset panel progress for a clean retry."), ObjectiveActor->GetAdvancedInteractionProgressFraction(), 0.0f);
	TestTrue(TEXT("Retry prompt should explain the overload in Chinese."), ObjectiveActor->GetInteractionPromptText().ToString().Contains(TEXT("过载")));
	FHorrorCampaignObjectiveRuntimeState RuntimeState = ObjectiveActor->BuildObjectiveRuntimeState();
	TestEqual(TEXT("Failed advanced runtime state should be explicitly retryable."), RuntimeState.Status, EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable);
	TestTrue(TEXT("Failed advanced runtime state should carry retry affordance."), RuntimeState.bRetryable);
	TestEqual(TEXT("Failed advanced runtime state should preserve the overload cause."), RuntimeState.FailureCause, FName(TEXT("Failure.Campaign.CircuitOverload")));
	TestTrue(TEXT("Failed advanced runtime state should keep a recovery HUD summary."), RuntimeState.AdvancedInteraction.bVisible);
	TestFalse(TEXT("Failed advanced runtime state should explain the overloaded device."), RuntimeState.AdvancedInteraction.DeviceStatusLabel.IsEmpty());
	TestFalse(TEXT("Failed advanced runtime state should expose retry guidance."), RuntimeState.AdvancedInteraction.NextActionLabel.IsEmpty());
	TestFalse(TEXT("Failed advanced runtime state should expose recovery guidance."), RuntimeState.AdvancedInteraction.FailureRecoveryLabel.IsEmpty());
	TestTrue(TEXT("Failed advanced runtime state should report zero performance after overload."), RuntimeState.AdvancedInteraction.PerformanceGradeFraction <= KINDA_SMALL_NUMBER);

	const FGameplayTag ObjectiveFailedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.ObjectiveFailed")), false);
	const FHorrorEventMessage* FailureMessage = EventBus->GetHistory().FindByPredicate(
		[ObjectiveFailedTag, &Objective](const FHorrorEventMessage& Message)
		{
			return Message.EventTag == ObjectiveFailedTag && Message.SourceId == Objective.ObjectiveId;
		});
	TestNotNull(TEXT("Stability exhaustion should publish a structured objective-failed event."), FailureMessage);
	if (FailureMessage)
	{
		TestFalse(TEXT("Objective failure event should include a localized toast title."), FailureMessage->DebugLabel.IsEmpty());
		TestFalse(TEXT("Objective failure event should include a retry hint."), FailureMessage->ObjectiveHint.IsEmpty());
		TestEqual(TEXT("Objective failure event should expose failure severity."), FailureMessage->FeedbackSeverity, EHorrorObjectiveFeedbackSeverity::Failure);
		TestTrue(TEXT("Objective failure event should explicitly mark retryable objectives."), FailureMessage->bRetryable);
		TestTrue(TEXT("Objective failure event should request a longer readable toast."), FailureMessage->DisplaySeconds >= 6.0f);
		TestEqual(TEXT("Objective failure event should expose circuit overload cause."), FailureMessage->FailureCause, FName(TEXT("Failure.Campaign.CircuitOverload")));
		TestEqual(TEXT("Objective failure event should expose retry recovery action."), FailureMessage->RecoveryAction, FName(TEXT("Recovery.Objective.Retry")));
		TestTrue(TEXT("Objective failure event should expose a positive attempt index."), FailureMessage->AttemptIndex >= 1);
		TestTrue(TEXT("Objective failure title should describe failure or overload."), FailureMessage->DebugLabel.ToString().Contains(TEXT("失败")) || FailureMessage->DebugLabel.ToString().Contains(TEXT("过载")));
		TestTrue(TEXT("Objective failure hint should tell the player to retry."), FailureMessage->ObjectiveHint.ToString().Contains(TEXT("重试")) || FailureMessage->ObjectiveHint.ToString().Contains(TEXT("重新")));
		TestFalse(TEXT("Objective failure hint should not expose English text."), ContainsLatinLetter(FailureMessage->ObjectiveHint.ToString()));
	}

	TestTrue(TEXT("Retrying the failed circuit should reopen the advanced panel."), ObjectiveActor->Interact_Implementation(InstigatorActor, Hit));
	TestTrue(TEXT("Advanced panel should be active after retry."), ObjectiveActor->IsAdvancedInteractionActive());
	TestEqual(TEXT("Retry should restore full device stability."), ObjectiveActor->BuildAdvancedInteractionHUDState().StabilityFraction, 1.0f);
	RuntimeState = ObjectiveActor->BuildObjectiveRuntimeState();
	TestEqual(TEXT("Retry runtime state should return to the advanced interaction state."), RuntimeState.Status, EHorrorCampaignObjectiveRuntimeStatus::AdvancedInteractionActive);
	TestFalse(TEXT("Retry runtime state should clear retry affordance while active."), RuntimeState.bRetryable);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveCompletionPublishesStructuredFeedbackTest,
	"HorrorProject.Game.Campaign.ObjectiveCompletionPublishesStructuredFeedback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveCompletionPublishesStructuredFeedbackTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for campaign feedback coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the campaign game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	TestNotNull(TEXT("Campaign feedback test should expose the campaign game mode."), GameMode);
	TestNotNull(TEXT("Campaign feedback test should expose the event bus."), EventBus);
	if (!GameMode || !EventBus)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FHorrorCampaignChapterDefinition* ForestChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.ForestOfSpikes"));
	TestNotNull(TEXT("Forest chapter should exist for campaign feedback coverage."), ForestChapter);
	if (!ForestChapter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* RootGlyphObjective = FHorrorCampaign::FindObjectiveById(
		*ForestChapter,
		TEXT("Forest.ReadRootGlyph"));
	TestNotNull(TEXT("Campaign feedback test should find the root glyph objective."), RootGlyphObjective);
	if (!RootGlyphObjective)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(ForestChapter->ChapterId);
	TestTrue(
		TEXT("Completing a campaign objective should succeed for feedback coverage."),
		GameMode->TryCompleteCampaignObjective(ForestChapter->ChapterId, RootGlyphObjective->ObjectiveId, nullptr));

	const FGameplayTag ObjectiveCompletedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.ObjectiveCompleted")), false);
	const FHorrorEventMessage* ObjectiveMessage = EventBus->GetHistory().FindByPredicate(
		[ObjectiveCompletedTag, RootGlyphObjective](const FHorrorEventMessage& Message)
		{
			return Message.EventTag == ObjectiveCompletedTag && Message.SourceId == RootGlyphObjective->ObjectiveId;
		});

	TestNotNull(TEXT("Campaign objective completion should publish an objective-completed event."), ObjectiveMessage);
	if (ObjectiveMessage)
	{
		TestFalse(TEXT("Campaign completion event should include a localized toast title."), ObjectiveMessage->DebugLabel.IsEmpty());
		TestFalse(TEXT("Campaign completion event should include a localized next-step hint."), ObjectiveMessage->ObjectiveHint.IsEmpty());
		TestEqual(TEXT("Campaign completion event should expose success severity."), ObjectiveMessage->FeedbackSeverity, EHorrorObjectiveFeedbackSeverity::Success);
		TestFalse(TEXT("Campaign completion event should not be marked retryable."), ObjectiveMessage->bRetryable);
		TestTrue(TEXT("Campaign completion event should use a readable toast duration."), ObjectiveMessage->DisplaySeconds >= 4.0f);
		TestTrue(TEXT("Campaign completion event title should describe completion."), ObjectiveMessage->DebugLabel.ToString().Contains(TEXT("完成")));
		TestFalse(TEXT("Campaign completion event hint should not expose English text."), ContainsLatinLetter(ObjectiveMessage->ObjectiveHint.ToString()));
	}

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveCompletionAppliesPlayerRewardsTest,
	"HorrorProject.Game.Campaign.ObjectiveCompletionAppliesPlayerRewards",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveCompletionAppliesPlayerRewardsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for campaign reward coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the campaign game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	TestNotNull(TEXT("Campaign reward test should expose the campaign game mode."), GameMode);
	TestNotNull(TEXT("Campaign reward test should expose the event bus."), EventBus);
	if (!GameMode || !EventBus)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FHorrorCampaignChapterDefinition* DeepWaterChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.DeepWaterStationFinale"));
	TestNotNull(TEXT("Deep Water chapter should exist for reward coverage."), DeepWaterChapter);
	if (!DeepWaterChapter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* FloodLogObjective = FHorrorCampaign::FindObjectiveById(
		*DeepWaterChapter,
		TEXT("DeepWater.ReviewFloodLog"));
	TestNotNull(TEXT("Campaign reward test should find the optional flood log objective."), FloodLogObjective);
	if (!FloodLogObjective)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Optional investigation should define a tangible reward package."), FloodLogObjective->Reward.HasAnyReward());
	TestFalse(TEXT("Optional investigation reward text should be localized."), FloodLogObjective->Reward.RewardText.IsEmpty());
	TestFalse(TEXT("Optional investigation should grant evidence."), FloodLogObjective->Reward.EvidenceId.IsNone());
	TestFalse(TEXT("Reward text should not expose English text."), ContainsLatinLetter(FloodLogObjective->Reward.RewardText.ToString()));

	AHorrorPlayerCharacter* PlayerCharacter = SpawnControlledCampaignPlayer(*this, World, FVector::ZeroVector);
	TestNotNull(TEXT("Campaign reward test should spawn a player character."), PlayerCharacter);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UInventoryComponent* Inventory = PlayerCharacter->GetInventoryComponent();
	UFearComponent* Fear = PlayerCharacter->GetFearComponent();
	UCameraBatteryComponent* Battery = PlayerCharacter->GetQuantumCameraComponent()
		? PlayerCharacter->GetQuantumCameraComponent()->GetBatteryComponent()
		: nullptr;
	TestNotNull(TEXT("Campaign reward test should expose inventory."), Inventory);
	TestNotNull(TEXT("Campaign reward test should expose fear."), Fear);
	TestNotNull(TEXT("Campaign reward test should expose camera battery."), Battery);
	if (!Inventory || !Fear || !Battery)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Fear->SetFearValue(45.0f);
	Battery->SetBatteryPercentage(35.0f);

	GameMode->ResetCampaignProgressForChapterForTests(DeepWaterChapter->ChapterId);
	TestTrue(
		TEXT("Completing an optional investigation should succeed for reward coverage."),
		GameMode->TryCompleteCampaignObjective(DeepWaterChapter->ChapterId, FloodLogObjective->ObjectiveId, PlayerCharacter));

	TestTrue(TEXT("Objective reward should add its evidence id to the player archive."), Inventory->HasCollectedEvidenceId(FloodLogObjective->Reward.EvidenceId));
	TestTrue(TEXT("Objective reward should reduce fear when configured."), Fear->GetFearValue() < 45.0f);
	TestTrue(TEXT("Objective reward should recharge the camera battery when configured."), Battery->GetBatteryPercentage() > 35.0f);

	FHorrorEvidenceMetadata RewardMetadata;
	TestTrue(TEXT("Objective reward evidence should register archive metadata."), Inventory->GetEvidenceMetadata(FloodLogObjective->Reward.EvidenceId, RewardMetadata));
	TestFalse(TEXT("Reward evidence metadata should expose a readable display name."), RewardMetadata.DisplayName.IsEmpty());
	TestFalse(TEXT("Reward evidence metadata should expose a readable description."), RewardMetadata.Description.IsEmpty());

	const FGameplayTag ObjectiveCompletedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.ObjectiveCompleted")), false);
	const FHorrorEventMessage* ObjectiveMessage = EventBus->GetHistory().FindByPredicate(
		[ObjectiveCompletedTag, FloodLogObjective](const FHorrorEventMessage& Message)
		{
			return Message.EventTag == ObjectiveCompletedTag && Message.SourceId == FloodLogObjective->ObjectiveId;
		});

	TestNotNull(TEXT("Campaign reward completion should publish an objective-completed event."), ObjectiveMessage);
	if (ObjectiveMessage)
	{
		const FString RewardHint = ObjectiveMessage->ObjectiveHint.ToString();
		TestTrue(TEXT("Campaign reward event hint should mention the reward."), RewardHint.Contains(TEXT("奖励")));
		TestTrue(TEXT("Campaign reward event hint should include reward text."), RewardHint.Contains(FloodLogObjective->Reward.RewardText.ToString()));
		TestFalse(TEXT("Campaign reward event hint should not expose English text."), ContainsLatinLetter(RewardHint));
	}

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignAdvancedInteractionPublishesInputOptionsToHUDTest,
	"HorrorProject.Game.Campaign.AdvancedInteractionPublishesInputOptionsToHUD",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignAdvancedInteractionPublishesInputOptionsToHUDTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for advanced input option coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* CircuitObjective = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	AHorrorCampaignObjectiveActor* GearObjective = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	AHorrorCampaignObjectiveActor* SpectralObjective = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	AHorrorCampaignObjectiveActor* SignalObjective = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Advanced input option test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Advanced input option test should spawn a circuit objective actor."), CircuitObjective);
	TestNotNull(TEXT("Advanced input option test should spawn a gear objective actor."), GearObjective);
	TestNotNull(TEXT("Advanced input option test should spawn a spectral scan objective actor."), SpectralObjective);
	TestNotNull(TEXT("Advanced input option test should spawn a signal tuning objective actor."), SignalObjective);
	if (!InstigatorActor || !CircuitObjective || !GearObjective || !SpectralObjective || !SignalObjective)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition CircuitDefinition;
	CircuitDefinition.ObjectiveId = TEXT("Test.CircuitOptions");
	CircuitDefinition.ObjectiveType = EHorrorCampaignObjectiveType::RestorePower;
	CircuitDefinition.InteractionMode = EHorrorCampaignInteractionMode::CircuitWiring;
	CircuitDefinition.PromptText = FText::FromString(TEXT("接线选项测试"));
	CircuitObjective->ConfigureObjective(TEXT("Chapter.Test"), CircuitDefinition);

	FHorrorCampaignObjectiveDefinition GearDefinition;
	GearDefinition.ObjectiveId = TEXT("Test.GearOptions");
	GearDefinition.ObjectiveType = EHorrorCampaignObjectiveType::DisableSeal;
	GearDefinition.InteractionMode = EHorrorCampaignInteractionMode::GearCalibration;
	GearDefinition.PromptText = FText::FromString(TEXT("齿轮选项测试"));
	GearObjective->ConfigureObjective(TEXT("Chapter.Test"), GearDefinition);

	FHorrorCampaignObjectiveDefinition SpectralDefinition;
	SpectralDefinition.ObjectiveId = TEXT("Test.SpectralOptions");
	SpectralDefinition.ObjectiveType = EHorrorCampaignObjectiveType::ScanAnomaly;
	SpectralDefinition.InteractionMode = EHorrorCampaignInteractionMode::SpectralScan;
	SpectralDefinition.PromptText = FText::FromString(TEXT("频谱选项测试"));
	SpectralObjective->ConfigureObjective(TEXT("Chapter.Test"), SpectralDefinition);

	FHorrorCampaignObjectiveDefinition SignalDefinition;
	SignalDefinition.ObjectiveId = TEXT("Test.SignalOptions");
	SignalDefinition.ObjectiveType = EHorrorCampaignObjectiveType::AcquireSignal;
	SignalDefinition.InteractionMode = EHorrorCampaignInteractionMode::SignalTuning;
	SignalDefinition.PromptText = FText::FromString(TEXT("信号选项测试"));
	SignalObjective->ConfigureObjective(TEXT("Chapter.Test"), SignalDefinition);

	const FHitResult EmptyHit;
	TestTrue(TEXT("Circuit objective should open its advanced panel."), CircuitObjective->Interact_Implementation(InstigatorActor, EmptyHit));
	TestTrue(TEXT("Gear objective should open its advanced panel."), GearObjective->Interact_Implementation(InstigatorActor, EmptyHit));
	TestTrue(TEXT("Spectral scan objective should open its advanced panel."), SpectralObjective->Interact_Implementation(InstigatorActor, EmptyHit));
	TestTrue(TEXT("Signal tuning objective should open its advanced panel."), SignalObjective->Interact_Implementation(InstigatorActor, EmptyHit));
	SpectralObjective->Tick(0.5f);
	SignalObjective->SubmitAdvancedInteractionInput(TEXT("左声道"), InstigatorActor);

	const FHorrorAdvancedInteractionHUDState CircuitState = CircuitObjective->BuildAdvancedInteractionHUDState();
	TestEqual(TEXT("Circuit HUD should publish three selectable terminal options."), CircuitState.InputOptions.Num(), 3);
	TestEqual(TEXT("Circuit first option should submit the blue terminal."), CircuitState.InputOptions[0].InputId, FName(TEXT("蓝色端子")));
	TestTrue(TEXT("Circuit first option should expose the device key hint."), CircuitState.InputOptions[0].KeyHint.ToString().Contains(TEXT("A")));
	TestEqual(TEXT("Circuit first option should expose the connect action role."), CircuitState.InputOptions[0].ActionRole, EHorrorAdvancedInteractionInputRole::ConnectCircuit);
	TestTrue(TEXT("Circuit safe terminal should require the timing window."), CircuitState.InputOptions[0].bRequiresTimingWindow);
	TestTrue(TEXT("Circuit safe terminal should advance the objective when correct."), CircuitState.InputOptions[0].bAdvancesObjective);
	TestTrue(TEXT("Circuit safe terminal should describe a concrete connect verb."), CircuitState.InputOptions[0].ActionVerb.ToString().Contains(TEXT("接入")));
	TestEqual(TEXT("Circuit first option should expose a stable visual slot."), CircuitState.InputOptions[0].VisualSlotIndex, 0);
	TestTrue(TEXT("Circuit first option should expose an authored terminal color."), CircuitState.InputOptions[0].VisualColor.B > CircuitState.InputOptions[0].VisualColor.R);
	TestTrue(TEXT("Circuit hazardous option should carry high-load telemetry."), CircuitState.InputOptions[1].bHazardous && CircuitState.InputOptions[1].LoadFraction > 0.8f);
	TestEqual(TEXT("Circuit hazardous option should expose the avoid hazard role."), CircuitState.InputOptions[1].ActionRole, EHorrorAdvancedInteractionInputRole::AvoidHazard);
	TestFalse(TEXT("Circuit hazardous option should not present itself as objective progress."), CircuitState.InputOptions[1].bAdvancesObjective);
	TestTrue(TEXT("Circuit hazardous option should mark a non-timed avoidance action."), !CircuitState.InputOptions[1].bRequiresTimingWindow);
	TestTrue(TEXT("Circuit hazardous option should describe avoidance instead of connection."), CircuitState.InputOptions[1].ActionVerb.ToString().Contains(TEXT("避开")));
	TestTrue(TEXT("Circuit expected input should be one of the published options."), CircuitState.InputOptions.ContainsByPredicate(
		[&CircuitState](const FHorrorAdvancedInteractionInputOption& Option)
		{
			return Option.InputId == CircuitState.ExpectedInputId && !Option.DisplayLabel.IsEmpty() && !Option.KeyHint.IsEmpty();
		}));

	const FHorrorAdvancedInteractionHUDState GearState = GearObjective->BuildAdvancedInteractionHUDState();
	TestEqual(TEXT("Gear HUD should publish three selectable gear options."), GearState.InputOptions.Num(), 3);
	TestEqual(TEXT("Gear first option should submit the first gear."), GearState.InputOptions[0].InputId, FName(TEXT("齿轮1")));
	TestTrue(TEXT("Gear first option should expose the device key hint."), GearState.InputOptions[0].KeyHint.ToString().Contains(TEXT("A")));
	TestEqual(TEXT("Gear options should expose the calibrate action role."), GearState.InputOptions[0].ActionRole, EHorrorAdvancedInteractionInputRole::CalibrateGear);
	TestTrue(TEXT("Gear options should require the timing window."), GearState.InputOptions[0].bRequiresTimingWindow);
	TestTrue(TEXT("Gear options should describe a mechanical action verb."), GearState.InputOptions[0].ActionVerb.ToString().Contains(TEXT("拨动")));
	TestEqual(TEXT("Gear first option should expose a stable visual slot."), GearState.InputOptions[0].VisualSlotIndex, 0);
	TestTrue(TEXT("Gear first option should expose a warm metal visual color."), GearState.InputOptions[0].VisualColor.R >= GearState.InputOptions[0].VisualColor.B);
	TestTrue(TEXT("Gear expected input should be one of the published options."), GearState.InputOptions.ContainsByPredicate(
		[&GearState](const FHorrorAdvancedInteractionInputOption& Option)
		{
			return Option.InputId == GearState.ExpectedInputId && !Option.DisplayLabel.IsEmpty() && !Option.KeyHint.IsEmpty();
		}));

	const FHorrorAdvancedInteractionHUDState SpectralState = SpectralObjective->BuildAdvancedInteractionHUDState();
	TestEqual(TEXT("Spectral scan HUD should publish three selectable resonance bands."), SpectralState.InputOptions.Num(), 3);
	TestEqual(TEXT("Spectral scan first option should sweep the low band."), SpectralState.InputOptions[0].InputId, FName(TEXT("低频波段")));
	TestTrue(TEXT("Spectral scan options should expose device key hints."), SpectralState.InputOptions[0].KeyHint.ToString().Contains(TEXT("A")));
	TestEqual(TEXT("Spectral scan low option should expose the sweep-left action role."), SpectralState.InputOptions[0].ActionRole, EHorrorAdvancedInteractionInputRole::TuneSignalLeft);
	TestFalse(TEXT("Spectral scan sweep controls should not require the timing window."), SpectralState.InputOptions[0].bRequiresTimingWindow);
	TestFalse(TEXT("Spectral scan sweep controls should tune the filter without advancing the objective."), SpectralState.InputOptions[0].bAdvancesObjective);
	TestTrue(TEXT("Spectral scan sweep verb should mention sweeping."), SpectralState.InputOptions[0].ActionVerb.ToString().Contains(TEXT("扫")));
	TestEqual(TEXT("Spectral scan center option should submit the lock band."), SpectralState.InputOptions[1].InputId, FName(TEXT("中频波段")));
	TestEqual(TEXT("Spectral scan center option should expose the lock band action role."), SpectralState.InputOptions[1].ActionRole, EHorrorAdvancedInteractionInputRole::LockSpectralBand);
	TestTrue(TEXT("Spectral scan center lock should require timing and confidence."), SpectralState.InputOptions[1].bRequiresTimingWindow && SpectralState.InputOptions[1].bRequiresStableSignal);
	TestTrue(TEXT("Spectral scan center action verb should mention locking the band."), SpectralState.InputOptions[1].ActionVerb.ToString().Contains(TEXT("锁定")));
	TestTrue(TEXT("Spectral scan lock band should carry lower load than sweep noise bands when aligned."), SpectralState.InputOptions.ContainsByPredicate(
		[](const FHorrorAdvancedInteractionInputOption& Option)
		{
			return Option.ActionRole == EHorrorAdvancedInteractionInputRole::LockSpectralBand && Option.LoadFraction < 0.85f;
		}));
	TestTrue(TEXT("Spectral scan noise bands should carry visible load telemetry for animated noise UI."), SpectralState.InputOptions.ContainsByPredicate(
		[&SpectralState](const FHorrorAdvancedInteractionInputOption& Option)
		{
			return Option.ActionRole != EHorrorAdvancedInteractionInputRole::LockSpectralBand && Option.LoadFraction > 0.45f;
		}));

	const FHorrorAdvancedInteractionHUDState SignalState = SignalObjective->BuildAdvancedInteractionHUDState();
	TestEqual(TEXT("Signal tuning HUD should publish three playback channel options."), SignalState.InputOptions.Num(), 3);
	TestEqual(TEXT("Signal tuning left option should expose a left-nudge action role."), SignalState.InputOptions[0].ActionRole, EHorrorAdvancedInteractionInputRole::TuneSignalLeft);
	TestFalse(TEXT("Signal tuning left nudge should not require the timing window."), SignalState.InputOptions[0].bRequiresTimingWindow);
	TestFalse(TEXT("Signal tuning left nudge should not advance objective progress directly."), SignalState.InputOptions[0].bAdvancesObjective);
	TestEqual(TEXT("Signal tuning center option should submit the frequency lock."), SignalState.InputOptions[1].InputId, FName(TEXT("中心频率")));
	TestTrue(TEXT("Signal tuning options should expose device key hints."), SignalState.InputOptions[1].KeyHint.ToString().Contains(TEXT("S")));
	TestEqual(TEXT("Signal tuning center option should expose the confirm-lock action role."), SignalState.InputOptions[1].ActionRole, EHorrorAdvancedInteractionInputRole::LockSignalCenter);
	TestTrue(TEXT("Signal tuning center lock should require timing and stable signal."), SignalState.InputOptions[1].bRequiresTimingWindow && SignalState.InputOptions[1].bRequiresStableSignal);
	TestTrue(TEXT("Signal tuning center lock should advance the objective when aligned."), SignalState.InputOptions[1].bAdvancesObjective);
	TestEqual(TEXT("Signal tuning right option should expose a right-nudge action role."), SignalState.InputOptions[2].ActionRole, EHorrorAdvancedInteractionInputRole::TuneSignalRight);
	TestTrue(TEXT("Signal tuning center lock should become hazardous while balance is off-center."), SignalState.InputOptions[1].bHazardous);
	TestTrue(TEXT("Signal tuning center lock should expose high load when balance is off-center."), SignalState.InputOptions[1].LoadFraction > 0.7f);
	TestTrue(TEXT("Signal tuning side-channel nudge should expose lower load than unsafe center confirm."), SignalState.InputOptions[0].LoadFraction < SignalState.InputOptions[1].LoadFraction);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignAdvancedInteractionHUDStateIsNormalizedAtSourceTest,
	"HorrorProject.Game.Campaign.AdvancedInteractionHUDStateIsNormalizedAtSource",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignAdvancedInteractionHUDStateIsNormalizedAtSourceTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for advanced HUD source contract coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* SourceActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	AHorrorCampaignObjectiveActor* RestoredActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Advanced HUD source contract test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Advanced HUD source contract test should spawn a source objective actor."), SourceActor);
	TestNotNull(TEXT("Advanced HUD source contract test should spawn a restored objective actor."), RestoredActor);
	if (!InstigatorActor || !SourceActor || !RestoredActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.AdvancedHUDSourceContract");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::RestorePower;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::CircuitWiring;
	Objective.PromptText = FText::FromString(TEXT("源头契约测试电路"));
	Objective.AdvancedInteractionTuning.RequiredStepCount = 3;
	Objective.AdvancedInteractionTuning.SuccessProgress = 0.34f;

	SourceActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);
	RestoredActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

	const FHitResult Hit;
	TestTrue(TEXT("Source objective should open an advanced panel before export."), SourceActor->Interact_Implementation(InstigatorActor, Hit));
	FHorrorCampaignObjectiveSaveState SaveState = SourceActor->ExportObjectiveSaveState();
	SaveState.ExpectedAdvancedInputId = TEXT("损坏的旧存档端子");
	SaveState.AdvancedInteractionProgressFraction = 4.0f;
	SaveState.AdvancedInteractionTimingFraction = -2.0f;
	SaveState.AdvancedInteractionStabilityFraction = 9.0f;
	SaveState.AdvancedInteractionStepIndex = 99;
	SaveState.AdvancedInteractionComboCount = -7;
	SaveState.AdvancedInteractionMistakeCount = -3;

	RestoredActor->ImportObjectiveSaveState(SaveState, InstigatorActor);
	const FHorrorAdvancedInteractionHUDState RestoredState = RestoredActor->BuildAdvancedInteractionHUDState();
	TestTrue(TEXT("Restored advanced HUD state should remain visible after repairing source data."), RestoredState.bVisible);
	TestEqual(TEXT("Restored advanced HUD state should preserve the authored interaction mode."), RestoredState.Mode, EHorrorCampaignInteractionMode::CircuitWiring);
	TestEqual(TEXT("Restored advanced HUD state should keep the authored option count."), RestoredState.InputOptions.Num(), 3);
	TestTrue(TEXT("Restored advanced HUD state should expose a full step track."), RestoredState.StepTrack.Num() >= RestoredState.RequiredStepCount);
	TestTrue(TEXT("Restored advanced HUD progress should be clamped at the source."), RestoredState.ProgressFraction >= 0.0f && RestoredState.ProgressFraction <= 1.0f);
	TestTrue(TEXT("Restored advanced HUD timing should be clamped at the source."), RestoredState.TimingFraction >= 0.0f && RestoredState.TimingFraction <= 1.0f);
	TestTrue(TEXT("Restored advanced HUD stability should be clamped at the source."), RestoredState.StabilityFraction >= 0.0f && RestoredState.StabilityFraction <= 1.0f);
	TestTrue(TEXT("Restored advanced HUD step index should be normalized against the authored track."), RestoredState.StepIndex >= 0 && RestoredState.StepIndex < RestoredState.RequiredStepCount);
	TestEqual(TEXT("Restored advanced HUD combo count should be non-negative."), RestoredState.ComboCount, 0);
	TestEqual(TEXT("Restored advanced HUD mistake count should be non-negative."), RestoredState.MistakeCount, 0);
	TestTrue(TEXT("Restored advanced HUD expected input should be repaired to a published option."), RestoredState.InputOptions.ContainsByPredicate(
		[&RestoredState](const FHorrorAdvancedInteractionInputOption& Option)
		{
			return Option.InputId == RestoredState.ExpectedInputId;
		}));
	TestTrue(TEXT("Restored advanced HUD active slot should be repaired to a published option slot."), RestoredState.InputOptions.ContainsByPredicate(
		[&RestoredState](const FHorrorAdvancedInteractionInputOption& Option)
		{
			return Option.VisualSlotIndex == RestoredState.ActiveInputSlotIndex;
		}));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveRuntimeStateSummarizesAdvancedInteractionTest,
	"HorrorProject.Game.Campaign.ObjectiveRuntimeStateSummarizesAdvancedInteraction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveRuntimeStateSummarizesAdvancedInteractionTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for objective runtime state coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Runtime state test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Runtime state test should spawn an objective actor."), ObjectiveActor);
	if (!InstigatorActor || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.RuntimeCircuit");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::RestorePower;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::CircuitWiring;
	Objective.PromptText = FText::FromString(TEXT("接通档案终端电源"));
	Objective.CompletionText = FText::FromString(TEXT("档案终端电力恢复。"));
	Objective.ObjectiveBeats.Add(FHorrorCampaignObjectiveBeat(
		FText::FromString(TEXT("拆开保险盒")),
		FText::FromString(TEXT("找到发蓝光的主线端。"))));
	Objective.ObjectiveBeats.Add(FHorrorCampaignObjectiveBeat(
		FText::FromString(TEXT("接入主线")),
		FText::FromString(TEXT("等待端子同步后再按下对应设备键。")),
		true,
		false));
	Objective.AdvancedInteractionTuning.RequiredStepCount = 3;
	Objective.AdvancedInteractionTuning.SuccessProgress = 0.34f;

	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);
	FHorrorCampaignObjectiveRuntimeState State = ObjectiveActor->BuildObjectiveRuntimeState();
	TestTrue(TEXT("Idle runtime state should be visible for configured objectives."), State.bVisible);
	TestTrue(TEXT("Idle runtime state should allow interaction before the panel opens."), State.bCanInteract);
	TestFalse(TEXT("Idle runtime state should not mark the objective complete."), State.bCompleted);
	TestEqual(TEXT("Runtime state should expose the configured chapter id."), State.ChapterId, FName(TEXT("Chapter.Test")));
	TestEqual(TEXT("Runtime state should expose the configured objective id."), State.ObjectiveId, Objective.ObjectiveId);
	TestEqual(TEXT("Runtime state should expose the interaction mode."), State.InteractionMode, EHorrorCampaignInteractionMode::CircuitWiring);
	TestEqual(TEXT("Runtime state should expose the first beat index."), State.CurrentBeatIndex, 0);
	TestEqual(TEXT("Runtime state should expose the required step count."), State.RequiredStepCount, 3);
	TestEqual(TEXT("Idle runtime state should start at zero progress."), State.ProgressFraction, 0.0f);
	TestFalse(TEXT("Idle runtime state should not show an advanced panel yet."), State.AdvancedInteraction.bVisible);

	const FHitResult Hit;
	TestTrue(TEXT("Interacting should open the advanced circuit panel."), ObjectiveActor->Interact_Implementation(InstigatorActor, Hit));
	ObjectiveActor->Tick(0.9f);

	State = ObjectiveActor->BuildObjectiveRuntimeState();
	TestTrue(TEXT("Active runtime state should show the advanced interaction panel."), State.bAdvancedInteractionActive);
	TestTrue(TEXT("Active runtime state should include visible advanced HUD state."), State.AdvancedInteraction.bVisible);
	TestEqual(TEXT("Runtime state should mirror advanced panel progress."), State.ProgressFraction, State.AdvancedInteraction.ProgressFraction);
	TestEqual(TEXT("Runtime state should expose advanced input options."), State.AdvancedInteraction.InputOptions.Num(), 3);
	TestFalse(TEXT("Runtime state should expose a localized phase text."), State.PhaseText.IsEmpty());
	TestFalse(TEXT("Runtime state phase text should not expose English text."), ContainsLatinLetter(State.PhaseText.ToString()));
	TestEqual(TEXT("Runtime state should mirror active beat label."), State.CurrentBeatLabel.ToString(), FString(TEXT("拆开保险盒")));
	TestEqual(TEXT("Runtime state should mirror active beat detail."), State.CurrentBeatDetail.ToString(), FString(TEXT("找到发蓝光的主线端。")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignAdvancedInteractionExplainsCircuitHazardFailureTest,
	"HorrorProject.Game.Campaign.AdvancedInteractionExplainsCircuitHazardFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignAdvancedInteractionOutcomeCapturesFailureSemanticsTest,
	"HorrorProject.Game.Campaign.AdvancedInteractionOutcomeCapturesFailureSemantics",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignAdvancedInteractionPublishesCircuitRoutingStateTest,
	"HorrorProject.Game.Campaign.AdvancedInteractionPublishesCircuitRoutingState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignAdvancedInteractionPublishesCircuitRoutingStateTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for circuit routing HUD state coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Circuit routing HUD state test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Circuit routing HUD state test should spawn an objective actor."), ObjectiveActor);
	if (!InstigatorActor || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.CircuitRoutingState");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::RestorePower;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::CircuitWiring;
	Objective.PromptText = FText::FromString(TEXT("接线状态窗口"));
	Objective.AdvancedInteractionTuning.RequiredStepCount = 3;
	Objective.AdvancedInteractionTuning.SuccessProgress = 0.34f;
	Objective.AdvancedInteractionTuning.TimingWindowStartFraction = 0.05f;
	Objective.AdvancedInteractionTuning.TimingWindowEndFraction = 0.95f;
	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

	TestTrue(TEXT("Circuit routing objective should open the advanced panel."), ObjectiveActor->Interact_Implementation(InstigatorActor, FHitResult()));
	ObjectiveActor->Tick(0.2f);
	const FName FirstTerminal = ObjectiveActor->GetExpectedAdvancedInputId();
	TestEqual(TEXT("Circuit routing should start on the blue safe terminal."), FirstTerminal, FName(TEXT("蓝色端子")));
	TestTrue(TEXT("Circuit routing should accept the first safe terminal during the timing window."), ObjectiveActor->SubmitAdvancedInteractionInput(FirstTerminal, InstigatorActor));

	const FHorrorAdvancedInteractionHUDState State = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestEqual(TEXT("Circuit routing should advance to the yellow safe terminal after the blue wire connects."), State.ExpectedInputId, FName(TEXT("黄色端子")));
	TestTrue(TEXT("Circuit success feedback should name the terminal that was just connected."), State.FeedbackText.ToString().Contains(TEXT("蓝色端子")));
	TestTrue(TEXT("Circuit success feedback should describe the animated blue arc."), State.FeedbackText.ToString().Contains(TEXT("蓝色电弧")));
	const FHorrorAdvancedInteractionInputOption* BlueTerminal = State.InputOptions.FindByPredicate(
		[](const FHorrorAdvancedInteractionInputOption& Option)
		{
			return Option.InputId == FName(TEXT("蓝色端子"));
		});
	const FHorrorAdvancedInteractionInputOption* YellowTerminal = State.InputOptions.FindByPredicate(
		[](const FHorrorAdvancedInteractionInputOption& Option)
		{
			return Option.InputId == FName(TEXT("黄色端子"));
		});
	const FHorrorAdvancedInteractionInputOption* RedTerminal = State.InputOptions.FindByPredicate(
		[](const FHorrorAdvancedInteractionInputOption& Option)
		{
			return Option.InputId == FName(TEXT("红色端子"));
		});
	TestNotNull(TEXT("Circuit HUD should expose the solved blue terminal."), BlueTerminal);
	TestNotNull(TEXT("Circuit HUD should expose the next yellow terminal."), YellowTerminal);
	TestNotNull(TEXT("Circuit HUD should expose the leaking red terminal."), RedTerminal);
	if (BlueTerminal && YellowTerminal && RedTerminal)
	{
		TestTrue(TEXT("Connected circuit terminals should be marked as chain linked for the animated wire path."), BlueTerminal->bChainLinked);
		TestFalse(TEXT("Connected circuit terminals should no longer be stalled as the current target."), BlueTerminal->bStalled);
		TestTrue(TEXT("Connected circuit terminals should retain full response telemetry."), BlueTerminal->ResponseWindowFraction >= 0.95f);
		TestTrue(TEXT("The next safe circuit terminal should be marked as the active stalled target."), YellowTerminal->bStalled);
		TestTrue(TEXT("The next safe circuit terminal should expose timing response telemetry."), YellowTerminal->ResponseWindowFraction >= 0.0f && YellowTerminal->ResponseWindowFraction <= 1.0f);
		TestTrue(TEXT("The next safe circuit terminal should carry lower load than the leaking red terminal."), YellowTerminal->LoadFraction < RedTerminal->LoadFraction);
		TestTrue(TEXT("The red circuit terminal should remain hazardous after safe routing progresses."), RedTerminal->bHazardous);
		TestFalse(TEXT("The red circuit terminal should not be marked as a solved route."), RedTerminal->bChainLinked);
	}
	TestTrue(TEXT("Circuit routing next action should identify the next yellow terminal."), State.NextActionLabel.ToString().Contains(TEXT("黄色端子")));
	TestTrue(TEXT("Circuit routing rhythm should describe safe route progress instead of a generic timing prompt."), State.RhythmLabel.ToString().Contains(TEXT("安全线路")) || State.RhythmLabel.ToString().Contains(TEXT("接入")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

bool FHorrorCampaignAdvancedInteractionOutcomeCapturesFailureSemanticsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for advanced outcome coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	TestNotNull(TEXT("Advanced outcome test should spawn an instigator."), InstigatorActor);
	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	TestNotNull(TEXT("Advanced outcome test should expose the event bus."), EventBus);
	if (!InstigatorActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	auto SpawnCircuitObjective = [World](const TCHAR* ObjectiveId, float FailureStabilityDamage = 0.2f) -> AHorrorCampaignObjectiveActor*
	{
		AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
		if (!ObjectiveActor)
		{
			return nullptr;
		}

		FHorrorCampaignObjectiveDefinition Objective;
		Objective.ObjectiveId = ObjectiveId;
		Objective.ObjectiveType = EHorrorCampaignObjectiveType::RestorePower;
		Objective.InteractionMode = EHorrorCampaignInteractionMode::CircuitWiring;
		Objective.PromptText = FText::FromString(TEXT("接线结果语义测试"));
		Objective.AdvancedInteractionTuning.RequiredStepCount = 3;
		Objective.AdvancedInteractionTuning.SuccessProgress = 0.34f;
		Objective.AdvancedInteractionTuning.TimingWindowStartFraction = 0.05f;
		Objective.AdvancedInteractionTuning.TimingWindowEndFraction = 0.95f;
		Objective.AdvancedInteractionTuning.FailureStabilityDamage = FailureStabilityDamage;
		ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);
		return ObjectiveActor;
	};

	AHorrorCampaignObjectiveActor* TimingObjective = SpawnCircuitObjective(TEXT("Test.OutcomeTiming"));
	TestNotNull(TEXT("Timing outcome test should spawn a circuit objective."), TimingObjective);
	if (!TimingObjective)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	TestTrue(TEXT("Timing outcome objective should open the advanced panel."), TimingObjective->Interact_Implementation(InstigatorActor, FHitResult()));
	const FName TimingExpectedInput = TimingObjective->GetExpectedAdvancedInputId();
	TestTrue(TEXT("Premature circuit input should still be consumed by the advanced interaction router."), TimingObjective->SubmitAdvancedInteractionInput(TimingExpectedInput, InstigatorActor));
	const FHorrorAdvancedInteractionOutcome TimingOutcome = TimingObjective->GetLastAdvancedInteractionOutcome();
	TestEqual(TEXT("Premature input should record a timing failure outcome."), TimingOutcome.Kind, EHorrorAdvancedInteractionOutcomeKind::TimingFailure);
	TestEqual(TEXT("Timing failure outcome should expose the timing fault id."), TimingOutcome.FaultId, FName(TEXT("Fault.Advanced.Timing")));
	TestEqual(TEXT("Timing failure outcome should carry failure feedback state."), TimingOutcome.FeedbackState, EHorrorAdvancedInteractionFeedbackState::Failure);
	TestTrue(TEXT("Timing failure outcome should consume the submitted input."), TimingOutcome.bConsumesInput);
	TestFalse(TEXT("Timing failure outcome should not advance objective progress."), TimingOutcome.bAdvancesProgress);
	TestFalse(TEXT("Timing failure outcome should not be retryable until stability is exhausted."), TimingOutcome.bRetryable);
	TimingObjective->Tick(3.0f);
	TestTrue(TEXT("Explicit selection prompt should be handled after the timing pause clears."), TimingObjective->PromptForExplicitAdvancedInteractionSelection());
	const FHorrorAdvancedInteractionOutcome PromptOutcome = TimingObjective->GetLastAdvancedInteractionOutcome();
	TestEqual(TEXT("Explicit selection should record a prompted outcome."), PromptOutcome.Kind, EHorrorAdvancedInteractionOutcomeKind::Prompted);
	TestTrue(TEXT("Explicit selection prompt should consume the player request."), PromptOutcome.bConsumesInput);
	TestFalse(TEXT("Explicit selection prompt should not advance objective progress."), PromptOutcome.bAdvancesProgress);
	FHorrorAdvancedInteractionInputCommand UnsupportedAxisCommand;
	UnsupportedAxisCommand.CommandType = EHorrorAdvancedInteractionCommandType::AdjustAxis;
	UnsupportedAxisCommand.AxisValue = 1.0f;
	const float StabilityBeforeUnsupportedAxis = TimingObjective->GetAdvancedInteractionStabilityFraction();
	const float ProgressBeforeUnsupportedAxis = TimingObjective->GetAdvancedInteractionProgressFraction();
	TestFalse(TEXT("Circuit objectives should ignore unsupported continuous axis commands."), TimingObjective->SubmitAdvancedInteractionCommand(UnsupportedAxisCommand, InstigatorActor));
	const FHorrorAdvancedInteractionOutcome UnsupportedAxisOutcome = TimingObjective->GetLastAdvancedInteractionOutcome();
	TestEqual(TEXT("Unsupported axis input should record an ignored outcome."), UnsupportedAxisOutcome.Kind, EHorrorAdvancedInteractionOutcomeKind::Ignored);
	TestFalse(TEXT("Unsupported axis input should not be consumed."), UnsupportedAxisOutcome.bConsumesInput);
	TestEqual(TEXT("Unsupported axis input should not damage device stability."), TimingObjective->GetAdvancedInteractionStabilityFraction(), StabilityBeforeUnsupportedAxis);
	TestEqual(TEXT("Unsupported axis input should not advance objective progress."), TimingObjective->GetAdvancedInteractionProgressFraction(), ProgressBeforeUnsupportedAxis);

	FHorrorAdvancedInteractionInputCommand CancelCommand;
	CancelCommand.CommandType = EHorrorAdvancedInteractionCommandType::Cancel;
	TestTrue(TEXT("Cancel command should be consumed as a non-destructive panel hint."), TimingObjective->SubmitAdvancedInteractionCommand(CancelCommand, InstigatorActor));
	const FHorrorAdvancedInteractionOutcome CancelOutcome = TimingObjective->GetLastAdvancedInteractionOutcome();
	TestEqual(TEXT("Cancel command should record a cancelled outcome."), CancelOutcome.Kind, EHorrorAdvancedInteractionOutcomeKind::Cancelled);
	TestEqual(TEXT("Cancel outcome should stay neutral because it does not pause gameplay."), CancelOutcome.FeedbackState, EHorrorAdvancedInteractionFeedbackState::Neutral);
	TestTrue(TEXT("Cancel command should keep the advanced interaction active."), TimingObjective->IsAdvancedInteractionActive());
	TestEqual(TEXT("Cancel command should not start a pause timer."), TimingObjective->GetAdvancedInteractionPauseRemainingSeconds(), 0.0f);

	AHorrorCampaignObjectiveActor* HazardObjective = SpawnCircuitObjective(TEXT("Test.OutcomeHazard"));
	TestNotNull(TEXT("Hazard outcome test should spawn a circuit objective."), HazardObjective);
	if (!HazardObjective)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	TestTrue(TEXT("Hazard outcome objective should open the advanced panel."), HazardObjective->Interact_Implementation(InstigatorActor, FHitResult()));
	HazardObjective->Tick(0.2f);
	TestTrue(TEXT("Hazard outcome objective should accept the red terminal as a handled failure."), HazardObjective->SubmitAdvancedInteractionInput(TEXT("红色端子"), InstigatorActor));
	const FHorrorAdvancedInteractionOutcome HazardOutcome = HazardObjective->GetLastAdvancedInteractionOutcome();
	TestEqual(TEXT("Hazard input should record a concrete hazard outcome."), HazardOutcome.Kind, EHorrorAdvancedInteractionOutcomeKind::Hazard);
	TestEqual(TEXT("Hazard input should expose the short-circuit fault id."), HazardOutcome.FaultId, FName(TEXT("Fault.Advanced.CircuitHazard")));
	TestEqual(TEXT("Hazard outcome should carry failure feedback state."), HazardOutcome.FeedbackState, EHorrorAdvancedInteractionFeedbackState::Failure);
	TestTrue(TEXT("Hazard outcome should consume the submitted input."), HazardOutcome.bConsumesInput);
	TestFalse(TEXT("Hazard outcome should not advance objective progress."), HazardOutcome.bAdvancesProgress);
	const FGameplayTag CircuitFailureTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Circuit.Failure")), false);
	const FHorrorEventMessage* HazardFailureMessage = EventBus ? EventBus->GetHistory().FindByPredicate(
		[CircuitFailureTag](const FHorrorEventMessage& Message)
		{
			return Message.EventTag == CircuitFailureTag && Message.SourceId == FName(TEXT("Test.OutcomeHazard"));
		}) : nullptr;
	TestNotNull(TEXT("Hazard input should publish a structured advanced failure event."), HazardFailureMessage);
	if (HazardFailureMessage)
	{
		TestEqual(TEXT("Hazard event should expose the advanced outcome kind."), HazardFailureMessage->AdvancedOutcomeKind, EHorrorAdvancedInteractionOutcomeKind::Hazard);
		TestEqual(TEXT("Hazard event should expose the advanced fault id."), HazardFailureMessage->AdvancedFaultId, FName(TEXT("Fault.Advanced.CircuitHazard")));
	}
	const FHorrorAdvancedInteractionHUDState HazardState = HazardObjective->BuildAdvancedInteractionHUDState();
	TestEqual(TEXT("Hazard HUD state should expose the structured last outcome."), HazardState.LastOutcome.Kind, EHorrorAdvancedInteractionOutcomeKind::Hazard);
	TestEqual(TEXT("Hazard HUD state should expose the concrete fault id."), HazardState.LastOutcome.FaultId, FName(TEXT("Fault.Advanced.CircuitHazard")));

	AHorrorCampaignObjectiveActor* OverloadObjective = SpawnCircuitObjective(TEXT("Test.OutcomeOverload"), 1.0f);
	TestNotNull(TEXT("Overload outcome test should spawn a circuit objective."), OverloadObjective);
	if (!OverloadObjective)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	TestTrue(TEXT("Overload outcome objective should open the advanced panel."), OverloadObjective->Interact_Implementation(InstigatorActor, FHitResult()));
	OverloadObjective->Tick(0.2f);
	TestTrue(TEXT("Overload outcome should consume the failing hazard input."), OverloadObjective->SubmitAdvancedInteractionInput(TEXT("红色端子"), InstigatorActor));
	const FHorrorAdvancedInteractionOutcome OverloadOutcome = OverloadObjective->GetLastAdvancedInteractionOutcome();
	TestEqual(TEXT("Stability exhaustion should record an overloaded outcome."), OverloadOutcome.Kind, EHorrorAdvancedInteractionOutcomeKind::Overloaded);
	TestEqual(TEXT("Overload outcome should expose circuit overload cause."), OverloadOutcome.FailureCause, FName(TEXT("Failure.Campaign.CircuitOverload")));
	TestEqual(TEXT("Overload outcome should expose retry recovery action."), OverloadOutcome.RecoveryAction, FName(TEXT("Recovery.Objective.Retry")));
	TestEqual(TEXT("Overload outcome should carry overloaded feedback state."), OverloadOutcome.FeedbackState, EHorrorAdvancedInteractionFeedbackState::Overloaded);
	TestTrue(TEXT("Overload outcome should remain retryable for the player."), OverloadOutcome.bRetryable);
	TestTrue(TEXT("Overload outcome should consume the input that exhausted stability."), OverloadOutcome.bConsumesInput);
	TestEqual(TEXT("Overload runtime state should still enter retryable failure."), OverloadObjective->BuildObjectiveRuntimeState().Status, EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable);
	const FHorrorCampaignObjectiveRuntimeState OverloadRuntimeState = OverloadObjective->BuildObjectiveRuntimeState();
	TestEqual(TEXT("Overload runtime HUD state should expose the overloaded outcome."), OverloadRuntimeState.AdvancedInteraction.LastOutcome.Kind, EHorrorAdvancedInteractionOutcomeKind::Overloaded);
	TestEqual(TEXT("Overload runtime HUD state should preserve retry recovery action in the last outcome."), OverloadRuntimeState.AdvancedInteraction.LastOutcome.RecoveryAction, FName(TEXT("Recovery.Objective.Retry")));

	AHorrorCampaignObjectiveActor* SignalObjective = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Signal adjust outcome test should spawn a signal objective."), SignalObjective);
	if (!SignalObjective)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	FHorrorCampaignObjectiveDefinition SignalDefinition;
	SignalDefinition.ObjectiveId = TEXT("Test.OutcomeSignalAdjust");
	SignalDefinition.ObjectiveType = EHorrorCampaignObjectiveType::AcquireSignal;
	SignalDefinition.InteractionMode = EHorrorCampaignInteractionMode::SignalTuning;
	SignalDefinition.PromptText = FText::FromString(TEXT("调谐结果语义测试"));
	SignalDefinition.AdvancedInteractionTuning.RequiredStepCount = 3;
	SignalDefinition.AdvancedInteractionTuning.SuccessProgress = 0.34f;
	SignalObjective->ConfigureObjective(TEXT("Chapter.Test"), SignalDefinition);
	TestTrue(TEXT("Signal adjust objective should open the advanced panel."), SignalObjective->Interact_Implementation(InstigatorActor, FHitResult()));
	FHorrorAdvancedInteractionInputCommand AdjustCommand;
	AdjustCommand.CommandType = EHorrorAdvancedInteractionCommandType::AdjustAxis;
	AdjustCommand.AxisValue = 1.0f;
	AdjustCommand.HoldSeconds = 0.4f;
	TestTrue(TEXT("Signal axis adjust should be consumed by the advanced interaction router."), SignalObjective->SubmitAdvancedInteractionCommand(AdjustCommand, InstigatorActor));
	const FHorrorAdvancedInteractionOutcome AdjustOutcome = SignalObjective->GetLastAdvancedInteractionOutcome();
	TestEqual(TEXT("Axis adjust should record an adjusted outcome instead of a failure."), AdjustOutcome.Kind, EHorrorAdvancedInteractionOutcomeKind::Adjusted);
	TestEqual(TEXT("Axis adjust outcome should stay neutral."), AdjustOutcome.FeedbackState, EHorrorAdvancedInteractionFeedbackState::Neutral);
	TestTrue(TEXT("Axis adjust outcome should consume the command."), AdjustOutcome.bConsumesInput);
	TestFalse(TEXT("Axis adjust outcome should not directly advance progress."), AdjustOutcome.bAdvancesProgress);
	TestTrue(TEXT("Axis adjust outcome should identify the adjusted control."), AdjustOutcome.InputId == FName(TEXT("右声道")) || AdjustOutcome.InputId == NAME_None);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

bool FHorrorCampaignAdvancedInteractionExplainsCircuitHazardFailureTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for circuit hazard diagnostic coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Circuit hazard diagnostic test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Circuit hazard diagnostic test should spawn an objective actor."), ObjectiveActor);
	if (!InstigatorActor || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.CircuitHazardDiagnostic");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::RestorePower;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::CircuitWiring;
	Objective.PromptText = FText::FromString(TEXT("诊断短路端子"));
	Objective.AdvancedInteractionTuning.RequiredStepCount = 3;
	Objective.AdvancedInteractionTuning.TimingWindowStartFraction = 0.05f;
	Objective.AdvancedInteractionTuning.TimingWindowEndFraction = 0.95f;
	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

	TestTrue(TEXT("Circuit hazard diagnostic objective should open the advanced panel."), ObjectiveActor->Interact_Implementation(InstigatorActor, FHitResult()));
	ObjectiveActor->Tick(0.2f);
	TestTrue(TEXT("Circuit hazard diagnostic should be inside a generous timing band."), ObjectiveActor->IsAdvancedInteractionTimingWindowOpen());
	TestTrue(TEXT("Submitting the red high-load terminal should be handled by the advanced panel."), ObjectiveActor->SubmitAdvancedInteractionInput(TEXT("红色端子"), InstigatorActor));

	const FHorrorAdvancedInteractionHUDState State = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestEqual(TEXT("Circuit hazard failure should publish a failure feedback state."), State.FeedbackState, EHorrorAdvancedInteractionFeedbackState::Failure);
	TestTrue(TEXT("Circuit hazard failure feedback should name the short circuit load."), State.FeedbackText.ToString().Contains(TEXT("短路负载")));
	TestTrue(TEXT("Circuit hazard diagnostics should persist the concrete short-circuit cause."), State.DeviceStatusLabel.ToString().Contains(TEXT("短路")));
	TestTrue(TEXT("Circuit hazard next action should point to the safe highlighted terminal."), State.NextActionLabel.ToString().Contains(TEXT("安全高亮端子")));
	TestTrue(TEXT("Circuit hazard recovery should tell the player to avoid the leaking red terminal."), State.FailureRecoveryLabel.ToString().Contains(TEXT("漏电端子")));
	TestFalse(TEXT("Circuit hazard diagnostics should remain fully localized."), ContainsDisallowedEnglishText(State.DeviceStatusLabel.ToString() + State.NextActionLabel.ToString() + State.FailureRecoveryLabel.ToString()));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignAdvancedInteractionBuildsDiagnosticHUDStateTest,
	"HorrorProject.Game.Campaign.AdvancedInteractionBuildsDiagnosticHUDState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignAdvancedInteractionBuildsDiagnosticHUDStateTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for advanced diagnostic HUD coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	TestNotNull(TEXT("Advanced diagnostic test should spawn an instigator."), InstigatorActor);
	if (!InstigatorActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	struct FDiagnosticModeCase
	{
		EHorrorCampaignInteractionMode Mode = EHorrorCampaignInteractionMode::Instant;
		EHorrorCampaignObjectiveType ObjectiveType = EHorrorCampaignObjectiveType::RestorePower;
		FName ObjectiveId = NAME_None;
		FText PromptText;
		const TCHAR* ExpectedStatusFragment = TEXT("");
		const TCHAR* ExpectedRhythmFragment = TEXT("");
	};

	const FDiagnosticModeCase Cases[] = {
		{
			EHorrorCampaignInteractionMode::CircuitWiring,
			EHorrorCampaignObjectiveType::RestorePower,
			TEXT("Test.DiagnosticCircuit"),
			FText::FromString(TEXT("诊断接线面板")),
			TEXT("端子"),
			TEXT("窗口")
		},
		{
			EHorrorCampaignInteractionMode::GearCalibration,
			EHorrorCampaignObjectiveType::DisableSeal,
			TEXT("Test.DiagnosticGear"),
			FText::FromString(TEXT("诊断齿轮面板")),
			TEXT("齿轮"),
			TEXT("窗口")
		},
		{
			EHorrorCampaignInteractionMode::SpectralScan,
			EHorrorCampaignObjectiveType::ScanAnomaly,
			TEXT("Test.DiagnosticScan"),
			FText::FromString(TEXT("诊断频谱面板")),
			TEXT("扫描"),
			TEXT("置信")
		},
		{
			EHorrorCampaignInteractionMode::SignalTuning,
			EHorrorCampaignObjectiveType::AcquireSignal,
			TEXT("Test.DiagnosticTuning"),
			FText::FromString(TEXT("诊断调谐面板")),
			TEXT("声像"),
			TEXT("中心")
		}
	};

	for (const FDiagnosticModeCase& ModeCase : Cases)
	{
		AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
		TestNotNull(
			FString::Printf(TEXT("Advanced diagnostic test should spawn objective actor for %s."), *ModeCase.ObjectiveId.ToString()),
			ObjectiveActor);
		if (!ObjectiveActor)
		{
			TestWorld.DestroyTestWorld(false);
			return false;
		}

		FHorrorCampaignObjectiveDefinition Objective;
		Objective.ObjectiveId = ModeCase.ObjectiveId;
		Objective.ObjectiveType = ModeCase.ObjectiveType;
		Objective.InteractionMode = ModeCase.Mode;
		Objective.PromptText = ModeCase.PromptText;
		Objective.AdvancedInteractionTuning.RequiredStepCount = 3;
		Objective.AdvancedInteractionTuning.SuccessProgress = 0.34f;
		ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

		TestTrue(
			FString::Printf(TEXT("Advanced diagnostic objective should open panel for %s."), *ModeCase.ObjectiveId.ToString()),
			ObjectiveActor->Interact_Implementation(InstigatorActor, FHitResult()));
		ObjectiveActor->Tick(0.9f);

		FHorrorAdvancedInteractionHUDState State = ObjectiveActor->BuildAdvancedInteractionHUDState();
		TestAdvancedInteractionDiagnosticsAreLocalized(*this, State, *ModeCase.ObjectiveId.ToString());
		TestTrue(
			FString::Printf(TEXT("Diagnostic status should describe the concrete device for %s."), *ModeCase.ObjectiveId.ToString()),
			State.DeviceStatusLabel.ToString().Contains(ModeCase.ExpectedStatusFragment));
		TestTrue(
			FString::Printf(TEXT("Diagnostic rhythm should describe the active timing read for %s."), *ModeCase.ObjectiveId.ToString()),
			State.RhythmLabel.ToString().Contains(ModeCase.ExpectedRhythmFragment));

		const float InitialGrade = State.PerformanceGradeFraction;
		TestTrue(TEXT("A fresh advanced panel should begin with a useful performance grade."), InitialGrade >= 0.35f);
		TestTrue(TEXT("An untouched advanced panel should not claim perfect mastery."), InitialGrade < 1.0f);

		TestTrue(
			FString::Printf(TEXT("Wrong input should degrade diagnostics for %s."), *ModeCase.ObjectiveId.ToString()),
			ObjectiveActor->SubmitAdvancedInteractionInput(TEXT("错误输入"), InstigatorActor));
		State = ObjectiveActor->BuildAdvancedInteractionHUDState();
		TestAdvancedInteractionDiagnosticsAreLocalized(*this, State, *ModeCase.ObjectiveId.ToString());
		TestTrue(TEXT("Diagnostic risk should escalate after mistakes."), State.RiskLabel.ToString().Contains(TEXT("风险")) || State.RiskLabel.ToString().Contains(TEXT("过载")));
		TestFalse(TEXT("Diagnostic recovery should explain how to recover after mistakes."), State.FailureRecoveryLabel.IsEmpty());
		TestFalse(TEXT("Diagnostic recovery should not expose English text."), ContainsLatinLetter(State.FailureRecoveryLabel.ToString()));
		TestTrue(TEXT("Performance grade should drop after a handled mistake."), State.PerformanceGradeFraction < InitialGrade);
	}

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveRuntimeSaveStateAdvancedInteractionRoundTripTest,
	"HorrorProject.Game.Campaign.ObjectiveRuntimeSaveState.AdvancedInteractionRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveRuntimeSaveStateAdvancedInteractionRoundTripTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for advanced interaction runtime save coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* SourceActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	AHorrorCampaignObjectiveActor* RestoredActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Runtime save test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Runtime save test should spawn a source objective actor."), SourceActor);
	TestNotNull(TEXT("Runtime save test should spawn a restored objective actor."), RestoredActor);
	if (!InstigatorActor || !SourceActor || !RestoredActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.RuntimeSaveCircuit");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::RestorePower;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::CircuitWiring;
	Objective.PromptText = FText::FromString(TEXT("保存中的测试电路"));
	Objective.CompletionText = FText::FromString(TEXT("测试电路恢复。"));
	Objective.AdvancedInteractionTuning.RequiredStepCount = 3;
	Objective.AdvancedInteractionTuning.SuccessProgress = 0.34f;
	Objective.AdvancedInteractionTuning.SuccessStabilityRecovery = 0.0f;

	SourceActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);
	RestoredActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

	const FHitResult Hit;
	TestTrue(TEXT("Source objective should open the advanced panel before saving."), SourceActor->Interact_Implementation(InstigatorActor, Hit));
	const FName FirstExpectedInput = SourceActor->GetExpectedAdvancedInputId();
	TestFalse(TEXT("Source objective should expose an expected advanced input before saving."), FirstExpectedInput.IsNone());
	SourceActor->Tick(0.65f);
	TestTrue(TEXT("Source objective should enter the timing window before accepting saved progress input."), SourceActor->IsAdvancedInteractionTimingWindowOpen());
	TestTrue(TEXT("Correct input should advance saved advanced progress."), SourceActor->SubmitAdvancedInteractionInput(FirstExpectedInput, InstigatorActor));
	SourceActor->Tick(0.42f);

	const FHorrorCampaignObjectiveSaveState SaveState = SourceActor->ExportObjectiveSaveState();
	TestTrue(TEXT("Advanced interaction save state should mark itself as persistent runtime state."), SaveState.HasRuntimeState());
	TestEqual(TEXT("Save state should carry the source chapter id."), SaveState.ChapterId, FName(TEXT("Chapter.Test")));
	TestEqual(TEXT("Save state should carry the source objective id."), SaveState.ObjectiveId, Objective.ObjectiveId);
	TestTrue(TEXT("Save state should preserve active advanced panel flag."), SaveState.bAdvancedInteractionActive);
	TestTrue(TEXT("Save state should preserve advanced progress."), SaveState.AdvancedInteractionProgressFraction > 0.0f);
	TestEqual(TEXT("Save state should preserve advanced step index."), SaveState.AdvancedInteractionStepIndex, SourceActor->BuildAdvancedInteractionHUDState().StepIndex);
	TestEqual(TEXT("Save state should preserve expected input id."), SaveState.ExpectedAdvancedInputId, SourceActor->GetExpectedAdvancedInputId());
	TestTrue(TEXT("Save state should preserve cue timing."), SaveState.AdvancedInteractionTimingFraction > 0.0f);

	RestoredActor->ImportObjectiveSaveState(SaveState, InstigatorActor);
	const FHorrorCampaignObjectiveRuntimeState RestoredRuntimeState = RestoredActor->BuildObjectiveRuntimeState();
	TestEqual(TEXT("Restored objective runtime should reopen the advanced interaction state."), RestoredRuntimeState.Status, EHorrorCampaignObjectiveRuntimeStatus::AdvancedInteractionActive);
	TestTrue(TEXT("Restored objective should be interactable through the active panel."), RestoredActor->IsAdvancedInteractionActive());
	TestEqual(TEXT("Restored runtime should preserve advanced progress."), RestoredRuntimeState.AdvancedInteraction.ProgressFraction, SaveState.AdvancedInteractionProgressFraction);
	TestEqual(TEXT("Restored runtime should preserve step index."), RestoredRuntimeState.AdvancedInteraction.StepIndex, SaveState.AdvancedInteractionStepIndex);
	TestEqual(TEXT("Restored runtime should preserve expected input."), RestoredRuntimeState.AdvancedInteraction.ExpectedInputId, SaveState.ExpectedAdvancedInputId);
	TestEqual(TEXT("Restored runtime should preserve stability."), RestoredRuntimeState.AdvancedInteraction.StabilityFraction, SaveState.AdvancedInteractionStabilityFraction);
	TestEqual(TEXT("Restored runtime should preserve combo count."), RestoredRuntimeState.AdvancedInteraction.ComboCount, SaveState.AdvancedInteractionComboCount);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveRuntimeSaveStatePreservesAdvancedMinigameStateTest,
	"HorrorProject.Game.Campaign.ObjectiveRuntimeSaveState.PreservesAdvancedMinigameState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveRuntimeSaveStatePreservesAdvancedMinigameStateTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for advanced minigame save coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* SourceSignalActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	AHorrorCampaignObjectiveActor* RestoredSignalActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	AHorrorCampaignObjectiveActor* SourceScanActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	AHorrorCampaignObjectiveActor* RestoredScanActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Advanced minigame save test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Advanced minigame save test should spawn a source signal objective."), SourceSignalActor);
	TestNotNull(TEXT("Advanced minigame save test should spawn a restored signal objective."), RestoredSignalActor);
	TestNotNull(TEXT("Advanced minigame save test should spawn a source scan objective."), SourceScanActor);
	TestNotNull(TEXT("Advanced minigame save test should spawn a restored scan objective."), RestoredScanActor);
	if (!InstigatorActor || !SourceSignalActor || !RestoredSignalActor || !SourceScanActor || !RestoredScanActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition SignalObjective;
	SignalObjective.ObjectiveId = TEXT("Test.SaveSignalTuning");
	SignalObjective.ObjectiveType = EHorrorCampaignObjectiveType::AcquireSignal;
	SignalObjective.InteractionMode = EHorrorCampaignInteractionMode::SignalTuning;
	SignalObjective.PromptText = FText::FromString(TEXT("保存调谐声像"));
	SourceSignalActor->ConfigureObjective(TEXT("Chapter.Test"), SignalObjective);
	RestoredSignalActor->ConfigureObjective(TEXT("Chapter.Test"), SignalObjective);

	TestTrue(TEXT("Signal objective should open the tuning panel before saving."), SourceSignalActor->Interact_Implementation(InstigatorActor, FHitResult()));
	TestTrue(TEXT("Signal objective should accept a left-channel tuning nudge before saving."), SourceSignalActor->SubmitAdvancedInteractionInput(TEXT("左声道"), InstigatorActor));
	const FHorrorCampaignObjectiveSaveState SignalSaveState = SourceSignalActor->ExportObjectiveSaveState();
	TestTrue(TEXT("Signal save state should preserve the shifted tuning balance."), SignalSaveState.SignalTuningBalanceFraction < 0.5f);
	RestoredSignalActor->ImportObjectiveSaveState(SignalSaveState, InstigatorActor);
	const FHorrorCampaignObjectiveRuntimeState RestoredSignalState = RestoredSignalActor->BuildObjectiveRuntimeState();
	TestEqual(TEXT("Restored signal runtime should preserve the tuning balance."), RestoredSignalState.AdvancedInteraction.SignalBalanceFraction, SignalSaveState.SignalTuningBalanceFraction);
	TestEqual(TEXT("Restored signal runtime should preserve the tuning target."), RestoredSignalState.AdvancedInteraction.SignalTargetBalanceFraction, SignalSaveState.SignalTuningTargetBalanceFraction);

	FHorrorCampaignObjectiveDefinition ScanObjective;
	ScanObjective.ObjectiveId = TEXT("Test.SaveSpectralScan");
	ScanObjective.ObjectiveType = EHorrorCampaignObjectiveType::ScanAnomaly;
	ScanObjective.InteractionMode = EHorrorCampaignInteractionMode::SpectralScan;
	ScanObjective.PromptText = FText::FromString(TEXT("保存扫描置信度"));
	SourceScanActor->ConfigureObjective(TEXT("Chapter.Test"), ScanObjective);
	RestoredScanActor->ConfigureObjective(TEXT("Chapter.Test"), ScanObjective);

	TestTrue(TEXT("Spectral objective should open the scan panel before saving."), SourceScanActor->Interact_Implementation(InstigatorActor, FHitResult()));
	SourceScanActor->Tick(0.42f);
	const FHorrorCampaignObjectiveSaveState ScanSaveState = SourceScanActor->ExportObjectiveSaveState();
	TestTrue(TEXT("Spectral save state should preserve scan confidence."), ScanSaveState.SpectralScanConfidenceFraction > 0.0f);
	RestoredScanActor->ImportObjectiveSaveState(ScanSaveState, InstigatorActor);
	const FHorrorCampaignObjectiveRuntimeState RestoredScanState = RestoredScanActor->BuildObjectiveRuntimeState();
	TestEqual(TEXT("Restored scan runtime should preserve confidence."), RestoredScanState.AdvancedInteraction.SpectralConfidenceFraction, ScanSaveState.SpectralScanConfidenceFraction);
	TestEqual(TEXT("Restored scan runtime should preserve noise."), RestoredScanState.AdvancedInteraction.SpectralNoiseFraction, ScanSaveState.SpectralScanNoiseFraction);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveRuntimeSaveStateGameModeRoundTripTest,
	"HorrorProject.Game.Campaign.ObjectiveRuntimeSaveState.GameModeRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveRuntimeSaveStateGameModeRoundTripTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for game mode objective runtime save coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the Horror game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AActor* InstigatorActor = World->SpawnActor<AActor>();
	TestNotNull(TEXT("Game mode runtime save test should expose the game mode."), GameMode);
	TestNotNull(TEXT("Game mode runtime save test should spawn an instigator."), InstigatorActor);
	if (!GameMode || !InstigatorActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FHorrorCampaignChapterDefinition* DeepWaterChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.DeepWaterStationFinale"));
	TestNotNull(TEXT("Deep Water chapter should exist for game mode runtime save coverage."), DeepWaterChapter);
	if (!DeepWaterChapter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(DeepWaterChapter->ChapterId);
	AHorrorCampaignObjectiveActor* BootObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Game mode runtime save test should spawn a boot objective actor."), BootObjectiveActor);
	if (!BootObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* BootObjective = FHorrorCampaign::FindObjectiveById(
		*DeepWaterChapter,
		TEXT("DeepWater.BootDryDock"));
	TestNotNull(TEXT("Game mode runtime save test should find the boot objective."), BootObjective);
	if (!BootObjective)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	BootObjectiveActor->ConfigureObjective(DeepWaterChapter->ChapterId, *BootObjective);
	GameMode->AddRuntimeCampaignObjectiveForTests(BootObjectiveActor);

	const FHitResult Hit;
	TestTrue(TEXT("Boot objective should open its circuit panel before exporting game mode state."), BootObjectiveActor->Interact_Implementation(InstigatorActor, Hit));
	BootObjectiveActor->Tick(0.65f);
	TestTrue(TEXT("Boot objective should accept a correct circuit input before game mode export."), BootObjectiveActor->SubmitAdvancedInteractionInput(BootObjectiveActor->GetExpectedAdvancedInputId(), InstigatorActor));

	const FHorrorCampaignSaveState SaveState = GameMode->ExportCampaignSaveState();
	TestEqual(TEXT("Game mode campaign save should capture one active objective runtime state."), SaveState.ObjectiveRuntimeStates.Num(), 1);
	TestEqual(TEXT("Game mode campaign save should identify the active circuit objective."), SaveState.ObjectiveRuntimeStates[0].ObjectiveId, BootObjective->ObjectiveId);
	TestTrue(TEXT("Game mode campaign save should preserve active advanced state."), SaveState.ObjectiveRuntimeStates[0].bAdvancedInteractionActive);

	AHorrorCampaignObjectiveActor* RestoredBootObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Game mode runtime restore test should spawn a replacement objective actor."), RestoredBootObjectiveActor);
	if (!RestoredBootObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	BootObjectiveActor->Destroy();
	GameMode->ResetRuntimeCampaignObjectiveViewsForTests();
	RestoredBootObjectiveActor->ConfigureObjective(DeepWaterChapter->ChapterId, *BootObjective);
	GameMode->AddRuntimeCampaignObjectiveForTests(RestoredBootObjectiveActor);

	GameMode->ImportCampaignSaveState(SaveState);
	const FHorrorCampaignObjectiveRuntimeState RestoredRuntimeState = RestoredBootObjectiveActor->BuildObjectiveRuntimeState();
	TestEqual(TEXT("Game mode import should restore the active advanced interaction state."), RestoredRuntimeState.Status, EHorrorCampaignObjectiveRuntimeStatus::AdvancedInteractionActive);
	TestEqual(TEXT("Game mode import should restore advanced progress."), RestoredRuntimeState.AdvancedInteraction.ProgressFraction, SaveState.ObjectiveRuntimeStates[0].AdvancedInteractionProgressFraction);
	TestEqual(TEXT("Game mode import should restore expected input."), RestoredRuntimeState.AdvancedInteraction.ExpectedInputId, SaveState.ObjectiveRuntimeStates[0].ExpectedAdvancedInputId);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveRuntimeSaveStateTimedPursuitRoundTripTest,
	"HorrorProject.Game.Campaign.ObjectiveRuntimeSaveState.TimedPursuitRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveRuntimeSaveStateTimedPursuitRoundTripTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for timed pursuit runtime save coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHorrorCampaignObjectiveActor* SourceActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHorrorCampaignObjectiveActor* RestoredActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	TestNotNull(TEXT("Timed pursuit save test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Timed pursuit save test should spawn a source objective actor."), SourceActor);
	TestNotNull(TEXT("Timed pursuit save test should spawn a restored objective actor."), RestoredActor);
	if (!InstigatorActor || !SourceActor || !RestoredActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.RuntimeSavePursuit");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::SurviveAmbush;
	Objective.PromptText = FText::FromString(TEXT("保存中的测试追逐"));
	Objective.CompletionText = FText::FromString(TEXT("测试追逐完成。"));
	Objective.TimedObjectiveDurationSeconds = 12.0f;
	Objective.EscapeDestinationOffset = FVector(3200.0f, 400.0f, 0.0f);
	Objective.EscapeCompletionRadius = 260.0f;

	SourceActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);
	RestoredActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

	const FHitResult Hit;
	TestTrue(TEXT("Source pursuit should start before saving."), SourceActor->Interact_Implementation(InstigatorActor, Hit));
	SourceActor->Tick(3.5f);
	TestTrue(TEXT("Source pursuit should still be active before saving."), SourceActor->IsTimedObjectiveActive());

	const FHorrorCampaignObjectiveSaveState SaveState = SourceActor->ExportObjectiveSaveState();
	TestTrue(TEXT("Pursuit save state should mark itself as runtime state."), SaveState.HasRuntimeState());
	TestTrue(TEXT("Pursuit save state should preserve active timed objective flag."), SaveState.bTimedObjectiveActive);
	TestTrue(TEXT("Pursuit save state should preserve remaining time."), SaveState.TimedObjectiveRemainingSeconds < SaveState.TimedObjectiveDurationSeconds);
	TestTrue(TEXT("Pursuit save state should preserve escape destination usage."), SaveState.bTimedObjectiveUsesEscapeDestination);
	TestTrue(TEXT("Pursuit save state should preserve an origin transform for recovery."), SaveState.bHasTimedObjectiveOriginTransform);

	RestoredActor->ImportObjectiveSaveState(SaveState, InstigatorActor);
	const FHorrorCampaignObjectiveRuntimeState RestoredRuntimeState = RestoredActor->BuildObjectiveRuntimeState();
	TestEqual(TEXT("Restored pursuit should return to timed active runtime state."), RestoredRuntimeState.Status, EHorrorCampaignObjectiveRuntimeStatus::TimedPursuitActive);
	TestTrue(TEXT("Restored pursuit should expose escape destination usage."), RestoredRuntimeState.bUsesEscapeDestination);
	TestEqual(TEXT("Restored pursuit should preserve remaining seconds."), RestoredRuntimeState.TimedRemainingSeconds, SaveState.TimedObjectiveRemainingSeconds);
	TestEqual(TEXT("Restored pursuit should preserve duration seconds."), RestoredRuntimeState.TimedDurationSeconds, SaveState.TimedObjectiveDurationSeconds);
	TestEqual(TEXT("Restored pursuit should move the objective marker back to the saved escape destination."), RestoredActor->GetActorLocation(), SaveState.TimedObjectiveEscapeDestinationLocation);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveRuntimeSaveStateFailedRetryableRoundTripTest,
	"HorrorProject.Game.Campaign.ObjectiveRuntimeSaveState.FailedRetryableRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignRelicCarryRequiresReturnAnchorBeforeCompletionTest,
	"HorrorProject.Game.Campaign.RelicCarryRequiresReturnAnchorBeforeCompletion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignRelicCarryRuntimeStateSurfacesDeliveryObjectiveTest,
	"HorrorProject.Game.Campaign.RelicCarryRuntimeStateSurfacesDeliveryObjective",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignRelicCarrySaveRestorePreservesCarriedStateTest,
	"HorrorProject.Game.Campaign.RelicCarrySaveRestorePreservesCarriedState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignTempleKeySigilUsesCarryReturnGameplayTest,
	"HorrorProject.Game.Campaign.TempleKeySigilUsesCarryReturnGameplay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveRuntimeSaveStateFailedRetryableRoundTripTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for retryable failure runtime save coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* SourceActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	AHorrorCampaignObjectiveActor* RestoredActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Retryable failure save test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Retryable failure save test should spawn a source objective actor."), SourceActor);
	TestNotNull(TEXT("Retryable failure save test should spawn a restored objective actor."), RestoredActor);
	if (!InstigatorActor || !SourceActor || !RestoredActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.RuntimeSaveRetry");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::RestorePower;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::CircuitWiring;
	Objective.PromptText = FText::FromString(TEXT("保存中的失败电路"));
	Objective.CompletionText = FText::FromString(TEXT("失败电路恢复。"));

	SourceActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);
	RestoredActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

	const FHitResult Hit;
	TestTrue(TEXT("Source retry objective should open its advanced panel."), SourceActor->Interact_Implementation(InstigatorActor, Hit));
	for (int32 FailureIndex = 0; FailureIndex < 6 && SourceActor->IsAdvancedInteractionActive(); ++FailureIndex)
	{
		TestTrue(TEXT("Wrong input should be accepted while driving the objective into a retryable failure."), SourceActor->SubmitAdvancedInteractionInput(TEXT("错误端子"), InstigatorActor));
	}
	const FHorrorCampaignObjectiveRuntimeState FailedRuntimeState = SourceActor->BuildObjectiveRuntimeState();
	TestEqual(TEXT("Source objective should reach failed retryable state before saving."), FailedRuntimeState.Status, EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable);

	const FHorrorCampaignObjectiveSaveState SaveState = SourceActor->ExportObjectiveSaveState();
	TestTrue(TEXT("Retryable failure save state should mark itself as runtime state."), SaveState.HasRuntimeState());
	TestTrue(TEXT("Retryable failure save state should preserve retryable flag."), SaveState.bObjectiveFailedRetryable);
	TestEqual(TEXT("Retryable failure save state should preserve failure cause."), SaveState.LastObjectiveFailureCause, FName(TEXT("Failure.Campaign.CircuitOverload")));
	TestEqual(TEXT("Retryable failure save state should preserve recovery action."), SaveState.LastObjectiveRecoveryAction, FName(TEXT("Recovery.Objective.Retry")));
	TestTrue(TEXT("Retryable failure save state should preserve attempt count."), SaveState.ObjectiveFailureAttemptCount >= 1);

	RestoredActor->ImportObjectiveSaveState(SaveState, InstigatorActor);
	const FHorrorCampaignObjectiveRuntimeState RestoredRuntimeState = RestoredActor->BuildObjectiveRuntimeState();
	TestEqual(TEXT("Restored objective should remain failed retryable."), RestoredRuntimeState.Status, EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable);
	TestTrue(TEXT("Restored retryable failure should expose retry affordance."), RestoredRuntimeState.bRetryable);
	TestEqual(TEXT("Restored retryable failure should preserve failure cause."), RestoredRuntimeState.FailureCause, SaveState.LastObjectiveFailureCause);
	TestEqual(TEXT("Restored retryable failure should preserve recovery action."), RestoredRuntimeState.RecoveryAction, SaveState.LastObjectiveRecoveryAction);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

bool FHorrorCampaignRelicCarryRequiresReturnAnchorBeforeCompletionTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for relic carry objective coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector(100.0f, 200.0f, 90.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Relic carry test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Relic carry test should spawn an objective actor."), ObjectiveActor);
	if (!InstigatorActor || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.RelicCarry");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::RecoverRelic;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::MultiStep;
	Objective.PromptText = FText::FromString(TEXT("搬运测试遗物"));
	Objective.CompletionText = FText::FromString(TEXT("测试遗物已归档。"));
	Objective.EscapeDestinationOffset = FVector(900.0f, 0.0f, 0.0f);

	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);
	TestEqual(TEXT("Carry-return relics should require pickup and delivery interactions."), ObjectiveActor->GetRequiredInteractionCount(), 2);
	TestFalse(TEXT("Relic should begin outside the carried state."), ObjectiveActor->IsRecoverRelicAwaitingDeliveryForTests());

	const FHitResult EmptyHit;
	TestTrue(TEXT("First interaction should pick up the relic instead of completing it."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	TestFalse(TEXT("Picking up a relic should not complete the objective yet."), ObjectiveActor->IsCompleted());
	TestTrue(TEXT("Relic should enter the awaiting-delivery state after pickup."), ObjectiveActor->IsRecoverRelicAwaitingDeliveryForTests());
	TestEqual(TEXT("Relic pickup should count as the first carry step."), ObjectiveActor->GetInteractionProgressCount(), 1);
	TestTrue(TEXT("Relic objective marker should move to the return anchor after pickup."), ObjectiveActor->GetActorLocation().X > 850.0f);
	TestTrue(TEXT("Delivery prompt should tell the player to return the relic."), ObjectiveActor->GetInteractionPromptText().ToString().Contains(TEXT("送回")));

	TestTrue(TEXT("Second interaction at the anchor should complete the relic objective."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	TestTrue(TEXT("Delivered relic should complete the objective."), ObjectiveActor->IsCompleted());
	TestFalse(TEXT("Delivered relic should clear the awaiting-delivery state."), ObjectiveActor->IsRecoverRelicAwaitingDeliveryForTests());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

bool FHorrorCampaignRelicCarryRuntimeStateSurfacesDeliveryObjectiveTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for relic carry runtime state coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	TestNotNull(TEXT("Relic carry runtime test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Relic carry runtime test should spawn an objective actor."), ObjectiveActor);
	if (!InstigatorActor || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.RelicCarryRuntime");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::RecoverRelic;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::MultiStep;
	Objective.PromptText = FText::FromString(TEXT("带回测试钥印"));
	Objective.CompletionText = FText::FromString(TEXT("测试钥印已归档。"));
	Objective.EscapeDestinationOffset = FVector(700.0f, 300.0f, 0.0f);

	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);
	TestTrue(TEXT("Pickup interaction should start relic delivery."), ObjectiveActor->Interact_Implementation(InstigatorActor, FHitResult()));
	const FHorrorCampaignObjectiveRuntimeState RuntimeState = ObjectiveActor->BuildObjectiveRuntimeState();
	TestEqual(TEXT("Relic delivery should remain an available objective state."), RuntimeState.Status, EHorrorCampaignObjectiveRuntimeStatus::CarryingRelic);
	TestTrue(TEXT("Relic delivery runtime should expose the carry flag."), RuntimeState.bRecoverRelicAwaitingDelivery);
	TestTrue(TEXT("Relic delivery phase should mention the return anchor."), RuntimeState.PhaseText.ToString().Contains(TEXT("送回")));
	TestTrue(TEXT("Relic delivery diagnostics should say the relic has been lifted."), RuntimeState.DeviceStatusLabel.ToString().Contains(TEXT("已取出")));
	TestTrue(TEXT("Relic delivery next action should guide the player back to the anchor."), RuntimeState.NextActionLabel.ToString().Contains(TEXT("锚点")));
	TestTrue(TEXT("Relic delivery should expose anchor distance telemetry."), RuntimeState.RelicDeliveryDistanceMeters > 0.0f);
	TestFalse(TEXT("Relic delivery runtime text should remain localized."), ContainsLatinLetter(RuntimeState.PhaseText.ToString() + RuntimeState.DeviceStatusLabel.ToString() + RuntimeState.NextActionLabel.ToString()));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

bool FHorrorCampaignRelicCarrySaveRestorePreservesCarriedStateTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for relic carry save coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* SourceActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector(50.0f, 25.0f, 90.0f), FRotator::ZeroRotator);
	AHorrorCampaignObjectiveActor* RestoredActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	TestNotNull(TEXT("Relic carry save test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Relic carry save test should spawn a source actor."), SourceActor);
	TestNotNull(TEXT("Relic carry save test should spawn a restored actor."), RestoredActor);
	if (!InstigatorActor || !SourceActor || !RestoredActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.RelicCarrySave");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::RecoverRelic;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::MultiStep;
	Objective.PromptText = FText::FromString(TEXT("保存中的搬运遗物"));
	Objective.CompletionText = FText::FromString(TEXT("搬运遗物已归档。"));
	Objective.EscapeDestinationOffset = FVector(640.0f, 160.0f, 0.0f);

	SourceActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);
	RestoredActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);
	TestTrue(TEXT("Source relic should be picked up before saving."), SourceActor->Interact_Implementation(InstigatorActor, FHitResult()));

	const FHorrorCampaignObjectiveSaveState SaveState = SourceActor->ExportObjectiveSaveState();
	TestTrue(TEXT("Relic carry save state should mark itself as runtime state."), SaveState.HasRuntimeState());
	TestTrue(TEXT("Relic carry save state should preserve the awaiting-delivery flag."), SaveState.bRecoverRelicAwaitingDelivery);
	TestEqual(TEXT("Relic carry save state should preserve pickup progress."), SaveState.InteractionProgressCount, 1);

	RestoredActor->ImportObjectiveSaveState(SaveState, InstigatorActor);
	const FHorrorCampaignObjectiveRuntimeState RestoredRuntimeState = RestoredActor->BuildObjectiveRuntimeState();
	TestEqual(TEXT("Restored relic should still wait for delivery."), RestoredRuntimeState.Status, EHorrorCampaignObjectiveRuntimeStatus::CarryingRelic);
	TestTrue(TEXT("Restored relic should preserve carry flag."), RestoredActor->IsRecoverRelicAwaitingDeliveryForTests());
	TestEqual(TEXT("Restored relic actor should move back to the saved delivery anchor."), RestoredActor->GetActorLocation(), SaveState.RecoverRelicDeliveryLocation);
	TestTrue(TEXT("Restored relic should still complete after the delivery interaction."), RestoredActor->Interact_Implementation(InstigatorActor, FHitResult()));
	TestTrue(TEXT("Restored relic should complete after delivery."), RestoredActor->IsCompleted());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

bool FHorrorCampaignTempleKeySigilUsesCarryReturnGameplayTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* TempleChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.DungeonTemple"));
	TestNotNull(TEXT("Temple chapter should exist for key-sigil gameplay coverage."), TempleChapter);
	if (!TempleChapter)
	{
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* KeySigilObjective = FHorrorCampaign::FindObjectiveById(*TempleChapter, TEXT("Temple.RecoverKeySigil"));
	TestNotNull(TEXT("Temple key sigil objective should exist."), KeySigilObjective);
	if (!KeySigilObjective)
	{
		return false;
	}

	TestEqual(TEXT("Temple key sigil should be authored as a relic recovery objective."), KeySigilObjective->ObjectiveType, EHorrorCampaignObjectiveType::RecoverRelic);
	TestEqual(TEXT("Temple key sigil should use carry-return gameplay instead of another panel minigame."), KeySigilObjective->InteractionMode, EHorrorCampaignInteractionMode::MultiStep);
	TestTrue(TEXT("Temple key sigil should have a delivery anchor offset."), KeySigilObjective->EscapeDestinationOffset.Size2D() >= 100.0f);
	TestFalse(TEXT("Temple key sigil should not open an interaction panel."), KeySigilObjective->Presentation.bOpensInteractionPanel);
	TestTrue(TEXT("Temple key sigil presentation should describe carrying the sigil back."), KeySigilObjective->Presentation.MechanicLabel.ToString().Contains(TEXT("搬运")));
	TestFalse(TEXT("Temple key sigil presentation should remain localized."), ContainsLatinLetter(KeySigilObjective->Presentation.MechanicLabel.ToString() + KeySigilObjective->Presentation.InputHint.ToString()));

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

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	TestNotNull(TEXT("Circuit wiring should publish audio feedback events through the event bus."), EventBus);
	if (!EventBus)
	{
		TestWorld.DestroyTestWorld(false);
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
	TestFalse(TEXT("Circuit wiring HUD state should describe the current device phase."), OpenCircuitPanel.PhaseText.IsEmpty());
	TestTrue(TEXT("Circuit wiring HUD state should expose a real timing window start."), OpenCircuitPanel.TimingWindowStartFraction > 0.0f);
	TestTrue(TEXT("Circuit wiring HUD state should expose a real timing window end."), OpenCircuitPanel.TimingWindowEndFraction > OpenCircuitPanel.TimingWindowStartFraction);
	TestEqual(TEXT("Circuit wiring should start with full device stability."), OpenCircuitPanel.StabilityFraction, 1.0f);
	TestEqual(TEXT("Circuit wiring should start without mistakes."), OpenCircuitPanel.MistakeCount, 0);
	TestEqual(TEXT("Circuit wiring should start without combo progress."), OpenCircuitPanel.ComboCount, 0);
	TestTrue(TEXT("Circuit wiring should expose input precision telemetry for animated sparks."), OpenCircuitPanel.InputPrecisionFraction >= 0.0f && OpenCircuitPanel.InputPrecisionFraction <= 1.0f);
	TestTrue(TEXT("Circuit wiring should expose device load telemetry for overload effects."), OpenCircuitPanel.DeviceLoadFraction >= 0.0f && OpenCircuitPanel.DeviceLoadFraction <= 1.0f);
	TestEqual(TEXT("Circuit wiring should publish the active board slot for the animated terminal focus."), OpenCircuitPanel.ActiveInputSlotIndex, 0);
	TestEqual(TEXT("Circuit wiring board flow should start at zero before any safe line is connected."), OpenCircuitPanel.RouteFlowFraction, 0.0f);
	TestTrue(TEXT("Circuit wiring should publish high hazard pressure from the leaking red terminal."), OpenCircuitPanel.HazardPressureFraction > 0.8f);
	TestTrue(TEXT("Circuit wiring should publish target alignment for the rotating timing cursor."), OpenCircuitPanel.TargetAlignmentFraction >= 0.0f && OpenCircuitPanel.TargetAlignmentFraction <= 1.0f);
	TestEqual(TEXT("Circuit wiring should expose a staged operation track for the animated panel."), OpenCircuitPanel.StepTrack.Num(), 3);
	if (OpenCircuitPanel.StepTrack.Num() >= 3)
	{
		TestEqual(TEXT("The first circuit track node should be the active blue terminal."), OpenCircuitPanel.StepTrack[0].InputId, FName(TEXT("蓝色端子")));
		TestEqual(TEXT("The first circuit track node should use the same key as the blue terminal input option."), OpenCircuitPanel.StepTrack[0].KeyHint.ToString(), FString(TEXT("A")));
		TestTrue(TEXT("The first circuit track node should be active before the player connects it."), OpenCircuitPanel.StepTrack[0].bActive);
		TestFalse(TEXT("The first circuit track node should not be complete before input."), OpenCircuitPanel.StepTrack[0].bComplete);
		TestFalse(TEXT("Safe circuit track nodes should not be marked hazardous."), OpenCircuitPanel.StepTrack[0].bHazardous);
		TestEqual(TEXT("The second circuit track node should preview the next safe yellow terminal."), OpenCircuitPanel.StepTrack[1].InputId, FName(TEXT("黄色端子")));
		TestTrue(TEXT("Future circuit track nodes should render as preview nodes."), OpenCircuitPanel.StepTrack[1].bPreview);
		TestTrue(TEXT("Circuit track nodes should carry non-zero visual colors for UMG animation."), OpenCircuitPanel.StepTrack[0].VisualColor.A > 0.0f);
	}

	const FName ExpectedFirstInput = ObjectiveActor->GetExpectedAdvancedInputId();
	TestTrue(TEXT("Circuit wiring should expose an expected input id."), !ExpectedFirstInput.IsNone());
	TestTrue(TEXT("Premature wire input should be treated as a timing failure event."), ObjectiveActor->SubmitAdvancedInteractionInput(ExpectedFirstInput, InstigatorActor));
	TestTrue(TEXT("Premature wire input should warn the player about the timing window."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("时机")));
	TestEqual(TEXT("Premature wire input should not advance circuit progress."), ObjectiveActor->GetAdvancedInteractionProgressFraction(), 0.0f);
	const FHorrorAdvancedInteractionHUDState CircuitFailurePanel = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestEqual(TEXT("Circuit timing failures should increment the mistake count."), CircuitFailurePanel.MistakeCount, 1);
	TestEqual(TEXT("Circuit timing failures should reset the combo count."), CircuitFailurePanel.ComboCount, 0);
	TestTrue(TEXT("Circuit timing failures should reduce device stability."), CircuitFailurePanel.StabilityFraction < OpenCircuitPanel.StabilityFraction);
	TestTrue(TEXT("Circuit timing failures should mark the last result as failure."), CircuitFailurePanel.bRecentFailure);
	TestTrue(TEXT("Circuit timing failures should increase device load telemetry."), CircuitFailurePanel.DeviceLoadFraction > OpenCircuitPanel.DeviceLoadFraction);
	const FGameplayTag CircuitFailureTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Circuit.Failure")), false);
	TestTrue(TEXT("Premature wire input should publish circuit failure audio feedback."), EventBus->GetHistory().ContainsByPredicate(
		[CircuitFailureTag](const FHorrorEventMessage& Message)
		{
			return Message.EventTag == CircuitFailureTag;
		}));

	ObjectiveActor->Tick(0.65f);
	TestTrue(TEXT("Wrong wire should be accepted as a failure event."), ObjectiveActor->SubmitAdvancedInteractionInput(TEXT("错误端子"), InstigatorActor));
	TestTrue(TEXT("Wrong wire should create red-spark feedback."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("红色火花")));
	TestTrue(TEXT("Wrong wire should keep progress below completion."), ObjectiveActor->GetAdvancedInteractionProgressFraction() < 0.5f);

	ObjectiveActor->Tick(0.65f);
	TestTrue(TEXT("Leaking circuit terminals should produce a short-circuit load warning instead of a generic miss."), ObjectiveActor->SubmitAdvancedInteractionInput(TEXT("红色端子"), InstigatorActor));
	TestTrue(TEXT("Leaking terminal feedback should warn about a short circuit."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("短路")));
	TestTrue(TEXT("Leaking terminal feedback should mention load so players learn to read the device UI."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("负载")));
	TestTrue(TEXT("Leaking terminal should not advance circuit progress."), ObjectiveActor->GetAdvancedInteractionProgressFraction() < 0.5f);

	ObjectiveActor->Tick(0.65f);
	TestTrue(TEXT("Correct first wire should advance progress."), ObjectiveActor->SubmitAdvancedInteractionInput(ExpectedFirstInput, InstigatorActor));
	const float ProgressAfterFirstWire = ObjectiveActor->GetAdvancedInteractionProgressFraction();
	TestTrue(TEXT("Correct wire should advance circuit progress."), ProgressAfterFirstWire > 0.0f);
	const FHorrorAdvancedInteractionHUDState CircuitSuccessPanel = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestTrue(TEXT("Correct wire input should mark the last result as success."), CircuitSuccessPanel.bRecentSuccess);
	TestTrue(TEXT("Correct wire input should build combo progress after failures."), CircuitSuccessPanel.ComboCount > 0);
	TestTrue(TEXT("Circuit HUD should expose a localized expected input label."), CircuitSuccessPanel.ExpectedInputLabel.ToString().Contains(TEXT("端子")));
	TestTrue(TEXT("Circuit success should keep input precision telemetry in range."), CircuitSuccessPanel.InputPrecisionFraction >= 0.0f && CircuitSuccessPanel.InputPrecisionFraction <= 1.0f);
	TestTrue(TEXT("Circuit board flow should advance after the first safe line is connected."), CircuitSuccessPanel.RouteFlowFraction > OpenCircuitPanel.RouteFlowFraction);
	TestTrue(TEXT("Circuit board flow should remain bounded for the animated route meter."), CircuitSuccessPanel.RouteFlowFraction > 0.0f && CircuitSuccessPanel.RouteFlowFraction < 1.0f);
	TestEqual(TEXT("Circuit route should move the active slot to the yellow safety terminal after the first link."), CircuitSuccessPanel.ActiveInputSlotIndex, 2);
	TestEqual(TEXT("Circuit success should keep the staged operation track visible."), CircuitSuccessPanel.StepTrack.Num(), 3);
	if (CircuitSuccessPanel.StepTrack.Num() >= 3)
	{
		TestTrue(TEXT("Connected circuit track nodes should become complete."), CircuitSuccessPanel.StepTrack[0].bComplete);
		TestFalse(TEXT("Completed circuit track nodes should no longer be active."), CircuitSuccessPanel.StepTrack[0].bActive);
		TestEqual(TEXT("Circuit track should advance the active node to the yellow terminal."), CircuitSuccessPanel.StepTrack[1].InputId, CircuitSuccessPanel.ExpectedInputId);
		TestTrue(TEXT("Circuit track should flag the next safe terminal as active."), CircuitSuccessPanel.StepTrack[1].bActive);
		TestFalse(TEXT("The final circuit track node should remain a preview until reached."), CircuitSuccessPanel.StepTrack[2].bActive);
		TestTrue(TEXT("The final circuit track node should remain visible as a preview until reached."), CircuitSuccessPanel.StepTrack[2].bPreview);
	}
	TestTrue(TEXT("Circuit success panel should mark the solved blue terminal as a linked route segment."), CircuitSuccessPanel.InputOptions.ContainsByPredicate(
		[](const FHorrorAdvancedInteractionInputOption& Option)
		{
			return Option.InputId == FName(TEXT("蓝色端子")) && Option.bChainLinked && !Option.bStalled;
		}));
	TestTrue(TEXT("Circuit success panel should mark the next yellow terminal as the live stalled target."), CircuitSuccessPanel.InputOptions.ContainsByPredicate(
		[&CircuitSuccessPanel](const FHorrorAdvancedInteractionInputOption& Option)
		{
			return Option.InputId == CircuitSuccessPanel.ExpectedInputId && Option.bStalled && !Option.bChainLinked && Option.ResponseWindowFraction > 0.0f;
		}));
	TestNotEqual(
		TEXT("Circuit safe-routing sequence should never ask players to press the high-load red terminal after a clean connection."),
		ObjectiveActor->GetExpectedAdvancedInputId(),
		FName(TEXT("红色端子")));
	const FGameplayTag CircuitSuccessTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Circuit.Success")), false);
	TestTrue(TEXT("Correct wire input should publish circuit success audio feedback."), EventBus->GetHistory().ContainsByPredicate(
		[CircuitSuccessTag](const FHorrorEventMessage& Message)
		{
			return Message.EventTag == CircuitSuccessTag;
		}));

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
	FHorrorCampaignObjectiveActorTicksOnlyForRuntimeTasksTest,
	"HorrorProject.Game.Campaign.ObjectiveActorTicksOnlyForRuntimeTasks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveActorTicksOnlyForRuntimeTasksTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for campaign objective tick lifecycle coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* CircuitObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	AHorrorCampaignObjectiveActor* PursuitObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Tick lifecycle test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Tick lifecycle test should spawn a circuit objective actor."), CircuitObjectiveActor);
	TestNotNull(TEXT("Tick lifecycle test should spawn a pursuit objective actor."), PursuitObjectiveActor);
	if (!InstigatorActor || !CircuitObjectiveActor || !PursuitObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition CircuitObjective;
	CircuitObjective.ObjectiveId = TEXT("Test.TickCircuit");
	CircuitObjective.ObjectiveType = EHorrorCampaignObjectiveType::RestorePower;
	CircuitObjective.InteractionMode = EHorrorCampaignInteractionMode::CircuitWiring;
	CircuitObjective.PromptText = FText::FromString(TEXT("接入节能测试电路"));
	CircuitObjective.CompletionText = FText::FromString(TEXT("节能测试电路完成。"));
	CircuitObjective.AdvancedInteractionTuning.RequiredStepCount = 1;
	CircuitObjective.AdvancedInteractionTuning.SuccessProgress = 1.0f;
	CircuitObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), CircuitObjective);

	TestFalse(TEXT("Idle campaign objective actors should not spend a constant actor tick."), CircuitObjectiveActor->IsActorTickEnabled());

	const FHitResult EmptyHit;
	TestTrue(TEXT("Opening an advanced objective should start runtime ticking."), CircuitObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	TestTrue(TEXT("Advanced interaction objectives should tick while their animated window is active."), CircuitObjectiveActor->IsActorTickEnabled());

	CircuitObjectiveActor->Tick(0.65f);
	TestTrue(TEXT("A single-step advanced objective should accept the correct input."), CircuitObjectiveActor->SubmitAdvancedInteractionInput(CircuitObjectiveActor->GetExpectedAdvancedInputId(), InstigatorActor));
	TestTrue(TEXT("Single-step advanced objective should complete after correct input."), CircuitObjectiveActor->IsCompleted());
	TestFalse(TEXT("Completed campaign objective actors should disable runtime ticking again."), CircuitObjectiveActor->IsActorTickEnabled());

	FHorrorCampaignObjectiveDefinition PursuitObjective;
	PursuitObjective.ObjectiveId = TEXT("Test.TickPursuit");
	PursuitObjective.ObjectiveType = EHorrorCampaignObjectiveType::SurviveAmbush;
	PursuitObjective.InteractionMode = EHorrorCampaignInteractionMode::TimedPursuit;
	PursuitObjective.PromptText = FText::FromString(TEXT("触发节能追逐"));
	PursuitObjective.CompletionText = FText::FromString(TEXT("节能追逐完成。"));
	PursuitObjective.TimedObjectiveDurationSeconds = 1.0f;
	PursuitObjective.EscapeDestinationOffset = FVector(600.0f, 0.0f, 0.0f);
	PursuitObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), PursuitObjective);
	TestFalse(TEXT("Idle pursuit objective actors should also keep actor tick disabled."), PursuitObjectiveActor->IsActorTickEnabled());

	TestTrue(TEXT("Starting a timed pursuit should enable runtime ticking."), PursuitObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	TestTrue(TEXT("Timed pursuit objectives should tick while the chase timer is active."), PursuitObjectiveActor->IsActorTickEnabled());
	TestTrue(TEXT("Aborting a timed pursuit should move the objective into retryable recovery."), PursuitObjectiveActor->AbortTimedObjectiveForRecovery(
		InstigatorActor,
		FName(TEXT("Failure.Test.TickAbort")),
		FText::FromString(TEXT("追逐中止")),
		FText::FromString(TEXT("重新互动可重试。"))));
	TestFalse(TEXT("Retryable failed pursuit objectives should disable runtime ticking until retried."), PursuitObjectiveActor->IsActorTickEnabled());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveActorUsesConfiguredAdvancedInteractionTuningTest,
	"HorrorProject.Game.Campaign.ObjectiveActorUsesConfiguredAdvancedInteractionTuning",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveActorUsesConfiguredAdvancedInteractionTuningTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for advanced tuning coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	AActor* InstigatorActor = World->SpawnActor<AActor>();
	TestNotNull(TEXT("Advanced tuning test should spawn an objective actor."), ObjectiveActor);
	TestNotNull(TEXT("Advanced tuning test should spawn an instigator."), InstigatorActor);
	if (!ObjectiveActor || !InstigatorActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.ConfiguredCircuit");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::RestorePower;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::CircuitWiring;
	Objective.PromptText = FText::FromString(TEXT("调参接线任务"));
	Objective.CompletionText = FText::FromString(TEXT("调参接线完成。"));
	Objective.AdvancedInteractionTuning.RequiredStepCount = 5;
	Objective.AdvancedInteractionTuning.SuccessProgress = 0.2f;
	Objective.AdvancedInteractionTuning.CueCycleSeconds = 2.0f;
	Objective.AdvancedInteractionTuning.TimingWindowStartFraction = 0.2f;
	Objective.AdvancedInteractionTuning.TimingWindowEndFraction = 0.4f;
	Objective.AdvancedInteractionTuning.FailureProgressPenalty = 0.35f;
	Objective.AdvancedInteractionTuning.FailureStabilityDamage = 0.45f;
	Objective.AdvancedInteractionTuning.SuccessStabilityRecovery = 0.1f;
	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

	const FHitResult EmptyHit;
	TestTrue(TEXT("Configured circuit task should start advanced interaction."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	FHorrorAdvancedInteractionHUDState State = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestEqual(TEXT("HUD should expose configured required step count."), State.RequiredStepCount, 5);
	TestEqual(TEXT("HUD should expose configured timing window start."), State.TimingWindowStartFraction, 0.2f);
	TestEqual(TEXT("HUD should expose configured timing window end."), State.TimingWindowEndFraction, 0.4f);

	ObjectiveActor->Tick(0.5f);
	TestTrue(TEXT("Configured slower cycle should place the cue inside the custom timing window."), ObjectiveActor->IsAdvancedInteractionTimingWindowOpen());
	TestTrue(TEXT("Correct configured input should be accepted."), ObjectiveActor->SubmitAdvancedInteractionInput(ObjectiveActor->GetExpectedAdvancedInputId(), InstigatorActor));
	TestEqual(TEXT("Configured success progress should advance by one fifth."), ObjectiveActor->GetAdvancedInteractionProgressFraction(), 0.2f);
	TestEqual(TEXT("Configured required step count should stay at five after progress."), ObjectiveActor->GetRequiredInteractionCount(), 5);

	ObjectiveActor->Tick(0.5f);
	const FName WrongInput = ObjectiveActor->GetExpectedAdvancedInputId() == FName(TEXT("红色端子"))
		? FName(TEXT("蓝色端子"))
		: FName(TEXT("红色端子"));
	TestTrue(TEXT("Wrong configured input should be handled."), ObjectiveActor->SubmitAdvancedInteractionInput(WrongInput, InstigatorActor));
	TestEqual(TEXT("Configured failure penalty should clamp progress back to zero."), ObjectiveActor->GetAdvancedInteractionProgressFraction(), 0.0f);
	TestTrue(TEXT("Configured failure stability damage should be much harsher than default."), ObjectiveActor->GetAdvancedInteractionStabilityFraction() <= 0.65f);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignAdvancedInteractionRetryAssistTest,
	"HorrorProject.Game.Campaign.AdvancedInteractionRetryAssist",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignAdvancedInteractionRetryAssistTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for advanced interaction retry assist coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Retry assist test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Retry assist test should spawn an objective actor."), ObjectiveActor);
	if (!InstigatorActor || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.CircuitRetryAssist");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::RestorePower;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::CircuitWiring;
	Objective.PromptText = FText::FromString(TEXT("恢复协议测试电路"));
	Objective.AdvancedInteractionTuning.FailureStabilityDamage = 1.0f;
	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

	const FHitResult EmptyHit;
	TestTrue(TEXT("Retry assist objective should open its first advanced window."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	const FHorrorAdvancedInteractionHUDState FirstAttemptState = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestEqual(TEXT("First attempt should use the authored default timing window start."), FirstAttemptState.TimingWindowStartFraction, 0.36f);
	TestEqual(TEXT("First attempt should use the authored default timing window end."), FirstAttemptState.TimingWindowEndFraction, 0.72f);

	TestTrue(TEXT("A lethal first mistake should overload the device into a retryable failure."), ObjectiveActor->SubmitAdvancedInteractionInput(ObjectiveActor->GetExpectedAdvancedInputId(), InstigatorActor));
	const FHorrorCampaignObjectiveRuntimeState FailedState = ObjectiveActor->BuildObjectiveRuntimeState();
	TestEqual(TEXT("Lethal first mistake should become retryable instead of dead-ending the campaign."), FailedState.Status, EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable);

	TestTrue(TEXT("Retrying the objective should reopen the advanced window."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	FHorrorAdvancedInteractionHUDState RetryState = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestTrue(TEXT("Retry assist should widen the timing window start after an overload."), RetryState.TimingWindowStartFraction < FirstAttemptState.TimingWindowStartFraction);
	TestTrue(TEXT("Retry assist should widen the timing window end after an overload."), RetryState.TimingWindowEndFraction > FirstAttemptState.TimingWindowEndFraction);
	TestTrue(TEXT("Retry assist should tell the player a recovery protocol is active."), RetryState.DeviceStatusLabel.ToString().Contains(TEXT("恢复协议")) || RetryState.RhythmLabel.ToString().Contains(TEXT("恢复协议")));

	TestTrue(TEXT("Retry assist should soften the next harsh mistake instead of instantly failing again."), ObjectiveActor->SubmitAdvancedInteractionInput(ObjectiveActor->GetExpectedAdvancedInputId(), InstigatorActor));
	RetryState = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestTrue(TEXT("Assisted retry should keep the advanced window active after one more mistake."), ObjectiveActor->IsAdvancedInteractionActive());
	TestTrue(TEXT("Assisted retry should preserve recoverable stability after one more harsh mistake."), RetryState.StabilityFraction > 0.0f);
	TestTrue(TEXT("Assisted retry should expose overload load without forcing a retryable failure immediately."), RetryState.DeviceLoadFraction > 0.0f && RetryState.FeedbackState != EHorrorAdvancedInteractionFeedbackState::Overloaded);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveActorRunsSpectralScanWindowTest,
	"HorrorProject.Game.Campaign.ObjectiveActorRunsSpectralScanWindow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveActorRunsSpectralScanWindowTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for spectral scan coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	TestNotNull(TEXT("Spectral scan should publish dedicated audio feedback events through the event bus."), EventBus);
	if (!EventBus)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Spectral scan test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Spectral scan test should spawn an objective actor."), ObjectiveActor);
	if (!InstigatorActor || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.SpectralScan");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::ScanAnomaly;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::SpectralScan;
	Objective.PromptText = FText::FromString(TEXT("扫描黑盒里的测试心跳"));
	Objective.AdvancedInteractionTuning.RequiredStepCount = 3;
	Objective.AdvancedInteractionTuning.SuccessProgress = 0.34f;

	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);
	TestEqual(TEXT("Spectral scan objectives should expose the dedicated mode."), ObjectiveActor->GetInteractionMode(), EHorrorCampaignInteractionMode::SpectralScan);
	TestTrue(TEXT("Interacting should open the spectral scan panel."), ObjectiveActor->Interact_Implementation(InstigatorActor, FHitResult()));
	ObjectiveActor->Tick(0.9f);

	FHorrorAdvancedInteractionHUDState State = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestTrue(TEXT("Spectral scan HUD state should be visible."), State.bVisible);
	TestEqual(TEXT("Spectral scan HUD state should report the scan mode."), State.Mode, EHorrorCampaignInteractionMode::SpectralScan);
	TestEqual(TEXT("Spectral scan should expose three resonance bands."), State.InputOptions.Num(), 3);
	TestTrue(TEXT("Spectral scan phase should mention signal locking or scan alignment."), State.PhaseText.ToString().Contains(TEXT("扫描")) || State.PhaseText.ToString().Contains(TEXT("锁定")));
	TestTrue(TEXT("Spectral scan should start with a localized expected band label."), State.ExpectedInputLabel.ToString().Contains(TEXT("波段")));
	TestTrue(TEXT("Spectral scan should expose rising confidence before the first lock."), State.SpectralConfidenceFraction > 0.0f);
	TestTrue(TEXT("Spectral scan should start with an offset filter focus so the player must actively sweep."), !FMath::IsNearlyEqual(State.SpectralFilterFocusFraction, State.SpectralTargetFocusFraction, 0.02f));
	TestTrue(TEXT("Spectral scan should expose input precision telemetry for waveform UI."), State.InputPrecisionFraction >= 0.0f && State.InputPrecisionFraction <= 1.0f);
	TestTrue(TEXT("Spectral scan should expose device load telemetry for noise UI."), State.DeviceLoadFraction >= 0.0f && State.DeviceLoadFraction <= 1.0f);
	const FHorrorAdvancedInteractionInputOption* InitialTargetBand = State.InputOptions.FindByPredicate(
		[&State](const FHorrorAdvancedInteractionInputOption& Option)
		{
			return Option.InputId == State.ExpectedInputId;
		});
	const FHorrorAdvancedInteractionInputOption* InitialNoiseBand = State.InputOptions.FindByPredicate(
		[&State](const FHorrorAdvancedInteractionInputOption& Option)
		{
			return Option.InputId != State.ExpectedInputId;
		});
	TestNotNull(TEXT("Spectral scan should expose the target resonance band option."), InitialTargetBand);
	TestNotNull(TEXT("Spectral scan should expose at least one non-target noise band option."), InitialNoiseBand);
	if (InitialTargetBand && InitialNoiseBand)
	{
		TestTrue(TEXT("Target spectral band should publish stronger response telemetry than a false peak."), InitialTargetBand->ResponseWindowFraction > InitialNoiseBand->ResponseWindowFraction);
		TestTrue(TEXT("Target spectral band should expose visible waveform motion telemetry."), InitialTargetBand->MotionFraction >= 0.0f && InitialTargetBand->MotionFraction <= 1.0f);
		TestTrue(TEXT("Before filtering, at least one sweep control should animate as a corrective waveform."), InitialNoiseBand->MotionFraction > 0.0f);
		TestFalse(TEXT("Target spectral band should not be marked as a noise hazard."), InitialTargetBand->bHazardous);
	}

	for (int32 SweepIndex = 0; SweepIndex < 3; ++SweepIndex)
	{
		const FHorrorAdvancedInteractionHUDState SweepState = ObjectiveActor->BuildAdvancedInteractionHUDState();
		if (FMath::Abs(SweepState.SpectralFilterFocusFraction - SweepState.SpectralTargetFocusFraction) <= 0.16f)
		{
			break;
		}

		FHorrorAdvancedInteractionInputCommand SweepCommand;
		SweepCommand.CommandType = EHorrorAdvancedInteractionCommandType::AdjustAxis;
		SweepCommand.AxisValue = SweepState.SpectralFilterFocusFraction < SweepState.SpectralTargetFocusFraction ? 1.0f : -1.0f;
		SweepCommand.HoldSeconds = 1.2f;
		TestTrue(TEXT("Active spectral sweep should move the filter focus before locking."), ObjectiveActor->SubmitAdvancedInteractionCommand(SweepCommand, InstigatorActor));
		TestEqual(TEXT("Active spectral sweep should not advance objective progress by itself."), ObjectiveActor->GetAdvancedInteractionProgressFraction(), 0.0f);
	}
	const FHorrorAdvancedInteractionHUDState ReadyToLockState = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestTrue(TEXT("Active spectral sweeps should align the filter before the lock command."), FMath::Abs(ReadyToLockState.SpectralFilterFocusFraction - ReadyToLockState.SpectralTargetFocusFraction) <= 0.16f);
	FHorrorAdvancedInteractionInputCommand LockCommand;
	LockCommand.CommandType = EHorrorAdvancedInteractionCommandType::Confirm;
	TestTrue(TEXT("Correct spectral lock command should advance progress inside the window after active filtering."), ObjectiveActor->SubmitAdvancedInteractionCommand(LockCommand, InstigatorActor));
	TestTrue(TEXT("Spectral scan progress should advance after a correct lock."), ObjectiveActor->GetAdvancedInteractionProgressFraction() > 0.0f);
	TestTrue(TEXT("Spectral scan success feedback should mention scan locking."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("锁定")));
	FHorrorAdvancedInteractionHUDState StateAfterSuccess = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestTrue(TEXT("Spectral scan success should keep precision telemetry useful."), StateAfterSuccess.InputPrecisionFraction > 0.0f);
	const FGameplayTag SpectralScanSuccessTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SpectralScan.Success")), false);
	const FGameplayTag CircuitSuccessTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Circuit.Success")), false);
	TestTrue(TEXT("Correct spectral input should publish spectral-scan success feedback."), EventBus->GetHistory().ContainsByPredicate(
		[SpectralScanSuccessTag](const FHorrorEventMessage& Message)
		{
			return Message.EventTag == SpectralScanSuccessTag;
		}));
	TestFalse(TEXT("Spectral scan should not reuse circuit success feedback."), EventBus->GetHistory().ContainsByPredicate(
		[CircuitSuccessTag, &Objective](const FHorrorEventMessage& Message)
		{
			return Message.EventTag == CircuitSuccessTag && Message.SourceId == Objective.ObjectiveId;
		}));

	ObjectiveActor->Tick(0.9f);
	TestTrue(TEXT("Wrong spectral band should still be handled inside the scan window."), ObjectiveActor->SubmitAdvancedInteractionInput(TEXT("错误波段"), InstigatorActor));
	TestTrue(TEXT("Wrong spectral band should mention scan/filter noise, not wiring."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("扫描")) || ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("噪点")) || ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("滤波")) || ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("扫频")));
	const FHorrorAdvancedInteractionHUDState StateAfterWrongBand = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestTrue(TEXT("Wrong spectral band should raise device load telemetry."), StateAfterWrongBand.DeviceLoadFraction > StateAfterSuccess.DeviceLoadFraction);
	const FGameplayTag SpectralScanFailureTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SpectralScan.Failure")), false);
	const FGameplayTag CircuitFailureTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Circuit.Failure")), false);
	TestTrue(TEXT("Wrong spectral band should publish spectral-scan failure feedback."), EventBus->GetHistory().ContainsByPredicate(
		[SpectralScanFailureTag](const FHorrorEventMessage& Message)
		{
			return Message.EventTag == SpectralScanFailureTag;
		}));
	TestFalse(TEXT("Spectral scan should not reuse circuit failure feedback."), EventBus->GetHistory().ContainsByPredicate(
		[CircuitFailureTag, &Objective](const FHorrorEventMessage& Message)
		{
			return Message.EventTag == CircuitFailureTag && Message.SourceId == Objective.ObjectiveId;
		}));

	AHorrorCampaignObjectiveActor* LowConfidenceObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Spectral scan low-confidence test should spawn an objective actor."), LowConfidenceObjectiveActor);
	if (!LowConfidenceObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition LowConfidenceObjective = Objective;
	LowConfidenceObjective.ObjectiveId = TEXT("Test.SpectralScanLowConfidence");
	LowConfidenceObjective.AdvancedInteractionTuning.TimingWindowStartFraction = 0.05f;
	LowConfidenceObjective.AdvancedInteractionTuning.TimingWindowEndFraction = 0.35f;
	LowConfidenceObjective.AdvancedInteractionTuning.FailureStabilityDamage = 0.35f;
	LowConfidenceObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), LowConfidenceObjective);
	TestTrue(TEXT("Low-confidence spectral scan should open the scan panel."), LowConfidenceObjectiveActor->Interact_Implementation(InstigatorActor, FHitResult()));
	LowConfidenceObjectiveActor->Tick(0.12f);
	const FHorrorAdvancedInteractionHUDState LowConfidenceState = LowConfidenceObjectiveActor->BuildAdvancedInteractionHUDState();
	TestTrue(TEXT("Low-confidence spectral scan should be inside the timing band."), LowConfidenceState.bTimingWindowOpen);
	TestTrue(TEXT("Low-confidence spectral scan should expose insufficient confidence."), LowConfidenceState.SpectralConfidenceFraction < 0.5f);
	TestTrue(TEXT("Low-confidence spectral scan should expose weaker precision than a confident scan."), LowConfidenceState.InputPrecisionFraction < StateAfterSuccess.InputPrecisionFraction);
	const FHorrorAdvancedInteractionInputOption* LowConfidenceTargetBand = LowConfidenceState.InputOptions.FindByPredicate(
		[&LowConfidenceState](const FHorrorAdvancedInteractionInputOption& Option)
		{
			return Option.InputId == LowConfidenceState.ExpectedInputId;
		});
	TestNotNull(TEXT("Low-confidence spectral scan should expose its current target band."), LowConfidenceTargetBand);
	if (LowConfidenceTargetBand)
	{
		TestTrue(TEXT("Low-confidence target band should publish a weak response meter until the peak stabilizes."), LowConfidenceTargetBand->ResponseWindowFraction < 0.55f);
		TestTrue(TEXT("Low-confidence target band should animate as an unstable peak."), LowConfidenceTargetBand->MotionFraction < 0.75f);
	}
	FHorrorAdvancedInteractionInputCommand WeakPeakLockCommand;
	WeakPeakLockCommand.CommandType = EHorrorAdvancedInteractionCommandType::Confirm;
	TestTrue(TEXT("Confirming a weak spectral peak should be handled as a failure."), LowConfidenceObjectiveActor->SubmitAdvancedInteractionCommand(WeakPeakLockCommand, InstigatorActor));
	TestTrue(TEXT("Weak spectral peak feedback should mention confidence or false peaks."), LowConfidenceObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("置信度")) || LowConfidenceObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("假峰值")));
	TestTrue(TEXT("Weak spectral lock should reduce stability."), LowConfidenceObjectiveActor->GetAdvancedInteractionStabilityFraction() < 1.0f);

	AHorrorCampaignObjectiveActor* FailingScanObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Spectral scan retry guidance test should spawn an objective actor."), FailingScanObjectiveActor);
	if (!FailingScanObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition FailingScanObjective = LowConfidenceObjective;
	FailingScanObjective.ObjectiveId = TEXT("Test.SpectralScanRetryGuidance");
	FailingScanObjective.AdvancedInteractionTuning.FailureStabilityDamage = 1.0f;
	FailingScanObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), FailingScanObjective);
	TestTrue(TEXT("Failing spectral scan should open the scan panel."), FailingScanObjectiveActor->Interact_Implementation(InstigatorActor, FHitResult()));
	FailingScanObjectiveActor->Tick(0.12f);
	TestTrue(TEXT("Confirming a weak peak should overload the failing scan."), FailingScanObjectiveActor->SubmitAdvancedInteractionCommand(WeakPeakLockCommand, InstigatorActor));
	const FHorrorCampaignObjectiveRuntimeState FailedScanState = FailingScanObjectiveActor->BuildObjectiveRuntimeState();
	TestEqual(TEXT("Spectral scan overload should be retryable."), FailedScanState.Status, EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable);
	TestEqual(TEXT("Spectral scan overload should preserve the spectral failure cause."), FailedScanState.FailureCause, FName(TEXT("Failure.Campaign.SpectralScanOverload")));
	TestTrue(TEXT("Spectral scan overload should keep the animated HUD panel visible."), FailedScanState.AdvancedInteraction.bVisible);
	TestEqual(TEXT("Spectral scan overload panel should stay in scan mode."), FailedScanState.AdvancedInteraction.Mode, EHorrorCampaignInteractionMode::SpectralScan);
	TestEqual(TEXT("Spectral scan overload panel should report overloaded feedback."), FailedScanState.AdvancedInteraction.FeedbackState, EHorrorAdvancedInteractionFeedbackState::Overloaded);
	TestTrue(TEXT("Spectral scan overload should explain retry in the phase text."), FailedScanState.AdvancedInteraction.PhaseText.ToString().Contains(TEXT("重新扫描")) || FailedScanState.AdvancedInteraction.PhaseText.ToString().Contains(TEXT("重新互动")));
	TestTrue(TEXT("Spectral scan overload should tell the player how to restart."), FailedScanState.AdvancedInteraction.NextActionLabel.ToString().Contains(TEXT("重新互动")));
	TestTrue(TEXT("Spectral scan overload should preserve recovery guidance."), FailedScanState.AdvancedInteraction.FailureRecoveryLabel.ToString().Contains(TEXT("重试")) || FailedScanState.AdvancedInteraction.FailureRecoveryLabel.ToString().Contains(TEXT("重新")));
	TestFalse(TEXT("Spectral scan retry guidance should remain localized."), ContainsLatinLetter(FailedScanState.AdvancedInteraction.PhaseText.ToString() + FailedScanState.AdvancedInteraction.NextActionLabel.ToString() + FailedScanState.AdvancedInteraction.FailureRecoveryLabel.ToString()));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveActorRunsSignalTuningWindowTest,
	"HorrorProject.Game.Campaign.ObjectiveActorRunsSignalTuningWindow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveSpectralScanRequiresActiveFilterSweepTest,
	"HorrorProject.Game.Campaign.ObjectiveSpectralScanRequiresActiveFilterSweep",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveSpectralScanRequiresActiveFilterSweepTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for active spectral filter coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Active spectral filter test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Active spectral filter test should spawn an objective actor."), ObjectiveActor);
	if (!InstigatorActor || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.SpectralActiveFilter");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::ScanAnomaly;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::SpectralScan;
	Objective.PromptText = FText::FromString(TEXT("主动扫频测试异常"));
	Objective.AdvancedInteractionTuning.RequiredStepCount = 3;
	Objective.AdvancedInteractionTuning.SuccessProgress = 0.34f;
	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

	const FHitResult EmptyHit;
	TestTrue(TEXT("Spectral objective should open its scan panel."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	ObjectiveActor->Tick(0.9f);

	FHorrorAdvancedInteractionInputCommand PrematureLockCommand;
	PrematureLockCommand.CommandType = EHorrorAdvancedInteractionCommandType::Confirm;
	TestTrue(TEXT("Direct spectral lock should be accepted by the command router."), ObjectiveActor->SubmitAdvancedInteractionCommand(PrematureLockCommand, InstigatorActor));
	TestEqual(TEXT("Direct spectral lock should not advance before the player sweeps the filter onto the peak."), ObjectiveActor->GetAdvancedInteractionProgressFraction(), 0.0f);
	TestTrue(
		TEXT("Direct spectral lock feedback should explain active filter alignment."),
		ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("滤波"))
			|| ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("扫频"))
			|| ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("焦点")));

	for (int32 SweepIndex = 0; SweepIndex < 3; ++SweepIndex)
	{
		const FHorrorAdvancedInteractionHUDState ScanState = ObjectiveActor->BuildAdvancedInteractionHUDState();
		if (FMath::Abs(ScanState.SpectralFilterFocusFraction - ScanState.SpectralTargetFocusFraction) <= 0.16f)
		{
			break;
		}

		FHorrorAdvancedInteractionInputCommand SweepCommand;
		SweepCommand.CommandType = EHorrorAdvancedInteractionCommandType::AdjustAxis;
		SweepCommand.AxisValue = ScanState.SpectralFilterFocusFraction < ScanState.SpectralTargetFocusFraction ? 1.0f : -1.0f;
		SweepCommand.HoldSeconds = 1.2f;
		TestTrue(TEXT("Spectral scan should accept active sweep commands."), ObjectiveActor->SubmitAdvancedInteractionCommand(SweepCommand, InstigatorActor));
		TestEqual(TEXT("Active spectral sweep should tune the filter without advancing objective progress."), ObjectiveActor->GetAdvancedInteractionProgressFraction(), 0.0f);
		TestTrue(
			TEXT("Active spectral sweep feedback should describe manual filtering."),
			ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("扫频"))
				|| ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("滤波")));
	}

	const FHorrorAdvancedInteractionHUDState FocusedScanState = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestTrue(TEXT("Repeated active sweeps should align the spectral filter before locking."), FMath::Abs(FocusedScanState.SpectralFilterFocusFraction - FocusedScanState.SpectralTargetFocusFraction) <= 0.16f);

	ObjectiveActor->Tick(0.65f);
	FHorrorAdvancedInteractionInputCommand LockCommand;
	LockCommand.CommandType = EHorrorAdvancedInteractionCommandType::Confirm;
	TestTrue(TEXT("Spectral lock should submit once the filter is swept onto the target peak."), ObjectiveActor->SubmitAdvancedInteractionCommand(LockCommand, InstigatorActor));
	TestTrue(TEXT("Spectral lock should advance after active filtering."), ObjectiveActor->GetAdvancedInteractionProgressFraction() > 0.0f);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

bool FHorrorCampaignObjectiveActorRunsSignalTuningWindowTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for signal tuning coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	TestNotNull(TEXT("Signal tuning should publish dedicated audio feedback events through the event bus."), EventBus);
	if (!EventBus)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Signal tuning test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Signal tuning test should spawn an objective actor."), ObjectiveActor);
	if (!InstigatorActor || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.SignalTuning");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::AcquireSignal;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::SignalTuning;
	Objective.PromptText = FText::FromString(TEXT("调谐测试黑盒回放"));
	Objective.AdvancedInteractionTuning.RequiredStepCount = 3;
	Objective.AdvancedInteractionTuning.SuccessProgress = 0.34f;

	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);
	TestEqual(TEXT("Signal tuning objectives should expose the dedicated mode."), ObjectiveActor->GetInteractionMode(), EHorrorCampaignInteractionMode::SignalTuning);
	TestTrue(TEXT("Interacting should open the signal tuning panel."), ObjectiveActor->Interact_Implementation(InstigatorActor, FHitResult()));

	FHorrorAdvancedInteractionHUDState State = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestTrue(TEXT("Signal tuning HUD state should be visible."), State.bVisible);
	TestEqual(TEXT("Signal tuning HUD state should report the tuning mode."), State.Mode, EHorrorCampaignInteractionMode::SignalTuning);
	TestEqual(TEXT("Signal tuning should expose three playback channels."), State.InputOptions.Num(), 3);
	TestTrue(TEXT("Signal tuning phase should mention tuning or synchronization."), State.PhaseText.ToString().Contains(TEXT("调谐")) || State.PhaseText.ToString().Contains(TEXT("同步")));
	TestTrue(TEXT("Signal tuning should start with a localized expected channel label."), State.ExpectedInputLabel.ToString().Contains(TEXT("声道")) || State.ExpectedInputLabel.ToString().Contains(TEXT("频率")));
	TestTrue(TEXT("Signal tuning should expose an input precision telemetry channel for animated UI feedback."), State.InputPrecisionFraction >= 0.0f && State.InputPrecisionFraction <= 1.0f);
	TestTrue(TEXT("Signal tuning should expose a device load telemetry channel for animated UI feedback."), State.DeviceLoadFraction >= 0.0f && State.DeviceLoadFraction <= 1.0f);
	TestTrue(TEXT("Signal tuning should start with a story-authored playback offset that requires calibration."), !FMath::IsNearlyEqual(State.SignalBalanceFraction, State.SignalTargetBalanceFraction, 0.02f));
	TestTrue(TEXT("Signal tuning should target a centered black-box playback image."), FMath::IsNearlyEqual(State.SignalTargetBalanceFraction, 0.5f, 0.01f));
	TestTrue(TEXT("Offset signal tuning should start with reduced precision feedback."), State.InputPrecisionFraction < 0.75f);
	TestTrue(TEXT("Offset signal tuning should start with elevated device load."), State.DeviceLoadFraction > 0.15f);

	TestEqual(TEXT("Signal tuning should start as a center-confirm mechanic, not a rotating channel prompt."), ObjectiveActor->GetExpectedAdvancedInputId(), FName(TEXT("中心频率")));
	const bool bInitialBalanceLeftOfTarget = State.SignalBalanceFraction < State.SignalTargetBalanceFraction;
	const FName CorrectiveInputId = bInitialBalanceLeftOfTarget ? FName(TEXT("右声道")) : FName(TEXT("左声道"));
	const FName NonCorrectiveInputId = bInitialBalanceLeftOfTarget ? FName(TEXT("左声道")) : FName(TEXT("右声道"));
	const FHorrorAdvancedInteractionInputOption* InitialCorrectiveNudge = State.InputOptions.FindByPredicate(
		[CorrectiveInputId](const FHorrorAdvancedInteractionInputOption& Option)
		{
			return Option.InputId == CorrectiveInputId;
		});
	const FHorrorAdvancedInteractionInputOption* InitialNonCorrectiveNudge = State.InputOptions.FindByPredicate(
		[NonCorrectiveInputId](const FHorrorAdvancedInteractionInputOption& Option)
		{
			return Option.InputId == NonCorrectiveInputId;
		});
	const FHorrorAdvancedInteractionInputOption* InitialCenterLock = State.InputOptions.FindByPredicate(
		[](const FHorrorAdvancedInteractionInputOption& Option)
		{
			return Option.InputId == FName(TEXT("中心频率"));
		});
	TestNotNull(TEXT("Signal tuning should expose the corrective channel option for the opening offset."), InitialCorrectiveNudge);
	TestNotNull(TEXT("Signal tuning should expose the non-corrective channel option for the opening offset."), InitialNonCorrectiveNudge);
	TestNotNull(TEXT("Signal tuning should expose the unsafe center lock option before calibration."), InitialCenterLock);
	if (InitialCorrectiveNudge && InitialNonCorrectiveNudge && InitialCenterLock)
	{
		TestTrue(TEXT("The corrective signal nudge should carry stronger response telemetry than the wrong nudge."), InitialCorrectiveNudge->ResponseWindowFraction > InitialNonCorrectiveNudge->ResponseWindowFraction);
		TestTrue(TEXT("The corrective signal nudge should animate with stronger motion than the wrong nudge."), InitialCorrectiveNudge->MotionFraction > InitialNonCorrectiveNudge->MotionFraction);
		TestTrue(TEXT("Unsafe center lock should be marked hazardous while signal balance is off-center."), InitialCenterLock->bHazardous);
		TestTrue(TEXT("Unsafe center lock should expose high load while signal balance is off-center."), InitialCenterLock->LoadFraction > InitialCorrectiveNudge->LoadFraction);
	}

	ObjectiveActor->Tick(0.9f);
	TestTrue(TEXT("Direct center confirm should be rejected while the opening playback is off-center."), ObjectiveActor->SubmitAdvancedInteractionInput(TEXT("中心频率"), InstigatorActor));
	TestEqual(TEXT("Off-center signal confirmation should not advance objective progress."), ObjectiveActor->GetAdvancedInteractionProgressFraction(), 0.0f);
	TestTrue(TEXT("Off-center confirmation should tell the player to correct the signal balance."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("声像")) || ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("回放")));

	TestTrue(TEXT("Corrective channel input should nudge the opening tuning balance without advancing progress."), ObjectiveActor->SubmitAdvancedInteractionInput(CorrectiveInputId, InstigatorActor));
	State = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestTrue(TEXT("HUD should expose centered tuning after the corrective nudge."), FMath::IsNearlyEqual(State.SignalBalanceFraction, State.SignalTargetBalanceFraction, 0.02f));
	TestTrue(TEXT("Signal tuning precision should recover after corrective calibration."), State.InputPrecisionFraction > 0.75f);
	const FHorrorAdvancedInteractionInputOption* CenteredLock = State.InputOptions.FindByPredicate(
		[](const FHorrorAdvancedInteractionInputOption& Option)
		{
			return Option.InputId == FName(TEXT("中心频率"));
		});
	TestNotNull(TEXT("Centered signal tuning should expose the center lock option."), CenteredLock);
	if (CenteredLock)
	{
		TestFalse(TEXT("Centered signal tuning should clear the unsafe center-lock hazard."), CenteredLock->bHazardous);
		TestTrue(TEXT("Centered signal tuning should expose a strong lock response meter."), CenteredLock->ResponseWindowFraction > 0.85f);
	}

	TestTrue(TEXT("Premature center confirm should be handled as a timing failure."), ObjectiveActor->SubmitAdvancedInteractionInput(TEXT("中心频率"), InstigatorActor));
	TestTrue(TEXT("Premature tuning input should mention tuning or playback, not wiring."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("调谐")) || ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("回放")));
	State = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestTrue(TEXT("Signal tuning device load should spike after a timing failure."), State.DeviceLoadFraction > 0.35f);
	const FGameplayTag SignalTuningFailureTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SignalTuning.Failure")), false);
	const FGameplayTag CircuitFailureTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Circuit.Failure")), false);
	TestTrue(TEXT("Premature tuning input should publish signal-tuning failure feedback."), EventBus->GetHistory().ContainsByPredicate(
		[SignalTuningFailureTag](const FHorrorEventMessage& Message)
		{
			return Message.EventTag == SignalTuningFailureTag;
		}));
	TestFalse(TEXT("Signal tuning should not reuse circuit failure feedback."), EventBus->GetHistory().ContainsByPredicate(
		[CircuitFailureTag, &Objective](const FHorrorEventMessage& Message)
		{
			return Message.EventTag == CircuitFailureTag && Message.SourceId == Objective.ObjectiveId;
		}));

	ObjectiveActor->Tick(0.9f);
	TestTrue(TEXT("Center confirm should advance progress inside the synchronized window."), ObjectiveActor->SubmitAdvancedInteractionInput(TEXT("中心频率"), InstigatorActor));
	TestTrue(TEXT("Signal tuning progress should advance after a correct alignment."), ObjectiveActor->GetAdvancedInteractionProgressFraction() > 0.0f);
	TestTrue(TEXT("Signal tuning success feedback should mention frequency alignment."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("频率")) || ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("对齐")));
	const FGameplayTag SignalTuningSuccessTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SignalTuning.Success")), false);
	const FGameplayTag CircuitSuccessTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Circuit.Success")), false);
	TestTrue(TEXT("Correct tuning input should publish signal-tuning success feedback."), EventBus->GetHistory().ContainsByPredicate(
		[SignalTuningSuccessTag](const FHorrorEventMessage& Message)
		{
			return Message.EventTag == SignalTuningSuccessTag;
		}));
	TestFalse(TEXT("Signal tuning should not reuse circuit success feedback."), EventBus->GetHistory().ContainsByPredicate(
		[CircuitSuccessTag, &Objective](const FHorrorEventMessage& Message)
		{
			return Message.EventTag == CircuitSuccessTag && Message.SourceId == Objective.ObjectiveId;
		}));

	AHorrorCampaignObjectiveActor* FailingObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Signal tuning failure-cause test should spawn an objective actor."), FailingObjectiveActor);
	if (!FailingObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition FailingObjective = Objective;
	FailingObjective.ObjectiveId = TEXT("Test.SignalTuningFailure");
	FailingObjective.AdvancedInteractionTuning.FailureStabilityDamage = 1.0f;
	FailingObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), FailingObjective);
	TestTrue(TEXT("Failure-cause tuning objective should open the tuning panel."), FailingObjectiveActor->Interact_Implementation(InstigatorActor, FHitResult()));
	FailingObjectiveActor->Tick(0.9f);
	const FHorrorAdvancedInteractionHUDState FailingInitialState = FailingObjectiveActor->BuildAdvancedInteractionHUDState();
	const FName FailingWrongInputId = FailingInitialState.SignalBalanceFraction < FailingInitialState.SignalTargetBalanceFraction
		? FName(TEXT("左声道"))
		: FName(TEXT("右声道"));
	TestTrue(TEXT("Repeated wrong tuning should move the playback outside the lock band."), FailingObjectiveActor->SubmitAdvancedInteractionInput(FailingWrongInputId, InstigatorActor));
	TestTrue(TEXT("Confirming outside the lock band should drive the objective into a retryable failure."), FailingObjectiveActor->SubmitAdvancedInteractionInput(TEXT("中心频率"), InstigatorActor));
	const FHorrorCampaignObjectiveRuntimeState FailedRuntimeState = FailingObjectiveActor->BuildObjectiveRuntimeState();
	TestEqual(TEXT("Failed tuning runtime state should preserve the signal tuning overload cause."), FailedRuntimeState.FailureCause, FName(TEXT("Failure.Campaign.SignalTuningOverload")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveSignalTuningAcceptsContinuousCommandsTest,
	"HorrorProject.Game.Campaign.ObjectiveSignalTuningAcceptsContinuousCommands",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveSignalTuningAcceptsContinuousCommandsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for continuous signal tuning coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Continuous tuning test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Continuous tuning test should spawn a signal objective actor."), ObjectiveActor);
	if (!InstigatorActor || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.SignalContinuous");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::AcquireSignal;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::SignalTuning;
	Objective.PromptText = FText::FromString(TEXT("连续调谐黑盒回放"));
	Objective.CompletionText = FText::FromString(TEXT("黑盒回放锁定。"));
	Objective.AdvancedInteractionTuning.RequiredStepCount = 3;
	Objective.AdvancedInteractionTuning.SuccessProgress = 0.34f;
	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

	const FHitResult EmptyHit;
	TestTrue(TEXT("Signal objective should open its advanced panel."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));

	FHorrorAdvancedInteractionHUDState InitialState = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestTrue(TEXT("Initial signal balance should start offset so continuous commands are required."), !FMath::IsNearlyEqual(InitialState.SignalBalanceFraction, InitialState.SignalTargetBalanceFraction, 0.02f));

	ObjectiveActor->Tick(0.65f);
	FHorrorAdvancedInteractionInputCommand PrematureConfirmCommand;
	PrematureConfirmCommand.CommandType = EHorrorAdvancedInteractionCommandType::Confirm;
	PrematureConfirmCommand.InputId = TEXT("中心频率");
	TestTrue(TEXT("Confirm command should be accepted but rejected while signal is off-center."), ObjectiveActor->SubmitAdvancedInteractionCommand(PrematureConfirmCommand, InstigatorActor));
	TestEqual(TEXT("Off-center confirm should not advance continuous tuning progress."), ObjectiveActor->GetAdvancedInteractionProgressFraction(), 0.0f);
	TestTrue(TEXT("Off-center confirm feedback should mention balance correction."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("声像")));

	FHorrorAdvancedInteractionInputCommand CorrectionCommand;
	CorrectionCommand.CommandType = EHorrorAdvancedInteractionCommandType::AdjustAxis;
	CorrectionCommand.AxisValue = InitialState.SignalBalanceFraction < InitialState.SignalTargetBalanceFraction ? 1.0f : -1.0f;
	CorrectionCommand.HoldSeconds = 1.0f;
	TestTrue(TEXT("Continuous corrective command should be accepted by signal tuning."), ObjectiveActor->SubmitAdvancedInteractionCommand(CorrectionCommand, InstigatorActor));
	FHorrorAdvancedInteractionHUDState CenteredState = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestTrue(TEXT("Continuous corrective command should pull balance into the center lock band."), FMath::Abs(CenteredState.SignalBalanceFraction - CenteredState.SignalTargetBalanceFraction) <= 0.12f);
	TestEqual(TEXT("Continuous axis input should not advance objective progress."), ObjectiveActor->GetAdvancedInteractionProgressFraction(), 0.0f);
	TestTrue(TEXT("Continuous axis feedback should mention fine tuning."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("微调")));

	ObjectiveActor->Tick(0.65f);
	FHorrorAdvancedInteractionInputCommand ConfirmCommand;
	ConfirmCommand.CommandType = EHorrorAdvancedInteractionCommandType::Confirm;
	ConfirmCommand.InputId = TEXT("中心频率");
	TestTrue(TEXT("Confirm command should submit the center frequency once aligned and timed."), ObjectiveActor->SubmitAdvancedInteractionCommand(ConfirmCommand, InstigatorActor));
	TestTrue(TEXT("Confirm command should advance progress after continuous tuning."), ObjectiveActor->GetAdvancedInteractionProgressFraction() > 0.0f);

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

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	TestNotNull(TEXT("Gear calibration should publish audio feedback events through the event bus."), EventBus);
	if (!EventBus)
	{
		TestWorld.DestroyTestWorld(false);
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
	TestFalse(TEXT("Gear calibration HUD state should describe the current device phase."), OpenGearPanel.PhaseText.IsEmpty());
	TestTrue(TEXT("Gear calibration HUD state should expose a real timing window start."), OpenGearPanel.TimingWindowStartFraction > 0.0f);
	TestTrue(TEXT("Gear calibration HUD state should expose a real timing window end."), OpenGearPanel.TimingWindowEndFraction > OpenGearPanel.TimingWindowStartFraction);
	TestEqual(TEXT("Gear calibration should start with full device stability."), OpenGearPanel.StabilityFraction, 1.0f);
	TestEqual(TEXT("Gear calibration should start without mistakes."), OpenGearPanel.MistakeCount, 0);
	TestEqual(TEXT("Gear calibration should start without combo progress."), OpenGearPanel.ComboCount, 0);
	TestTrue(TEXT("Gear calibration should describe the chain rhythm instead of feeling like a single button prompt."), OpenGearPanel.RhythmLabel.ToString().Contains(TEXT("连锁")));
	TestTrue(TEXT("Gear calibration next action should identify the drive gear chain."), OpenGearPanel.NextActionLabel.ToString().Contains(TEXT("驱动")));
	TestTrue(TEXT("Gear calibration should expose input precision telemetry for animated gear focus."), OpenGearPanel.InputPrecisionFraction >= 0.0f && OpenGearPanel.InputPrecisionFraction <= 1.0f);
	TestTrue(TEXT("Gear calibration should expose device load telemetry for jam effects."), OpenGearPanel.DeviceLoadFraction >= 0.0f && OpenGearPanel.DeviceLoadFraction <= 1.0f);
	TestEqual(TEXT("Gear calibration should publish the active center gear slot for mechanical focus rendering."), OpenGearPanel.ActiveInputSlotIndex, 1);
	TestEqual(TEXT("Gear chain engagement should begin at zero before any gear is linked."), OpenGearPanel.RouteFlowFraction, 0.0f);
	TestTrue(TEXT("Gear calibration should expose jam pressure from the stopped gear, not just global device load."), OpenGearPanel.HazardPressureFraction > OpenGearPanel.DeviceLoadFraction);
	TestTrue(TEXT("Gear calibration should publish mesh alignment telemetry for the timing cursor."), OpenGearPanel.TargetAlignmentFraction >= 0.0f && OpenGearPanel.TargetAlignmentFraction <= 1.0f);
	TestEqual(TEXT("Gear calibration should begin on the center drive gear instead of a rote 1-2-3 sequence."), ObjectiveActor->GetExpectedAdvancedInputId(), FName(TEXT("齿轮2")));
	const FHorrorAdvancedInteractionInputOption* OpenDriveGear = OpenGearPanel.InputOptions.FindByPredicate(
		[](const FHorrorAdvancedInteractionInputOption& Option)
		{
			return Option.InputId == FName(TEXT("齿轮2"));
		});
	const FHorrorAdvancedInteractionInputOption* OpenLeftGear = OpenGearPanel.InputOptions.FindByPredicate(
		[](const FHorrorAdvancedInteractionInputOption& Option)
		{
			return Option.InputId == FName(TEXT("齿轮1"));
		});
	TestNotNull(TEXT("Gear HUD should expose per-gear state for the stopped drive gear."), OpenDriveGear);
	TestNotNull(TEXT("Gear HUD should expose per-gear state for neighboring gears."), OpenLeftGear);
	if (OpenDriveGear && OpenLeftGear)
	{
		TestTrue(TEXT("The current gear should be marked as stalled for the animated repair window."), OpenDriveGear->bStalled);
		TestTrue(TEXT("The current gear should spin slower than neighboring gears."), OpenDriveGear->MotionFraction < OpenLeftGear->MotionFraction);
		TestTrue(TEXT("The current gear should expose a response meter for the timing window."), OpenDriveGear->ResponseWindowFraction >= 0.0f && OpenDriveGear->ResponseWindowFraction <= 1.0f);
		TestFalse(TEXT("The first stopped gear should not be marked as already linked before input."), OpenDriveGear->bChainLinked);
	}

	const FName ExpectedFirstGear = ObjectiveActor->GetExpectedAdvancedInputId();
	TestTrue(TEXT("Premature gear input should be treated as a timing failure event."), ObjectiveActor->SubmitAdvancedInteractionInput(ExpectedFirstGear, InstigatorActor));
	TestTrue(TEXT("Premature gear input should warn the player about the timing window."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("时机")));
	TestTrue(TEXT("Premature gear input should pause calibration."), ObjectiveActor->GetAdvancedInteractionPauseRemainingSeconds() > 0.0f);
	const FHorrorAdvancedInteractionHUDState GearFailurePanel = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestEqual(TEXT("Gear timing failures should increment the mistake count."), GearFailurePanel.MistakeCount, 1);
	TestEqual(TEXT("Gear timing failures should reset the combo count."), GearFailurePanel.ComboCount, 0);
	TestTrue(TEXT("Gear timing failures should reduce device stability."), GearFailurePanel.StabilityFraction < OpenGearPanel.StabilityFraction);
	TestTrue(TEXT("Gear timing failures should mark the last result as failure."), GearFailurePanel.bRecentFailure);
	TestTrue(TEXT("Gear timing failures should raise device load telemetry."), GearFailurePanel.DeviceLoadFraction > OpenGearPanel.DeviceLoadFraction);
	const FGameplayTag GearFailureTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Gear.Failure")), false);
	TestTrue(TEXT("Premature gear input should publish gear failure audio feedback."), EventBus->GetHistory().ContainsByPredicate(
		[GearFailureTag](const FHorrorEventMessage& Message)
		{
			return Message.EventTag == GearFailureTag;
		}));
	ObjectiveActor->Tick(3.0f);

	ObjectiveActor->Tick(0.65f);
	TestTrue(TEXT("Wrong gear should trigger the jam pause."), ObjectiveActor->SubmitAdvancedInteractionInput(TEXT("齿轮9"), InstigatorActor));
	TestTrue(TEXT("Wrong gear should pause calibration."), ObjectiveActor->GetAdvancedInteractionPauseRemainingSeconds() > 0.0f);
	TestTrue(TEXT("Wrong gear should create click feedback."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("咔哒")));
	TestTrue(TEXT("Wrong gear should clearly tell the player that the gear chain broke."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("连锁中断")));
	const FHorrorAdvancedInteractionHUDState GearJamPanel = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestTrue(TEXT("Gear jam should expose high device load telemetry for UI drag."), GearJamPanel.DeviceLoadFraction > OpenGearPanel.DeviceLoadFraction);
	TestTrue(TEXT("Gear jam should reduce input precision while the mechanism is paused."), GearJamPanel.InputPrecisionFraction < OpenGearPanel.InputPrecisionFraction);
	TestTrue(TEXT("Gear jam recovery should explain how to restart the chain rhythm."), GearJamPanel.FailureRecoveryLabel.ToString().Contains(TEXT("连锁")));

	const float PausedProgress = ObjectiveActor->GetAdvancedInteractionProgressFraction();
	ObjectiveActor->Tick(1.0f);
	TestEqual(TEXT("Gear progress should stay paused during a jam."), ObjectiveActor->GetAdvancedInteractionProgressFraction(), PausedProgress);
	ObjectiveActor->Tick(3.0f);
	TestEqual(TEXT("Gear pause should clear after the jam duration."), ObjectiveActor->GetAdvancedInteractionPauseRemainingSeconds(), 0.0f);

	for (int32 StepIndex = 0; StepIndex < 4 && !ObjectiveActor->IsCompleted(); ++StepIndex)
	{
		ObjectiveActor->Tick(0.65f);
		const FName ExpectedGear = ObjectiveActor->GetExpectedAdvancedInputId();
		TestTrue(
			FString::Printf(TEXT("Correct gear step should advance or complete calibration: %d."), StepIndex),
			ObjectiveActor->SubmitAdvancedInteractionInput(ExpectedGear, InstigatorActor));
		const FString GearFeedbackText = ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString();
		TestTrue(TEXT("Correct gear input should publish a chain calibration success callout."), GearFeedbackText.Contains(TEXT("连锁")) || ObjectiveActor->IsCompleted());
		if (StepIndex == 0 && !ObjectiveActor->IsCompleted())
		{
			const FName NextGearAfterDrive = ObjectiveActor->GetExpectedAdvancedInputId();
			TestTrue(
				TEXT("Gear sequence should continue to one of the side gears after the center drive gear."),
				NextGearAfterDrive == FName(TEXT("齿轮1")) || NextGearAfterDrive == FName(TEXT("齿轮3")));
			const FHorrorAdvancedInteractionHUDState ChainPanel = ObjectiveActor->BuildAdvancedInteractionHUDState();
			TestTrue(TEXT("Gear chain panel should show the next beat after the center drive gear."), ChainPanel.NextActionLabel.ToString().Contains(TEXT("下一拍")));
			TestTrue(TEXT("Gear chain panel should name the dynamic side gear after the center drive gear."), ChainPanel.NextActionLabel.ToString().Contains(NextGearAfterDrive.ToString()));
			TestTrue(TEXT("Gear chain panel should show an active combo after the first link."), ChainPanel.ComboCount >= 1);
			TestTrue(TEXT("Gear chain engagement telemetry should advance after the drive gear links."), ChainPanel.RouteFlowFraction > OpenGearPanel.RouteFlowFraction);
			TestTrue(TEXT("Gear chain telemetry should remain bounded for animated meters."), ChainPanel.RouteFlowFraction > 0.0f && ChainPanel.RouteFlowFraction < 1.0f);
			TestEqual(TEXT("Gear calibration should move the active slot to the next jammed side gear."), ChainPanel.ActiveInputSlotIndex, NextGearAfterDrive == FName(TEXT("齿轮1")) ? 0 : 2);
			const FHorrorAdvancedInteractionInputOption* LinkedDriveGear = ChainPanel.InputOptions.FindByPredicate(
				[](const FHorrorAdvancedInteractionInputOption& Option)
				{
					return Option.InputId == FName(TEXT("齿轮2"));
				});
			const FName CurrentNextGear = ObjectiveActor->GetExpectedAdvancedInputId();
			const FHorrorAdvancedInteractionInputOption* NextStoppedGear = ChainPanel.InputOptions.FindByPredicate(
				[CurrentNextGear](const FHorrorAdvancedInteractionInputOption& Option)
				{
					return Option.InputId == CurrentNextGear;
				});
			TestNotNull(TEXT("Gear chain HUD should preserve the solved drive gear state."), LinkedDriveGear);
			TestNotNull(TEXT("Gear chain HUD should expose the next stopped gear state."), NextStoppedGear);
			if (LinkedDriveGear && NextStoppedGear)
			{
				TestTrue(TEXT("Solved gears should be marked as linked in the chain instead of just greyed out."), LinkedDriveGear->bChainLinked);
				TestFalse(TEXT("Solved gears should no longer be stalled once the chain advances."), LinkedDriveGear->bStalled);
				TestTrue(TEXT("The next gear should become the newly stalled mechanical target."), NextStoppedGear->bStalled);
				TestTrue(TEXT("Gear jam pressure should track the current stopped gear load."), ChainPanel.HazardPressureFraction >= NextStoppedGear->LoadFraction - 0.01f);
			}
		}
	}

	TestTrue(TEXT("Gear calibration should complete after the required stopped-gear corrections."), ObjectiveActor->IsCompleted());
	const FGameplayTag GearSuccessTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Gear.Success")), false);
	TestTrue(TEXT("Correct gear input should publish gear success audio feedback."), EventBus->GetHistory().ContainsByPredicate(
		[GearSuccessTag](const FHorrorEventMessage& Message)
		{
			return Message.EventTag == GearSuccessTag;
		}));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveActorUsesDynamicGearCalibrationProfilesTest,
	"HorrorProject.Game.Campaign.ObjectiveActorUsesDynamicGearCalibrationProfiles",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveActorUsesDynamicGearCalibrationProfilesTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for dynamic gear calibration coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* LeftProfileObjective = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	AHorrorCampaignObjectiveActor* RightProfileObjective = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Dynamic gear profile test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Dynamic gear profile test should spawn the first gear objective."), LeftProfileObjective);
	TestNotNull(TEXT("Dynamic gear profile test should spawn the second gear objective."), RightProfileObjective);
	if (!InstigatorActor || !LeftProfileObjective || !RightProfileObjective)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition LeftProfile;
	LeftProfile.ObjectiveId = TEXT("Test.GearA");
	LeftProfile.ObjectiveType = EHorrorCampaignObjectiveType::DisableSeal;
	LeftProfile.InteractionMode = EHorrorCampaignInteractionMode::GearCalibration;
	LeftProfile.PromptText = FText::FromString(TEXT("左链齿轮校准"));

	FHorrorCampaignObjectiveDefinition RightProfile = LeftProfile;
	RightProfile.ObjectiveId = TEXT("Test.GearAB");
	RightProfile.PromptText = FText::FromString(TEXT("右链齿轮校准"));

	LeftProfileObjective->ConfigureObjective(TEXT("Chapter.Test"), LeftProfile);
	RightProfileObjective->ConfigureObjective(TEXT("Chapter.Test"), RightProfile);

	const FHitResult EmptyHit;
	TestTrue(TEXT("First dynamic gear objective should open the calibration panel."), LeftProfileObjective->Interact_Implementation(InstigatorActor, EmptyHit));
	TestTrue(TEXT("Second dynamic gear objective should open the calibration panel."), RightProfileObjective->Interact_Implementation(InstigatorActor, EmptyHit));
	TestEqual(TEXT("Dynamic gear profiles should still begin by checking the center drive gear."), LeftProfileObjective->GetExpectedAdvancedInputId(), FName(TEXT("齿轮2")));
	TestEqual(TEXT("Dynamic gear profiles should still begin by checking the center drive gear."), RightProfileObjective->GetExpectedAdvancedInputId(), FName(TEXT("齿轮2")));

	LeftProfileObjective->Tick(0.65f);
	TestTrue(TEXT("First dynamic profile should accept the center drive gear."), LeftProfileObjective->SubmitAdvancedInteractionInput(TEXT("齿轮2"), InstigatorActor));
	RightProfileObjective->Tick(0.65f);
	TestTrue(TEXT("Second dynamic profile should accept the center drive gear."), RightProfileObjective->SubmitAdvancedInteractionInput(TEXT("齿轮2"), InstigatorActor));

	const FName LeftNextGear = LeftProfileObjective->GetExpectedAdvancedInputId();
	const FName RightNextGear = RightProfileObjective->GetExpectedAdvancedInputId();
	TestTrue(TEXT("Different gear objectives should route the next jam to different side gears."), LeftNextGear != RightNextGear);
	TestTrue(TEXT("The first dynamic profile should route to a side gear after the center drive gear."), LeftNextGear == FName(TEXT("齿轮1")) || LeftNextGear == FName(TEXT("齿轮3")));
	TestTrue(TEXT("The second dynamic profile should route to a side gear after the center drive gear."), RightNextGear == FName(TEXT("齿轮1")) || RightNextGear == FName(TEXT("齿轮3")));
	const FString LeftNextAction = LeftProfileObjective->BuildAdvancedInteractionHUDState().NextActionLabel.ToString();
	const FString RightNextAction = RightProfileObjective->BuildAdvancedInteractionHUDState().NextActionLabel.ToString();
	TestTrue(TEXT("Dynamic gear next action should describe the next beat."), LeftNextAction.Contains(TEXT("下一拍")));
	TestTrue(TEXT("Dynamic gear next action should name the actual next jammed gear."), LeftNextAction.Contains(LeftNextGear.ToString()));
	TestTrue(TEXT("Dynamic gear next action should describe the next beat."), RightNextAction.Contains(TEXT("下一拍")));
	TestTrue(TEXT("Dynamic gear next action should name the actual next jammed gear."), RightNextAction.Contains(RightNextGear.ToString()));

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
	FHorrorCampaignEveryObjectiveHasPresentationMetadataTest,
	"HorrorProject.Game.Campaign.EveryObjectiveHasPresentationMetadata",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignEveryObjectiveHasPresentationMetadataTest::RunTest(const FString& Parameters)
{
	const TArray<FHorrorCampaignChapterDefinition>& Chapters = FHorrorCampaign::GetChapters();
	TestTrue(TEXT("Campaign should include playable chapters."), Chapters.Num() > 0);

	for (const FHorrorCampaignChapterDefinition& Chapter : Chapters)
	{
		for (const FHorrorCampaignObjectiveDefinition& Objective : Chapter.Objectives)
		{
			const FString ObjectiveLabel = FString::Printf(TEXT("%s / %s"), *Chapter.ChapterId.ToString(), *Objective.ObjectiveId.ToString());
			TestFalse(
				*FString::Printf(TEXT("%s should define a mechanic label."), *ObjectiveLabel),
				Objective.Presentation.MechanicLabel.IsEmpty());
			TestFalse(
				*FString::Printf(TEXT("%s should define an input hint."), *ObjectiveLabel),
				Objective.Presentation.InputHint.IsEmpty());
			TestFalse(
				*FString::Printf(TEXT("%s should define failure stakes."), *ObjectiveLabel),
				Objective.Presentation.FailureStakes.IsEmpty());
			TestTrue(
				*FString::Printf(TEXT("%s should define a concrete risk level."), *ObjectiveLabel),
				Objective.Presentation.RiskLevel != EHorrorCampaignObjectiveRiskLevel::None);
			TestTrue(
				*FString::Printf(TEXT("%s should identify whether it uses a panel or direct interaction."), *ObjectiveLabel),
				Objective.Presentation.bUsesFocusedInteraction || Objective.Presentation.bOpensInteractionPanel);
		}
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveTrackerIncludesMissionBriefTest,
	"HorrorProject.Game.Campaign.ObjectiveTrackerIncludesMissionBrief",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveTrackerIncludesMissionBriefTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for campaign tracker mission brief coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the Horror game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	TestNotNull(TEXT("Campaign mission brief test should expose the game mode."), GameMode);
	if (!GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(TEXT("Chapter.SignalCalibration"));
	const FHorrorObjectiveTrackerSnapshot Tracker = GameMode->BuildObjectiveTrackerSnapshot();
	TestEqual(TEXT("Campaign tracker should identify the active chapter."), Tracker.ActiveChapterId, FName(TEXT("Chapter.SignalCalibration")));
	TestFalse(TEXT("Campaign tracker should identify the active objective."), Tracker.ActiveObjectiveId.IsNone());
	TestFalse(TEXT("Campaign tracker should provide an interaction label."), Tracker.InteractionLabel.IsEmpty());
	TestFalse(TEXT("Campaign tracker should provide mission context detail."), Tracker.MissionContext.IsEmpty());
	TestFalse(TEXT("Campaign tracker should provide failure stakes."), Tracker.FailureStakes.IsEmpty());
	TestTrue(TEXT("Campaign tracker should expose whether the current objective opens a panel or is a direct action."), Tracker.bUsesFocusedInteraction || Tracker.bOpensInteractionPanel);
	TestTrue(TEXT("Campaign tracker should expose enriched checklist rows."), Tracker.ChecklistItems.ContainsByPredicate(
		[](const FHorrorObjectiveChecklistItem& Item)
		{
			return !Item.ObjectiveId.IsNone()
				&& !Item.Detail.IsEmpty()
				&& (!Item.InteractionLabel.IsEmpty() || Item.bComplete);
		}));
	const FHorrorObjectiveChecklistItem* TacticalItem = Tracker.ChecklistItems.FindByPredicate(
		[](const FHorrorObjectiveChecklistItem& Item)
		{
			return !Item.TacticalLabel.IsEmpty();
		});
	TestNotNull(TEXT("Campaign tracker should expose a localized tactical summary for objective rows."), TacticalItem);
	if (TacticalItem)
	{
		const FString TacticalText = TacticalItem->TacticalLabel.ToString();
		TestTrue(TEXT("Tactical summary should describe the interaction mode."), TacticalText.Contains(TEXT("玩法")));
		TestTrue(TEXT("Tactical summary should describe the completion rule."), TacticalText.Contains(TEXT("判定")));
		TestTrue(TEXT("Tactical summary should describe the failure policy."), TacticalText.Contains(TEXT("失败")));
		TestFalse(TEXT("Tactical summary should remain localized."), ContainsLatinLetter(TacticalText));
	}

	const FHorrorObjectiveChecklistItem* BlockedItem = Tracker.ChecklistItems.FindByPredicate(
		[](const FHorrorObjectiveChecklistItem& Item)
		{
			return Item.bBlocked && !Item.LockReason.IsEmpty();
		});
	TestNull(TEXT("Campaign tracker should not leak locked future objectives or prerequisite names."), BlockedItem);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignTrackerExportsAdvancedInteractionDiagnosticsTest,
	"HorrorProject.Game.Campaign.TrackerExportsAdvancedInteractionDiagnostics",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignTrackerExportsAdvancedInteractionDiagnosticsTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* DeepWaterChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.DeepWaterStationFinale"));
	TestNotNull(TEXT("Deep Water chapter should exist for advanced tracker diagnostics coverage."), DeepWaterChapter);
	if (!DeepWaterChapter)
	{
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* BootObjective = FHorrorCampaign::FindObjectiveById(
		*DeepWaterChapter,
		TEXT("DeepWater.BootDryDock"));
	TestNotNull(TEXT("Deep Water chapter should expose the dry-dock boot objective."), BootObjective);
	if (!BootObjective)
	{
		return false;
	}

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for advanced tracker diagnostics coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the campaign game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerCharacter* PlayerCharacter = SpawnControlledCampaignPlayer(*this, World, FVector::ZeroVector);
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector(520.0f, -200.0f, 95.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Advanced tracker diagnostics test should expose the game mode."), GameMode);
	TestNotNull(TEXT("Advanced tracker diagnostics test should spawn a player."), PlayerCharacter);
	TestNotNull(TEXT("Advanced tracker diagnostics test should spawn an objective actor."), ObjectiveActor);
	if (!GameMode || !PlayerCharacter || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(DeepWaterChapter->ChapterId);
	ObjectiveActor->ConfigureObjective(DeepWaterChapter->ChapterId, *BootObjective);
	GameMode->AddRuntimeCampaignObjectiveForTests(ObjectiveActor);

	const FHitResult Hit;
	TestTrue(TEXT("Interacting with the dry-dock boot objective should open the circuit window."), ObjectiveActor->Interact_Implementation(PlayerCharacter, Hit));
	TestTrue(TEXT("Circuit objective should be in advanced interaction state."), ObjectiveActor->IsAdvancedInteractionActive());

	const FHorrorObjectiveTrackerSnapshot Tracker = GameMode->BuildObjectiveTrackerSnapshot();
	const FHorrorObjectiveChecklistItem* ActiveItem = Tracker.ChecklistItems.FindByPredicate(
		[](const FHorrorObjectiveChecklistItem& Item)
		{
			return Item.bActive;
		});
	TestNotNull(TEXT("Tracker should expose the active circuit beat."), ActiveItem);
	if (ActiveItem)
	{
		TestEqual(TEXT("Active checklist item should report advanced interaction runtime."), ActiveItem->RuntimeStatus, EHorrorCampaignObjectiveRuntimeStatus::AdvancedInteractionActive);
		TestFalse(TEXT("Active checklist item should carry device diagnostics."), ActiveItem->DeviceStatusLabel.IsEmpty());
		TestFalse(TEXT("Active checklist item should carry the next operator action."), ActiveItem->NextActionLabel.IsEmpty());
		TestTrue(TEXT("Active checklist status should mention circuit work."), ActiveItem->StatusText.ToString().Contains(TEXT("端子")) || ActiveItem->StatusText.ToString().Contains(TEXT("接线")));
		TestTrue(TEXT("Active checklist performance grade should be bounded."), ActiveItem->PerformanceGradeFraction >= 0.0f && ActiveItem->PerformanceGradeFraction <= 1.0f);
		TestTrue(TEXT("Active checklist input precision should be bounded."), ActiveItem->InputPrecisionFraction >= 0.0f && ActiveItem->InputPrecisionFraction <= 1.0f);
		TestTrue(TEXT("Active checklist device load should be bounded."), ActiveItem->DeviceLoadFraction >= 0.0f && ActiveItem->DeviceLoadFraction <= 1.0f);
		TestFalse(TEXT("Checklist device diagnostics should remain localized."), ContainsLatinLetter(ActiveItem->DeviceStatusLabel.ToString()));
		TestFalse(TEXT("Checklist next action should remain localized."), ContainsLatinLetter(ActiveItem->NextActionLabel.ToString()));
	}

	TestTrue(TEXT("Submitting an invalid circuit input should produce retry guidance."), ObjectiveActor->SubmitAdvancedInteractionInput(TEXT("错误端子"), PlayerCharacter));
	const FHorrorObjectiveTrackerSnapshot FailedTracker = GameMode->BuildObjectiveTrackerSnapshot();
	const FHorrorObjectiveChecklistItem* FailedActiveItem = FailedTracker.ChecklistItems.FindByPredicate(
		[](const FHorrorObjectiveChecklistItem& Item)
		{
			return Item.bActive;
		});
	TestNotNull(TEXT("Tracker should keep the active beat after a circuit failure."), FailedActiveItem);
	if (FailedActiveItem)
	{
		TestFalse(TEXT("Failed active checklist item should carry recovery guidance."), FailedActiveItem->FailureRecoveryLabel.IsEmpty());
		TestTrue(TEXT("Failed active checklist item should expose raised device load."), FailedActiveItem->DeviceLoadFraction > 0.0f);
		TestTrue(TEXT("Failed checklist recovery should tell the player to wait or retry safely."),
			FailedActiveItem->FailureRecoveryLabel.ToString().Contains(TEXT("等待"))
			|| FailedActiveItem->FailureRecoveryLabel.ToString().Contains(TEXT("重试")));
	}

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignTrackerSurfacesTimedPursuitDiagnosticsTest,
	"HorrorProject.Game.Campaign.TrackerSurfacesTimedPursuitDiagnostics",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignTrackerSurfacesTimedPursuitDiagnosticsTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* ForestChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.ForestOfSpikes"));
	TestNotNull(TEXT("Forest chapter should exist for pursuit tracker diagnostics coverage."), ForestChapter);
	if (!ForestChapter)
	{
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* PursuitObjective = FHorrorCampaign::FindObjectiveById(
		*ForestChapter,
		TEXT("Forest.HoldSpikeCircle"));
	const FHorrorCampaignObjectiveDefinition* SpikeBeaconObjective = FHorrorCampaign::FindObjectiveById(
		*ForestChapter,
		TEXT("Forest.AlignSpikeBeacon"));
	TestNotNull(TEXT("Forest chapter should expose the pursuit objective."), PursuitObjective);
	TestNotNull(TEXT("Forest chapter should expose the beacon prerequisite objective."), SpikeBeaconObjective);
	const FHorrorCampaignObjectiveDefinition* RootGlyphObjective = FHorrorCampaign::FindObjectiveById(
		*ForestChapter,
		TEXT("Forest.ReadRootGlyph"));
	TestNotNull(TEXT("Forest chapter should expose the root glyph prerequisite objective."), RootGlyphObjective);
	if (!PursuitObjective || !SpikeBeaconObjective || !RootGlyphObjective)
	{
		return false;
	}

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for pursuit tracker diagnostics coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the campaign game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerCharacter* PlayerCharacter = SpawnControlledCampaignPlayer(*this, World, FVector::ZeroVector);
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector(100.0f, 0.0f, 95.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Pursuit tracker diagnostics test should expose the game mode."), GameMode);
	TestNotNull(TEXT("Pursuit tracker diagnostics test should spawn a player."), PlayerCharacter);
	TestNotNull(TEXT("Pursuit tracker diagnostics test should spawn a pursuit objective actor."), ObjectiveActor);
	if (!GameMode || !PlayerCharacter || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(ForestChapter->ChapterId);
	TestTrue(
		TEXT("Pursuit tracker setup should complete the root glyph objective first."),
		GameMode->TryCompleteCampaignObjective(ForestChapter->ChapterId, RootGlyphObjective->ObjectiveId, PlayerCharacter));
	TestTrue(
		TEXT("Pursuit tracker setup should complete the forest beacon prerequisite objective second."),
		GameMode->TryCompleteCampaignObjective(ForestChapter->ChapterId, SpikeBeaconObjective->ObjectiveId, PlayerCharacter));
	ObjectiveActor->ConfigureObjective(ForestChapter->ChapterId, *PursuitObjective);
	GameMode->ResetRuntimeCampaignObjectiveViewsForTests();
	GameMode->AddRuntimeCampaignObjectiveForTests(ObjectiveActor);
	ObjectiveActor->RefreshObjectiveState();

	const FHitResult Hit;
	TestTrue(TEXT("Interacting with the forest pursuit objective should start the timed chase."), ObjectiveActor->Interact_Implementation(PlayerCharacter, Hit));
	TestTrue(TEXT("Forest pursuit objective should be active for tracker diagnostics."), ObjectiveActor->IsTimedObjectiveActive());

	const FHorrorObjectiveTrackerSnapshot Tracker = GameMode->BuildObjectiveTrackerSnapshot();
	const FHorrorObjectiveChecklistItem* ActiveItem = Tracker.ChecklistItems.FindByPredicate(
		[](const FHorrorObjectiveChecklistItem& Item)
		{
			return Item.bActive;
		});
	TestNotNull(TEXT("Tracker should expose the active pursuit beat."), ActiveItem);
	if (ActiveItem)
	{
		TestEqual(TEXT("Active pursuit checklist item should report timed pursuit runtime."), ActiveItem->RuntimeStatus, EHorrorCampaignObjectiveRuntimeStatus::TimedPursuitActive);
		TestTrue(TEXT("Active pursuit checklist item should surface the escape budget."), ActiveItem->DeviceStatusLabel.ToString().Contains(TEXT("逃生窗口")));
		TestTrue(TEXT("Active pursuit checklist item should surface a sprint cue."), ActiveItem->NextActionLabel.ToString().Contains(TEXT("冲刺")));
		TestTrue(TEXT("Active pursuit checklist item should tell the player how retry recovery works."), ActiveItem->FailureRecoveryLabel.ToString().Contains(TEXT("追逐起点")) || ActiveItem->FailureRecoveryLabel.ToString().Contains(TEXT("重新互动")));
		TestTrue(TEXT("Active pursuit checklist item should track remaining time."), ActiveItem->RemainingSeconds > 0.0f);
		TestTrue(TEXT("Active pursuit checklist item should use escape-destination distance instead of the start trigger distance."), ActiveItem->DistanceMeters > 20.0f);
		TestFalse(TEXT("Pursuit checklist diagnostics should remain localized."), ContainsLatinLetter(ActiveItem->DeviceStatusLabel.ToString() + ActiveItem->NextActionLabel.ToString() + ActiveItem->FailureRecoveryLabel.ToString()));
	}

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignLockedObjectiveActorsStayHiddenUntilAvailableTest,
	"HorrorProject.Game.Campaign.LockedObjectiveActorsStayHiddenUntilAvailable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveActorBlockedReasonListsMissingPrerequisitesTest,
	"HorrorProject.Game.Campaign.ObjectiveActorBlockedReasonListsMissingPrerequisites",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignLockedObjectiveActorsStayHiddenUntilAvailableTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* ForestChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.ForestOfSpikes"));
	TestNotNull(TEXT("Forest chapter should exist for locked actor visibility coverage."), ForestChapter);
	if (!ForestChapter)
	{
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* PursuitObjective = FHorrorCampaign::FindObjectiveById(
		*ForestChapter,
		TEXT("Forest.HoldSpikeCircle"));
	const FHorrorCampaignObjectiveDefinition* SpikeBeaconObjective = FHorrorCampaign::FindObjectiveById(
		*ForestChapter,
		TEXT("Forest.AlignSpikeBeacon"));
	TestNotNull(TEXT("Forest chapter should expose the pursuit objective."), PursuitObjective);
	TestNotNull(TEXT("Forest chapter should expose the beacon prerequisite objective."), SpikeBeaconObjective);
	if (!PursuitObjective || !SpikeBeaconObjective)
	{
		return false;
	}

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for locked actor visibility coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the campaign game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorCampaignObjectiveActor* LockedObjective = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Locked actor visibility test should expose the campaign game mode."), GameMode);
	TestNotNull(TEXT("Locked actor visibility test should spawn the locked objective actor."), LockedObjective);
	if (!GameMode || !LockedObjective)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(ForestChapter->ChapterId);
	LockedObjective->ConfigureObjective(ForestChapter->ChapterId, *PursuitObjective);
	LockedObjective->RefreshObjectiveState();

	TestFalse(TEXT("Locked future objective should not be available yet."), LockedObjective->IsAvailableForInteraction());
	TestFalse(TEXT("Locked future objective should not be presented in the world."), LockedObjective->IsPresentationVisible());
	TestEqual(
		TEXT("Locked future objective should not be traceable by interaction bounds."),
		LockedObjective->GetInteractionBoundsForTests()->GetCollisionEnabled(),
		ECollisionEnabled::NoCollision);

	TestTrue(
		TEXT("Completing the prerequisite should unlock the hidden objective."),
		GameMode->TryCompleteCampaignObjective(ForestChapter->ChapterId, SpikeBeaconObjective->ObjectiveId, nullptr));
	LockedObjective->RefreshObjectiveState();

	TestTrue(TEXT("Unlocked objective should become available."), LockedObjective->IsAvailableForInteraction());
	TestTrue(TEXT("Unlocked objective should become visible again."), LockedObjective->IsPresentationVisible());
	TestEqual(
		TEXT("Unlocked objective should restore query collision for interaction."),
		LockedObjective->GetInteractionBoundsForTests()->GetCollisionEnabled(),
		ECollisionEnabled::QueryOnly);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

bool FHorrorCampaignObjectiveActorBlockedReasonListsMissingPrerequisitesTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* ForestChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.ForestOfSpikes"));
	TestNotNull(TEXT("Forest chapter should exist for locked prerequisite diagnostics."), ForestChapter);
	if (!ForestChapter)
	{
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* PursuitObjective = FHorrorCampaign::FindObjectiveById(
		*ForestChapter,
		TEXT("Forest.HoldSpikeCircle"));
	TestNotNull(TEXT("Forest chapter should expose the locked pursuit objective."), PursuitObjective);
	if (!PursuitObjective)
	{
		return false;
	}

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for locked prerequisite diagnostics."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the campaign game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorCampaignObjectiveActor* LockedObjective = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Locked prerequisite diagnostics should expose the campaign game mode."), GameMode);
	TestNotNull(TEXT("Locked prerequisite diagnostics should spawn the objective actor."), LockedObjective);
	if (!GameMode || !LockedObjective)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(ForestChapter->ChapterId);
	LockedObjective->ConfigureObjective(ForestChapter->ChapterId, *PursuitObjective);
	LockedObjective->RefreshObjectiveState();

	const FHorrorCampaignObjectiveRuntimeState RuntimeState = LockedObjective->BuildObjectiveRuntimeState();
	const FString DiagnosticText =
		RuntimeState.BlockedReason.ToString()
		+ RuntimeState.PhaseText.ToString()
		+ RuntimeState.DeviceStatusLabel.ToString()
		+ RuntimeState.NextActionLabel.ToString();
	TestEqual(TEXT("Locked objective should report a locked runtime status."), RuntimeState.Status, EHorrorCampaignObjectiveRuntimeStatus::Locked);
	TestTrue(TEXT("Locked objective should list the missing prerequisite action."), DiagnosticText.Contains(TEXT("放置信标稳定林地路径")));
	TestTrue(TEXT("Locked objective should tell players to finish prerequisites first."), DiagnosticText.Contains(TEXT("先完成")));
	TestTrue(TEXT("Locked objective device status should name the missing prerequisite."), RuntimeState.DeviceStatusLabel.ToString().Contains(TEXT("放置信标稳定林地路径")));
	TestTrue(TEXT("Locked objective next action should name the missing prerequisite."), RuntimeState.NextActionLabel.ToString().Contains(TEXT("放置信标稳定林地路径")));
	TestFalse(TEXT("Locked prerequisite diagnostics should remain localized."), ContainsLatinLetter(DiagnosticText));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveActorUsesConfiguredTimedPursuitDurationTest,
	"HorrorProject.Game.Campaign.ObjectiveActorUsesConfiguredTimedPursuitDuration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveActorUsesConfiguredTimedPursuitDurationTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for configured pursuit duration coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AActor* InstigatorActor = World->SpawnActor<AActor>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Configured pursuit test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Configured pursuit test should spawn an objective actor."), ObjectiveActor);
	if (!InstigatorActor || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.ConfiguredPursuit");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::SurviveAmbush;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::TimedPursuit;
	Objective.PromptText = FText::FromString(TEXT("完成调参追逐"));
	Objective.CompletionText = FText::FromString(TEXT("调参追逐完成。"));
	Objective.TimedObjectiveDurationSeconds = 7.5f;

	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);
	TestEqual(TEXT("Timed pursuit should expose the configured duration."), ObjectiveActor->GetTimedObjectiveDurationSeconds(), 7.5f);

	const FHitResult EmptyHit;
	TestTrue(TEXT("Configured pursuit should start on interaction."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	ObjectiveActor->Tick(7.4f);
	TestTrue(TEXT("Configured pursuit should still be active just before its configured duration."), ObjectiveActor->IsTimedObjectiveActive());
	TestFalse(TEXT("Configured pursuit should not complete before its configured duration."), ObjectiveActor->IsCompleted());

	ObjectiveActor->Tick(0.2f);
	TestFalse(TEXT("Configured pursuit should stop after its configured duration."), ObjectiveActor->IsTimedObjectiveActive());
	TestTrue(TEXT("Configured pursuit should complete after its configured duration."), ObjectiveActor->IsCompleted());

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
	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	TestNotNull(TEXT("Escape-pursuit objective test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Escape-pursuit objective test should spawn an objective actor."), ObjectiveActor);
	TestNotNull(TEXT("Escape-pursuit objective failure should publish through the event bus."), EventBus);
	if (!InstigatorActor || !ObjectiveActor || !EventBus)
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
	FHorrorCampaignObjectiveRuntimeState RuntimeState = ObjectiveActor->BuildObjectiveRuntimeState();
	TestEqual(TEXT("Active escape pursuit runtime state should be timed pursuit."), RuntimeState.Status, EHorrorCampaignObjectiveRuntimeStatus::TimedPursuitActive);
	TestTrue(TEXT("Active escape pursuit should expose remaining route distance."), RuntimeState.EscapeDistanceMeters > 20.0f);
	TestTrue(TEXT("Active escape pursuit should expose estimated arrival time."), RuntimeState.EstimatedEscapeArrivalSeconds > 0.0f);
	TestTrue(TEXT("Active escape pursuit should expose a positive escape time budget."), RuntimeState.EscapeTimeBudgetSeconds > 0.0f);
	TestTrue(TEXT("Active escape pursuit should label the escape budget."), RuntimeState.EscapeBudgetLabel.ToString().Contains(TEXT("逃生窗口")));
	TestTrue(TEXT("Active escape pursuit should expose a sprint action hint."), RuntimeState.EscapeActionLabel.ToString().Contains(TEXT("冲刺")));
	TestTrue(TEXT("Reachable escape pursuit should tell the player to keep the lit route instead of only showing numbers."), RuntimeState.EscapeActionLabel.ToString().Contains(TEXT("亮点路线")));
	TestFalse(TEXT("Escape pursuit action hint should remain localized."), ContainsLatinLetter(RuntimeState.EscapeActionLabel.ToString()));

	AHorrorCampaignObjectiveActor* DangerousObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	TestNotNull(TEXT("Escape-pursuit objective test should spawn a tight-budget objective actor."), DangerousObjectiveActor);
	if (!DangerousObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition DangerousObjective = Objective;
	DangerousObjective.ObjectiveId = TEXT("Test.DangerEscapePursuit");
	DangerousObjective.TimedObjectiveDurationSeconds = 5.0f;
	DangerousObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), DangerousObjective);
	TestTrue(TEXT("Interacting with a tight-budget pursuit should start the timer."), DangerousObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	const FHorrorCampaignObjectiveRuntimeState DangerousRuntimeState = DangerousObjectiveActor->BuildObjectiveRuntimeState();
	TestTrue(TEXT("Dangerous escape pursuit should expose negative or exhausted escape budget."), DangerousRuntimeState.EscapeTimeBudgetSeconds < 0.0f);
	TestTrue(TEXT("Dangerous escape pursuit should explicitly warn that the margin is gone."), DangerousRuntimeState.EscapeBudgetLabel.ToString().Contains(TEXT("余量不足")));
	TestTrue(TEXT("Dangerous escape pursuit should give a tactical route correction."), DangerousRuntimeState.EscapeActionLabel.ToString().Contains(TEXT("切内线")));
	TestTrue(TEXT("Dangerous escape pursuit should tell the player not to look back."), DangerousRuntimeState.EscapeActionLabel.ToString().Contains(TEXT("不要回头")));
	TestFalse(TEXT("Dangerous escape pursuit cues should remain localized."), ContainsLatinLetter(DangerousRuntimeState.EscapeBudgetLabel.ToString() + DangerousRuntimeState.EscapeActionLabel.ToString()));

	ObjectiveActor->Tick(ObjectiveActor->GetTimedObjectiveDurationSeconds() + 0.1f);
	TestFalse(TEXT("Escape-pursuit objective should fail and stop if the player misses the destination before time expires."), ObjectiveActor->IsTimedObjectiveActive());
	TestFalse(TEXT("Escape-pursuit objective should not complete when the player misses the destination timer."), ObjectiveActor->IsCompleted());
	RuntimeState = ObjectiveActor->BuildObjectiveRuntimeState();
	TestEqual(TEXT("Failed pursuit runtime state should be explicitly retryable."), RuntimeState.Status, EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable);
	TestTrue(TEXT("Failed pursuit runtime state should carry retry affordance."), RuntimeState.bRetryable);
	TestEqual(TEXT("Failed pursuit runtime state should preserve the timeout cause."), RuntimeState.FailureCause, FName(TEXT("Failure.Campaign.PursuitTimeout")));

	const FGameplayTag ObjectiveFailedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.ObjectiveFailed")), false);
	const FHorrorEventMessage* FailureMessage = EventBus->GetHistory().FindByPredicate(
		[ObjectiveFailedTag, &Objective](const FHorrorEventMessage& Message)
		{
			return Message.EventTag == ObjectiveFailedTag && Message.SourceId == Objective.ObjectiveId;
		});
	TestNotNull(TEXT("Failed escape-pursuit objective should publish structured failure feedback."), FailureMessage);
	if (FailureMessage)
	{
		TestEqual(TEXT("Pursuit failure should expose failure severity."), FailureMessage->FeedbackSeverity, EHorrorObjectiveFeedbackSeverity::Failure);
		TestTrue(TEXT("Pursuit failure should be explicitly retryable."), FailureMessage->bRetryable);
		TestTrue(TEXT("Pursuit failure should stay readable long enough."), FailureMessage->DisplaySeconds >= 6.0f);
		TestEqual(TEXT("Pursuit failure should expose timeout cause."), FailureMessage->FailureCause, FName(TEXT("Failure.Campaign.PursuitTimeout")));
		TestEqual(TEXT("Pursuit failure should expose escape-start recovery action."), FailureMessage->RecoveryAction, FName(TEXT("Recovery.Campaign.ReturnToEscapeStart")));
		TestTrue(TEXT("Pursuit failure should expose a positive attempt index."), FailureMessage->AttemptIndex >= 1);
		TestTrue(TEXT("Pursuit failure title should describe a failed chase."), FailureMessage->DebugLabel.ToString().Contains(TEXT("追逐失败")));
		TestTrue(TEXT("Pursuit failure hint should tell the player how to retry."), FailureMessage->ObjectiveHint.ToString().Contains(TEXT("重新")) || FailureMessage->ObjectiveHint.ToString().Contains(TEXT("重试")));
		TestFalse(TEXT("Pursuit failure hint should not expose English text."), ContainsLatinLetter(FailureMessage->ObjectiveHint.ToString()));
	}

	TestTrue(TEXT("Failed escape-pursuit objective should restart cleanly on a new interaction."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	RuntimeState = ObjectiveActor->BuildObjectiveRuntimeState();
	TestEqual(TEXT("Restarted pursuit runtime state should become timed active."), RuntimeState.Status, EHorrorCampaignObjectiveRuntimeStatus::TimedPursuitActive);
	TestFalse(TEXT("Restarted pursuit runtime state should clear retry affordance while active."), RuntimeState.bRetryable);
	InstigatorActor->SetActorLocation(FVector(2920.0f, 0.0f, 0.0f));
	ObjectiveActor->Tick(0.1f);
	TestFalse(TEXT("Escape-pursuit objective should stop as soon as the player reaches the destination."), ObjectiveActor->IsTimedObjectiveActive());
	TestTrue(TEXT("Escape-pursuit objective should complete immediately on destination arrival."), ObjectiveActor->IsCompleted());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignPursuitEscapeDestinationAvoidsBlockedRouteTest,
	"HorrorProject.Game.Campaign.PursuitEscapeDestinationAvoidsBlockedRoute",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignPursuitEscapeDestinationAvoidsBlockedRouteTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for blocked escape destination coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHorrorCampaignObjectiveActor* InstigatorActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	AStaticMeshActor* BlockingWall = World->SpawnActor<AStaticMeshActor>(FVector(1500.0f, 0.0f, 95.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Blocked escape route test should spawn an objective actor."), ObjectiveActor);
	TestNotNull(TEXT("Blocked escape route test should spawn an instigator actor."), InstigatorActor);
	TestNotNull(TEXT("Blocked escape route test should spawn a blocking wall."), BlockingWall);
	if (!ObjectiveActor || !InstigatorActor || !BlockingWall)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	TestNotNull(TEXT("Blocked escape route test should load the engine cube mesh."), CubeMesh);
	if (!CubeMesh)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	BlockingWall->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
	BlockingWall->SetActorScale3D(FVector(0.2f, 8.0f, 3.0f));
	BlockingWall->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BlockingWall->GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECR_Block);
	BlockingWall->GetStaticMeshComponent()->SetCollisionObjectType(ECC_WorldStatic);
	BlockingWall->GetStaticMeshComponent()->UpdateBounds();
	BlockingWall->GetStaticMeshComponent()->RecreatePhysicsState();

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.BlockedEscapeRoute");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::SurviveAmbush;
	Objective.PromptText = FText::FromString(TEXT("触发测试追逐"));
	Objective.CompletionText = FText::FromString(TEXT("测试追逐完成"));
	Objective.EscapeDestinationOffset = FVector(3000.0f, 0.0f, 0.0f);
	Objective.EscapeCompletionRadius = 220.0f;

	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

	const FHitResult EmptyHit;
	TestTrue(TEXT("Interacting with a blocked escape-pursuit objective should start the timer."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	TestTrue(TEXT("Blocked escape-pursuit objective should expose an active destination."), ObjectiveActor->HasActiveEscapeDestination());
	const FVector EscapeDestination = ObjectiveActor->GetActiveEscapeDestinationWorldLocation();
	TestTrue(
		TEXT("Blocked escape destination resolver should move the marker away from the fully blocked forward lane."),
		FMath::Abs(EscapeDestination.Y) >= 120.0f);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(HorrorCampaignBlockedEscapeRouteTest), false);
	QueryParams.AddIgnoredActor(ObjectiveActor);
	QueryParams.AddIgnoredActor(InstigatorActor);
	FHitResult SightlineHit;
	const bool bSightlineBlocked = World->LineTraceSingleByChannel(
		SightlineHit,
		InstigatorActor->GetActorLocation() + FVector(0.0f, 0.0f, 60.0f),
		EscapeDestination + FVector(0.0f, 0.0f, 60.0f),
		ECC_Visibility,
		QueryParams);
	TestFalse(TEXT("Resolved escape destination should keep a direct fallback sightline from the player."), bSightlineBlocked && SightlineHit.bBlockingHit);

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
	FHorrorCampaignDungeonHallEchoPulseEscapeStaysOnSafeRouteTest,
	"HorrorProject.Game.Campaign.DungeonHallEchoPulseEscapeStaysOnSafeRoute",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignDungeonHallEchoPulseEscapeStaysOnSafeRouteTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* HallChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.DungeonHall"));
	TestNotNull(TEXT("Dungeon hall chapter should exist for echo-pulse escape coverage."), HallChapter);
	if (!HallChapter)
	{
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* EchoPulseObjective = FHorrorCampaign::FindObjectiveById(
		*HallChapter,
		TEXT("Hall.SurviveEchoPulse"));
	const FHorrorCampaignObjectiveDefinition* SafeRouteObjective = FHorrorCampaign::FindObjectiveById(
		*HallChapter,
		TEXT("Hall.AnchorSafeRoute"));
	TestNotNull(TEXT("Dungeon hall should expose the echo-pulse survival objective."), EchoPulseObjective);
	TestNotNull(TEXT("Dungeon hall should expose the follow-up safe-route objective."), SafeRouteObjective);
	if (!EchoPulseObjective || !SafeRouteObjective)
	{
		return false;
	}

	const FVector EscapeDestination = EchoPulseObjective->RelativeLocation + EchoPulseObjective->EscapeDestinationOffset;
	const float EscapeRouteDistanceCm = EchoPulseObjective->EscapeDestinationOffset.Size2D();
	const float DistanceToSafeRouteCm = FVector::Dist2D(EscapeDestination, SafeRouteObjective->RelativeLocation);

	TestTrue(
		TEXT("Echo-pulse escape marker should stay close to the authored safe-route marker instead of landing outside the hall."),
		DistanceToSafeRouteCm <= 650.0f);
	TestTrue(
		TEXT("Echo-pulse escape route should be short enough for the tight dungeon hall chase."),
		EscapeRouteDistanceCm >= 650.0f && EscapeRouteDistanceCm <= 1500.0f);
	TestTrue(
		TEXT("Echo-pulse escape completion radius should be forgiving in the tight dungeon hall."),
		EchoPulseObjective->EscapeCompletionRadius >= 320.0f && EchoPulseObjective->EscapeCompletionRadius <= 420.0f);
	TestTrue(
		TEXT("Echo-pulse chase should give the player enough time to recover from the shock wave."),
		EchoPulseObjective->TimedObjectiveDurationSeconds >= 18.0f);

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
	TestFalse(TEXT("Missing the escape timer should clear the temporary chase threat."), CountAwakeCampaignBossActors(World) >= 1);
	TestFalse(TEXT("Missing the escape marker should leave the survival objective incomplete for a retry."), ObjectiveActor->IsCompleted());

	TestTrue(TEXT("Failed survival objective should restart the ambush on retry."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	TestTrue(TEXT("Retrying a survival objective should wake a fresh chase threat."), CountAwakeCampaignBossActors(World) >= 1);
	InstigatorActor->SetActorLocation(ObjectiveActor->GetActorLocation());
	ObjectiveActor->Tick(0.1f);
	TestFalse(TEXT("Reaching the escape marker should immediately clear the temporary chase threat."), CountAwakeCampaignBossActors(World) >= 1);
	TestTrue(TEXT("Reaching the escape marker should immediately complete the objective."), ObjectiveActor->IsCompleted());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignTimedPursuitStopsThreatWhenCompletionRejectedTest,
	"HorrorProject.Game.Campaign.TimedPursuitStopsThreatWhenCompletionRejected",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignTimedPursuitStopsThreatWhenCompletionRejectedTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* ForestChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.ForestOfSpikes"));
	TestNotNull(TEXT("Forest chapter should exist for rejected pursuit completion coverage."), ForestChapter);
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
	TestTrue(TEXT("Transient game world should be created for rejected pursuit completion coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the campaign game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorCampaignObjectiveActor* InstigatorActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	TestNotNull(TEXT("Rejected pursuit completion test should expose the game mode."), GameMode);
	TestNotNull(TEXT("Rejected pursuit completion test should spawn an instigator."), InstigatorActor);
	TestNotNull(TEXT("Rejected pursuit completion test should spawn an objective actor."), ObjectiveActor);
	if (!GameMode || !InstigatorActor || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(ForestChapter->ChapterId);
	for (int32 ObjectiveIndex = 0; ObjectiveIndex < SurvivalObjectiveIndex; ++ObjectiveIndex)
	{
		TestTrue(
			FString::Printf(TEXT("Setup should complete prerequisite objective %s."), *ForestChapter->Objectives[ObjectiveIndex].ObjectiveId.ToString()),
			GameMode->TryCompleteCampaignObjective(ForestChapter->ChapterId, ForestChapter->Objectives[ObjectiveIndex].ObjectiveId, InstigatorActor));
	}

	ObjectiveActor->ConfigureObjective(ForestChapter->ChapterId, ForestChapter->Objectives[SurvivalObjectiveIndex]);
	const FHitResult EmptyHit;
	TestTrue(TEXT("Rejected pursuit should still start locally for runtime cleanup coverage."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	TestTrue(TEXT("Rejected pursuit should spawn an awake temporary threat."), CountAwakeCampaignBossActors(World) >= 1);
	TestTrue(
		TEXT("Setup should make later completion rejected by marking the pursuit complete in progress while the actor is still active."),
		GameMode->TryCompleteCampaignObjective(ForestChapter->ChapterId, ForestChapter->Objectives[SurvivalObjectiveIndex].ObjectiveId, InstigatorActor));

	InstigatorActor->SetActorLocation(ObjectiveActor->GetActorLocation());
	ObjectiveActor->Tick(0.1f);

	TestFalse(TEXT("Rejected pursuit completion should stop the active timed objective."), ObjectiveActor->IsTimedObjectiveActive());
	TestFalse(TEXT("Rejected pursuit completion should clear the temporary chase threat."), CountAwakeCampaignBossActors(World) >= 1);
	TestFalse(TEXT("Rejected pursuit actor should not mark itself completed when progress rejects completion."), ObjectiveActor->IsCompleted());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignDeepWaterPursuitSpawnsThreatBehindEscapeRouteTest,
	"HorrorProject.Game.Campaign.DeepWaterPursuitSpawnsThreatBehindEscapeRoute",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignDeepWaterPursuitSpawnsThreatBehindEscapeRouteTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* DeepWaterChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.DeepWaterStationFinale"));
	TestNotNull(TEXT("Deep Water chapter should exist for pursuit spawn coverage."), DeepWaterChapter);
	if (!DeepWaterChapter)
	{
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* PursuitObjective = FHorrorCampaign::FindObjectiveById(
		*DeepWaterChapter,
		TEXT("DeepWater.SurviveDockPursuit"));
	TestNotNull(TEXT("Deep Water should expose the dry-dock pursuit objective."), PursuitObjective);
	if (!PursuitObjective)
	{
		return false;
	}

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for dry-dock pursuit spawn coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the campaign game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	TestNotNull(TEXT("Dry-dock pursuit spawn test should expose the campaign game mode."), GameMode);
	if (!GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	GameMode->ResetCampaignProgressForChapterForTests(DeepWaterChapter->ChapterId);
	TestTrue(
		TEXT("Dry-dock pursuit setup should complete the power objective."),
		GameMode->TryCompleteCampaignObjective(DeepWaterChapter->ChapterId, TEXT("DeepWater.BootDryDock"), nullptr));
	TestTrue(
		TEXT("Dry-dock pursuit setup should complete the heartbeat scan objective."),
		GameMode->TryCompleteCampaignObjective(DeepWaterChapter->ChapterId, TEXT("DeepWater.DecodeHeartbeat"), nullptr));

	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(
		FVector(1320.0f, -120.0f, 95.0f),
		FRotator::ZeroRotator);
	AHorrorCampaignObjectiveActor* InstigatorActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(
		ObjectiveActor ? ObjectiveActor->GetActorLocation() : FVector::ZeroVector,
		FRotator(0.0f, 180.0f, 0.0f));
	TestNotNull(TEXT("Dry-dock pursuit test should spawn the objective actor."), ObjectiveActor);
	TestNotNull(TEXT("Dry-dock pursuit test should spawn the instigator actor."), InstigatorActor);
	if (!ObjectiveActor || !InstigatorActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	ObjectiveActor->ConfigureObjective(DeepWaterChapter->ChapterId, *PursuitObjective);
	const FVector StartLocation = InstigatorActor->GetActorLocation();
	const FVector EscapeDestination = StartLocation + PursuitObjective->EscapeDestinationOffset;
	const FHitResult EmptyHit;
	TestTrue(TEXT("Starting the dry-dock pursuit should activate the objective."), ObjectiveActor->Interact_Implementation(InstigatorActor, EmptyHit));
	AHorrorCampaignBossActor* ThreatActor = FindFirstAwakeCampaignBossActor(World);
	TestNotNull(TEXT("Starting the dry-dock pursuit should spawn an awake threat."), ThreatActor);
	if (!ThreatActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FVector EscapeVector = EscapeDestination - StartLocation;
	const FVector EscapeDirection = FVector(EscapeVector.X, EscapeVector.Y, 0.0f).GetSafeNormal();
	const FVector ThreatVector = ThreatActor->GetActorLocation() - StartLocation;
	const float ForwardProjectionCm = FVector::DotProduct(FVector(ThreatVector.X, ThreatVector.Y, 0.0f), EscapeDirection);
	const float SideProjectionCm = FMath::Abs(FVector::CrossProduct(EscapeDirection, FVector(ThreatVector.X, ThreatVector.Y, 0.0f)).Z);
	const float RouteLengthCm = FVector(EscapeVector.X, EscapeVector.Y, 0.0f).Size();

	TestTrue(TEXT("Dry-dock pursuit threat should spawn behind the player's escape direction."), ForwardProjectionCm <= -250.0f);
	TestTrue(TEXT("Dry-dock pursuit threat should not spawn inside the forward escape corridor."), ForwardProjectionCm < 0.0f || ForwardProjectionCm > RouteLengthCm + 400.0f);
	TestTrue(TEXT("Dry-dock pursuit threat should keep a side offset so it does not body-block the escape lane."), SideProjectionCm >= 320.0f);
	TestTrue(TEXT("Dry-dock pursuit threat should spawn close enough to feel immediate without starting on top of the player."), ThreatVector.Size2D() >= 950.0f && ThreatVector.Size2D() <= 2300.0f);

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
	const FHorrorCampaignAmbushThreatTuning DeepWaterTuning =
		GameMode->ResolveCampaignAmbushThreatTuningForMapForTests(TEXT("/Game/DeepWaterStation/Maps/DemoMap_VerticalSlice_Day1"));
	const FHorrorCampaignAmbushThreatTuning ForestTuning =
		GameMode->ResolveCampaignAmbushThreatTuningForMapForTests(TEXT("/Game/ForestOfSpikes/Levels/Level_ForestOfSpikes_Demo_Night"));
	const FHorrorCampaignAmbushThreatTuning DungeonHallTuning =
		GameMode->ResolveCampaignAmbushThreatTuningForMapForTests(TEXT("/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_3_hall"));
	const FHorrorCampaignAmbushThreatTuning DungeonBossRoomTuning =
		GameMode->ResolveCampaignAmbushThreatTuningForMapForTests(TEXT("/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_5_bossroom"));
	const FHorrorCampaignAmbushThreatTuning DefaultTuning;

	TestTrue(TEXT("Bodycam corridors should use a smaller pursuer scale than the forest."), BodycamTuning.ActorScale < ForestTuning.ActorScale);
	TestTrue(TEXT("The forest chase should use a larger engagement radius than the tight prologue map."), ForestTuning.EngageRadius > BodycamTuning.EngageRadius);
	TestTrue(TEXT("Default ambush tuning should stay below a punishing fear burst."), DefaultTuning.FearPressurePerSecond <= 1.2f);
	TestTrue(TEXT("Deep Water chase should be slow enough to learn."), DeepWaterTuning.MoveSpeed <= 100.0f);
	TestTrue(TEXT("Bodycam chase should be slow enough to learn."), BodycamTuning.MoveSpeed <= 100.0f);
	TestTrue(TEXT("Forest chase should be faster than the prologue but still generous."), ForestTuning.MoveSpeed > BodycamTuning.MoveSpeed && ForestTuning.MoveSpeed <= 130.0f);
	TestTrue(TEXT("Castle/dungeon chase should be slow enough to outrun."), DungeonHallTuning.MoveSpeed <= 100.0f && DungeonBossRoomTuning.MoveSpeed <= 100.0f);
	TestTrue(TEXT("Castle/dungeon attack range should be forgiving during tight turns."), DungeonHallTuning.AttackRadius <= 110.0f && DungeonBossRoomTuning.AttackRadius <= 110.0f);
	TestTrue(TEXT("Castle/dungeon fear pressure should warn without instantly exhausting the player."), DungeonHallTuning.FearPressureRadius <= 900.0f && DungeonBossRoomTuning.FearPressureRadius <= 900.0f);
	TestTrue(TEXT("Opening chase attack radii should be forgiving while players learn the route."), BodycamTuning.AttackRadius <= 100.0f && ForestTuning.AttackRadius <= 140.0f);
	TestTrue(TEXT("Opening chase fear pressure should stay readable and recoverable."), DeepWaterTuning.FearPressurePerSecond <= 0.85f && BodycamTuning.FearPressurePerSecond <= 0.9f && ForestTuning.FearPressurePerSecond <= 1.05f);
	TestTrue(TEXT("Opening chase fear pressure radius should stay compact enough to outrun."), DeepWaterTuning.FearPressureRadius <= 900.0f && BodycamTuning.FearPressureRadius <= 900.0f && ForestTuning.FearPressureRadius <= 1100.0f);

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
		TEXT("GameMode should allow players to focus a parallel required objective for navigation."),
		GameMode->SetCampaignNavigationFocusObjective(ForestChapter->ChapterId, ForestChapter->Objectives[1].ObjectiveId));
	TestEqual(TEXT("Navigation focus should store the selected parallel objective."), GameMode->GetCampaignNavigationFocusObjectiveId(), ForestChapter->Objectives[1].ObjectiveId);
	TestTrue(TEXT("Focused navigation should expose the selected parallel objective location."), GameMode->TryGetCurrentCampaignObjectiveWorldLocation(NavigationLocation));
	TestEqual(TEXT("Campaign navigation should move to the focused parallel objective actor."), NavigationLocation, SecondLocation);
	FHorrorObjectiveNavigationState FocusedNavigationState;
	AHorrorPlayerCharacter* PlayerCharacter = SpawnControlledCampaignPlayer(*this, World, FVector::ZeroVector);
	TestNotNull(TEXT("Campaign focus navigation test should spawn a controlled player."), PlayerCharacter);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	TestTrue(TEXT("Focused structured navigation should build for the selected parallel objective."), GameMode->BuildCurrentCampaignObjectiveNavigationState(PlayerCharacter, FocusedNavigationState));
	TestEqual(TEXT("Structured navigation should expose the selected focus id."), FocusedNavigationState.FocusedObjectiveId, ForestChapter->Objectives[1].ObjectiveId);
	TestEqual(TEXT("Structured navigation should point at the focused parallel objective actor."), FocusedNavigationState.WorldLocation, SecondLocation);
	TestTrue(TEXT("Focused navigation should explain that a manual focus is active."), FocusedNavigationState.DeviceStatusLabel.ToString().Contains(TEXT("手动导航")));
	TestTrue(TEXT("Focused navigation should teach players how to cycle parallel mainline objectives."), FocusedNavigationState.NextActionLabel.ToString().Contains(TEXT("["))
		&& FocusedNavigationState.NextActionLabel.ToString().Contains(TEXT("]")));

	TestTrue(
		TEXT("Completing the first objective should advance the campaign navigation target."),
		GameMode->TryCompleteCampaignObjective(ForestChapter->ChapterId, ForestChapter->Objectives[0].ObjectiveId, nullptr));
	TestTrue(TEXT("GameMode should still expose a campaign navigation location after progress advances."), GameMode->TryGetCurrentCampaignObjectiveWorldLocation(NavigationLocation));
	TestEqual(TEXT("Campaign navigation should move to the second objective actor."), NavigationLocation, SecondLocation);
	const FString AdvancedActionText = GameMode->GetCurrentCampaignObjectiveActionText().ToString();
	TestTrue(TEXT("Advanced campaign objectives should name their action mode for HUD navigation."), AdvancedActionText.Contains(TEXT("锚定"))
		|| AdvancedActionText.Contains(TEXT("调谐")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignNavigationFocusCyclesParallelRequiredObjectivesTest,
	"HorrorProject.Game.Campaign.NavigationFocusCyclesParallelRequiredObjectives",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignNavigationFocusCyclesParallelRequiredObjectivesTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* ForestChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.ForestOfSpikes"));
	TestNotNull(TEXT("Forest chapter should exist for navigation focus cycling coverage."), ForestChapter);
	if (!ForestChapter)
	{
		return false;
	}

	AHorrorGameModeBase* GameMode = NewObject<AHorrorGameModeBase>();
	TestNotNull(TEXT("Navigation focus cycling test should create a game mode."), GameMode);
	if (!GameMode)
	{
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(ForestChapter->ChapterId);
	TestTrue(TEXT("Cycling should select the first available mainline objective."), GameMode->CycleCampaignNavigationFocus(1));
	TestEqual(TEXT("Forward cycling should start on the current story objective."), GameMode->GetCampaignNavigationFocusObjectiveId(), FName(TEXT("Forest.ReadRootGlyph")));
	TestTrue(TEXT("Cycling should move to the next parallel mainline objective."), GameMode->CycleCampaignNavigationFocus(1));
	TestEqual(TEXT("Forward cycling should select the spike beacon."), GameMode->GetCampaignNavigationFocusObjectiveId(), FName(TEXT("Forest.AlignSpikeBeacon")));
	TestTrue(TEXT("Backward cycling should wrap to the previous available mainline objective."), GameMode->CycleCampaignNavigationFocus(-1));
	TestEqual(TEXT("Backward cycling should return to the root glyph."), GameMode->GetCampaignNavigationFocusObjectiveId(), FName(TEXT("Forest.ReadRootGlyph")));
	TestTrue(TEXT("Cycling should skip optional investigations."), GameMode->CycleCampaignNavigationFocus(1));
	TestFalse(TEXT("Optional investigations should not become manual navigation focus."), GameMode->GetCampaignNavigationFocusObjectiveId() == FName(TEXT("Forest.RecordHangingTape")));
	TestFalse(TEXT("Navigation focus should reject optional investigations directly."), GameMode->SetCampaignNavigationFocusObjective(ForestChapter->ChapterId, TEXT("Forest.RecordHangingTape")));
	TestFalse(TEXT("Navigation focus should reject locked future objectives directly."), GameMode->SetCampaignNavigationFocusObjective(ForestChapter->ChapterId, TEXT("Forest.HoldSpikeCircle")));

	TestTrue(TEXT("Completing the focused spike beacon should clear invalid manual focus."), GameMode->TryCompleteCampaignObjective(ForestChapter->ChapterId, TEXT("Forest.AlignSpikeBeacon"), nullptr));
	TestEqual(TEXT("Completed manual focus should automatically fall back to the next story objective."), GameMode->GetCampaignNavigationFocusObjectiveId(), FName(TEXT("Forest.ReadRootGlyph")));
	TestTrue(TEXT("Clearing focus should be explicit and idempotent."), GameMode->ClearCampaignNavigationFocusObjective());
	TestTrue(TEXT("Cleared focus should expose no manual focus id."), GameMode->GetCampaignNavigationFocusObjectiveId().IsNone());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignBuildsStructuredObjectiveNavigationStateTest,
	"HorrorProject.Game.Campaign.BuildsStructuredObjectiveNavigationState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignBuildsStructuredObjectiveNavigationStateTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* ForestChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.ForestOfSpikes"));
	TestNotNull(TEXT("Forest chapter should exist for structured navigation coverage."), ForestChapter);
	if (!ForestChapter)
	{
		return false;
	}

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for structured navigation coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the campaign game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerCharacter* PlayerCharacter = SpawnControlledCampaignPlayer(*this, World, FVector::ZeroVector);
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector(1000.0f, 0.0f, 95.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Structured navigation test should expose the campaign game mode."), GameMode);
	TestNotNull(TEXT("Structured navigation test should spawn a player."), PlayerCharacter);
	TestNotNull(TEXT("Structured navigation test should spawn an objective actor."), ObjectiveActor);
	if (!GameMode || !PlayerCharacter || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(ForestChapter->ChapterId);
	ObjectiveActor->ConfigureObjective(ForestChapter->ChapterId, ForestChapter->Objectives[0]);

	FHorrorObjectiveNavigationState NavigationState;
	TestTrue(TEXT("GameMode should build structured navigation state for the current campaign objective."), GameMode->BuildCurrentCampaignObjectiveNavigationState(PlayerCharacter, NavigationState));
	TestTrue(TEXT("Structured navigation should be visible."), NavigationState.bVisible);
	TestFalse(TEXT("Structured navigation should carry a readable objective label."), NavigationState.Label.IsEmpty());
	TestEqual(TEXT("Structured navigation should expose the current objective world location."), NavigationState.WorldLocation, ObjectiveActor->GetActorLocation());
	TestTrue(TEXT("Structured navigation should expose distance in meters."), FMath::IsNearlyEqual(NavigationState.DistanceMeters, 10.0f, 0.2f));
	TestFalse(TEXT("Structured navigation should not mark a far objective as arrived."), NavigationState.bArrived);
	TestTrue(TEXT("Structured navigation should mark known objective actors as reachable."), NavigationState.bReachable);
	TestTrue(TEXT("Structured navigation should describe a forward target."), NavigationState.DirectionText.ToString().Contains(TEXT("前方")));
	TestTrue(TEXT("Structured navigation status should include meters."), NavigationState.StatusText.ToString().Contains(TEXT("米")));
	TestFalse(TEXT("Structured navigation should carry device or objective readiness diagnostics."), NavigationState.DeviceStatusLabel.IsEmpty());
	TestFalse(TEXT("Structured navigation should carry the next objective action."), NavigationState.NextActionLabel.IsEmpty());
	TestFalse(TEXT("Structured navigation should carry recovery guidance."), NavigationState.FailureRecoveryLabel.IsEmpty());
	TestFalse(TEXT("Structured navigation diagnostics should remain localized."),
		ContainsLatinLetter(NavigationState.DeviceStatusLabel.ToString() + NavigationState.NextActionLabel.ToString() + NavigationState.FailureRecoveryLabel.ToString()));
	TestFalse(TEXT("Structured navigation should not mention out-of-story parallel objectives."), NavigationState.StatusText.ToString().Contains(TEXT("另有")));
	TestFalse(TEXT("Structured navigation should not point players to hidden parallel objectives."), NavigationState.StatusText.ToString().Contains(TEXT("任务列表")));

	PlayerCharacter->SetActorLocation(ObjectiveActor->GetActorLocation());
	TestTrue(TEXT("GameMode should rebuild structured navigation after movement."), GameMode->BuildCurrentCampaignObjectiveNavigationState(PlayerCharacter, NavigationState));
	TestTrue(TEXT("Structured navigation should mark nearby objectives as arrived."), NavigationState.bArrived);
	TestTrue(TEXT("Structured navigation should use a nearby direction label after arrival."), NavigationState.DirectionText.ToString().Contains(TEXT("附近")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignNavigationMarksBlockedObjectiveUnreachableTest,
	"HorrorProject.Game.Campaign.NavigationMarksBlockedObjectiveUnreachable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignNavigationMarksBlockedObjectiveUnreachableTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* DeepWaterChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.DeepWaterStationFinale"));
	TestNotNull(TEXT("Deep Water chapter should exist for blocked navigation coverage."), DeepWaterChapter);
	if (!DeepWaterChapter)
	{
		return false;
	}

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for blocked navigation coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the campaign game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerCharacter* PlayerCharacter = SpawnControlledCampaignPlayer(*this, World, FVector(0.0f, 0.0f, 95.0f));
	AStaticMeshActor* BlockingWall = World->SpawnActor<AStaticMeshActor>(FVector(260.0f, -100.0f, 95.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Blocked navigation test should expose the campaign game mode."), GameMode);
	TestNotNull(TEXT("Blocked navigation test should spawn a player."), PlayerCharacter);
	TestNotNull(TEXT("Blocked navigation test should spawn a blocking wall."), BlockingWall);
	if (!GameMode || !PlayerCharacter || !BlockingWall)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	TestNotNull(TEXT("Blocked navigation test should load the engine cube mesh."), CubeMesh);
	if (!CubeMesh)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	BlockingWall->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
	BlockingWall->SetActorScale3D(FVector(0.5f, 8.0f, 4.0f));
	BlockingWall->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BlockingWall->GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECR_Block);
	BlockingWall->GetStaticMeshComponent()->SetCollisionObjectType(ECC_WorldStatic);
	BlockingWall->GetStaticMeshComponent()->UpdateBounds();
	BlockingWall->GetStaticMeshComponent()->RecreatePhysicsState();

	GameMode->ResetCampaignProgressForChapterForTests(DeepWaterChapter->ChapterId);

	FHorrorObjectiveNavigationState NavigationState;
	TestTrue(TEXT("GameMode should build blocked structured navigation state."), GameMode->BuildCurrentCampaignObjectiveNavigationState(PlayerCharacter, NavigationState));
	TestFalse(TEXT("Navigation should not claim an objective is reachable when a blocking wall cuts the route."), NavigationState.bReachable);
	TestTrue(TEXT("Blocked navigation status should explicitly warn the player instead of pretending the route is clean."),
		NavigationState.StatusText.ToString().Contains(TEXT("受阻")) || NavigationState.StatusText.ToString().Contains(TEXT("绕路")));
	TestTrue(TEXT("Blocked navigation status should give a localized detour hint when no nav data can resolve a path."),
		NavigationState.StatusText.ToString().Contains(TEXT("受阻")) || NavigationState.StatusText.ToString().Contains(TEXT("绕路")));
	TestTrue(TEXT("Blocked navigation next action should point the player toward a manual detour."),
		NavigationState.NextActionLabel.ToString().Contains(TEXT("墙体")) || NavigationState.NextActionLabel.ToString().Contains(TEXT("入口")) || NavigationState.NextActionLabel.ToString().Contains(TEXT("安全点")));
	TestTrue(TEXT("Blocked navigation recovery should tell the player how to refresh the objective route."),
		NavigationState.FailureRecoveryLabel.ToString().Contains(TEXT("刷新")) || NavigationState.FailureRecoveryLabel.ToString().Contains(TEXT("重进")));
	TestFalse(TEXT("Blocked navigation detour and refresh guidance should remain localized."),
		ContainsLatinLetter(NavigationState.StatusText.ToString() + NavigationState.NextActionLabel.ToString() + NavigationState.FailureRecoveryLabel.ToString()));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignNavigationUsesAuthoredObjectiveLocationWhenActorMissingTest,
	"HorrorProject.Game.Campaign.NavigationUsesAuthoredObjectiveLocationWhenActorMissing",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignNavigationUsesAuthoredObjectiveLocationWhenActorMissingTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* ForestChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.ForestOfSpikes"));
	TestNotNull(TEXT("Forest chapter should exist for missing actor navigation coverage."), ForestChapter);
	if (!ForestChapter)
	{
		return false;
	}

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for missing actor navigation coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the campaign game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerCharacter* PlayerCharacter = SpawnControlledCampaignPlayer(*this, World, FVector::ZeroVector);
	TestNotNull(TEXT("Missing actor navigation test should expose the game mode."), GameMode);
	TestNotNull(TEXT("Missing actor navigation test should spawn a player."), PlayerCharacter);
	if (!GameMode || !PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(ForestChapter->ChapterId);

	FVector AuthoredNavigationLocation = FVector::ZeroVector;
	TestTrue(TEXT("GameMode should expose an authored fallback location when the runtime objective actor is missing."), GameMode->TryGetCurrentCampaignObjectiveWorldLocation(AuthoredNavigationLocation));
	TestTrue(TEXT("Authored navigation fallback should produce a finite location."), IsFiniteVector(AuthoredNavigationLocation));
	TestTrue(TEXT("Authored navigation fallback should remain near the objective's designed location."), AuthoredNavigationLocation.Size2D() > 100.0f);

	FHorrorObjectiveNavigationState NavigationState;
	TestTrue(TEXT("Structured navigation should still build from the authored objective location when the actor is missing."), GameMode->BuildCurrentCampaignObjectiveNavigationState(PlayerCharacter, NavigationState));
	TestTrue(TEXT("Missing actor navigation should remain visible."), NavigationState.bVisible);
	TestEqual(TEXT("Missing actor navigation should use the authored fallback location."), NavigationState.WorldLocation, AuthoredNavigationLocation);
	TestTrue(TEXT("Missing actor navigation should explain that a fallback signal is being used."), NavigationState.DeviceStatusLabel.ToString().Contains(TEXT("预计位置")));
	TestFalse(TEXT("Missing actor navigation fallback diagnostics should remain localized."), ContainsLatinLetter(NavigationState.DeviceStatusLabel.ToString() + NavigationState.NextActionLabel.ToString()));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignNavigationSuggestsParallelRequiredObjectiveWhenCurrentRouteBlockedTest,
	"HorrorProject.Game.Campaign.NavigationSuggestsParallelRequiredObjectiveWhenCurrentRouteBlocked",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignNavigationSuggestsParallelRequiredObjectiveWhenCurrentRouteBlockedTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* ForestChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.ForestOfSpikes"));
	const FHorrorCampaignObjectiveDefinition* RootGlyphObjective = ForestChapter
		? FHorrorCampaign::FindObjectiveById(*ForestChapter, TEXT("Forest.ReadRootGlyph"))
		: nullptr;
	const FHorrorCampaignObjectiveDefinition* SpikeBeaconObjective = ForestChapter
		? FHorrorCampaign::FindObjectiveById(*ForestChapter, TEXT("Forest.AlignSpikeBeacon"))
		: nullptr;
	TestNotNull(TEXT("Forest chapter should exist for blocked-route alternate navigation coverage."), ForestChapter);
	TestNotNull(TEXT("Forest root glyph objective should exist for alternate navigation coverage."), RootGlyphObjective);
	TestNotNull(TEXT("Forest spike beacon objective should exist for alternate navigation coverage."), SpikeBeaconObjective);
	if (!ForestChapter || !RootGlyphObjective || !SpikeBeaconObjective)
	{
		return false;
	}

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for alternate navigation coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the campaign game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerCharacter* PlayerCharacter = SpawnControlledCampaignPlayer(*this, World, FVector(0.0f, 0.0f, 95.0f));
	AHorrorCampaignObjectiveActor* RootGlyphActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector(1000.0f, 0.0f, 95.0f), FRotator::ZeroRotator);
	AHorrorCampaignObjectiveActor* SpikeBeaconActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector(0.0f, 850.0f, 95.0f), FRotator::ZeroRotator);
	AStaticMeshActor* BlockingWall = World->SpawnActor<AStaticMeshActor>(FVector(500.0f, 0.0f, 95.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Alternate navigation test should expose the campaign game mode."), GameMode);
	TestNotNull(TEXT("Alternate navigation test should spawn a player."), PlayerCharacter);
	TestNotNull(TEXT("Alternate navigation test should spawn the current objective actor."), RootGlyphActor);
	TestNotNull(TEXT("Alternate navigation test should spawn a parallel mainline objective actor."), SpikeBeaconActor);
	TestNotNull(TEXT("Alternate navigation test should spawn a blocking wall."), BlockingWall);
	if (!GameMode || !PlayerCharacter || !RootGlyphActor || !SpikeBeaconActor || !BlockingWall)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	TestNotNull(TEXT("Alternate navigation test should load the engine cube mesh."), CubeMesh);
	if (!CubeMesh)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	BlockingWall->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
	BlockingWall->SetActorScale3D(FVector(0.25f, 8.0f, 4.0f));
	BlockingWall->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BlockingWall->GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECR_Block);
	BlockingWall->GetStaticMeshComponent()->SetCollisionObjectType(ECC_WorldStatic);
	BlockingWall->GetStaticMeshComponent()->UpdateBounds();
	BlockingWall->GetStaticMeshComponent()->RecreatePhysicsState();

	GameMode->ResetCampaignProgressForChapterForTests(ForestChapter->ChapterId);
	RootGlyphActor->ConfigureObjective(ForestChapter->ChapterId, *RootGlyphObjective);
	SpikeBeaconActor->ConfigureObjective(ForestChapter->ChapterId, *SpikeBeaconObjective);
	GameMode->AddRuntimeCampaignObjectiveForTests(RootGlyphActor);
	GameMode->AddRuntimeCampaignObjectiveForTests(SpikeBeaconActor);

	FHorrorObjectiveNavigationState NavigationState;
	TestTrue(TEXT("Navigation should build even when the current route is blocked."), GameMode->BuildCurrentCampaignObjectiveNavigationState(PlayerCharacter, NavigationState));
	TestFalse(TEXT("Current route should be detected as blocked."), NavigationState.bReachable);
	TestTrue(TEXT("Blocked navigation should suggest a parallel required objective instead of only telling the player to wander."), NavigationState.NextActionLabel.ToString().Contains(TEXT("附近")) || NavigationState.NextActionLabel.ToString().Contains(TEXT("并行")));
	TestTrue(TEXT("Blocked navigation status should mention there is another mainline target."), NavigationState.StatusText.ToString().Contains(TEXT("主线")) || NavigationState.StatusText.ToString().Contains(TEXT("并行")));
	TestFalse(TEXT("Blocked alternate navigation guidance should remain localized."), ContainsLatinLetter(NavigationState.StatusText.ToString() + NavigationState.NextActionLabel.ToString() + NavigationState.FailureRecoveryLabel.ToString()));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignNavigationUsesObjectiveRuntimeStateTest,
	"HorrorProject.Game.Campaign.NavigationUsesObjectiveRuntimeState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignNavigationUsesObjectiveRuntimeStateTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* ForestChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.ForestOfSpikes"));
	TestNotNull(TEXT("Forest chapter should exist for runtime navigation coverage."), ForestChapter);
	if (!ForestChapter)
	{
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* SurvivalObjective = nullptr;
	for (const FHorrorCampaignObjectiveDefinition& Objective : ForestChapter->Objectives)
	{
		if (Objective.ObjectiveType == EHorrorCampaignObjectiveType::SurviveAmbush)
		{
			SurvivalObjective = &Objective;
			break;
		}
	}
	TestNotNull(TEXT("Forest chapter should expose a survival objective for runtime navigation."), SurvivalObjective);
	if (!SurvivalObjective)
	{
		return false;
	}

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for runtime navigation coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the campaign game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	TestNotNull(TEXT("Runtime navigation test should expose the campaign game mode."), GameMode);
	if (!GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(ForestChapter->ChapterId);
	for (const FHorrorCampaignObjectiveDefinition& Objective : ForestChapter->Objectives)
	{
		if (Objective.ObjectiveId == SurvivalObjective->ObjectiveId)
		{
			break;
		}

		TestTrue(
			FString::Printf(TEXT("Runtime navigation setup should complete prerequisite objective %s."), *Objective.ObjectiveId.ToString()),
			GameMode->TryCompleteCampaignObjective(ForestChapter->ChapterId, Objective.ObjectiveId, nullptr));
	}

	AHorrorPlayerCharacter* PlayerCharacter = SpawnControlledCampaignPlayer(*this, World, FVector::ZeroVector);
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector(100.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Runtime navigation test should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Runtime navigation test should spawn a survival objective actor."), ObjectiveActor);
	if (!PlayerCharacter || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	ObjectiveActor->ConfigureObjective(ForestChapter->ChapterId, *SurvivalObjective);

	FHorrorObjectiveNavigationState NavigationState;
	TestTrue(TEXT("Navigation should point at the survival objective before the chase starts."), GameMode->BuildCurrentCampaignObjectiveNavigationState(PlayerCharacter, NavigationState));
	TestEqual(TEXT("Navigation should begin at the objective actor before the chase starts."), NavigationState.WorldLocation, ObjectiveActor->GetActorLocation());

	const FHitResult Hit;
	TestTrue(TEXT("Interacting should start the survival pursuit for runtime navigation."), ObjectiveActor->Interact_Implementation(PlayerCharacter, Hit));
	TestTrue(TEXT("Navigation should still build while the pursuit is active."), GameMode->BuildCurrentCampaignObjectiveNavigationState(PlayerCharacter, NavigationState));
	TestEqual(TEXT("Pursuit navigation should switch to the active escape destination."), NavigationState.WorldLocation, ObjectiveActor->GetActiveEscapeDestinationWorldLocation());
	TestTrue(TEXT("Pursuit navigation status should mention the escape point."), NavigationState.StatusText.ToString().Contains(TEXT("逃离点")));
	TestTrue(TEXT("Pursuit navigation should expose escape budget diagnostics."), NavigationState.DeviceStatusLabel.ToString().Contains(TEXT("逃生窗口")));
	TestTrue(TEXT("Pursuit navigation should expose a sprint action cue."), NavigationState.NextActionLabel.ToString().Contains(TEXT("冲刺")));
	TestTrue(TEXT("Pursuit navigation should expose estimated arrival seconds."), NavigationState.EstimatedEscapeArrivalSeconds > 0.0f);
	TestTrue(TEXT("Pursuit navigation should expose remaining escape budget seconds."), NavigationState.EscapeTimeBudgetSeconds > 0.0f);
	TestTrue(TEXT("Pursuit navigation performance grade should be bounded."), NavigationState.PerformanceGradeFraction >= 0.0f && NavigationState.PerformanceGradeFraction <= 1.0f);
	TestFalse(TEXT("Pursuit navigation label should remain localized."), ContainsLatinLetter(NavigationState.Label.ToString()));
	TestFalse(TEXT("Pursuit navigation budget should remain localized."), ContainsLatinLetter(NavigationState.DeviceStatusLabel.ToString()));
	TestFalse(TEXT("Pursuit navigation action cue should remain localized."), ContainsLatinLetter(NavigationState.NextActionLabel.ToString()));

	TestTrue(TEXT("Aborting pursuit should put navigation into retry recovery."), ObjectiveActor->AbortTimedObjectiveForRecovery(
		PlayerCharacter,
		FName(TEXT("Failure.Campaign.PursuitTimeout")),
		FText::FromString(TEXT("追逐失败：未能抵达逃离点")),
		FText::FromString(TEXT("重新互动可再次开始追逐。"))));
	TestTrue(TEXT("Navigation should still build after pursuit failure."), GameMode->BuildCurrentCampaignObjectiveNavigationState(PlayerCharacter, NavigationState));
	TestEqual(TEXT("Failed pursuit navigation should expose retryable runtime status."), NavigationState.RuntimeStatus, EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable);
	TestTrue(TEXT("Failed pursuit navigation should keep retry affordance."), NavigationState.bRetryable);
	TestTrue(TEXT("Failed pursuit navigation should explain the chase failure."), NavigationState.DeviceStatusLabel.ToString().Contains(TEXT("追逐失败")));
	TestTrue(TEXT("Failed pursuit navigation should point the player back to the chase start."), NavigationState.NextActionLabel.ToString().Contains(TEXT("追逐起点")) || NavigationState.NextActionLabel.ToString().Contains(TEXT("重新互动")));
	TestTrue(TEXT("Failed pursuit navigation should preserve the recovery action."), NavigationState.RecoveryAction == FName(TEXT("Recovery.Campaign.ReturnToEscapeStart")));
	TestFalse(TEXT("Failed pursuit navigation guidance should remain localized."), ContainsLatinLetter(NavigationState.DeviceStatusLabel.ToString() + NavigationState.NextActionLabel.ToString() + NavigationState.FailureRecoveryLabel.ToString()));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignNavigationExportsAdvancedInteractionDiagnosticsTest,
	"HorrorProject.Game.Campaign.NavigationExportsAdvancedInteractionDiagnostics",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignNavigationExportsAdvancedInteractionDiagnosticsTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* DeepWaterChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.DeepWaterStationFinale"));
	TestNotNull(TEXT("Deep Water chapter should exist for advanced navigation diagnostics coverage."), DeepWaterChapter);
	if (!DeepWaterChapter)
	{
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* BootObjective = FHorrorCampaign::FindObjectiveById(
		*DeepWaterChapter,
		TEXT("DeepWater.BootDryDock"));
	TestNotNull(TEXT("Deep Water chapter should expose the dry-dock boot objective for navigation diagnostics."), BootObjective);
	if (!BootObjective)
	{
		return false;
	}

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for advanced navigation diagnostics coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the campaign game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerCharacter* PlayerCharacter = SpawnControlledCampaignPlayer(*this, World, FVector::ZeroVector);
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector(520.0f, -200.0f, 95.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Advanced navigation diagnostics test should expose the game mode."), GameMode);
	TestNotNull(TEXT("Advanced navigation diagnostics test should spawn a player."), PlayerCharacter);
	TestNotNull(TEXT("Advanced navigation diagnostics test should spawn an objective actor."), ObjectiveActor);
	if (!GameMode || !PlayerCharacter || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(DeepWaterChapter->ChapterId);
	ObjectiveActor->ConfigureObjective(DeepWaterChapter->ChapterId, *BootObjective);
	GameMode->AddRuntimeCampaignObjectiveForTests(ObjectiveActor);

	const FHitResult Hit;
	TestTrue(TEXT("Interacting with the dry-dock boot objective should open the advanced circuit window."), ObjectiveActor->Interact_Implementation(PlayerCharacter, Hit));

	FHorrorObjectiveNavigationState NavigationState;
	TestTrue(TEXT("Navigation should build while a circuit window is active."), GameMode->BuildCurrentCampaignObjectiveNavigationState(PlayerCharacter, NavigationState));
	TestEqual(TEXT("Navigation should expose advanced runtime state."), NavigationState.RuntimeStatus, EHorrorCampaignObjectiveRuntimeStatus::AdvancedInteractionActive);
	TestFalse(TEXT("Navigation should carry device diagnostics."), NavigationState.DeviceStatusLabel.IsEmpty());
	TestFalse(TEXT("Navigation should carry a next action cue."), NavigationState.NextActionLabel.IsEmpty());
	TestTrue(TEXT("Navigation status should tell the player which circuit action to perform."), NavigationState.StatusText.ToString().Contains(TEXT("端子")) || NavigationState.StatusText.ToString().Contains(TEXT("接线")));
	TestTrue(TEXT("Navigation advanced grade should be bounded."), NavigationState.PerformanceGradeFraction >= 0.0f && NavigationState.PerformanceGradeFraction <= 1.0f);
	TestFalse(TEXT("Navigation diagnostics should remain localized."), ContainsLatinLetter(NavigationState.DeviceStatusLabel.ToString()));
	TestFalse(TEXT("Navigation next action should remain localized."), ContainsLatinLetter(NavigationState.NextActionLabel.ToString()));

	TestTrue(TEXT("Submitting an invalid circuit input should make navigation surface recovery guidance."), ObjectiveActor->SubmitAdvancedInteractionInput(TEXT("错误端子"), PlayerCharacter));
	TestTrue(TEXT("Navigation should rebuild after a circuit failure."), GameMode->BuildCurrentCampaignObjectiveNavigationState(PlayerCharacter, NavigationState));
	TestFalse(TEXT("Failed navigation should carry recovery guidance."), NavigationState.FailureRecoveryLabel.IsEmpty());
	TestTrue(TEXT("Failed navigation status should include recovery guidance."),
		NavigationState.StatusText.ToString().Contains(TEXT("等待"))
		|| NavigationState.StatusText.ToString().Contains(TEXT("重试"))
		|| NavigationState.StatusText.ToString().Contains(TEXT("端子")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignNavigationExportsRetryableAdvancedFailureDiagnosticsTest,
	"HorrorProject.Game.Campaign.NavigationExportsRetryableAdvancedFailureDiagnostics",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignNavigationExportsRetryableAdvancedFailureDiagnosticsTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* DeepWaterChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.DeepWaterStationFinale"));
	TestNotNull(TEXT("Deep Water chapter should exist for retryable advanced navigation diagnostics."), DeepWaterChapter);
	if (!DeepWaterChapter)
	{
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* BootObjective = FHorrorCampaign::FindObjectiveById(
		*DeepWaterChapter,
		TEXT("DeepWater.BootDryDock"));
	TestNotNull(TEXT("Deep Water chapter should expose the dry-dock boot objective for retryable navigation diagnostics."), BootObjective);
	if (!BootObjective)
	{
		return false;
	}

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for retryable advanced navigation diagnostics."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the campaign game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerCharacter* PlayerCharacter = SpawnControlledCampaignPlayer(*this, World, FVector::ZeroVector);
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector(520.0f, -200.0f, 95.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Retryable advanced navigation diagnostics test should expose the game mode."), GameMode);
	TestNotNull(TEXT("Retryable advanced navigation diagnostics test should spawn a player."), PlayerCharacter);
	TestNotNull(TEXT("Retryable advanced navigation diagnostics test should spawn an objective actor."), ObjectiveActor);
	if (!GameMode || !PlayerCharacter || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition FailingBootObjective = *BootObjective;
	FailingBootObjective.AdvancedInteractionTuning.FailureStabilityDamage = 1.0f;
	GameMode->ResetCampaignProgressForChapterForTests(DeepWaterChapter->ChapterId);
	ObjectiveActor->ConfigureObjective(DeepWaterChapter->ChapterId, FailingBootObjective);
	GameMode->AddRuntimeCampaignObjectiveForTests(ObjectiveActor);

	const FHitResult Hit;
	TestTrue(TEXT("Interacting with the dry-dock boot objective should open the circuit window."), ObjectiveActor->Interact_Implementation(PlayerCharacter, Hit));
	TestTrue(TEXT("Wrong input should overload the circuit into a retryable failure."), ObjectiveActor->SubmitAdvancedInteractionInput(TEXT("错误端子"), PlayerCharacter));

	FHorrorObjectiveNavigationState NavigationState;
	TestTrue(TEXT("Navigation should build for a retryable advanced failure."), GameMode->BuildCurrentCampaignObjectiveNavigationState(PlayerCharacter, NavigationState));
	TestEqual(TEXT("Failed navigation should expose retryable runtime status."), NavigationState.RuntimeStatus, EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable);
	TestTrue(TEXT("Failed navigation should keep retry affordance."), NavigationState.bRetryable);
	TestFalse(TEXT("Failed navigation should include device diagnostics."), NavigationState.DeviceStatusLabel.IsEmpty());
	TestFalse(TEXT("Failed navigation should include recovery guidance."), NavigationState.FailureRecoveryLabel.IsEmpty());
	TestTrue(TEXT("Failed navigation status should mention retry or overload."), NavigationState.StatusText.ToString().Contains(TEXT("重试")) || NavigationState.StatusText.ToString().Contains(TEXT("过载")));
	TestFalse(TEXT("Failed navigation diagnostics should remain localized."), ContainsLatinLetter(NavigationState.DeviceStatusLabel.ToString()));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveTransformProjectsToGroundTest,
	"HorrorProject.Game.Campaign.ObjectiveTransformProjectsToGround",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveTransformProjectsToGroundTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for objective ground projection coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorGameModeBase* GameMode = World->SpawnActor<AHorrorGameModeBase>();
	AStaticMeshActor* GroundActor = World->SpawnActor<AStaticMeshActor>(FVector(500.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Ground projection test should spawn a game mode."), GameMode);
	TestNotNull(TEXT("Ground projection test should spawn a blocking ground actor."), GroundActor);
	if (!GameMode || !GroundActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	TestNotNull(TEXT("Ground projection test should load the engine cube mesh."), CubeMesh);
	if (!CubeMesh)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GroundActor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
	GroundActor->GetStaticMeshComponent()->SetWorldScale3D(FVector(8.0f, 8.0f, 0.2f));
	GroundActor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GroundActor->GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECR_Block);
	GroundActor->GetStaticMeshComponent()->RegisterComponent();

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.ProjectToGround");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::ScanAnomaly;
	Objective.RelativeLocation = FVector(500.0f, 0.0f, 900.0f);

	const FTransform ProjectedTransform = GameMode->ResolveCampaignObjectiveTransformForTests(Objective);
	TestTrue(
		TEXT("Objective transform should project a high authored point down near the blocking floor."),
		ProjectedTransform.GetLocation().Z < 250.0f);
	TestTrue(
		TEXT("Objective transform should preserve the authored horizontal offset while fixing height."),
		FMath::IsNearlyEqual(ProjectedTransform.GetLocation().X, 500.0f, 5.0f));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveTransformSanitizesOutOfBoundsAuthoredOffsetsTest,
	"HorrorProject.Game.Campaign.ObjectiveTransformSanitizesOutOfBoundsAuthoredOffsets",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveTransformSanitizesOutOfBoundsAuthoredOffsetsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for objective transform safety coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	APlayerStart* PlayerStart = World->SpawnActor<APlayerStart>(FVector(100.0f, 200.0f, 40.0f), FRotator(0.0f, 35.0f, 0.0f));
	AHorrorGameModeBase* GameMode = World->SpawnActor<AHorrorGameModeBase>();
	TestNotNull(TEXT("Objective transform safety test should spawn a player start."), PlayerStart);
	TestNotNull(TEXT("Objective transform safety test should spawn a game mode."), GameMode);
	if (!PlayerStart || !GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.OutOfBoundsObjective");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::ScanAnomaly;
	Objective.RelativeLocation = FVector(50000.0f, -42000.0f, 9000.0f);

	const FTransform SafeTransform = GameMode->ResolveCampaignObjectiveTransformForTests(Objective);
	const FVector SafeLocation = SafeTransform.GetLocation();
	const FVector FromAnchor = SafeLocation - PlayerStart->GetActorLocation();

	TestTrue(TEXT("Resolved campaign objective transform should remain finite."), IsFiniteVector(SafeLocation));
	TestTrue(
		TEXT("Resolved campaign objective transform should clamp extreme authored offsets near the runtime anchor."),
		FVector(FromAnchor.X, FromAnchor.Y, 0.0f).Size() <= 3800.0f);
	TestTrue(
		TEXT("Resolved campaign objective transform should clamp extreme vertical offsets near the playable floor."),
		FMath::Abs(FromAnchor.Z) <= 550.0f);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignObjectiveTransformFindsClearSpaceAroundBlockingGeometryTest,
	"HorrorProject.Game.Campaign.ObjectiveTransformFindsClearSpaceAroundBlockingGeometry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignObjectiveTransformFindsClearSpaceAroundBlockingGeometryTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for objective clear-space coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	APlayerStart* PlayerStart = World->SpawnActor<APlayerStart>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHorrorGameModeBase* GameMode = World->SpawnActor<AHorrorGameModeBase>();
	AStaticMeshActor* BlockingActor = World->SpawnActor<AStaticMeshActor>(FVector(600.0f, 0.0f, 95.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Clear-space test should spawn a player start."), PlayerStart);
	TestNotNull(TEXT("Clear-space test should spawn a game mode."), GameMode);
	TestNotNull(TEXT("Clear-space test should spawn blocking geometry."), BlockingActor);
	if (!PlayerStart || !GameMode || !BlockingActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	TestNotNull(TEXT("Clear-space test should load the engine cube mesh."), CubeMesh);
	if (!CubeMesh)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	BlockingActor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
	BlockingActor->GetStaticMeshComponent()->SetWorldScale3D(FVector(3.0f, 3.0f, 3.0f));
	BlockingActor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BlockingActor->GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECR_Block);
	BlockingActor->GetStaticMeshComponent()->RegisterComponent();

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.BlockedObjective");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::ScanAnomaly;
	Objective.RelativeLocation = FVector(600.0f, 0.0f, 95.0f);

	const FTransform SafeTransform = GameMode->ResolveCampaignObjectiveTransformForTests(Objective);
	TestTrue(
		TEXT("Objective transform should move away from blocking geometry near the authored point."),
		FVector::Dist2D(SafeTransform.GetLocation(), BlockingActor->GetActorLocation()) >= 275.0f);

	const FCollisionShape ObjectiveShape = FCollisionShape::MakeBox(FVector(75.0f, 75.0f, 70.0f));
	TestFalse(
		TEXT("Resolved campaign objective location should not overlap blocking world geometry."),
		World->OverlapBlockingTestByChannel(
			SafeTransform.GetLocation() + FVector(0.0f, 0.0f, 85.0f),
			SafeTransform.GetRotation(),
			ECC_WorldStatic,
			ObjectiveShape));

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
	TestTrue(TEXT("Default boss pressure should warn without spiking fear too quickly."), BossActor->CalculateFearPressureAmount(0.0f, 1.0f) <= 1.2f);

	BossActor->SetBossDefeated(true);
	TestFalse(TEXT("Defeated boss should stop fear pressure."), BossActor->ApplyBossPressureToActor(TargetActor, 1.0f));
	TestEqual(TEXT("Defeated boss pressure should leave fear unchanged."), FearComponent->GetFearValue(), InitialFear);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignBossActorClampsUnsafeChasePressureTest,
	"HorrorProject.Game.Campaign.BossActorClampsUnsafeChasePressure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignBossActorClampsUnsafeChasePressureTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for unsafe boss tuning coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorCampaignBossActor* BossActor = World->SpawnActor<AHorrorCampaignBossActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHorrorCampaignObjectiveActor* TargetActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector(5000.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Unsafe tuning test should spawn a boss actor."), BossActor);
	TestNotNull(TEXT("Unsafe tuning test should spawn a target actor."), TargetActor);
	if (!BossActor || !TargetActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	BossActor->ConfigureBoss(TEXT("DeepWater.SurviveDockPursuit"), FText::FromString(TEXT("石像巨人")), 1);
	BossActor->ConfigureChasePressureWithFearRate(
		900.0f,
		8000.0f,
		900.0f,
		5000.0f,
		10.0f,
		6.0f);
	BossActor->SetBossAwake(true);

	const FVector InitialLocation = BossActor->GetActorLocation();
	TestTrue(TEXT("Unsafe boss tuning should still allow movement toward a distant target."), BossActor->MoveBossTowardActor(TargetActor, 1.0f));
	const float ActualStepDistance = FVector::Dist2D(InitialLocation, BossActor->GetActorLocation());
	TestTrue(TEXT("Boss movement should be clamped below a fair sprint-readable chase speed."), ActualStepDistance <= 180.0f + KINDA_SMALL_NUMBER);
	TestTrue(TEXT("Boss attack radius should be clamped so unsafe tuning does not create unavoidable long-range hits."), !BossActor->CanBossAttackActor(TargetActor));
	TestTrue(TEXT("Boss fear pressure should be clamped below a recoverable one-second spike."), BossActor->CalculateFearPressureAmount(0.0f, 1.0f) <= 1.4f + KINDA_SMALL_NUMBER);
	TestTrue(TEXT("Unsafe actor scale should be clamped so the pursuer cannot body-block entire corridors."), BossActor->GetActorScale3D().GetMax() <= 1.35f + KINDA_SMALL_NUMBER);

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
	TestTrue(TEXT("Campaign boss tick should be frequent enough to avoid visible sliding during chase."), BossActor->PrimaryActorTick.TickInterval <= 0.06f);
	const float InitialDistance = FVector::Dist(BossActor->GetActorLocation(), TargetActor->GetActorLocation());
	TestFalse(TEXT("Configured boss should wait dormant until the arena altar wakes it."), BossActor->IsBossAwake());
	TestFalse(TEXT("Dormant boss should not move before its wake objective resolves."), BossActor->MoveBossTowardActor(TargetActor, 1.0f));
	USkeletalMeshComponent* BossMesh = BossActor->FindComponentByClass<USkeletalMeshComponent>();
	UAnimationAsset* IdleAnimation = LoadObject<UAnimationAsset>(nullptr, TEXT("/Game/Stone_Golem/demo/animations/ThirdPersonIdle.ThirdPersonIdle"));
	UAnimationAsset* RunAnimation = LoadObject<UAnimationAsset>(nullptr, TEXT("/Game/Stone_Golem/demo/animations/ThirdPersonRun.ThirdPersonRun"));
	TestNotNull(TEXT("Campaign boss should expose a skeletal mesh component for animation coverage."), BossMesh);
	TestNotNull(TEXT("Campaign boss idle animation asset should be loadable."), IdleAnimation);
	TestNotNull(TEXT("Campaign boss run animation asset should be loadable."), RunAnimation);
	if (BossMesh && IdleAnimation && RunAnimation)
	{
		TestEqual(TEXT("Dormant campaign boss should use idle animation instead of sliding in a static pose."), BossMesh->AnimationData.AnimToPlay.Get(), IdleAnimation);
	}

	BossActor->SetBossAwake(true);
	TestTrue(TEXT("Boss should report awake after the arena altar resolves."), BossActor->IsBossAwake());
	if (BossMesh && RunAnimation)
	{
		TestEqual(TEXT("Awake campaign boss should switch to a run animation before chasing."), BossMesh->AnimationData.AnimToPlay.Get(), RunAnimation);
		TestTrue(TEXT("Awake campaign boss should scale run animation playback to chase speed instead of sliding at a fixed loop."), BossMesh->GetPlayRate() > 1.0f);
	}
	TestTrue(TEXT("Living boss should move toward an engageable target."), BossActor->MoveBossTowardActor(TargetActor, 1.0f));
	TestTrue(
		TEXT("Boss movement should reduce the distance to its target."),
		FVector::Dist(BossActor->GetActorLocation(), TargetActor->GetActorLocation()) < InitialDistance);

	TargetActor->SetActorLocation(BossActor->GetActorLocation() + FVector(120.0f, 0.0f, 0.0f));
	TestTrue(TEXT("Boss should recognize a target inside attack range."), BossActor->CanBossAttackActor(TargetActor));
	TargetActor->SetActorLocation(BossActor->GetActorLocation() + FVector(120.0f, 0.0f, 260.0f));
	TestFalse(TEXT("Boss should not attack targets separated by a large vertical gap."), BossActor->CanBossAttackActor(TargetActor));
	TargetActor->SetActorLocation(BossActor->GetActorLocation() + FVector(120.0f, 0.0f, 0.0f));
	TestTrue(TEXT("Boss should trigger an attack on a nearby target."), BossActor->TryTriggerBossAttack(TargetActor));
	TestFalse(TEXT("Boss attack cooldown should block immediate repeat attacks."), BossActor->TryTriggerBossAttack(TargetActor));

	BossActor->SetBossDefeated(true);
	if (BossMesh && IdleAnimation)
	{
		TestEqual(TEXT("Defeated campaign boss should return to idle animation."), BossMesh->AnimationData.AnimToPlay.Get(), IdleAnimation);
	}
	TargetActor->SetActorLocation(BossActor->GetActorLocation() + FVector(1000.0f, 0.0f, 0.0f));
	TestFalse(TEXT("Defeated boss should stop moving."), BossActor->MoveBossTowardActor(TargetActor, 1.0f));
	TestFalse(TEXT("Defeated boss should stop attacking."), BossActor->CanBossAttackActor(TargetActor));
	TestFalse(TEXT("Defeated boss should no longer report awake."), BossActor->IsBossAwake());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignBossAttackRequiresClearLineTest,
	"HorrorProject.Game.Campaign.BossAttackRequiresClearLine",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignBossAttackRequiresClearLineTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for boss line-of-sight attack coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorCampaignBossActor* BossActor = World->SpawnActor<AHorrorCampaignBossActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHorrorCampaignObjectiveActor* TargetActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector(120.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	AStaticMeshActor* WallActor = World->SpawnActor<AStaticMeshActor>(FVector(60.0f, 0.0f, 95.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Line-of-sight attack test should spawn a boss actor."), BossActor);
	TestNotNull(TEXT("Line-of-sight attack test should spawn a target actor."), TargetActor);
	TestNotNull(TEXT("Line-of-sight attack test should spawn a blocking wall."), WallActor);
	if (!BossActor || !TargetActor || !WallActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	if (UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		WallActor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
	}
	WallActor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
	WallActor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WallActor->GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECR_Block);
	WallActor->SetActorScale3D(FVector(0.08f, 2.0f, 2.0f));

	BossActor->ConfigureBoss(TEXT("Chapter.StoneGolemBoss"), FText::FromString(TEXT("石像巨人")), 3);
	BossActor->SetBossAwake(true);
	TestFalse(TEXT("Boss should not attack through a blocking wall."), BossActor->CanBossAttackActor(TargetActor));

	WallActor->Destroy();
	TestTrue(TEXT("Boss should attack nearby targets again once the line is clear."), BossActor->CanBossAttackActor(TargetActor));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignBossMovementDoesNotSlideThroughWallsTest,
	"HorrorProject.Game.Campaign.BossMovementDoesNotSlideThroughWalls",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignBossMovementDoesNotSlideThroughWallsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for boss wall movement coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorCampaignBossActor* BossActor = World->SpawnActor<AHorrorCampaignBossActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHorrorCampaignObjectiveActor* TargetActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector(1000.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	AStaticMeshActor* WallActor = World->SpawnActor<AStaticMeshActor>(FVector(500.0f, 0.0f, 95.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Wall movement test should spawn a boss actor."), BossActor);
	TestNotNull(TEXT("Wall movement test should spawn a target actor."), TargetActor);
	TestNotNull(TEXT("Wall movement test should spawn a blocking wall."), WallActor);
	if (!BossActor || !TargetActor || !WallActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	if (UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		WallActor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
	}
	WallActor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
	WallActor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WallActor->GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECR_Block);
	WallActor->SetActorScale3D(FVector(0.1f, 3.0f, 2.0f));

	BossActor->ConfigureBoss(TEXT("Chapter.StoneGolemBoss"), FText::FromString(TEXT("石像巨人")), 1);
	BossActor->SetBossAwake(true);
	const FVector InitialLocation = BossActor->GetActorLocation();
	TestFalse(TEXT("Boss should not slide directly through a blocking wall when no route is available."), BossActor->MoveBossTowardActor(TargetActor, 1.0f));
	TestEqual(TEXT("Blocked boss movement should leave the boss at its starting point."), BossActor->GetActorLocation(), InitialLocation);

	WallActor->Destroy();
	TestTrue(TEXT("Boss should move again once the forward lane is clear."), BossActor->MoveBossTowardActor(TargetActor, 1.0f));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignBossMovementUsesCornerSidestepWhenSightlineIsBlockedTest,
	"HorrorProject.Game.Campaign.BossMovementUsesCornerSidestepWhenSightlineIsBlocked",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignBossMovementUsesCornerSidestepWhenSightlineIsBlockedTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for boss corner movement coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorCampaignBossActor* BossActor = World->SpawnActor<AHorrorCampaignBossActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHorrorCampaignObjectiveActor* TargetActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(FVector(900.0f, 420.0f, 0.0f), FRotator::ZeroRotator);
	AStaticMeshActor* CornerWall = World->SpawnActor<AStaticMeshActor>(FVector(450.0f, 0.0f, 95.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Corner movement test should spawn a boss actor."), BossActor);
	TestNotNull(TEXT("Corner movement test should spawn a target actor."), TargetActor);
	TestNotNull(TEXT("Corner movement test should spawn a corner wall."), CornerWall);
	if (!BossActor || !TargetActor || !CornerWall)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	if (UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		CornerWall->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
	}
	CornerWall->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
	CornerWall->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CornerWall->GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECR_Block);
	CornerWall->SetActorScale3D(FVector(0.10f, 6.0f, 2.0f));

	BossActor->ConfigureBoss(TEXT("Chapter.StoneGolemBoss"), FText::FromString(TEXT("石像巨人")), 1);
	BossActor->SetBossAwake(true);
	const FVector InitialLocation = BossActor->GetActorLocation();
	const float InitialDistance = FVector::Dist2D(InitialLocation, TargetActor->GetActorLocation());

	bool bMovedAtLeastOnce = false;
	for (int32 TickIndex = 0; TickIndex < 6; ++TickIndex)
	{
		bMovedAtLeastOnce |= BossActor->MoveBossTowardActor(TargetActor, 0.35f);
	}

	const FVector MovedLocation = BossActor->GetActorLocation();
	TestTrue(TEXT("Boss should take a sidestep around a visible corner instead of freezing behind a wall."), bMovedAtLeastOnce);
	TestTrue(TEXT("Corner sidestep should move the boss laterally toward the open lane."), MovedLocation.Y > InitialLocation.Y + 40.0f);
	TestTrue(TEXT("Corner sidestep should reduce pressure distance without crossing the wall plane."), FVector::Dist2D(MovedLocation, TargetActor->GetActorLocation()) < InitialDistance);
	TestTrue(TEXT("Corner sidestep should not tunnel through the blocking wall."), MovedLocation.X < CornerWall->GetActorLocation().X - 35.0f);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignBossActorProfilesThreatPresentationByMapTest,
	"HorrorProject.Game.Campaign.BossActorProfilesThreatPresentationByMap",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignBossActorProfilesThreatPresentationByMapTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for boss presentation profile coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorCampaignBossActor* DeepWaterThreat = World->SpawnActor<AHorrorCampaignBossActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHorrorCampaignBossActor* ForestThreat = World->SpawnActor<AHorrorCampaignBossActor>(FVector(400.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Presentation profile test should spawn a Deep Water threat."), DeepWaterThreat);
	TestNotNull(TEXT("Presentation profile test should spawn a forest threat."), ForestThreat);
	if (!DeepWaterThreat || !ForestThreat)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	DeepWaterThreat->ConfigureBoss(TEXT("DeepWater.SurviveDockPursuit"), NSLOCTEXT("HorrorGameMode", "Chaser", "追猎者"), 1);
	ForestThreat->ConfigureBoss(TEXT("Forest.HoldSpikeCircle"), NSLOCTEXT("HorrorGameMode", "Chaser", "追猎者"), 1);
	DeepWaterThreat->SetBossAwake(true);
	ForestThreat->SetBossAwake(true);

	USkeletalMeshComponent* DeepWaterMesh = DeepWaterThreat->FindComponentByClass<USkeletalMeshComponent>();
	USkeletalMeshComponent* ForestMesh = ForestThreat->FindComponentByClass<USkeletalMeshComponent>();
	TestNotNull(TEXT("Deep Water presentation threat should expose a skeletal mesh."), DeepWaterMesh);
	TestNotNull(TEXT("Forest presentation threat should expose a skeletal mesh."), ForestMesh);
	if (DeepWaterMesh && ForestMesh)
	{
		TestNotNull(TEXT("Deep Water threat should resolve a material profile."), DeepWaterMesh->GetMaterial(0));
		TestNotNull(TEXT("Forest threat should resolve a material profile."), ForestMesh->GetMaterial(0));
		TestTrue(
			TEXT("Different maps should not present every pursuer with the exact same material profile."),
			DeepWaterMesh->GetMaterial(0) != ForestMesh->GetMaterial(0));
	}

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
	const FVector ExpectedWeakPointLocation = GameMode->ResolveCampaignObjectiveTransformForTests(*LeftShoulderObjective).GetLocation();
	TestTrue(
		TEXT("Boss attack recovery should not use the stale Day1 checkpoint outside the boss map."),
		FVector::Dist(RecoveredLocation, OldDay1CheckpointLocation) > 10000.0f);
	TestTrue(
		TEXT("Boss attack recovery should place the player close enough to retry the left shoulder weak point."),
		FVector::Dist2D(RecoveredLocation, ExpectedWeakPointLocation) <= 900.0f);
	TestTrue(
		TEXT("Boss attack recovery should keep the player inside a playable vertical band near the current objective."),
		FMath::Abs(RecoveredLocation.Z - ExpectedWeakPointLocation.Z) <= 650.0f);
	TestTrue(TEXT("Boss attack recovery should keep the arena boss awake after the safe reset."), CountAwakeCampaignBossActors(World) >= 1);

	UGameplayStatics::DeleteGameInSlot(AutosaveSlotName, AutosaveUserIndex);
	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignBossAttackFallsBackToCurrentGroundWhenObjectiveRecoveryIsUnsafeTest,
	"HorrorProject.Game.Campaign.BossAttackFallsBackToCurrentGroundWhenObjectiveRecoveryIsUnsafe",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignBossAttackFallsBackToCurrentGroundWhenObjectiveRecoveryIsUnsafeTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* BossChapter = FHorrorCampaign::FindBossChapter();
	TestNotNull(TEXT("Boss chapter should exist for unsafe recovery fallback coverage."), BossChapter);
	if (!BossChapter || BossChapter->Objectives.Num() < 2)
	{
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* LeftShoulderObjective =
		FHorrorCampaign::FindObjectiveById(*BossChapter, TEXT("Boss.WeakPoint.LeftShoulder"));
	TestNotNull(TEXT("Boss chapter should expose the left shoulder weak point objective."), LeftShoulderObjective);
	if (!LeftShoulderObjective)
	{
		return false;
	}

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for unsafe recovery fallback coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the campaign recovery game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerCharacter* PlayerCharacter = SpawnControlledCampaignPlayer(*this, World, FVector(5200.0f, 0.0f, 105.0f));
	AStaticMeshActor* GroundActor = World->SpawnActor<AStaticMeshActor>(FVector(5200.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Unsafe recovery fallback should expose the game mode."), GameMode);
	TestNotNull(TEXT("Unsafe recovery fallback should spawn a controlled player."), PlayerCharacter);
	TestNotNull(TEXT("Unsafe recovery fallback should spawn a local floor under the player."), GroundActor);
	if (!GameMode || !PlayerCharacter || !GroundActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	TestNotNull(TEXT("Unsafe recovery fallback test should load the engine cube mesh."), CubeMesh);
	if (!CubeMesh)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GroundActor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
	GroundActor->GetStaticMeshComponent()->SetWorldScale3D(FVector(6.0f, 6.0f, 0.2f));
	GroundActor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GroundActor->GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECR_Block);
	GroundActor->GetStaticMeshComponent()->RegisterComponent();

	const float GroundTopZ = GroundActor->GetStaticMeshComponent()->Bounds.Origin.Z + GroundActor->GetStaticMeshComponent()->Bounds.BoxExtent.Z;
	const float PlayerCapsuleHalfHeight = PlayerCharacter->GetCapsuleComponent()
		? PlayerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
		: 96.0f;
	PlayerCharacter->SetActorLocation(FVector(5200.0f, 0.0f, GroundTopZ + PlayerCapsuleHalfHeight));

	GameMode->ResetCampaignProgressForChapterForTests(BossChapter->ChapterId);
	TestTrue(
		TEXT("Completing the arena altar should make the left shoulder the current boss objective."),
		GameMode->TryCompleteCampaignObjective(BossChapter->ChapterId, BossChapter->Objectives[0].ObjectiveId, PlayerCharacter));

	const FVector SafePlayerArea = PlayerCharacter->GetActorLocation();
	const FVector UnsafeWeakPointArea = GameMode->ResolveCampaignObjectiveTransformForTests(*LeftShoulderObjective).GetLocation();
	TestTrue(
		TEXT("Unsafe recovery setup should place the player far away from the authored weak point area."),
		FVector::Dist2D(SafePlayerArea, UnsafeWeakPointArea) > 2500.0f);

	TestTrue(
		TEXT("Boss attack failure should recover on the player's current grounded area when the objective recovery point has no floor."),
		GameMode->RequestPlayerFailure(TEXT("Failure.Boss.StoneGolemAttack"), FText::FromString(TEXT("巨人抓住了你。"))));

	const FVector RecoveredLocation = PlayerCharacter->GetActorLocation();
	TestTrue(
		FString::Printf(
			TEXT("Unsafe objective recovery should fall back near the current grounded player area. Safe=%s Recovered=%s Objective=%s"),
			*SafePlayerArea.ToCompactString(),
			*RecoveredLocation.ToCompactString(),
			*UnsafeWeakPointArea.ToCompactString()),
		FVector::Dist2D(RecoveredLocation, SafePlayerArea) <= 450.0f);
	TestTrue(
		FString::Printf(
			TEXT("Unsafe objective recovery should not teleport the player to the ungrounded weak point area. Safe=%s Recovered=%s Objective=%s"),
			*SafePlayerArea.ToCompactString(),
			*RecoveredLocation.ToCompactString(),
			*UnsafeWeakPointArea.ToCompactString()),
		FVector::Dist2D(RecoveredLocation, UnsafeWeakPointArea) > 2500.0f);
	TestTrue(
		FString::Printf(
			TEXT("Unsafe objective recovery should land within the local playable vertical band. Safe=%s Recovered=%s Objective=%s"),
			*SafePlayerArea.ToCompactString(),
			*RecoveredLocation.ToCompactString(),
			*UnsafeWeakPointArea.ToCompactString()),
		FMath::Abs(RecoveredLocation.Z - SafePlayerArea.Z) <= 180.0f);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignBossAttackAbortsActivePursuitBeforeRecoveryTest,
	"HorrorProject.Game.Campaign.BossAttackAbortsActivePursuitBeforeRecovery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignBossAttackAbortsActivePursuitBeforeRecoveryTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* ForestChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.ForestOfSpikes"));
	TestNotNull(TEXT("Forest chapter should exist for boss pursuit recovery coverage."), ForestChapter);
	if (!ForestChapter)
	{
		return false;
	}

	const int32 SurvivalObjectiveIndex = ForestChapter->Objectives.IndexOfByPredicate(
		[](const FHorrorCampaignObjectiveDefinition& Objective)
		{
			return Objective.ObjectiveId == FName(TEXT("Forest.HoldSpikeCircle"));
		});
	TestTrue(TEXT("Forest chapter should expose the hold-spike pursuit objective."), SurvivalObjectiveIndex != INDEX_NONE);
	if (SurvivalObjectiveIndex == INDEX_NONE)
	{
		return false;
	}

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for boss pursuit recovery coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the campaign game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerCharacter* PlayerCharacter = SpawnControlledCampaignPlayer(*this, World, FVector::ZeroVector);
	TestNotNull(TEXT("Boss pursuit recovery test should expose the game mode."), GameMode);
	TestNotNull(TEXT("Boss pursuit recovery test should spawn a controlled player."), PlayerCharacter);
	if (!GameMode || !PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(ForestChapter->ChapterId);
	for (int32 ObjectiveIndex = 0; ObjectiveIndex < SurvivalObjectiveIndex; ++ObjectiveIndex)
	{
		TestTrue(
			FString::Printf(TEXT("Setup should complete prerequisite objective %s."), *ForestChapter->Objectives[ObjectiveIndex].ObjectiveId.ToString()),
			GameMode->TryCompleteCampaignObjective(ForestChapter->ChapterId, ForestChapter->Objectives[ObjectiveIndex].ObjectiveId, PlayerCharacter));
	}

	const FVector PursuitStart(1200.0f, -100.0f, 95.0f);
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(PursuitStart, FRotator::ZeroRotator);
	AHorrorCampaignBossActor* BossActor = World->SpawnActor<AHorrorCampaignBossActor>(PursuitStart + FVector(80.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Boss pursuit recovery test should spawn a survival objective actor."), ObjectiveActor);
	TestNotNull(TEXT("Boss pursuit recovery test should spawn a boss actor."), BossActor);
	if (!ObjectiveActor || !BossActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	ObjectiveActor->ConfigureObjective(ForestChapter->ChapterId, ForestChapter->Objectives[SurvivalObjectiveIndex]);
	GameMode->AddRuntimeCampaignObjectiveForTests(ObjectiveActor);

	const FHitResult EmptyHit;
	PlayerCharacter->SetActorLocation(PursuitStart);
	TestTrue(TEXT("Starting the pursuit should activate the objective."), ObjectiveActor->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestTrue(TEXT("Pursuit should move the objective marker to the escape destination."), FVector::Dist2D(ObjectiveActor->GetActorLocation(), PursuitStart) > 500.0f);
	const FVector EscapeMarkerLocation = ObjectiveActor->GetActorLocation();

	BossActor->ConfigureBoss(ForestChapter->ChapterId, FText::FromString(TEXT("石像巨人")), 1);
	BossActor->SetBossAwake(true);
	TestTrue(TEXT("Boss attack should trigger pursuit recovery."), BossActor->TryTriggerBossAttack(PlayerCharacter));

	TestFalse(TEXT("Boss attack recovery should stop the active timed pursuit."), ObjectiveActor->IsTimedObjectiveActive());
	TestFalse(TEXT("Boss attack recovery should leave the objective retryable instead of completing it."), ObjectiveActor->IsCompleted());
	TestTrue(TEXT("Boss attack recovery should move the objective marker back to the pursuit start."), FVector::Dist2D(ObjectiveActor->GetActorLocation(), PursuitStart) <= 10.0f);
	TestTrue(TEXT("Boss attack recovery should not keep the objective marker at the escape destination."), FVector::Dist2D(ObjectiveActor->GetActorLocation(), EscapeMarkerLocation) > 500.0f);
	TestTrue(TEXT("Boss attack recovery should teleport the player near the pursuit start, not the escape marker."), FVector::Dist2D(PlayerCharacter->GetActorLocation(), PursuitStart) <= 900.0f);

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
		TestTrue(
			TEXT("GameMode should allow dependency-free campaign objectives in any order."),
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
		TestTrue(
			TEXT("Dependency-free campaign objectives should be completable out of authored order."),
			Progress.TryCompleteObjective(ForestChapter->Objectives[1].ObjectiveId));
		TestEqual(TEXT("Out-of-order objective completion should count as progress."), Progress.GetCompletedObjectiveCount(), 1);
	}

	for (const FHorrorCampaignObjectiveDefinition& Objective : ForestChapter->Objectives)
	{
		if (Progress.HasCompletedObjective(Objective.ObjectiveId))
		{
			continue;
		}

		TestTrue(
			FString::Printf(TEXT("Objective should complete once dependencies are satisfied: %s."), *Objective.ObjectiveId.ToString()),
			Progress.TryCompleteObjective(Objective.ObjectiveId));
	}

	TestEqual(TEXT("All forest objectives should be counted."), Progress.GetCompletedObjectiveCount(), ForestChapter->Objectives.Num());
	TestTrue(TEXT("A non-boss chapter should unlock after all required objectives complete."), Progress.IsChapterComplete());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignProgressionSupportsDependenciesAndOptionalObjectivesTest,
	"HorrorProject.Game.Campaign.ProgressionSupportsDependenciesAndOptionalObjectives",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCampaignImportSaveStateRejectsDependencyBypassTest,
	"HorrorProject.Game.Campaign.ImportSaveStateRejectsDependencyBypass",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCampaignProgressionSupportsDependenciesAndOptionalObjectivesTest::RunTest(const FString& Parameters)
{
	FHorrorCampaignChapterDefinition Chapter;
	Chapter.ChapterId = TEXT("Chapter.DependencyGraph");

	FHorrorCampaignObjectiveDefinition RootA;
	RootA.ObjectiveId = TEXT("Graph.RootA");
	RootA.ObjectiveType = EHorrorCampaignObjectiveType::ScanAnomaly;
	RootA.PromptText = FText::FromString(TEXT("扫描左侧异常"));

	FHorrorCampaignObjectiveDefinition RootB;
	RootB.ObjectiveId = TEXT("Graph.RootB");
	RootB.ObjectiveType = EHorrorCampaignObjectiveType::RecoverRelic;
	RootB.PromptText = FText::FromString(TEXT("回收右侧遗物"));

	FHorrorCampaignObjectiveDefinition LockedMerge;
	LockedMerge.ObjectiveId = TEXT("Graph.Merge");
	LockedMerge.ObjectiveType = EHorrorCampaignObjectiveType::RestorePower;
	LockedMerge.PromptText = FText::FromString(TEXT("汇合两路线索"));
	LockedMerge.PrerequisiteObjectiveIds = { RootA.ObjectiveId, RootB.ObjectiveId };

	FHorrorCampaignObjectiveDefinition OptionalLore;
	OptionalLore.ObjectiveId = TEXT("Graph.OptionalLore");
	OptionalLore.ObjectiveType = EHorrorCampaignObjectiveType::AcquireSignal;
	OptionalLore.PromptText = FText::FromString(TEXT("读取额外黑盒旁白"));
	OptionalLore.bRequiredForChapterCompletion = false;
	OptionalLore.bOptional = true;

	Chapter.Objectives = { RootA, RootB, LockedMerge, OptionalLore };

	FHorrorCampaignProgress Progress;
	Progress.ResetForChapter(Chapter);

	TestTrue(TEXT("The first dependency-free objective should be active."), Progress.CanCompleteObjective(RootA.ObjectiveId));
	TestTrue(TEXT("A second dependency-free objective should also be active."), Progress.CanCompleteObjective(RootB.ObjectiveId));
	TestFalse(TEXT("The merge objective should stay locked until both prerequisites are complete."), Progress.CanCompleteObjective(LockedMerge.ObjectiveId));
	TestTrue(TEXT("Optional lore objectives should be completable without blocking the chapter."), Progress.CanCompleteObjective(OptionalLore.ObjectiveId));

	TestTrue(TEXT("Optional objective completion should be accepted."), Progress.TryCompleteObjective(OptionalLore.ObjectiveId));
	TestFalse(TEXT("Optional objective alone should not complete the chapter."), Progress.IsChapterComplete());
	TestTrue(TEXT("Second root objective should complete before the first root."), Progress.TryCompleteObjective(RootB.ObjectiveId));
	TestFalse(TEXT("One missing prerequisite should keep the merge locked."), Progress.CanCompleteObjective(LockedMerge.ObjectiveId));
	TestTrue(TEXT("First root objective should complete after the second root."), Progress.TryCompleteObjective(RootA.ObjectiveId));
	TestTrue(TEXT("All merge prerequisites should unlock the merge objective."), Progress.CanCompleteObjective(LockedMerge.ObjectiveId));
	TestTrue(TEXT("Merge objective should complete after prerequisites."), Progress.TryCompleteObjective(LockedMerge.ObjectiveId));
	TestTrue(TEXT("Chapter should complete when all required graph objectives are done."), Progress.IsChapterComplete());

	return true;
}

bool FHorrorCampaignImportSaveStateRejectsDependencyBypassTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* ForestChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.ForestOfSpikes"));
	TestNotNull(TEXT("Forest chapter should exist for dependency-safe import coverage."), ForestChapter);
	if (!ForestChapter)
	{
		return false;
	}

	FHorrorCampaignSaveState SaveState;
	SaveState.ChapterId = ForestChapter->ChapterId;
	SaveState.CompletedObjectiveIds = {
		TEXT("Forest.ExtractBlackSeed")
	};
	SaveState.bBossDefeated = true;

	FHorrorCampaignProgress Progress;
	Progress.ImportSaveState(SaveState);

	TestFalse(TEXT("Import should reject a completed objective whose prerequisites are missing."), Progress.HasCompletedObjective(TEXT("Forest.ExtractBlackSeed")));
	TestFalse(TEXT("Rejected dependency bypass should leave the black seed objective locked."), Progress.CanCompleteObjective(TEXT("Forest.ExtractBlackSeed")));
	TestFalse(TEXT("Rejected dependency bypass should not complete the forest chapter."), Progress.IsChapterComplete());
	TestEqual(TEXT("Rejected dependency bypass should not count as campaign progress."), Progress.GetCompletedObjectiveCount(), 0);

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
