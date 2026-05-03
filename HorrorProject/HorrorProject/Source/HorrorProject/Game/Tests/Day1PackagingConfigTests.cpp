// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Game/HorrorCampaign.h"
#include "Game/HorrorMapChain.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Misc/AutomationTest.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"

namespace
{
	const TCHAR* ExpectedDay1MapPackage = TEXT("/Game/DeepWaterStation/Maps/DemoMap_VerticalSlice_Day1");
	const TCHAR* GameMapsSettingsSection = TEXT("/Script/EngineSettings.GameMapsSettings");
	const TCHAR* ProjectPackagingSettingsSection = TEXT("/Script/UnrealEd.ProjectPackagingSettings");

	FString NormalizeLongPackagePath(FString Path)
	{
		Path.TrimStartAndEndInline();
		Path.TrimQuotesInline();

		const FString PackageName = FPackageName::ObjectPathToPackageName(Path);
		return PackageName.IsEmpty() ? Path : PackageName;
	}

	bool TryLoadConfigFile(const FString& RelativeConfigPath, FString& OutConfigText)
	{
		const FString ConfigPath = FPaths::Combine(FPaths::ProjectDir(), RelativeConfigPath);
		return FFileHelper::LoadFileToString(OutConfigText, *ConfigPath);
	}

	TArray<FString> ExtractSectionLines(
		const FString& ConfigText,
		const FString& SectionName)
	{
		TArray<FString> SectionLines;
		TArray<FString> Lines;
		ConfigText.ParseIntoArrayLines(Lines);

		const FString SectionHeader = FString::Printf(TEXT("[%s]"), *SectionName);
		bool bInSection = false;
		for (const FString& RawLine : Lines)
		{
			FString Line = RawLine;
			Line.TrimStartAndEndInline();

			if (Line.StartsWith(TEXT("[")) && Line.EndsWith(TEXT("]")))
			{
				bInSection = Line.Equals(SectionHeader, ESearchCase::CaseSensitive);
				continue;
			}

			if (bInSection)
			{
				SectionLines.Add(Line);
			}
		}

		return SectionLines;
	}

	TArray<FString> ExtractStructArrayPathValues(
		const TArray<FString>& Lines,
		const FString& ArrayKey,
		const FString& StructPathKey)
	{
		TArray<FString> Values;

		const FString Prefix = FString::Printf(TEXT("+%s=("), *ArrayKey);
		const FString PathPrefix = FString::Printf(TEXT("%s=\""), *StructPathKey);
		for (FString Line : Lines)
		{
			Line.TrimStartAndEndInline();
			if (!Line.StartsWith(Prefix))
			{
				continue;
			}

			const int32 PathKeyIndex = Line.Find(PathPrefix, ESearchCase::CaseSensitive);
			if (PathKeyIndex == INDEX_NONE)
			{
				continue;
			}

			const int32 PathStart = PathKeyIndex + PathPrefix.Len();
			const int32 PathEnd = Line.Find(TEXT("\""), ESearchCase::CaseSensitive, ESearchDir::FromStart, PathStart);
			if (PathEnd > PathStart)
			{
				Values.Add(Line.Mid(PathStart, PathEnd - PathStart));
			}
		}

		return Values;
	}

	void ParseCsvLineRespectingQuotes(const FString& Line, TArray<FString>& OutColumns)
	{
		OutColumns.Reset();

		FString CurrentColumn;
		bool bInQuotes = false;
		for (int32 CharacterIndex = 0; CharacterIndex < Line.Len(); ++CharacterIndex)
		{
			const TCHAR Character = Line[CharacterIndex];
			if (Character == TEXT('"'))
			{
				bInQuotes = !bInQuotes;
				continue;
			}

			if (Character == TEXT(',') && !bInQuotes)
			{
				CurrentColumn.TrimStartAndEndInline();
				OutColumns.Add(CurrentColumn);
				CurrentColumn.Reset();
				continue;
			}

			CurrentColumn.AppendChar(Character);
		}

		CurrentColumn.TrimStartAndEndInline();
		OutColumns.Add(CurrentColumn);
	}

