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
	TSharedRef<SWidget> CreateToolPanel();
	TSharedRef<SWidget> CreateToolButtons();
	TSharedRef<SWidget> CreateInfoPanel();
	TSharedRef<SWidget> CreatePerformanceMonitor();
	TSharedRef<SWidget> CreateLogViewer();
	TSharedRef<SWidget> CreateBuildStatus();

	FReply OnRunTests();
	FReply OnValidateAssets();
	FReply OnOptimizeMaterials();
	FReply OnGenerateLODs();
	FReply OnRefreshStats();

	FText GetFPSText() const;
	FText GetMemoryText() const;
	FText GetFrameTimeText() const;
	FText GetLatestLogText() const;
	FText GetBuildStatusText() const;
};

class FHorrorEditorUI
{
public:
	static void OpenDashboard();
	static void CloseDashboard();
};
