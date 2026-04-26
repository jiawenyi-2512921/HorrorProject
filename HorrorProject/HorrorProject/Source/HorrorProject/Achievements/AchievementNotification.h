#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AchievementDefinition.h"
#include "AchievementNotification.generated.h"

UCLASS()
class HORRORPROJECT_API UAchievementNotification : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Achievement")
	static void ShowAchievementNotification(UWorld* World, const FAchievementData& Achievement);

	UFUNCTION(BlueprintImplementableEvent, Category = "Achievement")
	void DisplayAchievement(const FAchievementData& Achievement);

	UFUNCTION(BlueprintCallable, Category = "Achievement")
	void ShowNotification(const FAchievementData& Achievement);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* AchievementNameText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* AchievementDescriptionText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* AchievementIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Achievement")
	float DisplayDuration = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Achievement")
	float FadeInDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Achievement")
	float FadeOutDuration = 0.5f;

private:
	FTimerHandle DisplayTimerHandle;

	void FadeIn();
	void FadeOut();
	void RemoveNotification();
};
