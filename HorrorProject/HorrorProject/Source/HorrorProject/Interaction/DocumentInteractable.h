// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/BaseInteractable.h"
#include "Player/Components/InventoryComponent.h"
#include "DocumentInteractable.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UTexture2D;
class UMaterialInterface;

UENUM(BlueprintType)
enum class EDocumentType : uint8
{
	Text UMETA(DisplayName="文本文档"),
	Image UMETA(DisplayName="图片"),
	Mixed UMETA(DisplayName="混合内容")
};

/**
 * Document interactable that displays text or images to the player.
 * Integrates with evidence system for collectible documents.
 */
UCLASS(BlueprintType, Blueprintable)
class HORRORPROJECT_API ADocumentInteractable : public ABaseInteractable
{
	GENERATED_BODY()

public:
	ADocumentInteractable();

	virtual bool CanInteract_Implementation(AActor* InstigatorActor, const FHitResult& Hit) const override;

	UFUNCTION(BlueprintPure, Category="Document")
	EDocumentType GetDocumentType() const { return DocumentType; }

	UFUNCTION(BlueprintPure, Category="Document")
	FText GetDocumentTitle() const { return DocumentTitle; }

	UFUNCTION(BlueprintPure, Category="Document")
	FText GetDocumentContent() const { return DocumentContent; }

	UFUNCTION(BlueprintPure, Category="Document")
	UTexture2D* GetDocumentImage() const { return DocumentImage; }

	UFUNCTION(BlueprintCallable, Category="Document")
	void CloseDocument();

protected:
	virtual void OnInteract(AActor* InstigatorActor, const FHitResult& Hit) override;
	virtual void BeginPlay() override;

private:
	bool TryAddToEvidence(AActor* InstigatorActor);
	void ShowDocumentUI();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Document", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> DocumentMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Document", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> InteractionVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Document|Content", meta=(AllowPrivateAccess="true"))
	EDocumentType DocumentType = EDocumentType::Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Document|Content", meta=(AllowPrivateAccess="true"))
	FText DocumentTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Document|Content", meta=(AllowPrivateAccess="true", MultiLine=true))
	FText DocumentContent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Document|Content", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UTexture2D> DocumentImage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Document|Content", meta=(AllowPrivateAccess="true"))
	TArray<UTexture2D*> AdditionalImages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Document|Evidence", meta=(AllowPrivateAccess="true"))
	bool bIsEvidence = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Document|Evidence", meta=(AllowPrivateAccess="true"))
	FHorrorEvidenceMetadata EvidenceMetadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Document|Audio", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USoundBase> OpenSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Document|Audio", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USoundBase> CloseSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Document|Behavior", meta=(AllowPrivateAccess="true"))
	bool bAutoCollectAsEvidence = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Document|Behavior", meta=(AllowPrivateAccess="true"))
	bool bDisablePlayerMovement = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Document|Behavior", meta=(AllowPrivateAccess="true"))
	bool bPauseGame = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Document|State", meta=(AllowPrivateAccess="true"))
	bool bIsCurrentlyOpen = false;

	UPROPERTY()
	TObjectPtr<APlayerController> ReadingPlayerController = nullptr;
};
