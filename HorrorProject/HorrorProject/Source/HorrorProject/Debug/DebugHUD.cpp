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

	LineHeight = 20.0f;
	SectionSpacing = 10.0f;
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

	float FPS = 1.0f / GetWorld()->GetDeltaSeconds();
	float FrameTime = GetWorld()->GetDeltaSeconds() * 1000.0f;

	DrawDebugText(FString::Printf(TEXT("FPS: %.1f"), FPS),
		FPS >= 60.0f ? FLinearColor::Green : (FPS >= 30.0f ? FLinearColor::Yellow : FLinearColor::Red));
	DrawDebugText(FString::Printf(TEXT("Frame Time: %.2f ms"), FrameTime));
	DrawDebugText(FString::Printf(TEXT("World Time: %.2f s"), GetWorld()->GetTimeSeconds()));

	CurrentYPosition += SectionSpacing;
}

void ADebugHUD::DrawMemoryStats()
{
	DrawDebugSection(TEXT("MEMORY"));

	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	float UsedPhysicalMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
	float UsedVirtualMB = MemStats.UsedVirtual / (1024.0f * 1024.0f);
	float AvailablePhysicalMB = MemStats.AvailablePhysical / (1024.0f * 1024.0f);

	DrawDebugText(FString::Printf(TEXT("Physical: %.2f MB"), UsedPhysicalMB));
	DrawDebugText(FString::Printf(TEXT("Virtual: %.2f MB"), UsedVirtualMB));
	DrawDebugText(FString::Printf(TEXT("Available: %.2f MB"), AvailablePhysicalMB));

	CurrentYPosition += SectionSpacing;
}

void ADebugHUD::DrawPlayerStats()
{
	DrawDebugSection(TEXT("PLAYER"));

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
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

	// Count actors
	int32 TotalActors = 0;
	int32 EnemyCount = 0;
	int32 EvidenceCount = 0;

	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		TotalActors++;
		if (It->ActorHasTag(FName("Enemy")))
		{
			EnemyCount++;
		}
		if (It->ActorHasTag(FName("Evidence")))
		{
			EvidenceCount++;
		}
	}

	DrawDebugText(FString::Printf(TEXT("Total Actors: %d"), TotalActors));
	DrawDebugText(FString::Printf(TEXT("Enemies: %d"), EnemyCount));
	DrawDebugText(FString::Printf(TEXT("Evidence: %d"), EvidenceCount));

	// TODO: Add sanity/fear stats when horror system is integrated
	DrawDebugText(TEXT("Sanity: N/A"), FLinearColor::Gray);
	DrawDebugText(TEXT("Fear: N/A"), FLinearColor::Gray);

	CurrentYPosition += SectionSpacing;
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

	FCanvasTextItem TextItem(FVector2D(10.0f, CurrentYPosition), FText::FromString(Text),
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
	FCanvasTileItem TileItem(FVector2D(5.0f, CurrentYPosition - 5.0f),
		FVector2D(300.0f, LineHeight * TextScale + 5.0f), BackgroundColor);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TileItem);

	// Draw section title
	FCanvasTextItem TextItem(FVector2D(10.0f, CurrentYPosition), FText::FromString(Title),
		GEngine->GetSmallFont(), FLinearColor::Yellow);
	TextItem.Scale = FVector2D(TextScale * 1.2f, TextScale * 1.2f);
	TextItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(TextItem);

	CurrentYPosition += LineHeight * TextScale * 1.2f;
}

void ADebugHUD::ResetDrawPosition()
{
	CurrentYPosition = 50.0f;
}
