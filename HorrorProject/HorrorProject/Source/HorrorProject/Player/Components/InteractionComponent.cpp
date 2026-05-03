// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/InteractionComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/TimelineComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Game/FoundFootageObjectiveInteractable.h"
#include "Game/HorrorCampaignObjectiveActor.h"
#include "HorrorProject.h"
#include "Interaction/BaseInteractable.h"
#include "Interaction/DoorInteractable.h"
#include "Interaction/InteractableInterface.h"
#include "Player/HorrorPlayerController.h"

namespace
{
static const FName LegacyInteractionFunctionNames[] = {
	TEXT("Interact"),
	TEXT("OnInteract"),
	TEXT("Use"),
	TEXT("ToggleDoor"),
	TEXT("OpenDoor")
};

UFunction* FindValidatedLegacyInteractionFunction(AActor* TargetActor)
{
	if (!TargetActor)
	{
		return nullptr;
	}

	for (const FName FunctionName : LegacyInteractionFunctionNames)
	{
		UFunction* Function = TargetActor->FindFunction(FunctionName);
		if (Function && Function->NumParms == 0)
		{
			return Function;
		}
	}

	return nullptr;
}

FText StripInteractionPromptPrefix(FText PromptText)
{
	FString Message = PromptText.ToString();
	if (Message.StartsWith(TEXT("互动键：")))
	{
		Message.RightChopInline(4, EAllowShrinking::No);
	}
	else if (Message.StartsWith(TEXT("E  ")))
	{
		Message.RightChopInline(3, EAllowShrinking::No);
	}

	Message.TrimStartAndEndInline();
	return Message.IsEmpty() ? FText::GetEmpty() : FText::FromString(Message);
}

FText DefaultInteractionInputText(EHorrorInteractionInputStyle InputStyle)
{
	switch (InputStyle)
	{
	case EHorrorInteractionInputStyle::Hold:
		return NSLOCTEXT("InteractionComponent", "HoldInteractInput", "按住互动键");
	case EHorrorInteractionInputStyle::Modal:
		return NSLOCTEXT("InteractionComponent", "ModalInteractInput", "互动键");
	case EHorrorInteractionInputStyle::RecordAim:
		return NSLOCTEXT("InteractionComponent", "RecordAimInput", "开启录像并保持对准");
	case EHorrorInteractionInputStyle::Timed:
		return NSLOCTEXT("InteractionComponent", "TimedInteractInput", "互动键");
	case EHorrorInteractionInputStyle::Press:
	default:
		return NSLOCTEXT("InteractionComponent", "PressInteractInput", "互动键");
	}
}

FName ResolveBaseInteractableId(const ABaseInteractable* BaseInteractable, const AActor* FallbackActor)
{
	if (BaseInteractable && !BaseInteractable->GetInteractableId().IsNone())
	{
		return BaseInteractable->GetInteractableId();
	}

	return FallbackActor ? FallbackActor->GetFName() : NAME_None;
}

EHorrorInteractionRiskLevel ConvertCampaignRiskLevel(EHorrorCampaignObjectiveRiskLevel CampaignRiskLevel)
{
	switch (CampaignRiskLevel)
	{
	case EHorrorCampaignObjectiveRiskLevel::Low:
		return EHorrorInteractionRiskLevel::Low;
	case EHorrorCampaignObjectiveRiskLevel::Medium:
		return EHorrorInteractionRiskLevel::Medium;
	case EHorrorCampaignObjectiveRiskLevel::High:
	case EHorrorCampaignObjectiveRiskLevel::Critical:
		return EHorrorInteractionRiskLevel::High;
	case EHorrorCampaignObjectiveRiskLevel::None:
	default:
			return EHorrorInteractionRiskLevel::None;
	}
}

FText ResolveCampaignInteractionInputText(const FHorrorCampaignObjectiveDefinition& ObjectiveDefinition, const FHorrorCampaignObjectiveRuntimeState& RuntimeState)
{
	if (!ObjectiveDefinition.Presentation.MechanicLabel.IsEmpty())
	{
		return ObjectiveDefinition.Presentation.MechanicLabel;
	}

	switch (RuntimeState.InteractionMode)
	{
	case EHorrorCampaignInteractionMode::CircuitWiring:
		return NSLOCTEXT("InteractionComponent", "CampaignCircuitWindowInput", "电路接线窗口");
	case EHorrorCampaignInteractionMode::GearCalibration:
		return NSLOCTEXT("InteractionComponent", "CampaignGearWindowInput", "齿轮校准窗口");
	case EHorrorCampaignInteractionMode::SpectralScan:
		return RuntimeState.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan
			&& RuntimeState.RiskLevel == EHorrorCampaignObjectiveRiskLevel::Critical
			? NSLOCTEXT("InteractionComponent", "CampaignBossSpectralWindowInput", "首领共鸣频谱窗口")
			: NSLOCTEXT("InteractionComponent", "CampaignSpectralWindowInput", "频谱扫描窗口");
	case EHorrorCampaignInteractionMode::SignalTuning:
		return ObjectiveDefinition.ObjectiveType == EHorrorCampaignObjectiveType::PlantBeacon
			? NSLOCTEXT("InteractionComponent", "CampaignBeaconTuningWindowInput", "信标锚定调谐窗口")
			: NSLOCTEXT("InteractionComponent", "CampaignSignalTuningWindowInput", "信号调谐窗口");
	default:
		return DefaultInteractionInputText(EHorrorInteractionInputStyle::Modal);
	}
}

FText BuildCampaignPanelEntryDetailText(const FText& WindowText)
{
	return FText::Format(
		NSLOCTEXT("InteractionComponent", "CampaignPanelEntryDetail", "按互动键打开{0}。"),
		WindowText.IsEmpty() ? DefaultInteractionInputText(EHorrorInteractionInputStyle::Modal) : WindowText);
}

FText BuildCampaignRetryableInputText(const FText& WindowText)
{
	return FText::Format(
		NSLOCTEXT("InteractionComponent", "CampaignRetryablePanelInput", "重新互动：{0}"),
		WindowText.IsEmpty() ? DefaultInteractionInputText(EHorrorInteractionInputStyle::Modal) : WindowText);
}

FText AppendCampaignDetailLine(const FText& First, const FText& Second)
{
	if (First.IsEmpty())
	{
		return Second;
	}

	if (Second.IsEmpty())
	{
		return First;
	}

	return FText::Format(
		NSLOCTEXT("InteractionComponent", "CampaignDetailJoin", "{0}；{1}"),
		First,
		Second);
}

FText BuildCampaignRetryableDetailText(const FHorrorCampaignObjectiveRuntimeState& RuntimeState)
{
	FText DetailText = AppendCampaignDetailLine(RuntimeState.DeviceStatusLabel, RuntimeState.FailureRecoveryLabel);
	DetailText = AppendCampaignDetailLine(DetailText, RuntimeState.PhaseText);
	return DetailText.IsEmpty()
		? NSLOCTEXT("InteractionComponent", "CampaignRetryableFallbackDetail", "设备过载保护已触发，按互动键重新互动并重试当前窗口。")
		: DetailText;
}

AHorrorCampaignObjectiveActor* ResolveCampaignObjectiveFromInteractionHit(UObject* InterfaceTarget, const FHitResult& Hit)
{
	if (AHorrorCampaignObjectiveActor* ObjectiveActor = Cast<AHorrorCampaignObjectiveActor>(InterfaceTarget))
	{
		return ObjectiveActor;
	}

	if (const UActorComponent* InterfaceComponent = Cast<UActorComponent>(InterfaceTarget))
	{
		if (AHorrorCampaignObjectiveActor* ObjectiveActor = Cast<AHorrorCampaignObjectiveActor>(InterfaceComponent->GetOwner()))
		{
			return ObjectiveActor;
		}
	}

	if (AHorrorCampaignObjectiveActor* ObjectiveActor = Cast<AHorrorCampaignObjectiveActor>(Hit.GetActor()))
	{
		return ObjectiveActor;
	}

	if (const UActorComponent* HitComponent = Hit.GetComponent())
	{
		if (AHorrorCampaignObjectiveActor* ObjectiveActor = Cast<AHorrorCampaignObjectiveActor>(HitComponent->GetOwner()))
		{
			return ObjectiveActor;
		}
	}

	return nullptr;
}

AHorrorPlayerController* ResolveHorrorPlayerControllerFromInteractionOwner(const AActor* OwnerActor)
{
	if (const APawn* OwnerPawn = Cast<APawn>(OwnerActor))
	{
		if (AHorrorPlayerController* PlayerController = Cast<AHorrorPlayerController>(OwnerPawn->GetController()))
		{
			return PlayerController;
		}
	}

	return const_cast<AHorrorPlayerController*>(Cast<AHorrorPlayerController>(OwnerActor));
}
}

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UInteractionComponent::TryInteract()
{
	FHitResult Hit;
	if (!FindInteractionHit(Hit))
	{
		return false;
	}

	return TryInteractWithHit(Hit);
}

