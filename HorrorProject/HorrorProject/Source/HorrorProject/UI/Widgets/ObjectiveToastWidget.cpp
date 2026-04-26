// Copyright Epic Games, Inc. All Rights Reserved.

#include "ObjectiveToastWidget.h"
#include "TimerManager.h"

void UObjectiveToastWidget::ShowObjectiveToast(FGameplayTag EventTag, const FText& ObjectiveText)
{
	ShowObjectiveWithHint(EventTag, ObjectiveText, FText::GetEmpty());
}

void UObjectiveToastWidget::ShowObjectiveWithHint(FGameplayTag EventTag, const FText& ObjectiveText, const FText& HintText)
{
	if (bIsVisible)
	{
		DismissToast();
	}

	bIsVisible = true;
	CurrentEventTag = EventTag;

	BP_ShowToast(EventTag, ObjectiveText, HintText);
	BP_PlaySlideIn();

	const float TypewriterDuration = ObjectiveText.ToString().Len() * TypewriterSpeed;
	BP_PlayTypewriterEffect(ObjectiveText, TypewriterDuration);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DismissTimerHandle,
			this,
			&UObjectiveToastWidget::DismissToast,
			DisplayDuration + TypewriterDuration,
			false
		);
	}
}

void UObjectiveToastWidget::DismissToast()
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
	CurrentEventTag = FGameplayTag::EmptyTag;
	BP_PlaySlideOut();
	BP_DismissToast();
}
