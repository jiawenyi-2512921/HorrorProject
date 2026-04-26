// Copyright Epic Games, Inc. All Rights Reserved.

#include "UIEventManager.h"
#include "Blueprint/UserWidget.h"

void UUIEventManager::BroadcastUIStateChanged(FName StateName)
{
	OnUIStateChanged.Broadcast(StateName);
}

void UUIEventManager::BroadcastWidgetOpened(UUserWidget* Widget, FName WidgetName)
{
	OnUIWidgetOpened.Broadcast(Widget, WidgetName);
}

void UUIEventManager::BroadcastWidgetClosed(UUserWidget* Widget, FName WidgetName)
{
	OnUIWidgetClosed.Broadcast(Widget, WidgetName);
}

void UUIEventManager::RegisterWidget(UUserWidget* Widget, FName WidgetName)
{
	if (Widget && WidgetName != NAME_None)
	{
		RegisteredWidgets.Add(WidgetName, Widget);
	}
}

void UUIEventManager::UnregisterWidget(FName WidgetName)
{
	RegisteredWidgets.Remove(WidgetName);
}

UUserWidget* UUIEventManager::GetRegisteredWidget(FName WidgetName) const
{
	if (const TWeakObjectPtr<UUserWidget>* WidgetPtr = RegisteredWidgets.Find(WidgetName))
	{
		return WidgetPtr->Get();
	}
	return nullptr;
}

bool UUIEventManager::IsWidgetRegistered(FName WidgetName) const
{
	return RegisteredWidgets.Contains(WidgetName) && RegisteredWidgets[WidgetName].IsValid();
}
