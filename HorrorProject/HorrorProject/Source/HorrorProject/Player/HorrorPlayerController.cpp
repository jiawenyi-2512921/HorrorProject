// Copyright Epic Games, Inc. All Rights Reserved.


#include "Player/HorrorPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Game/DeepWaterStationRouteKit.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Game/HorrorEncounterDirector.h"
#include "Game/HorrorFoundFootageContract.h"
#include "Game/HorrorGameModeBase.h"
#include "HorrorProjectCameraManager.h"
#include "InputCoreTypes.h"
#include "InputKeyEventArgs.h"
#include "Interaction/DoorInteractable.h"
#include "Kismet/GameplayStatics.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/Components/CameraBatteryComponent.h"
#include "Player/Components/FearComponent.h"
#include "Player/Components/InteractionComponent.h"
#include "Player/Components/NoteRecorderComponent.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "ControlSettings.h"
#include "GameSettingsSubsystem.h"
#include "GraphicsSettings.h"
#include "HorrorAudioSettings.h"
#include "UI/Day1SliceHUD.h"
#include "Variant_Horror/HorrorCharacter.h"
#include "Variant_Horror/UI/HorrorUI.h"
#include "HorrorProject.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Input/SVirtualJoystick.h"

namespace
{
	constexpr float Day1NavigationArrivedDistanceCm = 125.0f;

	EFoundFootageInteractableObjective ResolveNextDay1Objective(const AHorrorGameModeBase& GameMode)
	{
		if (!GameMode.HasBodycamAcquired())
		{
			return EFoundFootageInteractableObjective::Bodycam;
		}

		if (!GameMode.HasCollectedFirstNote())
		{
			return EFoundFootageInteractableObjective::FirstNote;
		}

		if (GameMode.HasPendingFirstAnomalyCandidate() && !GameMode.HasRecordedFirstAnomaly())
		{
			return EFoundFootageInteractableObjective::FirstAnomalyRecord;
		}

		if (!GameMode.HasRecordedFirstAnomaly())
		{
			return EFoundFootageInteractableObjective::FirstAnomalyCandidate;
		}

		if (!GameMode.HasReviewedArchive())
		{
			return EFoundFootageInteractableObjective::ArchiveReview;
		}

		if (GameMode.IsExitUnlocked())
		{
			return EFoundFootageInteractableObjective::ExitRouteGate;
		}

		return EFoundFootageInteractableObjective::ExitRouteGate;
	}

	const TCHAR* ResolveNavigationDirectionLabel(const APawn& Pawn, const FVector& ToObjective)
	{
		const FVector HorizontalToObjective(ToObjective.X, ToObjective.Y, 0.0f);
	if (HorizontalToObjective.SizeSquared() <= FMath::Square(Day1NavigationArrivedDistanceCm))
	{
		return TEXT("附近");
	}

		const FRotator YawRotation(0.0f, Pawn.GetActorRotation().Yaw, 0.0f);
		const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		const FVector Direction = HorizontalToObjective.GetSafeNormal();
		const float ForwardDot = FVector::DotProduct(Forward, Direction);
		const float RightDot = FVector::DotProduct(Right, Direction);

	if (ForwardDot >= 0.65f)
	{
		return TEXT("前方");
	}

	if (ForwardDot <= -0.65f)
	{
		return TEXT("后方");
	}

	return RightDot >= 0.0f ? TEXT("右侧") : TEXT("左侧");
}

	FText BuildDay1NavigationText(const APawn& Pawn, const AHorrorGameModeBase& GameMode, const ADeepWaterStationRouteKit& RouteKit)
	{
		if (GameMode.IsDay1Complete())
		{
			return FText::GetEmpty();
		}

		FVector ObjectiveLocation = FVector::ZeroVector;
		if (!RouteKit.TryGetObjectiveWorldLocation(ResolveNextDay1Objective(GameMode), ObjectiveLocation))
		{
			return FText::GetEmpty();
		}

		const FVector ToObjective = ObjectiveLocation - Pawn.GetActorLocation();
		const FVector HorizontalToObjective(ToObjective.X, ToObjective.Y, 0.0f);
	const int32 DistanceMeters = FMath::Max(0, FMath::RoundToInt(HorizontalToObjective.Size() / 100.0f));
	return FText::FromString(FString::Printf(
		TEXT("%s %d 米"),
		ResolveNavigationDirectionLabel(Pawn, ToObjective),
		DistanceMeters));
}
}

AHorrorPlayerController::AHorrorPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AHorrorProjectCameraManager::StaticClass();
	bEnableMotionControls = false;
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> HorrorGameplayContext(
		TEXT("/Game/Input/IMC_HorrorGameplay.IMC_HorrorGameplay"));
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MouseLookContext(
		TEXT("/Game/Input/IMC_MouseLook.IMC_MouseLook"));

	if (HorrorGameplayContext.Succeeded())
	{
		DefaultMappingContexts.AddUnique(HorrorGameplayContext.Object);
	}

	if (MouseLookContext.Succeeded())
	{
		MobileExcludedMappingContexts.AddUnique(MouseLookContext.Object);
	}
}

