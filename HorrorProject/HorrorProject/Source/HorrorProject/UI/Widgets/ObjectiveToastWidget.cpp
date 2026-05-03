// Copyright Epic Games, Inc. All Rights Reserved.

#include "ObjectiveToastWidget.h"
#include "TimerManager.h"

void UObjectiveToastWidget::ShowObjectiveToast(FGameplayTag EventTag, const FText& ObjectiveText)
{
	ShowObjectiveWithHint(EventTag, ObjectiveText, FText::GetEmpty());
}

void UObjectiveToastWidget::ShowObjectiveWithHint(FGameplayTag EventTag, const FText& ObjectiveText, const FText& HintText)
{
	ShowObjectiveWithPriority(EventTag, ObjectiveText, HintText, 0);
}

void UObjectiveToastWidget::ShowObjectiveWithPriority(FGameplayTag EventTag, const FText& ObjectiveText, const FText& HintText, int32 ToastPriority)
{
	FQueuedObjectiveToast Toast;
	Toast.EventTag = EventTag;
	Toast.ObjectiveText = ObjectiveText;
	Toast.HintText = HintText;
	Toast.Priority = ToastPriority;
	Toast.Sequence = NextToastSequence++;

	if (!bToastVisible)
	{
		ShowToastNow(Toast);
		return;
	}

	if (ToastPriority > CurrentToastPriority)
	{
		FQueuedObjectiveToast InterruptedToast;
		InterruptedToast.EventTag = CurrentEventTag;
		InterruptedToast.ObjectiveText = CurrentObjectiveText;
		InterruptedToast.HintText = CurrentHintText;
		InterruptedToast.Priority = CurrentToastPriority;
		InterruptedToast.Sequence = NextToastSequence++;
		EnqueueToast(InterruptedToast);
		HideActiveToast();
		ShowToastNow(Toast);
		return;
	}

	EnqueueToast(Toast);
}

void UObjectiveToastWidget::ShowToastNow(const FQueuedObjectiveToast& Toast)
{
	bToastVisible = true;
	CurrentEventTag = Toast.EventTag;
	CurrentObjectiveText = Toast.ObjectiveText;
	CurrentHintText = Toast.HintText;
	CurrentToastPriority = Toast.Priority;

	BP_ShowToast(Toast.EventTag, Toast.ObjectiveText, Toast.HintText);
	BP_PlaySlideIn();

	const float TypewriterDuration = CalculateTypewriterDuration(Toast.ObjectiveText, TypewriterSpeed, MaxTypewriterDuration);
	BP_PlayTypewriterEffect(Toast.ObjectiveText, TypewriterDuration);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DismissTimerHandle,
			this,
			&UObjectiveToastWidget::DismissToast,
			CalculateDismissDelay(Toast.ObjectiveText, DisplayDuration, TypewriterSpeed, MaxTypewriterDuration),
			false
		);
	}
}

void UObjectiveToastWidget::DismissToast()
{
	if (!bToastVisible)
	{
		return;
	}

	HideActiveToast();
	TryShowNextQueuedToast();
}

void UObjectiveToastWidget::HideActiveToast()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DismissTimerHandle);
	}

	bToastVisible = false;
	CurrentEventTag = FGameplayTag::EmptyTag;
	CurrentObjectiveText = FText::GetEmpty();
	CurrentHintText = FText::GetEmpty();
	CurrentToastPriority = 0;
	BP_PlaySlideOut();
	BP_DismissToast();
}

void UObjectiveToastWidget::EnqueueToast(const FQueuedObjectiveToast& Toast)
{
	if (Toast.ObjectiveText.IsEmpty() && Toast.HintText.IsEmpty())
	{
		return;
	}

	for (FQueuedObjectiveToast& QueuedToast : QueuedToasts)
	{
		if (IsSameToast(QueuedToast, Toast.EventTag, Toast.ObjectiveText))
		{
			const int32 OriginalSequence = QueuedToast.Sequence;
			QueuedToast = Toast;
			QueuedToast.Sequence = OriginalSequence;
			return;
		}
	}

	QueuedToasts.Add(Toast);
	QueuedToasts.Sort([](const FQueuedObjectiveToast& Left, const FQueuedObjectiveToast& Right)
	{
		if (Left.Priority != Right.Priority)
		{
			return Left.Priority > Right.Priority;
		}

		return Left.Sequence < Right.Sequence;
	});
}

bool UObjectiveToastWidget::TryShowNextQueuedToast()
{
	if (QueuedToasts.IsEmpty())
	{
		return false;
	}

	const FQueuedObjectiveToast NextToast = QueuedToasts[0];
	QueuedToasts.RemoveAt(0, 1, EAllowShrinking::No);
	ShowToastNow(NextToast);
	return true;
}

bool UObjectiveToastWidget::IsSameToast(const FQueuedObjectiveToast& Toast, FGameplayTag EventTag, const FText& ObjectiveText)
{
	return Toast.EventTag == EventTag && Toast.ObjectiveText.ToString() == ObjectiveText.ToString();
}

float UObjectiveToastWidget::CalculateTypewriterDuration(const FText& ObjectiveText, float InTypewriterSpeed, float InMaxTypewriterDuration)
{
	const float RawTypewriterDuration = static_cast<float>(ObjectiveText.ToString().Len()) * FMath::Max(0.0f, InTypewriterSpeed);
	return FMath::Clamp(RawTypewriterDuration, 0.0f, FMath::Max(0.0f, InMaxTypewriterDuration));
}

float UObjectiveToastWidget::CalculateDismissDelay(const FText& ObjectiveText, float InDisplayDuration, float InTypewriterSpeed, float InMaxTypewriterDuration)
{
	return FMath::Max(0.0f, InDisplayDuration) + CalculateTypewriterDuration(ObjectiveText, InTypewriterSpeed, InMaxTypewriterDuration);
}

#if WITH_DEV_AUTOMATION_TESTS
float UObjectiveToastWidget::CalculateTypewriterDurationForTests(const FText& ObjectiveText, float InTypewriterSpeed, float InMaxTypewriterDuration)
{
	return CalculateTypewriterDuration(ObjectiveText, InTypewriterSpeed, InMaxTypewriterDuration);
}

float UObjectiveToastWidget::CalculateDismissDelayForTests(const FText& ObjectiveText, float InDisplayDuration, float InTypewriterSpeed, float InMaxTypewriterDuration)
{
	return CalculateDismissDelay(ObjectiveText, InDisplayDuration, InTypewriterSpeed, InMaxTypewriterDuration);
}
#endif
