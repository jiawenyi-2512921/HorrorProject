#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Evidence/EvidenceCollectionComponent.h"
#include "Evidence/EvidenceTypes.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "Player/Components/InventoryComponent.h"
#include "GameFramework/Actor.h"

#if WITH_DEV_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(FEvidenceCollectionSpec, "HorrorProject.Evidence.Collection",
	EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	TObjectPtr<UWorld> TestWorld;
	TObjectPtr<AActor> TestActor;
	TObjectPtr<UEvidenceCollectionComponent> EvidenceCollection;
	TObjectPtr<UQuantumCameraComponent> CameraComponent;
	TObjectPtr<UInventoryComponent> InventoryComponent;
END_DEFINE_SPEC(FEvidenceCollectionSpec)

void FEvidenceCollectionSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
		TestActor = TestWorld->SpawnActor<AActor>();

		EvidenceCollection = NewObject<UEvidenceCollectionComponent>(TestActor);
		CameraComponent = NewObject<UQuantumCameraComponent>(TestActor);
		InventoryComponent = NewObject<UInventoryComponent>(TestActor);

		TestActor->AddInstanceComponent(EvidenceCollection);
		TestActor->AddInstanceComponent(CameraComponent);
		TestActor->AddInstanceComponent(InventoryComponent);

		EvidenceCollection->RegisterComponent();
		CameraComponent->RegisterComponent();
		InventoryComponent->RegisterComponent();
	});

	AfterEach([this]()
	{
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	Describe("Physical Evidence Collection", [this]()
	{
		It("should collect physical evidence with metadata", [this]()
		{
			FExtendedEvidenceMetadata Metadata;
			Metadata.EvidenceId = FName("TestEvidence");
			Metadata.DisplayName = FText::FromString("Test Evidence");
			Metadata.Type = EEvidenceType::Physical;
			Metadata.Category = EEvidenceCategory::Clue;

			bool bSuccess = EvidenceCollection->CollectPhysicalEvidence(FName("TestEvidence"), Metadata);

			TestTrue("Evidence collection succeeded", bSuccess);
			TestTrue("Evidence added to inventory", InventoryComponent->HasCollectedEvidenceId(FName("TestEvidence")));
		});

		It("should register evidence metadata", [this]()
		{
			FExtendedEvidenceMetadata Metadata;
			Metadata.EvidenceId = FName("TestEvidence");
			Metadata.DisplayName = FText::FromString("Test Evidence");
			Metadata.ImportanceLevel = 5;

			EvidenceCollection->RegisterEvidenceMetadata(Metadata);

			FExtendedEvidenceMetadata Retrieved;
			bool bFound = EvidenceCollection->GetEvidenceMetadata(FName("TestEvidence"), Retrieved);

			TestTrue("Metadata found", bFound);
			TestEqual("Evidence ID matches", Retrieved.EvidenceId, FName("TestEvidence"));
			TestEqual("Importance level matches", Retrieved.ImportanceLevel, 5);
		});

		It("should reject evidence with no ID", [this]()
		{
			FExtendedEvidenceMetadata Metadata;
			Metadata.EvidenceId = NAME_None;

			bool bSuccess = EvidenceCollection->CollectPhysicalEvidence(NAME_None, Metadata);

			TestFalse("Collection rejected", bSuccess);
		});
	});

	Describe("Photo Capture", [this]()
	{
		It("should capture photo when camera is in photo mode", [this]()
		{
			CameraComponent->SetCameraAcquired(true);
			CameraComponent->SetCameraEnabled(true);
			CameraComponent->SetCameraMode(EQuantumCameraMode::Photo);

			FExtendedEvidenceMetadata Metadata;
			Metadata.EvidenceId = FName("Photo001");
			Metadata.DisplayName = FText::FromString("Test Photo");

			bool bSuccess = EvidenceCollection->CapturePhotoEvidence(FName("Photo001"), Metadata);

			TestTrue("Photo captured", bSuccess);
			TestTrue("Photo added to inventory", InventoryComponent->HasCollectedEvidenceId(FName("Photo001")));
		});

		It("should respect max photo limit", [this]()
		{
			CameraComponent->SetCameraAcquired(true);
			CameraComponent->SetCameraEnabled(true);
			CameraComponent->SetCameraMode(EQuantumCameraMode::Photo);

			EvidenceCollection->MaxPhotos = 2;

			FExtendedEvidenceMetadata Metadata1;
			Metadata1.EvidenceId = FName("Photo001");
			FExtendedEvidenceMetadata Metadata2;
			Metadata2.EvidenceId = FName("Photo002");
			FExtendedEvidenceMetadata Metadata3;
			Metadata3.EvidenceId = FName("Photo003");

			TestTrue("First photo captured", EvidenceCollection->CapturePhotoEvidence(FName("Photo001"), Metadata1));
			TestTrue("Second photo captured", EvidenceCollection->CapturePhotoEvidence(FName("Photo002"), Metadata2));
			TestFalse("Third photo rejected", EvidenceCollection->CapturePhotoEvidence(FName("Photo003"), Metadata3));
		});
	});

	Describe("Video Recording", [this]()
	{
		It("should start and stop video recording", [this]()
		{
			CameraComponent->SetCameraAcquired(true);
			CameraComponent->SetCameraEnabled(true);
			CameraComponent->SetCameraMode(EQuantumCameraMode::Recording);

			bool bStarted = EvidenceCollection->StartVideoCapture(FName("Video001"));
			TestTrue("Video recording started", bStarted);
			TestTrue("Is capturing video", EvidenceCollection->IsCapturingVideo());

			FName OutId;
			float OutDuration;
			bool bStopped = EvidenceCollection->StopVideoCapture(OutId, OutDuration);

			TestTrue("Video recording stopped", bStopped);
			TestFalse("No longer capturing", EvidenceCollection->IsCapturingVideo());
			TestEqual("Video ID matches", OutId, FName("Video001"));
		});

		It("should not start recording if already recording", [this]()
		{
			CameraComponent->SetCameraAcquired(true);
			CameraComponent->SetCameraEnabled(true);
			CameraComponent->SetCameraMode(EQuantumCameraMode::Recording);

			EvidenceCollection->StartVideoCapture(FName("Video001"));
			bool bSecondStart = EvidenceCollection->StartVideoCapture(FName("Video002"));

			TestFalse("Second recording rejected", bSecondStart);
		});
	});

	Describe("Evidence Filtering", [this]()
	{
		It("should filter evidence by type", [this]()
		{
			FExtendedEvidenceMetadata Photo;
			Photo.EvidenceId = FName("Photo001");
			Photo.Type = EEvidenceType::Photo;
			EvidenceCollection->RegisterEvidenceMetadata(Photo);

			FExtendedEvidenceMetadata Physical;
			Physical.EvidenceId = FName("Physical001");
			Physical.Type = EEvidenceType::Physical;
			EvidenceCollection->RegisterEvidenceMetadata(Physical);

			TArray<FExtendedEvidenceMetadata> Photos = EvidenceCollection->GetEvidenceByType(EEvidenceType::Photo);

			TestEqual("One photo found", Photos.Num(), 1);
			TestEqual("Correct photo returned", Photos[0].EvidenceId, FName("Photo001"));
		});

		It("should filter evidence by category", [this]()
		{
			FExtendedEvidenceMetadata Clue;
			Clue.EvidenceId = FName("Clue001");
			Clue.Category = EEvidenceCategory::Clue;
			EvidenceCollection->RegisterEvidenceMetadata(Clue);

			FExtendedEvidenceMetadata Witness;
			Witness.EvidenceId = FName("Witness001");
			Witness.Category = EEvidenceCategory::Witness;
			EvidenceCollection->RegisterEvidenceMetadata(Witness);

			TArray<FExtendedEvidenceMetadata> Clues = EvidenceCollection->GetEvidenceByCategory(EEvidenceCategory::Clue);

			TestEqual("One clue found", Clues.Num(), 1);
			TestEqual("Correct clue returned", Clues[0].EvidenceId, FName("Clue001"));
		});
	});

	Describe("Evidence Events", [this]()
	{
		It("should broadcast evidence captured event", [this]()
		{
			bool bEventFired = false;
			FName CapturedId = NAME_None;

			EvidenceCollection->OnEvidenceCaptured.AddLambda([&](FName EvidenceId, const FEvidenceCaptureData& Data)
			{
				bEventFired = true;
				CapturedId = EvidenceId;
			});

			FExtendedEvidenceMetadata Metadata;
			Metadata.EvidenceId = FName("TestEvidence");
			EvidenceCollection->CollectPhysicalEvidence(FName("TestEvidence"), Metadata);

			TestTrue("Event fired", bEventFired);
			TestEqual("Correct ID in event", CapturedId, FName("TestEvidence"));
		});
	});
}

#endif

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
