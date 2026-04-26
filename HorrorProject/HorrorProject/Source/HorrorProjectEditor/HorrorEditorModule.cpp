// Copyright Epic Games, Inc. All Rights Reserved.

#include "HorrorEditorModule.h"
#include "ToolMenus.h"
#include "LevelEditor.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Styling/SlateStyle.h"
#include "Tools/AssetValidator.h"
#include "Tools/LODGenerator.h"
#include "Tools/MaterialOptimizer.h"
#include "Tools/AudioBatchProcessor.h"
#include "UI/HorrorEditorUI.h"

#define LOCTEXT_NAMESPACE "FHorrorProjectEditorModule"

void FHorrorProjectEditorModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("HorrorProjectEditor: Module Started"));

	FCoreDelegates::OnPostEngineInit.AddLambda([this]()
	{
		RegisterMenus();
	});
}

void FHorrorProjectEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	UE_LOG(LogTemp, Log, TEXT("HorrorProjectEditor: Module Shutdown"));
}

void FHorrorProjectEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
	FToolMenuSection& Section = Menu->AddSection("HorrorTools", LOCTEXT("HorrorTools", "Horror Project Tools"));

	Section.AddMenuEntry(
		"AssetValidator",
		LOCTEXT("AssetValidator", "Asset Validator"),
		LOCTEXT("AssetValidatorTooltip", "Validate all project assets"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FHorrorProjectEditorModule::OnAssetValidatorClicked))
	);

	Section.AddMenuEntry(
		"LODGenerator",
		LOCTEXT("LODGenerator", "LOD Generator"),
		LOCTEXT("LODGeneratorTooltip", "Generate LODs for static meshes"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FHorrorProjectEditorModule::OnLODGeneratorClicked))
	);

	Section.AddMenuEntry(
		"MaterialOptimizer",
		LOCTEXT("MaterialOptimizer", "Material Optimizer"),
		LOCTEXT("MaterialOptimizerTooltip", "Optimize material complexity"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FHorrorProjectEditorModule::OnMaterialOptimizerClicked))
	);

	Section.AddMenuEntry(
		"AudioBatchProcessor",
		LOCTEXT("AudioBatchProcessor", "Audio Batch Processor"),
		LOCTEXT("AudioBatchProcessorTooltip", "Batch process audio files"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FHorrorProjectEditorModule::OnAudioBatchProcessorClicked))
	);

	Section.AddMenuEntry(
		"EditorUI",
		LOCTEXT("EditorUI", "Horror Editor Dashboard"),
		LOCTEXT("EditorUITooltip", "Open Horror Project Editor Dashboard"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FHorrorProjectEditorModule::OnEditorUIClicked))
	);
}

void FHorrorProjectEditorModule::OnAssetValidatorClicked()
{
	UAssetValidator::ValidateAllAssets();
}

void FHorrorProjectEditorModule::OnLODGeneratorClicked()
{
	ULODGenerator::GenerateLODsForSelection();
}

void FHorrorProjectEditorModule::OnMaterialOptimizerClicked()
{
	UMaterialOptimizer::OptimizeMaterials();
}

void FHorrorProjectEditorModule::OnAudioBatchProcessorClicked()
{
	UAudioBatchProcessor::ProcessAudioFiles();
}

void FHorrorProjectEditorModule::OnEditorUIClicked()
{
	FHorrorEditorUI::OpenDashboard();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHorrorProjectEditorModule, HorrorProjectEditor)
