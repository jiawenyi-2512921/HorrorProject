// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FHorrorProjectEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterMenus();
	void OnAssetValidatorClicked();
	void OnLODGeneratorClicked();
	void OnMaterialOptimizerClicked();
	void OnAudioBatchProcessorClicked();
	void OnEditorUIClicked();

	TSharedPtr<class FUICommandList> PluginCommands;
};
