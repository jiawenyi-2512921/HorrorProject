// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UIEventManager.h"
#include "UIManagerSubsystem.generated.h"

class UUserWidget;

/**
 * UI manager subsystem for centralized UI lifecycle and state management
 */
UCLASS()
class HORRORPROJECT_API UUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category="UI|Manager")
	UUserWidget* CreateWidget(TSubclassOf<UUserWidget> WidgetClass, FName WidgetName);

	UFUNCTION(BlueprintCallable, Category="UI|Manager")
	void ShowWidget(FName WidgetName);

	UFUNCTION(BlueprintCallable, Category="UI|Manager")
	void HideWidget(FName WidgetName);

	UFUNCTION(BlueprintCallable, Category="UI|Manager")
	void RemoveWidget(FName WidgetName);

	UFUNCTION(BlueprintPure, Category="UI|Manager")
	UUserWidget* GetWidget(FName WidgetName) const;

	UFUNCTION(BlueprintPure, Category="UI|Manager")
	UUIEventManager* GetEventManager() const { return EventManager; }

	UFUNCTION(BlueprintCallable, Category="UI|Manager")
	void SetUIState(FName StateName);

	UFUNCTION(BlueprintPure, Category="UI|Manager")
	FName GetCurrentUIState() const { return CurrentUIState; }

	UFUNCTION(BlueprintCallable, Category="UI|Manager")
	void BatchUpdateBegin();

	UFUNCTION(BlueprintCallable, Category="UI|Manager")
	void BatchUpdateEnd();

private:
	UPROPERTY()
	TObjectPtr<UUIEventManager> EventManager;

	UPROPERTY()
	TMap<FName, TObjectPtr<UUserWidget>> ManagedWidgets;

	FName CurrentUIState = NAME_None;
	bool bBatchUpdateActive = false;
	TArray<FName> PendingUpdates;
};
