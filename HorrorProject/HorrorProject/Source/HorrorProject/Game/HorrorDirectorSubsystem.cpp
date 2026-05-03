// Copyright HorrorProject. All Rights Reserved.

#include "Game/HorrorDirectorSubsystem.h"
#include "Game/HorrorEncounterDirector.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Audio/HorrorAudioSubsystem.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HorrorProject.h"

void UHorrorDirectorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	TensionValue = 0.0f;
	CurrentTensionLevel = ETensionLevel::Silent;
	DirectorState = EHorrorDirectorState::Idle;
}

void UHorrorDirectorSubsystem::Deinitialize()
{
	GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
	RegisteredEncounters.Empty();
	ActiveEncounter = nullptr;
	Super::Deinitialize();
}

void UHorrorDirectorSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// Subscribe to EventBus for encounter resolution and other events
	if (UHorrorEventBusSubsystem* EventBus = InWorld.GetSubsystem<UHorrorEventBusSubsystem>())
	{
		EventBus->GetOnEventPublishedNative().AddUObject(this, &UHorrorDirectorSubsystem::HandleEventBusEvent);
	}

	if (bDirectorEnabled)
	{
		GetWorld()->GetTimerManager().SetTimer(UpdateTimerHandle, [this]()
		{
			if (UWorld* World = GetWorld())
			{
				UpdateTension(World->GetDeltaSeconds());
				UpdateDirectorState(World->GetDeltaSeconds());
				UpdateAmbientAudio();
			}
		}, 0.1f, true);
	}
}

void UHorrorDirectorSubsystem::SetDirectorEnabled(bool bEnabled)
{
	bDirectorEnabled = bEnabled;
	UWorld* World = GetWorld();
	if (!bEnabled)
	{
		if (World)
		{
			World->GetTimerManager().ClearTimer(UpdateTimerHandle);
		}
		DirectorState = EHorrorDirectorState::Idle;
		OnDirectorStateChanged.Broadcast(DirectorState, TensionValue);
	}
	else if (World)
	{
		World->GetTimerManager().SetTimer(UpdateTimerHandle, [this]()
		{
			if (UWorld* World = GetWorld())
			{
				UpdateTension(World->GetDeltaSeconds());
				UpdateDirectorState(World->GetDeltaSeconds());
				UpdateAmbientAudio();
			}
		}, 0.1f, true);
	}
}

void UHorrorDirectorSubsystem::AddTension(float Amount, FName SourceId)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	const float OldTension = TensionValue;
	TensionValue = FMath::Clamp(TensionValue + Amount, 0.0f, 1.0f);

	if (!FMath::IsNearlyEqual(TensionValue, OldTension))
	{
		ETensionLevel NewLevel = ETensionLevel::Silent;
		if (TensionValue >= TensionCurve.DreadMax)
		{
			NewLevel = ETensionLevel::Peak;
		}
		else if (TensionValue >= TensionCurve.TenseMax)
		{
			NewLevel = ETensionLevel::Dread;
		}
		else if (TensionValue >= TensionCurve.UneasyMax)
		{
			NewLevel = ETensionLevel::Tense;
		}
		else if (TensionValue >= TensionCurve.SilentMax)
		{
			NewLevel = ETensionLevel::Uneasy;
		}

		if (NewLevel != CurrentTensionLevel)
		{
			CurrentTensionLevel = NewLevel;
			OnTensionChanged.Broadcast(CurrentTensionLevel, TensionValue);
		}

		if (DirectorState == EHorrorDirectorState::Idle && TensionValue > TensionCurve.SilentMax)
		{
			DirectorState = EHorrorDirectorState::BuildingTension;
			OnDirectorStateChanged.Broadcast(DirectorState, TensionValue);
		}
	}
}

void UHorrorDirectorSubsystem::ReduceTension(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	TensionValue = FMath::Max(0.0f, TensionValue - Amount);
}

void UHorrorDirectorSubsystem::SetTension(float NewTension)
{
	TensionValue = FMath::Clamp(NewTension, 0.0f, 1.0f);
}

void UHorrorDirectorSubsystem::RegisterEncounter(AHorrorEncounterDirector* Encounter)
{
	if (!Encounter)
	{
		return;
	}

	RegisteredEncounters.AddUnique(Encounter);
}

void UHorrorDirectorSubsystem::UnregisterEncounter(AHorrorEncounterDirector* Encounter)
{
	RegisteredEncounters.Remove(Encounter);
	if (ActiveEncounter.Get() == Encounter)
	{
		ActiveEncounter = nullptr;
	}
}

