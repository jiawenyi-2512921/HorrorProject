// Copyright Epic Games, Inc. All Rights Reserved.

#include "HorrorEditorUI.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SSplitter.h"
#include "Framework/Docking/TabManager.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Tools/AssetValidator.h"
#include "Tools/MaterialOptimizer.h"
#include "Tools/LODGenerator.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMemory.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "HorrorEditorUI"

static const FName HorrorDashboardTabName("HorrorDashboard");

namespace HorrorEditorDashboard
{
	constexpr int32 DashboardTitleFontSize = 24;
	constexpr int32 SectionTitleFontSize = 16;
	constexpr float BytesPerKilobyte = 1024.0f;

	bool FindLatestFile(const FString& Directory, const FString& Pattern, FString& OutLatestFile)
	{
		TArray<FString> FileNames;
		IFileManager::Get().FindFiles(FileNames, *FPaths::Combine(Directory, Pattern), true, false);

		FDateTime LatestTimestamp = FDateTime::MinValue();
		FString Candidate;
		for (const FString& FileName : FileNames)
		{
			Candidate = FPaths::Combine(Directory, FileName);
			const FDateTime CandidateTimestamp = IFileManager::Get().GetTimeStamp(*Candidate);
			if (CandidateTimestamp > LatestTimestamp)
			{
				LatestTimestamp = CandidateTimestamp;
				OutLatestFile = Candidate;
			}
		}

		return !OutLatestFile.IsEmpty();
	}

	FString ReadLastLines(const FString& FilePath, int32 MaxLines)
	{
		FString Contents;
		if (!FFileHelper::LoadFileToString(Contents, *FilePath))
		{
			return FString::Printf(TEXT("Unable to read %s"), *FilePath);
		}

		TArray<FString> Lines;
		Contents.ParseIntoArrayLines(Lines, false);
		const int32 FirstLine = FMath::Max(0, Lines.Num() - MaxLines);

		TArray<FString> TailLines;
		for (int32 LineIndex = FirstLine; LineIndex < Lines.Num(); ++LineIndex)
		{
			TailLines.Add(Lines[LineIndex]);
		}

		return FString::Join(TailLines, TEXT("\n"));
	}

	FString ReadFirstMatchingLine(const FString& Contents, const FString& Token)
	{
		TArray<FString> Lines;
		Contents.ParseIntoArrayLines(Lines, false);
		for (const FString& Line : Lines)
		{
			if (Line.Contains(Token))
			{
				return Line.TrimStartAndEnd();
			}
		}

		return FString();
	}
}

void SHorrorEditorDashboard::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)

		// Header
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("DashboardTitle", "Horror Project Developer Dashboard"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", HorrorEditorDashboard::DashboardTitleFontSize))
		]

		// Main Content
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SSplitter)
			.Orientation(Orient_Horizontal)

			+ SSplitter::Slot()
			.Value(0.3f)
			[
				CreateToolPanel()
			]

			+ SSplitter::Slot()
			.Value(0.7f)
			[
				CreateInfoPanel()
			]
		]
	];
}

TSharedRef<SWidget> SHorrorEditorDashboard::CreateToolPanel()
{
	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(5)
			[
				CreatePerformanceMonitor()
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(5)
			[
				CreateToolButtons()
			]
		];
}

TSharedRef<SWidget> SHorrorEditorDashboard::CreateToolButtons()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		[
			SNew(SButton)
			.Text(LOCTEXT("RunTests", "Run All Tests"))
			.OnClicked(this, &SHorrorEditorDashboard::OnRunTests)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		[
			SNew(SButton)
			.Text(LOCTEXT("ValidateAssets", "Validate Assets"))
			.OnClicked(this, &SHorrorEditorDashboard::OnValidateAssets)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		[
			SNew(SButton)
			.Text(LOCTEXT("OptimizeMaterials", "Optimize Materials"))
			.OnClicked(this, &SHorrorEditorDashboard::OnOptimizeMaterials)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		[
			SNew(SButton)
			.Text(LOCTEXT("GenerateLODs", "Generate LODs"))
			.OnClicked(this, &SHorrorEditorDashboard::OnGenerateLODs)
		];
}

TSharedRef<SWidget> SHorrorEditorDashboard::CreateInfoPanel()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(0.5f)
		[
			CreateLogViewer()
		]

		+ SVerticalBox::Slot()
		.FillHeight(0.5f)
		[
			CreateBuildStatus()
		];
}

TSharedRef<SWidget> SHorrorEditorDashboard::CreatePerformanceMonitor()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("PerformanceTitle", "Performance Monitor"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", HorrorEditorDashboard::SectionTitleFontSize))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(STextBlock)
			.Text(this, &SHorrorEditorDashboard::GetFPSText)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(STextBlock)
			.Text(this, &SHorrorEditorDashboard::GetMemoryText)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(STextBlock)
			.Text(this, &SHorrorEditorDashboard::GetFrameTimeText)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(SButton)
			.Text(LOCTEXT("RefreshStats", "Refresh"))
			.OnClicked(this, &SHorrorEditorDashboard::OnRefreshStats)
		];
}

TSharedRef<SWidget> SHorrorEditorDashboard::CreateLogViewer()
{
	return SNew(SBox)
		.Padding(5)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LogViewer", "Log Viewer"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", HorrorEditorDashboard::SectionTitleFontSize))
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(this, &SHorrorEditorDashboard::GetLatestLogText)
				]
			]
		];
}

TSharedRef<SWidget> SHorrorEditorDashboard::CreateBuildStatus()
{
	return SNew(SBox)
		.Padding(5)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("BuildStatus", "Build Status"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", HorrorEditorDashboard::SectionTitleFontSize))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(5)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(this, &SHorrorEditorDashboard::GetBuildStatusText)
			]
		];
}

