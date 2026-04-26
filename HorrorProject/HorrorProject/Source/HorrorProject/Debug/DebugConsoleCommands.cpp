// Copyright Epic Games, Inc. All Rights Reserved.

#include "DebugConsoleCommands.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

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
	GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("show COLLISION %d"), bShow ? 1 : 0));
	UE_LOG(LogTemp, Log, TEXT("Debug Collision: %s"), bShow ? TEXT("ON") : TEXT("OFF"));
}

void UDebugConsoleCommands::DebugShowNavMesh(bool bShow)
{
	bShowNavMesh = bShow;
	GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("show Navigation %d"), bShow ? 1 : 0));
	UE_LOG(LogTemp, Log, TEXT("Debug NavMesh: %s"), bShow ? TEXT("ON") : TEXT("OFF"));
}

void UDebugConsoleCommands::DebugShowAI(bool bShow)
{
	bShowAI = bShow;
	GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("showdebug AI %d"), bShow ? 1 : 0));
	UE_LOG(LogTemp, Log, TEXT("Debug AI: %s"), bShow ? TEXT("ON") : TEXT("OFF"));
}

void UDebugConsoleCommands::DebugShowAudio(bool bShow)
{
	bShowAudio = bShow;
	GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("au.Debug.Sounds %d"), bShow ? 1 : 0));
	UE_LOG(LogTemp, Log, TEXT("Debug Audio: %s"), bShow ? TEXT("ON") : TEXT("OFF"));
}

void UDebugConsoleCommands::DebugShowPerformance(bool bShow)
{
	bShowPerformance = bShow;
	GetWorld()->Exec(GetWorld(), TEXT("stat fps"));
	GetWorld()->Exec(GetWorld(), TEXT("stat unit"));
	UE_LOG(LogTemp, Log, TEXT("Debug Performance: %s"), bShow ? TEXT("ON") : TEXT("OFF"));
}

void UDebugConsoleCommands::DebugFPS(bool bShow)
{
	GetWorld()->Exec(GetWorld(), TEXT("stat fps"));
	UE_LOG(LogTemp, Log, TEXT("FPS Display: %s"), bShow ? TEXT("ON") : TEXT("OFF"));
}

void UDebugConsoleCommands::DebugMemory()
{
	GetWorld()->Exec(GetWorld(), TEXT("stat memory"));
	GetWorld()->Exec(GetWorld(), TEXT("memreport -full"));
	UE_LOG(LogTemp, Log, TEXT("Memory stats displayed"));
}

void UDebugConsoleCommands::DebugGPU()
{
	GetWorld()->Exec(GetWorld(), TEXT("stat gpu"));
	GetWorld()->Exec(GetWorld(), TEXT("profilegpu"));
	UE_LOG(LogTemp, Log, TEXT("GPU stats displayed"));
}

void UDebugConsoleCommands::DebugDrawCalls()
{
	GetWorld()->Exec(GetWorld(), TEXT("stat rhi"));
	GetWorld()->Exec(GetWorld(), TEXT("stat scenerendering"));
	UE_LOG(LogTemp, Log, TEXT("Draw call stats displayed"));
}

