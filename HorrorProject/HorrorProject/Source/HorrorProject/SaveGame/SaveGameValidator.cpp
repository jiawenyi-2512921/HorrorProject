// Copyright Epic Games, Inc. All Rights Reserved.

#include "SaveGameValidator.h"
#include "HorrorProject/Save/HorrorSaveGame.h"
#include "SaveGameMigration.h"

ESaveGameValidationResult USaveGameValidator::ValidateSaveGame(const UHorrorSaveGame* SaveGame) const
{
	if (!SaveGame)
	{
		return ESaveGameValidationResult::CorruptedData;
	}

	if (!ValidateVersion(SaveGame))
	{
		return ESaveGameValidationResult::InvalidVersion;
	}

	if (!ValidateRequiredData(SaveGame))
	{
		return ESaveGameValidationResult::MissingRequiredData;
	}

	if (!ValidateDataIntegrity(SaveGame))
	{
		return ESaveGameValidationResult::ChecksumMismatch;
	}

	return ESaveGameValidationResult::Valid;
}

bool USaveGameValidator::IsSaveGameValid(const UHorrorSaveGame* SaveGame) const
{
	return ValidateSaveGame(SaveGame) == ESaveGameValidationResult::Valid;
}

FString USaveGameValidator::GetValidationErrorMessage(ESaveGameValidationResult Result) const
{
	switch (Result)
	{
	case ESaveGameValidationResult::Valid:
		return TEXT("Save game is valid");
	case ESaveGameValidationResult::CorruptedData:
		return TEXT("Save game data is corrupted");
	case ESaveGameValidationResult::InvalidVersion:
		return TEXT("Save game version is invalid");
	case ESaveGameValidationResult::MissingRequiredData:
		return TEXT("Save game is missing required data");
	case ESaveGameValidationResult::ChecksumMismatch:
		return TEXT("Save game checksum mismatch");
	default:
		return TEXT("Unknown validation error");
	}
}

bool USaveGameValidator::ValidateVersion(const UHorrorSaveGame* SaveGame) const
{
	if (!SaveGame)
	{
		return false;
	}

	return SaveGame->SaveVersion > 0 && SaveGame->SaveVersion <= USaveGameMigration::CurrentSaveVersion;
}

bool USaveGameValidator::ValidateRequiredData(const UHorrorSaveGame* SaveGame) const
{
	if (!SaveGame)
	{
		return false;
	}

	// Validate that checkpoint ID is set
	if (SaveGame->CheckpointId.IsNone())
	{
		return false;
	}

	return true;
}

bool USaveGameValidator::ValidateDataIntegrity(const UHorrorSaveGame* SaveGame) const
{
	if (!SaveGame)
	{
		return false;
	}

	// Basic integrity checks
	// Could add CRC or hash validation here
	return true;
}
