// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorCampaign.h"

#include "Game/HorrorMapChain.h"
#include "Misc/PackageName.h"

#include <initializer_list>

namespace
{
	const TCHAR* DeviceMeshPath = TEXT("/Game/DeepWaterStation/Meshes/SM_Device01.SM_Device01");
	const TCHAR* TerminalMeshPath = TEXT("/Game/DeepWaterStation/Meshes/SM_Terminal01.SM_Terminal01");
	const TCHAR* ScreenMeshPath = TEXT("/Game/DeepWaterStation/Meshes/SM_ScreensA01.SM_ScreensA01");
	const TCHAR* GateMeshPath = TEXT("/Game/DeepWaterStation/Meshes/SM_MetalGate01.SM_MetalGate01");
	const TCHAR* RelicMeshPath = TEXT("/Game/DeepWaterStation/Meshes/SM_Props07.SM_Props07");
	const TCHAR* DungeonKeyMeshPath = TEXT("/Game/Fantastic_Dungeon_Pack/meshes/props/tools/SM_PROP_key_dungeon_01.SM_PROP_key_dungeon_01");

	FHorrorCampaignObjectiveDefinition MakeObjective(
		const TCHAR* ObjectiveId,
		EHorrorCampaignObjectiveType ObjectiveType,
		const TCHAR* PromptText,
		const TCHAR* CompletionText,
		const FVector& RelativeLocation,
		const TCHAR* VisualMeshPath,
		const FVector& EscapeDestinationOffset = FVector::ZeroVector,
		float EscapeCompletionRadius = 260.0f)
	{
		FHorrorCampaignObjectiveDefinition Objective;
		Objective.ObjectiveId = FName(ObjectiveId);
		Objective.ObjectiveType = ObjectiveType;
		switch (ObjectiveType)
		{
			case EHorrorCampaignObjectiveType::RestorePower:
				Objective.InteractionMode = EHorrorCampaignInteractionMode::CircuitWiring;
				break;
			case EHorrorCampaignObjectiveType::DisableSeal:
				Objective.InteractionMode = EHorrorCampaignInteractionMode::GearCalibration;
				break;
			case EHorrorCampaignObjectiveType::SurviveAmbush:
				Objective.InteractionMode = EHorrorCampaignInteractionMode::TimedPursuit;
				break;
			case EHorrorCampaignObjectiveType::PlantBeacon:
			case EHorrorCampaignObjectiveType::BossWeakPoint:
				Objective.InteractionMode = EHorrorCampaignInteractionMode::MultiStep;
				break;
			case EHorrorCampaignObjectiveType::AcquireSignal:
			case EHorrorCampaignObjectiveType::ScanAnomaly:
			case EHorrorCampaignObjectiveType::RecoverRelic:
			case EHorrorCampaignObjectiveType::FinalTerminal:
			default:
				Objective.InteractionMode = EHorrorCampaignInteractionMode::Instant;
				break;
		}
		Objective.PromptText = FText::FromString(PromptText);
		Objective.CompletionText = FText::FromString(CompletionText);
		Objective.RelativeLocation = RelativeLocation;
		Objective.EscapeDestinationOffset = EscapeDestinationOffset;
		Objective.EscapeCompletionRadius = EscapeCompletionRadius;
		Objective.VisualMeshPath = FSoftObjectPath(VisualMeshPath);
		return Objective;
	}

	FHorrorCampaignChapterDefinition MakeChapter(
		const TCHAR* ChapterId,
		const TCHAR* MapPackageName,
		const TCHAR* Title,
		const TCHAR* StoryBrief,
		const TCHAR* CompletionBridgeText,
		bool bRequiresBoss,
		bool bIsFinalChapter,
		std::initializer_list<FHorrorCampaignObjectiveDefinition> Objectives)
	{
		FHorrorCampaignChapterDefinition Chapter;
		Chapter.ChapterId = FName(ChapterId);
		Chapter.MapPackageName = MapPackageName;
		Chapter.Title = FText::FromString(Title);
		Chapter.StoryBrief = FText::FromString(StoryBrief);
		Chapter.CompletionBridgeText = FText::FromString(CompletionBridgeText);
		Chapter.bRequiresBoss = bRequiresBoss;
		Chapter.bIsFinalChapter = bIsFinalChapter;
		Chapter.Objectives.Reserve(static_cast<int32>(Objectives.size()));
		for (const FHorrorCampaignObjectiveDefinition& Objective : Objectives)
		{
			Chapter.Objectives.Add(Objective);
		}
		return Chapter;
	}

