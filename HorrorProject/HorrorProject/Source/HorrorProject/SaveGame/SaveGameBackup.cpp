// Copyright Epic Games, Inc. All Rights Reserved.

#include "SaveGameBackup.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "Misc/FileHelper.h"

bool USaveGameBackup::CreateBackup(int32 SlotIndex)
{
	const FString SourcePath = FPaths::ProjectSavedDir() / TEXT("SaveGames") / GetSlotFileName(SlotIndex);

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.FileExists(*SourcePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Source save file does not exist: %s"), *SourcePath);
		return false;
	}

	const FDateTime Now = FDateTime::Now();
	const FString BackupDir = GetBackupDirectory();
	const FString BackupPath = BackupDir / GetBackupFileName(SlotIndex, Now);

	if (!PlatformFile.DirectoryExists(*BackupDir))
	{
		PlatformFile.CreateDirectoryTree(*BackupDir);
	}

	if (CopyFile(SourcePath, BackupPath))
	{
		UE_LOG(LogTemp, Log, TEXT("Created backup: %s"), *BackupPath);
		CleanupOldBackups(SlotIndex);
		return true;
	}

	return false;
}

bool USaveGameBackup::RestoreBackup(const FString& BackupName, int32 SlotIndex)
{
	const FString BackupPath = GetBackupDirectory() / BackupName;
	const FString DestPath = FPaths::ProjectSavedDir() / TEXT("SaveGames") / GetSlotFileName(SlotIndex);

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.FileExists(*BackupPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Backup file does not exist: %s"), *BackupPath);
		return false;
	}

	if (CopyFile(BackupPath, DestPath))
	{
		UE_LOG(LogTemp, Log, TEXT("Restored backup: %s to slot %d"), *BackupName, SlotIndex);
		return true;
	}

	return false;
}

bool USaveGameBackup::DeleteBackup(const FString& BackupName)
{
	const FString BackupPath = GetBackupDirectory() / BackupName;

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.DeleteFile(*BackupPath))
	{
		UE_LOG(LogTemp, Log, TEXT("Deleted backup: %s"), *BackupName);
		return true;
	}

	return false;
}

TArray<FSaveGameBackupInfo> USaveGameBackup::GetBackupsForSlot(int32 SlotIndex) const
{
	TArray<FSaveGameBackupInfo> Backups;
	const FString BackupDir = GetBackupDirectory();
	const FString SlotPrefix = FString::Printf(TEXT("SaveSlot_%d_Backup_"), SlotIndex);

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	class FBackupVisitor : public IPlatformFile::FDirectoryVisitor
	{
	public:
		TArray<FSaveGameBackupInfo>& BackupsRef;
		const FString& PrefixRef;
		int32 SlotIdx;

		FBackupVisitor(TArray<FSaveGameBackupInfo>& InBackups, const FString& InPrefix, int32 InSlotIdx)
			: BackupsRef(InBackups), PrefixRef(InPrefix), SlotIdx(InSlotIdx)
		{}

		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
		{
			if (!bIsDirectory)
			{
				FString Filename = FPaths::GetCleanFilename(FilenameOrDirectory);
				if (Filename.StartsWith(PrefixRef))
				{
					FSaveGameBackupInfo Info;
					Info.BackupName = Filename;
					Info.SlotIndex = SlotIdx;
					Info.FileSizeBytes = IFileManager::Get().FileSize(FilenameOrDirectory);

					// Parse timestamp from filename
					FString TimeStr = Filename.RightChop(PrefixRef.Len()).LeftChop(4); // Remove .sav
					FDateTime::Parse(TimeStr, Info.BackupTime);

					BackupsRef.Add(Info);
				}
			}
			return true;
		}
	};

	FBackupVisitor Visitor(Backups, SlotPrefix, SlotIndex);
	PlatformFile.IterateDirectory(*BackupDir, Visitor);

	// Sort by backup time, newest first
	Backups.Sort([](const FSaveGameBackupInfo& A, const FSaveGameBackupInfo& B)
	{
		return A.BackupTime > B.BackupTime;
	});

	return Backups;
}

void USaveGameBackup::CleanupOldBackups(int32 SlotIndex)
{
	TArray<FSaveGameBackupInfo> Backups = GetBackupsForSlot(SlotIndex);

	if (Backups.Num() > MaxBackupsPerSlot)
	{
		for (int32 i = MaxBackupsPerSlot; i < Backups.Num(); ++i)
		{
			DeleteBackup(Backups[i].BackupName);
		}
	}
}

FString USaveGameBackup::GetBackupDirectory() const
{
	return FPaths::ProjectSavedDir() / TEXT("SaveGames") / TEXT("Backups");
}

FString USaveGameBackup::GetBackupFileName(int32 SlotIndex, const FDateTime& Timestamp) const
{
	return FString::Printf(TEXT("SaveSlot_%d_Backup_%s.sav"),
		SlotIndex,
		*Timestamp.ToString(TEXT("%Y%m%d_%H%M%S")));
}

FString USaveGameBackup::GetSlotFileName(int32 SlotIndex) const
{
	return FString::Printf(TEXT("SaveSlot_%d.sav"), SlotIndex);
}

bool USaveGameBackup::CopyFile(const FString& SourcePath, const FString& DestPath) const
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	return PlatformFile.CopyFile(*DestPath, *SourcePath);
}