void UHorrorDirectorSubsystem::OnPlayerHealthChanged(float HealthPercent)
{
	PlayerHealthPercent = HealthPercent;
	if (HealthPercent < 0.3f)
	{
		AddTension(0.1f, TEXT("LowHealth"));
	}
}

void UHorrorDirectorSubsystem::OnPlayerSanityChanged(float SanityPercent)
{
	PlayerSanityPercent = SanityPercent;
	if (SanityPercent < 0.3f)
	{
		AddTension(0.08f, TEXT("LowSanity"));
	}
}

void UHorrorDirectorSubsystem::OnPlayerNoiseGenerated(float NoiseLevel)
{
	LastNoiseLevel = NoiseLevel;
	if (NoiseLevel > 0.7f)
	{
		AddTension(0.05f * NoiseLevel, TEXT("PlayerNoise"));
	}
}

void UHorrorDirectorSubsystem::OnEncounterResolved(FName EncounterId)
{
	if (ActiveEncounter.IsValid() && ActiveEncounter->GetEncounterId() == EncounterId)
	{
		ActiveEncounter = nullptr;
	}

	ReduceTension(0.2f);
	DirectorState = EHorrorDirectorState::CoolingDown;
	StateTimer = CooldownDuration;
	OnDirectorStateChanged.Broadcast(DirectorState, TensionValue);

	for (FHorrorDirectorEncounterWeight& Weight : EncounterWeights)
	{
		if (Weight.EncounterId == EncounterId)
		{
			Weight.TimesTriggered++;
			break;
		}
	}
}

void UHorrorDirectorSubsystem::ForceTriggerEncounter(FName EncounterId)
{
	for (const TWeakObjectPtr<AHorrorEncounterDirector>& Encounter : RegisteredEncounters)
	{
		if (Encounter.IsValid() && Encounter->GetEncounterId() == EncounterId)
		{
			Encounter->PrimeEncounter(EncounterId);
			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (PC && PC->GetPawn())
			{
				Encounter->TriggerReveal(PC->GetPawn());
			}
			ActiveEncounter = Encounter;
			break;
		}
	}
}

void UHorrorDirectorSubsystem::UpdateTension(float DeltaTime)
{
	switch (DirectorState)
	{
	case EHorrorDirectorState::BuildingTension:
		AddTension(TensionBuildRate * DeltaTime);
		if (TensionValue >= EncounterTriggerThreshold)
		{
			CheckEncounterTrigger();
		}
		break;

	case EHorrorDirectorState::Sustaining:
		StateTimer -= DeltaTime;
		if (StateTimer <= 0.0f)
		{
			DirectorState = EHorrorDirectorState::Releasing;
			OnDirectorStateChanged.Broadcast(DirectorState, TensionValue);
		}
		break;

	case EHorrorDirectorState::Releasing:
		TensionValue = FMath::Max(0.0f, TensionValue - TensionDecayRate * DeltaTime * 2.0f);
		if (TensionValue <= TensionCurve.SilentMax)
		{
			DirectorState = EHorrorDirectorState::CoolingDown;
			StateTimer = CooldownDuration;
			OnDirectorStateChanged.Broadcast(DirectorState, TensionValue);
		}
		break;

	case EHorrorDirectorState::CoolingDown:
		TensionValue = FMath::Max(0.0f, TensionValue - TensionDecayRate * DeltaTime);
		StateTimer -= DeltaTime;
		if (StateTimer <= 0.0f)
		{
			DirectorState = EHorrorDirectorState::Idle;
			OnDirectorStateChanged.Broadcast(DirectorState, TensionValue);
		}
		break;

	case EHorrorDirectorState::Idle:
		TensionValue = FMath::Max(0.0f, TensionValue - TensionDecayRate * DeltaTime * 0.5f);
		break;
	}

	EncounterCooldownTimer = FMath::Max(0.0f, EncounterCooldownTimer - DeltaTime);
}

void UHorrorDirectorSubsystem::UpdateDirectorState(float DeltaTime)
{
	if (DirectorState == EHorrorDirectorState::BuildingTension && TensionValue >= TensionCurve.DreadMax)
	{
		DirectorState = EHorrorDirectorState::Sustaining;
		StateTimer = SustainDuration;
		OnDirectorStateChanged.Broadcast(DirectorState, TensionValue);
	}
}

