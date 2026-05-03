// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorMapChainExit.h"

#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Game/HorrorGameModeBase.h"
#include "Game/HorrorMapChain.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/HUD.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "Blueprint/UserWidget.h"
#include "UI/EndingCreditsWidget.h"

namespace
{
	const FVector ExitTriggerExtent(130.0f, 130.0f, 120.0f);
	const FColor NextMapLightColor(80, 180, 255);
	const FColor FinalExitLightColor(255, 120, 60);
	const FText NextMapExitLabel = NSLOCTEXT("HorrorMapChainExit", "NextMapExitLabel", "下一地图");
	const FText FinalExitLabel = NSLOCTEXT("HorrorMapChainExit", "FinalExitLabel", "黑盒终章");
	const FText FinalEndingMessage = NSLOCTEXT("HorrorMapChainExit", "FinalEndingMessage", "黑盒回放完成。深水站真相已归档。");

	bool IsPlayerPawn(const AActor* Actor)
	{
		const APawn* Pawn = Cast<APawn>(Actor);
		return Pawn && Pawn->IsPlayerControlled();
	}

	bool IsDay1MapPackage(const FString& MapPackageName)
	{
		const FString NormalizedMapPackageName = FHorrorMapChain::NormalizeMapPackageName(MapPackageName);
		return NormalizedMapPackageName.Contains(TEXT("/DeepWaterStation/"))
			|| FPackageName::GetShortName(NormalizedMapPackageName) == TEXT("DemoMap_VerticalSlice_Day1");
	}
}

AHorrorMapChainExit::AHorrorMapChainExit()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBounds"));
	TriggerBounds->SetBoxExtent(ExitTriggerExtent);
	TriggerBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBounds->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBounds->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBounds->SetGenerateOverlapEvents(true);
	RootComponent = TriggerBounds;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(TriggerBounds);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -60.0f));
	VisualMesh->SetRelativeScale3D(FVector(1.3f, 0.08f, 2.2f));

	if (UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		VisualMesh->SetStaticMesh(CubeMesh);
	}

	LabelText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("LabelText"));
	LabelText->SetupAttachment(TriggerBounds);
	LabelText->SetHorizontalAlignment(EHTA_Center);
	LabelText->SetVerticalAlignment(EVRTA_TextCenter);
	LabelText->SetWorldSize(42.0f);
	LabelText->SetRelativeLocation(FVector(0.0f, 0.0f, 130.0f));
	LabelText->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));

	MarkerLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("MarkerLight"));
	MarkerLight->SetupAttachment(TriggerBounds);
	MarkerLight->SetIntensity(1800.0f);
	MarkerLight->SetAttenuationRadius(450.0f);
	MarkerLight->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));

	RefreshMarkerVisuals();
}

void AHorrorMapChainExit::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBounds)
	{
		TriggerBounds->OnComponentBeginOverlap.AddUniqueDynamic(this, &AHorrorMapChainExit::HandleTriggerBeginOverlap);
	}

	RefreshMarkerVisuals();
}

void AHorrorMapChainExit::ConfigureForMapChain(
	const FString& InCurrentMapPackageName,
	const FString& InNextMapPackageName,
	bool bInFinalExit)
{
	CurrentMapPackageName = InCurrentMapPackageName;
	NextMapPackageName = InNextMapPackageName;
	bFinalExit = bInFinalExit;
	bTransitionTriggered = false;

	RefreshMarkerVisuals();
}

void AHorrorMapChainExit::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	(void)OverlappedComponent;
	(void)OtherComp;
	(void)OtherBodyIndex;
	(void)bFromSweep;
	(void)SweepResult;

	if (bTransitionTriggered || !IsPlayerPawn(OtherActor))
	{
		return;
	}

	if (!CanUseExit(OtherActor))
	{
		ShowLockedFeedback(OtherActor);
		return;
	}

	bTransitionTriggered = true;
	if (bFinalExit || NextMapPackageName.IsEmpty())
	{
		TriggerFinalEnding(OtherActor);
		return;
	}

	TriggerMapTravel(OtherActor);
}

void AHorrorMapChainExit::RefreshMarkerVisuals()
{
	if (LabelText)
	{
		LabelText->SetText(bFinalExit ? FinalExitLabel : NextMapExitLabel);
		LabelText->SetTextRenderColor(bFinalExit ? FinalExitLightColor : NextMapLightColor);
	}

	if (MarkerLight)
	{
		MarkerLight->SetLightColor(FLinearColor(bFinalExit ? FinalExitLightColor : NextMapLightColor));
	}
}

