// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/EndingCreditsWidget.h"
#include "Rendering/DrawElements.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Kismet/GameplayStatics.h"

void UEndingCreditsWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CurrentState = ECreditsState::Idle;
	ScrollOffset = 0.0f;
	FinalTextTimer = 0.0f;
}

void UEndingCreditsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	switch (CurrentState)
	{
	case ECreditsState::Scrolling:
		ScrollOffset += ScrollSpeed * InDeltaTime;

		// 当滚动完成时切换到最终文字
		if (ScrollOffset >= TotalCreditsHeight + MyGeometry.GetLocalSize().Y)
		{
			CurrentState = ECreditsState::FinalText;
			FinalTextTimer = 0.0f;
		}
		break;

	case ECreditsState::FinalText:
		FinalTextTimer += InDeltaTime;
		if (FinalTextTimer >= FinalTextDisplayDuration)
		{
			CurrentState = ECreditsState::Complete;
			UGameplayStatics::OpenLevel(this, MainMenuMapName);
		}
		break;

	default:
		break;
	}
}

void UEndingCreditsWidget::StartCreditsRoll()
{
	CurrentState = ECreditsState::Scrolling;
	ScrollOffset = 0.0f;
	FinalTextTimer = 0.0f;

	// 计算总高度
	TArray<FString> Lines = GetCreditsLines();
	TotalCreditsHeight = Lines.Num() * 40.0f; // 每行40像素
}

TArray<FString> UEndingCreditsWidget::GetCreditsLines() const
{
	return {
		TEXT(""),
		TEXT(""),
		TEXT("DEEP WATER STATION: SIGNAL LOST"),
		TEXT("深水站：信号丢失"),
		TEXT(""),
		TEXT(""),
		TEXT("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"),
		TEXT(""),
		TEXT("制作团队"),
		TEXT(""),
		TEXT("南开大学"),
		TEXT("2512921 贾文一"),
		TEXT(""),
		TEXT("开发辅助"),
		TEXT("GPT / Claude / 即梦 AI"),
		TEXT(""),
		TEXT("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"),
		TEXT(""),
		TEXT("技术栈"),
		TEXT(""),
		TEXT("游戏引擎"),
		TEXT("Unreal Engine 5.6"),
		TEXT(""),
		TEXT("开发语言"),
		TEXT("C++ / Blueprint"),
		TEXT(""),
		TEXT("渲染技术"),
		TEXT("Lumen 全局光照"),
		TEXT("虚拟阴影贴图"),
		TEXT("光线追踪"),
		TEXT(""),
		TEXT("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"),
		TEXT(""),
		TEXT("使用资产"),
		TEXT(""),
		TEXT("环境资产"),
		TEXT("Deep Water Station"),
		TEXT("Forest of Spikes"),
		TEXT("Scrapopolis"),
		TEXT("Fantastic Dungeon Pack"),
		TEXT(""),
		TEXT("视觉特效"),
		TEXT("Bodycam VHS Effect"),
		TEXT("Sounds of Horror"),
		TEXT(""),
		TEXT("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"),
		TEXT(""),
		TEXT("核心系统"),
		TEXT(""),
		TEXT("战役系统 - 9章关卡链"),
		TEXT("AI系统 - 威胁追逐与Boss战"),
		TEXT("玩家系统 - 手电筒/摄像机/理智值"),
		TEXT("交互系统 - 6种交互物体"),
		TEXT("恐怖导演 - 动态紧张度管理"),
		TEXT("异常事件 - 现实扭曲"),
		TEXT("叙事系统 - 剧情节拍"),
		TEXT("音频系统 - 恐怖音效区域"),
		TEXT("存档系统 - 检查点自动保存"),
		TEXT("设置系统 - 图形/音频/控制"),
		TEXT("UI系统 - 主菜单/HUD/加载画面"),
		TEXT("证据系统 - 档案收集"),
		TEXT("成就系统 - 统计追踪"),
		TEXT("无障碍功能 - 色盲/字幕"),
		TEXT("本地化系统 - 多语言支持"),
		TEXT("性能优化 - 对象池/预算管理"),
		TEXT("测试系统 - 100+自动化测试"),
		TEXT(""),
		TEXT("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"),
		TEXT(""),
		TEXT("项目统计"),
		TEXT(""),
		TEXT("代码规模：50,000+ 行 C++"),
		TEXT("系统模块：17 个功能模块"),
		TEXT("子系统：20+ 个 Subsystem"),
		TEXT("组件：30+ 个 Component"),
		TEXT("Actor类：50+ 个"),
		TEXT("测试文件：100+ 个"),
		TEXT(""),
		TEXT("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"),
		TEXT(""),
		TEXT("战役章节"),
		TEXT(""),
		TEXT("第一章 - 信号校准"),
		TEXT("第二章 - 尖刺森林"),
		TEXT("第三章 - 废铁城"),
		TEXT("第四章 - 地牢入口"),
		TEXT("第五章 - 地牢深处"),
		TEXT("第六章 - 地牢大厅"),
		TEXT("第七章 - 地牢神殿"),
		TEXT("第八章 - 石魔像Boss战"),
		TEXT("第九章 - 深水站终章"),
		TEXT(""),
		TEXT("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"),
		TEXT(""),
		TEXT("技术亮点"),
		TEXT(""),
		TEXT("AAA级架构设计"),
		TEXT("现代UE5技术栈"),
		TEXT("完整的游戏系统"),
		TEXT("全面的测试覆盖"),
		TEXT("无障碍功能支持"),
		TEXT("性能优化方案"),
		TEXT("安全性保障"),
		TEXT("可维护性设计"),
		TEXT(""),
		TEXT("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"),
		TEXT(""),
		TEXT(""),
		TEXT("感谢您的游玩"),
		TEXT(""),
		TEXT(""),
		TEXT(""),
		TEXT(""),
		TEXT(""),
	};
}

