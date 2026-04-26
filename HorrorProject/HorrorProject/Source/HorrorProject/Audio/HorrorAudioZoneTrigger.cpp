// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/HorrorAudioZoneTrigger.h"
#include "Audio/HorrorAudioSubsystem.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

UHorrorAudioZoneTrigger::UHorrorAudioZoneTrigger()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHorrorAudioZoneTrigger::BeginPlay()
{
	Super::BeginPlay();

	CreateTriggerComponent();

	if (bAutoActivate)
	{
		ActivateTrigger();
	}
}

void UHorrorAudioZoneTrigger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DeactivateTrigger();

	if (TriggerComponent)
	{
		TriggerComponent->DestroyComponent();
		TriggerComponent = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void UHorrorAudioZoneTrigger::CreateTriggerComponent()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	if (TriggerShape == EHorrorAudioZoneTriggerShape::Box)
	{
		UBoxComponent* BoxComp = NewObject<UBoxComponent>(Owner, NAME_None, RF_Transient);
		BoxComp->SetBoxExtent(BoxExtent);
		BoxComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		BoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		BoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		BoxComp->SetupAttachment(Owner->GetRootComponent());
		BoxComp->RegisterComponent();
		TriggerComponent = BoxComp;
	}
	else
	{
		USphereComponent* SphereComp = NewObject<USphereComponent>(Owner, NAME_None, RF_Transient);
		SphereComp->SetSphereRadius(SphereRadius);
		SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		SphereComp->SetupAttachment(Owner->GetRootComponent());
		SphereComp->RegisterComponent();
		TriggerComponent = SphereComp;
	}
}

void UHorrorAudioZoneTrigger::ActivateTrigger()
{
	if (bIsActive || !TriggerComponent)
	{
		return;
	}

	bIsActive = true;
	BindTriggerEvents();
}

void UHorrorAudioZoneTrigger::DeactivateTrigger()
{
	if (!bIsActive)
	{
		return;
	}

	bIsActive = false;
	UnbindTriggerEvents();

	if (UWorld* World = GetWorld())
	{
		if (UHorrorAudioSubsystem* AudioSys = World->GetSubsystem<UHorrorAudioSubsystem>())
		{
			if (AudioSys->GetCurrentZoneId() == ZoneId)
			{
				AudioSys->ExitAudioZone(ZoneId);
			}
		}
	}
}

void UHorrorAudioZoneTrigger::BindTriggerEvents()
{
	if (!TriggerComponent)
	{
		return;
	}

	TriggerComponent->OnComponentBeginOverlap.AddDynamic(this, &UHorrorAudioZoneTrigger::OnTriggerBeginOverlap);
	TriggerComponent->OnComponentEndOverlap.AddDynamic(this, &UHorrorAudioZoneTrigger::OnTriggerEndOverlap);
}

void UHorrorAudioZoneTrigger::UnbindTriggerEvents()
{
	if (!TriggerComponent)
	{
		return;
	}

	TriggerComponent->OnComponentBeginOverlap.RemoveDynamic(this, &UHorrorAudioZoneTrigger::OnTriggerBeginOverlap);
	TriggerComponent->OnComponentEndOverlap.RemoveDynamic(this, &UHorrorAudioZoneTrigger::OnTriggerEndOverlap);
}

void UHorrorAudioZoneTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsActive || !IsPlayerActor(OtherActor))
	{
		return;
	}

	if (bTriggerOnce && bHasTriggered)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UHorrorAudioSubsystem* AudioSys = World->GetSubsystem<UHorrorAudioSubsystem>())
		{
			if (AudioSys->EnterAudioZone(ZoneId))
			{
				bHasTriggered = true;
			}
		}
	}
}

void UHorrorAudioZoneTrigger::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!bIsActive || !IsPlayerActor(OtherActor))
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UHorrorAudioSubsystem* AudioSys = World->GetSubsystem<UHorrorAudioSubsystem>())
		{
			AudioSys->ExitAudioZone(ZoneId);
		}
	}
}

bool UHorrorAudioZoneTrigger::IsPlayerActor(AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			return PC->GetPawn() == Actor;
		}
	}

	return false;
}
