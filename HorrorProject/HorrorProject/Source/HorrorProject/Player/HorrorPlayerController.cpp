// Copyright Epic Games, Inc. All Rights Reserved.


#include "Player/HorrorPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Game/DeepWaterStationRouteKit.h"
#include "Game/FoundFootageObjectiveInteractable.h"
#include "Game/HorrorCampaignObjectiveActor.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Game/HorrorEncounterDirector.h"
#include "Game/HorrorFoundFootageContract.h"
#include "Game/HorrorGameModeBase.h"
#include "HorrorProjectCameraManager.h"
#include "InputCoreTypes.h"
#include "InputKeyEventArgs.h"
#include "Interaction/DoorInteractable.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/Components/CameraBatteryComponent.h"
#include "Player/Components/FearComponent.h"
#include "Player/Components/FlashlightComponent.h"
#include "Player/Components/InteractionComponent.h"
#include "Player/Components/NoteRecorderComponent.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "Player/Components/InventoryComponent.h"
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
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Input/SVirtualJoystick.h"

namespace
{
	constexpr float Day1NavigationArrivedDistanceCm = 125.0f;
	constexpr float FirstAnomalyCaptureLockSeconds = 0.85f;
	constexpr float FirstAnomalyCaptureDecaySecondsPerSecond = 1.5f;
	constexpr float FirstAnomalyCaptureMaxDistanceCm = 650.0f;
	constexpr float FirstAnomalyCaptureMaxFocusAngleDegrees = 12.0f;

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

	const TCHAR* ResolveDay1ObjectiveNavigationLabel(EFoundFootageInteractableObjective Objective)
	{
		switch (Objective)
		{
		case EFoundFootageInteractableObjective::Bodycam:
			return TEXT("随身摄像机");
		case EFoundFootageInteractableObjective::FirstNote:
			return TEXT("站内备忘录");
		case EFoundFootageInteractableObjective::FirstAnomalyCandidate:
			return TEXT("第一个异常");
		case EFoundFootageInteractableObjective::FirstAnomalyRecord:
			return TEXT("异常录像窗口");
		case EFoundFootageInteractableObjective::ArchiveReview:
			return TEXT("档案终端");
		case EFoundFootageInteractableObjective::ExitRouteGate:
			return TEXT("出口闸门");
		}

		return TEXT("目标");
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

		const EFoundFootageInteractableObjective NextObjective = ResolveNextDay1Objective(GameMode);
		FVector ObjectiveLocation = FVector::ZeroVector;
		if (!RouteKit.TryGetObjectiveWorldLocation(NextObjective, ObjectiveLocation))
		{
			return FText::GetEmpty();
		}

		const FVector ToObjective = ObjectiveLocation - Pawn.GetActorLocation();
		const FVector HorizontalToObjective(ToObjective.X, ToObjective.Y, 0.0f);
		const int32 DistanceMeters = FMath::Max(0, FMath::RoundToInt(HorizontalToObjective.Size() / 100.0f));
		return FText::FromString(FString::Printf(
			TEXT("%s：%s %d 米"),
			ResolveDay1ObjectiveNavigationLabel(NextObjective),
			ResolveNavigationDirectionLabel(Pawn, ToObjective),
			DistanceMeters));
	}

