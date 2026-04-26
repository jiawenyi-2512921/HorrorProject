// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SHorrorEditorDashboard : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SHorrorEditorDashboard) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedRef<SWidget> CreatePerformanceMonitor();
	TSharedRef<SWidget> CreateAssetBrowser();
	TSharedRef<SWidget> CreateLogViewer();
	TSharedRef<SWidget> CreateTestRunner();
	TSharedRef<SWidget> CreateBuildStatus();

	FReply OnRunTests();
	FReply OnValidateAssets();
	FReply OnOptimizeMaterials();
	FReply OnGenerateLODs();
	FReply OnRefreshStats();

	FText GetFPSText() const;
	FText GetMemoryText() const;
	FText GetDrawCallsText() const;
};

class FHorrorEditorUI
{
public:
	static void OpenDashboard();
	static void CloseDashboard();

private:
	static TSharedPtr<class SDockTab> DashboardTab;
};
