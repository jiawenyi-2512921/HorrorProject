// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/DoorInteractable.h"
#include "Interaction/InteractableInterface.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Audio/HorrorAudioSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

namespace
{
	UStaticMeshComponent* FindDoorMesh(ADoorInteractable& Door)
	{
		TArray<UStaticMeshComponent*> MeshComponents;
		Door.GetComponents<UStaticMeshComponent>(MeshComponents);
		for (UStaticMeshComponent* MeshComponent : MeshComponents)
		{
			if (MeshComponent && MeshComponent->GetName() == TEXT("DoorMesh"))
			{
				return MeshComponent;
			}
		}

		return nullptr;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDoorInteractableClearsDoorwayCollisionWhenOpenTest,
	"HorrorProject.Interaction.Door.ClearsDoorwayCollisionWhenOpen",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDoorInteractableClearsDoorwayCollisionWhenOpenTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for door collision coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();
	AActor* InstigatorActor = World->SpawnActor<AActor>();
	TestNotNull(TEXT("Door collision test should spawn a door."), Door);
	TestNotNull(TEXT("Door collision test should spawn an instigator."), InstigatorActor);
	if (!Door || !InstigatorActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Door->DispatchBeginPlay();
	TestFalse(TEXT("Idle closed doors should not spend a constant actor tick."), Door->IsActorTickEnabled());
	IInteractableInterface* DoorInterface = Cast<IInteractableInterface>(Door);
	TestNotNull(TEXT("Door collision test should use the native interactable interface path."), DoorInterface);
	if (!DoorInterface)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UStaticMeshComponent* DoorMesh = FindDoorMesh(*Door);
	TestNotNull(TEXT("Door collision test should find the DoorMesh component."), DoorMesh);
	if (!DoorMesh)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestEqual(TEXT("Closed doors should block the doorway."), DoorMesh->GetCollisionEnabled(), ECollisionEnabled::QueryAndPhysics);

	FHitResult Hit;
	TestTrue(TEXT("Closed door should accept an open interaction."), DoorInterface->Interact_Implementation(InstigatorActor, Hit));
	TestTrue(TEXT("Opening doors should enable actor tick only for animation."), Door->IsActorTickEnabled());
	TestEqual(TEXT("Opening doors should clear blocking collision immediately."), DoorMesh->GetCollisionEnabled(), ECollisionEnabled::NoCollision);

	for (int32 TickIndex = 0; TickIndex < 60; ++TickIndex)
	{
		Door->Tick(0.05f);
	}

	TestEqual(TEXT("Open doors should keep doorway collision clear."), Door->GetDoorState(), EDoorState::Open);
	TestFalse(TEXT("Open doors without auto-close should disable actor tick after animation settles."), Door->IsActorTickEnabled());
	TestEqual(TEXT("Open doors should not block the doorway."), DoorMesh->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
	TestTrue(TEXT("Open door should accept a close interaction."), DoorInterface->Interact_Implementation(InstigatorActor, Hit));
	TestTrue(TEXT("Closing doors should re-enable actor tick for animation."), Door->IsActorTickEnabled());
	TestEqual(TEXT("Closing doors should restore blocking collision."), DoorMesh->GetCollisionEnabled(), ECollisionEnabled::QueryAndPhysics);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDoorInteractablePasswordUnlockPublishesDoorOpenedEventTest,
	"HorrorProject.Interaction.Door.PasswordUnlockPublishesDoorOpenedEvent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDoorInteractablePasswordUnlockPublishesDoorOpenedEventTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for door event coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>();
	TestNotNull(TEXT("Door event test should create the horror event bus."), EventBus);
	TestNotNull(TEXT("Door event test should create the horror audio subsystem."), AudioSubsystem);
	if (!EventBus || !AudioSubsystem)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FGameplayTag DoorOpenedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Interaction.Door.Opened")), false);
	TestTrue(TEXT("Door opened gameplay tag should be registered."), DoorOpenedTag.IsValid());
	TestTrue(TEXT("Door opened event should have a Day1 audio mapping."), AudioSubsystem->HasEventMappingForTests(DoorOpenedTag));
	TestEqual(TEXT("Door opened event should route to site audio."), AudioSubsystem->GetEventMappingCategoryForTests(DoorOpenedTag), EHorrorAudioCategory::Site);

	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();
	AActor* InstigatorActor = World->SpawnActor<AActor>();
	TestNotNull(TEXT("Door event test should spawn a door."), Door);
	TestNotNull(TEXT("Door event test should spawn an instigator."), InstigatorActor);
	if (!Door || !InstigatorActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Door->Rename(TEXT("Door_Maintenance_01"));
	Door->ConfigurePassword(TEXT("0417"), FText::FromString(TEXT("Test hint")));
	Door->DispatchBeginPlay();

	int32 DoorOpenedEventCount = 0;
	FHorrorEventMessage LastDoorOpenedMessage;
	const FDelegateHandle EventHandle = EventBus->GetOnEventPublishedNative().AddLambda(
		[&DoorOpenedEventCount, &LastDoorOpenedMessage, DoorOpenedTag](const FHorrorEventMessage& Message)
		{
			if (Message.EventTag == DoorOpenedTag)
			{
				++DoorOpenedEventCount;
				LastDoorOpenedMessage = Message;
			}
		});

	TestFalse(TEXT("Wrong passwords should be rejected before publishing door opened feedback."), Door->SubmitPassword(InstigatorActor, TEXT("0000")));
	TestEqual(TEXT("Wrong passwords should not publish door opened feedback."), DoorOpenedEventCount, 0);
	TestEqual(TEXT("Wrong passwords should not add event bus history."), EventBus->GetHistory().Num(), 0);

	TestTrue(TEXT("Correct password should be accepted."), Door->SubmitPassword(InstigatorActor, TEXT("0417")));
	TestEqual(TEXT("Correct password should publish exactly one door opened event."), DoorOpenedEventCount, 1);
	TestTrue(TEXT("Door opened event should be retained in event bus history."), EventBus->GetHistory().Num() >= 1);
	TestEqual(TEXT("Door opened event should keep the door opened tag."), LastDoorOpenedMessage.EventTag, DoorOpenedTag);
	TestEqual(TEXT("Door opened event should use a stable door source id."), LastDoorOpenedMessage.SourceId, Door->GetFName());
	TestEqual(TEXT("Door opened event should identify the door actor as the source."), LastDoorOpenedMessage.SourceObject.Get(), Cast<UObject>(Door));

	TestTrue(TEXT("Submitting the same password while the door is already opening should remain accepted."), Door->SubmitPassword(InstigatorActor, TEXT("0417")));
	TestEqual(TEXT("Repeated password submissions should not duplicate door opened events."), DoorOpenedEventCount, 1);

	EventBus->GetOnEventPublishedNative().Remove(EventHandle);
	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDoorInteractablePasswordGateTest,
	"HorrorProject.Interaction.Door.PasswordGate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDoorInteractablePasswordGateTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for password door coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();
	AActor* InstigatorActor = World->SpawnActor<AActor>();
	TestNotNull(TEXT("Password door test should spawn a door."), Door);
	TestNotNull(TEXT("Password door test should spawn an instigator."), InstigatorActor);
	if (!Door || !InstigatorActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Door->ConfigurePassword(TEXT("0417"), FText::FromString(TEXT("Test hint")));
	Door->DispatchBeginPlay();

	TestTrue(TEXT("Password configured doors should require a password."), Door->RequiresPassword());
	TestFalse(TEXT("Password configured doors should start locked."), Door->IsPasswordUnlocked());
	TestEqual(TEXT("Password configured doors should block before unlock."), Door->GetDoorState(), EDoorState::Locked);

	FHitResult Hit;
	IInteractableInterface* DoorInterface = Cast<IInteractableInterface>(Door);
	TestNotNull(TEXT("Password door test should use the native interface path."), DoorInterface);
	if (!DoorInterface)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Locked password doors should still accept interaction for password entry."), DoorInterface->Interact_Implementation(InstigatorActor, Hit));
	TestEqual(TEXT("Interacting without a submitted password should keep the door locked."), Door->GetDoorState(), EDoorState::Locked);

	TestFalse(TEXT("Wrong passwords should be rejected."), Door->SubmitPassword(InstigatorActor, TEXT("0000")));
	TestEqual(TEXT("Wrong passwords should keep the door locked."), Door->GetDoorState(), EDoorState::Locked);

	TestTrue(TEXT("Correct passwords should be accepted."), Door->SubmitPassword(InstigatorActor, TEXT("0417")));
	TestTrue(TEXT("Correct passwords should unlock the door."), Door->IsPasswordUnlocked());
	TestEqual(TEXT("Correct passwords should begin opening the door."), Door->GetDoorState(), EDoorState::Opening);

	UStaticMeshComponent* DoorMesh = FindDoorMesh(*Door);
	TestNotNull(TEXT("Password door test should find the DoorMesh component."), DoorMesh);
	if (DoorMesh)
	{
		TestEqual(TEXT("Unlocked opening password doors should clear blocking collision."), DoorMesh->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
	}

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
