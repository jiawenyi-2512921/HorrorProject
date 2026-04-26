// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "LODGenerator.generated.h"

USTRUCT(BlueprintType)
struct FLODGenerationSettings
{
	GENERATED_BODY()

	UPROPERTY()
	int32 NumLODs = 4;

	UPROPERTY()
	float ReductionPercentage = 0.5f;

	UPROPERTY()
	bool bAutoComputeLODScreenSize = true;

	UPROPERTY()
	bool bRecalculateNormals = true;
};

UCLASS()
class HORRORPROJECTEDITOR_API ULODGenerator : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	static void GenerateLODsForSelection();
	static void GenerateLODsForMesh(UStaticMesh* Mesh, const FLODGenerationSettings& Settings);
	static void GenerateLODsForAllMeshes();

private:
	static FLODGenerationSettings DefaultSettings;
};
