// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SaveGameBackup.generated.h"

class UHorrorSaveGame;

USTRUCT(BlueprintType)
struct FSaveGameBackupInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="SaveGame")
	FString BackupName;

	UPROPERTY(BlueprintReadOnly, Category="SaveGame")
	FDateTime BackupTime;

	UPROPERTY(BlueprintReadOnly, Category="SaveGame")
	int32 SlotIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category="SaveGame")
	int64 FileSizeBytes = 0;
};

/**
 * Defines Save Game Backup behavior for the SaveGame module.
 */
UCLASS()
class HORRORPROJECT_API USaveGameBackup : public UObject
{
	GENERATED_BODY()

public:
	static constexpr int32 MaxBackupsPerSlot = 5;

	UFUNCTION(BlueprintCallable, Category="SaveGame")
	bool CreateBackup(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category="SaveGame")
	bool RestoreBackup(const FString& BackupName, int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category="SaveGame")
	bool DeleteBackup(const FString& BackupName);

	UFUNCTION(BlueprintCallable, Category="SaveGame")
	TArray<FSaveGameBackupInfo> GetBackupsForSlot(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category="SaveGame")
	void CleanupOldBackups(int32 SlotIndex);

private:
	FString GetBackupDirectory() const;
	FString GetBackupFileName(int32 SlotIndex, const FDateTime& Timestamp) const;
	FString GetSlotFileName(int32 SlotIndex) const;
	bool CopyFile(const FString& SourcePath, const FString& DestPath) const;
};
