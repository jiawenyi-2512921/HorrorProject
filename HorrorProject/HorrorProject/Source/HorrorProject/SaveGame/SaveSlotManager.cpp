// Copyright Epic Games, Inc. All Rights Reserved.

#include "SaveSlotManager.h"
#include "HorrorProject/Save/HorrorSaveGame.h"
#include "Kismet/GameplayStatics.h"

bool USaveSlotManager::SaveToSlot(int32 SlotIndex, UHorrorSaveGame* SaveGame)
{
	if (SlotIndex < 0 || SlotIndex >= MaxSaveSlots || !SaveGame)
	{
		return false;
	}

	const FString SlotName = GetSlotName(SlotIndex);
	const bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0);

	if (bSuccess)
	{
		LoadMetadata();
		UpdateSlotMetadataFromSave(SlotIndex, SaveGame);
		SetLastUsedSlot(SlotIndex);
		SaveMetadata();
		OnSaveSlotChanged.Broadcast(SlotIndex, true);
	}

	return bSuccess;
}

UHorrorSaveGame* USaveSlotManager::LoadFromSlot(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxSaveSlots)
	{
		return nullptr;
	}

	const FString SlotName = GetSlotName(SlotIndex);
	USaveGame* LoadedSave = UGameplayStatics::LoadGameFromSlot(SlotName, 0);

	if (UHorrorSaveGame* HorrorSave = Cast<UHorrorSaveGame>(LoadedSave))
	{
		SetLastUsedSlot(SlotIndex);
		return HorrorSave;
	}

	return nullptr;
}

bool USaveSlotManager::DeleteSlot(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxSaveSlots)
	{
		return false;
	}

	const FString SlotName = GetSlotName(SlotIndex);
	const bool bSuccess = UGameplayStatics::DeleteGameInSlot(SlotName, 0);

	if (bSuccess)
	{
		LoadMetadata();
		if (Metadata)
		{
			Metadata->RemoveSlotMetadata(SlotIndex);
			SaveMetadata();
		}
		OnSaveSlotDeleted.Broadcast(SlotIndex);
	}

	return bSuccess;
}

bool USaveSlotManager::IsSlotEmpty(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= MaxSaveSlots)
	{
		return true;
	}

	const FString SlotName = GetSlotName(SlotIndex);
	return !UGameplayStatics::DoesSaveGameExist(SlotName, 0);
}

TArray<FSaveSlotMetadata> USaveSlotManager::GetAllSlotMetadata() const
{
	if (!Metadata)
	{
		LoadMetadata();
	}

	return Metadata ? Metadata->SaveSlots : TArray<FSaveSlotMetadata>();
}

FSaveSlotMetadata USaveSlotManager::GetSlotMetadata(int32 SlotIndex) const
{
	if (!Metadata)
	{
		LoadMetadata();
	}

	if (Metadata)
	{
		if (const FSaveSlotMetadata* SlotMeta = Metadata->FindSlotMetadata(SlotIndex))
		{
			return *SlotMeta;
		}
	}

	FSaveSlotMetadata EmptyMetadata;
	EmptyMetadata.SlotIndex = SlotIndex;
	return EmptyMetadata;
}

int32 USaveSlotManager::GetLastUsedSlot() const
{
	if (!Metadata)
	{
		LoadMetadata();
	}

	return Metadata ? Metadata->LastUsedSlot : 0;
}

void USaveSlotManager::SetLastUsedSlot(int32 SlotIndex)
{
	LoadMetadata();
	if (Metadata)
	{
		Metadata->LastUsedSlot = SlotIndex;
		SaveMetadata();
	}
}

FString USaveSlotManager::GetSlotName(int32 SlotIndex) const
{
	return FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);
}

void USaveSlotManager::LoadMetadata() const
{
	if (!Metadata)
	{
		USaveGame* LoadedMetadata = UGameplayStatics::LoadGameFromSlot(MetadataSlotName, 0);
		Metadata = Cast<USaveGameMetadata>(LoadedMetadata);

		if (!Metadata)
		{
			Metadata = NewObject<USaveGameMetadata>(const_cast<USaveSlotManager*>(this));
		}
	}
}

void USaveSlotManager::SaveMetadata()
{
	if (Metadata)
	{
		UGameplayStatics::SaveGameToSlot(Metadata, MetadataSlotName, 0);
	}
}

void USaveSlotManager::UpdateSlotMetadataFromSave(int32 SlotIndex, const UHorrorSaveGame* SaveGame)
{
	if (!Metadata || !SaveGame)
	{
		return;
	}

	FSaveSlotMetadata SlotMeta;
	SlotMeta.SlotIndex = SlotIndex;
	SlotMeta.SlotName = GetSlotName(SlotIndex);
	SlotMeta.SaveTime = FDateTime::Now();
	SlotMeta.SaveVersion = SaveGame->SaveVersion;
	SlotMeta.CheckpointId = SaveGame->CheckpointId;
	SlotMeta.bIsCloudSynced = false;
	SlotMeta.bIsCorrupted = false;

	Metadata->UpdateSlotMetadata(SlotMeta);
}
