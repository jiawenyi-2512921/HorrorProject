// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/InteractionComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/TimelineComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "HorrorProject.h"
#include "Interaction/InteractableInterface.h"

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

/**
 * Attempts to interact with a specific hit result
 * Resolves the appropriate interaction method (interface, legacy function, or door timeline)
 * @param Hit - The raycast hit result containing the target object
 * @return True if interaction was successfully invoked
 */
bool UInteractionComponent::TryInteractWithHit(const FHitResult& Hit) const
{
	AActor* TargetActor = Hit.GetActor();
	if (UObject* InterfaceTarget = ResolveInterfaceTarget(Hit))
	{
		return TryInvokeInteractableInterface(InterfaceTarget, Hit);
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

	const FVector End = Start + (ViewDirection * TraceDistance);

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
			return false;
		}

		return IInteractableInterface::Execute_Interact(TargetObject, OwnerActor, Hit);
	}

	UFunction* CanInteractFunction = TargetObject->FindFunction(TEXT("CanInteract"));
	if (CanInteractFunction && CanInteractFunction->Script.Num() > 0)
	{
		if (!IInteractableInterface::Execute_CanInteract(TargetObject, OwnerActor, Hit))
		{
			return false;
		}
	}
	else if (!NativeInterface->CanInteract_Implementation(OwnerActor, Hit))
	{
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
