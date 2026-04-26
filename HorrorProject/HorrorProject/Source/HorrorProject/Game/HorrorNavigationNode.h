// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/HorrorObjectiveNode.h"
#include "HorrorNavigationNode.generated.h"

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Horror))
class HORRORPROJECT_API AHorrorNavigationNode : public AHorrorObjectiveNode
{
	GENERATED_BODY()

public:
	AHorrorNavigationNode();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Navigation")
	bool bAutoCompleteOnOverlap = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Navigation")
	float ActivationRadius = 500.0f;

protected:
	virtual void OnNodeActivated_Implementation() override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
