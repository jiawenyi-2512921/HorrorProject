// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Evidence/EvidenceTypes.h"
#include "ArchiveSubsystem.generated.h"

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FArchiveEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Archive")
	FName EntryId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category="Archive")
	FExtendedEvidenceMetadata Metadata;

	UPROPERTY(BlueprintReadOnly, Category="Archive")
	FDateTime UnlockedTimestamp;

	UPROPERTY(BlueprintReadOnly, Category="Archive")
	bool bIsNew = true;

	UPROPERTY(BlueprintReadOnly, Category="Archive")
	bool bIsViewed = false;
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FArchiveFilter
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Archive")
	TArray<EEvidenceType> AllowedTypes;

	UPROPERTY(BlueprintReadWrite, Category="Archive")
	TArray<EEvidenceCategory> AllowedCategories;

	UPROPERTY(BlueprintReadWrite, Category="Archive")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(BlueprintReadWrite, Category="Archive")
	bool bKeyEvidenceOnly = false;

	UPROPERTY(BlueprintReadWrite, Category="Archive")
	int32 MinImportanceLevel = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FArchiveEntryAddedSignature, const FArchiveEntry&, Entry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FArchiveEntryViewedSignature, FName, EntryId);

/**
 * Coordinates Archive Subsystem services for the Evidence module.
 */
UCLASS()
class HORRORPROJECT_API UArchiveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category="Archive")
	bool AddArchiveEntry(const FExtendedEvidenceMetadata& Metadata);

	UFUNCTION(BlueprintCallable, Category="Archive")
	bool MarkEntryAsViewed(FName EntryId);

	UFUNCTION(BlueprintCallable, Category="Archive")
	bool MarkEntryAsOld(FName EntryId);

	UFUNCTION(BlueprintPure, Category="Archive")
	bool GetArchiveEntry(FName EntryId, FArchiveEntry& OutEntry) const;

	UFUNCTION(BlueprintPure, Category="Archive")
	TArray<FArchiveEntry> GetAllArchiveEntries() const;

	UFUNCTION(BlueprintPure, Category="Archive")
	TArray<FArchiveEntry> GetFilteredArchiveEntries(const FArchiveFilter& Filter) const;

	UFUNCTION(BlueprintPure, Category="Archive")
	TArray<FArchiveEntry> GetNewEntries() const;

	UFUNCTION(BlueprintPure, Category="Archive")
	TArray<FArchiveEntry> GetUnviewedEntries() const;

	UFUNCTION(BlueprintPure, Category="Archive")
	int32 GetNewEntryCount() const;

	UFUNCTION(BlueprintPure, Category="Archive")
	int32 GetTotalEntryCount() const;

	UFUNCTION(BlueprintCallable, Category="Archive")
	void ClearAllEntries();

	UFUNCTION(BlueprintCallable, Category="Archive")
	TArray<FName> ExportArchiveState() const;

	UFUNCTION(BlueprintCallable, Category="Archive")
	void ImportArchiveState(const TArray<FName>& EntryIds);

	UFUNCTION(BlueprintCallable, Category="Archive")
	TArray<FArchiveEntry> ExportArchiveEntries() const;

	UFUNCTION(BlueprintCallable, Category="Archive")
	void ImportArchiveEntries(const TArray<FArchiveEntry>& Entries);

	UPROPERTY(BlueprintAssignable, Category="Archive")
	FArchiveEntryAddedSignature OnArchiveEntryAdded;

	UPROPERTY(BlueprintAssignable, Category="Archive")
	FArchiveEntryViewedSignature OnArchiveEntryViewed;

private:
	bool PassesFilter(const FArchiveEntry& Entry, const FArchiveFilter& Filter) const;

	UPROPERTY()
	TMap<FName, FArchiveEntry> ArchiveEntries;
};