void UHorrorDirectorSubsystem::CheckEncounterTrigger()
{
	if (EncounterCooldownTimer > 0.0f || ActiveEncounter.IsValid())
	{
		return;
	}

	TriggerBestEncounter();
}

void UHorrorDirectorSubsystem::TriggerBestEncounter()
{
	AHorrorEncounterDirector* Best = SelectEncounterByWeight();
	if (!Best)
	{
		return;
	}

	Best->PrimeEncounter(Best->GetEncounterId());
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC && PC->GetPawn())
	{
		Best->TriggerReveal(PC->GetPawn());
	}

	ActiveEncounter = Best;
	EncounterCooldownTimer = EncounterCooldown;
	DirectorState = EHorrorDirectorState::Sustaining;
	StateTimer = SustainDuration;
	OnDirectorStateChanged.Broadcast(DirectorState, TensionValue);

	PublishDirectorEvent(TEXT("Director.EncounterTriggered"));
}

AHorrorEncounterDirector* UHorrorDirectorSubsystem::SelectEncounterByWeight() const
{
	if (RegisteredEncounters.Num() == 0)
	{
		return nullptr;
	}

	float TotalWeight = 0.0f;
	for (const TWeakObjectPtr<AHorrorEncounterDirector>& Encounter : RegisteredEncounters)
	{
		if (!Encounter.IsValid())
		{
			continue;
		}

		const FName EncounterId = Encounter->GetEncounterId();
		float Weight = 1.0f;
		for (const FHorrorDirectorEncounterWeight& Entry : EncounterWeights)
		{
			if (Entry.EncounterId == EncounterId)
			{
				if (Entry.TimesTriggered >= Entry.MaxTriggerCount)
				{
					Weight = 0.0f;
				}
				else
				{
					Weight = Entry.BaseWeight;
				}
				break;
			}
		}
		TotalWeight += Weight;
	}

	if (TotalWeight <= 0.0f)
	{
		return nullptr;
	}

	float Roll = FMath::RandRange(0.0f, TotalWeight);
	for (const TWeakObjectPtr<AHorrorEncounterDirector>& Encounter : RegisteredEncounters)
	{
		if (!Encounter.IsValid())
		{
			continue;
		}

		const FName EncounterId = Encounter->GetEncounterId();
		float Weight = 1.0f;
		for (const FHorrorDirectorEncounterWeight& Entry : EncounterWeights)
		{
			if (Entry.EncounterId == EncounterId)
			{
				if (Entry.TimesTriggered >= Entry.MaxTriggerCount)
				{
					Weight = 0.0f;
				}
				else
				{
					Weight = Entry.BaseWeight;
				}
				break;
			}
		}

		Roll -= Weight;
		if (Roll <= 0.0f)
		{
			return Encounter.Get();
		}
	}

	return nullptr;
}

void UHorrorDirectorSubsystem::UpdateAmbientAudio()
{
	if (!AmbientTensionSound)
	{
		return;
	}

	UHorrorAudioSubsystem* AudioSys = GetWorld()->GetSubsystem<UHorrorAudioSubsystem>();
	if (!AudioSys)
	{
		return;
	}

	const float TargetVolume = TensionValue * MaxAmbientVolume;
	AudioSys->SetCategoryVolume(EHorrorAudioCategory::Ambient, TargetVolume);
}

void UHorrorDirectorSubsystem::PublishDirectorEvent(FName EventName)
{
	UHorrorEventBusSubsystem* EventBus = GetWorld()->GetSubsystem<UHorrorEventBusSubsystem>();
	if (!EventBus)
	{
		return;
	}

	const FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(EventName, false);
	if (EventTag.IsValid())
	{
		EventBus->Publish(EventTag, TEXT("HorrorDirector"), FGameplayTag::EmptyTag, this);
	}
}

void UHorrorDirectorSubsystem::HandleEventBusEvent(const FHorrorEventMessage& Message)
{
	if (!bDirectorEnabled)
	{
		return;
	}

	// React to encounter resolution
	if (Message.EventTag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Encounter.Resolved"), false)))
	{
		OnEncounterResolved(Message.SourceId);
	}

	// React to anomaly events
	if (Message.EventTag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Anomaly.Triggered"), false)))
	{
		AddTension(0.15f, TEXT("AnomalyTriggered"));
	}

	if (Message.EventTag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Anomaly.Escalated"), false)))
	{
		AddTension(0.1f, TEXT("AnomalyEscalated"));
	}
}