int32 UEndingCreditsWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	int32 MaxLayerId = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	// 绘制黑色背景
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		MaxLayerId++,
		AllottedGeometry.ToPaintGeometry(),
		FCoreStyle::Get().GetBrush("WhiteBrush"),
		ESlateDrawEffect::None,
		FLinearColor::Black
	);

	switch (CurrentState)
	{
	case ECreditsState::Scrolling:
		DrawScrollingCredits(AllottedGeometry, OutDrawElements, MaxLayerId++);
		break;

	case ECreditsState::FinalText:
		DrawFinalText(AllottedGeometry, OutDrawElements, MaxLayerId++);
		break;

	default:
		break;
	}

	return MaxLayerId;
}

void UEndingCreditsWidget::DrawScrollingCredits(const FGeometry& AllottedGeometry,
	FSlateWindowElementList& OutDrawElements, int32 LayerId) const
{
	TArray<FString> Lines = GetCreditsLines();
	const FVector2D LocalSize = AllottedGeometry.GetLocalSize();
	const float CenterX = LocalSize.X * 0.5f;
	const float StartY = LocalSize.Y; // 从屏幕底部开始
	const float LineHeight = 40.0f;

	FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle("Regular", 18);
	const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

	for (int32 i = 0; i < Lines.Num(); ++i)
	{
		const FString& Line = Lines[i];
		if (Line.IsEmpty()) continue;

		float CurrentY = StartY - ScrollOffset + (i * LineHeight);

		// 只绘制在屏幕范围内的文字
		if (CurrentY < -LineHeight || CurrentY > LocalSize.Y + LineHeight)
		{
			continue;
		}

		// 标题行使用更大字体
		bool bIsTitle = Line.Contains(TEXT("DEEP WATER")) || Line.Contains(TEXT("深水站"));
		bool bIsSection = Line.Contains(TEXT("━"));
		bool bIsSectionTitle = !bIsSection && !bIsTitle && i > 1
			&& (Lines[i - 1].IsEmpty() && Lines[i - 2].Contains(TEXT("━")));

		if (bIsTitle)
		{
			FontInfo = FCoreStyle::GetDefaultFontStyle("Bold", 32);
		}
		else if (bIsSectionTitle)
		{
			FontInfo = FCoreStyle::GetDefaultFontStyle("Bold", 24);
		}
		else if (bIsSection)
		{
			FontInfo = FCoreStyle::GetDefaultFontStyle("Regular", 14);
		}
		else
		{
			FontInfo = FCoreStyle::GetDefaultFontStyle("Regular", 18);
		}

		FVector2D TextSize = FontMeasure->Measure(Line, FontInfo);
		FVector2D TextPosition(CenterX - TextSize.X * 0.5f, CurrentY);

		FLinearColor TextColor = FLinearColor::White;
		if (bIsSection)
		{
			TextColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
		}
		else if (bIsTitle)
		{
			TextColor = FLinearColor(0.8f, 0.6f, 0.2f, 1.0f);
		}

		// 淡入淡出效果
		float Alpha = 1.0f;
		if (CurrentY < 100.0f)
		{
			Alpha = FMath::Clamp(CurrentY / 100.0f, 0.0f, 1.0f);
		}
		else if (CurrentY > LocalSize.Y - 100.0f)
		{
			Alpha = FMath::Clamp((LocalSize.Y - CurrentY) / 100.0f, 0.0f, 1.0f);
		}
		TextColor.A = Alpha;

		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(TextSize, FSlateLayoutTransform(TextPosition)),
			Line,
			FontInfo,
			ESlateDrawEffect::None,
			TextColor
		);
	}
}

