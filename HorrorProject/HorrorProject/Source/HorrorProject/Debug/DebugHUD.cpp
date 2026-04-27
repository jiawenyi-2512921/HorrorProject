// Copyright Epic Games, Inc. All Rights Reserved.

#include "DebugHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformMemory.h"
#include "EngineUtils.h"
#include "DynamicRHI.h"
#include "AI/HorrorThreatCharacter.h"
#include "Evidence/EvidenceActor.h"
#include "Player/Components/FearComponent.h"

namespace HorrorDebugHUD
{
	constexpr float DefaultLineHeight = 20.0f;
	constexpr float DefaultSectionSpacing = 10.0f;
	constexpr float MillisecondsPerSecond = 1000.0f;
	constexpr float TargetFrameRate = 60.0f;
	constexpr float WarningFrameRate = 30.0f;
	constexpr float BytesPerMegabyte = 1024.0f * 1024.0f;
	constexpr float PercentMultiplier = 100.0f;
	constexpr float HealthySanityPercent = 60.0f;
	constexpr float WarningSanityPercent = 30.0f;
	constexpr float HealthyFearPercent = 40.0f;
	constexpr float WarningFearPercent = 70.0f;
	constexpr float TextOriginX = 10.0f;
	constexpr float SectionBackgroundOriginX = 5.0f;
	constexpr float SectionBackgroundPaddingY = 5.0f;
	constexpr float SectionBackgroundWidth = 300.0f;
	constexpr float SectionTitleScale = 1.2f;
	constexpr float InitialYPosition = 50.0f;

	struct FGameplayActorCounts
	{
		int32 TotalActors = 0;
		int32 EnemyCount = 0;
		int32 EvidenceCount = 0;
		int32 CollectedEvidenceCount = 0;
	};

	FGameplayActorCounts CountGameplayActors(UWorld* World)
	{
		FGameplayActorCounts Counts;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			Counts.TotalActors++;
			if (It->ActorHasTag(FName("Enemy")) || Cast<AHorrorThreatCharacter>(*It))
			{
				Counts.EnemyCount++;
			}

			if (const AEvidenceActor* EvidenceActor = Cast<AEvidenceActor>(*It))
			{
				Counts.EvidenceCount++;
				Counts.CollectedEvidenceCount += EvidenceActor->IsCollected() ? 1 : 0;
			}
		}
		return Counts;
	}
}

ADebugHUD::ADebugHUD()
{
	bShowDebugHUD = false;
	bShowPerformance = true;
	bShowMemory = true;
	bShowPlayer = true;
	bShowGameplay = true;
	bShowSystem = false;
	bShowFrameTiming = false;
	bShowNetwork = false;

	TextColor = FLinearColor::White;
	BackgroundColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.5f);
	TextScale = 1.0f;

	LineHeight = HorrorDebugHUD::DefaultLineHeight;
	SectionSpacing = HorrorDebugHUD::DefaultSectionSpacing;
}

void ADebugHUD::BeginPlay()
{
	Super::BeginPlay();
}

void ADebugHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!bShowDebugHUD) return;

	ResetDrawPosition();

	if (bShowPerformance)
	{
		DrawPerformanceStats();
	}

	if (bShowMemory)
	{
		DrawMemoryStats();
	}

	if (bShowPlayer)
	{
		DrawPlayerStats();
	}

	if (bShowGameplay)
	{
		DrawGameplayStats();
	}

	if (bShowSystem)
	{
		DrawSystemInfo();
	}

	if (bShowFrameTiming)
	{
		DrawFrameTiming();
	}

	if (bShowNetwork)
	{
		DrawNetworkStats();
	}
}

void ADebugHUD::DrawPerformanceStats()
{
	DrawDebugSection(TEXT("PERFORMANCE"));

	UWorld* World = GetWorld();
	if (!World)
	{
		DrawDebugText(TEXT("World unavailable"), FLinearColor::Red);
		CurrentYPosition += SectionSpacing;
		return;
	}

	float FPS = 1.0f / World->GetDeltaSeconds();
	float FrameTime = World->GetDeltaSeconds() * HorrorDebugHUD::MillisecondsPerSecond;

	DrawDebugText(FString::Printf(TEXT("FPS: %.1f"), FPS),
		FPS >= HorrorDebugHUD::TargetFrameRate
			? FLinearColor::Green
			: (FPS >= HorrorDebugHUD::WarningFrameRate ? FLinearColor::Yellow : FLinearColor::Red));
	DrawDebugText(FString::Printf(TEXT("Frame Time: %.2f ms"), FrameTime));
	DrawDebugText(FString::Printf(TEXT("World Time: %.2f s"), World->GetTimeSeconds()));

	CurrentYPosition += SectionSpacing;
}