FReply SHorrorEditorDashboard::OnRunTests()
{
	UE_LOG(LogTemp, Log, TEXT("Running all tests..."));
	return FReply::Handled();
}

FReply SHorrorEditorDashboard::OnValidateAssets()
{
	UAssetValidator::ValidateAllAssets();
	return FReply::Handled();
}

FReply SHorrorEditorDashboard::OnOptimizeMaterials()
{
	UMaterialOptimizer::OptimizeMaterials();
	return FReply::Handled();
}

FReply SHorrorEditorDashboard::OnGenerateLODs()
{
	ULODGenerator::GenerateLODsForSelection();
	return FReply::Handled();
}

FReply SHorrorEditorDashboard::OnRefreshStats()
{
	UE_LOG(LogTemp, Log, TEXT("Refreshing performance stats..."));
	return FReply::Handled();
}

FText SHorrorEditorDashboard::GetFPSText() const
{
	return FText::FromString(FString::Printf(TEXT("FPS: %.1f"), 1.0f / FApp::GetDeltaTime()));
}

FText SHorrorEditorDashboard::GetMemoryText() const
{
	FPlatformMemoryStats Stats = FPlatformMemory::GetStats();
	float UsedGB = Stats.UsedPhysical
		/ (HorrorEditorDashboard::BytesPerKilobyte * HorrorEditorDashboard::BytesPerKilobyte * HorrorEditorDashboard::BytesPerKilobyte);
	return FText::FromString(FString::Printf(TEXT("Memory: %.2f GB"), UsedGB));
}

FText SHorrorEditorDashboard::GetFrameTimeText() const
{
	return FText::FromString(FString::Printf(TEXT("Frame Time: %.2f ms"), FApp::GetDeltaTime() * 1000.0f));
}

FText SHorrorEditorDashboard::GetLatestLogText() const
{
	const FString PackageLogDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("Build/Logs/Package"));
	const FString EditorLogDir = FPaths::ProjectLogDir();

	FString LatestLog;
	if (!HorrorEditorDashboard::FindLatestFile(PackageLogDir, TEXT("*.log"), LatestLog))
	{
		HorrorEditorDashboard::FindLatestFile(EditorLogDir, TEXT("*.log"), LatestLog);
	}

	if (LatestLog.IsEmpty())
	{
		return FText::FromString(TEXT("No project logs found yet."));
	}

	const FString LogTail = HorrorEditorDashboard::ReadLastLines(LatestLog, 40);
	return FText::FromString(FString::Printf(TEXT("%s\n\n%s"), *FPaths::GetCleanFilename(LatestLog), *LogTail));
}

FText SHorrorEditorDashboard::GetBuildStatusText() const
{
	FString StatusText;

	const FString CompilationReportPath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Docs/Compilation/CompilationReport.md"));
	FString CompilationReport;
	if (FFileHelper::LoadFileToString(CompilationReport, *CompilationReportPath))
	{
		const FString GeneratedLine = HorrorEditorDashboard::ReadFirstMatchingLine(CompilationReport, TEXT("Generated:"));
		const FString SyntaxLine = HorrorEditorDashboard::ReadFirstMatchingLine(CompilationReport, TEXT("PowerShell syntax error count:"));
		StatusText += GeneratedLine.IsEmpty() ? TEXT("Compilation report: available") : GeneratedLine;
		if (!SyntaxLine.IsEmpty())
		{
			StatusText += FString::Printf(TEXT("\n%s"), *SyntaxLine);
		}
	}
	else
	{
		StatusText += TEXT("Compilation report: not generated");
	}

	const FString PackageLogDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("Build/Logs/Package"));
	FString LatestPackageLog;
	if (HorrorEditorDashboard::FindLatestFile(PackageLogDir, TEXT("Package_*.log"), LatestPackageLog))
	{
		FString PackageLog;
		FFileHelper::LoadFileToString(PackageLog, *LatestPackageLog);
		const bool bPackageFailed = PackageLog.Contains(TEXT("PACKAGE FAILED"));
		const bool bPackageComplete = PackageLog.Contains(TEXT("PACKAGE COMPLETE"));
		const TCHAR* PackageState = bPackageFailed ? TEXT("Failed") : (bPackageComplete ? TEXT("Complete") : TEXT("Inconclusive"));
		StatusText += FString::Printf(TEXT("\nLatest package: %s (%s)"), PackageState, *FPaths::GetCleanFilename(LatestPackageLog));
	}
	else
	{
		StatusText += TEXT("\nLatest package: no package log found");
	}

	return FText::FromString(StatusText);
}

void FHorrorEditorUI::OpenDashboard()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		HorrorDashboardTabName,
		FOnSpawnTab::CreateLambda([](const FSpawnTabArgs& Args) -> TSharedRef<SDockTab>
		{
			return SNew(SDockTab)
				.TabRole(ETabRole::NomadTab)
				[
					SNew(SHorrorEditorDashboard)
				];
		}))
		.SetDisplayName(LOCTEXT("DashboardTabTitle", "Horror Dashboard"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	FGlobalTabmanager::Get()->TryInvokeTab(HorrorDashboardTabName);
}

void FHorrorEditorUI::CloseDashboard()
{
	TSharedPtr<SDockTab> DashboardTab = FGlobalTabmanager::Get()->FindExistingLiveTab(HorrorDashboardTabName);
	if (DashboardTab.IsValid())
	{
		DashboardTab->RequestCloseTab();
	}
}

#undef LOCTEXT_NAMESPACE