	const TArray<FHorrorCampaignChapterDefinition>& BuildCampaignChapters()
	{
		static const TArray<FHorrorCampaignChapterDefinition> Chapters = {
			MakeChapter(
				TEXT("Chapter.DeepWaterStationFinale"),
				TEXT("/Game/DeepWaterStation/Maps/DemoMap_VerticalSlice_Day1"),
				TEXT("第一章：深水站黑盒"),
				TEXT("深水站的黑盒在午夜自动开机，录像带里传出巨人的心跳。玩家作为调查员进入干船坞，确认这不是事故，而是一场从未来倒灌回来的求救。"),
				TEXT("黑盒吐出第一段坐标，信号把调查路线推向尖刺林地。"),
				false,
				false,
				{
					MakeObjective(TEXT("DeepWater.BootDryDock"), EHorrorCampaignObjectiveType::RestorePower, TEXT("接通干船坞电路，唤醒黑盒供电"), TEXT("干船坞恢复供电，黑盒开始回放。"), FVector(520.0f, -200.0f, 95.0f), TerminalMeshPath),
					MakeObjective(TEXT("DeepWater.DecodeHeartbeat"), EHorrorCampaignObjectiveType::ScanAnomaly, TEXT("扫描黑盒里的巨人心跳频率"), TEXT("巨人心跳被记录，录像带坐标浮现。"), FVector(920.0f, 60.0f, 95.0f), ScreenMeshPath),
					MakeObjective(TEXT("DeepWater.SurviveDockPursuit"), EHorrorCampaignObjectiveType::SurviveAmbush, TEXT("拉下警报闸，逃向干船坞集合点"), TEXT("警报声压住追逐脚步，第一段坐标稳定。"), FVector(1320.0f, -120.0f, 95.0f), DeviceMeshPath, FVector(2450.0f, 720.0f, 0.0f), 280.0f)
				}),
			MakeChapter(
				TEXT("Chapter.ForestOfSpikes"),
				TEXT("/Game/ForestOfSpikes/Levels/Level_ForestOfSpikes_Demo_Night"),
				TEXT("第二章：尖刺林地"),
				TEXT("黑盒坐标把玩家带到被铁刺寄生的夜林。树根里保存着深水站早期实验的录像带残片，也藏着唤醒巨人的第一段错误指令。"),
				TEXT("黑色种核吐出一串机械坐标，黑盒信号转向废铁村。"),
				false,
				false,
				{
					MakeObjective(TEXT("Forest.ReadRootGlyph"), EHorrorCampaignObjectiveType::ScanAnomaly, TEXT("读取荆棘根系上的符号"), TEXT("根系符号已记录。"), FVector(700.0f, -360.0f, 90.0f), RelicMeshPath),
					MakeObjective(TEXT("Forest.AlignSpikeBeacon"), EHorrorCampaignObjectiveType::PlantBeacon, TEXT("放置信标稳定林地路径"), TEXT("林地路径被稳定。"), FVector(1450.0f, 80.0f, 90.0f), DeviceMeshPath),
					MakeObjective(TEXT("Forest.HoldSpikeCircle"), EHorrorCampaignObjectiveType::SurviveAmbush, TEXT("激活信标后逃向林地汇合点"), TEXT("尖刺退潮，种核暴露。"), FVector(1850.0f, -280.0f, 90.0f), DeviceMeshPath, FVector(2550.0f, 1150.0f, 0.0f), 280.0f),
					MakeObjective(TEXT("Forest.ExtractBlackSeed"), EHorrorCampaignObjectiveType::RecoverRelic, TEXT("取回黑色种核"), TEXT("黑色种核正在发热。"), FVector(2350.0f, 430.0f, 90.0f), RelicMeshPath)
				}),
			MakeChapter(
				TEXT("Chapter.Scrapopolis"),
				TEXT("/Game/Scrapopolis/Levels/Level_Scrapopolis_Demo"),
				TEXT("第三章：废铁村"),
				TEXT("废铁村的蒸汽钟楼仍在运行。玩家需要恢复村口能源，把林地坐标翻译成通往地下城的路线，并确认深水站黑盒为什么记得这里。"),
				TEXT("桥梁控制信号打开，地下城入口正在接入。"),
				false,
				false,
				{
					MakeObjective(TEXT("Scrapopolis.RecoverPressureValve"), EHorrorCampaignObjectiveType::RecoverRelic, TEXT("回收蒸汽压力阀"), TEXT("压力阀已接入。"), FVector(650.0f, -420.0f, 95.0f), RelicMeshPath),
					MakeObjective(TEXT("Scrapopolis.RestoreGenerator"), EHorrorCampaignObjectiveType::RestorePower, TEXT("重启废铁村发电机并修复巨人膝关节驱动环"), TEXT("村落电力恢复，巨人关节驱动环重新点亮。"), FVector(1500.0f, 80.0f, 95.0f), TerminalMeshPath),
					MakeObjective(TEXT("Scrapopolis.DecodeBridge"), EHorrorCampaignObjectiveType::AcquireSignal, TEXT("解码桥梁控制信号"), TEXT("地下入口坐标已解码。"), FVector(2350.0f, 500.0f, 95.0f), ScreenMeshPath)
				}),
			MakeChapter(
				TEXT("Chapter.DungeonEntrance"),
				TEXT("/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_2_entrance"),
				TEXT("第四章：地牢入口"),
				TEXT("地下城入口被雾门封住。玩家用废铁村的坐标打开第一层封印，确认黑盒录像带里的石像巨人并不是幻觉。"),
				TEXT("雾门裂开，回廊深处传来石质回声。"),
				false,
				false,
				{
					MakeObjective(TEXT("Entrance.ScanFogDoor"), EHorrorCampaignObjectiveType::ScanAnomaly, TEXT("扫描雾门封印"), TEXT("雾门结构显形。"), FVector(750.0f, -300.0f, 95.0f), ScreenMeshPath),
					MakeObjective(TEXT("Entrance.RecoverGolemFinger"), EHorrorCampaignObjectiveType::RecoverRelic, TEXT("回收巨人断指石片"), TEXT("巨人断指石片已归档。"), FVector(1180.0f, -80.0f, 95.0f), RelicMeshPath),
					MakeObjective(TEXT("Entrance.DisableOuterSeal"), EHorrorCampaignObjectiveType::DisableSeal, TEXT("用巨人石片解除外层封印"), TEXT("外层封印已解除。"), FVector(1650.0f, 260.0f, 95.0f), GateMeshPath)
				}),
			MakeChapter(
				TEXT("Chapter.DungeonDepths"),
				TEXT("/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_1_dungeon"),
				TEXT("第五章：地下回廊"),
				TEXT("真正的地牢向下延伸。玩家恢复旧档案机，知道深水站不是起点，而是所有异常回流的终点，黑盒录像带只是最后的证词。"),
				TEXT("档案读取器吐出巨人苏醒记录，机关大厅的路径出现。"),
				false,
				false,
				{
					MakeObjective(TEXT("Depths.RecoverArchivePlate"), EHorrorCampaignObjectiveType::RecoverRelic, TEXT("回收石质档案板"), TEXT("档案板已回收。"), FVector(650.0f, -380.0f, 95.0f), RelicMeshPath),
					MakeObjective(TEXT("Depths.PowerArchiveReader"), EHorrorCampaignObjectiveType::RestorePower, TEXT("启动档案读取器并修复巨人记忆石板"), TEXT("档案读取器启动，巨人苏醒记录被重新拼合。"), FVector(1550.0f, 40.0f, 95.0f), TerminalMeshPath),
					MakeObjective(TEXT("Depths.MarkEscapeLine"), EHorrorCampaignObjectiveType::PlantBeacon, TEXT("标记回廊逃生线"), TEXT("逃生线已标记。"), FVector(2450.0f, 360.0f, 95.0f), DeviceMeshPath)
				}),
			MakeChapter(
				TEXT("Chapter.DungeonHall"),
				TEXT("/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_3_hall"),
				TEXT("第六章：机关大厅"),
				TEXT("大厅里的陷阱仍按古老节律运作。玩家要关闭机关，追踪巨人被唤醒前的最后一段回声，并为最终录像带回放校准黑盒频率。"),
				TEXT("安全路线锁定，神殿钥印在更深处发光。"),
				false,
				false,
				{
					MakeObjective(TEXT("Hall.DisableBladeRhythm"), EHorrorCampaignObjectiveType::DisableSeal, TEXT("停止机关节律"), TEXT("机关节律中断。"), FVector(720.0f, -420.0f, 95.0f), GateMeshPath),
					MakeObjective(TEXT("Hall.ScanGolemEcho"), EHorrorCampaignObjectiveType::ScanAnomaly, TEXT("扫描石像回声"), TEXT("巨人回声已记录。"), FVector(1600.0f, 20.0f, 95.0f), ScreenMeshPath),
					MakeObjective(TEXT("Hall.CalibrateGolemRestraint"), EHorrorCampaignObjectiveType::RestorePower, TEXT("校准巨人束缚机关"), TEXT("束缚机关短暂锁住了巨人回声。"), FVector(1900.0f, -360.0f, 95.0f), TerminalMeshPath),
					MakeObjective(TEXT("Hall.SurviveEchoPulse"), EHorrorCampaignObjectiveType::SurviveAmbush, TEXT("撑过巨人回声的震荡"), TEXT("回声震荡消散。"), FVector(2100.0f, -260.0f, 95.0f), RelicMeshPath),
					MakeObjective(TEXT("Hall.AnchorSafeRoute"), EHorrorCampaignObjectiveType::PlantBeacon, TEXT("锚定安全路线"), TEXT("安全路线已锚定。"), FVector(2550.0f, 440.0f, 95.0f), DeviceMeshPath)
				}),
			MakeChapter(
				TEXT("Chapter.DungeonTemple"),
				TEXT("/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_4_temple"),
				TEXT("第七章：沉没神殿"),
				TEXT("神殿保存着控制巨人的核心咒纹。玩家给祭坛充能，拿到进入巨人王座厅的最后钥印，深水站黑盒开始出现倒计时。"),
				TEXT("钥印拼合完成，巨人王座厅封印正在打开。"),
				false,
				false,
				{
					MakeObjective(TEXT("Temple.ChargeAltar"), EHorrorCampaignObjectiveType::RestorePower, TEXT("给神殿祭坛充能"), TEXT("祭坛亮起。"), FVector(760.0f, -360.0f, 95.0f), TerminalMeshPath),
					MakeObjective(TEXT("Temple.RecoverKeySigil"), EHorrorCampaignObjectiveType::RecoverRelic, TEXT("寻找钥匙碎片：取回发光钥印"), TEXT("钥匙碎片已回收。"), FVector(1260.0f, -80.0f, 105.0f), DungeonKeyMeshPath),
					MakeObjective(TEXT("Temple.RepairGolemHeartConduit"), EHorrorCampaignObjectiveType::RestorePower, TEXT("修复巨人心核导管"), TEXT("巨人心核导管开始低频震动。"), FVector(1850.0f, 220.0f, 95.0f), TerminalMeshPath),
					MakeObjective(TEXT("Temple.OpenBossSeal"), EHorrorCampaignObjectiveType::DisableSeal, TEXT("打开巨人王座厅封印"), TEXT("巨人王座厅封印开启。"), FVector(2600.0f, 380.0f, 95.0f), GateMeshPath)
				}),
			MakeChapter(
				TEXT("Chapter.StoneGolemBoss"),
				TEXT("/Game/Fantastic_Dungeon_Pack/maps/map_dungeon_level_5_bossroom"),
				TEXT("第八章：石像巨人"),
				TEXT("石像巨人守着黑盒信号的源头。玩家需要激活祭坛、击碎三个共鸣弱点，再封存核心，让最后一盘录像带可以完整播放。"),
				TEXT("巨人核心归档，黑盒把你拖向最后的录像带。"),
				true,
				false,
				{
					MakeObjective(TEXT("Boss.ActivateArenaAltar"), EHorrorCampaignObjectiveType::RestorePower, TEXT("激活竞技场祭坛"), TEXT("巨人被完全唤醒。"), FVector(500.0f, -380.0f, 100.0f), TerminalMeshPath),
					MakeObjective(TEXT("Boss.WeakPoint.LeftShoulder"), EHorrorCampaignObjectiveType::BossWeakPoint, TEXT("击碎左肩共鸣弱点"), TEXT("左肩弱点碎裂。"), FVector(1100.0f, -520.0f, 115.0f), RelicMeshPath),
					MakeObjective(TEXT("Boss.WeakPoint.Chest"), EHorrorCampaignObjectiveType::BossWeakPoint, TEXT("击碎胸口共鸣弱点"), TEXT("胸口核心暴露。"), FVector(1600.0f, 0.0f, 115.0f), RelicMeshPath),
					MakeObjective(TEXT("Boss.WeakPoint.RightArm"), EHorrorCampaignObjectiveType::BossWeakPoint, TEXT("击碎右臂共鸣弱点"), TEXT("右臂弱点碎裂。"), FVector(1100.0f, 520.0f, 115.0f), RelicMeshPath),
					MakeObjective(TEXT("Boss.SealCore"), EHorrorCampaignObjectiveType::DisableSeal, TEXT("封存巨人核心"), TEXT("巨人倒下，升降井打开。"), FVector(2100.0f, 0.0f, 115.0f), GateMeshPath)
				}),
			MakeChapter(
				TEXT("Chapter.SignalCalibration"),
				TEXT("/Game/Bodycam_VHS_Effect/Maps/LVL_Showcase_01"),
				TEXT("终章：黑盒录像带"),
				TEXT("巨人核心被封存后，黑盒把玩家拖进最后一盘录像带。深水站的走廊、尖刺林地的种核和王座厅的回声叠在一起，真相只剩一次完整回放。"),
				TEXT("录像带停止转动，深水站黑盒归档，结局信号已经锁定。"),
				false,
				true,
				{
					MakeObjective(TEXT("Signal.CalibrateLens"), EHorrorCampaignObjectiveType::AcquireSignal, TEXT("校准黑盒录像带画面"), TEXT("录像带噪点稳定，最后走廊显形。"), FVector(650.0f, -260.0f, 90.0f), DeviceMeshPath),
					MakeObjective(TEXT("Signal.ScanCorridor"), EHorrorCampaignObjectiveType::ScanAnomaly, TEXT("扫描走廊尽头的深水站倒影"), TEXT("深水站倒影与黑盒频率重合。"), FVector(1450.0f, 220.0f, 90.0f), ScreenMeshPath),
					MakeObjective(TEXT("Signal.SurviveStaticPursuit"), EHorrorCampaignObjectiveType::SurviveAmbush, TEXT("触发最后追逐，跑向录像带出口"), TEXT("巨人的残响被甩在录像带之外。"), FVector(2050.0f, -300.0f, 90.0f), DeviceMeshPath, FVector(2300.0f, 820.0f, 0.0f), 300.0f),
					MakeObjective(TEXT("Signal.FinalTerminal"), EHorrorCampaignObjectiveType::FinalTerminal, TEXT("播放完整黑盒录像带，确认深水站真相"), TEXT("真相归档，黑盒关闭。"), FVector(2620.0f, 580.0f, 90.0f), TerminalMeshPath)
				})
		};

		return Chapters;
	}
}