void AHorrorPlayerController::BeginPlay()
{
	Super::BeginPlay();

	EnsureDay1NativeHUD();

	// only spawn touch controls on local player controllers
	if (SVirtualJoystick::ShouldDisplayTouchInterface() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogHorrorProject, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}

	ApplyInputMappingContexts();
	BindObjectiveEventBus();
	RefreshDay1HUDState();
}

void AHorrorPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	RefreshDay1HUDState();
}

void AHorrorPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bDay1PauseMenuOpen)
	{
		CloseDay1PauseMenu();
	}

	if (bDay1NotesJournalOpen)
	{
		CloseDay1NotesJournal();
	}

	UnbindPawnNoteRecorder();
	UnbindObjectiveEventBus();
	Super::EndPlay(EndPlayReason);
}

void AHorrorPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);
	BindPawnNoteRecorder(aPawn);
	EnsureDay1NativeHUD();

	// only spawn UI on local player controllers
	if (IsLocalPlayerController() && bAutoCreateHorrorUI && !ShouldUseDay1NativeHUD())
	{
		// set up the UI for the character
		if (AHorrorCharacter* HorrorCharacter = Cast<AHorrorCharacter>(aPawn))
		{
			// create the UI
			if (!HorrorUI)
			{
				HorrorUI = CreateWidget<UHorrorUI>(this, HorrorUIClass);
				if (HorrorUI)
				{
					HorrorUI->AddToViewport(0);
				}
				else
				{
					UE_LOG(LogHorrorProject, Error, TEXT("Could not spawn horror UI widget."));
					return;
				}
			}

			HorrorUI->SetupCharacter(HorrorCharacter);
		}
	}

	RefreshDay1HUDState();
}

void AHorrorPlayerController::OnUnPossess()
{
	UnbindPawnNoteRecorder();
	Super::OnUnPossess();
	RefreshDay1HUDState();
}

void AHorrorPlayerController::RestampCheckpointLoadedUIState()
{
	EnsureDay1NativeHUD();
	ResetDay1ModalInputState();

	if (HorrorUI)
	{
		HorrorUI->RestampCheckpointLoadedState();
	}

	if (ADay1SliceHUD* Day1HUD = GetDay1SliceHUD())
	{
		Day1HUD->ClearPasswordPrompt();
		Day1HUD->ClearPauseMenu();
		Day1HUD->ClearNotesJournal();
	}

	RefreshDay1HUDState();

	const AHorrorGameModeBase* HorrorGameMode = Cast<AHorrorGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (HorrorGameMode && HorrorGameMode->IsDay1Complete())
	{
		ApplyDay1CompletionInputLock();

		if (ADay1SliceHUD* Day1HUD = GetDay1SliceHUD())
		{
			Day1HUD->ShowDay1CompletionOverlay(
				FText::FromString(TEXT("第 1 天完成")),
				FText::FromString(TEXT("证据已保全。画面切黑。")));
		}
	}
	else if (bDay1CompletionInputLocked)
	{
		bDay1CompletionInputLocked = false;
		SetIgnoreMoveInput(false);
		SetIgnoreLookInput(false);
		if (ADay1SliceHUD* Day1HUD = GetDay1SliceHUD())
		{
			Day1HUD->ClearDay1CompletionOverlay();
		}
	}
}

void AHorrorPlayerController::BeginDoorPasswordEntry(ADoorInteractable* Door)
{
	if (bDay1CompletionInputLocked || !Door || !Door->RequiresPassword() || Door->IsPasswordUnlocked())
	{
		return;
	}

	PendingPasswordDoor = Door;
	DoorPasswordBuffer.Reset();
	ShowDoorPasswordPrompt();
}

void AHorrorPlayerController::ResetDay1ModalInputState()
{
	if (bDay1PauseMenuOpen)
	{
		bDay1PauseMenuOpen = false;
		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}

	if (bDay1NotesJournalOpen)
	{
		bDay1NotesJournalOpen = false;
	}

	ClearDoorPasswordEntry();

	if (!bDay1CompletionInputLocked)
	{
		SetIgnoreMoveInput(false);
		SetIgnoreLookInput(false);
	}

	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	if (ADay1SliceHUD* Day1HUD = GetDay1SliceHUD())
	{
		Day1HUD->ClearPauseMenu();
		Day1HUD->ClearNotesJournal();
		if (!bDay1CompletionInputLocked)
		{
			Day1HUD->ClearDay1CompletionOverlay();
		}
	}
}

void AHorrorPlayerController::ShowPlayerMessage(const FText& MessageText, const FLinearColor& MessageColor, float DisplaySeconds)
{
	if (ADay1SliceHUD* Day1HUD = GetDay1SliceHUD())
	{
		Day1HUD->ShowTransientMessage(MessageText, MessageColor, DisplaySeconds);
		return;
	}

	if (GEngine && !MessageText.IsEmpty())
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			DisplaySeconds,
			MessageColor.ToFColor(true),
			MessageText.ToString());
	}
}

void AHorrorPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	ApplyInputMappingContexts();
}

bool AHorrorPlayerController::InputKey(const FInputKeyEventArgs& Params)
{
	if (bDay1CompletionInputLocked && Params.Event == IE_Pressed)
	{
		return true;
	}

	if (PendingPasswordDoor.IsValid() && Params.Event == IE_Pressed)
	{
		HandleDoorPasswordKey(Params.Key);
		return true;
	}

	if (Params.Event == IE_Pressed)
	{
		if (bDay1PauseMenuOpen)
		{
			if (HandleDay1PauseMenuKey(Params.Key))
			{
				return true;
			}
		}
		else if (bDay1NotesJournalOpen)
		{
			if (Params.Key == EKeys::J || Params.Key == EKeys::Tab || Params.Key == EKeys::Escape)
			{
				CloseDay1NotesJournal();
				return true;
			}
		}
		else if (Params.Key == EKeys::J || Params.Key == EKeys::Tab)
		{
			return ToggleDay1NotesJournal();
		}
		else if (Params.Key == EKeys::Escape)
		{
			OpenDay1PauseMenu();
			return true;
		}
	}

	return Super::InputKey(Params);
}

#if WITH_DEV_AUTOMATION_TESTS
bool AHorrorPlayerController::HandleInputKeyForTests(const FKey& Key, EInputEvent Event)
{
	return InputKey(FInputKeyEventArgs::CreateSimulated(Key, Event, 1.0f));
}

void AHorrorPlayerController::BindObjectiveEventBusForTests()
{
	BindObjectiveEventBus();
}

void AHorrorPlayerController::RefreshDay1HUDStateForTests()
{
	RefreshDay1HUDState();
}

bool AHorrorPlayerController::IsBoundToNoteRecorderForTests(const UNoteRecorderComponent* NoteRecorder) const
{
	return NoteRecorder && BoundNoteRecorder.Get() == NoteRecorder;
}

bool AHorrorPlayerController::HasNoteRecordedDelegateForTests(const UNoteRecorderComponent* NoteRecorder)
{
	return NoteRecorder && BoundNoteRecorder.Get() == NoteRecorder && NoteRecordedHandle.IsValid();
}
#endif

void AHorrorPlayerController::ApplyInputMappingContexts()
{
	if (bInputContextsApplied || !IsLocalPlayerController())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			if (CurrentContext)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
		}

		if (!SVirtualJoystick::ShouldDisplayTouchInterface())
		{
			for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
			{
				if (CurrentContext)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}

		bInputContextsApplied = true;
	}
}

ADay1SliceHUD* AHorrorPlayerController::GetDay1SliceHUD() const
{
	return Cast<ADay1SliceHUD>(GetHUD());
}

bool AHorrorPlayerController::ShouldUseDay1NativeHUD() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FString MapName = World->GetMapName();
	return MapName.Contains(TEXT("DemoMap_VerticalSlice_Day1"))
		|| Cast<AHorrorGameModeBase>(UGameplayStatics::GetGameMode(this)) != nullptr;
}

void AHorrorPlayerController::EnsureDay1NativeHUD()
{
	if (!IsLocalPlayerController() || !ShouldUseDay1NativeHUD())
	{
		return;
	}

	if (!GetDay1SliceHUD())
	{
		ClientSetHUD(ADay1SliceHUD::StaticClass());
	}

	if (HorrorUI)
	{
		HorrorUI->RemoveFromParent();
		HorrorUI = nullptr;
	}
}

void AHorrorPlayerController::BindObjectiveEventBus()
{
	UWorld* World = GetWorld();
	UHorrorEventBusSubsystem* EventBus = World ? World->GetSubsystem<UHorrorEventBusSubsystem>() : nullptr;
	if (!EventBus)
	{
		return;
	}

	if (BoundObjectiveEventBus.Get() == EventBus && ObjectiveEventHandle.IsValid())
	{
		return;
	}

	UnbindObjectiveEventBus();
	BoundObjectiveEventBus = EventBus;
	ObjectiveEventHandle = EventBus->GetOnEventPublishedNative().AddUObject(this, &AHorrorPlayerController::HandleObjectiveEventPublished);
}

void AHorrorPlayerController::UnbindObjectiveEventBus()
{
	if (UHorrorEventBusSubsystem* EventBus = BoundObjectiveEventBus.Get())
	{
		if (ObjectiveEventHandle.IsValid())
		{
			EventBus->GetOnEventPublishedNative().Remove(ObjectiveEventHandle);
		}
	}

	ObjectiveEventHandle.Reset();
	BoundObjectiveEventBus.Reset();
}

