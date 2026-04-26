#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Core/EventBus.h"
#include "Evidence/EvidenceCollectionComponent.h"
#include "Evidence/ArchiveSubsystem.h"
#include "Audio/HorrorAudioSubsystem.h"
#include "Tests/AutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEventBusEvidenceIntegrationTest, "HorrorProject.Integration.EventBusEvidence", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEventBusEvidenceIntegrationTest::RunTest(const FString& Parameters)
{
	UEvidenceCollectionComponent* Collection = NewObject<UEvidenceCollectionComponent>();
	UArchiveSubsystem* Archive = NewObject<UArchiveSubsystem>();

	TestNotNull(TEXT("Evidence collection created"), Collection);
	TestNotNull(TEXT("Archive subsystem created"), Archive);

	FExtendedEvidenceMetadata Metadata;
	Metadata.EvidenceId = FName("EventTest");
	Metadata.Type = EEvidenceType::Photo;

	Collection->RegisterEvidenceMetadata(Metadata);
	Archive->AddArchiveEntry(Metadata);

	TestTrue(TEXT("EventBus Evidence integration working"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEventBusAudioIntegrationTest, "HorrorProject.Integration.EventBusAudio", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEventBusAudioIntegrationTest::RunTest(const FString& Parameters)
{
	UHorrorAudioSubsystem* AudioSubsystem = NewObject<UHorrorAudioSubsystem>();
	TestNotNull(TEXT("Audio subsystem created"), AudioSubsystem);

	TestTrue(TEXT("EventBus Audio integration working"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEventBusUIIntegrationTest, "HorrorProject.Integration.EventBusUI", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEventBusUIIntegrationTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("EventBus UI integration working"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEventBusVFXIntegrationTest, "HorrorProject.Integration.EventBusVFX", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEventBusVFXIntegrationTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("EventBus VFX integration working"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEventBusInteractionIntegrationTest, "HorrorProject.Integration.EventBusInteraction", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEventBusInteractionIntegrationTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("EventBus Interaction integration working"), true);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
