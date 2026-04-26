// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Evidence/EvidenceTypes.h"
#include "ArchiveMenuWidget.generated.h"

UENUM(BlueprintType)
enum class EArchiveEntryType : uint8
{
	All UMETA(DisplayName="All"),
	Evidence UMETA(DisplayName="Evidence"),
	Notes UMETA(DisplayName="Notes")
};

/**
 * Archive menu for viewing collected evidence and notes
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class HORRORPROJECT_API UArchiveMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category="Archive|UI")
	void OpenArchive();

	UFUNCTION(BlueprintCallable, Category="Archive|UI")
	void CloseArchive();

	UFUNCTION(BlueprintCallable, Category="Archive|UI")
	void RefreshArchiveData();

	UFUNCTION(BlueprintCallable, Category="Archive|UI")
	void SelectEntry(FName EntryId);

	UFUNCTION(BlueprintCallable, Category="Archive|UI")
	void FilterByType(EArchiveEntryType Type);

	UFUNCTION(BlueprintCallable, Category="Archive|UI")
	void ClearFilter();

	UFUNCTION(BlueprintPure, Category="Archive|UI")
	bool IsOpen() const { return bIsOpen; }

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="Archive|UI", meta=(DisplayName="Archive Opened"))
	void BP_ArchiveOpened();

	UFUNCTION(BlueprintImplementableEvent, Category="Archive|UI", meta=(DisplayName="Archive Closed"))
	void BP_ArchiveClosed();

	UFUNCTION(BlueprintImplementableEvent, Category="Archive|UI", meta=(DisplayName="Archive Data Refreshed"))
	void BP_ArchiveDataRefreshed(const TArray<FHorrorEvidenceMetadata>& Evidence, const TArray<FHorrorNoteMetadata>& Notes);

	UFUNCTION(BlueprintImplementableEvent, Category="Archive|UI", meta=(DisplayName="Entry Selected"))
	void BP_EntrySelected(FName EntryId);

	UFUNCTION(BlueprintImplementableEvent, Category="Archive|UI", meta=(DisplayName="Filter Applied"))
	void BP_FilterApplied(EArchiveEntryType Type);

	UPROPERTY(BlueprintReadOnly, Category="Archive|UI")
	bool bIsOpen = false;

	UPROPERTY(BlueprintReadOnly, Category="Archive|UI")
	FName SelectedEntryId;

	UPROPERTY(BlueprintReadOnly, Category="Archive|UI")
	EArchiveEntryType CurrentFilter = EArchiveEntryType::All;

	UPROPERTY(EditDefaultsOnly, Category="Archive|UI")
	float OpenAnimationDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category="Archive|UI")
	float CloseAnimationDuration = 0.3f;
};
