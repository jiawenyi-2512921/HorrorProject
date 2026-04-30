// Copyright HorrorProject. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Player/Components/SanityComponent.h"
#include "GameFramework/Actor.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSanityComponentDrainTest,
	"HorrorProject.Player.Sanity.DrainSanityReducesValue",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSanityComponentDrainTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AActor* Owner = World->SpawnActor<AActor>();
	USanityComponent* Sanity = NewObject<USanityComponent>(Owner);
	Sanity->RegisterComponent();

	const float InitialSanity = Sanity->GetSanity();
	Sanity->DrainSanity(30.0f);

	TestTrue(TEXT("Sanity should decrease after drain"), Sanity->GetSanity() < InitialSanity);
	TestEqual(TEXT("Sanity should be 70 after draining 30"), Sanity->GetSanity(), 70.0f);

	Sanity->DestroyComponent();
	Owner->Destroy();
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSanityComponentRecoveryTest,
	"HorrorProject.Player.Sanity.RecoverSanityIncreasesValue",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSanityComponentRecoveryTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AActor* Owner = World->SpawnActor<AActor>();
	USanityComponent* Sanity = NewObject<USanityComponent>(Owner);
	Sanity->RegisterComponent();

	Sanity->DrainSanity(50.0f);
	const float AfterDrain = Sanity->GetSanity();
	Sanity->RecoverSanity(20.0f);

	TestTrue(TEXT("Sanity should increase after recovery"), Sanity->GetSanity() > AfterDrain);
	TestEqual(TEXT("Sanity should be 70 after drain 50 and recover 20"), Sanity->GetSanity(), 70.0f);

	Sanity->DestroyComponent();
	Owner->Destroy();
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSanityComponentClampTest,
	"HorrorProject.Player.Sanity.SanityClampedToValidRange",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSanityComponentClampTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AActor* Owner = World->SpawnActor<AActor>();
	USanityComponent* Sanity = NewObject<USanityComponent>(Owner);
	Sanity->RegisterComponent();

	Sanity->DrainSanity(999.0f);
	TestEqual(TEXT("Sanity should clamp to 0"), Sanity->GetSanity(), 0.0f);

	Sanity->RecoverSanity(999.0f);
	TestEqual(TEXT("Sanity should clamp to 100"), Sanity->GetSanity(), 100.0f);

	Sanity->DestroyComponent();
	Owner->Destroy();
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSanityComponentLevelTest,
	"HorrorProject.Player.Sanity.LevelTransitions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSanityComponentLevelTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AActor* Owner = World->SpawnActor<AActor>();
	USanityComponent* Sanity = NewObject<USanityComponent>(Owner);
	Sanity->RegisterComponent();

	TestEqual(TEXT("Initial level should be Stable"), Sanity->GetSanityLevel(), ESanityLevel::Stable);

	Sanity->DrainSanity(30.0f); // 70% remaining
	TestEqual(TEXT("Should be Uneasy at 70%"), Sanity->GetSanityLevel(), ESanityLevel::Uneasy);

	Sanity->DrainSanity(20.0f); // 50% remaining
	TestEqual(TEXT("Should be Disturbed at 50%"), Sanity->GetSanityLevel(), ESanityLevel::Disturbed);

	Sanity->DrainSanity(20.0f); // 30% remaining
	TestEqual(TEXT("Should be Breaking at 30%"), Sanity->GetSanityLevel(), ESanityLevel::Breaking);

	Sanity->DrainSanity(20.0f); // 10% remaining
	TestEqual(TEXT("Should be Shattered at 10%"), Sanity->GetSanityLevel(), ESanityLevel::Shattered);

	Sanity->DestroyComponent();
	Owner->Destroy();
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSanityComponentPercentTest,
	"HorrorProject.Player.Sanity.GetSanityPercentReturnsCorrectValue",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSanityComponentPercentTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AActor* Owner = World->SpawnActor<AActor>();
	USanityComponent* Sanity = NewObject<USanityComponent>(Owner);
	Sanity->RegisterComponent();

	TestTrue(TEXT("Initial percent should be 1.0"), FMath::IsNearlyEqual(Sanity->GetSanityPercent(), 1.0f));

	Sanity->DrainSanity(25.0f);
	TestTrue(TEXT("Percent should be 0.75"), FMath::IsNearlyEqual(Sanity->GetSanityPercent(), 0.75f));

	Sanity->DestroyComponent();
	Owner->Destroy();
	World->DestroyWorld(false);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