bool UInteractionComponent::GetFocusedInteractionPrompt(FText& OutPrompt) const
{
	FHitResult Hit;
	if (!FindInteractionHit(Hit))
	{
		OutPrompt = FText::GetEmpty();
		return false;
	}

	UObject* TargetObject = ResolveInterfaceTarget(Hit);
	if (!TargetObject)
	{
		OutPrompt = FText::GetEmpty();
		return false;
	}

	OutPrompt = BuildInteractionPrompt(TargetObject, Hit);
	return !OutPrompt.IsEmpty();
}

bool UInteractionComponent::GetFocusedInteractionContext(FHorrorInteractionContext& OutContext) const
{
	OutContext = FHorrorInteractionContext();

	FHitResult Hit;
	if (!FindInteractionHit(Hit))
	{
		return false;
	}

	UObject* TargetObject = ResolveInterfaceTarget(Hit);
	if (!TargetObject)
	{
		return false;
	}

	OutContext = BuildInteractionContext(TargetObject, Hit);
	return OutContext.bVisible && OutContext.HasAnyText();
}

bool UInteractionComponent::FindFocusedInteractable(FHitResult& OutHit, UObject*& OutTargetObject) const
{
	OutTargetObject = nullptr;

	if (!FindFocusedInteractionTarget(OutHit, OutTargetObject))
	{
		return false;
	}

	if (!CanInvokeInteractableInterface(OutTargetObject, OutHit))
	{
		OutHit = FHitResult();
		OutTargetObject = nullptr;
		return false;
	}

	return true;
}

