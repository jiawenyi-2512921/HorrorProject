// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractableInterface.h"
#include "Evidence/EvidenceTypes.h"
#include "EvidenceActor.generated.h"

/**
 * Implements actor-level Evidence Actor behavior for the Evidence module.
 */
UCLASS(BlueprintType, Blueprintable)
class HORRORPROJECT_API AEvidenceActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AEvidenceActor();

	virtual void BeginPlay() override;

	// IInteractableInterface
	virtual bool Interact_Implementation(AActor* InstigatorActor, const FHitResult& Hit) override;
	virtual bool CanInteract_Implementation(AActor* InstigatorActor, const FHitResult& Hit) const override;

	UFUNCTION(BlueprintPure, Category="Evidence")
	FText GetInteractionPrompt() const;

	UFUNCTION(BlueprintCallable, Category="Evidence")
	void SetEvidenceMetadata(const FExtendedEvidenceMetadata& Metadata);

	UFUNCTION(BlueprintPure, Category="Evidence")
	const FExtendedEvidenceMetadata& GetEvidenceMetadata() const { return EvidenceMetadata; }

	UFUNCTION(BlueprintCallable, Category="Evidence")
	void SetCollected(bool bCollected);

	UFUNCTION(BlueprintPure, Category="Evidence")
	bool IsCollected() const { return bIsCollected; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Evidence")
	FExtendedEvidenceMetadata EvidenceMetadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Evidence")
	FText InteractionPromptOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Evidence")
	bool bAutoRegisterMetadata = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Evidence")
	bool bDestroyOnCollect = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Evidence")
	bool bHideOnCollect = true;

	UPROPERTY(BlueprintReadOnly, Category="Evidence")
	bool bIsCollected = false;

	UFUNCTION(BlueprintImplementableEvent, Category="Evidence", meta=(DisplayName="证据已收集"))
	void BP_OnEvidenceCollected(AActor* Collector);

	UFUNCTION(BlueprintNativeEvent, Category="Evidence")
	void OnEvidenceCollected(AActor* Collector);
	virtual void OnEvidenceCollected_Implementation(AActor* Collector);
};
