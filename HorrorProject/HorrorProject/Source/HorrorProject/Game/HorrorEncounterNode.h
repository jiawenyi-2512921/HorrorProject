// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/HorrorObjectiveNode.h"
#include "HorrorEncounterNode.generated.h"

class AHorrorEncounterDirector;
class AHorrorThreatCharacter;

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Horror))
class HORRORPROJECT_API AHorrorEncounterNode : public AHorrorObjectiveNode
{
	GENERATED_BODY()

public:
	AHorrorEncounterNode();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Horror|Encounter")
	bool TriggerEncounter(AActor* InstigatorActor);

	UFUNCTION(BlueprintCallable, Category="Horror|Encounter")
	bool ResolveEncounter();

	UFUNCTION(BlueprintPure, Category="Horror|Encounter")
	bool IsEncounterActive() const { return bEncounterActive; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter")
	TSubclassOf<AHorrorEncounterDirector> EncounterDirectorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter")
	TSubclassOf<AHorrorThreatCharacter> ThreatClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter")
	FTransform ThreatSpawnTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter")
	bool bAutoTriggerOnActivate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter")
	bool bAutoCompleteOnResolve = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter")
	float TriggerRadius = 1000.0f;

protected:
	virtual void OnNodeActivated_Implementation() override;

	UFUNCTION(BlueprintNativeEvent, Category="Horror|Encounter")
	void OnEncounterTriggered(AActor* InstigatorActor);

	UFUNCTION(BlueprintNativeEvent, Category="Horror|Encounter")
	void OnEncounterResolved();

	virtual void OnEncounterTriggered_Implementation(AActor* InstigatorActor);
	virtual void OnEncounterResolved_Implementation();

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(Transient)
	TObjectPtr<AHorrorEncounterDirector> SpawnedEncounterDirector;

	UPROPERTY(Transient)
	TObjectPtr<AHorrorThreatCharacter> SpawnedThreat;

	bool bEncounterActive = false;
};
