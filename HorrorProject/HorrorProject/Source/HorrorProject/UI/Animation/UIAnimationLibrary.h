// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UIAnimationLibrary.generated.h"

class UUserWidget;

UENUM(BlueprintType)
enum class EUIAnimationCurve : uint8
{
	Linear,
	EaseIn,
	EaseOut,
	EaseInOut,
	Bounce,
	Elastic
};

/**
 * UI animation utility library for common UI transitions
 */
UCLASS()
class HORRORPROJECT_API UUIAnimationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="UI|Animation")
	static void FadeWidget(UUserWidget* Widget, float TargetOpacity, float Duration, EUIAnimationCurve Curve = EUIAnimationCurve::EaseInOut);

	UFUNCTION(BlueprintCallable, Category="UI|Animation")
	static void SlideWidget(UUserWidget* Widget, FVector2D FromPosition, FVector2D ToPosition, float Duration, EUIAnimationCurve Curve = EUIAnimationCurve::EaseOut);

	UFUNCTION(BlueprintCallable, Category="UI|Animation")
	static void ScaleWidget(UUserWidget* Widget, FVector2D TargetScale, float Duration, EUIAnimationCurve Curve = EUIAnimationCurve::EaseInOut);

	UFUNCTION(BlueprintCallable, Category="UI|Animation")
	static void PulseWidget(UUserWidget* Widget, float MinScale, float MaxScale, float Duration, bool bLoop = true);

	UFUNCTION(BlueprintCallable, Category="UI|Animation")
	static void TypewriterText(UUserWidget* Widget, const FText& FullText, float CharacterDelay = 0.05f);

	UFUNCTION(BlueprintCallable, Category="UI|Animation")
	static void GlitchWidget(UUserWidget* Widget, float Duration, float Intensity);

	UFUNCTION(BlueprintPure, Category="UI|Animation")
	static float EvaluateCurve(float Alpha, EUIAnimationCurve Curve);

	UFUNCTION(BlueprintPure, Category="UI|Animation")
	static float EaseIn(float Alpha);

	UFUNCTION(BlueprintPure, Category="UI|Animation")
	static float EaseOut(float Alpha);

	UFUNCTION(BlueprintPure, Category="UI|Animation")
	static float EaseInOut(float Alpha);

	UFUNCTION(BlueprintPure, Category="UI|Animation")
	static float Bounce(float Alpha);

	UFUNCTION(BlueprintPure, Category="UI|Animation")
	static float Elastic(float Alpha);
};
