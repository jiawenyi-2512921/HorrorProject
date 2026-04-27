// Copyright Epic Games, Inc. All Rights Reserved.

#include "DebugVisualization.h"
#include "DrawDebugHelpers.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "EngineUtils.h"
#include "Components/AudioComponent.h"
#include "Components/LocalLightComponent.h"
#include "Player/Components/FearComponent.h"

namespace HorrorDebugVisualization
{
	constexpr float PlayerSphereRadiusCm = 50.0f;
	constexpr float PlayerForwardVectorLengthCm = 200.0f;
	constexpr float PlayerForwardArrowSizeCm = 50.0f;
	constexpr float VelocityArrowSizeCm = 30.0f;
	constexpr float PlayerTextHeightCm = 150.0f;
	constexpr float EnemySphereRadiusCm = 60.0f;
	constexpr float EnemyForwardVectorLengthCm = 150.0f;
	constexpr float EnemyForwardArrowSizeCm = 40.0f;
	constexpr float EnemyDetectionRadiusCm = 1000.0f;
	constexpr int32 DebugSphereSegments = 12;
	constexpr int32 DetectionCircleSegments = 32;
	constexpr float EnemyTextHeightCm = 120.0f;
	constexpr float FearUnavailableTextHeightCm = 220.0f;
	constexpr float SanityBarHeightCm = 200.0f;
	constexpr float FearBarHeightCm = 180.0f;
	constexpr float StatusBarLengthCm = 100.0f;
	constexpr float StatusTextHeightCm = 220.0f;
	constexpr float PercentMultiplier = 100.0f;
	constexpr float MillisecondsPerSecond = 1000.0f;
	constexpr float TargetFrameRate = 60.0f;
	constexpr float WarningFrameRate = 30.0f;
	constexpr float PerformanceTextHeightCm = 260.0f;
	constexpr float NavigationSphereRadiusCm = 30.0f;
	constexpr float AudioSphereRadiusCm = 120.0f;
	constexpr int32 AudioSphereSegments = 16;
	constexpr float ComponentLabelHeightCm = 80.0f;
	constexpr int32 LightSphereSegments = 24;
}

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

	if (bShowPerformanceInfo)
	{
		DrawPerformanceDebugInfo();
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
	DrawDebugSphere(
		GetWorld(),
		PlayerLocation,
		HorrorDebugVisualization::PlayerSphereRadiusCm,
		HorrorDebugVisualization::DebugSphereSegments,
		FColor::Green,
		false,
		-1.0f,
		0,
		2.0f);

	// Draw forward vector
	DrawDebugDirectionalArrow(GetWorld(), PlayerLocation,
		PlayerLocation + PlayerCharacter->GetActorForwardVector() * HorrorDebugVisualization::PlayerForwardVectorLengthCm,
		HorrorDebugVisualization::PlayerForwardArrowSizeCm, FColor::Red, false, -1.0f, 0, 3.0f);

	// Draw velocity vector
	if (!PlayerVelocity.IsNearlyZero())
	{
		DrawDebugDirectionalArrow(GetWorld(), PlayerLocation,
			PlayerLocation + PlayerVelocity,
			HorrorDebugVisualization::VelocityArrowSizeCm, FColor::Yellow, false, -1.0f, 0, 2.0f);
	}

	// Draw text info
	FString DebugText = FString::Printf(TEXT("Player\nPos: %s\nRot: %s\nVel: %.2f"),
		*PlayerLocation.ToCompactString(),
		*PlayerRotation.ToCompactString(),
		PlayerVelocity.Size());

	DrawDebugString(GetWorld(), PlayerLocation + FVector(0.0f, 0.0f, HorrorDebugVisualization::PlayerTextHeightCm), DebugText,
		nullptr, FColor::White, 0.0f, true, 1.2f);
}

