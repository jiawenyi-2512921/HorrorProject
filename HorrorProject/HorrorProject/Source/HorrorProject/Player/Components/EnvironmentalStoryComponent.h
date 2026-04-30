// Copyright HorrorProject. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "EnvironmentalStoryComponent.generated.h"

class USoundBase;
class UMaterialInstanceDynamic;
class AActor;

UENUM(BlueprintType)
enum class EEnvironmentalStoryType : uint8
{
	LightFlicker UMETA(DisplayName="灯光闪烁"),
	ObjectMovement UMETA(DisplayName="物体移动"),
	TemperatureDrop UMETA(DisplayName="温度骤降"),
	WaterLeak UMETA(DisplayName="漏水"),
	ElectricalSurge UMETA(DisplayName="电涌"),
	DistantSound UMETA(DisplayName="远处声响"),
	ShadowPlay UMETA(DisplayName="影子异动"),
	WritingReveal UMETA(DisplayName="字迹显现")
};

USTRUCT(BlueprintType)
struct FEnvironmentalStoryEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Story")
	FName EventId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Story")
	EEnvironmentalStoryType Type = EEnvironmentalStoryType::LightFlicker;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Story")
	FGameplayTag TriggerTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Story")
	TObjectPtr<USoundBase> AmbientSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Story")
	TObjectPtr<USoundBase> TriggerSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Story", meta=(ClampMin="0.0", Units="s"))
	float Duration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Story", meta=(ClampMin="0.0", ClampMax="1.0"))
	float Intensity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Story")
	bool bRepeatable = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Story")
	bool bHasTriggered = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEnvironmentalStoryEventSignature, FName, EventId, EEnvironmentalStoryType, Type);

UCLASS(ClassGroup=(Horror), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UEnvironmentalStoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnvironmentalStoryComponent();

	UFUNCTION(BlueprintCallable, Category="Story|Environmental")
	void TriggerStoryEvent(FName EventId);

	UFUNCTION(BlueprintCallable, Category="Story|Environmental")
	void TriggerByTag(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category="Story|Environmental")
	void StopAllEvents();

	UFUNCTION(BlueprintPure, Category="Story|Environmental")
	bool IsEventActive(FName EventId) const;

	UFUNCTION(BlueprintPure, Category="Story|Environmental")
	int32 GetActiveEventCount() const;

	UPROPERTY(BlueprintAssignable, Category="Story|Environmental")
	FEnvironmentalStoryEventSignature OnStoryEventTriggered;

	UPROPERTY(BlueprintAssignable, Category="Story|Environmental")
	FEnvironmentalStoryEventSignature OnStoryEventCompleted;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Story|Environmental")
	TArray<FEnvironmentalStoryEvent> StoryEvents;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Story|Environmental")
	bool bAutoTriggerOnTag = true;

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void UpdateActiveEvents(float DeltaTime);
	void ApplyEventEffects(const FEnvironmentalStoryEvent& Event);
	void RemoveEventEffects(const FEnvironmentalStoryEvent& Event);

	struct FActiveEvent
	{
		int32 EventIndex;
		float RemainingTime;
	};

	TArray<FActiveEvent> ActiveEvents;
};
