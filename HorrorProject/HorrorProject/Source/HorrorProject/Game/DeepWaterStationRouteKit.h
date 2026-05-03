// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Set.h"
#include "Game/FoundFootageObjectiveInteractable.h"
#include "GameFramework/Actor.h"
#include "DeepWaterStationRouteKit.generated.h"

class AHorrorEncounterDirector;
class AHorrorThreatCharacter;
class USceneComponent;

namespace HorrorRouteKitDefaults
{
	inline constexpr float ObjectiveHeightCm = 80.0f;
	inline constexpr float EncounterDirectorXOffsetCm = 1000.0f;
	inline constexpr float EncounterDirectorYOffsetCm = 300.0f;
	inline constexpr float EncounterRevealRadiusCm = 1200.0f;
	inline constexpr float EncounterThreatXOffsetCm = 100.0f;
}

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FDeepWaterStationObjectiveNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route")
	EFoundFootageInteractableObjective Objective = EFoundFootageInteractableObjective::Bodycam;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route")
	FName SourceId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route")
	FTransform RelativeTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route")
	bool bEnableBodycamOnInteract = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route")
	bool bIsRecordingForFirstAnomalyRecord = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route")
	FHorrorEvidenceMetadata EvidenceMetadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route")
	FHorrorNoteMetadata NoteMetadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route")
	FName TrailerBeatId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route")
	FText ObjectiveHint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route")
	FText DebugLabel;
};

/**
 * Implements actor-level Deep Water Station Route Kit behavior for the Game module.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Horror))
class HORRORPROJECT_API ADeepWaterStationRouteKit : public AActor
{
	GENERATED_BODY()

public:
	ADeepWaterStationRouteKit();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Horror|Route")
	void ConfigureDefaultFirstLoopObjectiveNodes();

	UFUNCTION(BlueprintCallable, Category="Horror|Route")
	bool EnsureDefaultFirstLoopObjectiveNodes();

	UFUNCTION(BlueprintCallable, Category="Horror|Route")
	int32 SpawnObjectiveNodes();

	const TArray<AFoundFootageObjectiveInteractable*>& GetSpawnedObjectiveInteractables() const { return SpawnedObjectiveInteractableViews; }

	const TArray<AFoundFootageObjectiveInteractable*>& GetSpawnedObjectiveInteractablesForTests() const { return SpawnedObjectiveInteractableViews; }

	UFUNCTION(BlueprintPure, Category="Horror|Route")
	bool ValidateObjectiveNodes(TArray<FText>& ValidationErrors) const;

	UFUNCTION(BlueprintCallable, Category="Horror|Route")
	AHorrorEncounterDirector* SpawnEncounterDirector();

	UFUNCTION(BlueprintPure, Category="Horror|Route")
	AHorrorEncounterDirector* GetSpawnedEncounterDirector() const;

	UFUNCTION(BlueprintCallable, Category="Horror|Route")
	bool TriggerEncounterReveal(AActor* PlayerActor);

	UFUNCTION(BlueprintPure, Category="Horror|Route")
	bool CanTriggerEncounterReveal(AActor* PlayerActor) const;

	UFUNCTION(BlueprintCallable, Category="Horror|Route")
	bool ResolveEncounter();

	UFUNCTION(BlueprintCallable, Category="Horror|Route")
	bool HandleObjectiveCompleted(EFoundFootageInteractableObjective Objective, AActor* InstigatorActor);

	UFUNCTION(BlueprintPure, Category="Horror|Route")
	bool IsRouteGatedByEncounter() const;

	UFUNCTION(BlueprintPure, Category="Horror|Route")
	bool CanResolveEncounterAtExit() const;

	UFUNCTION(BlueprintPure, Category="Horror|Route")
	FName GetTrailerBeatIdForSourceId(FName SourceId) const;

	UFUNCTION(BlueprintPure, Category="Horror|Route")
	TArray<FName> GetTrailerBeatIds() const;

	UFUNCTION(BlueprintPure, Category="Horror|Route")
	bool TryGetObjectiveWorldLocation(EFoundFootageInteractableObjective Objective, FVector& OutWorldLocation) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route")
	TSubclassOf<AHorrorEncounterDirector> EncounterDirectorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route")
	FTransform EncounterDirectorRelativeTransform = FTransform(
		FRotator::ZeroRotator,
		FVector(
			HorrorRouteKitDefaults::EncounterDirectorXOffsetCm,
			HorrorRouteKitDefaults::EncounterDirectorYOffsetCm,
			HorrorRouteKitDefaults::ObjectiveHeightCm));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route")
	FName EncounterId = TEXT("Encounter.GolemReveal01");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route", meta=(ClampMin="0.0", Units="cm"))
	float EncounterRevealRadius = HorrorRouteKitDefaults::EncounterRevealRadiusCm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route")
	TSubclassOf<AHorrorThreatCharacter> EncounterThreatClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route")
	FTransform EncounterThreatRelativeTransform = FTransform(
		FRotator::ZeroRotator,
		FVector(HorrorRouteKitDefaults::EncounterThreatXOffsetCm, 0.0f, 0.0f));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route")
	bool bAutoPrimeEncounter = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route")
	TSubclassOf<AFoundFootageObjectiveInteractable> ObjectiveInteractableClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route")
	TArray<FDeepWaterStationObjectiveNode> ObjectiveNodes;

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Horror|Route", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AFoundFootageObjectiveInteractable>> SpawnedObjectiveInteractables;

	UPROPERTY(Transient)
	TObjectPtr<AHorrorEncounterDirector> SpawnedEncounterDirector;

	TArray<AFoundFootageObjectiveInteractable*> SpawnedObjectiveInteractableViews;

	void ValidateRouteKitClassSettings(TArray<FText>& ValidationErrors) const;
	void ValidateObjectiveRouteOrder(TArray<FText>& ValidationErrors) const;
	void ValidateObjectiveNodeDefinitions(TArray<FText>& ValidationErrors) const;
	void ValidateObjectiveNodeIdentity(
		const FDeepWaterStationObjectiveNode& ObjectiveNode,
		int32 NodeIndex,
		TSet<FName>& SeenSourceIds,
		TSet<FName>& SeenTrailerBeatIds,
		TArray<FText>& ValidationErrors) const;
	void ValidateObjectiveNodeText(
		const FDeepWaterStationObjectiveNode& ObjectiveNode,
		int32 NodeIndex,
		TArray<FText>& ValidationErrors) const;
	void ValidateObjectiveNodeRequirements(
		const FDeepWaterStationObjectiveNode& ObjectiveNode,
		int32 NodeIndex,
		TArray<FText>& ValidationErrors) const;
	void ValidateArchiveRevealEscapeBudget(TArray<FText>& ValidationErrors) const;
};
