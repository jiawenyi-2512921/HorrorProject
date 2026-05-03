// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interaction/HorrorInteractionTypes.h"
#include "Templates/Function.h"
#include "InteractionComponent.generated.h"

class AActor;
class UObject;

/**
 * Component that handles player interaction with world objects
 * Supports both modern IInteractable interface and legacy interaction methods
 * Uses raycasting with optional sphere sweep fallback for narrow targets
 */
UCLASS(ClassGroup=(Horror), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	static constexpr float DefaultTraceDistance = 450.0f;
	static constexpr float DefaultNarrowTargetTraceRadius = 12.0f;

	UInteractionComponent();

	/**
	 * Attempts to interact with an object in front of the player
	 * Performs raycast from camera/viewpoint and invokes interaction if valid target found
	 * @return True if interaction was successful
	 */
	UFUNCTION(BlueprintCallable, Category="Interaction")
	bool TryInteract();

	UFUNCTION(BlueprintCallable, Category="Interaction")
	bool GetFocusedInteractionPrompt(FText& OutPrompt) const;

	UFUNCTION(BlueprintCallable, Category="Interaction")
	bool GetFocusedInteractionContext(FHorrorInteractionContext& OutContext) const;

	bool FindFocusedInteractable(FHitResult& OutHit, UObject*& OutTargetObject) const;
	bool FindFocusedInteractionTarget(FHitResult& OutHit, UObject*& OutTargetObject) const;
	bool TryInteractWithHit(const FHitResult& Hit) const;
	void SetLegacyDoorTimelineFallbackEnabledForTests(bool bEnabled) { bEnableLegacyDoorTimelineFallback = bEnabled; }
	UObject* ResolveInterfaceTargetForTests(const FHitResult& Hit) const { return ResolveInterfaceTarget(Hit); }
	FHorrorInteractionContext BuildInteractionContextForTests(UObject* TargetObject, const FHitResult& Hit) const { return BuildInteractionContext(TargetObject, Hit); }
	float CalculatePerpendicularDistanceToTraceForTests(const FVector& Start, const FVector& End, const FVector& Point) const;

protected:
	/** Maximum distance for interaction raycasts (in cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction", meta=(ClampMin="1.0", Units="cm"))
	float TraceDistance = DefaultTraceDistance;

	/** Radius for sphere sweep fallback when line trace misses narrow targets (in cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction", meta=(ClampMin="0.0", Units="cm"))
	float NarrowTargetTraceRadius = DefaultNarrowTargetTraceRadius;

	/** Collision channel used for interaction traces */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	/** Use complex collision for more accurate interaction detection */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	bool bTraceComplex = true;

	/** Enable sphere sweep fallback for small/narrow interactable objects */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	bool bEnableNarrowTargetSweepFallback = true;

	/** Support legacy blueprint interaction functions (deprecated) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction|Compatibility")
	bool bEnableLegacyFunctionFallback = false;

	/** Support legacy door timeline system (deprecated) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction|Compatibility")
	bool bEnableLegacyDoorTimelineFallback = false;

private:
	bool FindInteractionHit(FHitResult& OutHit) const;
	bool FindInteractionHitBySweep(const FVector& Start, const FVector& End, const FCollisionQueryParams& QueryParams, FHitResult& OutHit) const;
	bool IsInteractionCandidate(const FHitResult& Hit) const;
	UObject* ResolveInterfaceTarget(const FHitResult& Hit) const;
	bool CanInvokeInteractableInterface(UObject* TargetObject, const FHitResult& Hit) const;
	FText BuildInteractionPrompt(UObject* TargetObject, const FHitResult& Hit) const;
	FHorrorInteractionContext BuildInteractionContext(UObject* TargetObject, const FHitResult& Hit) const;
	void ShowBlockedInteractionFeedback(UObject* TargetObject, const FHitResult& Hit) const;
	bool TryInvokeInteractableInterface(UObject* TargetObject, const FHitResult& Hit) const;
	bool HasLegacyInteractionFunction(AActor* TargetActor) const;
	bool TryInvokeLegacyInteractionFunction(AActor* TargetActor) const;
	bool HasLegacyDoorTimeline(AActor* TargetActor) const;
	bool TryPlayDoorTimeline(AActor* TargetActor) const;
	bool ResolveViewPoint(FVector& OutStart, FVector& OutDirection) const;
};
