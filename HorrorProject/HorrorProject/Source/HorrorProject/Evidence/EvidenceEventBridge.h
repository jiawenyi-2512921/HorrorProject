// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Evidence/EvidenceTypes.h"
#include "EvidenceEventBridge.generated.h"

class UEvidenceCollectionComponent;
class UArchiveSubsystem;
class UHorrorEventBusSubsystem;

/**
 * Defines Evidence Event Bridge behavior for the Evidence module.
 */
UCLASS(ClassGroup=(Horror), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UEvidenceEventBridge : public UActorComponent
{
	GENERATED_BODY()

public:
	UEvidenceEventBridge();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category="Evidence|Events")
	void BindToEvidenceCollection(UEvidenceCollectionComponent* CollectionComponent);

	UFUNCTION(BlueprintCallable, Category="Evidence|Events")
	void UnbindFromEvidenceCollection();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Evidence|Events")
	bool bAutoBindOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Evidence|Events")
	bool bPublishToEventBus = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Evidence|Events")
	bool bAddToArchive = true;

protected:
	UFUNCTION()
	void OnEvidenceCaptured(FName EvidenceId, const FEvidenceCaptureData& CaptureData);

	UFUNCTION()
	void OnPhotoTaken(FName PhotoId, UTexture2D* Photo, FVector Location);

	UFUNCTION()
	void OnVideoRecorded(FName VideoId, float Duration, FVector Location);

private:
	void PublishEvidenceEvent(FName EvidenceId, EEvidenceType Type);

	UPROPERTY()
	TWeakObjectPtr<UEvidenceCollectionComponent> BoundCollectionComponent;

	UPROPERTY()
	TWeakObjectPtr<UArchiveSubsystem> CachedArchiveSubsystem;

	UPROPERTY()
	TWeakObjectPtr<UHorrorEventBusSubsystem> CachedEventBus;
};
