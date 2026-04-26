// Copyright Epic Games, Inc. All Rights Reserved.

#include "SaveGameMetadata.h"

FSaveSlotMetadata* USaveGameMetadata::FindSlotMetadata(int32 SlotIndex)
{
	for (FSaveSlotMetadata& Slot : SaveSlots)
	{
		if (Slot.SlotIndex == SlotIndex)
		{
			return &Slot;
		}
	}
	return nullptr;
}

const FSaveSlotMetadata* USaveGameMetadata::FindSlotMetadata(int32 SlotIndex) const
{
	for (const FSaveSlotMetadata& Slot : SaveSlots)
	{
		if (Slot.SlotIndex == SlotIndex)
		{
			return &Slot;
		}
	}
	return nullptr;
}

void USaveGameMetadata::UpdateSlotMetadata(const FSaveSlotMetadata& Metadata)
{
	FSaveSlotMetadata* ExistingSlot = FindSlotMetadata(Metadata.SlotIndex);
	if (ExistingSlot)
	{
		*ExistingSlot = Metadata;
	}
	else
	{
		SaveSlots.Add(Metadata);
	}
}

void USaveGameMetadata::RemoveSlotMetadata(int32 SlotIndex)
{
	SaveSlots.RemoveAll([SlotIndex](const FSaveSlotMetadata& Slot)
	{
		return Slot.SlotIndex == SlotIndex;
	});
}
