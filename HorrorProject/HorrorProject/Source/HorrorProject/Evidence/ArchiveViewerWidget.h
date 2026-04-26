// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Evidence/EvidenceTypes.h"
#include "Evidence/ArchiveSubsystem.h"
#include "ArchiveViewerWidget.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class HORRORPROJECT_API UArchiveViewerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category="Archive|UI")
	void RefreshArchiveView();

	UFUNCTION(BlueprintCallable, Category="Archive|UI")
	void ApplyFilter(const FArchiveFilter& Filter);

	UFUNCTION(BlueprintCallable, Category="Archive|UI")
	void ClearFilter();

	UFUNCTION(BlueprintCallable, Category="Archive|UI")
	void ViewEntry(FName EntryId);

	UFUNCTION(BlueprintPure, Category="Archive|UI")
	int32 GetNewEntryCount() const;

	UFUNCTION(BlueprintPure, Category="Archive|UI")
	int32 GetTotalEntryCount() const;

	UFUNCTION(BlueprintCallable, Category="Archive|UI")
	void SortByDate(bool bNewestFirst = true);

	UFUNCTION(BlueprintCallable, Category="Archive|UI")
	void SortByImportance(bool bHighestFirst = true);

	UFUNCTION(BlueprintCallable, Category="Archive|UI")
	void SortByType();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="Archive|UI", meta=(DisplayName="Archive Entries Updated"))
	void BP_ArchiveEntriesUpdated(const TArray<FArchiveEntry>& Entries);

	UFUNCTION(BlueprintImplementableEvent, Category="Archive|UI", meta=(DisplayName="Entry Viewed"))
	void BP_EntryViewed(FName EntryId);

	UFUNCTION(BlueprintImplementableEvent, Category="Archive|UI", meta=(DisplayName="New Entry Added"))
	void BP_NewEntryAdded(const FArchiveEntry& Entry);

	UFUNCTION()
	void OnArchiveEntryAdded(const FArchiveEntry& Entry);

	UFUNCTION()
	void OnArchiveEntryViewed(FName EntryId);

	UPROPERTY(BlueprintReadOnly, Category="Archive|UI")
	TArray<FArchiveEntry> CurrentEntries;

	UPROPERTY(BlueprintReadOnly, Category="Archive|UI")
	FArchiveFilter CurrentFilter;

	UPROPERTY(BlueprintReadOnly, Category="Archive|UI")
	bool bFilterActive = false;

private:
	void BindToArchiveSubsystem();
	void UnbindFromArchiveSubsystem();
	void UpdateCurrentEntries();

	UPROPERTY()
	TWeakObjectPtr<UArchiveSubsystem> CachedArchiveSubsystem;
};
