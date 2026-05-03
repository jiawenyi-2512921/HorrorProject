// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorAdvancedInteractionTypes.h"

namespace
{
	const FHorrorAdvancedInteractionInputOption* FindOptionByInputId(const FHorrorAdvancedInteractionHUDState& State, FName InputId)
	{
		return State.InputOptions.FindByPredicate(
			[InputId](const FHorrorAdvancedInteractionInputOption& Option)
			{
				return Option.InputId == InputId;
			});
	}

	const FHorrorAdvancedInteractionInputOption* FindOptionByVisualSlot(const FHorrorAdvancedInteractionHUDState& State, int32 VisualSlotIndex)
	{
		return State.InputOptions.FindByPredicate(
			[VisualSlotIndex](const FHorrorAdvancedInteractionInputOption& Option)
			{
				return Option.VisualSlotIndex == VisualSlotIndex;
			});
	}

	FName ResolveRenderableExpectedInputId(const FHorrorAdvancedInteractionHUDState& State)
	{
		if (FindOptionByInputId(State, State.ExpectedInputId))
		{
			return State.ExpectedInputId;
		}

		if (State.StepTrack.IsValidIndex(State.StepIndex) && FindOptionByInputId(State, State.StepTrack[State.StepIndex].InputId))
		{
			return State.StepTrack[State.StepIndex].InputId;
		}

		if (const FHorrorAdvancedInteractionInputOption* ProgressOption = State.InputOptions.FindByPredicate(
			[](const FHorrorAdvancedInteractionInputOption& Option)
			{
				return !Option.InputId.IsNone() && Option.bAdvancesObjective && !Option.bHazardous;
			}))
		{
			return ProgressOption->InputId;
		}

		if (const FHorrorAdvancedInteractionInputOption* FallbackOption = State.InputOptions.FindByPredicate(
			[](const FHorrorAdvancedInteractionInputOption& Option)
			{
				return !Option.InputId.IsNone();
			}))
		{
			return FallbackOption->InputId;
		}

		return NAME_None;
	}

	int32 ResolveRenderableActiveSlot(const FHorrorAdvancedInteractionHUDState& State)
	{
		if (FindOptionByVisualSlot(State, State.ActiveInputSlotIndex))
		{
			return State.ActiveInputSlotIndex;
		}

		if (const FHorrorAdvancedInteractionInputOption* ExpectedOption = FindOptionByInputId(State, State.ExpectedInputId))
		{
			return ExpectedOption->VisualSlotIndex;
		}

		return INDEX_NONE;
	}
}

bool IsHorrorAdvancedInteractionPanelMode(EHorrorCampaignInteractionMode Mode)
{
	return Mode == EHorrorCampaignInteractionMode::CircuitWiring
		|| Mode == EHorrorCampaignInteractionMode::GearCalibration
		|| Mode == EHorrorCampaignInteractionMode::SpectralScan
		|| Mode == EHorrorCampaignInteractionMode::SignalTuning;
}