void UDebugConsoleCommands::DebugGodMode(bool bEnable)
{
	bGodModeEnabled = bEnable;
	if (ACharacter* Character = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
	{
		Character->SetActorEnableCollision(!bEnable);
	}
	UE_LOG(LogTemp, Warning, TEXT("God Mode: %s"), bEnable ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UDebugConsoleCommands::DebugNoClip(bool bEnable)
{
	bNoClipEnabled = bEnable;
	if (ACharacter* Character = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
	{
		if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			Movement->SetMovementMode(bEnable ? MOVE_Flying : MOVE_Walking);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("NoClip Mode: %s"), bEnable ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UDebugConsoleCommands::DebugTeleport(float X, float Y, float Z)
{
	if (ACharacter* Character = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
	{
		FVector NewLocation(X, Y, Z);
		Character->SetActorLocation(NewLocation);
		UE_LOG(LogTemp, Log, TEXT("Teleported to: %s"), *NewLocation.ToString());
	}
}

void UDebugConsoleCommands::DebugSpawnEnemy(FString EnemyType)
{
	if (ACharacter* Character = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
	{
		FVector SpawnLocation = Character->GetActorLocation() + Character->GetActorForwardVector() * 500.0f;
		UE_LOG(LogTemp, Log, TEXT("Spawning enemy: %s at %s"), *EnemyType, *SpawnLocation.ToString());
		// TODO: Implement enemy spawning based on type
	}
}

void UDebugConsoleCommands::DebugKillAllEnemies()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		Actor->Destroy();
	}

	UE_LOG(LogTemp, Warning, TEXT("Killed %d enemies"), FoundActors.Num());
}

void UDebugConsoleCommands::DebugSetSanity(float Value)
{
	Value = FMath::Clamp(Value, 0.0f, 100.0f);
	UE_LOG(LogTemp, Log, TEXT("Setting sanity to: %.2f"), Value);
	// TODO: Set sanity value in horror system
}

void UDebugConsoleCommands::DebugSetFear(float Value)
{
	Value = FMath::Clamp(Value, 0.0f, 100.0f);
	UE_LOG(LogTemp, Log, TEXT("Setting fear to: %.2f"), Value);
	// TODO: Set fear value in horror system
}

void UDebugConsoleCommands::DebugTriggerHallucination()
{
	UE_LOG(LogTemp, Warning, TEXT("Triggering hallucination"));
	// TODO: Trigger hallucination in horror system
}

void UDebugConsoleCommands::DebugShowSanityInfo()
{
	UE_LOG(LogTemp, Log, TEXT("=== Sanity System Info ==="));
	// TODO: Display sanity system information
}

void UDebugConsoleCommands::DebugShowAllEvidence()
{
	UE_LOG(LogTemp, Log, TEXT("=== All Evidence ==="));
	// TODO: Display all evidence in the level
}

void UDebugConsoleCommands::DebugCollectAllEvidence()
{
	UE_LOG(LogTemp, Warning, TEXT("Collecting all evidence"));
	// TODO: Collect all evidence
}

void UDebugConsoleCommands::DebugResetEvidence()
{
	UE_LOG(LogTemp, Warning, TEXT("Resetting evidence"));
	// TODO: Reset evidence collection
}

void UDebugConsoleCommands::DebugSaveGame(FString SlotName)
{
	UE_LOG(LogTemp, Log, TEXT("Saving game to slot: %s"), *SlotName);
	// TODO: Save game to specified slot
}

void UDebugConsoleCommands::DebugLoadGame(FString SlotName)
{
	UE_LOG(LogTemp, Log, TEXT("Loading game from slot: %s"), *SlotName);
	// TODO: Load game from specified slot
}

void UDebugConsoleCommands::DebugDeleteSave(FString SlotName)
{
	UE_LOG(LogTemp, Warning, TEXT("Deleting save slot: %s"), *SlotName);
	// TODO: Delete save slot
}

void UDebugConsoleCommands::DebugSetLogLevel(FString Category, FString Level)
{
	UE_LOG(LogTemp, Log, TEXT("Setting log level for %s to %s"), *Category, *Level);
	FString Command = FString::Printf(TEXT("log %s %s"), *Category, *Level);
	GetWorld()->Exec(GetWorld(), *Command);
}

void UDebugConsoleCommands::DebugDumpGameState()
{
	UE_LOG(LogTemp, Log, TEXT("=== Game State Dump ==="));
	UE_LOG(LogTemp, Log, TEXT("World: %s"), *GetWorld()->GetName());
	UE_LOG(LogTemp, Log, TEXT("Time: %.2f"), GetWorld()->GetTimeSeconds());
	UE_LOG(LogTemp, Log, TEXT("Actors: %d"), GetWorld()->GetActorCount());
}

void UDebugConsoleCommands::DebugDumpActors()
{
	UE_LOG(LogTemp, Log, TEXT("=== Actor Dump ==="));
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		UE_LOG(LogTemp, Log, TEXT("Actor: %s at %s"), *Actor->GetName(), *Actor->GetActorLocation().ToString());
	}
}

void UDebugConsoleCommands::DebugStartProfiling()
{
	GetWorld()->Exec(GetWorld(), TEXT("stat startfile"));
	UE_LOG(LogTemp, Warning, TEXT("Started profiling"));
}

void UDebugConsoleCommands::DebugStopProfiling()
{
	GetWorld()->Exec(GetWorld(), TEXT("stat stopfile"));
	UE_LOG(LogTemp, Warning, TEXT("Stopped profiling"));
}

void UDebugConsoleCommands::DebugCaptureFrame()
{
	GetWorld()->Exec(GetWorld(), TEXT("profilegpu"));
	UE_LOG(LogTemp, Log, TEXT("Captured frame profile"));
}