void UDebugVisualization::DrawEnemyDebugInfo()
{
	TArray<AActor*> EnemyActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), EnemyActors);

	FString DebugText;
	for (AActor* Enemy : EnemyActors)
	{
		if (!Enemy) continue;

		FVector EnemyLocation = Enemy->GetActorLocation();

		// Draw enemy sphere
		DrawDebugSphere(
			GetWorld(),
			EnemyLocation,
			HorrorDebugVisualization::EnemySphereRadiusCm,
			HorrorDebugVisualization::DebugSphereSegments,
			FColor::Red,
			false,
			-1.0f,
			0,
			2.0f);

		// Draw forward direction
		DrawDebugDirectionalArrow(GetWorld(), EnemyLocation,
			EnemyLocation + Enemy->GetActorForwardVector() * HorrorDebugVisualization::EnemyForwardVectorLengthCm,
			HorrorDebugVisualization::EnemyForwardArrowSizeCm, FColor::Orange, false, -1.0f, 0, 2.0f);

		// Draw detection radius
		DrawDebugCircle(GetWorld(), EnemyLocation, HorrorDebugVisualization::EnemyDetectionRadiusCm, HorrorDebugVisualization::DetectionCircleSegments, FColor::Yellow, false, -1.0f, 0, 2.0f,
			FVector(0, 1, 0), FVector(1, 0, 0), false);

		// Draw text info
		DebugText.Reset();
		DebugText.Appendf(TEXT("Enemy: %s\nPos: %s"),
			*Enemy->GetName(),
			*EnemyLocation.ToCompactString());

		DrawDebugString(GetWorld(), EnemyLocation + FVector(0.0f, 0.0f, HorrorDebugVisualization::EnemyTextHeightCm), DebugText,
			nullptr, FColor::Red, 0.0f, true, 1.0f);
	}
}

void UDebugVisualization::DrawSanityDebugInfo()
{
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerCharacter) return;

	UFearComponent* FearComponent = PlayerCharacter->FindComponentByClass<UFearComponent>();
	if (!FearComponent)
	{
		DrawDebugString(GetWorld(), PlayerCharacter->GetActorLocation() + FVector(0.0f, 0.0f, HorrorDebugVisualization::FearUnavailableTextHeightCm),
			TEXT("Sanity/Fear component unavailable"), nullptr, FColor::Yellow, 0.0f, true, 1.2f);
		return;
	}

	FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	const float CurrentFear = FearComponent->GetFearPercent() * HorrorDebugVisualization::PercentMultiplier;
	const float CurrentSanity = HorrorDebugVisualization::PercentMultiplier - CurrentFear;

	// Draw sanity bar
	FVector BarStart = PlayerLocation + FVector(0.0f, 0.0f, HorrorDebugVisualization::SanityBarHeightCm);
	FVector BarEnd = BarStart + FVector(HorrorDebugVisualization::StatusBarLengthCm, 0.0f, 0.0f);

	DrawDebugLine(GetWorld(), BarStart, BarEnd, FColor::White, false, -1.0f, 0, 3.0f);

	FVector SanityEnd = FMath::Lerp(BarStart, BarEnd, CurrentSanity / HorrorDebugVisualization::PercentMultiplier);
	FColor SanityColor = FColor::MakeRedToGreenColorFromScalar(CurrentSanity / HorrorDebugVisualization::PercentMultiplier);
	DrawDebugLine(GetWorld(), BarStart, SanityEnd, SanityColor, false, -1.0f, 0, 5.0f);

	// Draw fear indicator
	FVector FearStart = PlayerLocation + FVector(0.0f, 0.0f, HorrorDebugVisualization::FearBarHeightCm);
	FVector FearEnd = FearStart + FVector(HorrorDebugVisualization::StatusBarLengthCm, 0.0f, 0.0f);

	DrawDebugLine(GetWorld(), FearStart, FearEnd, FColor::White, false, -1.0f, 0, 3.0f);

	FVector FearLevelEnd = FMath::Lerp(FearStart, FearEnd, CurrentFear / HorrorDebugVisualization::PercentMultiplier);
	DrawDebugLine(GetWorld(), FearStart, FearLevelEnd, FColor::Purple, false, -1.0f, 0, 5.0f);

	// Draw text
	FString SanityText = FString::Printf(TEXT("Sanity: %.1f%%\nFear: %.1f%%"),
		CurrentSanity, CurrentFear);
	DrawDebugString(GetWorld(), PlayerLocation + FVector(0.0f, 0.0f, HorrorDebugVisualization::StatusTextHeightCm), SanityText,
		nullptr, FColor::Cyan, 0.0f, true, 1.2f);
}

