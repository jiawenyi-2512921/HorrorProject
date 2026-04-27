// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEvidenceCollectedSignature, FName, EvidenceId, int32, TotalEvidenceCount);

UCLASS(MinimalAPI, Hidden, NotBlueprintable, NotPlaceable)
class UInventoryEvidenceDelegateProbe : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void HandleEvidenceCollected(FName EvidenceId, int32 TotalEvidenceCount);

	int32 BroadcastCount = 0;
	FName LastEvidenceId = NAME_None;
	int32 LastTotalEvidenceCount = 0;
	TArray<FName> EvidenceIds;
	TArray<int32> TotalEvidenceCounts;
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorEvidenceMetadata
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Evidence")
	FName EvidenceId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Evidence")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Evidence")
	FText Description;
};

/**
 * Adds Inventory Component behavior to its owning actor in the Player module.
 */
UCLASS(ClassGroup=(Horror), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	virtual void InitializeComponent() override;

	UFUNCTION(BlueprintCallable, Category="Inventory|Evidence")
	bool AddCollectedEvidenceId(FName EvidenceId);

	UFUNCTION(BlueprintPure, Category="Inventory|Evidence")
	bool HasCollectedEvidenceId(FName EvidenceId) const;

	UFUNCTION(BlueprintPure, Category="Inventory|Evidence")
	const TArray<FName>& GetCollectedEvidenceIds() const;

	UFUNCTION(BlueprintPure, Category="Inventory|Evidence")
	const TArray<FName>& ExportCollectedEvidenceIds() const;

	UFUNCTION(BlueprintCallable, Category="Inventory|Evidence")
	void ImportCollectedEvidenceIds(const TArray<FName>& EvidenceIds);

	UFUNCTION(BlueprintCallable, Category="Inventory|Evidence")
	void RegisterEvidenceMetadata(const FHorrorEvidenceMetadata& Metadata);

	UFUNCTION(BlueprintPure, Category="Inventory|Evidence")
	bool GetEvidenceMetadata(FName EvidenceId, FHorrorEvidenceMetadata& OutMetadata) const;

	UFUNCTION(BlueprintPure, Category="Inventory|Evidence")
	TArray<FHorrorEvidenceMetadata> GetCollectedEvidenceMetadata() const;

	UFUNCTION(BlueprintPure, Category="Inventory|Evidence")
	int32 GetCollectedEvidenceCount() const;

	UPROPERTY(BlueprintAssignable, Category="Inventory|Evidence")
	FEvidenceCollectedSignature OnEvidenceCollected;

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, SaveGame, Category="Inventory|Evidence", meta=(AllowPrivateAccess="true"))
	TArray<FName> CollectedEvidenceIds;

	UPROPERTY(Transient)
	TMap<FName, FHorrorEvidenceMetadata> EvidenceMetadataById;
};