const TArray<FHorrorCampaignChapterDefinition>& FHorrorCampaign::GetChapters()
{
	return BuildCampaignChapters();
}

const FHorrorCampaignChapterDefinition* FHorrorCampaign::FindChapterById(FName ChapterId)
{
	for (const FHorrorCampaignChapterDefinition& Chapter : GetChapters())
	{
		if (Chapter.ChapterId == ChapterId)
		{
			return &Chapter;
		}
	}

	return nullptr;
}

const FHorrorCampaignChapterDefinition* FHorrorCampaign::FindChapterForMap(const FString& MapPackageName)
{
	const int32 ChapterIndex = FindChapterIndexForMap(MapPackageName);
	return ChapterIndex == INDEX_NONE ? nullptr : &GetChapters()[ChapterIndex];
}

const FHorrorCampaignChapterDefinition* FHorrorCampaign::FindBossChapter()
{
	for (const FHorrorCampaignChapterDefinition& Chapter : GetChapters())
	{
		if (Chapter.bRequiresBoss)
		{
			return &Chapter;
		}
	}

	return nullptr;
}

int32 FHorrorCampaign::FindChapterIndexForMap(const FString& MapPackageName)
{
	const FString NormalizedMapPackageName = FHorrorMapChain::NormalizeMapPackageName(MapPackageName);
	const FString ShortMapName = FPackageName::GetShortName(NormalizedMapPackageName);

	const TArray<FHorrorCampaignChapterDefinition>& Chapters = GetChapters();
	for (int32 ChapterIndex = 0; ChapterIndex < Chapters.Num(); ++ChapterIndex)
	{
		const FString ChapterPackageName = FHorrorMapChain::NormalizeMapPackageName(Chapters[ChapterIndex].MapPackageName);
		if (ChapterPackageName == NormalizedMapPackageName)
		{
			return ChapterIndex;
		}

		if (!ShortMapName.IsEmpty() && FPackageName::GetShortName(ChapterPackageName) == ShortMapName)
		{
			return ChapterIndex;
		}
	}

	return INDEX_NONE;
}

