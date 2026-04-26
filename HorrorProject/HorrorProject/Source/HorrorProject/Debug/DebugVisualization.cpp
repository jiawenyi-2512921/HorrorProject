// Copyright Epic Games, Inc. All Rights Reserved.

#include "DebugVisualization.h"
#include "DrawDebugHelpers.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

UDebugVisualization::UDebugVisualization()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	bShowPlayerInfo = false;
	bShowEnemyInfo = false;
	bShowSanityInfo = false;
	bShowPerformanceInfo = false;
	bShowCollisionShapes = false;
	bShowNavigation = false;
	bShowAudioSources = false;
	bShowLightSources = false;
}

void UDebugVisualization::BeginPlay()
{
	Super::BeginPlay();
}

void UDebugVisualization::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bShowPlayerInfo)
	{
		DrawPlayerDebugInfo();
	}

	if (bShowEnemyInfo)
	{
		DrawEnemyDebugInfo();
	}

	if (bShowSanityInfo)
	{
		DrawSanityDebugInfo();
	}

	if (bShowCollisionShapes)
	{
		DrawCollisionDebugShapes();
	}

	if (bShowNavigation)
	{
		DrawNavigationDebugInfo();
	}

	if (bShowAudioSources)
	{
		DrawAudioDebugInfo();
	}

	if (bShowLightSources)
	{
		DrawLightDebugInfo();
	}
}

void UDebugVisualization::DrawPlayerDebugInfo()
{
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerCharacter) return;

	FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	FRotator PlayerRotation = PlayerCharacter->GetActorRotation();
	FVector PlayerVelocity = PlayerCharacter->GetVelocity();

	// Draw player position sphere
	DrawDebugSphere(GetWorld(), PlayerLocation, 50.0f, 12, FColor::Green, false, -1.0f, 0, 2.0f);

	// Draw forward vector
	DrawDebugDirectionalArrow(GetWorld(), PlayerLocation,
		PlayerLocation + PlayerCharacter->GetActorForwardVector() * 200.0f,
		50.0f, FColor::Red, false, -1.0f, 0, 3.0f);

	// Draw velocity vector
	if (!PlayerVelocity.IsNearlyZero())
	{
		DrawDebugDirectionalArrow(GetWorld(), PlayerLocation,
			PlayerLocation + PlayerVelocity,
			30.0f, FColor::Yellow, false, -1.0f, 0, 2.0f);
	}

	// Draw text info
	FString DebugText = FString::Printf(TEXT("Player\nPos: %s\nRot: %s\nVel: %.2f"),
		*PlayerLocation.ToCompactString(),
		*PlayerRotation.ToCompactString(),
		PlayerVelocity.Size());

	DrawDebugString(GetWorld(), PlayerLocation + FVector(0, 0, 150), DebugText,
		nullptr, FColor::White, 0.0f, true, 1.2f);
}

void UDebugVisualization::DrawEnemyDebugInfo()
{
	TArray<AActor*> EnemyActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), EnemyActors);

	for (AActor* Enemy : EnemyActors)
	{
		if (!Enemy) continue;

		FVector EnemyLocation = Enemy->GetActorLocation();

		// Draw enemy sphere
		DrawDebugSphere(GetWorld(), EnemyLocation, 60.0f, 12, FColor::Red, false, -1.0f, 0, 2.0f);

		// Draw forward direction
		DrawDebugDirectionalArrow(GetWorld(), EnemyLocation,
			EnemyLocation + Enemy->GetActorForwardVector() * 150.0f,
			40.0f, FColor::Orange, false, -1.0f, 0, 2.0f);

		// Draw detection radius
		DrawDebugCircle(GetWorld(), EnemyLocation, 1000.0f, 32, FColor::Yellow, false, -1.0f, 0, 2.0f,
			FVector(0, 1, 0), FVector(1, 0, 0), false);

		// Draw text info
		FString DebugText = FString::Printf(TEXT("Enemy: %s\nPos: %s"),
			*Enemy->GetName(),
			*EnemyLocation.ToCompactString());

		DrawDebugString(GetWorld(), EnemyLocation + FVector(0, 0, 120), DebugText,
			nullptr, FColor::Red, 0.0f, true, 1.0f);
	}
}

void UDebugVisualization::DrawSanityDebugInfo()
{
	// TODO: Get sanity values from horror system
	float CurrentSanity = 75.0f;
	float CurrentFear = 30.0f;

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerCharacter) return;

	FVector PlayerLocation = PlayerCharacter->GetActorLocation();

	// Draw sanity bar
	FVector BarStart = PlayerLocation + FVector(0, 0, 200);
	FVector BarEnd = BarStart + FVector(100, 0, 0);

	DrawDebugLine(GetWorld(), BarStart, BarEnd, FColor::White, false, -1.0f, 0, 3.0f);

	FVector SanityEnd = FMath::Lerp(BarStart, BarEnd, CurrentSanity / 100.0f);
	FColor SanityColor = FColor::MakeRedToGreenColorFromScalar(CurrentSanity / 100.0f);
	DrawDebugLine(GetWorld(), BarStart, SanityEnd, SanityColor, false, -1.0f, 0, 5.0f);

	// Draw fear indicator
	FVector FearStart = PlayerLocation + FVector(0, 0, 180);
	FVector FearEnd = FearStart + FVector(100, 0, 0);

	DrawDebugLine(GetWorld(), FearStart, FearEnd, FColor::White, false, -1.0f, 0, 3.0f);

	FVector FearLevelEnd = FMath::Lerp(FearStart, FearEnd, CurrentFear / 100.0f);
	DrawDebugLine(GetWorld(), FearStart, FearLevelEnd, FColor::Purple, false, -1.0f, 0, 5.0f);

	// Draw text
	FString SanityText = FString::Printf(TEXT("Sanity: %.1f%%\nFear: %.1f%%"),
		CurrentSanity, CurrentFear);
	DrawDebugString(GetWorld(), PlayerLocation + FVector(0, 0, 220), SanityText,
		nullptr, FColor::Cyan, 0.0f, true, 1.2f);
}

