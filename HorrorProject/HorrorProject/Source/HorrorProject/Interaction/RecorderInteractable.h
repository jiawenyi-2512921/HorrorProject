// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/BaseInteractable.h"
#include "Player/Components/InventoryComponent.h"
#include "RecorderInteractable.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class USoundBase;
class UAudioComponent;

UENUM(BlueprintType)
enum class ERecorderState : uint8
{
	Idle UMETA(DisplayName="空闲"),
	Playing UMETA(DisplayName="播放中"),
	Paused UMETA(DisplayName="已暂停"),
	Stopped UMETA(DisplayName="已停止")
};

/**
 * Recorder interactable that plays audio logs and integrates with evidence system.
 * Supports playback control and evidence collection.
 */
UCLASS(BlueprintType, Blueprintable)
class HORRORPROJECT_API ARecorderInteractable : public ABaseInteractable
{
	GENERATED_BODY()

public:
	ARecorderInteractable();

	virtual void Tick(float DeltaTime) override;
	virtual bool CanInteract_Implementation(AActor* InstigatorActor, const FHitResult& Hit) const override;
	virtual void SaveState(TMap<FName, bool>& OutStateMap) const override;
	virtual void LoadState(const TMap<FName, bool>& InStateMap) override;

	UFUNCTION(BlueprintCallable, Category="Recorder")
	void PlayAudioLog();

	UFUNCTION(BlueprintCallable, Category="Recorder")
	void StopAudioLog();

	UFUNCTION(BlueprintCallable, Category="Recorder")
	void PauseAudioLog();

	UFUNCTION(BlueprintPure, Category="Recorder")
	bool IsPlaying() const { return RecorderState == ERecorderState::Playing; }

	UFUNCTION(BlueprintPure, Category="Recorder")
	float GetPlaybackProgress() const;

	UFUNCTION(BlueprintPure, Category="Recorder")
	FText GetAudioLogTitle() const { return AudioLogTitle; }

protected:
	virtual void OnInteract(AActor* InstigatorActor, const FHitResult& Hit) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool TryAddToEvidence(AActor* InstigatorActor);
	void OnAudioFinished();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Recorder", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> RecorderMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Recorder", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> InteractionVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Recorder", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAudioComponent> AudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Recorder|Audio", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USoundBase> AudioLog = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Recorder|Audio", meta=(AllowPrivateAccess="true"))
	FText AudioLogTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Recorder|Audio", meta=(AllowPrivateAccess="true", MultiLine=true))
	FText AudioLogTranscript;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Recorder|Audio", meta=(AllowPrivateAccess="true"))
	bool bUse3DAudio = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Recorder|Audio", meta=(AllowPrivateAccess="true"))
	float MaxAudibleDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Recorder|Evidence", meta=(AllowPrivateAccess="true"))
	bool bIsEvidence = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Recorder|Evidence", meta=(AllowPrivateAccess="true"))
	FHorrorEvidenceMetadata EvidenceMetadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Recorder|Behavior", meta=(AllowPrivateAccess="true"))
	bool bAutoPlayOnInteract = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Recorder|Behavior", meta=(AllowPrivateAccess="true"))
	bool bAutoCollectAsEvidence = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Recorder|Behavior", meta=(AllowPrivateAccess="true"))
	bool bStopOnReInteract = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Recorder|Behavior", meta=(AllowPrivateAccess="true"))
	bool bLoopAudio = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Recorder|State", meta=(AllowPrivateAccess="true"))
	ERecorderState RecorderState = ERecorderState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Recorder|State", meta=(AllowPrivateAccess="true"))
	bool bHasBeenListened = false;

	FTimerHandle AudioFinishedTimer;
};
