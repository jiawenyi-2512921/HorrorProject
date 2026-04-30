// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorMapChain.h"

#include "Misc/ConfigCacheIni.h"
#include "Misc/PackageName.h"

namespace
{
	const TArray<FHorrorMapChainEntry>& BuildMapChainEntries()
	{
		static const TArray<FHorrorMapChainEntry> Entries = {
			{
				TEXT("/Game/DeepWaterStation/Maps/DemoMap_VerticalSlice_Day1"),
				FText::FromString(TEXT("深水站"))
			},
			{
				TEXT("/Game/ForestOfSpikes/Levels/Level_ForestOfSpikes_Demo_Night"),
				FText::FromString(TEXT("尖刺林地"))
			},
			{
				TEXT("/Game/Scrapopolis/Levels/Level_Scrapopolis_Demo"),
				FText::FromString(TEXT("废铁村"))
			},
			{
				TEXT("/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_2_entrance"),
				FText::FromString(TEXT("地牢入口"))
			},
			{
				TEXT("/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_1_dungeon"),
				FText::FromString(TEXT("地下回廊"))
			},
			{
				TEXT("/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_3_hall"),
				FText::FromString(TEXT("机关大厅"))
			},
			{
				TEXT("/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_4_temple"),
				FText::FromString(TEXT("沉没神殿"))
			},
			{
				TEXT("/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_5_bossroom"),
				FText::FromString(TEXT("巨人王座厅"))
			},
			{
				TEXT("/Game/Bodycam_VHS_Effect/Maps/LVL_Showcase_01"),
				FText::FromString(TEXT("黑盒录像带"))
			}
		};

		return Entries;
	}

	FString ResolveConfiguredGameModeClassPath()
	{
		const TCHAR* GameMapsSettingsSection = TEXT("/Script/EngineSettings.GameMapsSettings");

		FString ConfiguredGameModeClassPath;
		if (GConfig)
		{
			GConfig->GetString(
				GameMapsSettingsSection,
				TEXT("GlobalDefaultGameMode"),
				ConfiguredGameModeClassPath,
				GEngineIni);
		}

		ConfiguredGameModeClassPath.TrimStartAndEndInline();
		ConfiguredGameModeClassPath.TrimQuotesInline();
		if (ConfiguredGameModeClassPath.IsEmpty() || ConfiguredGameModeClassPath == TEXT("None"))
		{
			return TEXT("/Script/HorrorProject.HorrorGameModeBase");
		}

		return ConfiguredGameModeClassPath;
	}

	FString StripPiePrefixFromShortName(const FString& ShortName)
	{
		TArray<FString> Parts;
		ShortName.ParseIntoArray(Parts, TEXT("_"), false);
		if (Parts.Num() < 3 || Parts[0] != TEXT("UEDPIE") || !Parts[1].IsNumeric())
		{
			return ShortName;
		}

		Parts.RemoveAt(0, 2, EAllowShrinking::No);
		return FString::Join(Parts, TEXT("_"));
	}

	FString StripPiePrefixFromPackage(FString PackageName)
	{
		FString PackagePath;
		FString ShortName;
		if (!PackageName.Split(TEXT("/"), &PackagePath, &ShortName, ESearchCase::CaseSensitive, ESearchDir::FromEnd))
		{
			return StripPiePrefixFromShortName(PackageName);
		}

		const FString StrippedShortName = StripPiePrefixFromShortName(ShortName);
		return FString::Printf(TEXT("%s/%s"), *PackagePath, *StrippedShortName);
	}
}

const TArray<FHorrorMapChainEntry>& FHorrorMapChain::GetEntries()
{
	return BuildMapChainEntries();
}

FString FHorrorMapChain::NormalizeMapPackageName(FString MapPackageName)
{
	MapPackageName.TrimStartAndEndInline();
	MapPackageName.TrimQuotesInline();
	MapPackageName.ReplaceInline(TEXT("\\"), TEXT("/"));

	const FString PackageName = FPackageName::ObjectPathToPackageName(MapPackageName);
	if (!PackageName.IsEmpty())
	{
		MapPackageName = PackageName;
	}

	return StripPiePrefixFromPackage(MapPackageName);
}

int32 FHorrorMapChain::FindMapIndex(const FString& MapPackageName)
{
	const FString NormalizedMapPackageName = NormalizeMapPackageName(MapPackageName);
	const FString ShortMapName = FPackageName::GetShortName(NormalizedMapPackageName);

	const TArray<FHorrorMapChainEntry>& Entries = GetEntries();
	for (int32 EntryIndex = 0; EntryIndex < Entries.Num(); ++EntryIndex)
	{
		const FString EntryPackageName = NormalizeMapPackageName(Entries[EntryIndex].MapPackageName);
		if (EntryPackageName == NormalizedMapPackageName)
		{
			return EntryIndex;
		}

		if (!ShortMapName.IsEmpty() && FPackageName::GetShortName(EntryPackageName) == ShortMapName)
		{
			return EntryIndex;
		}
	}

	return INDEX_NONE;
}

bool FHorrorMapChain::IsMapInChain(const FString& MapPackageName)
{
	return FindMapIndex(MapPackageName) != INDEX_NONE;
}

bool FHorrorMapChain::IsFinalMap(const FString& MapPackageName)
{
	const int32 MapIndex = FindMapIndex(MapPackageName);
	return MapIndex != INDEX_NONE && MapIndex == GetEntries().Num() - 1;
}

bool FHorrorMapChain::TryGetNextMapPackage(const FString& CurrentMapPackageName, FString& OutNextMapPackage)
{
	OutNextMapPackage.Reset();

	const int32 CurrentMapIndex = FindMapIndex(CurrentMapPackageName);
	if (CurrentMapIndex == INDEX_NONE || CurrentMapIndex >= GetEntries().Num() - 1)
	{
		return false;
	}

	OutNextMapPackage = GetEntries()[CurrentMapIndex + 1].MapPackageName;
	return true;
}

FString FHorrorMapChain::BuildTravelOptionsForConfiguredGameMode()
{
	return FString::Printf(TEXT("game=%s"), *ResolveConfiguredGameModeClassPath());
}
