// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SaveGameMetadata.h"
#include "SaveSlotManager.generated.h"

class UHorrorSaveGame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSaveSlotChanged, int32, SlotIndex, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveSlotDeleted, int32, SlotIndex);

/**
 * Defines Save Slot Manager behavior for the SaveGame module.
 */
UCLASS(BlueprintType)
class HORRORPROJECT_API USaveSlotManager : public UObject
{
	GENERATED_BODY()

public:
	static constexpr int32 MaxSaveSlots = 3;
	static constexpr const TCHAR* MetadataSlotName = TEXT("SaveMetadata");

	UFUNCTION(BlueprintCallable, Category="SaveGame")
	bool SaveToSlot(int32 SlotIndex, UHorrorSaveGame* SaveGame);

	UFUNCTION(BlueprintCallable, Category="SaveGame")
	UHorrorSaveGame* LoadFromSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category="SaveGame")
	bool DeleteSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category="SaveGame")
	bool IsSlotEmpty(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category="SaveGame")
	TArray<FSaveSlotMetadata> GetAllSlotMetadata() const;

	UFUNCTION(BlueprintCallable, Category="SaveGame")
	FSaveSlotMetadata GetSlotMetadata(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category="SaveGame")
	int32 GetLastUsedSlot() const;

	UFUNCTION(BlueprintCallable, Category="SaveGame")
	void SetLastUsedSlot(int32 SlotIndex);

	UPROPERTY(BlueprintAssignable, Category="SaveGame")
	FOnSaveSlotChanged OnSaveSlotChanged;

	UPROPERTY(BlueprintAssignable, Category="SaveGame")
	FOnSaveSlotDeleted OnSaveSlotDeleted;

private:
	FString GetSlotName(int32 SlotIndex) const;
	void LoadMetadata() const;
	void SaveMetadata();
	void UpdateSlotMetadataFromSave(int32 SlotIndex, const UHorrorSaveGame* SaveGame);

	UPROPERTY(Transient)
	mutable TObjectPtr<USaveGameMetadata> Metadata;
};
