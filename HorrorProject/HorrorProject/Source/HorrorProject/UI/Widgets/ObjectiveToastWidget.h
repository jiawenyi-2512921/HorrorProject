// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "ObjectiveToastWidget.generated.h"

/**
 * Toast notification for objective updates with typewriter text effect
 */
UCLASS(BlueprintType, Blueprintable)
class HORRORPROJECT_API UObjectiveToastWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Objective|UI")
	void ShowObjectiveToast(FGameplayTag EventTag, const FText& ObjectiveText);

	UFUNCTION(BlueprintCallable, Category="Objective|UI")
	void ShowObjectiveWithHint(FGameplayTag EventTag, const FText& ObjectiveText, const FText& HintText);

	UFUNCTION(BlueprintCallable, Category="Objective|UI")
	void ShowObjectiveWithPriority(FGameplayTag EventTag, const FText& ObjectiveText, const FText& HintText, int32 ToastPriority);

	UFUNCTION(BlueprintCallable, Category="Objective|UI")
	void DismissToast();

	UFUNCTION(BlueprintPure, Category="Objective|UI")
	bool IsToastVisible() const { return bToastVisible; }

#if WITH_DEV_AUTOMATION_TESTS
	static float CalculateTypewriterDurationForTests(const FText& ObjectiveText, float InTypewriterSpeed, float InMaxTypewriterDuration);
	static float CalculateDismissDelayForTests(const FText& ObjectiveText, float InDisplayDuration, float InTypewriterSpeed, float InMaxTypewriterDuration);
	FGameplayTag GetCurrentEventTagForTests() const { return CurrentEventTag; }
	FText GetCurrentObjectiveTextForTests() const { return CurrentObjectiveText; }
	FText GetCurrentHintTextForTests() const { return CurrentHintText; }
	int32 GetCurrentToastPriorityForTests() const { return CurrentToastPriority; }
	int32 GetQueuedToastCountForTests() const { return QueuedToasts.Num(); }
#endif

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="Objective|UI", meta=(DisplayName="显示提示"))
	void BP_ShowToast(FGameplayTag EventTag, const FText& ObjectiveText, const FText& HintText);

	UFUNCTION(BlueprintImplementableEvent, Category="Objective|UI", meta=(DisplayName="关闭提示"))
	void BP_DismissToast();

	UFUNCTION(BlueprintImplementableEvent, Category="Objective|UI", meta=(DisplayName="播放打字机效果"))
	void BP_PlayTypewriterEffect(const FText& Text, float Duration);

	UFUNCTION(BlueprintImplementableEvent, Category="Objective|UI", meta=(DisplayName="播放滑入"))
	void BP_PlaySlideIn();

	UFUNCTION(BlueprintImplementableEvent, Category="Objective|UI", meta=(DisplayName="播放滑出"))
	void BP_PlaySlideOut();

	UPROPERTY(EditDefaultsOnly, Category="Objective|UI")
	float DisplayDuration = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category="Objective|UI")
	float TypewriterSpeed = 0.05f;

	UPROPERTY(EditDefaultsOnly, Category="Objective|UI", meta=(ClampMin="0.0", Units="s"))
	float MaxTypewriterDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category="Objective|UI")
	float SlideInDuration = 0.4f;

	UPROPERTY(EditDefaultsOnly, Category="Objective|UI")
	float SlideOutDuration = 0.3f;

	UPROPERTY(BlueprintReadOnly, Category="Objective|UI")
	bool bToastVisible = false;

	UPROPERTY(BlueprintReadOnly, Category="Objective|UI")
	FGameplayTag CurrentEventTag;

private:
	struct FQueuedObjectiveToast
	{
		FGameplayTag EventTag;
		FText ObjectiveText;
		FText HintText;
		int32 Priority = 0;
		int32 Sequence = 0;
	};

	static float CalculateTypewriterDuration(const FText& ObjectiveText, float InTypewriterSpeed, float InMaxTypewriterDuration);
	static float CalculateDismissDelay(const FText& ObjectiveText, float InDisplayDuration, float InTypewriterSpeed, float InMaxTypewriterDuration);
	static bool IsSameToast(const FQueuedObjectiveToast& Toast, FGameplayTag EventTag, const FText& ObjectiveText);

	void ShowToastNow(const FQueuedObjectiveToast& Toast);
	void HideActiveToast();
	void EnqueueToast(const FQueuedObjectiveToast& Toast);
	bool TryShowNextQueuedToast();

	TArray<FQueuedObjectiveToast> QueuedToasts;

	UPROPERTY(Transient)
	FText CurrentObjectiveText;

	UPROPERTY(Transient)
	FText CurrentHintText;

	FTimerHandle DismissTimerHandle;
	int32 CurrentToastPriority = 0;
	int32 NextToastSequence = 0;
};
