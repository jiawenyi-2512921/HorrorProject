// Copyright Epic Games, Inc. All Rights Reserved.

#include "LODGenerator.h"
#include "Engine/StaticMesh.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Misc/MessageDialog.h"
#include "MeshUtilities.h"
#include "IMeshReductionManagerModule.h"

const FLODGenerationSettings& ULODGenerator::GetDefaultSettings()
{
	static const FLODGenerationSettings Settings;
	return Settings;
}

void ULODGenerator::GenerateLODsForSelection()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FAssetData> SelectedAssets;
	ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);

	int32 ProcessedCount = 0;

	for (const FAssetData& AssetData : SelectedAssets)
	{
		UStaticMesh* Mesh = Cast<UStaticMesh>(AssetData.GetAsset());
		if (Mesh)
		{
			GenerateLODsForMesh(Mesh, GetDefaultSettings());
			ProcessedCount++;
		}
	}

	FText Message = FText::FromString(FString::Printf(
		TEXT("LOD Generation Complete\n\nProcessed %d meshes"),
		ProcessedCount
	));
	FMessageDialog::Open(EAppMsgType::Ok, Message);

	UE_LOG(LogTemp, Log, TEXT("LOD Generation: Processed %d meshes"), ProcessedCount);
}

void ULODGenerator::GenerateLODsForMesh(UStaticMesh* Mesh, const FLODGenerationSettings& Settings)
{
	if (!Mesh) return;

	IMeshReductionManagerModule& MeshReductionModule = FModuleManager::Get().LoadModuleChecked<IMeshReductionManagerModule>("MeshReductionInterface");
	IMeshReduction* MeshReduction = MeshReductionModule.GetStaticMeshReductionInterface();

	if (!MeshReduction)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get mesh reduction interface"));
		return;
	}

	// Set number of LODs
	int32 CurrentLODs = Mesh->GetNumLODs();
	if (CurrentLODs < Settings.NumLODs)
	{
		Mesh->SetNumSourceModels(Settings.NumLODs);
	}

	// Configure LOD settings
	for (int32 LODIndex = 1; LODIndex < Settings.NumLODs; ++LODIndex)
	{
		FStaticMeshSourceModel& SourceModel = Mesh->GetSourceModel(LODIndex);

		// Set reduction settings
		FMeshReductionSettings& ReductionSettings = SourceModel.ReductionSettings;
		ReductionSettings.PercentTriangles = FMath::Pow(Settings.ReductionPercentage, LODIndex);
		ReductionSettings.PercentVertices = FMath::Pow(Settings.ReductionPercentage, LODIndex);

		if (Settings.bRecalculateNormals)
		{
			ReductionSettings.bRecalculateNormals = true;
		}

		// Set screen size
		if (Settings.bAutoComputeLODScreenSize)
		{
			SourceModel.ScreenSize = FMath::Pow(0.5f, LODIndex);
		}
	}

	// Build LODs
	Mesh->Build();
	Mesh->PostEditChange();

	UE_LOG(LogTemp, Log, TEXT("Generated %d LODs for mesh: %s"), Settings.NumLODs, *Mesh->GetName());
}

void ULODGenerator::GenerateLODsForAllMeshes()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetDataList;
	AssetRegistryModule.Get().GetAssetsByClass(UStaticMesh::StaticClass()->GetClassPathName(), AssetDataList, true);

	int32 ProcessedCount = 0;

	for (const FAssetData& AssetData : AssetDataList)
	{
		UStaticMesh* Mesh = Cast<UStaticMesh>(AssetData.GetAsset());
		if (Mesh)
		{
			GenerateLODsForMesh(Mesh, GetDefaultSettings());
			ProcessedCount++;
		}
	}

	FText Message = FText::FromString(FString::Printf(
		TEXT("Batch LOD Generation Complete\n\nProcessed %d meshes"),
		ProcessedCount
	));
	FMessageDialog::Open(EAppMsgType::Ok, Message);

	UE_LOG(LogTemp, Log, TEXT("Batch LOD Generation: Processed %d meshes"), ProcessedCount);
}
