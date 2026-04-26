// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Templates/Function.h"
#include "InteractionComponent.generated.h"

class AActor;
class UObject;

UCLASS(ClassGroup=(Horror), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

	UFUNCTION(BlueprintCallable, Category="Interaction")
	bool TryInteract();

	bool TryInteractWithHit(const FHitResult& Hit) const;
	void SetLegacyDoorTimelineFallbackEnabledForTests(bool bEnabled) { bEnableLegacyDoorTimelineFallback = bEnabled; }
	UObject* ResolveInterfaceTargetForTests(const FHitResult& Hit) const { return ResolveInterfaceTarget(Hit); }
	float CalculatePerpendicularDistanceToTraceForTests(const FVector& Start, const FVector& End, const FVector& Point) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction", meta=(ClampMin="1.0", Units="cm"))
	float TraceDistance = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction", meta=(ClampMin="0.0", Units="cm"))
	float NarrowTargetTraceRadius = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	bool bTraceComplex = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	bool bEnableNarrowTargetSweepFallback = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction|Compatibility")
	bool bEnableLegacyFunctionFallback = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction|Compatibility")
	bool bEnableLegacyDoorTimelineFallback = false;

private:
	bool FindInteractionHit(FHitResult& OutHit) const;
	bool FindInteractionHitBySweep(const FVector& Start, const FVector& End, const FCollisionQueryParams& QueryParams, FHitResult& OutHit) const;
	bool IsInteractionCandidate(const FHitResult& Hit) const;
	UObject* ResolveInterfaceTarget(const FHitResult& Hit) const;
	bool TryInvokeInteractableInterface(UObject* TargetObject, const FHitResult& Hit) const;
	bool HasLegacyInteractionFunction(AActor* TargetActor) const;
	bool TryInvokeLegacyInteractionFunction(AActor* TargetActor) const;
	bool HasLegacyDoorTimeline(AActor* TargetActor) const;
	bool TryPlayDoorTimeline(AActor* TargetActor) const;
	bool ResolveViewPoint(FVector& OutStart, FVector& OutDirection) const;
};