void ADebugHUD::DrawMemoryStats()
{
	DrawDebugSection(TEXT("MEMORY"));

	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	float UsedPhysicalMB = MemStats.UsedPhysical / HorrorDebugHUD::BytesPerMegabyte;
	float UsedVirtualMB = MemStats.UsedVirtual / HorrorDebugHUD::BytesPerMegabyte;
	float AvailablePhysicalMB = MemStats.AvailablePhysical / HorrorDebugHUD::BytesPerMegabyte;

	DrawDebugText(FString::Printf(TEXT("Physical: %.2f MB"), UsedPhysicalMB));
	DrawDebugText(FString::Printf(TEXT("Virtual: %.2f MB"), UsedVirtualMB));
	DrawDebugText(FString::Printf(TEXT("Available: %.2f MB"), AvailablePhysicalMB));

	CurrentYPosition += SectionSpacing;
}

void ADebugHUD::DrawPlayerStats()
{
	DrawDebugSection(TEXT("PLAYER"));

	UWorld* World = GetWorld();
	ACharacter* PlayerCharacter = World ? UGameplayStatics::GetPlayerCharacter(World, 0) : nullptr;
	if (PlayerCharacter)
	{
		FVector Location = PlayerCharacter->GetActorLocation();
		FRotator Rotation = PlayerCharacter->GetActorRotation();
		FVector Velocity = PlayerCharacter->GetVelocity();

		DrawDebugText(FString::Printf(TEXT("Position: %s"), *Location.ToCompactString()));
		DrawDebugText(FString::Printf(TEXT("Rotation: %s"), *Rotation.ToCompactString()));
		DrawDebugText(FString::Printf(TEXT("Velocity: %.2f"), Velocity.Size()));
		DrawDebugText(FString::Printf(TEXT("Speed: %.2f"), Velocity.Size2D()));
	}
	else
	{
		DrawDebugText(TEXT("No Player Character"), FLinearColor::Red);
	}

	CurrentYPosition += SectionSpacing;
}

void ADebugHUD::DrawGameplayStats()
{
	DrawDebugSection(TEXT("GAMEPLAY"));

	UWorld* World = GetWorld();
	if (!World)
	{
		DrawDebugText(TEXT("World unavailable"), FLinearColor::Red);
		CurrentYPosition += SectionSpacing;
		return;
	}

	const HorrorDebugHUD::FGameplayActorCounts Counts = HorrorDebugHUD::CountGameplayActors(World);
	DrawDebugText(FString::Printf(TEXT("Total Actors: %d"), Counts.TotalActors));
	DrawDebugText(FString::Printf(TEXT("Enemies: %d"), Counts.EnemyCount));
	DrawDebugText(FString::Printf(TEXT("Evidence: %d/%d collected"), Counts.CollectedEvidenceCount, Counts.EvidenceCount));

	DrawFearStats(World);
	CurrentYPosition += SectionSpacing;
}

void ADebugHUD::DrawFearStats(UWorld* World)
{
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(World, 0);
	UFearComponent* FearComponent = PlayerCharacter ? PlayerCharacter->FindComponentByClass<UFearComponent>() : nullptr;
	if (FearComponent)
	{
		const float FearPercent = FearComponent->GetFearPercent() * HorrorDebugHUD::PercentMultiplier;
		const float SanityPercent = HorrorDebugHUD::PercentMultiplier - FearPercent;
		DrawDebugText(FString::Printf(TEXT("Sanity: %.0f%%"), SanityPercent),
			SanityPercent >= HorrorDebugHUD::HealthySanityPercent
				? FLinearColor::Green
				: (SanityPercent >= HorrorDebugHUD::WarningSanityPercent ? FLinearColor::Yellow : FLinearColor::Red));
		DrawDebugText(FString::Printf(TEXT("Fear: %.0f%%"), FearPercent),
			FearPercent < HorrorDebugHUD::HealthyFearPercent
				? FLinearColor::Green
				: (FearPercent < HorrorDebugHUD::WarningFearPercent ? FLinearColor::Yellow : FLinearColor::Red));
	}
	else
	{
		DrawDebugText(TEXT("Sanity/Fear: component unavailable"), FLinearColor::Gray);
	}
}

