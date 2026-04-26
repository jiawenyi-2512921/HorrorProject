// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "EvidenceTypes.generated.h"

UENUM(BlueprintType)
enum class EEvidenceType : uint8
{
	Physical UMETA(DisplayName="Physical Item"),
	Photo UMETA(DisplayName="Photograph"),
	Video UMETA(DisplayName="Video Recording"),
	Audio UMETA(DisplayName="Audio Recording"),
	Note UMETA(DisplayName="Written Note"),
	Document UMETA(DisplayName="Document")
};

UENUM(BlueprintType)
enum class EEvidenceCategory : uint8
{
	Clue UMETA(DisplayName="Clue"),
	Witness UMETA(DisplayName="Witness Account"),
	Location UMETA(DisplayName="Location Evidence"),
	Temporal UMETA(DisplayName="Temporal Anomaly"),
	Personal UMETA(DisplayName="Personal Item"),
	Environmental UMETA(DisplayName="Environmental")
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FEvidenceCaptureData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Evidence")
	FName EvidenceId = NAME_None;

	UPROPERTY(BlueprintReadWrite, Category="Evidence")
	EEvidenceType Type = EEvidenceType::Physical;

	UPROPERTY(BlueprintReadWrite, Category="Evidence")
	FVector CaptureLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category="Evidence")
	FRotator CaptureRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite, Category="Evidence")
	FDateTime CaptureTimestamp;

	UPROPERTY(BlueprintReadWrite, Category="Evidence")
	FGameplayTagContainer ContextTags;

	UPROPERTY(BlueprintReadWrite, Category="Evidence")
	UTexture2D* CapturedImage = nullptr;

	UPROPERTY(BlueprintReadWrite, Category="Evidence")
	float VideoDuration = 0.0f;
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FExtendedEvidenceMetadata
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Evidence")
	FName EvidenceId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Evidence")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Evidence")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Evidence")
	EEvidenceType Type = EEvidenceType::Physical;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Evidence")
	EEvidenceCategory Category = EEvidenceCategory::Clue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Evidence")
	FGameplayTagContainer Tags;

	UPROPERTY(BlueprintReadOnly, Category="Evidence")
	FDateTime CollectionTimestamp;

	UPROPERTY(BlueprintReadOnly, Category="Evidence")
	FVector CollectionLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Evidence")
	UTexture2D* ThumbnailImage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Evidence")
	int32 ImportanceLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Evidence")
	bool bIsKeyEvidence = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Evidence")
	TArray<FName> RelatedEvidenceIds;
};
