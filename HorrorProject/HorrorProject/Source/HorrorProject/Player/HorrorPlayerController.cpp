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
#include "Components/SpotLightComponent.h"
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
	constexpr float FirstAnomalyFallbackScanCacheSeconds = 0.35f;
	constexpr float Day1HUDFullRefreshIntervalSeconds = 0.12f;
	const FName RuntimeFlashlightComponentName(TEXT("RuntimeFlashlight"));
	const FName RuntimeFlashlightSpotLightName(TEXT("RuntimeFlashlightSpotLight"));
	constexpr float RuntimeFlashlightIntensityLumens = 950.0f;
	constexpr float RuntimeFlashlightAttenuationRadiusCm = 1150.0f;
	constexpr float RuntimeFlashlightInnerConeAngleDegrees = 19.0f;
	constexpr float RuntimeFlashlightOuterConeAngleDegrees = 46.0f;
	const FVector RuntimeFlashlightRelativeLocation(30.0f, 17.5f, -5.0f);
	const FRotator RuntimeFlashlightRelativeRotation(-18.6f, -1.3f, 5.26f);

	int32 PriorityForObjectiveFeedbackSeverity(EHorrorObjectiveFeedbackSeverity Severity)
	{
		switch (Severity)
		{
			case EHorrorObjectiveFeedbackSeverity::Critical:
				return 100;
			case EHorrorObjectiveFeedbackSeverity::Failure:
				return 90;
			case EHorrorObjectiveFeedbackSeverity::Warning:
				return 70;
			case EHorrorObjectiveFeedbackSeverity::Success:
				return 60;
			case EHorrorObjectiveFeedbackSeverity::Info:
			default:
				return 30;
		}
	}

	FGameplayTag AdvancedCircuitSuccessEventTag()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Circuit.Success")), false);
	}

	FGameplayTag AdvancedCircuitFailureEventTag()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Circuit.Failure")), false);
	}

	FGameplayTag AdvancedGearSuccessEventTag()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Gear.Success")), false);
	}

	FGameplayTag AdvancedGearFailureEventTag()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Gear.Failure")), false);
	}

	FGameplayTag AdvancedSpectralScanSuccessEventTag()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SpectralScan.Success")), false);
	}

	FGameplayTag AdvancedSpectralScanFailureEventTag()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SpectralScan.Failure")), false);
	}

	FGameplayTag AdvancedSignalTuningSuccessEventTag()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SignalTuning.Success")), false);
	}

	FGameplayTag AdvancedSignalTuningFailureEventTag()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SignalTuning.Failure")), false);
	}

	bool TryResolveAdvancedInteractionFeedback(const FGameplayTag& EventTag, FText& OutFeedback, FLinearColor& OutColor)
	{
		if (EventTag == AdvancedCircuitSuccessEventTag())
		{
			OutFeedback = NSLOCTEXT("HorrorPlayerController", "CircuitAdvancedSuccess", "蓝色电弧接入成功，线路正在恢复。");
			OutColor = FLinearColor(0.28f, 0.82f, 1.0f);
			return true;
		}

		if (EventTag == AdvancedCircuitFailureEventTag())
		{
			OutFeedback = NSLOCTEXT("HorrorPlayerController", "CircuitAdvancedFailure", "接线失败，红色火花让进度回退。");
			OutColor = FLinearColor(1.0f, 0.24f, 0.12f);
			return true;
		}

		if (EventTag == AdvancedGearSuccessEventTag())
		{
			OutFeedback = NSLOCTEXT("HorrorPlayerController", "GearAdvancedSuccess", "齿轮重新咬合，机械节律恢复。");
			OutColor = FLinearColor(0.98f, 0.72f, 0.26f);
			return true;
		}

		if (EventTag == AdvancedGearFailureEventTag())
		{
			OutFeedback = NSLOCTEXT("HorrorPlayerController", "GearAdvancedFailure", "齿轮卡死，校准暂停三秒。");
			OutColor = FLinearColor(1.0f, 0.32f, 0.16f);
			return true;
		}

		if (EventTag == AdvancedSpectralScanSuccessEventTag())
		{
			OutFeedback = NSLOCTEXT("HorrorPlayerController", "SpectralScanAdvancedSuccess", "波段锁定，异常频谱正在写入黑盒。");
			OutColor = FLinearColor(0.34f, 1.0f, 0.72f);
			return true;
		}

		if (EventTag == AdvancedSpectralScanFailureEventTag())
		{
			OutFeedback = NSLOCTEXT("HorrorPlayerController", "SpectralScanAdvancedFailure", "扫描偏离，噪点反冲让稳定度下降。");
			OutColor = FLinearColor(0.96f, 0.28f, 0.86f);
			return true;
		}

		if (EventTag == AdvancedSignalTuningSuccessEventTag())
		{
			OutFeedback = NSLOCTEXT("HorrorPlayerController", "SignalTuningAdvancedSuccess", "频率对齐，黑盒回放正在变清晰。");
			OutColor = FLinearColor(1.0f, 0.86f, 0.34f);
			return true;
		}

		if (EventTag == AdvancedSignalTuningFailureEventTag())
		{
			OutFeedback = NSLOCTEXT("HorrorPlayerController", "SignalTuningAdvancedFailure", "调谐偏离，回放撕裂让进度回退。");
			OutColor = FLinearColor(1.0f, 0.38f, 0.24f);
			return true;
		}

		return false;
	}

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

		const AController* PawnController = Pawn.GetController();
		const FRotator ControlRotation = PawnController ? PawnController->GetControlRotation() : Pawn.GetActorRotation();
		const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
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
		FHorrorObjectiveNavigationState NavigationState;
		if (!GameMode.BuildCurrentCampaignObjectiveNavigationState(&Pawn, NavigationState))
		{
			return FText::GetEmpty();
		}

		return NavigationState.StatusText;
	}

	bool TryResolveAdvancedInteractionOptionIndexForKey(const FKey& Key, int32& OutOptionIndex)
	{
		if (Key == EKeys::One || Key == EKeys::NumPadOne || Key == EKeys::A || Key == EKeys::Gamepad_DPad_Left)
		{
			OutOptionIndex = 0;
			return true;
		}

		if (Key == EKeys::Two || Key == EKeys::NumPadTwo || Key == EKeys::S || Key == EKeys::Gamepad_DPad_Up)
		{
			OutOptionIndex = 1;
			return true;
		}

		if (Key == EKeys::Three || Key == EKeys::NumPadThree || Key == EKeys::D || Key == EKeys::Gamepad_DPad_Right)
		{
			OutOptionIndex = 2;
			return true;
		}

		return false;
	}

	bool TryBuildSignalTuningCommandForKey(const FKey& Key, FHorrorAdvancedInteractionInputCommand& OutCommand)
	{
		if (Key == EKeys::A || Key == EKeys::Gamepad_DPad_Left || Key == EKeys::Left)
		{
			OutCommand.CommandType = EHorrorAdvancedInteractionCommandType::AdjustAxis;
			OutCommand.AxisValue = -1.0f;
			OutCommand.HoldSeconds = 0.18f;
			OutCommand.VisualSlotIndex = 0;
			return true;
		}

		if (Key == EKeys::D || Key == EKeys::Gamepad_DPad_Right || Key == EKeys::Right)
		{
			OutCommand.CommandType = EHorrorAdvancedInteractionCommandType::AdjustAxis;
			OutCommand.AxisValue = 1.0f;
			OutCommand.HoldSeconds = 0.18f;
			OutCommand.VisualSlotIndex = 2;
			return true;
		}

		if (Key == EKeys::S || Key == EKeys::Enter || Key == EKeys::SpaceBar || Key == EKeys::Gamepad_FaceButton_Bottom || Key == EKeys::Gamepad_DPad_Up)
		{
			OutCommand.CommandType = EHorrorAdvancedInteractionCommandType::Confirm;
			OutCommand.VisualSlotIndex = 1;
			return true;
		}

		return false;
	}

	bool TryBuildSpectralScanCommandForKey(const FKey& Key, FHorrorAdvancedInteractionInputCommand& OutCommand)
	{
		if (Key == EKeys::A || Key == EKeys::Gamepad_DPad_Left || Key == EKeys::Left)
		{
			OutCommand.CommandType = EHorrorAdvancedInteractionCommandType::AdjustAxis;
			OutCommand.AxisValue = -1.0f;
			OutCommand.HoldSeconds = 0.45f;
			OutCommand.VisualSlotIndex = 0;
			return true;
		}

		if (Key == EKeys::D || Key == EKeys::Gamepad_DPad_Right || Key == EKeys::Right)
		{
			OutCommand.CommandType = EHorrorAdvancedInteractionCommandType::AdjustAxis;
			OutCommand.AxisValue = 1.0f;
			OutCommand.HoldSeconds = 0.45f;
			OutCommand.VisualSlotIndex = 2;
			return true;
		}

		if (Key == EKeys::S || Key == EKeys::Enter || Key == EKeys::SpaceBar || Key == EKeys::Gamepad_FaceButton_Bottom || Key == EKeys::Gamepad_DPad_Up)
		{
			OutCommand.CommandType = EHorrorAdvancedInteractionCommandType::Confirm;
			OutCommand.VisualSlotIndex = 1;
			return true;
		}

		return false;
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
	ClearFirstAnomalyFallbackCandidateCache();
	Super::EndPlay(EndPlayReason);
}

void AHorrorPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);
	ClearFirstAnomalyFallbackCandidateCache();
	EnsurePawnFlashlight(aPawn);
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
	ClearFirstAnomalyFallbackCandidateCache();
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

bool AHorrorPlayerController::TryPromptActiveAdvancedInteractionSelection()
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

	const bool bPrompted = ObjectiveActor->PromptForExplicitAdvancedInteractionSelection();
	RefreshDay1HUDState();
	return bPrompted;
}

void AHorrorPlayerController::NotifyAdvancedInteractionObjectiveOpened(AHorrorCampaignObjectiveActor* ObjectiveActor)
{
	if (!ObjectiveActor || !ObjectiveActor->IsAdvancedInteractionActive() || ObjectiveActor->IsCompleted())
	{
		return;
	}

	ActiveAdvancedInteractionObjective = ObjectiveActor;
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
		else if (TryHandleCampaignNavigationFocusKey(Params.Key))
		{
			return true;
		}
		else if (Params.Key == EKeys::J || Params.Key == EKeys::Tab)
		{
			return ToggleDay1NotesJournal();
		}
		else if (Params.Key == EKeys::F)
		{
			if (TogglePawnFlashlight())
			{
				return true;
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

bool AHorrorPlayerController::ShouldPrioritizeCampaignNavigationForTests(const AHorrorGameModeBase* HorrorGameMode) const
{
	return ShouldPrioritizeCampaignNavigation(HorrorGameMode);
}

bool AHorrorPlayerController::UpdateDay1RuntimeStateForTests(float DeltaSeconds)
{
	return UpdateDay1RuntimeState(DeltaSeconds);
}

int32 AHorrorPlayerController::GetCachedFirstAnomalyFallbackCandidateCountForTests() const
{
	int32 ValidCandidateCount = 0;
	for (const TWeakObjectPtr<AFoundFootageObjectiveInteractable>& Candidate : CachedFirstAnomalyFallbackCandidates)
	{
		if (Candidate.IsValid())
		{
			++ValidCandidateCount;
		}
	}
	return ValidCandidateCount;
}

void AHorrorPlayerController::ExpireFirstAnomalyFallbackCacheForTests()
{
	LastFirstAnomalyFallbackScanWorldSeconds = -1000000.0f;
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

UFlashlightComponent* AHorrorPlayerController::EnsurePawnFlashlight(APawn* PawnToPrepare) const
{
	if (!PawnToPrepare)
	{
		return nullptr;
	}

	UFlashlightComponent* Flashlight = nullptr;
	if (AHorrorPlayerCharacter* HorrorPlayerCharacter = Cast<AHorrorPlayerCharacter>(PawnToPrepare))
	{
		Flashlight = HorrorPlayerCharacter->GetFlashlightComponent();
	}

	if (!Flashlight)
	{
		Flashlight = PawnToPrepare->FindComponentByClass<UFlashlightComponent>();
	}

	if (!Flashlight)
	{
		Flashlight = NewObject<UFlashlightComponent>(PawnToPrepare, RuntimeFlashlightComponentName);
		if (Flashlight)
		{
			Flashlight->RegisterComponent();
		}
	}

	if (!Flashlight)
	{
		return nullptr;
	}

	USpotLightComponent* SpotLight = Flashlight->GetBoundSpotLight();
	if (!SpotLight)
	{
		if (AHorrorCharacter* HorrorCharacter = Cast<AHorrorCharacter>(PawnToPrepare))
		{
			SpotLight = HorrorCharacter->GetSpotLight();
		}
	}

	if (!SpotLight)
	{
		SpotLight = PawnToPrepare->FindComponentByClass<USpotLightComponent>();
	}

	if (!SpotLight)
	{
		SpotLight = NewObject<USpotLightComponent>(PawnToPrepare, RuntimeFlashlightSpotLightName);
		if (SpotLight)
		{
			if (USceneComponent* PawnRoot = PawnToPrepare->GetRootComponent())
			{
				SpotLight->SetupAttachment(PawnRoot);
			}
			SpotLight->SetRelativeLocationAndRotation(RuntimeFlashlightRelativeLocation, RuntimeFlashlightRelativeRotation);
			SpotLight->SetIntensityUnits(ELightUnits::Lumens);
			SpotLight->SetIntensity(RuntimeFlashlightIntensityLumens);
			SpotLight->SetAttenuationRadius(RuntimeFlashlightAttenuationRadiusCm);
			SpotLight->SetInnerConeAngle(RuntimeFlashlightInnerConeAngleDegrees);
			SpotLight->SetOuterConeAngle(RuntimeFlashlightOuterConeAngleDegrees);
			SpotLight->RegisterComponent();
		}
	}

	if (SpotLight)
	{
		if (SpotLight->Intensity <= 0.0f)
		{
			SpotLight->SetIntensity(RuntimeFlashlightIntensityLumens);
		}
		if (SpotLight->AttenuationRadius <= 0.0f)
		{
			SpotLight->SetAttenuationRadius(RuntimeFlashlightAttenuationRadiusCm);
			SpotLight->AttenuationRadius = RuntimeFlashlightAttenuationRadiusCm;
		}
		Flashlight->BindSpotLight(SpotLight);
	}

	return Flashlight;
}

bool AHorrorPlayerController::TogglePawnFlashlight()
{
	UFlashlightComponent* Flashlight = EnsurePawnFlashlight(GetPawn());
	if (!Flashlight)
	{
		return false;
	}

	Flashlight->ToggleFlashlight();
	return true;
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

	FText AdvancedInteractionFeedback;
	FLinearColor AdvancedInteractionColor = FLinearColor::White;
	if (TryResolveAdvancedInteractionFeedback(Message.EventTag, AdvancedInteractionFeedback, AdvancedInteractionColor))
	{
		Day1HUD->ShowTransientMessage(AdvancedInteractionFeedback, AdvancedInteractionColor, 1.8f);
		return;
	}

	if (Message.EventTag == HorrorSaveTags::CheckpointSavedEvent())
	{
		Day1HUD->ShowAutosaveIndicator(NSLOCTEXT("HorrorPlayerController", "CheckpointSaved", "检查点已保存。"));
		return;
	}

	if (Message.EventTag == HorrorSaveTags::CheckpointSaveFailedEvent())
	{
		Day1HUD->ShowTransientMessage(
			NSLOCTEXT("HorrorPlayerController", "CheckpointSaveFailed", "检查点保存失败。"),
			FLinearColor(1.0f, 0.78f, 0.15f),
			3.0f);
		return;
	}

	const int32 IncomingToastPriority = ResolveObjectiveNotificationPriority(Message);
	const FText Title = !Message.DebugLabel.IsEmpty()
		? Message.DebugLabel
		: NSLOCTEXT("HorrorPlayerController", "ObjectiveUpdated", "目标已更新");
	const FText Hint = !Message.ObjectiveHint.IsEmpty()
		? Message.ObjectiveHint
		: BuildObjectivePrompt();

	FHorrorObjectiveNotification Notification;
	Notification.Title = Title;
	Notification.Hint = Hint;
	Notification.Priority = IncomingToastPriority;
	Notification.Severity = Message.FeedbackSeverity;
	Notification.bRetryable = Message.bRetryable;
	Notification.DisplaySeconds = FMath::Max(0.1f, Message.DisplaySeconds);
	Notification.SourceId = Message.SourceId;
	Notification.EventTag = Message.EventTag;
	ShowObjectiveNotificationNow(*Day1HUD, Notification);

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
		NSLOCTEXT("HorrorPlayerController", "NoteRecorded", "笔记已记录，按笔记键/物品栏键查看。"),
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
	Day1HUDFullRefreshAccumulatorSeconds += FMath::Clamp(DeltaSeconds, 0.0f, 1.0f);
	const bool bRunFullHUDRefresh = Day1HUDFullRefreshAccumulatorSeconds >= Day1HUDFullRefreshIntervalSeconds;
	if (bRunFullHUDRefresh)
	{
		Day1HUDFullRefreshAccumulatorSeconds = 0.0f;
	}
	RefreshDay1HUDState(bRunFullHUDRefresh);
	bool bFlushedQueuedNotification = false;
	if (ADay1SliceHUD* Day1HUD = GetDay1SliceHUD())
	{
		bFlushedQueuedNotification = TryFlushQueuedObjectiveNotification(*Day1HUD, DeltaSeconds);
	}
	return bCapturedFocusedAnomaly || bFlushedQueuedNotification;
}

void AHorrorPlayerController::RefreshDay1HUDState(bool bForceFullRefresh)
{
	BindPawnNoteRecorder();
	BindObjectiveEventBus();

	if (ADay1SliceHUD* Day1HUD = GetDay1SliceHUD())
	{
		const AHorrorPlayerCharacter* HorrorPlayerCharacter = Cast<AHorrorPlayerCharacter>(GetPawn());
		const AHorrorGameModeBase* HorrorGameMode = Cast<AHorrorGameModeBase>(UGameplayStatics::GetGameMode(this));
		RefreshDay1HUDRealtimeState(*Day1HUD, HorrorPlayerCharacter, HorrorGameMode);

		if (bForceFullRefresh)
		{
			RefreshDay1HUDObjectiveState(*Day1HUD, HorrorPlayerCharacter, HorrorGameMode);
		}

		RefreshAnomalyCaptureHUD(*Day1HUD, HorrorPlayerCharacter, HorrorGameMode);
		RefreshAdvancedInteractionHUD(*Day1HUD, HorrorPlayerCharacter);
	}

	if (bForceFullRefresh)
	{
		RefreshInteractionPrompt();

		if (bDay1NotesJournalOpen)
		{
			RefreshDay1NotesJournalHUD();
		}
	}
}

void AHorrorPlayerController::RefreshDay1HUDRealtimeState(
	ADay1SliceHUD& Day1HUD,
	const AHorrorPlayerCharacter* HorrorPlayerCharacter,
	const AHorrorGameModeBase* HorrorGameMode) const
{
	const UQuantumCameraComponent* QuantumCamera = HorrorPlayerCharacter ? HorrorPlayerCharacter->GetQuantumCameraComponent() : nullptr;
	const UCameraBatteryComponent* CameraBattery = QuantumCamera ? QuantumCamera->GetBatteryComponent() : nullptr;
	const UFearComponent* Fear = HorrorPlayerCharacter ? HorrorPlayerCharacter->GetFearComponent() : nullptr;
	const ADeepWaterStationRouteKit* RouteKit = HorrorGameMode ? HorrorGameMode->GetRuntimeRouteKit() : nullptr;
	const AHorrorEncounterDirector* EncounterDirector = HorrorGameMode ? HorrorGameMode->GetRuntimeEncounterDirector() : nullptr;
	const bool bDanger = (RouteKit && RouteKit->IsRouteGatedByEncounter()) || (EncounterDirector && EncounterDirector->IsRouteGated());

	Day1HUD.SetSurvivalStatus(
		QuantumCamera && QuantumCamera->IsCameraAcquired(),
		QuantumCamera && QuantumCamera->IsCameraEnabled(),
		QuantumCamera && QuantumCamera->IsCameraMode(EQuantumCameraMode::Recording),
		Fear ? Fear->GetFearPercent() : 0.0f,
		HorrorPlayerCharacter ? HorrorPlayerCharacter->GetSprintPercent() : 1.0f,
		bDanger);
	if (CameraBattery)
	{
		Day1HUD.SetBodycamBatteryStatus(CameraBattery->GetBatteryPercentage(), CameraBattery->IsBatteryLow());
	}
	else
	{
		Day1HUD.ClearBodycamBatteryStatus();
	}
}

void AHorrorPlayerController::RefreshDay1HUDObjectiveState(
	ADay1SliceHUD& Day1HUD,
	const AHorrorPlayerCharacter* HorrorPlayerCharacter,
	const AHorrorGameModeBase* HorrorGameMode)
{
	const ADeepWaterStationRouteKit* RouteKit = HorrorGameMode ? HorrorGameMode->GetRuntimeRouteKit() : nullptr;

	if (HorrorGameMode)
	{
		Day1HUD.SetObjectiveTracker(HorrorGameMode->BuildObjectiveTrackerSnapshot());
	}
	else
	{
		Day1HUD.SetCurrentObjective(BuildObjectivePrompt());
	}

	const bool bPrioritizeCampaignNavigation = ShouldPrioritizeCampaignNavigation(HorrorGameMode);
	if (HorrorPlayerCharacter && HorrorGameMode && bPrioritizeCampaignNavigation)
	{
		FHorrorObjectiveNavigationState NavigationState;
		if (HorrorGameMode->BuildCurrentCampaignObjectiveNavigationState(HorrorPlayerCharacter, NavigationState))
		{
			Day1HUD.SetObjectiveNavigationState(NavigationState);
		}
		else
		{
			Day1HUD.ClearObjectiveNavigation();
		}
	}
	else if (HorrorPlayerCharacter && HorrorGameMode && RouteKit)
	{
		const FText NavigationText = BuildDay1NavigationText(*HorrorPlayerCharacter, *HorrorGameMode, *RouteKit);
		if (!NavigationText.IsEmpty())
		{
			Day1HUD.SetObjectiveNavigation(NavigationText);
		}
		else
		{
			Day1HUD.ClearObjectiveNavigation();
		}
	}
	else if (HorrorPlayerCharacter && HorrorGameMode)
	{
		FHorrorObjectiveNavigationState NavigationState;
		if (HorrorGameMode->BuildCurrentCampaignObjectiveNavigationState(HorrorPlayerCharacter, NavigationState))
		{
			Day1HUD.SetObjectiveNavigationState(NavigationState);
		}
		else
		{
			Day1HUD.ClearObjectiveNavigation();
		}
	}
	else
	{
		Day1HUD.ClearObjectiveNavigation();
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
	FHorrorInteractionContext InteractionContext;
	if (InteractionComponent && InteractionComponent->GetFocusedInteractionContext(InteractionContext))
	{
		Day1HUD->SetInteractionContext(InteractionContext);
	}
	else
	{
		Day1HUD->ClearInteractionPrompt();
	}
}

bool AHorrorPlayerController::ShouldPrioritizeCampaignNavigation(const AHorrorGameModeBase* HorrorGameMode) const
{
	return HorrorGameMode
		&& HorrorGameMode->ShouldExposeCampaignObjectivesToHUD();
}

AHorrorCampaignObjectiveActor* AHorrorPlayerController::ResolveActiveAdvancedInteractionObjective(const AHorrorPlayerCharacter* HorrorPlayerCharacter)
{
	const AHorrorGameModeBase* HorrorGameMode = Cast<AHorrorGameModeBase>(UGameplayStatics::GetGameMode(this));
	const auto CanUseAdvancedObjective = [HorrorGameMode](const AHorrorCampaignObjectiveActor* ObjectiveActor)
	{
		return ObjectiveActor
			&& (ObjectiveActor->IsAdvancedInteractionActive() || ObjectiveActor->IsObjectiveFailedRetryable())
			&& !ObjectiveActor->IsCompleted()
			&& (!HorrorGameMode || HorrorGameMode->CanCompleteCampaignObjective(ObjectiveActor->GetChapterId(), ObjectiveActor->GetObjectiveId()));
	};

	if (AHorrorCampaignObjectiveActor* CachedObjective = ActiveAdvancedInteractionObjective.Get())
	{
		if (CanUseAdvancedObjective(CachedObjective))
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

	if (CanUseAdvancedObjective(FocusedObjective))
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

AFoundFootageObjectiveInteractable* AHorrorPlayerController::ResolveFocusedFirstAnomalyTarget(const AHorrorPlayerCharacter& HorrorPlayerCharacter)
{
	if (const UInteractionComponent* InteractionComponent = HorrorPlayerCharacter.GetInteractionComponent())
	{
		FHitResult FocusedHit;
		UObject* FocusedTargetObject = nullptr;
		if (InteractionComponent->FindFocusedInteractionTarget(FocusedHit, FocusedTargetObject))
		{
			if (AFoundFootageObjectiveInteractable* FocusedTarget = Cast<AFoundFootageObjectiveInteractable>(FocusedTargetObject))
			{
				if (IsFocusedFirstAnomalyTarget(FocusedTarget))
				{
					return FocusedTarget;
				}
			}

			if (AFoundFootageObjectiveInteractable* FocusedActor = Cast<AFoundFootageObjectiveInteractable>(FocusedHit.GetActor()))
			{
				if (IsFocusedFirstAnomalyTarget(FocusedActor))
				{
					return FocusedActor;
				}
			}
		}
	}

	const AHorrorGameModeBase* HorrorGameMode = Cast<AHorrorGameModeBase>(UGameplayStatics::GetGameMode(this));
	const ADeepWaterStationRouteKit* RouteKit = HorrorGameMode ? HorrorGameMode->GetRuntimeRouteKit() : nullptr;
	if (RouteKit)
	{
		if (AFoundFootageObjectiveInteractable* RouteCandidate =
			ResolveFocusedFirstAnomalyTargetFromCandidates(HorrorPlayerCharacter, RouteKit->GetSpawnedObjectiveInteractables()))
		{
			return RouteCandidate;
		}
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	if (!IsFirstAnomalyFallbackCacheFresh(World))
	{
		RefreshFirstAnomalyFallbackCandidateCache(World);
	}

	TArray<AFoundFootageObjectiveInteractable*> FallbackCandidates;
	FallbackCandidates.Reserve(CachedFirstAnomalyFallbackCandidates.Num());
	for (int32 CandidateIndex = CachedFirstAnomalyFallbackCandidates.Num() - 1; CandidateIndex >= 0; --CandidateIndex)
	{
		AFoundFootageObjectiveInteractable* Candidate = CachedFirstAnomalyFallbackCandidates[CandidateIndex].Get();
		if (!Candidate)
		{
			CachedFirstAnomalyFallbackCandidates.RemoveAtSwap(CandidateIndex, 1, EAllowShrinking::No);
			continue;
		}

		FallbackCandidates.Add(Candidate);
	}

	return ResolveFocusedFirstAnomalyTargetFromCandidates(HorrorPlayerCharacter, FallbackCandidates);
}

AFoundFootageObjectiveInteractable* AHorrorPlayerController::ResolveFocusedFirstAnomalyTargetFromCandidates(
	const AHorrorPlayerCharacter& HorrorPlayerCharacter,
	TConstArrayView<AFoundFootageObjectiveInteractable*> Candidates) const
{
	const UCameraComponent* FirstPersonCamera = HorrorPlayerCharacter.GetFirstPersonCameraComponent();
	if (!FirstPersonCamera)
	{
		return nullptr;
	}

	AFoundFootageObjectiveInteractable* BestCandidate = nullptr;
	float BestFocusScore = -1.0f;
	const FVector CameraLocation = FirstPersonCamera->GetComponentLocation();
	const FVector CameraForward = FirstPersonCamera->GetForwardVector();
	const float RequiredFocusDot = FMath::Cos(FMath::DegreesToRadians(FirstAnomalyCaptureMaxFocusAngleDegrees));

	for (AFoundFootageObjectiveInteractable* Candidate : Candidates)
	{
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

bool AHorrorPlayerController::IsFirstAnomalyFallbackCacheFresh(const UWorld* World) const
{
	if (!World || CachedFirstAnomalyFallbackCandidates.IsEmpty())
	{
		return false;
	}

	return (World->GetTimeSeconds() - LastFirstAnomalyFallbackScanWorldSeconds) < FirstAnomalyFallbackScanCacheSeconds;
}

void AHorrorPlayerController::RefreshFirstAnomalyFallbackCandidateCache(UWorld* World)
{
	CachedFirstAnomalyFallbackCandidates.Reset();
	LastFirstAnomalyFallbackScanWorldSeconds = World ? World->GetTimeSeconds() : -1000000.0f;

	if (!World)
	{
		return;
	}

#if WITH_DEV_AUTOMATION_TESTS
	++FirstAnomalyFallbackWorldScanCountForTests;
#endif

	for (TActorIterator<AFoundFootageObjectiveInteractable> It(World); It; ++It)
	{
		if (IsFocusedFirstAnomalyTarget(*It))
		{
			CachedFirstAnomalyFallbackCandidates.Add(*It);
		}
	}
}

void AHorrorPlayerController::ClearFirstAnomalyFallbackCandidateCache()
{
	CachedFirstAnomalyFallbackCandidates.Reset();
	LastFirstAnomalyFallbackScanWorldSeconds = -1000000.0f;
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
	ClearFirstAnomalyFallbackCandidateCache();
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

bool AHorrorPlayerController::TryHandleCampaignNavigationFocusKey(const FKey& Key)
{
	if (bDay1CompletionInputLocked || bDay1PauseMenuOpen || bDay1NotesJournalOpen || PendingPasswordDoor.IsValid())
	{
		return false;
	}

	const int32 Direction = Key == EKeys::RightBracket
		? 1
		: Key == EKeys::LeftBracket
		? -1
		: 0;
	if (Direction == 0)
	{
		return false;
	}

	AHorrorGameModeBase* HorrorGameMode = Cast<AHorrorGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (!HorrorGameMode || !HorrorGameMode->ShouldExposeCampaignObjectivesToHUD())
	{
		return false;
	}

	if (!HorrorGameMode->CycleCampaignNavigationFocus(Direction))
	{
		return false;
	}

	RefreshDay1HUDState();
	return true;
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

	const FHorrorAdvancedInteractionHUDState AdvancedState = ObjectiveActor->BuildAdvancedInteractionHUDState();
	if (ObjectiveActor->IsObjectiveFailedRetryable())
	{
		if (Key == EKeys::E || Key == EKeys::Enter || Key == EKeys::SpaceBar || Key == EKeys::Gamepad_FaceButton_Bottom)
		{
			const FHitResult EmptyHit;
			const bool bRestarted = ObjectiveActor->Interact_Implementation(GetPawn() ? Cast<AActor>(GetPawn()) : Cast<AActor>(this), EmptyHit);
			if (bRestarted && ObjectiveActor->IsAdvancedInteractionActive())
			{
				ActiveAdvancedInteractionObjective = ObjectiveActor;
			}
			RefreshDay1HUDState();
			return bRestarted;
		}

		return false;
	}

	if (AdvancedState.Mode == EHorrorCampaignInteractionMode::SignalTuning)
	{
		FHorrorAdvancedInteractionInputCommand SignalCommand;
		if (TryBuildSignalTuningCommandForKey(Key, SignalCommand))
		{
			ObjectiveActor->SubmitAdvancedInteractionCommand(SignalCommand, GetPawn() ? Cast<AActor>(GetPawn()) : Cast<AActor>(this));
			if (!ObjectiveActor->IsAdvancedInteractionActive() || ObjectiveActor->IsCompleted())
			{
				ActiveAdvancedInteractionObjective.Reset();
			}
			RefreshDay1HUDState();
			return true;
		}
	}

	if (AdvancedState.Mode == EHorrorCampaignInteractionMode::SpectralScan)
	{
		FHorrorAdvancedInteractionInputCommand ScanCommand;
		if (TryBuildSpectralScanCommandForKey(Key, ScanCommand))
		{
			ObjectiveActor->SubmitAdvancedInteractionCommand(ScanCommand, GetPawn() ? Cast<AActor>(GetPawn()) : Cast<AActor>(this));
			if (!ObjectiveActor->IsAdvancedInteractionActive() || ObjectiveActor->IsCompleted())
			{
				ActiveAdvancedInteractionObjective.Reset();
			}
			RefreshDay1HUDState();
			return true;
		}
	}

	if (Key == EKeys::E || Key == EKeys::Enter || Key == EKeys::SpaceBar || Key == EKeys::Gamepad_FaceButton_Bottom)
	{
		const bool bPrompted = ObjectiveActor->PromptForExplicitAdvancedInteractionSelection();
		RefreshDay1HUDState();
		return bPrompted;
	}

	int32 SubmittedOptionIndex = INDEX_NONE;
	if (!TryResolveAdvancedInteractionOptionIndexForKey(Key, SubmittedOptionIndex))
	{
		return false;
	}

	const FName SubmittedInputId = AdvancedState.InputOptions.IsValidIndex(SubmittedOptionIndex)
		? AdvancedState.InputOptions[SubmittedOptionIndex].InputId
		: NAME_None;
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

int32 AHorrorPlayerController::ResolveObjectiveNotificationPriority(const FHorrorEventMessage& Message) const
{
	const int32 SeverityPriority = PriorityForObjectiveFeedbackSeverity(Message.FeedbackSeverity);
	if (SeverityPriority > 30)
	{
		return SeverityPriority;
	}

	if (Message.EventTag == HorrorDay1Tags::Day1CompletedEvent())
	{
		return 100;
	}

	if (Message.EventTag == HorrorDay1Tags::PlayerFailureEvent()
		|| Message.EventTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.ObjectiveFailed")), false)))
	{
		return 90;
	}

	if (Message.EventTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.ObjectiveCompleted")), false)))
	{
		return 60;
	}

	return 30;
}

bool AHorrorPlayerController::ShouldSuppressObjectiveToastForPriority(const ADay1SliceHUD& Day1HUD, int32 IncomingPriority) const
{
	const UWorld* World = GetWorld();
	const float WorldSeconds = World ? World->GetTimeSeconds() : 0.0f;
	return ActiveObjectiveToastExpireWorldSeconds > WorldSeconds
		&& !Day1HUD.GetObjectiveToastTitleForTests().IsEmpty()
		&& IncomingPriority < ActiveObjectiveToastPriority;
}

void AHorrorPlayerController::EnqueueObjectiveNotification(const FHorrorObjectiveNotification& Notification)
{
	if (Notification.Title.IsEmpty() && Notification.Hint.IsEmpty())
	{
		return;
	}

	for (FHorrorObjectiveNotification& QueuedNotification : ObjectiveNotificationQueue)
	{
		if (QueuedNotification.SourceId == Notification.SourceId
			&& QueuedNotification.EventTag == Notification.EventTag)
		{
			QueuedNotification = Notification;
			return;
		}
	}

	ObjectiveNotificationQueue.Add(Notification);
	ObjectiveNotificationQueue.Sort([](const FHorrorObjectiveNotification& Left, const FHorrorObjectiveNotification& Right)
	{
		if (Left.Priority != Right.Priority)
		{
			return Left.Priority > Right.Priority;
		}

		return Left.SourceId.LexicalLess(Right.SourceId);
	});

	constexpr int32 MaxQueuedObjectiveNotifications = 4;
	if (ObjectiveNotificationQueue.Num() > MaxQueuedObjectiveNotifications)
	{
		ObjectiveNotificationQueue.SetNum(MaxQueuedObjectiveNotifications, EAllowShrinking::No);
	}
}

void AHorrorPlayerController::ShowObjectiveNotificationNow(ADay1SliceHUD& Day1HUD, const FHorrorObjectiveNotification& Notification)
{
	if (ShouldSuppressObjectiveToastForPriority(Day1HUD, Notification.Priority))
	{
		EnqueueObjectiveNotification(Notification);
		return;
	}

	Day1HUD.ShowObjectiveToast(Notification.Title, Notification.Hint, Notification.DisplaySeconds, Notification.Severity);
	ActiveObjectiveToastPriority = Notification.Priority;
	ActiveObjectiveToastExpireWorldSeconds = GetWorld()
		? GetWorld()->GetTimeSeconds() + FMath::Max(0.1f, Notification.DisplaySeconds)
		: -1.0f;
}

bool AHorrorPlayerController::TryFlushQueuedObjectiveNotification(ADay1SliceHUD& Day1HUD, float ElapsedSeconds)
{
	if (ObjectiveNotificationQueue.IsEmpty())
	{
		return false;
	}

	const UWorld* World = GetWorld();
	const float WorldSeconds = World ? World->GetTimeSeconds() : 0.0f;
	const float EffectiveWorldSeconds = WorldSeconds + FMath::Max(0.0f, ElapsedSeconds);
	if (ActiveObjectiveToastExpireWorldSeconds > EffectiveWorldSeconds && !Day1HUD.GetObjectiveToastTitleForTests().IsEmpty())
	{
		return false;
	}

	FHorrorObjectiveNotification Notification = ObjectiveNotificationQueue[0];
	ObjectiveNotificationQueue.RemoveAt(0, 1, EAllowShrinking::No);
	Day1HUD.ShowObjectiveToast(Notification.Title, Notification.Hint, Notification.DisplaySeconds, Notification.Severity);
	ActiveObjectiveToastPriority = Notification.Priority;
	ActiveObjectiveToastExpireWorldSeconds = World
		? EffectiveWorldSeconds + FMath::Max(0.1f, Notification.DisplaySeconds)
		: -1.0f;
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
	if (NoteRecorder && NoteRecorder->GetRecordedNoteCount() > 0 && !HintText.Contains(TEXT("笔记键/物品栏键")))
	{
		HintText += TEXT(" 按笔记键/物品栏键查看已记录笔记。");
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
