// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Evidence/EvidenceTypes.h"
#include "EvidenceBlueprintLibrary.generated.h"

class UEvidenceCollectionComponent;
class UArchiveSubsystem;

UCLASS()
class HORRORPROJECT_API UEvidenceBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="Evidence", meta=(WorldContext="WorldContextObject"))
	static UArchiveSubsystem* GetArchiveSubsystem(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category="Evidence")
	static UEvidenceCollectionComponent* GetEvidenceCollectionComponent(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category="Evidence")
	static FExtendedEvidenceMetadata MakeEvidenceMetadata(
		FName EvidenceId,
		FText DisplayName,
		FText Description,
		EEvidenceType Type,
		EEvidenceCategory Category,
		int32 ImportanceLevel = 1,
		bool bIsKeyEvidence = false);

	UFUNCTION(BlueprintCallable, Category="Evidence")
	static FArchiveFilter MakeArchiveFilter(
		TArray<EEvidenceType> AllowedTypes,
		TArray<EEvidenceCategory> AllowedCategories,
		bool bKeyEvidenceOnly = false,
		int32 MinImportanceLevel = 0);

	UFUNCTION(BlueprintPure, Category="Evidence")
	static FString GetEvidenceTypeName(EEvidenceType Type);

	UFUNCTION(BlueprintPure, Category="Evidence")
	static FString GetEvidenceCategoryName(EEvidenceCategory Category);

	UFUNCTION(BlueprintCallable, Category="Evidence")
	static void SortEvidenceByDate(UPARAM(ref) TArray<FExtendedEvidenceMetadata>& Evidence, bool bNewestFirst = true);

	UFUNCTION(BlueprintCallable, Category="Evidence")
	static void SortEvidenceByImportance(UPARAM(ref) TArray<FExtendedEvidenceMetadata>& Evidence, bool bHighestFirst = true);

	UFUNCTION(BlueprintPure, Category="Evidence")
	static bool IsEvidenceKeyEvidence(const FExtendedEvidenceMetadata& Evidence);

	UFUNCTION(BlueprintPure, Category="Evidence")
	static int32 CountEvidenceByType(const TArray<FExtendedEvidenceMetadata>& Evidence, EEvidenceType Type);

	UFUNCTION(BlueprintPure, Category="Evidence")
	static int32 CountEvidenceByCategory(const TArray<FExtendedEvidenceMetadata>& Evidence, EEvidenceCategory Category);
};
