// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HorrorAudioZoneActor.generated.h"

class UHorrorAudioZoneTrigger;
class USoundBase;

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Horror))
class HORRORPROJECT_API AHorrorAudioZoneActor : public AActor
{
	GENERATED_BODY()

public:
	AHorrorAudioZoneActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Audio")
	FName ZoneId = TEXT("Zone.Default");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Audio")
	TObjectPtr<USoundBase> AmbientSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Audio", meta=(ClampMin="0.0", ClampMax="1.0"))
	float AmbientVolume = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Audio")
	bool bLoopAmbient = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Audio", meta=(ClampMin="0.0"))
	float FadeInDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Audio", meta=(ClampMin="0.0"))
	float FadeOutDuration = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Audio")
	bool bAutoRegister = true;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror|Audio", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UHorrorAudioZoneTrigger> ZoneTrigger;

	void RegisterZoneConfig();
	void UnregisterZoneConfig();
};