bool UInteractionComponent::FindFocusedInteractionTarget(FHitResult& OutHit, UObject*& OutTargetObject) const
{
	OutTargetObject = nullptr;

	if (!FindInteractionHit(OutHit))
	{
		return false;
	}

	UObject* InterfaceTarget = ResolveInterfaceTarget(OutHit);
	if (!InterfaceTarget)
	{
		OutHit = FHitResult();
		return false;
	}

	OutTargetObject = InterfaceTarget;
	return true;
}

/**
 * Attempts to interact with a specific hit result
 * Resolves the appropriate interaction method (interface, legacy function, or door timeline)
 * @param Hit - The raycast hit result containing the target object
 * @return True if interaction was successfully invoked
 */
bool UInteractionComponent::TryInteractWithHit(const FHitResult& Hit) const
{
	AActor* TargetActor = Hit.GetActor();
	if (!TargetActor)
	{
		TargetActor = Hit.GetComponent() ? Hit.GetComponent()->GetOwner() : nullptr;
	}
	if (UObject* InterfaceTarget = ResolveInterfaceTarget(Hit))
	{
		const bool bInteracted = TryInvokeInteractableInterface(InterfaceTarget, Hit);
		if (bInteracted)
		{
			if (AHorrorCampaignObjectiveActor* CampaignObjective = ResolveCampaignObjectiveFromInteractionHit(InterfaceTarget, Hit))
			{
				if (CampaignObjective->IsAdvancedInteractionActive())
				{
					if (AHorrorPlayerController* PlayerController = ResolveHorrorPlayerControllerFromInteractionOwner(GetOwner()))
					{
						PlayerController->NotifyAdvancedInteractionObjectiveOpened(CampaignObjective);
					}
				}
			}
		}
		return bInteracted;
	}

	if (!TargetActor)
	{
		return false;
	}

	if (bEnableLegacyFunctionFallback && TryInvokeLegacyInteractionFunction(TargetActor))
	{
		UE_LOG(LogHorrorProject, Verbose, TEXT("Interaction used legacy function fallback on %s."), *GetNameSafe(TargetActor));
		return true;
	}

	if (bEnableLegacyDoorTimelineFallback && TryPlayDoorTimeline(TargetActor))
	{
		UE_LOG(LogHorrorProject, Verbose, TEXT("Interaction used legacy door timeline fallback on %s."), *GetNameSafe(TargetActor));
		return true;
	}

	return false;
}

/**
 * Performs interaction raycast from player viewpoint
 * First attempts line trace, then falls back to sphere sweep for narrow targets
 * @param OutHit - Populated with hit information if successful
 * @return True if a valid interactable object was found
 */
