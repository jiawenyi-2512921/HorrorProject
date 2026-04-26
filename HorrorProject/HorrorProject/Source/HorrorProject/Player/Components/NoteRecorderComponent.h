// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NoteRecorderComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNoteRecordedSignature, FName, NoteId, int32, TotalRecordedNotes);

#if WITH_DEV_AUTOMATION_TESTS
UCLASS(MinimalAPI, Hidden, NotBlueprintable, NotPlaceable)
class UNoteRecorderDelegateProbe : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void HandleNoteRecorded(FName NoteId, int32 TotalRecordedNotes);

	int32 BroadcastCount = 0;
	FName LastNoteId = NAME_None;
	int32 LastTotalRecordedNotes = 0;
	TArray<FName> NoteIds;
	TArray<int32> TotalRecordedNoteCounts;
};
#endif

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorNoteMetadata
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Notes|Archive")
	FName NoteId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Notes|Archive")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Notes|Archive")
	FText Body;
};

UCLASS(ClassGroup=(Horror), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UNoteRecorderComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNoteRecorderComponent();

	UFUNCTION(BlueprintCallable, Category="Notes|Archive")
	bool AddRecordedNoteId(FName NoteId);

	UFUNCTION(BlueprintPure, Category="Notes|Archive")
	bool HasRecordedNoteId(FName NoteId) const;

	UFUNCTION(BlueprintPure, Category="Notes|Archive")
	const TArray<FName>& GetRecordedNoteIds() const;

	UFUNCTION(BlueprintPure, Category="Notes|Archive")
	const TArray<FName>& ExportRecordedNoteIds() const;

	UFUNCTION(BlueprintCallable, Category="Notes|Archive")
	void ImportRecordedNoteIds(const TArray<FName>& NoteIds);

	UFUNCTION(BlueprintCallable, Category="Notes|Archive")
	void RegisterNoteMetadata(const FHorrorNoteMetadata& Metadata);

	UFUNCTION(BlueprintPure, Category="Notes|Archive")
	bool GetNoteMetadata(FName NoteId, FHorrorNoteMetadata& OutMetadata) const;

	UFUNCTION(BlueprintPure, Category="Notes|Archive")
	TArray<FHorrorNoteMetadata> GetRecordedNoteMetadata() const;

	UFUNCTION(BlueprintPure, Category="Notes|Archive")
	int32 GetRecordedNoteCount() const;

	UPROPERTY(BlueprintAssignable, Category="Notes|Archive")
	FNoteRecordedSignature OnNoteRecorded;

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, SaveGame, Category="Notes|Archive", meta=(AllowPrivateAccess="true"))
	TArray<FName> RecordedNoteIds;

	UPROPERTY(Transient)
	TMap<FName, FHorrorNoteMetadata> NoteMetadataById;
};
