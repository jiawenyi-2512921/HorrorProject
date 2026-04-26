// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UIEventManager.generated.h"

class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIStateChanged, FName, StateName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUIWidgetOpened, UUserWidget*, Widget, FName, WidgetName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUIWidgetClosed, UUserWidget*, Widget, FName, WidgetName);

/**
 * UI event manager for coordinating UI state and widget lifecycle
 */
UCLASS(BlueprintType)
class HORRORPROJECT_API UUIEventManager : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="UI|Events")
	FOnUIStateChanged OnUIStateChanged;

	UPROPERTY(BlueprintAssignable, Category="UI|Events")
	FOnUIWidgetOpened OnUIWidgetOpened;

	UPROPERTY(BlueprintAssignable, Category="UI|Events")
	FOnUIWidgetClosed OnUIWidgetClosed;

	UFUNCTION(BlueprintCallable, Category="UI|Events")
	void BroadcastUIStateChanged(FName StateName);

	UFUNCTION(BlueprintCallable, Category="UI|Events")
	void BroadcastWidgetOpened(UUserWidget* Widget, FName WidgetName);

	UFUNCTION(BlueprintCallable, Category="UI|Events")
	void BroadcastWidgetClosed(UUserWidget* Widget, FName WidgetName);

	UFUNCTION(BlueprintCallable, Category="UI|Events")
	void RegisterWidget(UUserWidget* Widget, FName WidgetName);

	UFUNCTION(BlueprintCallable, Category="UI|Events")
	void UnregisterWidget(FName WidgetName);

	UFUNCTION(BlueprintPure, Category="UI|Events")
	UUserWidget* GetRegisteredWidget(FName WidgetName) const;

	UFUNCTION(BlueprintPure, Category="UI|Events")
	bool IsWidgetRegistered(FName WidgetName) const;

private:
	UPROPERTY()
	TMap<FName, TWeakObjectPtr<UUserWidget>> RegisteredWidgets;
};
