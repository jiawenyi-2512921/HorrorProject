// Copyright Epic Games, Inc. All Rights Reserved.

#include "MaterialOptimizer.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionTextureSampleParameter.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Engine/Texture.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/MessageDialog.h"

void UMaterialOptimizer::OptimizeMaterials()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetDataList;
	AssetRegistryModule.Get().GetAssetsByClass(UMaterial::StaticClass()->GetClassPathName(), AssetDataList, true);

	int32 OptimizedCount = 0;
	float TotalOptimization = 0.0f;

	for (const FAssetData& AssetData : AssetDataList)
	{
		UMaterial* Material = Cast<UMaterial>(AssetData.GetAsset());
		if (Material)
		{
			FMaterialOptimizationResult Result = OptimizeMaterial(Material);
			if (Result.OptimizationPercentage > 0.0f)
			{
				OptimizedCount++;
				TotalOptimization += Result.OptimizationPercentage;
			}
		}
	}

	float AverageOptimization = OptimizedCount > 0 ? TotalOptimization / OptimizedCount : 0.0f;

	FText Message = FText::FromString(FString::Printf(
		TEXT("Material Optimization Complete\n\nOptimized: %d materials\nAverage Optimization: %.1f%%"),
		OptimizedCount, AverageOptimization
	));
	FMessageDialog::Open(EAppMsgType::Ok, Message);

	UE_LOG(LogTemp, Log, TEXT("Material Optimization: %d materials, %.1f%% average"), OptimizedCount, AverageOptimization);
}

FMaterialOptimizationResult UMaterialOptimizer::OptimizeMaterial(UMaterial* Material)
{
	FMaterialOptimizationResult Result;
	Result.MaterialPath = Material->GetPathName();
	Result.OriginalInstructionCount = Material->GetExpressions().Num();

	RemoveUnusedNodes(Material, Result);
	SimplifyExpressions(Material, Result);
	OptimizeTextureSampling(Material, Result);
	ConvertToMaterialInstance(Material, Result);

	Result.OptimizedInstructionCount = Material->GetExpressions().Num();

	if (Result.OriginalInstructionCount > 0)
	{
		Result.OptimizationPercentage =
			((float)(Result.OriginalInstructionCount - Result.OptimizedInstructionCount) / Result.OriginalInstructionCount) * 100.0f;
	}

	Material->PreEditChange(nullptr);
	Material->PostEditChange();

	UE_LOG(LogTemp, Log, TEXT("Optimized material %s: %.1f%% reduction"),
		*Material->GetName(), Result.OptimizationPercentage);

	return Result;
}

void UMaterialOptimizer::RemoveUnusedNodes(UMaterial* Material, FMaterialOptimizationResult& OutResult)
{
	TArray<UMaterialExpression*> ExpressionsToRemove;

	for (UMaterialExpression* Expression : Material->GetExpressions())
	{
		if (Expression && Expression->GetOutputs().Num() == 0)
		{
			ExpressionsToRemove.Add(Expression);
		}
	}

	for (UMaterialExpression* Expression : ExpressionsToRemove)
	{
		Material->GetExpressionCollection().RemoveExpression(Expression);
	}

	if (ExpressionsToRemove.Num() > 0)
	{
		OutResult.OptimizationSteps.Add(FString::Printf(TEXT("Removed %d unused nodes"), ExpressionsToRemove.Num()));
	}
}

void UMaterialOptimizer::SimplifyExpressions(UMaterial* Material, FMaterialOptimizationResult& OutResult)
{
	int32 ParameterCount = 0;
	for (UMaterialExpression* Expression : Material->GetExpressions())
	{
		if (Cast<UMaterialExpressionScalarParameter>(Expression)
			|| Cast<UMaterialExpressionVectorParameter>(Expression)
			|| Cast<UMaterialExpressionTextureSampleParameter>(Expression))
		{
			ParameterCount++;
		}
	}

	if (ParameterCount > 0)
	{
		OutResult.OptimizationSteps.Add(FString::Printf(TEXT("Found %d reusable material parameters"), ParameterCount));
	}
}

void UMaterialOptimizer::OptimizeTextureSampling(UMaterial* Material, FMaterialOptimizationResult& OutResult)
{
	int32 TextureSampleCount = 0;
	TMap<UTexture*, int32> TextureUsageCounts;

	for (UMaterialExpression* Expression : Material->GetExpressions())
	{
		if (UMaterialExpressionTextureSample* TextureSample = Cast<UMaterialExpressionTextureSample>(Expression))
		{
			TextureSampleCount++;
			if (TextureSample->Texture)
			{
				int32& UsageCount = TextureUsageCounts.FindOrAdd(TextureSample->Texture);
				UsageCount++;
			}
		}
	}

	if (TextureSampleCount > 8)
	{
		OutResult.OptimizationSteps.Add(FString::Printf(
			TEXT("Warning: High texture sample count (%d)"), TextureSampleCount));
	}

	for (const TPair<UTexture*, int32>& Pair : TextureUsageCounts)
	{
		if (Pair.Value > 1)
		{
			OutResult.OptimizationSteps.Add(FString::Printf(
				TEXT("Texture '%s' is sampled %d times; consider sharing UV/math upstream"),
				*GetNameSafe(Pair.Key),
				Pair.Value));
		}
	}
}

void UMaterialOptimizer::ConvertToMaterialInstance(UMaterial* Material, FMaterialOptimizationResult& OutResult)
{
	bool bHasInstanceParameters = false;
	for (UMaterialExpression* Expression : Material->GetExpressions())
	{
		if (Cast<UMaterialExpressionScalarParameter>(Expression)
			|| Cast<UMaterialExpressionVectorParameter>(Expression)
			|| Cast<UMaterialExpressionTextureSampleParameter>(Expression))
		{
			bHasInstanceParameters = true;
			break;
		}
	}

	OutResult.OptimizationSteps.Add(bHasInstanceParameters
		? TEXT("Material is ready for Material Instance workflows")
		: TEXT("Material has no exposed parameters; add parameters before creating reusable instances"));
}
