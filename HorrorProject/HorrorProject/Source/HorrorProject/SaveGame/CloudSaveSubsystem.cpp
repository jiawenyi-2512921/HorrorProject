// Copyright Epic Games, Inc. All Rights Reserved.

#include "CloudSaveSubsystem.h"
#include "SaveSlotManager.h"
#include "HorrorProject/Save/HorrorSaveGame.h"
#include "Kismet/GameplayStatics.h"

void UCloudSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SlotManager = NewObject<USaveSlotManager>(this);
	DetectCloudProvider();
}

void UCloudSaveSubsystem::Deinitialize()
{
	SlotManager = nullptr;
	Super::Deinitialize();
}

void UCloudSaveSubsystem::UploadSaveToCloud(int32 SlotIndex)
{
	if (!IsCloudSaveEnabled() || !SlotManager)
	{
		OnCloudSaveComplete.Broadcast(SlotIndex, ECloudSaveStatus::Failed, TEXT("Cloud save not enabled"));
		return;
	}

	// Security: Validate slot index
	if (SlotIndex < 0 || SlotIndex >= 100)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid save slot index: %d"), SlotIndex);
		OnCloudSaveComplete.Broadcast(SlotIndex, ECloudSaveStatus::Failed, TEXT("Invalid slot index"));
		return;
	}

	if (SlotManager->IsSlotEmpty(SlotIndex))
	{
		OnCloudSaveComplete.Broadcast(SlotIndex, ECloudSaveStatus::Failed, TEXT("Save slot is empty"));
		return;
	}

	CurrentStatus = ECloudSaveStatus::Uploading;

	bool bSuccess = false;
	switch (CloudProvider)
	{
	case ECloudSaveProvider::Steam:
		bSuccess = UploadToSteamCloud(SlotIndex);
		break;
	case ECloudSaveProvider::Epic:
		bSuccess = UploadToEpicCloud(SlotIndex);
		break;
	default:
		break;
	}

	CurrentStatus = bSuccess ? ECloudSaveStatus::Success : ECloudSaveStatus::Failed;
	OnCloudSaveComplete.Broadcast(SlotIndex, CurrentStatus, bSuccess ? TEXT("") : TEXT("Upload failed"));
}

void UCloudSaveSubsystem::DownloadSaveFromCloud(int32 SlotIndex)
{
	if (!IsCloudSaveEnabled())
	{
		OnCloudSaveComplete.Broadcast(SlotIndex, ECloudSaveStatus::Failed, TEXT("Cloud save not enabled"));
		return;
	}

	// Security: Validate slot index
	if (SlotIndex < 0 || SlotIndex >= 100)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid save slot index: %d"), SlotIndex);
		OnCloudSaveComplete.Broadcast(SlotIndex, ECloudSaveStatus::Failed, TEXT("Invalid slot index"));
		return;
	}

	CurrentStatus = ECloudSaveStatus::Downloading;

	bool bSuccess = false;
	switch (CloudProvider)
	{
	case ECloudSaveProvider::Steam:
		bSuccess = DownloadFromSteamCloud(SlotIndex);
		break;
	case ECloudSaveProvider::Epic:
		bSuccess = DownloadFromEpicCloud(SlotIndex);
		break;
	default:
		break;
	}

	CurrentStatus = bSuccess ? ECloudSaveStatus::Success : ECloudSaveStatus::Failed;
	OnCloudSaveComplete.Broadcast(SlotIndex, CurrentStatus, bSuccess ? TEXT("") : TEXT("Download failed"));
}

void UCloudSaveSubsystem::SyncAllSaves()
{
	if (!IsCloudSaveEnabled() || !SlotManager)
	{
		return;
	}

	for (int32 i = 0; i < USaveSlotManager::MaxSaveSlots; ++i)
	{
		if (!SlotManager->IsSlotEmpty(i))
		{
			UploadSaveToCloud(i);
		}
	}
}

bool UCloudSaveSubsystem::IsCloudSaveEnabled() const
{
	return CloudProvider != ECloudSaveProvider::None;
}

ECloudSaveProvider UCloudSaveSubsystem::GetCloudProvider() const
{
	return CloudProvider;
}

void UCloudSaveSubsystem::DetectCloudProvider()
{
	CloudProvider = ECloudSaveProvider::None;

#if WITH_STEAMWORKS
	if (IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get(STEAM_SUBSYSTEM))
	{
		CloudProvider = ECloudSaveProvider::Steam;
		return;
	}
#endif

#if WITH_EOS_SDK
	if (IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get(EOS_SUBSYSTEM))
	{
		CloudProvider = ECloudSaveProvider::Epic;
		return;
	}
#endif
}

bool UCloudSaveSubsystem::UploadToSteamCloud(int32 SlotIndex)
{
#if WITH_STEAMWORKS
	// Steam Cloud implementation would go here
	// For now, return true as placeholder
	UE_LOG(LogTemp, Log, TEXT("Uploading save slot %d to Steam Cloud"), SlotIndex);
	return true;
#else
	return false;
#endif
}

bool UCloudSaveSubsystem::DownloadFromSteamCloud(int32 SlotIndex)
{
#if WITH_STEAMWORKS
	// Steam Cloud implementation would go here
	UE_LOG(LogTemp, Log, TEXT("Downloading save slot %d from Steam Cloud"), SlotIndex);
	return true;
#else
	return false;
#endif
}

bool UCloudSaveSubsystem::UploadToEpicCloud(int32 SlotIndex)
{
#if WITH_EOS_SDK
	// Epic Cloud implementation would go here
	UE_LOG(LogTemp, Log, TEXT("Uploading save slot %d to Epic Cloud"), SlotIndex);
	return true;
#else
	return false;
#endif
}

bool UCloudSaveSubsystem::DownloadFromEpicCloud(int32 SlotIndex)
{
#if WITH_EOS_SDK
	// Epic Cloud implementation would go here
	UE_LOG(LogTemp, Log, TEXT("Downloading save slot %d from Epic Cloud"), SlotIndex);
	return true;
#else
	return false;
#endif
}
