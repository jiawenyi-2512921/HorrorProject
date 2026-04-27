// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DebugHUD.generated.h"

/**
 * Debug HUD for displaying runtime information
 */
UCLASS()
class HORRORPROJECT_API ADebugHUD : public AHUD
{
	GENERATED_BODY()

public:
	ADebugHUD();

	virtual void DrawHUD() override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void ToggleDebugHUD();

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void SetDebugCategory(FString Category, bool bEnabled);

protected:
	void DrawPerformanceStats();
	void DrawMemoryStats();
	void DrawPlayerStats();
	void DrawGameplayStats();
	void DrawSystemInfo();
	void DrawFrameTiming();
	void DrawNetworkStats();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebugHUD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowPerformance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowMemory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowGameplay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowFrameTiming;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowNetwork;

	UPROPERTY(EditAnywhere, Category = "Debug")
	FLinearColor TextColor;

	UPROPERTY(EditAnywhere, Category = "Debug")
	FLinearColor BackgroundColor;

	UPROPERTY(EditAnywhere, Category = "Debug")
	float TextScale;

private:
	float CurrentYPosition;
	float LineHeight;
	float SectionSpacing;

	void DrawFearStats(UWorld* World);
	void DrawDebugText(const FString& Text, FLinearColor Color = FLinearColor::White);
	void DrawDebugSection(const FString& Title);
	void ResetDrawPosition();
};
