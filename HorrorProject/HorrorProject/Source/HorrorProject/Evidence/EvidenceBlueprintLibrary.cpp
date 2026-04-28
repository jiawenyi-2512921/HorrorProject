// Copyright Epic Games, Inc. All Rights Reserved.

#include "EvidenceBlueprintLibrary.h"
#include "Evidence/EvidenceCollectionComponent.h"
#include "Evidence/ArchiveSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

UArchiveSubsystem* UEvidenceBlueprintLibrary::GetArchiveSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	if (UWorld* World = WorldContextObject->GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UArchiveSubsystem>();
		}
	}
	return nullptr;
}

UEvidenceCollectionComponent* UEvidenceBlueprintLibrary::GetEvidenceCollectionComponent(AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}
	return Actor->FindComponentByClass<UEvidenceCollectionComponent>();
}

FExtendedEvidenceMetadata UEvidenceBlueprintLibrary::MakeEvidenceMetadata(
	FName EvidenceId,
	FText DisplayName,
	FText Description,
	EEvidenceType Type,
	EEvidenceCategory Category,
	int32 ImportanceLevel,
	bool bIsKeyEvidence)
{
	FExtendedEvidenceMetadata Metadata;
	Metadata.EvidenceId = EvidenceId;
	Metadata.DisplayName = DisplayName;
	Metadata.Description = Description;
	Metadata.Type = Type;
	Metadata.Category = Category;
	Metadata.ImportanceLevel = ImportanceLevel;
	Metadata.bIsKeyEvidence = bIsKeyEvidence;
	return Metadata;
}

FArchiveFilter UEvidenceBlueprintLibrary::MakeArchiveFilter(
	TArray<EEvidenceType> AllowedTypes,
	TArray<EEvidenceCategory> AllowedCategories,
	bool bKeyEvidenceOnly,
	int32 MinImportanceLevel)
{
	FArchiveFilter Filter;
	Filter.AllowedTypes = AllowedTypes;
	Filter.AllowedCategories = AllowedCategories;
	Filter.bKeyEvidenceOnly = bKeyEvidenceOnly;
	Filter.MinImportanceLevel = MinImportanceLevel;
	return Filter;
}

FString UEvidenceBlueprintLibrary::GetEvidenceTypeName(EEvidenceType Type)
{
	switch (Type)
	{
	case EEvidenceType::Physical: return TEXT("实物");
	case EEvidenceType::Photo: return TEXT("照片");
	case EEvidenceType::Video: return TEXT("录像");
	case EEvidenceType::Audio: return TEXT("录音");
	case EEvidenceType::Note: return TEXT("笔记");
	case EEvidenceType::Document: return TEXT("文档");
	default: return TEXT("未知");
	}
}

FString UEvidenceBlueprintLibrary::GetEvidenceCategoryName(EEvidenceCategory Category)
{
	switch (Category)
	{
	case EEvidenceCategory::Clue: return TEXT("线索");
	case EEvidenceCategory::Witness: return TEXT("证言");
	case EEvidenceCategory::Location: return TEXT("地点证据");
	case EEvidenceCategory::Temporal: return TEXT("时间异常");
	case EEvidenceCategory::Personal: return TEXT("个人物品");
	case EEvidenceCategory::Environmental: return TEXT("环境证据");
	default: return TEXT("未知");
	}
}

void UEvidenceBlueprintLibrary::SortEvidenceByDate(TArray<FExtendedEvidenceMetadata>& Evidence, bool bNewestFirst)
{
	Evidence.Sort([bNewestFirst](const FExtendedEvidenceMetadata& A, const FExtendedEvidenceMetadata& B)
	{
		return bNewestFirst ? (A.CollectionTimestamp > B.CollectionTimestamp) : (A.CollectionTimestamp < B.CollectionTimestamp);
	});
}

void UEvidenceBlueprintLibrary::SortEvidenceByImportance(TArray<FExtendedEvidenceMetadata>& Evidence, bool bHighestFirst)
{
	Evidence.Sort([bHighestFirst](const FExtendedEvidenceMetadata& A, const FExtendedEvidenceMetadata& B)
	{
		return bHighestFirst ? (A.ImportanceLevel > B.ImportanceLevel) : (A.ImportanceLevel < B.ImportanceLevel);
	});
}

bool UEvidenceBlueprintLibrary::IsEvidenceKeyEvidence(const FExtendedEvidenceMetadata& Evidence)
{
	return Evidence.bIsKeyEvidence;
}

int32 UEvidenceBlueprintLibrary::CountEvidenceByType(const TArray<FExtendedEvidenceMetadata>& Evidence, EEvidenceType Type)
{
	int32 Count = 0;
	for (const FExtendedEvidenceMetadata& Item : Evidence)
	{
		if (Item.Type == Type)
		{
			Count++;
		}
	}
	return Count;
}

int32 UEvidenceBlueprintLibrary::CountEvidenceByCategory(const TArray<FExtendedEvidenceMetadata>& Evidence, EEvidenceCategory Category)
{
	int32 Count = 0;
	for (const FExtendedEvidenceMetadata& Item : Evidence)
	{
		if (Item.Category == Category)
		{
			Count++;
		}
	}
	return Count;
}