	FText BuildCampaignNavigationText(const APawn& Pawn, const AHorrorGameModeBase& GameMode)
	{
		FVector ObjectiveLocation = FVector::ZeroVector;
		if (!GameMode.TryGetCurrentCampaignObjectiveWorldLocation(ObjectiveLocation))
		{
			return FText::GetEmpty();
		}

		const FVector ToObjective = ObjectiveLocation - Pawn.GetActorLocation();
		const FVector HorizontalToObjective(ToObjective.X, ToObjective.Y, 0.0f);
		const int32 DistanceMeters = FMath::Max(0, FMath::RoundToInt(HorizontalToObjective.Size() / 100.0f));
		const FText ActionText = GameMode.GetCurrentCampaignObjectiveActionText();
		if (!ActionText.IsEmpty())
		{
			return FText::Format(
				NSLOCTEXT("HorrorPlayerController", "CampaignNavigationWithAction", "目标：{0} / {1} {2} 米"),
				ActionText,
				FText::FromString(ResolveNavigationDirectionLabel(Pawn, ToObjective)),
				FText::AsNumber(DistanceMeters));
		}

		return FText::Format(
			NSLOCTEXT("HorrorPlayerController", "CampaignNavigation", "目标：{0} {1} 米"),
			FText::FromString(ResolveNavigationDirectionLabel(Pawn, ToObjective)),
			FText::AsNumber(DistanceMeters));
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
	UpdateDay1RuntimeState(DeltaSeconds);
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
				NSLOCTEXT("HorrorPlayerController", "Day1CompleteTitle", "第一天完成"),
				NSLOCTEXT("HorrorPlayerController", "Day1CompleteHint", "证据已保存，画面正在淡出。"));
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

bool AHorrorPlayerController::TrySubmitActiveAdvancedInteractionExpectedInput()
{
	if (bDay1CompletionInputLocked || bDay1PauseMenuOpen || bDay1NotesJournalOpen || PendingPasswordDoor.IsValid())
	{
		return false;
	}

	AHorrorCampaignObjectiveActor* ObjectiveActor = ResolveActiveAdvancedInteractionObjective();
	if (!ObjectiveActor)
	{
		return false;
	}

	const FName ExpectedInputId = ObjectiveActor->GetExpectedAdvancedInputId();
	if (ExpectedInputId.IsNone())
	{
		return false;
	}

	const bool bSubmitted = ObjectiveActor->SubmitAdvancedInteractionInput(ExpectedInputId, GetPawn() ? Cast<AActor>(GetPawn()) : Cast<AActor>(this));
	RefreshDay1HUDState();
	return bSubmitted;
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
		else if (TryHandleAdvancedInteractionKey(Params.Key))
		{
			return true;
		}
		else if (Params.Key == EKeys::J || Params.Key == EKeys::Tab)
		{
			return ToggleDay1NotesJournal();
		}
		else if (Params.Key == EKeys::F)
		{
			if (AHorrorPlayerCharacter* HorrorPlayerCharacter = Cast<AHorrorPlayerCharacter>(GetPawn()))
			{
				if (UFlashlightComponent* FlashlightComponent = HorrorPlayerCharacter->GetFlashlightComponent())
				{
					FlashlightComponent->ToggleFlashlight();
					return true;
				}
			}
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

bool AHorrorPlayerController::UpdateDay1RuntimeStateForTests(float DeltaSeconds)
{
	return UpdateDay1RuntimeState(DeltaSeconds);
}

bool AHorrorPlayerController::IsBoundToNoteRecorderForTests(const UNoteRecorderComponent* NoteRecorder) const
{
	return NoteRecorder && BoundNoteRecorder.Get() == NoteRecorder;
}

bool AHorrorPlayerController::HasNoteRecordedDelegateForTests(const UNoteRecorderComponent* NoteRecorder)
{
	return NoteRecorder && BoundNoteRecorder.Get() == NoteRecorder && NoteRecordedHandle.IsValid();
}

void AHorrorPlayerController::SetActiveAdvancedInteractionObjectiveForTests(AHorrorCampaignObjectiveActor* ObjectiveActor)
{
	ActiveAdvancedInteractionObjective = ObjectiveActor;
}

AHorrorCampaignObjectiveActor* AHorrorPlayerController::GetActiveAdvancedInteractionObjectiveForTests() const
{
	return ActiveAdvancedInteractionObjective.Get();
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
				NSLOCTEXT("HorrorPlayerController", "CheckpointRestored", "已恢复到上一个检查点。"),
				FLinearColor(0.42f, 0.82f, 1.0f),
				2.5f);
		}
		else if (Message.StateTag == HorrorDay1Tags::CheckpointMissingState())
		{
			Day1HUD->ShowTransientMessage(
				NSLOCTEXT("HorrorPlayerController", "NoCheckpoint", "没有可用检查点。"),
				FLinearColor(1.0f, 0.78f, 0.15f),
				2.5f);
		}
		return;
	}

	const FText Title = !Message.DebugLabel.IsEmpty()
		? Message.DebugLabel
		: NSLOCTEXT("HorrorPlayerController", "ObjectiveUpdated", "目标已更新");
	const FText Hint = !Message.ObjectiveHint.IsEmpty()
		? Message.ObjectiveHint
		: BuildObjectivePrompt();

	Day1HUD->ShowObjectiveToast(Title, Hint);

	if (Message.EventTag == HorrorSaveTags::CheckpointSavedEvent())
	{
		Day1HUD->ShowAutosaveIndicator(NSLOCTEXT("HorrorPlayerController", "CheckpointSaved", "检查点已保存。"));
	}