void UDebugVisualization::DrawCollisionDebugShapes()
{
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor) continue;

		TArray<UPrimitiveComponent*> Components;
		Actor->GetComponents<UPrimitiveComponent>(Components);

		for (UPrimitiveComponent* Component : Components)
		{
			if (!Component || !Component->IsCollisionEnabled()) continue;

			FVector Location = Component->GetComponentLocation();
			FBoxSphereBounds Bounds = Component->Bounds;

			// Draw bounding box
			DrawDebugBox(GetWorld(), Location, Bounds.BoxExtent,
				Component->GetComponentQuat(), FColor::Cyan, false, -1.0f, 0, 1.0f);
		}
	}
}

void UDebugVisualization::DrawNavigationDebugInfo()
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys) return;

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerCharacter) return;

	FVector PlayerLocation = PlayerCharacter->GetActorLocation();

	// Draw nav mesh around player
	FNavLocation NavLocation;
	if (NavSys->ProjectPointToNavigation(PlayerLocation, NavLocation))
	{
		DrawDebugSphere(GetWorld(), NavLocation.Location, 30.0f, 12, FColor::Green, false, -1.0f, 0, 2.0f);
	}
}

void UDebugVisualization::DrawAudioDebugInfo()
{
	// TODO: Get audio sources from audio system
	// Draw audio source locations and attenuation radii
}

void UDebugVisualization::DrawLightDebugInfo()
{
	// TODO: Get light sources
	// Draw light locations and influence radii
}

void UDebugVisualization::TogglePlayerInfo()
{
	bShowPlayerInfo = !bShowPlayerInfo;
	UE_LOG(LogTemp, Log, TEXT("Player Debug Info: %s"), bShowPlayerInfo ? TEXT("ON") : TEXT("OFF"));
}

void UDebugVisualization::ToggleEnemyInfo()
{
	bShowEnemyInfo = !bShowEnemyInfo;
	UE_LOG(LogTemp, Log, TEXT("Enemy Debug Info: %s"), bShowEnemyInfo ? TEXT("ON") : TEXT("OFF"));
}

void UDebugVisualization::ToggleSanityInfo()
{
	bShowSanityInfo = !bShowSanityInfo;
	UE_LOG(LogTemp, Log, TEXT("Sanity Debug Info: %s"), bShowSanityInfo ? TEXT("ON") : TEXT("OFF"));
}

void UDebugVisualization::TogglePerformanceInfo()
{
	bShowPerformanceInfo = !bShowPerformanceInfo;
	UE_LOG(LogTemp, Log, TEXT("Performance Debug Info: %s"), bShowPerformanceInfo ? TEXT("ON") : TEXT("OFF"));
}

void UDebugVisualization::ToggleCollisionShapes()
{
	bShowCollisionShapes = !bShowCollisionShapes;
	UE_LOG(LogTemp, Log, TEXT("Collision Debug Shapes: %s"), bShowCollisionShapes ? TEXT("ON") : TEXT("OFF"));
}

void UDebugVisualization::ToggleNavigation()
{
	bShowNavigation = !bShowNavigation;
	UE_LOG(LogTemp, Log, TEXT("Navigation Debug: %s"), bShowNavigation ? TEXT("ON") : TEXT("OFF"));
}

void UDebugVisualization::ToggleAudioSources()
{
	bShowAudioSources = !bShowAudioSources;
	UE_LOG(LogTemp, Log, TEXT("Audio Debug: %s"), bShowAudioSources ? TEXT("ON") : TEXT("OFF"));
}

void UDebugVisualization::ToggleLightSources()
{
	bShowLightSources = !bShowLightSources;
	UE_LOG(LogTemp, Log, TEXT("Light Debug: %s"), bShowLightSources ? TEXT("ON") : TEXT("OFF"));
}

void UDebugVisualization::SetAllVisualization(bool bEnabled)
{
	bShowPlayerInfo = bEnabled;
	bShowEnemyInfo = bEnabled;
	bShowSanityInfo = bEnabled;
	bShowPerformanceInfo = bEnabled;
	bShowCollisionShapes = bEnabled;
	bShowNavigation = bEnabled;
	bShowAudioSources = bEnabled;
	bShowLightSources = bEnabled;

	UE_LOG(LogTemp, Warning, TEXT("All Debug Visualization: %s"), bEnabled ? TEXT("ON") : TEXT("OFF"));
}
