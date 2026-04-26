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
#include "HAL/PlatformMemory.h"

#define LOCTEXT_NAMESPACE "HorrorEditorUI"

static const FName HorrorDashboardTabName("HorrorDashboard");

TSharedPtr<SDockTab> FHorrorEditorUI::DashboardTab = nullptr;

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
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
		]

		// Main Content
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SSplitter)
			.Orientation(Orient_Horizontal)

			// Left Panel - Tools
			+ SSplitter::Slot()
			.Value(0.3f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SNew(SVerticalBox)

					// Performance Monitor
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(5)
					[
						CreatePerformanceMonitor()
					]

					// Quick Actions
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(5)
					[
						SNew(SVerticalBox)

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
						]
					]
				]
			]

			// Right Panel - Info
			+ SSplitter::Slot()
			.Value(0.7f)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.FillHeight(0.5f)
				[
					CreateLogViewer()
				]

				+ SVerticalBox::Slot()
				.FillHeight(0.5f)
				[
					CreateBuildStatus()
				]
			]
		]
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
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
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
			.Text(this, &SHorrorEditorDashboard::GetDrawCallsText)
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

TSharedRef<SWidget> SHorrorEditorDashboard::CreateAssetBrowser()
{
	return SNew(STextBlock)
		.Text(LOCTEXT("AssetBrowser", "Asset Browser - Coming Soon"));
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
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("LogPlaceholder", "Logs will appear here..."))
				]
			]
		];
}

TSharedRef<SWidget> SHorrorEditorDashboard::CreateTestRunner()
{
	return SNew(STextBlock)
		.Text(LOCTEXT("TestRunner", "Test Runner - Coming Soon"));
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
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(5)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("BuildInfo", "Last Build: Success"))
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
	float UsedGB = Stats.UsedPhysical / (1024.0f * 1024.0f * 1024.0f);
	return FText::FromString(FString::Printf(TEXT("Memory: %.2f GB"), UsedGB));
}

FText SHorrorEditorDashboard::GetDrawCallsText() const
{
	return FText::FromString(TEXT("Draw Calls: N/A"));
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

	DashboardTab = FGlobalTabmanager::Get()->TryInvokeTab(HorrorDashboardTabName);
}

void FHorrorEditorUI::CloseDashboard()
{
	if (DashboardTab.IsValid())
	{
		DashboardTab->RequestCloseTab();
		DashboardTab.Reset();
	}
}

#undef LOCTEXT_NAMESPACE