	if (Message.EventTag == HorrorSaveTags::CheckpointSaveFailedEvent())
	{
		Day1HUD->ShowTransientMessage(
			NSLOCTEXT("HorrorPlayerController", "CheckpointSaveFailed", "检查点保存失败。"),
			FLinearColor(1.0f, 0.78f, 0.15f),
			3.0f);
	}

	if (bDay1Completed)
	{
		Day1HUD->ShowDay1CompletionOverlay(
			NSLOCTEXT("HorrorPlayerController", "Day1CompleteTitle2", "第 1 天完成"),
			NSLOCTEXT("HorrorPlayerController", "Day1CompleteHint2", "证据已保存，画面正在淡出。"));
		Day1HUD->ShowTransientMessage(
			NSLOCTEXT("HorrorPlayerController", "Day1CompleteMessage", "第一天完成。证据已保存，画面正在淡出。"),
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
		NSLOCTEXT("HorrorPlayerController", "NoteRecorded", "笔记已记录，按 J键/Tab键 查看。"),
		FLinearColor(0.42f, 0.82f, 1.0f),
		2.5f);

	if (bDay1NotesJournalOpen)
	{
		RefreshDay1NotesJournalHUD();
	}
}

bool AHorrorPlayerController::UpdateDay1RuntimeState(float DeltaSeconds)
{
	const bool bCapturedFocusedAnomaly = TryAutoCaptureFocusedAnomaly(DeltaSeconds);
	RefreshDay1HUDState();
	return bCapturedFocusedAnomaly;
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

		if (HorrorGameMode)
		{
			Day1HUD->SetObjectiveTracker(HorrorGameMode->BuildObjectiveTrackerSnapshot());
		}
		else
		{
			Day1HUD->SetCurrentObjective(BuildObjectivePrompt());
		}
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
		else if (HorrorPlayerCharacter && HorrorGameMode)
		{
			const FText NavigationText = BuildCampaignNavigationText(*HorrorPlayerCharacter, *HorrorGameMode);
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

		RefreshAnomalyCaptureHUD(*Day1HUD, HorrorPlayerCharacter, HorrorGameMode);
		RefreshAdvancedInteractionHUD(*Day1HUD, HorrorPlayerCharacter);
	}

	RefreshInteractionPrompt();

	if (bDay1NotesJournalOpen)
	{
		RefreshDay1NotesJournalHUD();
	}
}

void AHorrorPlayerController::RefreshAdvancedInteractionHUD(ADay1SliceHUD& Day1HUD, const AHorrorPlayerCharacter* HorrorPlayerCharacter)
{
	AHorrorCampaignObjectiveActor* ObjectiveActor = ResolveActiveAdvancedInteractionObjective(HorrorPlayerCharacter);
	if (!ObjectiveActor)
	{
		Day1HUD.ClearAdvancedInteractionState();
		return;
	}

	Day1HUD.SetAdvancedInteractionState(ObjectiveActor->BuildAdvancedInteractionHUDState());
	if (ObjectiveActor->IsAdvancedInteractionActive())
	{
		Day1HUD.ClearInteractionPrompt();
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

	if (ResolveActiveAdvancedInteractionObjective())
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

AHorrorCampaignObjectiveActor* AHorrorPlayerController::ResolveActiveAdvancedInteractionObjective(const AHorrorPlayerCharacter* HorrorPlayerCharacter)
{
	if (AHorrorCampaignObjectiveActor* CachedObjective = ActiveAdvancedInteractionObjective.Get())
	{
		if (CachedObjective->IsAdvancedInteractionActive() && !CachedObjective->IsCompleted())
		{
			return CachedObjective;
		}
	}
	ActiveAdvancedInteractionObjective.Reset();

	if (!HorrorPlayerCharacter)
	{
		HorrorPlayerCharacter = Cast<AHorrorPlayerCharacter>(GetPawn());
	}

	const UInteractionComponent* InteractionComponent = HorrorPlayerCharacter ? HorrorPlayerCharacter->GetInteractionComponent() : nullptr;
	if (!InteractionComponent)
	{
		return nullptr;
	}

	FHitResult FocusedHit;
	UObject* FocusedTargetObject = nullptr;
	if (!InteractionComponent->FindFocusedInteractionTarget(FocusedHit, FocusedTargetObject))
	{
		return nullptr;
	}

	AHorrorCampaignObjectiveActor* FocusedObjective = Cast<AHorrorCampaignObjectiveActor>(FocusedTargetObject);
	if (!FocusedObjective)
	{
		FocusedObjective = Cast<AHorrorCampaignObjectiveActor>(FocusedHit.GetActor());
	}

	if (FocusedObjective && FocusedObjective->IsAdvancedInteractionActive() && !FocusedObjective->IsCompleted())
	{
		ActiveAdvancedInteractionObjective = FocusedObjective;
		return FocusedObjective;
	}

	return nullptr;
}

void AHorrorPlayerController::RefreshAnomalyCaptureHUD(
	ADay1SliceHUD& Day1HUD,
	const AHorrorPlayerCharacter* HorrorPlayerCharacter,
	const AHorrorGameModeBase* HorrorGameMode)
{
	if (!HorrorPlayerCharacter || !HorrorGameMode || HorrorGameMode->HasRecordedFirstAnomaly())
	{
		Day1HUD.ClearAnomalyCaptureStatus();
		return;
	}

	const UInteractionComponent* InteractionComponent = HorrorPlayerCharacter->GetInteractionComponent();
	const UQuantumCameraComponent* QuantumCamera = HorrorPlayerCharacter->GetQuantumCameraComponent();
	if (!HorrorGameMode->HasCollectedFirstNote() || !InteractionComponent || !QuantumCamera)
	{
		Day1HUD.ClearAnomalyCaptureStatus();
		return;
	}

	const AFoundFootageObjectiveInteractable* ObjectiveActor = ResolveFocusedFirstAnomalyTarget(*HorrorPlayerCharacter);
	const bool bFocusedFirstAnomaly = ObjectiveActor != nullptr;
	if (!bFocusedFirstAnomaly)
	{
		Day1HUD.SetAnomalyCaptureStatus(
			NSLOCTEXT("HorrorPlayerController", "AnomalyCaptureSearching", "搜索异常信号。"),
			FMath::Clamp(FocusedAnomalyLockSeconds / FirstAnomalyCaptureLockSeconds, 0.0f, 1.0f),
			false,
			false);
		return;
	}

	const bool bRecording = QuantumCamera->IsCameraMode(EQuantumCameraMode::Recording);
	const float LockProgress = FMath::Clamp(FocusedAnomalyLockSeconds / FirstAnomalyCaptureLockSeconds, 0.0f, 1.0f);
	Day1HUD.SetAnomalyCaptureStatus(
		bRecording
			? NSLOCTEXT("HorrorPlayerController", "AnomalyCaptureLocking", "异常已对准，保持录像锁定。")
			: NSLOCTEXT("HorrorPlayerController", "AnomalyCaptureNeedsRecording", "异常已对准，开启录像锁定。"),
		bRecording ? FMath::Max(0.1f, LockProgress) : 0.35f,
		bRecording && LockProgress >= 1.0f,
		!bRecording);
}

AFoundFootageObjectiveInteractable* AHorrorPlayerController::ResolveFocusedFirstAnomalyTarget(const AHorrorPlayerCharacter& HorrorPlayerCharacter) const
{
	const UCameraComponent* FirstPersonCamera = HorrorPlayerCharacter.GetFirstPersonCameraComponent();
	UWorld* World = GetWorld();
	if (!FirstPersonCamera || !World)
	{
		return nullptr;
	}

	AFoundFootageObjectiveInteractable* BestCandidate = nullptr;
	float BestFocusScore = -1.0f;
	const FVector CameraLocation = FirstPersonCamera->GetComponentLocation();
	const FVector CameraForward = FirstPersonCamera->GetForwardVector();
	const float RequiredFocusDot = FMath::Cos(FMath::DegreesToRadians(FirstAnomalyCaptureMaxFocusAngleDegrees));

	for (TActorIterator<AFoundFootageObjectiveInteractable> It(World); It; ++It)
	{
		AFoundFootageObjectiveInteractable* Candidate = *It;
		if (!IsFocusedFirstAnomalyTarget(Candidate))
		{
			continue;
		}

		const FVector ToAnomaly = Candidate->GetActorLocation() - CameraLocation;
		const float DistanceSquared = ToAnomaly.SizeSquared();
		if (DistanceSquared > FMath::Square(FirstAnomalyCaptureMaxDistanceCm) || DistanceSquared <= UE_KINDA_SMALL_NUMBER)
		{
			continue;
		}

		const float FocusDot = FVector::DotProduct(CameraForward, ToAnomaly.GetSafeNormal());
		if (FocusDot < RequiredFocusDot)
		{
			continue;
		}

		const float FocusScore = FocusDot - (DistanceSquared / FMath::Square(FirstAnomalyCaptureMaxDistanceCm)) * 0.05f;
		if (FocusScore > BestFocusScore)
		{
			BestFocusScore = FocusScore;
			BestCandidate = Candidate;
		}
	}

	return BestCandidate;
}

bool AHorrorPlayerController::IsFocusedFirstAnomalyTarget(const AFoundFootageObjectiveInteractable* ObjectiveActor) const
{
	return ObjectiveActor
		&& (ObjectiveActor->Objective == EFoundFootageInteractableObjective::FirstAnomalyCandidate
			|| ObjectiveActor->Objective == EFoundFootageInteractableObjective::FirstAnomalyRecord);
}

bool AHorrorPlayerController::TryAutoCaptureFocusedAnomaly(float DeltaSeconds)
{
	AHorrorPlayerCharacter* HorrorPlayerCharacter = Cast<AHorrorPlayerCharacter>(GetPawn());
	AHorrorGameModeBase* HorrorGameMode = Cast<AHorrorGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (!HorrorPlayerCharacter || !HorrorGameMode || HorrorGameMode->HasRecordedFirstAnomaly())
	{
		FocusedAnomalyLockSeconds = 0.0f;
		LockedAnomalyTarget.Reset();
		return false;
	}

	UQuantumCameraComponent* QuantumCamera = HorrorPlayerCharacter->GetQuantumCameraComponent();
	if (!QuantumCamera)
	{
		return false;
	}

	const float SafeDeltaSeconds = FMath::Clamp(DeltaSeconds, 0.0f, 1.0f);

	AFoundFootageObjectiveInteractable* ObjectiveActor = ResolveFocusedFirstAnomalyTarget(*HorrorPlayerCharacter);
	const bool bRecording = HorrorGameMode->IsLeadPlayerRecording() || QuantumCamera->IsCameraMode(EQuantumCameraMode::Recording);
	if (!ObjectiveActor || !bRecording)
	{
		FocusedAnomalyLockSeconds = FMath::Max(0.0f, FocusedAnomalyLockSeconds - (SafeDeltaSeconds * FirstAnomalyCaptureDecaySecondsPerSecond));
		if (!ObjectiveActor)
		{
			LockedAnomalyTarget.Reset();
		}
		return false;
	}

	if (LockedAnomalyTarget.Get() != ObjectiveActor)
	{
		LockedAnomalyTarget = ObjectiveActor;
		FocusedAnomalyLockSeconds = 0.0f;
	}
	FocusedAnomalyLockSeconds = FMath::Min(FirstAnomalyCaptureLockSeconds, FocusedAnomalyLockSeconds + SafeDeltaSeconds);
	if (FocusedAnomalyLockSeconds < FirstAnomalyCaptureLockSeconds)
	{
		return false;
	}

	if (ObjectiveActor->Objective == EFoundFootageInteractableObjective::FirstAnomalyCandidate
		&& !HorrorGameMode->HasPendingFirstAnomalyCandidate())
	{
		if (!HorrorGameMode->BeginFirstAnomalyCandidate(ObjectiveActor->SourceId))
		{
			return false;
		}
	}

	const FName PendingAnomalyId = HorrorGameMode->GetPendingFirstAnomalySourceId();
	const bool bRecordedFirstAnomaly = !PendingAnomalyId.IsNone() && HorrorGameMode->TryRecordFirstAnomaly(bRecording);
	if (!bRecordedFirstAnomaly)
	{
		return false;
	}

	if (UInventoryComponent* Inventory = HorrorPlayerCharacter->GetInventoryComponent())
	{
		if (!ObjectiveActor->EvidenceMetadata.EvidenceId.IsNone()
			&& ObjectiveActor->EvidenceMetadata.EvidenceId == PendingAnomalyId)
		{
			Inventory->RegisterEvidenceMetadata(ObjectiveActor->EvidenceMetadata);
		}
		Inventory->AddCollectedEvidenceId(PendingAnomalyId);
	}

	ShowPlayerMessage(
		NSLOCTEXT("HorrorPlayerController", "AnomalyVideoLocked", "异常录像已锁定。"),
		FLinearColor(0.32f, 0.95f, 0.78f),
		2.5f);
	FocusedAnomalyLockSeconds = 0.0f;
	LockedAnomalyTarget.Reset();
	return true;
}

FText AHorrorPlayerController::BuildObjectivePrompt() const
{
	const AHorrorGameModeBase* HorrorGameMode = Cast<AHorrorGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (!HorrorGameMode)
	{
		return NSLOCTEXT("HorrorPlayerController", "ObjRecoverBodycam", "取回随身摄像机。");
	}

	const FHorrorObjectiveTrackerSnapshot Tracker = HorrorGameMode->BuildObjectiveTrackerSnapshot();
	return Tracker.PrimaryInstruction.IsEmpty()
		? NSLOCTEXT("HorrorPlayerController", "ObjContinueThrough", "继续深入站内。")
		: Tracker.PrimaryInstruction;
}

bool AHorrorPlayerController::TryHandleAdvancedInteractionKey(const FKey& Key)
{
	if (bDay1CompletionInputLocked || bDay1PauseMenuOpen || bDay1NotesJournalOpen || PendingPasswordDoor.IsValid())
	{
		return false;
	}

	AHorrorCampaignObjectiveActor* ObjectiveActor = ResolveActiveAdvancedInteractionObjective();
	if (!ObjectiveActor)
	{
		return false;
	}

	FName SubmittedInputId = NAME_None;
	if (Key == EKeys::One || Key == EKeys::NumPadOne)
	{
		SubmittedInputId = ObjectiveActor->GetInteractionMode() == EHorrorCampaignInteractionMode::CircuitWiring
			? FName(TEXT("蓝色端子"))
			: FName(TEXT("齿轮1"));
	}
	else if (Key == EKeys::Two || Key == EKeys::NumPadTwo)
	{
		SubmittedInputId = ObjectiveActor->GetInteractionMode() == EHorrorCampaignInteractionMode::CircuitWiring
			? FName(TEXT("红色端子"))
			: FName(TEXT("齿轮2"));
	}
	else if (Key == EKeys::Three || Key == EKeys::NumPadThree)
	{
		SubmittedInputId = ObjectiveActor->GetInteractionMode() == EHorrorCampaignInteractionMode::CircuitWiring
			? FName(TEXT("黄色端子"))
			: FName(TEXT("齿轮3"));
	}
	else if (Key == EKeys::E || Key == EKeys::Enter || Key == EKeys::SpaceBar || Key == EKeys::Gamepad_FaceButton_Bottom)
	{
		SubmittedInputId = ObjectiveActor->GetExpectedAdvancedInputId();
	}
	else
	{
		return false;
	}

	if (SubmittedInputId.IsNone())
	{
		return true;
	}

	ObjectiveActor->SubmitAdvancedInteractionInput(SubmittedInputId, GetPawn() ? Cast<AActor>(GetPawn()) : Cast<AActor>(this));
	if (!ObjectiveActor->IsAdvancedInteractionActive() || ObjectiveActor->IsCompleted())
	{
		ActiveAdvancedInteractionObjective.Reset();
	}
	RefreshDay1HUDState();
	return true;
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
	ShowPlayerMessage(NSLOCTEXT("HorrorPlayerController", "PasswordCancelled", "门禁码输入已取消。"), FLinearColor(0.75f, 0.78f, 0.80f), 1.5f);
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
		? FString(TEXT("在环境中寻找门禁码。"))
		: Door->GetPasswordHint().ToString();
	const AHorrorPlayerCharacter* HorrorPlayerCharacter = Cast<AHorrorPlayerCharacter>(GetPawn());
	const UNoteRecorderComponent* NoteRecorder = HorrorPlayerCharacter ? HorrorPlayerCharacter->GetNoteRecorderComponent() : nullptr;
	if (NoteRecorder && NoteRecorder->GetRecordedNoteCount() > 0 && !HintText.Contains(TEXT("J键/Tab键")))
	{
		HintText += TEXT(" 按 J键/Tab键 查看已记录笔记。");
	}

	if (ADay1SliceHUD* Day1HUD = GetDay1SliceHUD())
	{
		Day1HUD->ShowPasswordPrompt(
			NSLOCTEXT("HorrorPlayerController", "AccessCodeTitle", "门禁码"),
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
				TEXT("需要门禁码\n输入：%s\n数字键 + 回车确认，退格删除，取消键取消。提示：%s"),
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
				NSLOCTEXT("HorrorPlayerController", "WrongPassword", "密码错误。"),
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