int32 FHorrorCampaign::CountRequiredObjectives(const FHorrorCampaignChapterDefinition& Chapter)
{
	int32 Count = 0;
	for (const FHorrorCampaignObjectiveDefinition& Objective : Chapter.Objectives)
	{
		if (Objective.bRequiredForChapterCompletion)
		{
			++Count;
		}
	}
	return Count;
}

int32 FHorrorCampaign::CountObjectivesOfType(const FHorrorCampaignChapterDefinition& Chapter, EHorrorCampaignObjectiveType ObjectiveType)
{
	int32 Count = 0;
	for (const FHorrorCampaignObjectiveDefinition& Objective : Chapter.Objectives)
	{
		if (Objective.ObjectiveType == ObjectiveType)
		{
			++Count;
		}
	}
	return Count;
}

const FHorrorCampaignObjectiveDefinition* FHorrorCampaign::FindObjectiveById(
	const FHorrorCampaignChapterDefinition& Chapter,
	FName ObjectiveId)
{
	for (const FHorrorCampaignObjectiveDefinition& Objective : Chapter.Objectives)
	{
		if (Objective.ObjectiveId == ObjectiveId)
		{
			return &Objective;
		}
	}

	return nullptr;
}

void FHorrorCampaignProgress::ResetForChapter(const FHorrorCampaignChapterDefinition& Chapter)
{
	ActiveChapter = &Chapter;
	CompletedObjectiveIds.Reset();
	bBossDefeated = !Chapter.bRequiresBoss;
}

