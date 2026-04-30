// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorMapChain.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Misc/AutomationTest.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/PackageName.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorMapChainResolvesImportedMapOrderTest,
	"HorrorProject.Game.MapChain.ResolvesImportedMapOrder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorMapChainResolvesImportedMapOrderTest::RunTest(const FString& Parameters)
{
	const TArray<FHorrorMapChainEntry>& Entries = FHorrorMapChain::GetEntries();
	TestTrue(TEXT("The multi-map prototype should include the existing Day1 start map."), Entries.Num() > 0);
	TestEqual(
		TEXT("The map chain should now begin in Deep Water Station as the investigation opening."),
		Entries[0].MapPackageName,
		FString(TEXT("/Game/DeepWaterStation/Maps/DemoMap_VerticalSlice_Day1")));

	FString NextMapPackage;
	TestTrue(
		TEXT("The Deep Water Station opening should resolve a playable follow-up map."),
		FHorrorMapChain::TryGetNextMapPackage(
			TEXT("/Game/DeepWaterStation/Maps/DemoMap_VerticalSlice_Day1.DemoMap_VerticalSlice_Day1"),
			NextMapPackage));
	TestEqual(
		TEXT("The first transition should move from Deep Water Station into the night forest map."),
		NextMapPackage,
		FString(TEXT("/Game/ForestOfSpikes/Levels/Level_ForestOfSpikes_Demo_Night")));

	TestTrue(
		TEXT("PIE-prefixed short map names should still resolve during editor play testing."),
		FHorrorMapChain::TryGetNextMapPackage(TEXT("UEDPIE_0_DemoMap_VerticalSlice_Day1"), NextMapPackage));
	TestEqual(
		TEXT("The Deep Water Station opening should lead into the night forest map."),
		NextMapPackage,
		FString(TEXT("/Game/ForestOfSpikes/Levels/Level_ForestOfSpikes_Demo_Night")));

	TestTrue(
		TEXT("Scrapopolis should skip non-playable asset-library maps and lead into the dungeon entrance."),
		FHorrorMapChain::TryGetNextMapPackage(TEXT("/Game/Scrapopolis/Levels/Level_Scrapopolis_Demo"), NextMapPackage));
	TestEqual(
		TEXT("The fifth playable stop should be the dungeon entrance, not an asset library."),
		NextMapPackage,
		FString(TEXT("/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_2_entrance")));

	for (const FHorrorMapChainEntry& Entry : Entries)
	{
		TestFalse(
			FString::Printf(TEXT("Map-chain package should not include non-playable asset-library maps: %s."), *Entry.MapPackageName),
			Entry.MapPackageName.Contains(TEXT("Asset_Library")));
		TestFalse(
			FString::Printf(TEXT("Map-chain package should not include pipe zoo showcase maps: %s."), *Entry.MapPackageName),
			Entry.MapPackageName.Contains(TEXT("IndustrialPipesM")));
		TestFalse(
			FString::Printf(TEXT("Map-chain package should not include Stone Golem model showcase maps: %s."), *Entry.MapPackageName),
			Entry.MapPackageName.Contains(TEXT("Stone_Golem/map")));
		TestFalse(
			FString::Printf(TEXT("Map-chain package should not include Advanced Universal Spawner showcase maps: %s."), *Entry.MapPackageName),
			Entry.MapPackageName.Contains(TEXT("AdvancedUniversalSpawner")));
	}

	const FString FinalMapPackage = Entries.Last().MapPackageName;
	TestEqual(
		TEXT("The final chapter should now be the Bodycam VHS black-box ending."),
		FinalMapPackage,
		FString(TEXT("/Game/Bodycam_VHS_Effect/Maps/LVL_Showcase_01")));
	TestTrue(TEXT("Bodycam VHS should be treated as the final black-box ending map."), FHorrorMapChain::IsFinalMap(FinalMapPackage));
	TestFalse(TEXT("The temporary ending should not resolve another map yet."), FHorrorMapChain::TryGetNextMapPackage(FinalMapPackage, NextMapPackage));

	for (const FHorrorMapChainEntry& Entry : Entries)
	{
		TestTrue(
			FString::Printf(TEXT("Map-chain package should exist: %s."), *Entry.MapPackageName),
			FPackageName::DoesPackageExist(Entry.MapPackageName));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorMapChainDefaultMapMatchesFirstChapterTest,
	"HorrorProject.Game.MapChain.DefaultMapMatchesFirstChapter",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorMapChainDefaultMapMatchesFirstChapterTest::RunTest(const FString& Parameters)
{
	const TArray<FHorrorMapChainEntry>& Entries = FHorrorMapChain::GetEntries();
	TestTrue(TEXT("Map chain should have a first chapter."), Entries.Num() > 0);
	if (Entries.IsEmpty())
	{
		return false;
	}

	FString GameDefaultMap;
	if (GConfig)
	{
		GConfig->GetString(
			TEXT("/Script/EngineSettings.GameMapsSettings"),
			TEXT("GameDefaultMap"),
			GameDefaultMap,
			GEngineIni);
	}

	TestEqual(
		TEXT("The packaged game should boot into the first campaign chapter after the startup movie."),
		FHorrorMapChain::NormalizeMapPackageName(GameDefaultMap),
		FHorrorMapChain::NormalizeMapPackageName(Entries[0].MapPackageName));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorMapChainUsesConfiguredGameModeTravelOptionsTest,
	"HorrorProject.Game.MapChain.UsesConfiguredGameModeTravelOptions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorMapChainUsesConfiguredGameModeTravelOptionsTest::RunTest(const FString& Parameters)
{
	const FString TravelOptions = FHorrorMapChain::BuildTravelOptionsForConfiguredGameMode();
	TestEqual(
		TEXT("Map-chain travel should keep the configured Blueprint game mode so imported maps spawn the playable horror pawn."),
		TravelOptions,
		FString(TEXT("game=/Game/Horror/BP_HorrorGameModeBase.BP_HorrorGameModeBase_C")));

	return true;
}

#endif
