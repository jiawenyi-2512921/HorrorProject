// Copyright Epic Games, Inc. All Rights Reserved.

#include "UIManagerSubsystem.h"
#include "Blueprint/UserWidget.h"

void UUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	EventManager = NewObject<UUIEventManager>(this);
}

void UUIManagerSubsystem::Deinitialize()
{
	for (auto& Pair : ManagedWidgets)
	{
		if (Pair.Value)
		{
			Pair.Value->RemoveFromParent();
		}
	}
	ManagedWidgets.Empty();

	Super::Deinitialize();
}

UUserWidget* UUIManagerSubsystem::CreateWidget(TSubclassOf<UUserWidget> WidgetClass, FName WidgetName)
{
	if (!WidgetClass || WidgetName == NAME_None)
	{
		return nullptr;
	}

	if (ManagedWidgets.Contains(WidgetName))
	{
		return ManagedWidgets[WidgetName];
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UUserWidget* NewWidget = ::CreateWidget<UUserWidget>(World, WidgetClass);
	if (NewWidget)
	{
		ManagedWidgets.Add(WidgetName, NewWidget);
		EventManager->RegisterWidget(NewWidget, WidgetName);
	}

	return NewWidget;
}

void UUIManagerSubsystem::ShowWidget(FName WidgetName)
{
	if (UUserWidget* Widget = GetWidget(WidgetName))
	{
		if (!Widget->IsInViewport())
		{
			Widget->AddToViewport();
			EventManager->BroadcastWidgetOpened(Widget, WidgetName);
		}
	}
}

void UUIManagerSubsystem::HideWidget(FName WidgetName)
{
	if (UUserWidget* Widget = GetWidget(WidgetName))
	{
		if (Widget->IsInViewport())
		{
			Widget->RemoveFromParent();
			EventManager->BroadcastWidgetClosed(Widget, WidgetName);
		}
	}
}

void UUIManagerSubsystem::RemoveWidget(FName WidgetName)
{
	if (UUserWidget* Widget = GetWidget(WidgetName))
	{
		Widget->RemoveFromParent();
		EventManager->UnregisterWidget(WidgetName);
		ManagedWidgets.Remove(WidgetName);
	}
}

UUserWidget* UUIManagerSubsystem::GetWidget(FName WidgetName) const
{
	if (const TObjectPtr<UUserWidget>* WidgetPtr = ManagedWidgets.Find(WidgetName))
	{
		return *WidgetPtr;
	}
	return nullptr;
}

void UUIManagerSubsystem::SetUIState(FName StateName)
{
	if (CurrentUIState != StateName)
	{
		CurrentUIState = StateName;
		EventManager->BroadcastUIStateChanged(StateName);
	}
}

void UUIManagerSubsystem::BatchUpdateBegin()
{
	bBatchUpdateActive = true;
	PendingUpdates.Empty();
}

void UUIManagerSubsystem::BatchUpdateEnd()
{
	bBatchUpdateActive = false;

	for (const FName& WidgetName : PendingUpdates)
	{
		ShowWidget(WidgetName);
	}

	PendingUpdates.Empty();
}
