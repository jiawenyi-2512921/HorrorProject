// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "MaterialOptimizer.generated.h"

USTRUCT(BlueprintType)
struct FMaterialOptimizationResult
{
	GENERATED_BODY()

	UPROPERTY()
	FString MaterialPath;

	UPROPERTY()
	int32 OriginalInstructionCount = 0;

	UPROPERTY()
	int32 OptimizedInstructionCount = 0;

	UPROPERTY()
	float OptimizationPercentage = 0.0f;

	UPROPERTY()
	TArray<FString> OptimizationSteps;
};

UCLASS()
class HORRORPROJECTEDITOR_API UMaterialOptimizer : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	static void OptimizeMaterials();
	static FMaterialOptimizationResult OptimizeMaterial(UMaterial* Material);

private:
	static void RemoveUnusedNodes(UMaterial* Material, FMaterialOptimizationResult& Result);
	static void SimplifyExpressions(UMaterial* Material, FMaterialOptimizationResult& Result);
	static void OptimizeTextureSampling(UMaterial* Material, FMaterialOptimizationResult& Result);
	static void ConvertToMaterialInstance(UMaterial* Material, FMaterialOptimizationResult& Result);
};