void ADebugHUD::DrawSystemInfo()
{
	DrawDebugSection(TEXT("SYSTEM"));

	DrawDebugText(FString::Printf(TEXT("Platform: %s"), ANSI_TO_TCHAR(FPlatformProperties::PlatformName())));
	DrawDebugText(FString::Printf(TEXT("Build: %s"),
#if UE_BUILD_DEBUG
		TEXT("Debug")
#elif UE_BUILD_DEVELOPMENT
		TEXT("Development")
#elif UE_BUILD_SHIPPING
		TEXT("Shipping")
#else
		TEXT("Unknown")
#endif
	));
	DrawDebugText(FString::Printf(TEXT("Engine: %s"), *FEngineVersion::Current().ToString()));

	CurrentYPosition += SectionSpacing;
}

void ADebugHUD::DrawFrameTiming()
{
	DrawDebugSection(TEXT("FRAME TIMING"));

	float GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
	float RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
	float GPUFrameTime = FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles());

	DrawDebugText(FString::Printf(TEXT("Game Thread: %.2f ms"), GameThreadTime));
	DrawDebugText(FString::Printf(TEXT("Render Thread: %.2f ms"), RenderThreadTime));
	DrawDebugText(FString::Printf(TEXT("GPU: %.2f ms"), GPUFrameTime));

	CurrentYPosition += SectionSpacing;
}

void ADebugHUD::DrawNetworkStats()
{
	DrawDebugSection(TEXT("NETWORK"));

	APlayerController* PC = GetOwningPlayerController();
	if (PC && PC->PlayerState)
	{
		float Ping = PC->PlayerState->GetPingInMilliseconds();
		DrawDebugText(FString::Printf(TEXT("Ping: %.0f ms"), Ping));
	}
	else
	{
		DrawDebugText(TEXT("Not Connected"), FLinearColor::Gray);
	}

	CurrentYPosition += SectionSpacing;
}

void ADebugHUD::ToggleDebugHUD()
{
	bShowDebugHUD = !bShowDebugHUD;
	UE_LOG(LogTemp, Log, TEXT("Debug HUD: %s"), bShowDebugHUD ? TEXT("ON") : TEXT("OFF"));
}

void ADebugHUD::SetDebugCategory(FString Category, bool bEnabled)
{
	if (Category == TEXT("Performance"))
	{
		bShowPerformance = bEnabled;
	}
	else if (Category == TEXT("Memory"))
	{
		bShowMemory = bEnabled;
	}
	else if (Category == TEXT("Player"))
	{
		bShowPlayer = bEnabled;
	}
	else if (Category == TEXT("Gameplay"))
	{
		bShowGameplay = bEnabled;
	}
	else if (Category == TEXT("System"))
	{
		bShowSystem = bEnabled;
	}
	else if (Category == TEXT("FrameTiming"))
	{
		bShowFrameTiming = bEnabled;
	}
	else if (Category == TEXT("Network"))
	{
		bShowNetwork = bEnabled;
	}

	UE_LOG(LogTemp, Log, TEXT("Debug Category %s: %s"), *Category, bEnabled ? TEXT("ON") : TEXT("OFF"));
}

void ADebugHUD::DrawDebugText(const FString& Text, FLinearColor Color)
{
	if (!Canvas) return;

	FCanvasTextItem TextItem(FVector2D(HorrorDebugHUD::TextOriginX, CurrentYPosition), FText::FromString(Text),
		GEngine->GetSmallFont(), Color);
	TextItem.Scale = FVector2D(TextScale, TextScale);
	TextItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(TextItem);

	CurrentYPosition += LineHeight * TextScale;
}

void ADebugHUD::DrawDebugSection(const FString& Title)
{
	if (!Canvas) return;

	// Draw section background
	FCanvasTileItem TileItem(
		FVector2D(HorrorDebugHUD::SectionBackgroundOriginX, CurrentYPosition - HorrorDebugHUD::SectionBackgroundPaddingY),
		FVector2D(HorrorDebugHUD::SectionBackgroundWidth, LineHeight * TextScale + HorrorDebugHUD::SectionBackgroundPaddingY),
		BackgroundColor);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TileItem);

	// Draw section title
	FCanvasTextItem TextItem(FVector2D(HorrorDebugHUD::TextOriginX, CurrentYPosition), FText::FromString(Title),
		GEngine->GetSmallFont(), FLinearColor::Yellow);
	TextItem.Scale = FVector2D(TextScale * HorrorDebugHUD::SectionTitleScale, TextScale * HorrorDebugHUD::SectionTitleScale);
	TextItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(TextItem);

	CurrentYPosition += LineHeight * TextScale * HorrorDebugHUD::SectionTitleScale;
}

void ADebugHUD::ResetDrawPosition()
{
	CurrentYPosition = HorrorDebugHUD::InitialYPosition;
}
