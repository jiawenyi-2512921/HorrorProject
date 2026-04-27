// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DebugVisualization.generated.h"

/**
 * Component for runtime debug visualization
 */
UCLASS(ClassGroup=(Debug), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UDebugVisualization : public UActorComponent
{
	GENERATED_BODY()

public:
	UDebugVisualization();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Toggle Functions
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void TogglePlayerInfo();

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void ToggleEnemyInfo();

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void ToggleSanityInfo();

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void TogglePerformanceInfo();

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void ToggleCollisionShapes();

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void ToggleNavigation();

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void ToggleAudioSources();

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void ToggleLightSources();

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void SetAllVisualization(bool bEnabled);

protected:
	// Draw Functions
	void DrawPlayerDebugInfo();
	void DrawEnemyDebugInfo();
	void DrawSanityDebugInfo();
	void DrawPerformanceDebugInfo();
	void DrawCollisionDebugShapes();
	void DrawNavigationDebugInfo();
	void DrawAudioDebugInfo();
	void DrawLightDebugInfo();

	// Visualization Flags
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowPlayerInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowEnemyInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowSanityInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowPerformanceInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowCollisionShapes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowNavigation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowAudioSources;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowLightSources;
};