	TSet<FName> BuildCampaignObjectiveIdSet()
	{
		TSet<FName> ObjectiveIds;
		for (const FHorrorCampaignChapterDefinition& Chapter : FHorrorCampaign::GetChapters())
		{
			for (const FHorrorCampaignObjectiveDefinition& Objective : Chapter.Objectives)
			{
				ObjectiveIds.Add(Objective.ObjectiveId);
			}
		}

		return ObjectiveIds;
	}

	TArray<FString> ParseRequiredObjectiveColumn(const FString& RequiredObjectivesColumn)
	{
		TArray<FString> RequiredObjectives;
		RequiredObjectivesColumn.ParseIntoArray(RequiredObjectives, TEXT(";"), true);
		for (FString& RequiredObjective : RequiredObjectives)
		{
			RequiredObjective.TrimStartAndEndInline();
		}
		RequiredObjectives.RemoveAll([](const FString& RequiredObjective)
		{
			return RequiredObjective.IsEmpty();
		});
		return RequiredObjectives;
	}

	TArray<FString> BuildRequiredObjectiveIdsInChapterOrder(const FHorrorCampaignChapterDefinition& Chapter)
	{
		TArray<FString> ObjectiveIds;
		for (const FHorrorCampaignObjectiveDefinition& Objective : Chapter.Objectives)
		{
			if (Objective.bRequiredForChapterCompletion)
			{
				ObjectiveIds.Add(Objective.ObjectiveId.ToString());
			}
		}
		return ObjectiveIds;
	}

