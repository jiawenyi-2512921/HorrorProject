// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Evidence/EvidenceTypes.h"
#include "EvidenceToastWidget.generated.h"

/**
 * Toast notification for evidence collection with fade-in/out animation
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class HORRORPROJECT_API UEvidenceToastWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Evidence|UI")
	void ShowEvidenceToast(FName EvidenceId, const FHorrorEvidenceMetadata& Metadata);

	UFUNCTION(BlueprintCallable, Category="Evidence|UI")
	void ShowSimpleToast(FName EvidenceId, const FText& DisplayName);

	UFUNCTION(BlueprintCallable, Category="Evidence|UI")
	void DismissToast();

	UFUNCTION(BlueprintPure, Category="Evidence|UI")
	bool IsVisible() const { return bIsVisible; }

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="Evidence|UI", meta=(DisplayName="Show Toast"))
	void BP_ShowToast(FName EvidenceId, const FText& DisplayName, const FText& Description);

	UFUNCTION(BlueprintImplementableEvent, Category="Evidence|UI", meta=(DisplayName="Dismiss Toast"))
	void BP_DismissToast();

	UFUNCTION(BlueprintImplementableEvent, Category="Evidence|UI", meta=(DisplayName="Play Fade In"))
	void BP_PlayFadeIn();

	UFUNCTION(BlueprintImplementableEvent, Category="Evidence|UI", meta=(DisplayName="Play Fade Out"))
	void BP_PlayFadeOut();

	UPROPERTY(EditDefaultsOnly, Category="Evidence|UI")
	float DisplayDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category="Evidence|UI")
	float FadeInDuration = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category="Evidence|UI")
	float FadeOutDuration = 0.5f;

	UPROPERTY(BlueprintReadOnly, Category="Evidence|UI")
	bool bIsVisible = false;

private:
	FTimerHandle DismissTimerHandle;
};
