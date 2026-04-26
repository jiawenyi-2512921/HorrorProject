// Copyright Epic Games, Inc. All Rights Reserved.

#include "EvidenceSaveGame.h"
#include "Kismet/GameplayStatics.h"

UEvidenceSaveGame* UEvidenceSaveGame::CreateEvidenceSaveGame()
{
	return Cast<UEvidenceSaveGame>(UGameplayStatics::CreateSaveGameObject(UEvidenceSaveGame::StaticClass()));
}

bool UEvidenceSaveGame::SaveEvidenceData(const FString& SlotName, const FEvidenceSaveData& Data)
{
	UEvidenceSaveGame* SaveGameInstance = CreateEvidenceSaveGame();
	if (!SaveGameInstance)
	{
		return false;
	}

	SaveGameInstance->EvidenceData = Data;
	return UGameplayStatics::SaveGameToSlot(SaveGameInstance, SlotName, 0);
}

bool UEvidenceSaveGame::LoadEvidenceData(const FString& SlotName, FEvidenceSaveData& OutData)
{
	if (UEvidenceSaveGame* LoadedGame = Cast<UEvidenceSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0)))
	{
		OutData = LoadedGame->EvidenceData;
		return true;
	}
	return false;
}
