// Copyright Epic Games, Inc. All Rights Reserved.

#include "EvidenceToastWidget.h"
#include "TimerManager.h"

void UEvidenceToastWidget::ShowEvidenceToast(FName EvidenceId, const FHorrorEvidenceMetadata& Metadata)
{
	if (bIsVisible)
	{
		DismissToast();
	}

	bIsVisible = true;
	BP_ShowToast(EvidenceId, Metadata.DisplayName, Metadata.Description);
	BP_PlayFadeIn();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DismissTimerHandle,
			this,
			&UEvidenceToastWidget::DismissToast,
			DisplayDuration,
			false
		);
	}
}

void UEvidenceToastWidget::ShowSimpleToast(FName EvidenceId, const FText& DisplayName)
{
	if (bIsVisible)
	{
		DismissToast();
	}

	bIsVisible = true;
	BP_ShowToast(EvidenceId, DisplayName, FText::GetEmpty());
	BP_PlayFadeIn();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DismissTimerHandle,
			this,
			&UEvidenceToastWidget::DismissToast,
			DisplayDuration,
			false
		);
	}
}

void UEvidenceToastWidget::DismissToast()
{
	if (!bIsVisible)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DismissTimerHandle);
	}

	bIsVisible = false;
	BP_PlayFadeOut();
	BP_DismissToast();
}
