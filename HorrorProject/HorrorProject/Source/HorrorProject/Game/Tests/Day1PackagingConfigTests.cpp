// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"

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
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDay1PackagingConfigCooksDefaultMapTest,
	"HorrorProject.Game.Day1.PackagingConfig.CooksDefaultMap",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDay1PackagingConfigCooksDefaultMapTest::RunTest(const FString& Parameters)
{
	FString GameDefaultMap;
	TestTrue(
		TEXT("GameDefaultMap should be present in merged engine config."),
		GConfig && GConfig->GetString(GameMapsSettingsSection, TEXT("GameDefaultMap"), GameDefaultMap, GEngineIni));

	const FString NormalizedGameDefaultMap = NormalizeLongPackagePath(GameDefaultMap);
	TestEqual(
		TEXT("Packaged game should boot directly into the Day1 vertical-slice map."),
		NormalizedGameDefaultMap,
		FString(ExpectedDay1MapPackage));

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

#endif
