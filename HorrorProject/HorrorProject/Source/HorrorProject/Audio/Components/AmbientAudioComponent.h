// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "AmbientAudioComponent.generated.h"

UENUM(BlueprintType)
enum class EAmbientAudioType : uint8
{
	Static UMETA(DisplayName="Static"),
	Dynamic UMETA(DisplayName="Dynamic"),
	Layered UMETA(DisplayName="Layered"),
	Randomized UMETA(DisplayName="Randomized")
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FAmbientAudioLayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient Audio")
	TObjectPtr<USoundBase> Sound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient Audio", meta=(ClampMin="0.0", ClampMax="1.0"))
	float BaseVolume = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient Audio")
	bool bRandomizeVolume = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient Audio", meta=(ClampMin="0.0", ClampMax="1.0"))
	float VolumeVariation = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient Audio")
	float FadeInTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient Audio")
	float FadeOutTime = 2.0f;
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UAmbientAudioComponent : public UAudioComponent
{
	GENERATED_BODY()

public:
	UAmbientAudioComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Ambient Audio")
	void StartAmbient();

	UFUNCTION(BlueprintCallable, Category="Ambient Audio")
	void StopAmbient(float FadeOutDuration = 2.0f);

	UFUNCTION(BlueprintCallable, Category="Ambient Audio")
	void AddLayer(const FAmbientAudioLayer& Layer);

	UFUNCTION(BlueprintCallable, Category="Ambient Audio")
	void RemoveLayer(int32 LayerIndex);

	UFUNCTION(BlueprintCallable, Category="Ambient Audio")
	void SetLayerVolume(int32 LayerIndex, float Volume, float FadeTime = 1.0f);

	UFUNCTION(BlueprintCallable, Category="Ambient Audio")
	void SetTimeOfDayIntensity(float Intensity);

	UFUNCTION(BlueprintPure, Category="Ambient Audio")
	bool IsAmbientPlaying() const { return bIsAmbientPlaying; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient Audio")
	EAmbientAudioType AmbientType = EAmbientAudioType::Static;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient Audio")
	TArray<FAmbientAudioLayer> AudioLayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient Audio")
	bool bAutoStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient Audio")
	bool bUseTimeOfDay = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient Audio", meta=(ClampMin="0.0", ClampMax="24.0"))
	float PeakHour = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient Audio")
	float RandomizationInterval = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient Audio")
	float MaxDistance = 5000.0f;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UAudioComponent>> LayerComponents;

	bool bIsAmbientPlaying = false;
	float TimeSinceLastRandomization = 0.0f;
	float CurrentTimeOfDayIntensity = 1.0f;

	void UpdateLayeredAudio(float DeltaTime);
	void UpdateRandomizedAudio(float DeltaTime);
	void UpdateDynamicAudio(float DeltaTime);
	float CalculateTimeOfDayMultiplier() const;
};
