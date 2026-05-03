// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Widgets/ObjectiveToastWidget.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FObjectiveToastWidgetClampsLongTypewriterDismissTimeTest,
	"HorrorProject.UI.Widgets.ObjectiveToast.ClampsLongTypewriterDismissTime",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FObjectiveToastWidgetClampsLongTypewriterDismissTimeTest::RunTest(const FString& Parameters)
{
	constexpr float DisplayDuration = 5.0f;
	constexpr float TypewriterSpeed = 0.05f;
	constexpr float MaxTypewriterDuration = 3.0f;

	FString LongObjectiveText;
	LongObjectiveText.Reserve(512);
	for (int32 Index = 0; Index < 160; ++Index)
	{
		LongObjectiveText += TEXT("目标链路恢复");
	}

	const float LongTypewriterDuration = UObjectiveToastWidget::CalculateTypewriterDurationForTests(
		FText::FromString(LongObjectiveText),
		TypewriterSpeed,
		MaxTypewriterDuration);
	const float LongDismissDelay = UObjectiveToastWidget::CalculateDismissDelayForTests(
		FText::FromString(LongObjectiveText),
		DisplayDuration,
		TypewriterSpeed,
		MaxTypewriterDuration);

	TestTrue(TEXT("Long objective toasts should clamp typewriter duration."), LongTypewriterDuration <= MaxTypewriterDuration + KINDA_SMALL_NUMBER);
	TestTrue(TEXT("Long objective toasts should not remain visible beyond the clamped typewriter budget."), LongDismissDelay <= DisplayDuration + MaxTypewriterDuration + KINDA_SMALL_NUMBER);

	const FText ShortObjectiveText = FText::FromString(TEXT("目标完成"));
	const float ExpectedShortTypewriterDuration = static_cast<float>(ShortObjectiveText.ToString().Len()) * TypewriterSpeed;
	const float ShortTypewriterDuration = UObjectiveToastWidget::CalculateTypewriterDurationForTests(
		ShortObjectiveText,
		TypewriterSpeed,
		MaxTypewriterDuration);
	const float ShortDismissDelay = UObjectiveToastWidget::CalculateDismissDelayForTests(
		ShortObjectiveText,
		DisplayDuration,
		TypewriterSpeed,
		MaxTypewriterDuration);

	TestTrue(TEXT("Short objective toasts should keep the original typewriter pacing."), FMath::Abs(ShortTypewriterDuration - ExpectedShortTypewriterDuration) <= KINDA_SMALL_NUMBER);
	TestTrue(TEXT("Short objective toasts should dismiss after display time plus original typewriter duration."), FMath::Abs(ShortDismissDelay - (DisplayDuration + ExpectedShortTypewriterDuration)) <= KINDA_SMALL_NUMBER);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FObjectiveToastWidgetQueuesAndDeduplicatesObjectiveFeedbackTest,
	"HorrorProject.UI.Widgets.ObjectiveToast.QueuesAndDeduplicatesObjectiveFeedback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FObjectiveToastWidgetQueuesAndDeduplicatesObjectiveFeedbackTest::RunTest(const FString& Parameters)
{
	UObjectiveToastWidget* Widget = NewObject<UObjectiveToastWidget>();
	TestNotNull(TEXT("Objective toast widget should be constructible for queue diagnostics."), Widget);
	if (!Widget)
	{
		return false;
	}

	const FGameplayTag CompletionTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.ObjectiveCompleted")), false);
	const FGameplayTag ClueTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.OptionalClue")), false);
	const FGameplayTag FailureTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.ObjectiveFailed")), false);

	Widget->ShowObjectiveWithPriority(
		CompletionTag,
		FText::FromString(TEXT("目标完成")),
		FText::FromString(TEXT("继续前进。")),
		20);
	TestTrue(TEXT("The first objective toast should become visible immediately."), Widget->IsToastVisible());
	TestEqual(TEXT("The active toast should preserve its event tag."), Widget->GetCurrentEventTagForTests(), CompletionTag);
	TestEqual(TEXT("The active toast should preserve its priority."), Widget->GetCurrentToastPriorityForTests(), 20);

	Widget->ShowObjectiveWithPriority(
		ClueTag,
		FText::FromString(TEXT("旁路线索")),
		FText::FromString(TEXT("已记录到档案。")),
		10);
	TestEqual(TEXT("Lower-priority objective feedback should queue behind the active toast."), Widget->GetQueuedToastCountForTests(), 1);
	TestEqual(TEXT("Lower-priority objective feedback should not overwrite the current toast."), Widget->GetCurrentObjectiveTextForTests().ToString(), FString(TEXT("目标完成")));

	Widget->ShowObjectiveWithPriority(
		ClueTag,
		FText::FromString(TEXT("旁路线索")),
		FText::FromString(TEXT("档案条目已更新。")),
		10);
	TestEqual(TEXT("Duplicate queued feedback should be merged instead of stacked."), Widget->GetQueuedToastCountForTests(), 1);

	Widget->ShowObjectiveWithPriority(
		FailureTag,
		FText::FromString(TEXT("目标失败")),
		FText::FromString(TEXT("重新互动可重试。")),
		90);
	TestEqual(TEXT("Critical objective feedback should interrupt the active toast."), Widget->GetCurrentObjectiveTextForTests().ToString(), FString(TEXT("目标失败")));
	TestEqual(TEXT("Interrupted and queued feedback should remain available after the critical toast."), Widget->GetQueuedToastCountForTests(), 2);

	Widget->DismissToast();
	TestEqual(TEXT("The interrupted toast should resume after the critical toast is dismissed."), Widget->GetCurrentObjectiveTextForTests().ToString(), FString(TEXT("目标完成")));
	Widget->DismissToast();
	TestEqual(TEXT("The deduplicated queued toast should play after the interrupted toast."), Widget->GetCurrentObjectiveTextForTests().ToString(), FString(TEXT("旁路线索")));
	TestEqual(TEXT("Merged queued feedback should keep the latest hint."), Widget->GetCurrentHintTextForTests().ToString(), FString(TEXT("档案条目已更新。")));
	Widget->DismissToast();
	TestFalse(TEXT("The widget should hide after the final queued toast is dismissed."), Widget->IsToastVisible());
	TestEqual(TEXT("The queue should be empty after every toast has played."), Widget->GetQueuedToastCountForTests(), 0);
	return true;
}

#endif
