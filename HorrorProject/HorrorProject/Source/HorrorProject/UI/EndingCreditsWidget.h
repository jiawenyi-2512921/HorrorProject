#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EndingCreditsWidget.generated.h"

UCLASS()
class HORRORPROJECT_API UEndingCreditsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="Horror|Credits")
	void StartCreditsRoll();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Credits")
	FName MainMenuMapName = TEXT("DemoMap_VerticalSlice_Day1");

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Credits")
	float ScrollSpeed = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Credits")
	float FinalTextDisplayDuration = 5.0f;

private:
	enum class ECreditsState : uint8
	{
		Idle,
		Scrolling,
		FinalText,
		Complete
	};

	ECreditsState CurrentState = ECreditsState::Idle;
	float ScrollOffset = 0.0f;
	float FinalTextTimer = 0.0f;
	float TotalCreditsHeight = 0.0f;

	TArray<FString> GetCreditsLines() const;
	void DrawScrollingCredits(const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;
	void DrawFinalText(const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;

	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
};