	const FHorrorCampaignObjectiveDefinition* FindLastRequiredObjective(const FHorrorCampaignChapterDefinition& Chapter)
	{
		for (int32 ObjectiveIndex = Chapter.Objectives.Num() - 1; ObjectiveIndex >= 0; --ObjectiveIndex)
		{
			const FHorrorCampaignObjectiveDefinition& Objective = Chapter.Objectives[ObjectiveIndex];
			if (Objective.bRequiredForChapterCompletion)
			{
				return &Objective;
			}
		}

		return nullptr;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDay1PackagingConfigCooksFinaleMapTest,
	"HorrorProject.Game.Day1.PackagingConfig.CooksFinaleMap",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDay1PackagingConfigCooksFinaleMapTest::RunTest(const FString& Parameters)
{
	FString GameDefaultMap;
	TestTrue(
		TEXT("GameDefaultMap should be present in merged engine config."),
		GConfig && GConfig->GetString(GameMapsSettingsSection, TEXT("GameDefaultMap"), GameDefaultMap, GEngineIni));

	const FString NormalizedGameDefaultMap = NormalizeLongPackagePath(GameDefaultMap);
	const TArray<FHorrorMapChainEntry>& MapChainEntries = FHorrorMapChain::GetEntries();
	TestTrue(TEXT("The current campaign map chain should define an opening chapter."), MapChainEntries.Num() > 0);
	if (MapChainEntries.IsEmpty())
	{
		return false;
	}

	TestEqual(
		TEXT("Packaged game should boot into the current campaign opening map."),
		NormalizedGameDefaultMap,
		FHorrorMapChain::NormalizeMapPackageName(MapChainEntries[0].MapPackageName));
	TestTrue(
		TEXT("The Day1 vertical-slice map should remain the campaign finale package."),
		FHorrorMapChain::IsFinalMap(ExpectedDay1MapPackage));

	FString DefaultGameConfigText;
	TestTrue(
		TEXT("DefaultGame.ini should be readable for packaging map contract validation."),
		TryLoadConfigFile(TEXT("Config/DefaultGame.ini"), DefaultGameConfigText));

	const TArray<FString> PackagingSettingsLines = ExtractSectionLines(DefaultGameConfigText, ProjectPackagingSettingsSection);
	TestTrue(
		TEXT("DefaultGame.ini should define ProjectPackagingSettings for the Day1 cook contract."),
		PackagingSettingsLines.Num() > 0);

	const TArray<FString> MapsToCook = ExtractStructArrayPathValues(PackagingSettingsLines, TEXT("MapsToCook"), TEXT("FilePath"));
	TestTrue(
		TEXT("MapsToCook should include the Day1 vertical-slice map package."),
		MapsToCook.ContainsByPredicate([](const FString& MapToCook)
		{
			return NormalizeLongPackagePath(MapToCook) == ExpectedDay1MapPackage;
		}));

	const TArray<FString> DirectoriesToAlwaysCook = ExtractStructArrayPathValues(PackagingSettingsLines, TEXT("DirectoriesToAlwaysCook"), TEXT("Path"));
	TestTrue(
		TEXT("DeepWaterStation content should always cook for the Day1 map."),
		DirectoriesToAlwaysCook.Contains(TEXT("/Game/DeepWaterStation")));

	TestTrue(
		TEXT("The Day1 vertical-slice map package should exist on disk or in the asset registry."),
		FPackageName::DoesPackageExist(ExpectedDay1MapPackage));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMapChainPackagingConfigCooksAllMapsTest,
	"HorrorProject.Game.MapChain.PackagingConfig.CooksAllMaps",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMapChainPackagingConfigCooksAllMapsTest::RunTest(const FString& Parameters)
{
	FString DefaultGameConfigText;
	TestTrue(
		TEXT("DefaultGame.ini should be readable for map-chain packaging validation."),
		TryLoadConfigFile(TEXT("Config/DefaultGame.ini"), DefaultGameConfigText));

	const TArray<FString> PackagingSettingsLines = ExtractSectionLines(DefaultGameConfigText, ProjectPackagingSettingsSection);
	TestTrue(
		TEXT("DefaultGame.ini should define ProjectPackagingSettings for the map-chain cook contract."),
		PackagingSettingsLines.Num() > 0);

	const TArray<FString> MapsToCook = ExtractStructArrayPathValues(PackagingSettingsLines, TEXT("MapsToCook"), TEXT("FilePath"));
	TArray<FString> MapChainPackagesInCookOrder;
	TArray<FString> ExpectedMapChainPackages;
	for (const FHorrorMapChainEntry& Entry : FHorrorMapChain::GetEntries())
	{
		ExpectedMapChainPackages.Add(Entry.MapPackageName);
		TestTrue(
			FString::Printf(TEXT("MapsToCook should include map-chain package %s."), *Entry.MapPackageName),
			MapsToCook.ContainsByPredicate([&Entry](const FString& MapToCook)
			{
				return NormalizeLongPackagePath(MapToCook) == Entry.MapPackageName;
			}));
	}
	for (const FString& MapToCook : MapsToCook)
	{
		const FString NormalizedMapToCook = NormalizeLongPackagePath(MapToCook);
		if (FHorrorMapChain::IsMapInChain(NormalizedMapToCook))
		{
			MapChainPackagesInCookOrder.Add(NormalizedMapToCook);
		}
	}

	TestEqual(
		TEXT("MapsToCook should list playable map-chain packages in the same order as the runtime story chain."),
		MapChainPackagesInCookOrder,
		ExpectedMapChainPackages);

	TestFalse(
		TEXT("MapsToCook should not include asset-library-only maps in the playable chain package list."),
		MapsToCook.ContainsByPredicate([](const FString& MapToCook)
		{
			return NormalizeLongPackagePath(MapToCook).Contains(TEXT("Asset_Library"));
		}));
	TestFalse(
		TEXT("MapsToCook should not include pipe zoo showcase maps in the playable chain package list."),
		MapsToCook.ContainsByPredicate([](const FString& MapToCook)
		{
			return NormalizeLongPackagePath(MapToCook).Contains(TEXT("IndustrialPipesM"));
		}));
	TestFalse(
		TEXT("MapsToCook should not include Stone Golem model showcase maps in the playable chain package list."),
		MapsToCook.ContainsByPredicate([](const FString& MapToCook)
		{
			return NormalizeLongPackagePath(MapToCook).Contains(TEXT("Stone_Golem/map"));
		}));
	TestFalse(
		TEXT("MapsToCook should not include Advanced Universal Spawner showcase maps in the playable chain package list."),
		MapsToCook.ContainsByPredicate([](const FString& MapToCook)
		{
			return NormalizeLongPackagePath(MapToCook).Contains(TEXT("AdvancedUniversalSpawner"));
		}));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMapChainTransitionTriggersUseCurrentCampaignMapsTest,
	"HorrorProject.Game.MapChain.TransitionTriggers.UseCurrentCampaignMaps",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMapChainTransitionTriggersUseCurrentCampaignMapsTest::RunTest(const FString& Parameters)
{
	FString TransitionConfigText;
	TestTrue(
		TEXT("TransitionTriggers.csv should be readable for stale level-flow validation."),
		TryLoadConfigFile(TEXT("Config/TransitionTriggers.csv"), TransitionConfigText));

	TestFalse(TEXT("Transition triggers should not reference the deprecated hospital prototype."), TransitionConfigText.Contains(TEXT("Hospital")));
	TestFalse(TEXT("Transition triggers should not reference the deprecated basement prototype."), TransitionConfigText.Contains(TEXT("Basement")));
	TestFalse(TEXT("Transition triggers should not reference the deprecated ending prototype."), TransitionConfigText.Contains(TEXT("Level_04_Ending")));

	TestTrue(TEXT("Transition triggers should begin from the Deep Water opening."), TransitionConfigText.Contains(TEXT("Trigger_DeepWater_To_Forest")));
	TestTrue(TEXT("Transition triggers should route the campaign into Scrapopolis."), TransitionConfigText.Contains(TEXT("Trigger_Forest_To_Scrapopolis")));
	TestTrue(TEXT("Transition triggers should route the campaign into the dungeon entrance."), TransitionConfigText.Contains(TEXT("Trigger_Scrapopolis_To_DungeonEntrance")));
	TestTrue(TEXT("Transition triggers should end at the black-box VHS map."), TransitionConfigText.Contains(TEXT("Trigger_BossRoom_To_BlackBoxVHS")));

	for (int32 EntryIndex = 1; EntryIndex < FHorrorMapChain::GetEntries().Num(); ++EntryIndex)
	{
		const FString& MapPackageName = FHorrorMapChain::GetEntries()[EntryIndex].MapPackageName;
		TestTrue(
			FString::Printf(TEXT("TransitionTriggers.csv should reference current map-chain package %s."), *MapPackageName),
			TransitionConfigText.Contains(MapPackageName));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMapChainTransitionTriggersReferenceCurrentCampaignObjectivesTest,
	"HorrorProject.Game.MapChain.TransitionTriggers.ReferenceCurrentCampaignObjectives",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMapChainTransitionTriggersReferenceCurrentCampaignObjectivesTest::RunTest(const FString& Parameters)
{
	FString TransitionConfigText;
	TestTrue(
		TEXT("TransitionTriggers.csv should be readable for objective prerequisite validation."),
		TryLoadConfigFile(TEXT("Config/TransitionTriggers.csv"), TransitionConfigText));

	TArray<FString> Lines;
	TransitionConfigText.ParseIntoArrayLines(Lines);
	TestTrue(TEXT("TransitionTriggers.csv should contain a header and at least one transition."), Lines.Num() > 1);
	if (Lines.Num() <= 1)
	{
		return false;
	}

	const TSet<FName> CampaignObjectiveIds = BuildCampaignObjectiveIdSet();
	for (int32 LineIndex = 1; LineIndex < Lines.Num(); ++LineIndex)
	{
		TArray<FString> Columns;
		ParseCsvLineRespectingQuotes(Lines[LineIndex], Columns);
		TestTrue(
			FString::Printf(TEXT("Transition trigger row %d should preserve the expected CSV column count."), LineIndex + 1),
			Columns.Num() >= 11);
		if (Columns.Num() < 11)
		{
			continue;
		}

		const FString TriggerName = Columns[0];
		const FString RequiredObjectivesColumn = Columns[10];
		TArray<FString> RequiredObjectives;
		RequiredObjectivesColumn.ParseIntoArray(RequiredObjectives, TEXT(";"), true);
		for (FString RequiredObjective : RequiredObjectives)
		{
			RequiredObjective.TrimStartAndEndInline();
			if (RequiredObjective.IsEmpty())
			{
				continue;
			}

			TestTrue(
				FString::Printf(TEXT("%s should reference an existing campaign objective id: %s."), *TriggerName, *RequiredObjective),
				CampaignObjectiveIds.Contains(FName(*RequiredObjective)));
		}
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMapChainTransitionTriggersGateOnSourceChapterFinalObjectiveTest,
	"HorrorProject.Game.MapChain.TransitionTriggers.GateOnSourceChapterFinalObjective",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMapChainTransitionTriggerTargetsMatchMapChainNextMapsTest,
	"HorrorProject.Game.Campaign.TransitionTriggerTargetsMatchMapChainNextMaps",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMapChainTransitionTriggersMatchChapterRequiredObjectivesTest,
	"HorrorProject.Game.Campaign.TransitionTriggersMatchChapterRequiredObjectives",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMapChainTransitionTriggersGateOnSourceChapterFinalObjectiveTest::RunTest(const FString& Parameters)
{
	FString TransitionConfigText;
	TestTrue(
		TEXT("TransitionTriggers.csv should be readable for source chapter gating validation."),
		TryLoadConfigFile(TEXT("Config/TransitionTriggers.csv"), TransitionConfigText));

	TArray<FString> Lines;
	TransitionConfigText.ParseIntoArrayLines(Lines);
	TestTrue(TEXT("TransitionTriggers.csv should contain a header and at least one transition."), Lines.Num() > 1);
	if (Lines.Num() <= 1)
	{
		return false;
	}

	const TArray<FHorrorCampaignChapterDefinition>& Chapters = FHorrorCampaign::GetChapters();
	for (int32 LineIndex = 1; LineIndex < Lines.Num(); ++LineIndex)
	{
		TArray<FString> Columns;
		ParseCsvLineRespectingQuotes(Lines[LineIndex], Columns);
		if (Columns.Num() < 11)
		{
			continue;
		}

		const FString TriggerName = Columns[0];
		const FString TargetLevel = NormalizeLongPackagePath(Columns[2]);
		const FString RequiredObjectivesColumn = Columns[10];
		TArray<FString> RequiredObjectives;
		RequiredObjectivesColumn.ParseIntoArray(RequiredObjectives, TEXT(";"), true);

		const int32 TargetMapIndex = FHorrorMapChain::FindMapIndex(TargetLevel);
		TestTrue(
			FString::Printf(TEXT("%s should target a current map-chain entry."), *TriggerName),
			TargetMapIndex > 0);
		TestTrue(
			FString::Printf(TEXT("%s should have a source campaign chapter before its target map."), *TriggerName),
			Chapters.IsValidIndex(TargetMapIndex - 1));
		if (TargetMapIndex <= 0 || !Chapters.IsValidIndex(TargetMapIndex - 1))
		{
			continue;
		}

		const FHorrorCampaignChapterDefinition& SourceChapter = Chapters[TargetMapIndex - 1];
		const FHorrorCampaignObjectiveDefinition* FinalObjective = FindLastRequiredObjective(SourceChapter);
		TestNotNull(
			FString::Printf(TEXT("%s source chapter should define a final required objective."), *TriggerName),
			FinalObjective);
		if (!FinalObjective)
		{
			continue;
		}

		TestTrue(
			FString::Printf(TEXT("%s should wait for the source chapter final required objective %s."), *TriggerName, *FinalObjective->ObjectiveId.ToString()),
			RequiredObjectives.ContainsByPredicate([FinalObjective](FString RequiredObjective)
			{
				RequiredObjective.TrimStartAndEndInline();
				return FName(*RequiredObjective) == FinalObjective->ObjectiveId;
			}));
	}

	return true;
}

bool FMapChainTransitionTriggerTargetsMatchMapChainNextMapsTest::RunTest(const FString& Parameters)
{
	FString TransitionConfigText;
	TestTrue(
		TEXT("TransitionTriggers.csv should be readable for map-chain target validation."),
		TryLoadConfigFile(TEXT("Config/TransitionTriggers.csv"), TransitionConfigText));

	TArray<FString> Lines;
	TransitionConfigText.ParseIntoArrayLines(Lines);
	const TArray<FHorrorMapChainEntry>& Entries = FHorrorMapChain::GetEntries();
	TestEqual(
		TEXT("TransitionTriggers.csv should define exactly one transition for each non-final map-chain entry."),
		Lines.Num() - 1,
		FMath::Max(0, Entries.Num() - 1));
	if (Lines.Num() <= 1)
	{
		return false;
	}

	for (int32 LineIndex = 1; LineIndex < Lines.Num(); ++LineIndex)
	{
		TArray<FString> Columns;
		ParseCsvLineRespectingQuotes(Lines[LineIndex], Columns);
		TestTrue(
			FString::Printf(TEXT("Transition trigger row %d should preserve the expected CSV column count."), LineIndex + 1),
			Columns.Num() >= 11);
		if (Columns.Num() < 11 || !Entries.IsValidIndex(LineIndex))
		{
			continue;
		}

		const FString TriggerName = Columns[0];
		const FString TargetLevel = NormalizeLongPackagePath(Columns[2]);
		TestEqual(
			FString::Printf(TEXT("%s should target the next runtime map-chain entry."), *TriggerName),
			TargetLevel,
			Entries[LineIndex].MapPackageName);
	}

	return true;
}

bool FMapChainTransitionTriggersMatchChapterRequiredObjectivesTest::RunTest(const FString& Parameters)
{
	FString TransitionConfigText;
	TestTrue(
		TEXT("TransitionTriggers.csv should be readable for required objective contract validation."),
		TryLoadConfigFile(TEXT("Config/TransitionTriggers.csv"), TransitionConfigText));

	TArray<FString> Lines;
	TransitionConfigText.ParseIntoArrayLines(Lines);
	const TArray<FHorrorCampaignChapterDefinition>& Chapters = FHorrorCampaign::GetChapters();
	TestEqual(
		TEXT("Transition trigger rows should align one-to-one with non-final campaign chapters."),
		Lines.Num() - 1,
		FMath::Max(0, Chapters.Num() - 1));
	if (Lines.Num() <= 1)
	{
		return false;
	}

	for (int32 LineIndex = 1; LineIndex < Lines.Num(); ++LineIndex)
	{
		TArray<FString> Columns;
		ParseCsvLineRespectingQuotes(Lines[LineIndex], Columns);
		TestTrue(
			FString::Printf(TEXT("Transition trigger row %d should preserve the expected CSV column count."), LineIndex + 1),
			Columns.Num() >= 11);
		if (Columns.Num() < 11 || !Chapters.IsValidIndex(LineIndex - 1))
		{
			continue;
		}

		const FString TriggerName = Columns[0];
		const FHorrorCampaignChapterDefinition& SourceChapter = Chapters[LineIndex - 1];
		TestFalse(
			FString::Printf(TEXT("%s should not be authored from the final chapter."), *TriggerName),
			SourceChapter.bIsFinalChapter);

		const TArray<FString> RequiredObjectives = ParseRequiredObjectiveColumn(Columns[10]);
		const TArray<FString> ExpectedRequiredObjectives = BuildRequiredObjectiveIdsInChapterOrder(SourceChapter);
		TestEqual(
			FString::Printf(TEXT("%s should list every required source-chapter objective in authored order."), *TriggerName),
			RequiredObjectives,
			ExpectedRequiredObjectives);
	}

	return true;
}

#endif
