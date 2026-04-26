// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AutoSaveComponent.generated.h"

class UHorrorSaveSubsystem;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UAutoSaveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAutoSaveComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category="AutoSave")
	void EnableAutoSave(bool bEnable);

	UFUNCTION(BlueprintCallable, Category="AutoSave")
	void SetAutoSaveInterval(float Seconds);

	UFUNCTION(BlueprintCallable, Category="AutoSave")
	void TriggerAutoSave();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AutoSave")
	bool bAutoSaveEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AutoSave", meta=(ClampMin="30.0", ClampMax="600.0"))
	float AutoSaveIntervalSeconds = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AutoSave")
	int32 AutoSaveSlotIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AutoSave")
	bool bShowAutoSaveNotification = true;

private:
	void ScheduleNextAutoSave();
	void PerformAutoSave();

	FTimerHandle AutoSaveTimerHandle;

	UPROPERTY(Transient)
	TObjectPtr<UHorrorSaveSubsystem> SaveSubsystem;
};
