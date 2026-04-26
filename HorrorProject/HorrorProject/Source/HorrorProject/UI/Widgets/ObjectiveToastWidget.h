// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "ObjectiveToastWidget.generated.h"

/**
 * Toast notification for objective updates with typewriter text effect
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class HORRORPROJECT_API UObjectiveToastWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Objective|UI")
	void ShowObjectiveToast(FGameplayTag EventTag, const FText& ObjectiveText);

	UFUNCTION(BlueprintCallable, Category="Objective|UI")
	void ShowObjectiveWithHint(FGameplayTag EventTag, const FText& ObjectiveText, const FText& HintText);

	UFUNCTION(BlueprintCallable, Category="Objective|UI")
	void DismissToast();

	UFUNCTION(BlueprintPure, Category="Objective|UI")
	bool IsToastVisible() const { return bToastVisible; }

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="Objective|UI", meta=(DisplayName="Show Toast"))
	void BP_ShowToast(FGameplayTag EventTag, const FText& ObjectiveText, const FText& HintText);

	UFUNCTION(BlueprintImplementableEvent, Category="Objective|UI", meta=(DisplayName="Dismiss Toast"))
	void BP_DismissToast();

	UFUNCTION(BlueprintImplementableEvent, Category="Objective|UI", meta=(DisplayName="Play Typewriter Effect"))
	void BP_PlayTypewriterEffect(const FText& Text, float Duration);

	UFUNCTION(BlueprintImplementableEvent, Category="Objective|UI", meta=(DisplayName="Play Slide In"))
	void BP_PlaySlideIn();

	UFUNCTION(BlueprintImplementableEvent, Category="Objective|UI", meta=(DisplayName="Play Slide Out"))
	void BP_PlaySlideOut();

	UPROPERTY(EditDefaultsOnly, Category="Objective|UI")
	float DisplayDuration = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category="Objective|UI")
	float TypewriterSpeed = 0.05f;

	UPROPERTY(EditDefaultsOnly, Category="Objective|UI")
	float SlideInDuration = 0.4f;

	UPROPERTY(EditDefaultsOnly, Category="Objective|UI")
	float SlideOutDuration = 0.3f;

	UPROPERTY(BlueprintReadOnly, Category="Objective|UI")
	bool bToastVisible = false;

	UPROPERTY(BlueprintReadOnly, Category="Objective|UI")
	FGameplayTag CurrentEventTag;

private:
	FTimerHandle DismissTimerHandle;
};