bool AHorrorMapChainExit::CanUseExit(AActor* InstigatorActor) const
{
	(void)InstigatorActor;

	const UWorld* World = GetWorld();
	const AHorrorGameModeBase* HorrorGameMode = World ? World->GetAuthGameMode<AHorrorGameModeBase>() : nullptr;
	if (!HorrorGameMode)
	{
		return true;
	}

	const bool bIsDay1Exit = IsDay1MapPackage(CurrentMapPackageName.IsEmpty() ? (World ? World->GetPackage()->GetName() : FString()) : CurrentMapPackageName);
	if (bIsDay1Exit)
	{
		return HorrorGameMode->IsDay1Complete();
	}

	return HorrorGameMode->IsCurrentCampaignChapterComplete();
}

void AHorrorMapChainExit::ShowLockedFeedback(AActor* InstigatorActor) const
{
	const UWorld* World = GetWorld();
	const AHorrorGameModeBase* HorrorGameMode = World ? World->GetAuthGameMode<AHorrorGameModeBase>() : nullptr;
	const int32 CompletedObjectiveCount = HorrorGameMode ? HorrorGameMode->GetCampaignCompletedObjectiveCount() : 0;
	const int32 RequiredObjectiveCount = HorrorGameMode ? HorrorGameMode->GetCampaignRequiredObjectiveCount() : 0;
	const FString Message = RequiredObjectiveCount > 0
		? FString::Printf(TEXT("出口已锁定：完成当前章节目标后解锁（%d/%d）。"), CompletedObjectiveCount, RequiredObjectiveCount)
		: FString(TEXT("出口已锁定：完成当前章节目标后解锁。"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.0f, FColor::Yellow, Message);
	}

	APawn* InstigatorPawn = Cast<APawn>(InstigatorActor);
	if (APlayerController* PlayerController = InstigatorPawn ? Cast<APlayerController>(InstigatorPawn->GetController()) : nullptr)
	{
		PlayerController->ClientMessage(Message);
	}
}

void AHorrorMapChainExit::TriggerMapTravel(AActor* InstigatorActor)
{
	(void)InstigatorActor;

	UWorld* World = GetWorld();
	if (!World)
	{
		bTransitionTriggered = false;
		return;
	}

	const FString TravelOptions = FHorrorMapChain::BuildTravelOptionsForConfiguredGameMode();
	UGameplayStatics::OpenLevel(World, FName(*NextMapPackageName), true, TravelOptions);
}

void AHorrorMapChainExit::TriggerFinalEnding(AActor* InstigatorActor)
{
	if (TriggerBounds)
	{
		TriggerBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	APawn* InstigatorPawn = Cast<APawn>(InstigatorActor);
	APlayerController* PlayerController = InstigatorPawn ? Cast<APlayerController>(InstigatorPawn->GetController()) : nullptr;
	if (!PlayerController)
	{
		// cast 失败时重置，避免出口永久锁死
		bTransitionTriggered = false;
		if (TriggerBounds)
		{
			TriggerBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		return;
	}

	if (AHUD* HUD = PlayerController->GetHUD())
	{
		HUD->bShowHUD = false;
	}

	// 优先尝试 Blueprint Widget，路径使用正确的 _C 后缀格式
	UClass* WidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/UI/WBP_EndingCredits.WBP_EndingCredits_C"));
	if (WidgetClass)
	{
		if (UUserWidget* CreditsWidget = CreateWidget<UUserWidget>(PlayerController, WidgetClass))
		{
			CreditsWidget->AddToViewport(100);
			if (UFunction* StartFunc = CreditsWidget->FindFunction(FName("StartCreditsRoll")))
			{
				CreditsWidget->ProcessEvent(StartFunc, nullptr);
			}
		}
	}
	else
	{
		if (UEndingCreditsWidget* CreditsWidget = CreateWidget<UEndingCreditsWidget>(PlayerController, UEndingCreditsWidget::StaticClass()))
		{
			CreditsWidget->AddToViewport(100);
			CreditsWidget->StartCreditsRoll();
		}
	}

	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = false;
}

#if WITH_DEV_AUTOMATION_TESTS
FText AHorrorMapChainExit::GetLabelTextForTests() const
{
	return LabelText ? LabelText->Text : FText::GetEmpty();
}

FText AHorrorMapChainExit::GetFinalEndingMessageForTests() const
{
	return FinalEndingMessage;
}
#endif
