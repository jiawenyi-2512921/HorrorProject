// Copyright Epic Games, Inc. All Rights Reserved.

#include "DebugConsoleCommands.h"
#include "AI/HorrorThreatCharacter.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Evidence/EvidenceActor.h"
#include "Evidence/EvidenceCollectionComponent.h"
#include "Misc/Paths.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/Components/FearComponent.h"
#include "Player/Components/InventoryComponent.h"
#include "Save/HorrorSaveGame.h"
#include "Save/HorrorSaveSubsystem.h"

namespace
{
	constexpr float MaxDebugTeleportCoordinate = 1000000.0f;

	UWorld* GetDebugWorld(const UObject* Context)
	{
		UWorld* World = Context ? Context->GetWorld() : nullptr;
		if (!World)
		{
			UE_LOG(LogTemp, Warning, TEXT("Debug command ignored: world is unavailable"));
		}
		return World;
	}

	bool ExecDebugCommand(const UObject* Context, const TCHAR* Command)
	{
		if (UWorld* World = GetDebugWorld(Context))
		{
			return World->Exec(World, Command);
		}
		return false;
	}

	AHorrorPlayerCharacter* GetDebugPlayerCharacter(const UObject* Context)
	{
		UWorld* World = GetDebugWorld(Context);
		return World ? Cast<AHorrorPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0)) : nullptr;
	}

	UFearComponent* GetDebugFearComponent(const UObject* Context)
	{
		if (AHorrorPlayerCharacter* PlayerCharacter = GetDebugPlayerCharacter(Context))
		{
			return PlayerCharacter->GetFearComponent();
		}

		UWorld* World = GetDebugWorld(Context);
		ACharacter* Character = World ? UGameplayStatics::GetPlayerCharacter(World, 0) : nullptr;
		return Character ? Character->FindComponentByClass<UFearComponent>() : nullptr;
	}

	UHorrorSaveSubsystem* GetDebugSaveSubsystem(const UObject* Context)
	{
		UWorld* World = GetDebugWorld(Context);
		UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		return GameInstance ? GameInstance->GetSubsystem<UHorrorSaveSubsystem>() : nullptr;
	}

	FString GetValidatedDebugSlotName(const FString& SlotName)
	{
		FString Trimmed = SlotName;
		Trimmed.TrimStartAndEndInline();
		if (Trimmed.IsEmpty())
		{
			Trimmed = TEXT("SM13_Day1_Autosave_Debug");
		}

		const FString Sanitized = FPaths::MakeValidFileName(Trimmed);
		return Sanitized.IsEmpty() ? FString(TEXT("SM13_Day1_Autosave_Debug")) : Sanitized;
	}
}

UDebugConsoleCommands::UDebugConsoleCommands()
	: bGodModeEnabled(false)
	, bNoClipEnabled(false)
	, bShowCollision(false)
	, bShowNavMesh(false)
	, bShowAI(false)
	, bShowAudio(false)
	, bShowPerformance(false)
{
}

void UDebugConsoleCommands::DebugShowCollision(bool bShow)
{
	bShowCollision = bShow;
	ExecDebugCommand(this, *FString::Printf(TEXT("show COLLISION %d"), bShow ? 1 : 0));
	UE_LOG(LogTemp, Log, TEXT("Debug Collision: %s"), bShow ? TEXT("ON") : TEXT("OFF"));
}

void UDebugConsoleCommands::DebugShowNavMesh(bool bShow)
{
	bShowNavMesh = bShow;
	ExecDebugCommand(this, *FString::Printf(TEXT("show Navigation %d"), bShow ? 1 : 0));
	UE_LOG(LogTemp, Log, TEXT("Debug NavMesh: %s"), bShow ? TEXT("ON") : TEXT("OFF"));
}

void UDebugConsoleCommands::DebugShowAI(bool bShow)
{
	bShowAI = bShow;
	ExecDebugCommand(this, *FString::Printf(TEXT("showdebug AI %d"), bShow ? 1 : 0));
	UE_LOG(LogTemp, Log, TEXT("Debug AI: %s"), bShow ? TEXT("ON") : TEXT("OFF"));
}