void AHorrorPlayerController::HandleObjectiveEventPublished(const FHorrorEventMessage& Message)
{
	const bool bDay1Completed = Message.EventTag == HorrorDay1Tags::Day1CompletedEvent();
	if (bDay1Completed)
	{
		ApplyDay1CompletionInputLock();
	}

	ADay1SliceHUD* Day1HUD = GetDay1SliceHUD();
	if (!Day1HUD)
	{
		if (GEngine && (!Message.ObjectiveHint.IsEmpty() || !Message.DebugLabel.IsEmpty()))
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				4.0f,
				FColor::Cyan,
				!Message.ObjectiveHint.IsEmpty() ? Message.ObjectiveHint.ToString() : Message.DebugLabel.ToString());
		}
		return;
	}

	if (Message.EventTag == HorrorDay1Tags::PlayerFailureEvent())
	{
		if (Message.StateTag == HorrorDay1Tags::CheckpointRestoredState())
		{
			Day1HUD->ShowTransientMessage(
				FText::FromString(TEXT("已恢复到上一个检查点。")),
				FLinearColor(0.42f, 0.82f, 1.0f),
				2.5f);
		}
		else if (Message.StateTag == HorrorDay1Tags::CheckpointMissingState())
		{
			Day1HUD->ShowTransientMessage(
				FText::FromString(TEXT("没有可用检查点。")),
				FLinearColor(1.0f, 0.78f, 0.15f),
				2.5f);
		}
		return;
	}

	const FText Title = !Message.DebugLabel.IsEmpty()
		? Message.DebugLabel
		: FText::FromString(Message.EventTag.IsValid() ? Message.EventTag.ToString() : TEXT("目标已更新"));
	const FText Hint = !Message.ObjectiveHint.IsEmpty()
		? Message.ObjectiveHint
		: BuildObjectivePrompt();

	Day1HUD->ShowObjectiveToast(Title, Hint);

	if (Message.EventTag == HorrorSaveTags::CheckpointSavedEvent())
	{
		Day1HUD->ShowAutosaveIndicator(FText::FromString(TEXT("检查点已保存。")));
	}

	if (Message.EventTag == HorrorSaveTags::CheckpointSaveFailedEvent())
	{
		Day1HUD->ShowTransientMessage(
			FText::FromString(TEXT("检查点保存失败。")),
			FLinearColor(1.0f, 0.78f, 0.15f),
			3.0f);
	}

	if (bDay1Completed)
	{
		Day1HUD->ShowDay1CompletionOverlay(
			FText::FromString(TEXT("第 1 天完成")),
			FText::FromString(TEXT("证据已保全。画面切黑。")));
		Day1HUD->ShowTransientMessage(
			FText::FromString(TEXT("第 1 天完成。证据已保全。画面切黑。")),
			FLinearColor(0.32f, 0.95f, 0.78f),
			6.0f);
	}
}

void AHorrorPlayerController::BindPawnNoteRecorder(APawn* PawnOverride)
{
	UNoteRecorderComponent* NoteRecorder = nullptr;
	if (const AHorrorPlayerCharacter* HorrorPlayerCharacter = Cast<AHorrorPlayerCharacter>(PawnOverride))
	{
		NoteRecorder = HorrorPlayerCharacter->GetNoteRecorderComponent();
	}
	else
	{
		NoteRecorder = GetPawnNoteRecorder();
	}

	if (!NoteRecorder)
	{
		if (PawnOverride)
		{
			UnbindPawnNoteRecorder();
		}
		return;
	}

	if (BoundNoteRecorder.Get() == NoteRecorder)
	{
		return;
	}

	UnbindPawnNoteRecorder();

	BoundNoteRecorder = NoteRecorder;
	NoteRecordedHandle = NoteRecorder->OnNoteRecordedNative.AddUObject(this, &AHorrorPlayerController::HandlePawnNoteRecorded);
}

void AHorrorPlayerController::UnbindPawnNoteRecorder()
{
	if (UNoteRecorderComponent* NoteRecorder = BoundNoteRecorder.Get())
	{
		if (NoteRecordedHandle.IsValid())
		{
			NoteRecorder->OnNoteRecordedNative.Remove(NoteRecordedHandle);
		}
	}
	NoteRecordedHandle.Reset();
	BoundNoteRecorder.Reset();
}

void AHorrorPlayerController::HandlePawnNoteRecorded(FName NoteId, int32 TotalRecordedNotes)
{
#if WITH_DEV_AUTOMATION_TESTS
	++NoteRecordedFeedbackCountForTests;
#endif
	ShowPlayerMessage(
		FText::FromString(TEXT("笔记已记录。按 J键/Tab键查看。")),
		FLinearColor(0.42f, 0.82f, 1.0f),
		2.5f);

	if (bDay1NotesJournalOpen)
	{
		RefreshDay1NotesJournalHUD();
	}
}

