// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CloudSaveSubsystem.generated.h"

class UHorrorSaveGame;
class USaveSlotManager;

UENUM(BlueprintType)
enum class ECloudSaveProvider : uint8
{
	None,
	Steam,
	Epic
};

UENUM(BlueprintType)
enum class ECloudSaveStatus : uint8
{
	Idle,
	Uploading,
	Downloading,
	Success,
	Failed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCloudSaveComplete, int32, SlotIndex, ECloudSaveStatus, Status, const FString&, ErrorMessage);

UCLASS()
class HORRORPROJECT_API UCloudSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category="CloudSave")
	void UploadSaveToCloud(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category="CloudSave")
	void DownloadSaveFromCloud(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category="CloudSave")
	void SyncAllSaves();

	UFUNCTION(BlueprintCallable, Category="CloudSave")
	bool IsCloudSaveEnabled() const;

	UFUNCTION(BlueprintCallable, Category="CloudSave")
	ECloudSaveProvider GetCloudProvider() const;

	UFUNCTION(BlueprintPure, Category="CloudSave")
	ECloudSaveStatus GetCurrentStatus() const { return CurrentStatus; }

	UPROPERTY(BlueprintAssignable, Category="CloudSave")
	FOnCloudSaveComplete OnCloudSaveComplete;

private:
	void DetectCloudProvider();
	bool UploadToSteamCloud(int32 SlotIndex);
	bool DownloadFromSteamCloud(int32 SlotIndex);
	bool UploadToEpicCloud(int32 SlotIndex);
	bool DownloadFromEpicCloud(int32 SlotIndex);

	UPROPERTY(Transient)
	TObjectPtr<USaveSlotManager> SlotManager;

	UPROPERTY(Transient)
	ECloudSaveProvider CloudProvider = ECloudSaveProvider::None;

	UPROPERTY(Transient)
	ECloudSaveStatus CurrentStatus = ECloudSaveStatus::Idle;
};
