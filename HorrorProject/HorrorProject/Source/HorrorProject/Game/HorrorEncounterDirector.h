// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HorrorEncounterDirector.generated.h"

class AHorrorThreatCharacter;
class USceneComponent;
class USoundBase;
class UCameraShakeBase;
class UHorrorEventBusSubsystem;

UENUM(BlueprintType)
enum class EHorrorEncounterPhase : uint8
{
	Dormant UMETA(DisplayName="Dormant"),
	Primed UMETA(DisplayName="Primed"),
	Revealed UMETA(DisplayName="Revealed"),
	Resolved UMETA(DisplayName="Resolved")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHorrorEncounterPhaseChangedSignature, EHorrorEncounterPhase, NewPhase, FName, EncounterId);

UCLASS(MinimalAPI, Hidden, NotBlueprintable, NotPlaceable)
class UHorrorEncounterPhaseDelegateProbe : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void HandleEncounterPhaseChanged(EHorrorEncounterPhase NewPhase, FName EncounterId);

	TArray<EHorrorEncounterPhase> PhaseValues;
	TArray<FName> EncounterIds;
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Horror))
class HORRORPROJECT_API AHorrorEncounterDirector : public AActor
{
	GENERATED_BODY()

public:
	AHorrorEncounterDirector();

	UFUNCTION(BlueprintCallable, Category="Horror|Encounter")
	bool PrimeEncounter(FName InEncounterId);

	UFUNCTION(BlueprintCallable, Category="Horror|Encounter")
	bool TriggerReveal(AActor* PlayerActor);

	UFUNCTION(BlueprintCallable, Category="Horror|Encounter")
	bool ResolveEncounter();

	UFUNCTION(BlueprintCallable, Category="Horror|Encounter")
	bool ResetEncounter();

	UFUNCTION(BlueprintPure, Category="Horror|Encounter")
	bool CanTriggerReveal(const AActor* PlayerActor) const;

	UFUNCTION(BlueprintPure, Category="Horror|Encounter")
	bool IsRouteGated() const;

	UFUNCTION(BlueprintPure, Category="Horror|Encounter")
	EHorrorEncounterPhase GetEncounterPhase() const;

	UFUNCTION(BlueprintPure, Category="Horror|Encounter")
	FName GetEncounterId() const;

	UFUNCTION(BlueprintPure, Category="Horror|Encounter")
	AActor* GetLastRevealTarget() const;

	UFUNCTION(BlueprintCallable, Category="Horror|Encounter")
	AHorrorThreatCharacter* SpawnThreatActor();

	UFUNCTION(BlueprintPure, Category="Horror|Encounter")
	AHorrorThreatCharacter* GetThreatActor() const;

	UFUNCTION(BlueprintCallable, Category="Horror|Encounter")
	bool ActivateGolemBehavior(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category="Horror|Encounter")
	bool DeactivateGolemBehavior();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter")
	FName DefaultEncounterId = TEXT("Encounter.GolemReveal01");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter")
	TSubclassOf<AHorrorThreatCharacter> ThreatClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter")
	FTransform ThreatRelativeTransform = FTransform(FRotator::ZeroRotator, FVector(100.0f, 0.0f, 0.0f));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter")
	TObjectPtr<AHorrorThreatCharacter> ThreatActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter", meta=(ClampMin="0.0", Units="cm"))
	float RevealRadius = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter")
	bool bGateRouteDuringReveal = true;

	UPROPERTY(BlueprintAssignable, Category="Horror|Encounter")
	FHorrorEncounterPhaseChangedSignature OnEncounterPhaseChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter|Audio")
	TObjectPtr<USoundBase> PrimeSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter|Audio")
	TObjectPtr<USoundBase> RevealSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter|Audio")
	TObjectPtr<USoundBase> ResolveSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter|Camera")
	TSubclassOf<UCameraShakeBase> RevealCameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter|Camera", meta=(ClampMin="0.0", ClampMax="1.0"))
	float RevealCameraShakeScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter|Timing")
	float RevealDelaySeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter|EventBus")
	bool bPublishToEventBus = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter|EventBus")
	FName EventBusSourceId = TEXT("EncounterDirector");

	UFUNCTION(BlueprintCallable, Category="Horror|Encounter")
	void PlayEncounterSound(USoundBase* Sound, float VolumeMultiplier = 1.0f);

	UFUNCTION(BlueprintCallable, Category="Horror|Encounter")
	void TriggerCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale = 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter|Audio")
	bool bUseAudioSubsystem = true;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Encounter")
	void BP_OnEncounterPrimed(FName PrimedEncounterId);

	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Encounter")
	void BP_OnEncounterRevealed(FName RevealedEncounterId, AActor* PlayerActor, AHorrorThreatCharacter* RevealedThreat);

	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Encounter")
	void BP_OnEncounterResolved(FName ResolvedEncounterId);

	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Encounter")
	void BP_OnEncounterReset(FName ResetEncounterId);

	UFUNCTION(BlueprintNativeEvent, Category="Horror|Encounter")
	void BP_OnRevealSequenceStart(AActor* PlayerActor);

	UFUNCTION(BlueprintNativeEvent, Category="Horror|Encounter")
	void BP_OnRevealSequenceComplete(AActor* PlayerActor, AHorrorThreatCharacter* RevealedThreat);

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Horror|Encounter", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Horror|Encounter", meta=(AllowPrivateAccess="true"))
	EHorrorEncounterPhase EncounterPhase = EHorrorEncounterPhase::Dormant;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Horror|Encounter", meta=(AllowPrivateAccess="true"))
	FName ActiveEncounterId = NAME_None;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="Horror|Encounter", meta=(AllowPrivateAccess="true"))
	TObjectPtr<AActor> LastRevealTarget;

	FTimerHandle RevealDelayTimerHandle;

	void ExecuteDelayedReveal(AActor* PlayerActor);
	void PublishEncounterEvent(FName EventName, FName PhaseTag);
};
