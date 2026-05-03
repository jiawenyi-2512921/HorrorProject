// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HorrorInteractionTypes.generated.h"

UENUM(BlueprintType)
enum class EHorrorInteractionVerb : uint8
{
	Interact UMETA(DisplayName="互动"),
	Open UMETA(DisplayName="打开"),
	Close UMETA(DisplayName="关闭"),
	EnterCode UMETA(DisplayName="输入密码"),
	Record UMETA(DisplayName="录像"),
	Review UMETA(DisplayName="复查"),
	Repair UMETA(DisplayName="修复"),
	Survive UMETA(DisplayName="坚持")
};

UENUM(BlueprintType)
enum class EHorrorInteractionInputStyle : uint8
{
	Press UMETA(DisplayName="按下"),
	Hold UMETA(DisplayName="长按"),
	Modal UMETA(DisplayName="窗口"),
	RecordAim UMETA(DisplayName="录像瞄准"),
	Timed UMETA(DisplayName="限时")
};

UENUM(BlueprintType)
enum class EHorrorInteractionRiskLevel : uint8
{
	None UMETA(DisplayName="无"),
	Low UMETA(DisplayName="低"),
	Medium UMETA(DisplayName="中"),
	High UMETA(DisplayName="高")
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorInteractionContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Horror|Interaction")
	bool bVisible = false;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Interaction")
	bool bCanInteract = false;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Interaction")
	bool bRequiresRecording = false;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Interaction")
	bool bOpensPanel = false;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Interaction")
	EHorrorInteractionVerb Verb = EHorrorInteractionVerb::Interact;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Interaction")
	EHorrorInteractionInputStyle InputStyle = EHorrorInteractionInputStyle::Press;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Interaction")
	EHorrorInteractionRiskLevel RiskLevel = EHorrorInteractionRiskLevel::None;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Interaction")
	FText ActionText;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Interaction")
	FText DetailText;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Interaction")
	FText BlockedReason;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Interaction")
	FText InputText;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Interaction")
	FName TargetId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Interaction")
	float ProgressFraction = 0.0f;

	bool HasAnyText() const
	{
		return !ActionText.IsEmpty() || !DetailText.IsEmpty() || !BlockedReason.IsEmpty() || !InputText.IsEmpty();
	}
};