bool UInteractionComponent::FindInteractionHit(FHitResult& OutHit) const
{
	const AActor* OwnerActor = GetOwner();
	const UWorld* World = GetWorld();
	FVector Start = FVector::ZeroVector;
	FVector ViewDirection = FVector::ForwardVector;

	if (!OwnerActor || !World)
	{
		UE_LOG(LogHorrorProject, Warning, TEXT("Interaction trace aborted: owner or world was unavailable."));
		return false;
	}

	if (!ResolveViewPoint(Start, ViewDirection))
	{
		UE_LOG(LogHorrorProject, Warning, TEXT("Interaction trace aborted: no valid viewpoint was found for %s."), *GetNameSafe(OwnerActor));
		return false;
	}

	const float EffectiveTraceDistance = FMath::Max(TraceDistance, DefaultTraceDistance);
	const FVector End = Start + (ViewDirection * EffectiveTraceDistance);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(HorrorInteractionTrace), bTraceComplex, OwnerActor);
	QueryParams.AddIgnoredActor(OwnerActor);

	if (World->LineTraceSingleByChannel(OutHit, Start, End, TraceChannel, QueryParams))
	{
		if (IsInteractionCandidate(OutHit))
		{
			return true;
		}

		OutHit = FHitResult();
	}

	return FindInteractionHitBySweep(Start, End, QueryParams, OutHit);
}

bool UInteractionComponent::FindInteractionHitBySweep(const FVector& Start, const FVector& End, const FCollisionQueryParams& QueryParams, FHitResult& OutHit) const
{
	if (!bEnableNarrowTargetSweepFallback || NarrowTargetTraceRadius <= 0.0f)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	TArray<FHitResult> SweepHits;
	const FCollisionShape SweepShape = FCollisionShape::MakeSphere(NarrowTargetTraceRadius);
	if (!World->SweepMultiByChannel(SweepHits, Start, End, FQuat::Identity, TraceChannel, SweepShape, QueryParams))
	{
		return false;
	}

	const FVector TraceDirection = (End - Start).GetSafeNormal();
	SweepHits.Sort([this, Start, End, TraceDirection](const FHitResult& Left, const FHitResult& Right)
	{
		const float LeftPerpendicularDistance = CalculatePerpendicularDistanceToTraceForTests(Start, End, Left.ImpactPoint);
		const float RightPerpendicularDistance = CalculatePerpendicularDistanceToTraceForTests(Start, End, Right.ImpactPoint);
		if (!FMath::IsNearlyEqual(LeftPerpendicularDistance, RightPerpendicularDistance))
		{
			return LeftPerpendicularDistance < RightPerpendicularDistance;
		}

		const float LeftForwardDistance = FVector::DotProduct(Left.ImpactPoint - Start, TraceDirection);
		const float RightForwardDistance = FVector::DotProduct(Right.ImpactPoint - Start, TraceDirection);
		return LeftForwardDistance < RightForwardDistance;
	});

	for (const FHitResult& SweepHit : SweepHits)
	{
		if (CalculatePerpendicularDistanceToTraceForTests(Start, End, SweepHit.ImpactPoint) > NarrowTargetTraceRadius)
		{
			continue;
		}

		if (IsInteractionCandidate(SweepHit))
		{
			OutHit = SweepHit;
			return true;
		}
	}

	return false;
}

bool UInteractionComponent::IsInteractionCandidate(const FHitResult& Hit) const
{
	if (ResolveInterfaceTarget(Hit))
	{
		return true;
	}

	AActor* TargetActor = Hit.GetActor();
	if (!TargetActor)
	{
		return false;
	}

	if (bEnableLegacyFunctionFallback && HasLegacyInteractionFunction(TargetActor))
	{
		return true;
	}

	return bEnableLegacyDoorTimelineFallback && HasLegacyDoorTimeline(TargetActor);
}