bool FHorrorCampaignProgress::CanCompleteObjective(FName ObjectiveId) const
{
	const FHorrorCampaignObjectiveDefinition* NextObjective = GetNextObjective();
	return NextObjective && NextObjective->ObjectiveId == ObjectiveId;
}

bool FHorrorCampaignProgress::TryCompleteObjective(FName ObjectiveId)
{
	const FHorrorCampaignObjectiveDefinition* NextObjective = GetNextObjective();
	if (!NextObjective || NextObjective->ObjectiveId != ObjectiveId)
	{
		return false;
	}

	CompletedObjectiveIds.Add(ObjectiveId);

	if (ActiveChapter && NextObjective->ObjectiveType == EHorrorCampaignObjectiveType::BossWeakPoint)
	{
		const int32 RequiredWeakPointCount = FHorrorCampaign::CountObjectivesOfType(*ActiveChapter, EHorrorCampaignObjectiveType::BossWeakPoint);
		bBossDefeated = RequiredWeakPointCount > 0
			&& CountCompletedObjectivesOfType(EHorrorCampaignObjectiveType::BossWeakPoint) >= RequiredWeakPointCount;
	}

	return true;
}

bool FHorrorCampaignProgress::IsChapterComplete() const
{
	if (!ActiveChapter)
	{
		return false;
	}

	return AreAllRequiredObjectivesComplete() && (!ActiveChapter->bRequiresBoss || bBossDefeated);
}