void AHorrorPlayerController::RefreshDay1HUDState()
{
	BindPawnNoteRecorder();
	BindObjectiveEventBus();

	if (ADay1SliceHUD* Day1HUD = GetDay1SliceHUD())
	{
		const AHorrorPlayerCharacter* HorrorPlayerCharacter = Cast<AHorrorPlayerCharacter>(GetPawn());
		const UQuantumCameraComponent* QuantumCamera = HorrorPlayerCharacter ? HorrorPlayerCharacter->GetQuantumCameraComponent() : nullptr;
		const UCameraBatteryComponent* CameraBattery = QuantumCamera ? QuantumCamera->GetBatteryComponent() : nullptr;
		const UFearComponent* Fear = HorrorPlayerCharacter ? HorrorPlayerCharacter->GetFearComponent() : nullptr;
		const AHorrorGameModeBase* HorrorGameMode = Cast<AHorrorGameModeBase>(UGameplayStatics::GetGameMode(this));
		const ADeepWaterStationRouteKit* RouteKit = HorrorGameMode ? HorrorGameMode->GetRuntimeRouteKit() : nullptr;
		const AHorrorEncounterDirector* EncounterDirector = HorrorGameMode ? HorrorGameMode->GetRuntimeEncounterDirector() : nullptr;
		const bool bDanger = (RouteKit && RouteKit->IsRouteGatedByEncounter()) || (EncounterDirector && EncounterDirector->IsRouteGated());

		Day1HUD->SetCurrentObjective(BuildObjectivePrompt());
		if (HorrorPlayerCharacter && HorrorGameMode && RouteKit)
		{
			const FText NavigationText = BuildDay1NavigationText(*HorrorPlayerCharacter, *HorrorGameMode, *RouteKit);
			if (!NavigationText.IsEmpty())
			{
				Day1HUD->SetObjectiveNavigation(NavigationText);
			}
			else
			{
				Day1HUD->ClearObjectiveNavigation();
			}
		}
		else
		{
			Day1HUD->ClearObjectiveNavigation();
		}

		Day1HUD->SetSurvivalStatus(
			QuantumCamera && QuantumCamera->IsCameraAcquired(),
			QuantumCamera && QuantumCamera->IsCameraEnabled(),
			QuantumCamera && QuantumCamera->IsCameraMode(EQuantumCameraMode::Recording),
			Fear ? Fear->GetFearPercent() : 0.0f,
			HorrorPlayerCharacter ? HorrorPlayerCharacter->GetSprintPercent() : 1.0f,
			bDanger);
		if (CameraBattery)
		{
			Day1HUD->SetBodycamBatteryStatus(CameraBattery->GetBatteryPercentage(), CameraBattery->IsBatteryLow());
		}
		else
		{
			Day1HUD->ClearBodycamBatteryStatus();
		}
	}

	RefreshInteractionPrompt();

	if (bDay1NotesJournalOpen)
	{
		RefreshDay1NotesJournalHUD();
	}
}

void AHorrorPlayerController::RefreshInteractionPrompt()
{
	ADay1SliceHUD* Day1HUD = GetDay1SliceHUD();
	if (!Day1HUD)
	{
		return;
	}

	if (PendingPasswordDoor.IsValid())
	{
		Day1HUD->ClearInteractionPrompt();
		return;
	}

	const AHorrorPlayerCharacter* HorrorPlayerCharacter = Cast<AHorrorPlayerCharacter>(GetPawn());
	const UInteractionComponent* InteractionComponent = HorrorPlayerCharacter ? HorrorPlayerCharacter->GetInteractionComponent() : nullptr;
	FText PromptText;
	if (InteractionComponent && InteractionComponent->GetFocusedInteractionPrompt(PromptText))
	{
		Day1HUD->SetInteractionPrompt(PromptText);
	}
	else
	{
		Day1HUD->ClearInteractionPrompt();
	}
}

FText AHorrorPlayerController::BuildObjectivePrompt() const
{
	const AHorrorGameModeBase* HorrorGameMode = Cast<AHorrorGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (!HorrorGameMode)
	{
		return FText::FromString(TEXT("找回随身摄像机。"));
	}

	if (HorrorGameMode->IsDay1Complete())
	{
		return FText::FromString(TEXT("第 1 天完成。证据已保全。"));
	}

	if (!HorrorGameMode->HasBodycamAcquired())
	{
		return FText::FromString(TEXT("找回随身摄像机。"));
	}

	if (!HorrorGameMode->HasCollectedFirstNote())
	{
		return FText::FromString(TEXT("找到并阅读第一份站内笔记。"));
	}

	if (HorrorGameMode->HasPendingFirstAnomalyCandidate() && !HorrorGameMode->HasRecordedFirstAnomaly())
	{
		return FText::FromString(TEXT("保持随身摄像机录制，拍下异常点。"));
	}

	if (!HorrorGameMode->HasRecordedFirstAnomaly())
	{
		return FText::FromString(TEXT("找到第一个异常点。"));
	}

	if (!HorrorGameMode->HasReviewedArchive())
	{
		return FText::FromString(TEXT("前往档案终端查看录像。"));
	}

	if (HorrorGameMode->IsExitUnlocked())
	{
		return FText::FromString(TEXT("从勤务闸门离开。"));
	}

	return FText::FromString(TEXT("继续穿过站点。"));
}

void AHorrorPlayerController::ClearDoorPasswordEntry()
{
	PendingPasswordDoor.Reset();
	DoorPasswordBuffer.Reset();

	if (ADay1SliceHUD* Day1HUD = GetDay1SliceHUD())
	{
		Day1HUD->ClearPasswordPrompt();
	}
}

