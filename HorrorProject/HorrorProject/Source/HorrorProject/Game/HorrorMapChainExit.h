// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HorrorMapChainExit.generated.h"

class UBoxComponent;
class UPointLightComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Horror))
class HORRORPROJECT_API AHorrorMapChainExit : public AActor
{
	GENERATED_BODY()

public:
	AHorrorMapChainExit();

	void ConfigureForMapChain(const FString& InCurrentMapPackageName, const FString& InNextMapPackageName, bool bInFinalExit);

	UBoxComponent* GetTriggerBoundsForTests() const { return TriggerBounds.Get(); }
	FString GetCurrentMapPackageNameForTests() const { return CurrentMapPackageName; }
	FString GetNextMapPackageNameForTests() const { return NextMapPackageName; }
	bool IsFinalExitForTests() const { return bFinalExit; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror|Map Chain")
	TObjectPtr<UBoxComponent> TriggerBounds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror|Map Chain")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror|Map Chain")
	TObjectPtr<UTextRenderComponent> LabelText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror|Map Chain")
	TObjectPtr<UPointLightComponent> MarkerLight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Horror|Map Chain")
	FString CurrentMapPackageName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Horror|Map Chain")
	FString NextMapPackageName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Horror|Map Chain")
	bool bFinalExit = false;

private:
	UFUNCTION()
	void HandleTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void RefreshMarkerVisuals();
	bool CanUseExit(AActor* InstigatorActor) const;
	void ShowLockedFeedback(AActor* InstigatorActor) const;
	void TriggerMapTravel(AActor* InstigatorActor);
	void TriggerFinalEnding(AActor* InstigatorActor);

	UPROPERTY(Transient)
	bool bTransitionTriggered = false;
};