void UDebugConsoleCommands::DebugShowAudio(bool bShow)
{
	bShowAudio = bShow;
	ExecDebugCommand(this, *FString::Printf(TEXT("au.Debug.Sounds %d"), bShow ? 1 : 0));
	UE_LOG(LogTemp, Log, TEXT("Debug Audio: %s"), bShow ? TEXT("ON") : TEXT("OFF"));
}

void UDebugConsoleCommands::DebugShowPerformance(bool bShow)
{
	bShowPerformance = bShow;
	ExecDebugCommand(this, TEXT("stat fps"));
	ExecDebugCommand(this, TEXT("stat unit"));
	UE_LOG(LogTemp, Log, TEXT("Debug Performance: %s"), bShow ? TEXT("ON") : TEXT("OFF"));
}

void UDebugConsoleCommands::DebugFPS(bool bShow)
{
	ExecDebugCommand(this, TEXT("stat fps"));
	UE_LOG(LogTemp, Log, TEXT("FPS Display: %s"), bShow ? TEXT("ON") : TEXT("OFF"));
}

void UDebugConsoleCommands::DebugMemory()
{
	ExecDebugCommand(this, TEXT("stat memory"));
	ExecDebugCommand(this, TEXT("memreport -full"));
	UE_LOG(LogTemp, Log, TEXT("Memory stats displayed"));
}

void UDebugConsoleCommands::DebugGPU()
{
	ExecDebugCommand(this, TEXT("stat gpu"));
	ExecDebugCommand(this, TEXT("profilegpu"));
	UE_LOG(LogTemp, Log, TEXT("GPU stats displayed"));
}

void UDebugConsoleCommands::DebugDrawCalls()
{
	ExecDebugCommand(this, TEXT("stat rhi"));
	ExecDebugCommand(this, TEXT("stat scenerendering"));
	UE_LOG(LogTemp, Log, TEXT("Draw call stats displayed"));
}