void AHorrorPlayerController::CancelDoorPasswordEntry()
{
	ClearDoorPasswordEntry();
	ShowPlayerMessage(FText::FromString(TEXT("已取消输入门禁密码。")), FLinearColor(0.75f, 0.78f, 0.80f), 1.5f);
}

void AHorrorPlayerController::ApplyDay1CompletionInputLock()
{
	if (bDay1PauseMenuOpen)
	{
		bDay1PauseMenuOpen = false;
		UGameplayStatics::SetGamePaused(GetWorld(), false);
		if (ADay1SliceHUD* Day1HUD = GetDay1SliceHUD())
		{
			Day1HUD->ClearPauseMenu();
		}
	}

	if (bDay1NotesJournalOpen)
	{
		bDay1NotesJournalOpen = false;
		if (ADay1SliceHUD* Day1HUD = GetDay1SliceHUD())
		{
			Day1HUD->ClearNotesJournal();
		}
	}

	ClearDoorPasswordEntry();
	bDay1CompletionInputLocked = true;
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
}

void AHorrorPlayerController::OpenDay1PauseMenu()
{
	if (bDay1CompletionInputLocked || bDay1PauseMenuOpen)
	{
		return;
	}

	bDay1PauseMenuOpen = true;
	if (bDay1NotesJournalOpen)
	{
		CloseDay1NotesJournal();
	}
	Day1PauseMenuSelection = EDay1PauseMenuSelection::Resume;
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
	UGameplayStatics::SetGamePaused(GetWorld(), true);
	RefreshDay1PauseMenuHUD();
}

void AHorrorPlayerController::CloseDay1PauseMenu()
{
	if (!bDay1PauseMenuOpen)
	{
		return;
	}

	bDay1PauseMenuOpen = false;
	if (!bDay1CompletionInputLocked)
	{
		SetIgnoreMoveInput(false);
		SetIgnoreLookInput(false);
	}
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	if (ADay1SliceHUD* Day1HUD = GetDay1SliceHUD())
	{
		Day1HUD->ClearPauseMenu();
	}
}

void AHorrorPlayerController::OpenDay1NotesJournal()
{
	if (bDay1CompletionInputLocked)
	{
		return;
	}

	if (bDay1NotesJournalOpen)
	{
		RefreshDay1NotesJournalHUD();
		return;
	}

	bDay1NotesJournalOpen = true;
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
	RefreshDay1NotesJournalHUD();
}

void AHorrorPlayerController::CloseDay1NotesJournal()
{
	if (!bDay1NotesJournalOpen)
	{
		return;
	}

	bDay1NotesJournalOpen = false;
	if (!bDay1CompletionInputLocked)
	{
		SetIgnoreMoveInput(false);
		SetIgnoreLookInput(false);
	}
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	if (ADay1SliceHUD* Day1HUD = GetDay1SliceHUD())
	{
		Day1HUD->ClearNotesJournal();
	}
}

bool AHorrorPlayerController::ToggleDay1NotesJournal()
{
	if (bDay1NotesJournalOpen)
	{
		CloseDay1NotesJournal();
	}
	else
	{
		OpenDay1NotesJournal();
	}

	return true;
}

void AHorrorPlayerController::RefreshDay1NotesJournalHUD()
{
	if (!bDay1NotesJournalOpen)
	{
		return;
	}

	ADay1SliceHUD* Day1HUD = GetDay1SliceHUD();
	if (!Day1HUD)
	{
		return;
	}

	const UNoteRecorderComponent* NoteRecorder = GetPawnNoteRecorder();
	Day1HUD->ShowNotesJournal(NoteRecorder ? NoteRecorder->GetRecordedNoteMetadata() : TArray<FHorrorNoteMetadata>());
}

UNoteRecorderComponent* AHorrorPlayerController::GetPawnNoteRecorder() const
{
	const AHorrorPlayerCharacter* HorrorPlayerCharacter = Cast<AHorrorPlayerCharacter>(GetPawn());
	return HorrorPlayerCharacter ? HorrorPlayerCharacter->GetNoteRecorderComponent() : nullptr;
}

bool AHorrorPlayerController::HandleDay1PauseMenuKey(const FKey& Key)
{
	if (Key == EKeys::Escape || Key == EKeys::Enter || Key == EKeys::SpaceBar)
	{
		CloseDay1PauseMenu();
		return true;
	}

	if (Key == EKeys::Up || Key == EKeys::W)
	{
		MoveDay1PauseMenuSelection(-1);
		return true;
	}

	if (Key == EKeys::Down || Key == EKeys::S)
	{
		MoveDay1PauseMenuSelection(1);
		return true;
	}

	if (Key == EKeys::Left || Key == EKeys::A)
	{
		AdjustDay1PauseMenuValue(-1.0f);
		return true;
	}

	if (Key == EKeys::Right || Key == EKeys::D)
	{
		AdjustDay1PauseMenuValue(1.0f);
		return true;
	}

	return false;
}

