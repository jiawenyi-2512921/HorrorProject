// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoSaveComponent.h"
#include "HorrorProject/Save/HorrorSaveSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UAutoSaveComponent::UAutoSaveComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAutoSaveComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
	{
		SaveSubsystem = GameInstance->GetSubsystem<UHorrorSaveSubsystem>();
	}

	if (bAutoSaveEnabled)
	{
		ScheduleNextAutoSave();
	}
}

void UAutoSaveComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UAutoSaveComponent::EnableAutoSave(bool bEnable)
{
	bAutoSaveEnabled = bEnable;

	if (UWorld* World = GetWorld())
	{
		if (bEnable)
		{
			ScheduleNextAutoSave();
		}
		else
		{
			World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
		}
	}
}

void UAutoSaveComponent::SetAutoSaveInterval(float Seconds)
{
	AutoSaveIntervalSeconds = FMath::Clamp(Seconds, 30.0f, 600.0f);

	if (bAutoSaveEnabled)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
			ScheduleNextAutoSave();
		}
	}
}

void UAutoSaveComponent::TriggerAutoSave()
{
	PerformAutoSave();
}

void UAutoSaveComponent::ScheduleNextAutoSave()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			AutoSaveTimerHandle,
			this,
			&UAutoSaveComponent::PerformAutoSave,
			AutoSaveIntervalSeconds,
			false
		);
	}
}

void UAutoSaveComponent::PerformAutoSave()
{
	if (SaveSubsystem)
	{
		const FName AutoSaveCheckpoint = FName(TEXT("AutoSave"));
		const bool bSuccess = SaveSubsystem->SaveCheckpoint(this, AutoSaveCheckpoint);

		if (bSuccess && bShowAutoSaveNotification)
		{
			UE_LOG(LogTemp, Log, TEXT("Auto-save completed successfully"));
		}
	}

	if (bAutoSaveEnabled)
	{
		ScheduleNextAutoSave();
	}
}
