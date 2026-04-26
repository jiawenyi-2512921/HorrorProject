// Copyright Epic Games, Inc. All Rights Reserved.

#include "UIAnimationLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanelSlot.h"

void UUIAnimationLibrary::FadeWidget(UUserWidget* Widget, float TargetOpacity, float Duration, EUIAnimationCurve Curve)
{
	if (!Widget)
	{
		return;
	}

	// Blueprint implementation handles the actual animation
}

void UUIAnimationLibrary::SlideWidget(UUserWidget* Widget, FVector2D FromPosition, FVector2D ToPosition, float Duration, EUIAnimationCurve Curve)
{
	if (!Widget)
	{
		return;
	}

	// Blueprint implementation handles the actual animation
}

void UUIAnimationLibrary::ScaleWidget(UUserWidget* Widget, FVector2D TargetScale, float Duration, EUIAnimationCurve Curve)
{
	if (!Widget)
	{
		return;
	}

	// Blueprint implementation handles the actual animation
}

void UUIAnimationLibrary::PulseWidget(UUserWidget* Widget, float MinScale, float MaxScale, float Duration, bool bLoop)
{
	if (!Widget)
	{
		return;
	}

	// Blueprint implementation handles the actual animation
}

void UUIAnimationLibrary::TypewriterText(UUserWidget* Widget, const FText& FullText, float CharacterDelay)
{
	if (!Widget)
	{
		return;
	}

	// Blueprint implementation handles the actual animation
}

void UUIAnimationLibrary::GlitchWidget(UUserWidget* Widget, float Duration, float Intensity)
{
	if (!Widget)
	{
		return;
	}

	// Blueprint implementation handles the actual animation
}

float UUIAnimationLibrary::EvaluateCurve(float Alpha, EUIAnimationCurve Curve)
{
	switch (Curve)
	{
	case EUIAnimationCurve::Linear:
		return Alpha;
	case EUIAnimationCurve::EaseIn:
		return EaseIn(Alpha);
	case EUIAnimationCurve::EaseOut:
		return EaseOut(Alpha);
	case EUIAnimationCurve::EaseInOut:
		return EaseInOut(Alpha);
	case EUIAnimationCurve::Bounce:
		return Bounce(Alpha);
	case EUIAnimationCurve::Elastic:
		return Elastic(Alpha);
	default:
		return Alpha;
	}
}

float UUIAnimationLibrary::EaseIn(float Alpha)
{
	return Alpha * Alpha;
}

float UUIAnimationLibrary::EaseOut(float Alpha)
{
	return 1.0f - FMath::Square(1.0f - Alpha);
}

float UUIAnimationLibrary::EaseInOut(float Alpha)
{
	if (Alpha < 0.5f)
	{
		return 2.0f * Alpha * Alpha;
	}
	return 1.0f - FMath::Pow(-2.0f * Alpha + 2.0f, 2.0f) / 2.0f;
}

float UUIAnimationLibrary::Bounce(float Alpha)
{
	const float n1 = 7.5625f;
	const float d1 = 2.75f;

	if (Alpha < 1.0f / d1)
	{
		return n1 * Alpha * Alpha;
	}
	else if (Alpha < 2.0f / d1)
	{
		Alpha -= 1.5f / d1;
		return n1 * Alpha * Alpha + 0.75f;
	}
	else if (Alpha < 2.5f / d1)
	{
		Alpha -= 2.25f / d1;
		return n1 * Alpha * Alpha + 0.9375f;
	}
	else
	{
		Alpha -= 2.625f / d1;
		return n1 * Alpha * Alpha + 0.984375f;
	}
}

float UUIAnimationLibrary::Elastic(float Alpha)
{
	const float c4 = (2.0f * PI) / 3.0f;

	if (Alpha == 0.0f)
	{
		return 0.0f;
	}
	if (Alpha == 1.0f)
	{
		return 1.0f;
	}

	return FMath::Pow(2.0f, -10.0f * Alpha) * FMath::Sin((Alpha * 10.0f - 0.75f) * c4) + 1.0f;
}