void AHorrorPlayerController::MoveDay1PauseMenuSelection(int32 Direction)
{
	const int32 SelectionCount = 4;
	const int32 CurrentIndex = static_cast<int32>(Day1PauseMenuSelection);
	const int32 NextIndex = (CurrentIndex + Direction + SelectionCount) % SelectionCount;
	Day1PauseMenuSelection = static_cast<EDay1PauseMenuSelection>(NextIndex);
	RefreshDay1PauseMenuHUD();
}

void AHorrorPlayerController::AdjustDay1PauseMenuValue(float Direction)
{
	const float SignedDirection = Direction >= 0.0f ? 1.0f : -1.0f;
	if (Day1PauseMenuSelection == EDay1PauseMenuSelection::MouseSensitivity)
	{
		if (UControlSettings* ControlSettings = GetControlSettings())
		{
			ControlSettings->MouseSensitivity = FMath::Clamp(ControlSettings->MouseSensitivity + (0.1f * SignedDirection), 0.1f, 5.0f);
			if (UGameSettingsSubsystem* SettingsSubsystem = GetGameSettingsSubsystem())
			{
				SettingsSubsystem->MarkDirty(TEXT("Controls"));
				SettingsSubsystem->SaveSettings();
			}
		}
	}
	else if (Day1PauseMenuSelection == EDay1PauseMenuSelection::MasterVolume)
	{
		if (UHorrorAudioSettings* AudioSettings = GetAudioSettings())
		{
			AudioSettings->SetMasterVolume(AudioSettings->MasterVolume + (0.05f * SignedDirection), true);
			if (UGameSettingsSubsystem* SettingsSubsystem = GetGameSettingsSubsystem())
			{
				SettingsSubsystem->MarkDirty(TEXT("Audio"));
				SettingsSubsystem->SaveSettings();
			}
		}
	}
	else if (Day1PauseMenuSelection == EDay1PauseMenuSelection::Brightness)
	{
		if (UGraphicsSettings* GraphicsSettings = GetGraphicsSettings())
		{
			GraphicsSettings->Brightness = FMath::Clamp(GraphicsSettings->Brightness + (0.05f * SignedDirection), 0.2f, 2.0f);
			GraphicsSettings->Apply();
			if (UGameSettingsSubsystem* SettingsSubsystem = GetGameSettingsSubsystem())
			{
				SettingsSubsystem->MarkDirty(TEXT("Graphics"));
				SettingsSubsystem->SaveSettings();
			}
		}
	}

	RefreshDay1PauseMenuHUD();
}

void AHorrorPlayerController::RefreshDay1PauseMenuHUD()
{
	if (!bDay1PauseMenuOpen)
	{
		return;
	}

	const UControlSettings* ControlSettings = GetControlSettings();
	const UHorrorAudioSettings* AudioSettings = GetAudioSettings();
	const UGraphicsSettings* GraphicsSettings = GetGraphicsSettings();

	if (ADay1SliceHUD* Day1HUD = GetDay1SliceHUD())
	{
		Day1HUD->ShowPauseMenu(
			Day1PauseMenuSelection,
			ControlSettings ? ControlSettings->MouseSensitivity : 1.0f,
			AudioSettings ? AudioSettings->MasterVolume : 1.0f,
			GraphicsSettings ? GraphicsSettings->Brightness : 1.0f);
	}
}

UGameSettingsSubsystem* AHorrorPlayerController::GetGameSettingsSubsystem() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UGameSettingsSubsystem>() : nullptr;
}

UControlSettings* AHorrorPlayerController::GetControlSettings() const
{
	const UGameSettingsSubsystem* SettingsSubsystem = GetGameSettingsSubsystem();
	return SettingsSubsystem ? SettingsSubsystem->GetControlSettings() : nullptr;
}

UHorrorAudioSettings* AHorrorPlayerController::GetAudioSettings() const
{
	const UGameSettingsSubsystem* SettingsSubsystem = GetGameSettingsSubsystem();
	return SettingsSubsystem ? SettingsSubsystem->GetAudioSettings() : nullptr;
}

UGraphicsSettings* AHorrorPlayerController::GetGraphicsSettings() const
{
	const UGameSettingsSubsystem* SettingsSubsystem = GetGameSettingsSubsystem();
	return SettingsSubsystem ? SettingsSubsystem->GetGraphicsSettings() : nullptr;
}

bool AHorrorPlayerController::IsDay1AutosaveMilestoneState(FGameplayTag StateTag)
{
	return StateTag == HorrorFoundFootageTags::BodycamAcquiredState()
		|| StateTag == HorrorFoundFootageTags::FirstNoteCollectedState()
		|| StateTag == HorrorFoundFootageTags::ArchiveReviewedState()
		|| StateTag == HorrorFoundFootageTags::ExitUnlockedState();
}

