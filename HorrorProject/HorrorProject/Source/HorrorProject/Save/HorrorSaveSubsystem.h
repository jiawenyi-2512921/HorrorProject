// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HorrorSaveSubsystem.generated.h"

class UHorrorSaveGame;
class UWorld;

UCLASS()
class HORRORPROJECT_API UHorrorSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Horror|Save", meta=(WorldContext="WorldContextObject"))
	bool SaveCheckpoint(UObject* WorldContextObject, FName CheckpointId);

	UFUNCTION(BlueprintCallable, Category="Horror|Save", meta=(WorldContext="WorldContextObject"))
	bool LoadCheckpoint(UObject* WorldContextObject);

	UHorrorSaveGame* CreateCheckpointSnapshot(UObject* WorldContextObject, FName CheckpointId) const;
	bool ApplyCheckpointSnapshot(UObject* WorldContextObject, const UHorrorSaveGame* SaveGame) const;

	UFUNCTION(BlueprintPure, Category="Horror|Save")
	bool HasSave() const;

	UFUNCTION(BlueprintCallable, Category="Horror|Save")
	void ClearCachedSaveOnly();

private:
	UWorld* ResolveWorld(const UObject* WorldContextObject) const;

	UPROPERTY(Transient)
	TObjectPtr<UHorrorSaveGame> CachedSave;
};
