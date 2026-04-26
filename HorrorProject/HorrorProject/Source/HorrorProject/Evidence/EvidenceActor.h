// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractableInterface.h"
#include "Evidence/EvidenceTypes.h"
#include "EvidenceActor.generated.h"

UCLASS(BlueprintType, Blueprintable)
class HORRORPROJECT_API AEvidenceActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AEvidenceActor();

	virtual void BeginPlay() override;

	// IInteractableInterface
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;

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

	UFUNCTION(BlueprintImplementableEvent, Category="Evidence", meta=(DisplayName="On Evidence Collected"))
	void BP_OnEvidenceCollected(AActor* Collector);

	UFUNCTION(BlueprintNativeEvent, Category="Evidence")
	void OnEvidenceCollected(AActor* Collector);
	virtual void OnEvidenceCollected_Implementation(AActor* Collector);
};