void AHorrorPlayerController::ShowDoorPasswordPrompt() const
{
	if (!PendingPasswordDoor.IsValid())
	{
		return;
	}

	const ADoorInteractable* Door = PendingPasswordDoor.Get();
	FString HintText = Door->GetPasswordHint().IsEmpty()
		? FString(TEXT("在环境中寻找密码。"))
		: Door->GetPasswordHint().ToString();
	const AHorrorPlayerCharacter* HorrorPlayerCharacter = Cast<AHorrorPlayerCharacter>(GetPawn());
	const UNoteRecorderComponent* NoteRecorder = HorrorPlayerCharacter ? HorrorPlayerCharacter->GetNoteRecorderComponent() : nullptr;
	if (NoteRecorder && NoteRecorder->GetRecordedNoteCount() > 0 && !HintText.Contains(TEXT("J键/Tab键")))
	{
		HintText += TEXT(" 按 J键/Tab键查看已记录笔记。");
	}

	if (ADay1SliceHUD* Day1HUD = GetDay1SliceHUD())
	{
		Day1HUD->ShowPasswordPrompt(
			FText::FromString(TEXT("门禁密码")),
			FText::FromString(HintText),
			DoorPasswordBuffer.Len(),
			Door->GetRequiredPasswordLength());
		return;
	}

	FString MaskedInput;
	for (int32 Index = 0; Index < DoorPasswordBuffer.Len(); ++Index)
	{
		MaskedInput.AppendChar(TEXT('*'));
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			741700,
			6.0f,
				FColor::Cyan,
				FString::Printf(
				TEXT("需要门禁密码\n输入：%s\n数字键 + 回车键确认，退格键删除，Esc键取消。提示：%s"),
				*MaskedInput,
				*HintText));
	}
}

bool AHorrorPlayerController::HandleDoorPasswordKey(const FKey& Key)
{
	if (!PendingPasswordDoor.IsValid())
	{
		return false;
	}

	if (Key == EKeys::Escape)
	{
		CancelDoorPasswordEntry();
		return true;
	}

	if (Key == EKeys::BackSpace)
	{
		if (!DoorPasswordBuffer.IsEmpty())
		{
			DoorPasswordBuffer.LeftChopInline(1, EAllowShrinking::No);
		}
		ShowDoorPasswordPrompt();
		return true;
	}

	ADoorInteractable* Door = PendingPasswordDoor.Get();
	if (Key == EKeys::Enter)
	{
		const bool bAccepted = Door->SubmitPassword(GetPawn() ? Cast<AActor>(GetPawn()) : Cast<AActor>(this), DoorPasswordBuffer);
		if (bAccepted)
		{
			PendingPasswordDoor.Reset();
			DoorPasswordBuffer.Reset();
			if (ADay1SliceHUD* Day1HUD = GetDay1SliceHUD())
			{
				Day1HUD->ClearPasswordPrompt();
			}
		}
		else
		{
			DoorPasswordBuffer.Reset();
			ShowPlayerMessage(
				FText::FromString(TEXT("密码错误。")),
				FLinearColor(1.0f, 0.24f, 0.16f),
				2.5f);
			ShowDoorPasswordPrompt();
		}
		return true;
	}

	TCHAR Digit = TEXT('\0');
	if (TryGetDigitForKey(Key, Digit))
	{
		const int32 RequiredLength = Door->GetRequiredPasswordLength();
		const int32 MaxLength = RequiredLength > 0 ? RequiredLength : 8;
		if (DoorPasswordBuffer.Len() < MaxLength)
		{
			DoorPasswordBuffer.AppendChar(Digit);
		}
		ShowDoorPasswordPrompt();
		return true;
	}

	return false;
}

bool AHorrorPlayerController::TryGetDigitForKey(const FKey& Key, TCHAR& OutDigit)
{
	if (Key == EKeys::Zero || Key == EKeys::NumPadZero)
	{
		OutDigit = TEXT('0');
		return true;
	}
	if (Key == EKeys::One || Key == EKeys::NumPadOne)
	{
		OutDigit = TEXT('1');
		return true;
	}
	if (Key == EKeys::Two || Key == EKeys::NumPadTwo)
	{
		OutDigit = TEXT('2');
		return true;
	}
	if (Key == EKeys::Three || Key == EKeys::NumPadThree)
	{
		OutDigit = TEXT('3');
		return true;
	}
	if (Key == EKeys::Four || Key == EKeys::NumPadFour)
	{
		OutDigit = TEXT('4');
		return true;
	}
	if (Key == EKeys::Five || Key == EKeys::NumPadFive)
	{
		OutDigit = TEXT('5');
		return true;
	}
	if (Key == EKeys::Six || Key == EKeys::NumPadSix)
	{
		OutDigit = TEXT('6');
		return true;
	}
	if (Key == EKeys::Seven || Key == EKeys::NumPadSeven)
	{
		OutDigit = TEXT('7');
		return true;
	}
	if (Key == EKeys::Eight || Key == EKeys::NumPadEight)
	{
		OutDigit = TEXT('8');
		return true;
	}
	if (Key == EKeys::Nine || Key == EKeys::NumPadNine)
	{
		OutDigit = TEXT('9');
		return true;
	}

	return false;
}