FName FHorrorCampaignProgress::GetActiveChapterId() const
{
	return ActiveChapter ? ActiveChapter->ChapterId : NAME_None;
}

int32 FHorrorCampaignProgress::GetRequiredObjectiveCount() const
{
	return ActiveChapter ? FHorrorCampaign::CountRequiredObjectives(*ActiveChapter) : 0;
}

const FHorrorCampaignObjectiveDefinition* FHorrorCampaignProgress::GetNextObjective() const
{
	if (!ActiveChapter)
	{
		return nullptr;
	}

	for (const FHorrorCampaignObjectiveDefinition& Objective : ActiveChapter->Objectives)
	{
		if (Objective.bRequiredForChapterCompletion && !CompletedObjectiveIds.Contains(Objective.ObjectiveId))
		{
			return &Objective;
		}
	}

	return nullptr;
}

int32 FHorrorCampaignProgress::CountCompletedObjectivesOfType(EHorrorCampaignObjectiveType ObjectiveType) const
{
	if (!ActiveChapter)
	{
		return 0;
	}

	int32 Count = 0;
	for (const FName CompletedObjectiveId : CompletedObjectiveIds)
	{
		const FHorrorCampaignObjectiveDefinition* Objective = FHorrorCampaign::FindObjectiveById(*ActiveChapter, CompletedObjectiveId);
		if (Objective && Objective->ObjectiveType == ObjectiveType)
		{
			++Count;
		}
	}
	return Count;
}

bool FHorrorCampaignProgress::AreAllRequiredObjectivesComplete() const
{
	return ActiveChapter
		&& CompletedObjectiveIds.Num() >= FHorrorCampaign::CountRequiredObjectives(*ActiveChapter);
}