void UDebugVisualization::DrawPerformanceDebugInfo()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float DeltaSeconds = World->GetDeltaSeconds();
	const float FPS = DeltaSeconds > SMALL_NUMBER ? 1.0f / DeltaSeconds : 0.0f;
	const float FrameTimeMS = DeltaSeconds * HorrorDebugVisualization::MillisecondsPerSecond;
	const AActor* Owner = GetOwner();
	const FVector AnchorLocation = Owner
		? Owner->GetActorLocation() + FVector(0.0f, 0.0f, HorrorDebugVisualization::PerformanceTextHeightCm)
		: FVector::ZeroVector;

	const FString DebugText = FString::Printf(TEXT("Perf\nFPS: %.1f\nFrame: %.2f ms\nActors: %d"),
		FPS,
		FrameTimeMS,
		World->GetActorCount());
	const FColor PerfColor = FPS >= HorrorDebugVisualization::TargetFrameRate
		? FColor::Green
		: (FPS >= HorrorDebugVisualization::WarningFrameRate ? FColor::Yellow : FColor::Red);
	DrawDebugString(World, AnchorLocation, DebugText, nullptr, PerfColor, 0.0f, true, 1.1f);
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
		DrawDebugSphere(GetWorld(), NavLocation.Location, HorrorDebugVisualization::NavigationSphereRadiusCm, HorrorDebugVisualization::DebugSphereSegments, FColor::Green, false, -1.0f, 0, 2.0f);
	}
}

void UDebugVisualization::DrawAudioDebugInfo()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!IsValid(Actor))
		{
			continue;
		}

		TArray<UAudioComponent*> AudioComponents;
		Actor->GetComponents<UAudioComponent>(AudioComponents);
		for (UAudioComponent* AudioComponent : AudioComponents)
		{
			if (!AudioComponent || !AudioComponent->IsActive())
			{
				continue;
			}

			const FVector Location = AudioComponent->GetComponentLocation();
			DrawDebugSphere(World, Location, HorrorDebugVisualization::AudioSphereRadiusCm, HorrorDebugVisualization::AudioSphereSegments, FColor::Blue, false, -1.0f, 0, 1.5f);
			DrawDebugString(World, Location + FVector(0.0f, 0.0f, HorrorDebugVisualization::ComponentLabelHeightCm),
				FString::Printf(TEXT("Audio: %s"), *GetNameSafe(AudioComponent)),
				nullptr, FColor::Blue, 0.0f, true, 0.9f);
		}
	}
}

void UDebugVisualization::DrawLightDebugInfo()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!IsValid(Actor))
		{
			continue;
		}

		TArray<ULocalLightComponent*> LightComponents;
		Actor->GetComponents<ULocalLightComponent>(LightComponents);
		for (ULocalLightComponent* LightComponent : LightComponents)
		{
			if (!LightComponent || !LightComponent->IsVisible())
			{
				continue;
			}

			const FVector Location = LightComponent->GetComponentLocation();
			const float Radius = FMath::Max(100.0f, LightComponent->AttenuationRadius);
			DrawDebugSphere(World, Location, Radius, HorrorDebugVisualization::LightSphereSegments, FColor::Yellow, false, -1.0f, 0, 1.0f);
			DrawDebugString(World, Location + FVector(0.0f, 0.0f, HorrorDebugVisualization::ComponentLabelHeightCm),
				FString::Printf(TEXT("Light: %s\nRadius: %.0f"), *GetNameSafe(LightComponent), Radius),
				nullptr, FColor::Yellow, 0.0f, true, 0.9f);
		}
	}
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