void UEndingCreditsWidget::DrawFinalText(const FGeometry& AllottedGeometry,
	FSlateWindowElementList& OutDrawElements, int32 LayerId) const
{
	const FVector2D LocalSize = AllottedGeometry.GetLocalSize();
	const float CenterX = LocalSize.X * 0.5f;
	const float CenterY = LocalSize.Y * 0.5f;

	FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 48);
	FSlateFontInfo SubFont = FCoreStyle::GetDefaultFontStyle("Regular", 24);

	const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

	// 淡入效果
	float Alpha = FMath::Clamp(FinalTextTimer / 1.0f, 0.0f, 1.0f);

	// 主标题
	FString MainText = TEXT("感谢您的游玩");
	FVector2D MainTextSize = FontMeasure->Measure(MainText, TitleFont);
	FVector2D MainTextPos(CenterX - MainTextSize.X * 0.5f, CenterY - 80.0f);

	FLinearColor MainColor = FLinearColor(0.8f, 0.6f, 0.2f, Alpha);
	FSlateDrawElement::MakeText(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(MainTextSize, FSlateLayoutTransform(MainTextPos)),
		MainText,
		TitleFont,
		ESlateDrawEffect::None,
		MainColor
	);

	// 副标题
	FString SubText = TEXT("THANK YOU FOR PLAYING");
	FVector2D SubTextSize = FontMeasure->Measure(SubText, SubFont);
	FVector2D SubTextPos(CenterX - SubTextSize.X * 0.5f, CenterY + 20.0f);

	FLinearColor SubColor = FLinearColor::White;
	SubColor.A = Alpha;
	FSlateDrawElement::MakeText(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(SubTextSize, FSlateLayoutTransform(SubTextPos)),
		SubText,
		SubFont,
		ESlateDrawEffect::None,
		SubColor
	);

	// 底部文字
	FString BottomText = TEXT("Deep Water Station: Signal Lost");
	FVector2D BottomTextSize = FontMeasure->Measure(BottomText, SubFont);
	FVector2D BottomTextPos(CenterX - BottomTextSize.X * 0.5f, CenterY + 100.0f);

	FLinearColor BottomColor = FLinearColor(0.6f, 0.6f, 0.6f, Alpha);
	FSlateDrawElement::MakeText(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(BottomTextSize, FSlateLayoutTransform(BottomTextPos)),
		BottomText,
		SubFont,
		ESlateDrawEffect::None,
		BottomColor
	);
}