void NormalizeHorrorAdvancedInteractionHUDState(FHorrorAdvancedInteractionHUDState& State)
{
	State.ProgressFraction = FMath::Clamp(State.ProgressFraction, 0.0f, 1.0f);
	State.TimingFraction = FMath::Clamp(State.TimingFraction, 0.0f, 1.0f);
	State.TimingWindowStartFraction = FMath::Clamp(State.TimingWindowStartFraction, 0.0f, 1.0f);
	State.TimingWindowEndFraction = FMath::Clamp(State.TimingWindowEndFraction, 0.0f, 1.0f);
	if (State.TimingWindowEndFraction < State.TimingWindowStartFraction)
	{
		Swap(State.TimingWindowStartFraction, State.TimingWindowEndFraction);
	}
	State.StabilityFraction = FMath::Clamp(State.StabilityFraction, 0.0f, 1.0f);
	State.SignalBalanceFraction = FMath::Clamp(State.SignalBalanceFraction, 0.0f, 1.0f);
	State.SignalTargetBalanceFraction = FMath::Clamp(State.SignalTargetBalanceFraction, 0.0f, 1.0f);
	State.SpectralConfidenceFraction = FMath::Clamp(State.SpectralConfidenceFraction, 0.0f, 1.0f);
	State.SpectralNoiseFraction = FMath::Clamp(State.SpectralNoiseFraction, 0.0f, 1.0f);
	State.SpectralFilterFocusFraction = FMath::Clamp(State.SpectralFilterFocusFraction, 0.0f, 1.0f);
	State.SpectralTargetFocusFraction = FMath::Clamp(State.SpectralTargetFocusFraction, 0.0f, 1.0f);
	State.PerformanceGradeFraction = FMath::Clamp(State.PerformanceGradeFraction, 0.0f, 1.0f);
	State.InputPrecisionFraction = FMath::Clamp(State.InputPrecisionFraction, 0.0f, 1.0f);
	State.DeviceLoadFraction = FMath::Clamp(State.DeviceLoadFraction, 0.0f, 1.0f);
	State.RouteFlowFraction = FMath::Clamp(State.RouteFlowFraction, 0.0f, 1.0f);
	State.HazardPressureFraction = FMath::Clamp(State.HazardPressureFraction, 0.0f, 1.0f);
	State.TargetAlignmentFraction = FMath::Clamp(State.TargetAlignmentFraction, 0.0f, 1.0f);
	State.PauseRemainingSeconds = FMath::Max(0.0f, State.PauseRemainingSeconds);
	State.StepIndex = FMath::Max(0, State.StepIndex);
	State.RequiredStepCount = FMath::Max(0, State.RequiredStepCount);
	State.ComboCount = FMath::Max(0, State.ComboCount);
	State.MistakeCount = FMath::Max(0, State.MistakeCount);

	for (int32 OptionIndex = 0; OptionIndex < State.InputOptions.Num(); ++OptionIndex)
	{
		FHorrorAdvancedInteractionInputOption& Option = State.InputOptions[OptionIndex];
		if (Option.VisualSlotIndex < 0)
		{
			Option.VisualSlotIndex = OptionIndex;
		}
		Option.LoadFraction = FMath::Clamp(Option.LoadFraction, 0.0f, 1.0f);
		Option.MotionFraction = FMath::Clamp(Option.MotionFraction, 0.0f, 1.0f);
		Option.ResponseWindowFraction = FMath::Clamp(Option.ResponseWindowFraction, 0.0f, 1.0f);
	}

	for (FHorrorAdvancedInteractionStepTrackItem& TrackItem : State.StepTrack)
	{
		TrackItem.StepIndex = FMath::Max(0, TrackItem.StepIndex);
		if (const FHorrorAdvancedInteractionInputOption* Option = FindOptionByInputId(State, TrackItem.InputId))
		{
			if (!FindOptionByVisualSlot(State, TrackItem.VisualSlotIndex))
			{
				TrackItem.VisualSlotIndex = Option->VisualSlotIndex;
			}
			if (TrackItem.DisplayLabel.IsEmpty())
			{
				TrackItem.DisplayLabel = Option->DisplayLabel;
			}
			if (TrackItem.KeyHint.IsEmpty())
			{
				TrackItem.KeyHint = Option->KeyHint;
			}
		}
	}

	if (!State.bVisible || !IsHorrorAdvancedInteractionPanelMode(State.Mode))
	{
		State.ActiveInputSlotIndex = INDEX_NONE;
		return;
	}

	if (State.RequiredStepCount <= 0 && !State.StepTrack.IsEmpty())
	{
		State.RequiredStepCount = State.StepTrack.Num();
	}
	State.RequiredStepCount = FMath::Max(1, State.RequiredStepCount);
	if (State.InputOptions.IsEmpty() || State.StepTrack.Num() < State.RequiredStepCount)
	{
		State.bVisible = false;
		State.ActiveInputSlotIndex = INDEX_NONE;
		return;
	}

	State.StepIndex = FMath::Clamp(State.StepIndex, 0, State.RequiredStepCount - 1);
	State.ExpectedInputId = ResolveRenderableExpectedInputId(State);
	if (State.ExpectedInputId.IsNone())
	{
		State.bVisible = false;
		State.ActiveInputSlotIndex = INDEX_NONE;
		return;
	}

	State.ActiveInputSlotIndex = ResolveRenderableActiveSlot(State);
}
