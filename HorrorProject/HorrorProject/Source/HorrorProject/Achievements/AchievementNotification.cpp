#include "AchievementNotification.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void UAchievementNotification::ShowAchievementNotification(UWorld* World, const FAchievementData& Achievement)
{
	if (!World)
	{
		return;
	}

	// Create widget
	TSubclassOf<UAchievementNotification> WidgetClass = LoadClass<UAchievementNotification>(nullptr, TEXT("/Game/UI/Achievements/WBP_AchievementNotification.WBP_AchievementNotification_C"));

	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Achievement notification widget class not found"));
		return;
	}

	UAchievementNotification* NotificationWidget = CreateWidget<UAchievementNotification>(World, WidgetClass);
	if (NotificationWidget)
	{
		NotificationWidget->AddToViewport(100); // High Z-order
		NotificationWidget->ShowNotification(Achievement);
	}
}

void UAchievementNotification::ShowNotification(const FAchievementData& Achievement)
{
	// Set text
	if (AchievementNameText)
	{
		AchievementNameText->SetText(Achievement.Name);
	}

	if (AchievementDescriptionText)
	{
		AchievementDescriptionText->SetText(Achievement.Description);
	}

	// Load and set icon
	if (AchievementIcon && !Achievement.Icon.IsNull())
	{
		UTexture2D* IconTexture = Achievement.Icon.LoadSynchronous();
		if (IconTexture)
		{
			AchievementIcon->SetBrushFromTexture(IconTexture);
		}
	}

	// Play animation
	FadeIn();

	// Set timer to fade out
	GetWorld()->GetTimerManager().SetTimer(
		DisplayTimerHandle,
		this,
		&UAchievementNotification::FadeOut,
		DisplayDuration,
		false
	);

	// Play sound
	// UGameplayStatics::PlaySound2D(GetWorld(), AchievementUnlockSound);

	// Call blueprint event
	DisplayAchievement(Achievement);
}

void UAchievementNotification::NativeConstruct()
{
	Super::NativeConstruct();
}

void UAchievementNotification::FadeIn()
{
	// Play fade in animation
	PlayAnimationForward(GetAnimationByName(FName("FadeIn")), FadeInDuration);
}

void UAchievementNotification::FadeOut()
{
	// Play fade out animation
	PlayAnimationReverse(GetAnimationByName(FName("FadeOut")), FadeOutDuration);

	// Set timer to remove widget
	GetWorld()->GetTimerManager().SetTimer(
		DisplayTimerHandle,
		this,
		&UAchievementNotification::RemoveNotification,
		FadeOutDuration,
		false
	);
}

void UAchievementNotification::RemoveNotification()
{
	RemoveFromParent();
}
