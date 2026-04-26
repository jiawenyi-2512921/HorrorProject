// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HorrorAudioZoneTrigger.generated.h"

class UBoxComponent;
class USphereComponent;
class UShapeComponent;

UENUM(BlueprintType)
enum class EHorrorAudioZoneTriggerShape : uint8
{
	Box UMETA(DisplayName="Box"),
	Sphere UMETA(DisplayName="Sphere")
};

UCLASS(ClassGroup=(Horror), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UHorrorAudioZoneTrigger : public UActorComponent
{
	GENERATED_BODY()

public:
	UHorrorAudioZoneTrigger();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Audio")
	FName ZoneId = TEXT("Zone.Default");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Audio")
	EHorrorAudioZoneTriggerShape TriggerShape = EHorrorAudioZoneTriggerShape::Box;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Audio", meta=(EditCondition="TriggerShape == EHorrorAudioZoneTriggerShape::Box", EditConditionHides))
	FVector BoxExtent = FVector(500.0f, 500.0f, 200.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Audio", meta=(EditCondition="TriggerShape == EHorrorAudioZoneTriggerShape::Sphere", EditConditionHides, ClampMin="0.0"))
	float SphereRadius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Audio")
	bool bAutoActivateTrigger = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Audio")
	bool bTriggerOnce = false;

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	void ActivateTrigger();

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	void DeactivateTrigger();

	UFUNCTION(BlueprintPure, Category="Horror|Audio")
	bool IsTriggerActive() const { return bTriggerActive; }

protected:
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	UPROPERTY(Transient)
	TObjectPtr<UShapeComponent> TriggerComponent;

	UPROPERTY(Transient)
	bool bTriggerActive = false;

	UPROPERTY(Transient)
	bool bHasTriggered = false;

	void CreateTriggerComponent();
	void BindTriggerEvents();
	void UnbindTriggerEvents();
	bool IsPlayerActor(AActor* Actor) const;
};