void UDebugConsoleCommands::DebugGodMode(bool bEnable)
{
	bGodModeEnabled = bEnable;
	if (UWorld* World = GetDebugWorld(this))
	{
		if (ACharacter* Character = UGameplayStatics::GetPlayerCharacter(World, 0))
		{
			Character->SetActorEnableCollision(!bEnable);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("God Mode: %s"), bEnable ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UDebugConsoleCommands::DebugNoClip(bool bEnable)
{
	bNoClipEnabled = bEnable;
	if (UWorld* World = GetDebugWorld(this))
	{
		if (ACharacter* Character = UGameplayStatics::GetPlayerCharacter(World, 0))
		{
			if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
			{
				Movement->SetMovementMode(bEnable ? MOVE_Flying : MOVE_Walking);
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("NoClip Mode: %s"), bEnable ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UDebugConsoleCommands::DebugTeleport(float X, float Y, float Z)
{
#if !UE_BUILD_SHIPPING
	// Security: Validate coordinates
	if (FMath::IsNaN(X) || FMath::IsNaN(Y) || FMath::IsNaN(Z))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid teleport coordinates: NaN values"));
		return;
	}

	if (FMath::Abs(X) > MaxDebugTeleportCoordinate
		|| FMath::Abs(Y) > MaxDebugTeleportCoordinate
		|| FMath::Abs(Z) > MaxDebugTeleportCoordinate)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid teleport coordinates: out of bounds"));
		return;
	}

	if (UWorld* World = GetDebugWorld(this))
	{
		if (ACharacter* Character = UGameplayStatics::GetPlayerCharacter(World, 0))
		{
			FVector NewLocation(X, Y, Z);
			Character->SetActorLocation(NewLocation);
			UE_LOG(LogTemp, Log, TEXT("Teleported to: %s"), *NewLocation.ToString());
		}
	}
#endif
}

void UDebugConsoleCommands::DebugSpawnEnemy(FString EnemyType)
{
#if !UE_BUILD_SHIPPING
	if (UWorld* World = GetDebugWorld(this))
	{
		if (ACharacter* Character = UGameplayStatics::GetPlayerCharacter(World, 0))
		{
			const FVector SpawnLocation = Character->GetActorLocation() + Character->GetActorForwardVector() * 500.0f;
			const FRotator SpawnRotation = (Character->GetActorLocation() - SpawnLocation).Rotation();
			AHorrorThreatCharacter* SpawnedThreat = World->SpawnActor<AHorrorThreatCharacter>(
				AHorrorThreatCharacter::StaticClass(),
				SpawnLocation,
				SpawnRotation);

			if (SpawnedThreat)
			{
				SpawnedThreat->ThreatId = EnemyType.IsEmpty() ? FName(TEXT("Threat.Debug")) : FName(*EnemyType);
				SpawnedThreat->Tags.AddUnique(FName(TEXT("Enemy")));
				SpawnedThreat->ActivateThreat();
				SpawnedThreat->UpdateDetectedTarget(Character);
				UE_LOG(LogTemp, Log, TEXT("Spawned debug threat '%s' at %s"), *SpawnedThreat->ThreatId.ToString(), *SpawnLocation.ToString());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to spawn debug threat '%s'"), *EnemyType);
			}
		}
	}
#endif
}

void UDebugConsoleCommands::DebugKillAllEnemies()
{
	UWorld* World = GetDebugWorld(this);
	if (!World)
	{
		return;
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(World, FName("Enemy"), FoundActors);

	for (TActorIterator<AHorrorThreatCharacter> It(World); It; ++It)
	{
		FoundActors.AddUnique(*It);
	}

	int32 DestroyedCount = 0;
	for (AActor* Actor : FoundActors)
	{
		if (IsValid(Actor) && Actor->Destroy())
		{
			DestroyedCount++;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Killed %d enemies"), DestroyedCount);
}

void UDebugConsoleCommands::DebugSetSanity(float Value)
{
	Value = FMath::Clamp(Value, 0.0f, 100.0f);
	if (UFearComponent* FearComponent = GetDebugFearComponent(this))
	{
		const float InferredFear = 100.0f - Value;
		FearComponent->SetFearValue(InferredFear);
		UE_LOG(LogTemp, Log, TEXT("Set inferred sanity to %.2f (fear %.2f)"), Value, InferredFear);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Unable to set sanity: no fear/sanity runtime component found"));
}

void UDebugConsoleCommands::DebugSetFear(float Value)
{
	Value = FMath::Clamp(Value, 0.0f, 100.0f);
	if (UFearComponent* FearComponent = GetDebugFearComponent(this))
	{
		FearComponent->SetFearValue(Value);
		UE_LOG(LogTemp, Log, TEXT("Set fear to: %.2f"), Value);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Unable to set fear: no fear component found"));
}

void UDebugConsoleCommands::DebugTriggerHallucination()
{
	DebugSetFear(100.0f);
	ExecDebugCommand(this, TEXT("CameraShake 1"));
	UE_LOG(LogTemp, Warning, TEXT("Triggered debug hallucination surrogate by maxing fear"));
}

void UDebugConsoleCommands::DebugShowSanityInfo()
{
	UE_LOG(LogTemp, Log, TEXT("=== Sanity System Info ==="));
	if (UFearComponent* FearComponent = GetDebugFearComponent(this))
	{
		const float Fear = FearComponent->GetFearValue();
		const float FearPercent = FearComponent->GetFearPercent();
		const float InferredSanity = 100.0f - (FearPercent * 100.0f);
		UE_LOG(LogTemp, Log, TEXT("Fear: %.2f (%.0f%%), Inferred Sanity: %.2f, Fear Level: %d"),
			Fear,
			FearPercent * 100.0f,
			InferredSanity,
			static_cast<int32>(FearComponent->GetFearLevel()));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("No fear/sanity runtime component found"));
}

void UDebugConsoleCommands::DebugShowAllEvidence()
{
	UE_LOG(LogTemp, Log, TEXT("=== All Evidence ==="));
	UWorld* World = GetDebugWorld(this);
	if (!World)
	{
		return;
	}

	int32 EvidenceCount = 0;
	for (TActorIterator<AEvidenceActor> It(World); It; ++It)
	{
		const AEvidenceActor* EvidenceActor = *It;
		if (!IsValid(EvidenceActor))
		{
			continue;
		}

		const FExtendedEvidenceMetadata& Metadata = EvidenceActor->GetEvidenceMetadata();
		UE_LOG(LogTemp, Log, TEXT("Evidence[%d] Id=%s Name='%s' Collected=%s Location=%s"),
			EvidenceCount,
			*Metadata.EvidenceId.ToString(),
			*Metadata.DisplayName.ToString(),
			EvidenceActor->IsCollected() ? TEXT("true") : TEXT("false"),
			*EvidenceActor->GetActorLocation().ToString());
		EvidenceCount++;
	}

	UE_LOG(LogTemp, Log, TEXT("Evidence actors in level: %d"), EvidenceCount);
}

void UDebugConsoleCommands::DebugCollectAllEvidence()
{
	UE_LOG(LogTemp, Warning, TEXT("Collecting all evidence"));
	UWorld* World = GetDebugWorld(this);
	AHorrorPlayerCharacter* PlayerCharacter = GetDebugPlayerCharacter(this);
	if (!World || !PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to collect evidence: player character unavailable"));
		return;
	}

	if (!PlayerCharacter->GetEvidenceCollectionComponent())
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to collect evidence: player evidence collection component unavailable"));
		return;
	}

	TArray<AEvidenceActor*> EvidenceActors;
	for (TActorIterator<AEvidenceActor> It(World); It; ++It)
	{
		if (AEvidenceActor* EvidenceActor = *It)
		{
			EvidenceActors.Add(EvidenceActor);
		}
	}

	int32 CollectedCount = 0;
	const FHitResult EmptyHit;
	for (AEvidenceActor* EvidenceActor : EvidenceActors)
	{
		if (IsValid(EvidenceActor) && !EvidenceActor->IsCollected())
		{
			CollectedCount += EvidenceActor->Interact_Implementation(PlayerCharacter, EmptyHit) ? 1 : 0;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Collected %d evidence actors"), CollectedCount);
}

void UDebugConsoleCommands::DebugResetEvidence()
{
	UE_LOG(LogTemp, Warning, TEXT("Resetting evidence"));
	UWorld* World = GetDebugWorld(this);
	AHorrorPlayerCharacter* PlayerCharacter = GetDebugPlayerCharacter(this);
	if (!World)
	{
		return;
	}

	if (PlayerCharacter && PlayerCharacter->GetInventoryComponent())
	{
		PlayerCharacter->GetInventoryComponent()->ImportCollectedEvidenceIds(TArray<FName>());
	}

	int32 ResetCount = 0;
	for (TActorIterator<AEvidenceActor> It(World); It; ++It)
	{
		AEvidenceActor* EvidenceActor = *It;
		if (!IsValid(EvidenceActor))
		{
			continue;
		}

		EvidenceActor->SetCollected(false);
		EvidenceActor->SetActorHiddenInGame(false);
		EvidenceActor->SetActorEnableCollision(true);
		ResetCount++;
	}

	UE_LOG(LogTemp, Warning, TEXT("Reset %d evidence actors"), ResetCount);
}

void UDebugConsoleCommands::DebugSaveGame(FString SlotName)
{
	const FString ResolvedSlotName = GetValidatedDebugSlotName(SlotName);
	UHorrorSaveSubsystem* SaveSubsystem = GetDebugSaveSubsystem(this);
	if (!SaveSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to save game: save subsystem unavailable"));
		return;
	}

	UHorrorSaveGame* Snapshot = SaveSubsystem->CreateCheckpointSnapshot(this, FName(*ResolvedSlotName));
	if (!Snapshot)
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to save game: checkpoint snapshot creation failed"));
		return;
	}

	const bool bSaved = UGameplayStatics::SaveGameToSlot(Snapshot, ResolvedSlotName, 0);
	UE_LOG(LogTemp, Log, TEXT("Saving game to slot '%s': %s"), *ResolvedSlotName, bSaved ? TEXT("success") : TEXT("failed"));
}

void UDebugConsoleCommands::DebugLoadGame(FString SlotName)
{
	const FString ResolvedSlotName = GetValidatedDebugSlotName(SlotName);
	UHorrorSaveSubsystem* SaveSubsystem = GetDebugSaveSubsystem(this);
	if (!SaveSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to load game: save subsystem unavailable"));
		return;
	}

	UHorrorSaveGame* Snapshot = Cast<UHorrorSaveGame>(UGameplayStatics::LoadGameFromSlot(ResolvedSlotName, 0));
	if (!Snapshot)
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to load game: slot '%s' is empty or invalid"), *ResolvedSlotName);
		return;
	}

	const bool bLoaded = SaveSubsystem->ApplyCheckpointSnapshot(this, Snapshot);
	UE_LOG(LogTemp, Log, TEXT("Loading game from slot '%s': %s"), *ResolvedSlotName, bLoaded ? TEXT("success") : TEXT("failed"));
}

void UDebugConsoleCommands::DebugDeleteSave(FString SlotName)
{
	const FString ResolvedSlotName = GetValidatedDebugSlotName(SlotName);
	if (!UGameplayStatics::DoesSaveGameExist(ResolvedSlotName, 0))
	{
		UE_LOG(LogTemp, Warning, TEXT("Save slot '%s' does not exist"), *ResolvedSlotName);
		return;
	}

	const bool bDeleted = UGameplayStatics::DeleteGameInSlot(ResolvedSlotName, 0);
	UE_LOG(LogTemp, Warning, TEXT("Deleting save slot '%s': %s"), *ResolvedSlotName, bDeleted ? TEXT("success") : TEXT("failed"));
}

void UDebugConsoleCommands::DebugSetLogLevel(FString Category, FString Level)
{
	UE_LOG(LogTemp, Log, TEXT("Setting log level for %s to %s"), *Category, *Level);
	FString Command = FString::Printf(TEXT("log %s %s"), *Category, *Level);
	ExecDebugCommand(this, *Command);
}

void UDebugConsoleCommands::DebugDumpGameState()
{
	UWorld* World = GetDebugWorld(this);
	if (!World)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("=== Game State Dump ==="));
	UE_LOG(LogTemp, Log, TEXT("World: %s"), *World->GetName());
	UE_LOG(LogTemp, Log, TEXT("Time: %.2f"), World->GetTimeSeconds());
	UE_LOG(LogTemp, Log, TEXT("Actors: %d"), World->GetActorCount());
}

void UDebugConsoleCommands::DebugDumpActors()
{
	UWorld* World = GetDebugWorld(this);
	if (!World)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("=== Actor Dump ==="));
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		UE_LOG(LogTemp, Log, TEXT("Actor: %s at %s"), *Actor->GetName(), *Actor->GetActorLocation().ToString());
	}
}

void UDebugConsoleCommands::DebugStartProfiling()
{
	ExecDebugCommand(this, TEXT("stat startfile"));
	UE_LOG(LogTemp, Warning, TEXT("Started profiling"));
}

void UDebugConsoleCommands::DebugStopProfiling()
{
	ExecDebugCommand(this, TEXT("stat stopfile"));
	UE_LOG(LogTemp, Warning, TEXT("Stopped profiling"));
}

void UDebugConsoleCommands::DebugCaptureFrame()
{
	ExecDebugCommand(this, TEXT("profilegpu"));
	UE_LOG(LogTemp, Log, TEXT("Captured frame profile"));
}