UObject* UInteractionComponent::ResolveInterfaceTarget(const FHitResult& Hit) const
{
	if (UPrimitiveComponent* HitComponent = Hit.GetComponent())
	{
		if (HitComponent->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
		{
			return HitComponent;
		}
	}

	AActor* HitActor = Hit.GetActor();
	if (HitActor && HitActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
	{
		return HitActor;
	}

	return nullptr;
}

bool UInteractionComponent::CanInvokeInteractableInterface(UObject* TargetObject, const FHitResult& Hit) const
{
	if (!TargetObject || !TargetObject->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
	{
		return false;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return false;
	}

	IInteractableInterface* NativeInterface = Cast<IInteractableInterface>(TargetObject);
	if (!NativeInterface)
	{
		return IInteractableInterface::Execute_CanInteract(TargetObject, OwnerActor, Hit);
	}

	UFunction* CanInteractFunction = TargetObject->FindFunction(TEXT("CanInteract"));
	if (CanInteractFunction && CanInteractFunction->Script.Num() > 0)
	{
		return IInteractableInterface::Execute_CanInteract(TargetObject, OwnerActor, Hit);
	}

	return NativeInterface->CanInteract_Implementation(OwnerActor, Hit);
}

FText UInteractionComponent::BuildInteractionPrompt(UObject* TargetObject, const FHitResult& Hit) const
{
	AActor* TargetActor = Cast<AActor>(TargetObject);
	if (!TargetActor)
	{
		TargetActor = Hit.GetActor();
	}

	if (const ADoorInteractable* Door = Cast<ADoorInteractable>(TargetActor))
	{
		if (Door->RequiresPassword() && !Door->IsPasswordUnlocked())
		{
			return FText::FromString(TEXT("互动键：输入门禁密码"));
		}

		if (Door->IsOpen())
		{
			return FText::FromString(TEXT("互动键：关门"));
		}

		return FText::FromString(TEXT("互动键：开门"));
	}

	if (const AFoundFootageObjectiveInteractable* ObjectiveActor = Cast<AFoundFootageObjectiveInteractable>(TargetActor))
	{
		return ObjectiveActor->GetInteractionPromptText(GetOwner());
	}

	if (const AHorrorCampaignObjectiveActor* CampaignObjectiveActor = Cast<AHorrorCampaignObjectiveActor>(TargetActor))
	{
		return CampaignObjectiveActor->GetInteractionPromptText();
	}

	if (const ABaseInteractable* BaseInteractable = Cast<ABaseInteractable>(TargetActor))
	{
		const FText PromptText = BaseInteractable->GetInteractionPromptText();
		return PromptText.IsEmpty()
			? FText::FromString(TEXT("互动键：互动"))
			: FText::Format(FText::FromString(TEXT("互动键：{0}")), PromptText);
	}

	return FText::FromString(TEXT("互动键：互动"));
}

FHorrorInteractionContext UInteractionComponent::BuildInteractionContext(UObject* TargetObject, const FHitResult& Hit) const
{
	FHorrorInteractionContext Context;
	if (!TargetObject)
	{
		return Context;
	}

	AActor* TargetActor = Cast<AActor>(TargetObject);
	if (!TargetActor)
	{
		TargetActor = Hit.GetActor();
	}

	Context.bVisible = true;
	Context.bCanInteract = CanInvokeInteractableInterface(TargetObject, Hit);
	Context.ActionText = StripInteractionPromptPrefix(BuildInteractionPrompt(TargetObject, Hit));
	Context.InputStyle = EHorrorInteractionInputStyle::Press;
	Context.InputText = DefaultInteractionInputText(Context.InputStyle);
	Context.TargetId = TargetActor ? TargetActor->GetFName() : NAME_None;
	Context.RiskLevel = EHorrorInteractionRiskLevel::Low;

	if (const ADoorInteractable* Door = Cast<ADoorInteractable>(TargetActor))
	{
		Context.TargetId = ResolveBaseInteractableId(Door, TargetActor);
		if (Door->RequiresPassword() && !Door->IsPasswordUnlocked())
		{
			Context.Verb = EHorrorInteractionVerb::EnterCode;
			Context.InputStyle = EHorrorInteractionInputStyle::Modal;
			Context.InputText = DefaultInteractionInputText(Context.InputStyle);
			Context.bOpensPanel = true;
			Context.RiskLevel = EHorrorInteractionRiskLevel::Medium;
			Context.ActionText = NSLOCTEXT("InteractionComponent", "EnterDoorCodeAction", "输入门禁密码");
			Context.DetailText = Door->GetPasswordHint().IsEmpty()
				? NSLOCTEXT("InteractionComponent", "DoorNeedsCodeDetail", "这扇门需要门禁码。")
				: Door->GetPasswordHint();
			return Context;
		}

		Context.Verb = Door->IsOpen() ? EHorrorInteractionVerb::Close : EHorrorInteractionVerb::Open;
		Context.ActionText = Door->IsOpen()
			? NSLOCTEXT("InteractionComponent", "CloseDoorAction", "关门")
			: NSLOCTEXT("InteractionComponent", "OpenDoorAction", "开门");
		return Context;
	}

	if (const AFoundFootageObjectiveInteractable* ObjectiveActor = Cast<AFoundFootageObjectiveInteractable>(TargetActor))
	{
		Context.TargetId = ObjectiveActor->SourceId.IsNone() ? ObjectiveActor->GetFName() : ObjectiveActor->SourceId;
		Context.RiskLevel = EHorrorInteractionRiskLevel::Medium;

		switch (ObjectiveActor->Objective)
		{
		case EFoundFootageInteractableObjective::Bodycam:
			Context.Verb = EHorrorInteractionVerb::Interact;
			Context.ActionText = Context.ActionText.IsEmpty()
				? NSLOCTEXT("InteractionComponent", "RecoverBodycamAction", "取回随身摄像机")
				: Context.ActionText;
			break;

		case EFoundFootageInteractableObjective::FirstNote:
			Context.Verb = EHorrorInteractionVerb::Review;
			Context.ActionText = Context.ActionText.IsEmpty()
				? NSLOCTEXT("InteractionComponent", "ReadFirstNoteAction", "阅读站内备忘录")
				: Context.ActionText;
			break;

		case EFoundFootageInteractableObjective::FirstAnomalyCandidate:
		case EFoundFootageInteractableObjective::FirstAnomalyRecord:
			Context.Verb = EHorrorInteractionVerb::Record;
			Context.InputStyle = EHorrorInteractionInputStyle::RecordAim;
			Context.InputText = DefaultInteractionInputText(Context.InputStyle);
			Context.bRequiresRecording = true;
			Context.RiskLevel = EHorrorInteractionRiskLevel::High;
			Context.ActionText = Context.bCanInteract
				? (Context.ActionText.IsEmpty() ? NSLOCTEXT("InteractionComponent", "RecordAnomalyAction", "记录异常") : Context.ActionText)
				: NSLOCTEXT("InteractionComponent", "RecordAnomalyAction", "记录异常");
			break;

		case EFoundFootageInteractableObjective::ArchiveReview:
			Context.Verb = EHorrorInteractionVerb::Review;
			Context.bOpensPanel = true;
			Context.ActionText = Context.bCanInteract
				? (Context.ActionText.IsEmpty() ? NSLOCTEXT("InteractionComponent", "ReviewArchiveAction", "复查档案终端") : Context.ActionText)
				: NSLOCTEXT("InteractionComponent", "ReviewArchiveAction", "复查档案终端");
			break;

		case EFoundFootageInteractableObjective::ExitRouteGate:
			Context.Verb = EHorrorInteractionVerb::Open;
			Context.ActionText = Context.bCanInteract
				? (Context.ActionText.IsEmpty() ? NSLOCTEXT("InteractionComponent", "OpenExitGateAction", "打开出口闸门") : Context.ActionText)
				: NSLOCTEXT("InteractionComponent", "OpenExitGateAction", "打开出口闸门");
			break;
		}

		if (!Context.bCanInteract)
		{
			Context.BlockedReason = StripInteractionPromptPrefix(ObjectiveActor->GetInteractionPromptText(GetOwner()));
		}

		return Context;
	}

	if (const AHorrorCampaignObjectiveActor* CampaignObjectiveActor = Cast<AHorrorCampaignObjectiveActor>(TargetActor))
	{
		const FHorrorCampaignObjectiveDefinition& ObjectiveDefinition = CampaignObjectiveActor->GetObjectiveDefinitionForPresentation();
		const FHorrorCampaignObjectiveRuntimeState RuntimeState = CampaignObjectiveActor->BuildObjectiveRuntimeState();
		const bool bAdvancedCampaignObjective =
			RuntimeState.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring
			|| RuntimeState.InteractionMode == EHorrorCampaignInteractionMode::GearCalibration
			|| RuntimeState.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan
			|| RuntimeState.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning;

		Context.TargetId = RuntimeState.ObjectiveId;
		Context.RiskLevel = RuntimeState.RiskLevel != EHorrorCampaignObjectiveRiskLevel::None
			? ConvertCampaignRiskLevel(RuntimeState.RiskLevel)
			: (RuntimeState.bBeatUrgent
			? EHorrorInteractionRiskLevel::High
			: EHorrorInteractionRiskLevel::Medium);
		Context.bRequiresRecording = RuntimeState.bRequiresRecording;
		Context.ProgressFraction = RuntimeState.ProgressFraction;

		if (RuntimeState.bTimedObjectiveActive)
		{
			Context.Verb = EHorrorInteractionVerb::Survive;
			Context.InputStyle = EHorrorInteractionInputStyle::Timed;
			Context.InputText = DefaultInteractionInputText(Context.InputStyle);
			Context.DetailText = RuntimeState.PhaseText;
		}
		else if (bAdvancedCampaignObjective)
		{
			const FText WindowText = ResolveCampaignInteractionInputText(ObjectiveDefinition, RuntimeState);
			Context.Verb = RuntimeState.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan
				|| RuntimeState.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning
				? EHorrorInteractionVerb::Record
				: EHorrorInteractionVerb::Repair;
			Context.InputStyle = EHorrorInteractionInputStyle::Modal;
			Context.InputText = RuntimeState.Status == EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable || RuntimeState.bRetryable
				? BuildCampaignRetryableInputText(WindowText)
				: WindowText;
			Context.DetailText = RuntimeState.Status == EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable || RuntimeState.bRetryable
				? BuildCampaignRetryableDetailText(RuntimeState)
				: (!ObjectiveDefinition.Presentation.InputHint.IsEmpty() && !RuntimeState.bAdvancedInteractionActive
					? ObjectiveDefinition.Presentation.InputHint
					: (RuntimeState.bAdvancedInteractionActive
						? RuntimeState.PhaseText
						: BuildCampaignPanelEntryDetailText(WindowText)));
			Context.bOpensPanel = true;
		}
		else
		{
			Context.Verb = EHorrorInteractionVerb::Interact;
			Context.bOpensPanel = ObjectiveDefinition.Presentation.bOpensInteractionPanel;
			Context.DetailText = !ObjectiveDefinition.Presentation.InputHint.IsEmpty() && !RuntimeState.bAdvancedInteractionActive
				? ObjectiveDefinition.Presentation.InputHint
				: RuntimeState.PhaseText;
		}

		if (!Context.bCanInteract)
		{
			Context.BlockedReason = RuntimeState.BlockedReason.IsEmpty()
				? StripInteractionPromptPrefix(RuntimeState.PromptText)
				: StripInteractionPromptPrefix(RuntimeState.BlockedReason);
		}
		return Context;
	}

	if (const ABaseInteractable* BaseInteractable = Cast<ABaseInteractable>(TargetActor))
	{
		Context.TargetId = ResolveBaseInteractableId(BaseInteractable, TargetActor);
		if (!Context.bCanInteract)
		{
			Context.BlockedReason = Context.ActionText.IsEmpty()
				? NSLOCTEXT("InteractionComponent", "GenericBlockedReason", "现在无法互动。")
				: Context.ActionText;
		}
		return Context;
	}

	if (!Context.bCanInteract)
	{
		Context.BlockedReason = Context.ActionText.IsEmpty()
			? NSLOCTEXT("InteractionComponent", "GenericBlockedReason", "现在无法互动。")
			: Context.ActionText;
	}

	return Context;
}

void UInteractionComponent::ShowBlockedInteractionFeedback(UObject* TargetObject, const FHitResult& Hit) const
{
	const FHorrorInteractionContext Context = BuildInteractionContext(TargetObject, Hit);
	FString Message = !Context.BlockedReason.IsEmpty()
		? Context.BlockedReason.ToString()
		: StripInteractionPromptPrefix(BuildInteractionPrompt(TargetObject, Hit)).ToString();

	if (Message.IsEmpty())
	{
		Message = TEXT("现在无法互动。");
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	AHorrorPlayerController* PlayerController = OwnerPawn ? Cast<AHorrorPlayerController>(OwnerPawn->GetController()) : nullptr;
	if (!PlayerController)
	{
		PlayerController = Cast<AHorrorPlayerController>(GetOwner());
	}

	if (PlayerController)
	{
		PlayerController->ShowPlayerMessage(
			FText::FromString(Message),
			FLinearColor(1.0f, 0.72f, 0.22f),
			2.5f);
	}
}

bool UInteractionComponent::TryInvokeInteractableInterface(UObject* TargetObject, const FHitResult& Hit) const
{
	if (!TargetObject)
	{
		return false;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return false;
	}

	if (!TargetObject->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
	{
		return false;
	}

	IInteractableInterface* NativeInterface = Cast<IInteractableInterface>(TargetObject);
	if (!NativeInterface)
	{
		if (!IInteractableInterface::Execute_CanInteract(TargetObject, OwnerActor, Hit))
		{
			ShowBlockedInteractionFeedback(TargetObject, Hit);
			return false;
		}

		return IInteractableInterface::Execute_Interact(TargetObject, OwnerActor, Hit);
	}

	UFunction* CanInteractFunction = TargetObject->FindFunction(TEXT("CanInteract"));
	if (CanInteractFunction && CanInteractFunction->Script.Num() > 0)
	{
		if (!IInteractableInterface::Execute_CanInteract(TargetObject, OwnerActor, Hit))
		{
			ShowBlockedInteractionFeedback(TargetObject, Hit);
			return false;
		}
	}
	else if (!NativeInterface->CanInteract_Implementation(OwnerActor, Hit))
	{
		ShowBlockedInteractionFeedback(TargetObject, Hit);
		return false;
	}

	UFunction* InteractFunction = TargetObject->FindFunction(TEXT("Interact"));
	if (InteractFunction && InteractFunction->Script.Num() > 0)
	{
		return IInteractableInterface::Execute_Interact(TargetObject, OwnerActor, Hit);
	}

	return NativeInterface->Interact_Implementation(OwnerActor, Hit);
}

bool UInteractionComponent::HasLegacyInteractionFunction(AActor* TargetActor) const
{
	return FindValidatedLegacyInteractionFunction(TargetActor) != nullptr;
}

bool UInteractionComponent::TryInvokeLegacyInteractionFunction(AActor* TargetActor) const
{
	UFunction* Function = FindValidatedLegacyInteractionFunction(TargetActor);
	if (!Function)
	{
		return false;
	}

	TargetActor->ProcessEvent(Function, nullptr);
	return true;
}

bool UInteractionComponent::HasLegacyDoorTimeline(AActor* TargetActor) const
{
	if (!TargetActor)
	{
		return false;
	}

	TArray<UTimelineComponent*> Timelines;
	TargetActor->GetComponents(Timelines);

	for (UTimelineComponent* Timeline : Timelines)
	{
		if (Timeline && Timeline->GetName().Contains(TEXT("Door Control")))
		{
			return true;
		}
	}

	return false;
}

bool UInteractionComponent::TryPlayDoorTimeline(AActor* TargetActor) const
{
	TArray<UTimelineComponent*> Timelines;
	TargetActor->GetComponents(Timelines);

	for (UTimelineComponent* Timeline : Timelines)
	{
		if (Timeline && Timeline->GetName().Contains(TEXT("Door Control")))
		{
			Timeline->PlayFromStart();
			return true;
		}
	}

	return false;
}

float UInteractionComponent::CalculatePerpendicularDistanceToTraceForTests(const FVector& Start, const FVector& End, const FVector& Point) const
{
	const FVector Trace = End - Start;
	const float TraceLengthSquared = Trace.SizeSquared();
	if (TraceLengthSquared <= UE_KINDA_SMALL_NUMBER)
	{
		return FVector::Dist(Point, Start);
	}

	const float TraceAlpha = FMath::Clamp(FVector::DotProduct(Point - Start, Trace) / TraceLengthSquared, 0.0f, 1.0f);
	const FVector ClosestPoint = Start + (Trace * TraceAlpha);
	return FVector::Dist(Point, ClosestPoint);
}

bool UInteractionComponent::ResolveViewPoint(FVector& OutStart, FVector& OutDirection) const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return false;
	}

	if (const UCameraComponent* ViewCamera = OwnerActor->FindComponentByClass<UCameraComponent>())
	{
		OutStart = ViewCamera->GetComponentLocation();
		OutDirection = ViewCamera->GetForwardVector();
		return true;
	}

	if (const APawn* OwnerPawn = Cast<APawn>(OwnerActor))
	{
		FRotator ViewRotation = FRotator::ZeroRotator;
		OwnerPawn->GetActorEyesViewPoint(OutStart, ViewRotation);
		OutDirection = ViewRotation.Vector();
		return true;
	}

	OutStart = OwnerActor->GetActorLocation();
	OutDirection = OwnerActor->GetActorForwardVector();
	return true;
}
