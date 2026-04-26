// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "DebugConsoleCommands.generated.h"

/**
 * Debug console commands for runtime debugging
 */
UCLASS()
class HORRORPROJECT_API UDebugConsoleCommands : public UCheatManager
{
	GENERATED_BODY()

public:
	UDebugConsoleCommands();

	// Debug Visualization
	UFUNCTION(Exec)
	void DebugShowCollision(bool bShow);

	UFUNCTION(Exec)
	void DebugShowNavMesh(bool bShow);

	UFUNCTION(Exec)
	void DebugShowAI(bool bShow);

	UFUNCTION(Exec)
	void DebugShowAudio(bool bShow);

	UFUNCTION(Exec)
	void DebugShowPerformance(bool bShow);

	// Performance Commands
	UFUNCTION(Exec)
	void DebugFPS(bool bShow);

	UFUNCTION(Exec)
	void DebugMemory();

	UFUNCTION(Exec)
	void DebugGPU();

	UFUNCTION(Exec)
	void DebugDrawCalls();

	// Gameplay Commands
	UFUNCTION(Exec)
	void DebugGodMode(bool bEnable);

	UFUNCTION(Exec)
	void DebugNoClip(bool bEnable);

	UFUNCTION(Exec)
	void DebugTeleport(float X, float Y, float Z);

	UFUNCTION(Exec)
	void DebugSpawnEnemy(FString EnemyType);

	UFUNCTION(Exec)
	void DebugKillAllEnemies();

	// Horror System Commands
	UFUNCTION(Exec)
	void DebugSetSanity(float Value);

	UFUNCTION(Exec)
	void DebugSetFear(float Value);

	UFUNCTION(Exec)
	void DebugTriggerHallucination();

	UFUNCTION(Exec)
	void DebugShowSanityInfo();

	// Evidence Commands
	UFUNCTION(Exec)
	void DebugShowAllEvidence();

	UFUNCTION(Exec)
	void DebugCollectAllEvidence();

	UFUNCTION(Exec)
	void DebugResetEvidence();

	// Save/Load Commands
	UFUNCTION(Exec)
	void DebugSaveGame(FString SlotName);

	UFUNCTION(Exec)
	void DebugLoadGame(FString SlotName);

	UFUNCTION(Exec)
	void DebugDeleteSave(FString SlotName);

	// Logging Commands
	UFUNCTION(Exec)
	void DebugSetLogLevel(FString Category, FString Level);

	UFUNCTION(Exec)
	void DebugDumpGameState();

	UFUNCTION(Exec)
	void DebugDumpActors();

	// Profiling Commands
	UFUNCTION(Exec)
	void DebugStartProfiling();

	UFUNCTION(Exec)
	void DebugStopProfiling();

	UFUNCTION(Exec)
	void DebugCaptureFrame();

private:
	bool bGodModeEnabled;
	bool bNoClipEnabled;
	bool bShowCollision;
	bool bShowNavMesh;
	bool bShowAI;
	bool bShowAudio;
	bool bShowPerformance;
};
