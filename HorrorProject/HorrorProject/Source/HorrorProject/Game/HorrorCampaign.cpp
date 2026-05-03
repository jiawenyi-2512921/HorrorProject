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

	FHorrorCampaignObjectiveBeat MakeBeat(
		const TCHAR* Label,
		const TCHAR* Detail,
		bool bUrgent = false,
		bool bRequiresRecording = false)
	{
		FHorrorCampaignObjectiveBeat Beat(
			FText::FromString(Label),
			FText::FromString(Detail),
			bUrgent,
			bRequiresRecording);
		Beat.BeatId = FName(Label);
		return Beat;
	}

	void ApplyBeatRulesForObjectiveType(
		EHorrorCampaignObjectiveType ObjectiveType,
		EHorrorCampaignInteractionMode InteractionMode,
		TArray<FHorrorCampaignObjectiveBeat>& Beats)
	{
		for (int32 BeatIndex = 0; BeatIndex < Beats.Num(); ++BeatIndex)
		{
			FHorrorCampaignObjectiveBeat& Beat = Beats[BeatIndex];
			Beat.NavigationRole = EHorrorCampaignObjectiveBeatNavigationRole::ObjectiveActor;
			Beat.FailurePolicy = EHorrorCampaignObjectiveBeatFailurePolicy::None;

			if (BeatIndex == Beats.Num() - 1)
			{
				Beat.CompletionRule = EHorrorCampaignObjectiveBeatCompletionRule::ConfirmResult;
				Beat.NavigationRole = EHorrorCampaignObjectiveBeatNavigationRole::ConfirmationPoint;
			}
			else if (InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring
				|| InteractionMode == EHorrorCampaignInteractionMode::GearCalibration
				|| InteractionMode == EHorrorCampaignInteractionMode::SpectralScan
				|| InteractionMode == EHorrorCampaignInteractionMode::SignalTuning)
			{
				Beat.CompletionRule = BeatIndex == 1
					? EHorrorCampaignObjectiveBeatCompletionRule::AdvancedWindow
					: EHorrorCampaignObjectiveBeatCompletionRule::InteractOnce;
				Beat.FailurePolicy = BeatIndex == 1
					? EHorrorCampaignObjectiveBeatFailurePolicy::RetryBeat
					: EHorrorCampaignObjectiveBeatFailurePolicy::None;
				Beat.bOpensInteractionPanel = BeatIndex == 1;
			}
			else if (InteractionMode == EHorrorCampaignInteractionMode::TimedPursuit)
			{
				Beat.CompletionRule = BeatIndex == 1
					? EHorrorCampaignObjectiveBeatCompletionRule::ReachEscapePoint
					: EHorrorCampaignObjectiveBeatCompletionRule::HoldInteract;
				Beat.FailurePolicy = EHorrorCampaignObjectiveBeatFailurePolicy::CampaignRecovery;
				Beat.NavigationRole = BeatIndex == 1
					? EHorrorCampaignObjectiveBeatNavigationRole::EscapeDestination
					: EHorrorCampaignObjectiveBeatNavigationRole::ObjectiveActor;
				Beat.bUrgent = true;
			}
			else if (InteractionMode == EHorrorCampaignInteractionMode::MultiStep)
			{
				Beat.CompletionRule = EHorrorCampaignObjectiveBeatCompletionRule::HoldInteract;
				Beat.FailurePolicy = ObjectiveType == EHorrorCampaignObjectiveType::BossWeakPoint
					? EHorrorCampaignObjectiveBeatFailurePolicy::CampaignRecovery
					: EHorrorCampaignObjectiveBeatFailurePolicy::RetryBeat;
			}
			else
			{
				Beat.CompletionRule = EHorrorCampaignObjectiveBeatCompletionRule::InteractOnce;
			}
		}
	}

	TArray<FHorrorCampaignObjectiveBeat> MakeDefaultObjectiveBeats(EHorrorCampaignObjectiveType ObjectiveType, EHorrorCampaignInteractionMode InteractionMode)
	{
		TArray<FHorrorCampaignObjectiveBeat> Beats;
		switch (ObjectiveType)
		{
			case EHorrorCampaignObjectiveType::AcquireSignal:
				Beats = {
					MakeBeat(TEXT("定位信号"), TEXT("靠近设备并确认黑盒频率来源。")),
					MakeBeat(TEXT("稳定频率"), TEXT("压住噪点，让信号从杂音里显形。")),
					MakeBeat(TEXT("确认回放"), TEXT("等待画面稳定后记录下一段坐标。"))
				};
				break;
			case EHorrorCampaignObjectiveType::ScanAnomaly:
				Beats = {
					MakeBeat(TEXT("取景"), TEXT("把镜头或扫描器对准异常源。")),
					MakeBeat(TEXT("采样"), TEXT("保持设备稳定，读取异常的脉冲节律。")),
					MakeBeat(TEXT("归档"), TEXT("确认采样完成，把结果写入黑盒记录。"))
				};
				break;
			case EHorrorCampaignObjectiveType::RecoverRelic:
				Beats = {
					MakeBeat(TEXT("辨认遗物"), TEXT("确认目标与黑盒记录中的形状一致。")),
					MakeBeat(TEXT("取出遗物"), TEXT("双手托住遗物，让回响保持稳定。")),
					MakeBeat(TEXT("送回锚点"), TEXT("沿导航把遗物带回祭坛或读取器，完成归档。"))
				};
				break;
			case EHorrorCampaignObjectiveType::RestorePower:
				Beats = {
					MakeBeat(TEXT("断电检查"), TEXT("确认端子顺序，避开烧毁的旧线路。")),
					MakeBeat(TEXT("接线同步"), TEXT("在发光窗口内接入正确线路。")),
					MakeBeat(TEXT("合闸确认"), TEXT("听到设备回鸣后锁住供电。"))
				};
				break;
			case EHorrorCampaignObjectiveType::PlantBeacon:
				Beats = {
					MakeBeat(TEXT("选择落点"), TEXT("把信标放在回声最强的位置。")),
					MakeBeat(TEXT("调谐锚点"), TEXT("在声像居中后锁住信标频率，让路径从雾里显现。")),
					MakeBeat(TEXT("确认回声"), TEXT("观察周围异常是否退潮，并归档新的安全路线。"))
				};
				break;
			case EHorrorCampaignObjectiveType::SurviveAmbush:
				Beats = {
					MakeBeat(TEXT("惊醒巨人"), TEXT("触发装置后石像巨人会从身后追来。"), true),
					MakeBeat(TEXT("逃向亮点"), TEXT("不要回头，沿导航光跑向集合点。"), true),
					MakeBeat(TEXT("确认甩脱"), TEXT("抵达目标点并等巨人的脚步声远去。"), true)
				};
				break;
			case EHorrorCampaignObjectiveType::DisableSeal:
				Beats = {
					MakeBeat(TEXT("读取封印"), TEXT("确认锁轮上缺失的齿位。")),
					MakeBeat(TEXT("齿轮校准"), TEXT("在校准窗口内拨动停转齿轮。")),
					MakeBeat(TEXT("解除锁止"), TEXT("等待机械咬合后拉开封印。"))
				};
				break;
			case EHorrorCampaignObjectiveType::BossWeakPoint:
				Beats = {
					MakeBeat(TEXT("确认弱点"), TEXT("绕开巨人攻击，锁定发光裂纹。"), true),
					MakeBeat(TEXT("频谱压制"), TEXT("在裂纹回声进入锁定带时压住对应波段。"), true),
					MakeBeat(TEXT("击碎封印"), TEXT("趁共鸣失稳前完成最后一次破坏。"), true)
				};
				break;
			case EHorrorCampaignObjectiveType::FinalTerminal:
				Beats = {
					MakeBeat(TEXT("装入录像带"), TEXT("把完整黑盒记录接入终端。")),
					MakeBeat(TEXT("校准回放"), TEXT("同步深水站、林地和王座厅的残响。")),
					MakeBeat(TEXT("确认真相"), TEXT("播放最后一帧并保存结局信号。"))
				};
				break;
			default:
				Beats = {
					MakeBeat(TEXT("接近目标"), TEXT("确认目标位置和周围风险。")),
					MakeBeat(TEXT("执行操作"), TEXT("完成设备或证物的核心互动。")),
					MakeBeat(TEXT("确认结果"), TEXT("等待系统反馈并更新任务链。"))
				};
				break;
		}

		ApplyBeatRulesForObjectiveType(ObjectiveType, InteractionMode, Beats);
		return Beats;
	}

	FName MakeRewardEvidenceId(FName ObjectiveId)
	{
		return ObjectiveId.IsNone()
			? NAME_None
			: FName(*FString::Printf(TEXT("Evidence.Campaign.%s"), *ObjectiveId.ToString()));
	}

	FText MakeRewardEvidenceDisplayName(const FText& PromptText)
	{
		return PromptText.IsEmpty()
			? NSLOCTEXT("HorrorCampaign", "RewardEvidenceFallbackTitle", "任务证据")
			: FText::Format(
				NSLOCTEXT("HorrorCampaign", "RewardEvidenceTitle", "证据：{0}"),
				PromptText);
	}

	FText MakeRewardEvidenceDescription(const FText& PromptText, const FText& CompletionText)
	{
		if (!CompletionText.IsEmpty())
		{
			return FText::Format(
				NSLOCTEXT("HorrorCampaign", "RewardEvidenceDescriptionWithCompletion", "任务完成后归档：{0}"),
				CompletionText);
		}

		return PromptText.IsEmpty()
			? NSLOCTEXT("HorrorCampaign", "RewardEvidenceDescriptionFallback", "这条证据来自当前任务链。")
			: FText::Format(
				NSLOCTEXT("HorrorCampaign", "RewardEvidenceDescriptionWithPrompt", "调查目标归档：{0}"),
				PromptText);
	}

	FHorrorCampaignObjectiveReward MakeDefaultObjectiveReward(
		FName ObjectiveId,
		EHorrorCampaignObjectiveType ObjectiveType,
		const FText& PromptText,
		const FText& CompletionText)
	{
		FHorrorCampaignObjectiveReward Reward;
		switch (ObjectiveType)
		{
			case EHorrorCampaignObjectiveType::AcquireSignal:
			case EHorrorCampaignObjectiveType::ScanAnomaly:
			case EHorrorCampaignObjectiveType::RecoverRelic:
				Reward.RewardText = NSLOCTEXT("HorrorCampaign", "RewardEvidenceAndFocus", "新增证据，恐惧下降，相机电量小幅恢复");
				Reward.EvidenceId = MakeRewardEvidenceId(ObjectiveId);
				Reward.EvidenceDisplayName = MakeRewardEvidenceDisplayName(PromptText);
				Reward.EvidenceDescription = MakeRewardEvidenceDescription(PromptText, CompletionText);
				Reward.FearRelief = 8.0f;
				Reward.BatteryChargePercent = 5.0f;
				break;
			case EHorrorCampaignObjectiveType::RestorePower:
				Reward.RewardText = NSLOCTEXT("HorrorCampaign", "RewardPowerRestored", "供电稳定，相机电量恢复");
				Reward.BatteryChargePercent = 12.0f;
				break;
			case EHorrorCampaignObjectiveType::PlantBeacon:
				Reward.RewardText = NSLOCTEXT("HorrorCampaign", "RewardBeaconAnchor", "路径被锚定，恐惧下降");
				Reward.FearRelief = 10.0f;
				break;
			case EHorrorCampaignObjectiveType::SurviveAmbush:
				Reward.RewardText = NSLOCTEXT("HorrorCampaign", "RewardSurvivalBreath", "暂时甩脱威胁，恐惧明显下降");
				Reward.FearRelief = 16.0f;
				break;
			case EHorrorCampaignObjectiveType::DisableSeal:
				Reward.RewardText = NSLOCTEXT("HorrorCampaign", "RewardSealDisabled", "封印解除，路线稳定");
				Reward.FearRelief = 5.0f;
				break;
			case EHorrorCampaignObjectiveType::BossWeakPoint:
				Reward.RewardText = NSLOCTEXT("HorrorCampaign", "RewardBossWeakPoint", "巨人共鸣被压制，恐惧大幅下降");
				Reward.FearRelief = 18.0f;
				break;
			case EHorrorCampaignObjectiveType::FinalTerminal:
				Reward.RewardText = NSLOCTEXT("HorrorCampaign", "RewardFinalTruth", "真相证据归档，结局信号锁定");
				Reward.EvidenceId = MakeRewardEvidenceId(ObjectiveId);
				Reward.EvidenceDisplayName = MakeRewardEvidenceDisplayName(PromptText);
				Reward.EvidenceDescription = MakeRewardEvidenceDescription(PromptText, CompletionText);
				Reward.AchievementId = TEXT("Achievement.Campaign.FinalTruth");
				break;
			default:
				break;
		}

		return Reward;
	}

	FHorrorCampaignObjectivePresentation MakeDefaultObjectivePresentation(
		EHorrorCampaignObjectiveType ObjectiveType,
		EHorrorCampaignInteractionMode InteractionMode,
		const FText& PromptText)
	{
		FHorrorCampaignObjectivePresentation Presentation;
		Presentation.bUsesFocusedInteraction = true;
		Presentation.bOpensInteractionPanel =
			InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring
			|| InteractionMode == EHorrorCampaignInteractionMode::GearCalibration
			|| InteractionMode == EHorrorCampaignInteractionMode::SpectralScan
			|| InteractionMode == EHorrorCampaignInteractionMode::SignalTuning;

		switch (InteractionMode)
		{
			case EHorrorCampaignInteractionMode::CircuitWiring:
				Presentation.MechanicLabel = NSLOCTEXT("HorrorCampaign", "PresentationCircuit", "电路接线窗口");
				Presentation.InputHint = NSLOCTEXT("HorrorCampaign", "PresentationCircuitInput", "互动键打开接线窗口，按 A/S/D 接入对应端子。");
				Presentation.FailureStakes = NSLOCTEXT("HorrorCampaign", "PresentationCircuitStakes", "接错端子会回退进度并产生火花噪声。");
				Presentation.RiskLevel = EHorrorCampaignObjectiveRiskLevel::High;
				break;
			case EHorrorCampaignInteractionMode::GearCalibration:
				Presentation.MechanicLabel = NSLOCTEXT("HorrorCampaign", "PresentationGear", "齿轮校准窗口");
				Presentation.InputHint = NSLOCTEXT("HorrorCampaign", "PresentationGearInput", "互动键打开齿轮盘，按 A/S/D 拨动停转齿轮。");
				Presentation.FailureStakes = NSLOCTEXT("HorrorCampaign", "PresentationGearStakes", "未及时拨动会让齿轮卡死并暂停校准。");
				Presentation.RiskLevel = EHorrorCampaignObjectiveRiskLevel::High;
				break;
			case EHorrorCampaignInteractionMode::SpectralScan:
				Presentation.MechanicLabel = NSLOCTEXT("HorrorCampaign", "PresentationSpectralScan", "频谱扫描窗口");
				Presentation.InputHint = NSLOCTEXT("HorrorCampaign", "PresentationSpectralScanInput", "互动键打开扫描窗口，先按 A/D 主动扫频滤噪，再按 S 锁定异常峰。");
				Presentation.FailureStakes = NSLOCTEXT("HorrorCampaign", "PresentationSpectralScanStakes", "锁错波段会让噪点反冲，扫描稳定度下降。");
				Presentation.RiskLevel = EHorrorCampaignObjectiveRiskLevel::High;
				break;
			case EHorrorCampaignInteractionMode::SignalTuning:
				Presentation.MechanicLabel = NSLOCTEXT("HorrorCampaign", "PresentationSignalTuning", "信号调谐窗口");
				Presentation.InputHint = NSLOCTEXT("HorrorCampaign", "PresentationSignalTuningInput", "互动键打开调谐窗口，按 A/D 微调左右声道，声像居中后按 S 锁定中心频率。");
				Presentation.FailureStakes = NSLOCTEXT("HorrorCampaign", "PresentationSignalTuningStakes", "调谐失误会放大杂音，黑盒回放短暂失真。");
				Presentation.RiskLevel = EHorrorCampaignObjectiveRiskLevel::Medium;
				break;
			case EHorrorCampaignInteractionMode::TimedPursuit:
				Presentation.MechanicLabel = NSLOCTEXT("HorrorCampaign", "PresentationPursuit", "巨人追逐");
				Presentation.InputHint = NSLOCTEXT("HorrorCampaign", "PresentationPursuitInput", "互动键惊醒石像巨人后，立刻跑向导航目标点。");
				Presentation.FailureStakes = NSLOCTEXT("HorrorCampaign", "PresentationPursuitStakes", "被巨人追上会触发失败恢复并回到目标附近。");
				Presentation.RiskLevel = EHorrorCampaignObjectiveRiskLevel::Critical;
				break;
			case EHorrorCampaignInteractionMode::MultiStep:
				Presentation.MechanicLabel = NSLOCTEXT("HorrorCampaign", "PresentationMultiStep", "多段压制");
				Presentation.InputHint = NSLOCTEXT("HorrorCampaign", "PresentationMultiStepInput", "按互动键推进阶段，观察任务窗口的当前步骤。");
				Presentation.FailureStakes = NSLOCTEXT("HorrorCampaign", "PresentationMultiStepStakes", "阶段未完成前，目标不会归档，威胁仍会持续。");
				Presentation.RiskLevel = EHorrorCampaignObjectiveRiskLevel::High;
				break;
			case EHorrorCampaignInteractionMode::Instant:
			default:
				Presentation.MechanicLabel = NSLOCTEXT("HorrorCampaign", "PresentationInstant", "直接调查");
				Presentation.InputHint = NSLOCTEXT("HorrorCampaign", "PresentationInstantInput", "靠近目标后按互动键。");
				Presentation.FailureStakes = NSLOCTEXT("HorrorCampaign", "PresentationInstantStakes", "未完成前，后续路线保持锁定。");
				Presentation.RiskLevel = EHorrorCampaignObjectiveRiskLevel::Medium;
				break;
		}

		if (ObjectiveType == EHorrorCampaignObjectiveType::AcquireSignal
			|| ObjectiveType == EHorrorCampaignObjectiveType::RecoverRelic)
		{
			Presentation.RiskLevel = InteractionMode == EHorrorCampaignInteractionMode::Instant
				? Presentation.RiskLevel
				: EHorrorCampaignObjectiveRiskLevel::High;
		}

		if (ObjectiveType == EHorrorCampaignObjectiveType::RecoverRelic)
		{
			if (InteractionMode == EHorrorCampaignInteractionMode::MultiStep)
			{
				Presentation.MechanicLabel = NSLOCTEXT("HorrorCampaign", "PresentationRelicCarryReturn", "遗物搬运回锚点");
				Presentation.InputHint = NSLOCTEXT("HorrorCampaign", "PresentationRelicCarryReturnInput", "第一次互动取出遗物，沿导航送回发光锚点，再次互动完成归档。");
				Presentation.FailureStakes = NSLOCTEXT("HorrorCampaign", "PresentationRelicCarryReturnStakes", "遗物未送回前，后续封印和机关保持锁定。");
				Presentation.bOpensInteractionPanel = false;
				Presentation.RiskLevel = EHorrorCampaignObjectiveRiskLevel::High;
			}
			else if (InteractionMode == EHorrorCampaignInteractionMode::SpectralScan)
			{
				Presentation.MechanicLabel = NSLOCTEXT("HorrorCampaign", "PresentationRelicSpectralSeal", "遗物封装频谱窗口");
				Presentation.InputHint = NSLOCTEXT("HorrorCampaign", "PresentationRelicSpectralSealInput", "互动键打开封装窗口，先按 A/D 扫频压住遗物回响，再按 S 锁定。");
				Presentation.FailureStakes = NSLOCTEXT("HorrorCampaign", "PresentationRelicSpectralSealStakes", "锁错回响会让遗物噪点反冲，封装进度回退。");
				Presentation.bOpensInteractionPanel = true;
				Presentation.RiskLevel = EHorrorCampaignObjectiveRiskLevel::High;
			}
			else if (InteractionMode == EHorrorCampaignInteractionMode::GearCalibration)
			{
				Presentation.MechanicLabel = NSLOCTEXT("HorrorCampaign", "PresentationRelicGearSeal", "遗物机械封装窗口");
				Presentation.InputHint = NSLOCTEXT("HorrorCampaign", "PresentationRelicGearSealInput", "互动键打开机械封装盘，按 A/S/D 拨动停转齿轮锁住遗物外壳。");
				Presentation.FailureStakes = NSLOCTEXT("HorrorCampaign", "PresentationRelicGearSealStakes", "拨错齿轮会让封装卡死，遗物回响短暂扩散。");
				Presentation.bOpensInteractionPanel = true;
				Presentation.RiskLevel = EHorrorCampaignObjectiveRiskLevel::High;
			}
		}

		if (ObjectiveType == EHorrorCampaignObjectiveType::BossWeakPoint)
		{
			Presentation.MechanicLabel = NSLOCTEXT("HorrorCampaign", "PresentationBossWeakPoint", "首领共鸣频谱窗口");
			Presentation.InputHint = NSLOCTEXT("HorrorCampaign", "PresentationBossWeakPointInput", "互动键打开频谱压制窗口，先按 A/D 扫频压住裂纹回声，再按 S 锁定。");
			Presentation.FailureStakes = NSLOCTEXT("HorrorCampaign", "PresentationBossWeakPointStakes", "锁错波段会让巨人共鸣反冲，弱点压制进度回退。");
			Presentation.bOpensInteractionPanel = true;
			Presentation.RiskLevel = EHorrorCampaignObjectiveRiskLevel::Critical;
		}
		else if (ObjectiveType == EHorrorCampaignObjectiveType::PlantBeacon)
		{
			Presentation.MechanicLabel = NSLOCTEXT("HorrorCampaign", "PresentationPlantBeacon", "信标锚定调谐窗口");
			Presentation.InputHint = NSLOCTEXT("HorrorCampaign", "PresentationPlantBeaconInput", "互动键打开信标调谐窗口，按 A/D 微调声像，按 S 锁定锚点。");
			Presentation.FailureStakes = NSLOCTEXT("HorrorCampaign", "PresentationPlantBeaconStakes", "锚定失误会让路径回声扩散，信标进度回退。");
			Presentation.bOpensInteractionPanel = true;
			Presentation.RiskLevel = EHorrorCampaignObjectiveRiskLevel::High;
		}
		else if (ObjectiveType == EHorrorCampaignObjectiveType::FinalTerminal)
		{
			Presentation.MechanicLabel = NSLOCTEXT("HorrorCampaign", "PresentationFinalTerminal", "黑盒终章调谐");
			Presentation.InputHint = NSLOCTEXT("HorrorCampaign", "PresentationFinalTerminalInput", "互动键打开终章调谐窗口，按 A/D 调整声像，按 S 锁定完整录像。");
			Presentation.FailureStakes = NSLOCTEXT("HorrorCampaign", "PresentationFinalTerminalStakes", "终章调谐失败会让黑盒回放失真，结局出口不会开启。");
			Presentation.bOpensInteractionPanel = true;
			Presentation.RiskLevel = EHorrorCampaignObjectiveRiskLevel::High;
		}

		Presentation.MissionContext = PromptText.IsEmpty()
			? NSLOCTEXT("HorrorCampaign", "PresentationMissionContextFallback", "完成当前目标以推进任务链。")
			: FText::Format(
				NSLOCTEXT("HorrorCampaign", "PresentationMissionContext", "当前目标：{0}"),
				PromptText);
		return Presentation;
	}

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
				Objective.InteractionMode = EHorrorCampaignInteractionMode::SignalTuning;
				break;
			case EHorrorCampaignObjectiveType::BossWeakPoint:
				Objective.InteractionMode = EHorrorCampaignInteractionMode::SpectralScan;
				break;
			case EHorrorCampaignObjectiveType::ScanAnomaly:
				Objective.InteractionMode = EHorrorCampaignInteractionMode::SpectralScan;
				break;
			case EHorrorCampaignObjectiveType::AcquireSignal:
			case EHorrorCampaignObjectiveType::FinalTerminal:
				Objective.InteractionMode = EHorrorCampaignInteractionMode::SignalTuning;
				break;
			case EHorrorCampaignObjectiveType::RecoverRelic:
				Objective.InteractionMode = EHorrorCampaignInteractionMode::SpectralScan;
				break;
			default:
				Objective.InteractionMode = EHorrorCampaignInteractionMode::Instant;
				break;
		}
		Objective.PromptText = FText::FromString(PromptText);
		Objective.CompletionText = FText::FromString(CompletionText);
		Objective.ObjectiveBeats = MakeDefaultObjectiveBeats(ObjectiveType, Objective.InteractionMode);
		Objective.Reward = MakeDefaultObjectiveReward(
			Objective.ObjectiveId,
			ObjectiveType,
			Objective.PromptText,
			Objective.CompletionText);
		Objective.Presentation = MakeDefaultObjectivePresentation(
			ObjectiveType,
			Objective.InteractionMode,
			Objective.PromptText);
		Objective.RelativeLocation = RelativeLocation;
		Objective.EscapeDestinationOffset = EscapeDestinationOffset;
		Objective.EscapeCompletionRadius = EscapeCompletionRadius;
		Objective.VisualMeshPath = FSoftObjectPath(VisualMeshPath);
		return Objective;
	}

	FHorrorCampaignObjectiveDefinition WithPrerequisites(
		FHorrorCampaignObjectiveDefinition Objective,
		std::initializer_list<const TCHAR*> PrerequisiteObjectiveIds)
	{
		Objective.PrerequisiteObjectiveIds.Reserve(static_cast<int32>(PrerequisiteObjectiveIds.size()));
		for (const TCHAR* PrerequisiteObjectiveId : PrerequisiteObjectiveIds)
		{
			Objective.PrerequisiteObjectiveIds.Add(FName(PrerequisiteObjectiveId));
		}
		return Objective;
	}

	FHorrorCampaignObjectiveDefinition WithTimedDuration(
		FHorrorCampaignObjectiveDefinition Objective,
		float DurationSeconds)
	{
		Objective.TimedObjectiveDurationSeconds = FMath::Max(0.1f, DurationSeconds);
		return Objective;
	}

	FHorrorCampaignObjectiveDefinition WithInteractionMode(
		FHorrorCampaignObjectiveDefinition Objective,
		EHorrorCampaignInteractionMode InteractionMode)
	{
		Objective.InteractionMode = InteractionMode;
		Objective.ObjectiveBeats = MakeDefaultObjectiveBeats(Objective.ObjectiveType, Objective.InteractionMode);
		Objective.Presentation = MakeDefaultObjectivePresentation(
			Objective.ObjectiveType,
			Objective.InteractionMode,
			Objective.PromptText);
		return Objective;
	}

	FHorrorCampaignObjectiveDefinition WithRelicDeliveryAnchor(
		FHorrorCampaignObjectiveDefinition Objective,
		const FVector& DeliveryAnchorOffset,
		float CompletionRadius = 220.0f)
	{
		Objective.InteractionMode = EHorrorCampaignInteractionMode::MultiStep;
		Objective.EscapeDestinationOffset = DeliveryAnchorOffset;
		Objective.RelicDeliveryCompletionRadius = FMath::Max(50.0f, CompletionRadius);
		Objective.ObjectiveBeats = MakeDefaultObjectiveBeats(Objective.ObjectiveType, Objective.InteractionMode);
		Objective.Presentation = MakeDefaultObjectivePresentation(
			Objective.ObjectiveType,
			Objective.InteractionMode,
			Objective.PromptText);
		return Objective;
	}

	FHorrorCampaignObjectiveDefinition AsOptionalInvestigation(FHorrorCampaignObjectiveDefinition Objective)
	{
		Objective.bRequiredForChapterCompletion = false;
		Objective.bOptional = true;
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
					AsOptionalInvestigation(MakeObjective(TEXT("DeepWater.ReviewFloodLog"), EHorrorCampaignObjectiveType::AcquireSignal, TEXT("读取干船坞淹没日志"), TEXT("淹没日志已归档，黑盒故障时间被标记。"), FVector(760.0f, -520.0f, 95.0f), ScreenMeshPath)),
					WithPrerequisites(MakeObjective(TEXT("DeepWater.DecodeHeartbeat"), EHorrorCampaignObjectiveType::ScanAnomaly, TEXT("扫描黑盒里的巨人心跳频率"), TEXT("巨人心跳被记录，录像带坐标浮现。"), FVector(840.0f, -40.0f, 95.0f), ScreenMeshPath), { TEXT("DeepWater.BootDryDock") }),
					WithPrerequisites(WithTimedDuration(MakeObjective(TEXT("DeepWater.SurviveDockPursuit"), EHorrorCampaignObjectiveType::SurviveAmbush, TEXT("拉下警报闸，躲开石像巨人并逃向干船坞集合点"), TEXT("警报声压住巨人的脚步，第一段坐标稳定。"), FVector(1320.0f, -120.0f, 95.0f), DeviceMeshPath, FVector(2450.0f, 720.0f, 0.0f), 280.0f), 18.0f), { TEXT("DeepWater.DecodeHeartbeat") })
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
					AsOptionalInvestigation(MakeObjective(TEXT("Forest.RecordHangingTape"), EHorrorCampaignObjectiveType::AcquireSignal, TEXT("记录树冠里悬挂的残带"), TEXT("残带回声已归档，林地实验编号被保留。"), FVector(1080.0f, -720.0f, 120.0f), ScreenMeshPath)),
					WithPrerequisites(WithTimedDuration(MakeObjective(TEXT("Forest.HoldSpikeCircle"), EHorrorCampaignObjectiveType::SurviveAmbush, TEXT("激活信标后躲开石像巨人，逃向林地汇合点"), TEXT("巨人的脚步被甩开，尖刺退潮，种核暴露。"), FVector(1850.0f, -280.0f, 90.0f), DeviceMeshPath, FVector(2550.0f, 1150.0f, 0.0f), 280.0f), 20.0f), { TEXT("Forest.AlignSpikeBeacon") }),
					WithPrerequisites(MakeObjective(TEXT("Forest.ExtractBlackSeed"), EHorrorCampaignObjectiveType::RecoverRelic, TEXT("取回黑色种核"), TEXT("黑色种核正在发热。"), FVector(2350.0f, 430.0f, 90.0f), RelicMeshPath), { TEXT("Forest.ReadRootGlyph"), TEXT("Forest.HoldSpikeCircle") })
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
					WithInteractionMode(MakeObjective(TEXT("Scrapopolis.RecoverPressureValve"), EHorrorCampaignObjectiveType::RecoverRelic, TEXT("回收蒸汽压力阀"), TEXT("压力阀已接入。"), FVector(650.0f, -420.0f, 95.0f), RelicMeshPath), EHorrorCampaignInteractionMode::GearCalibration),
					AsOptionalInvestigation(MakeObjective(TEXT("Scrapopolis.ReadShiftLedger"), EHorrorCampaignObjectiveType::AcquireSignal, TEXT("读取废铁村轮班账本"), TEXT("轮班账本已归档，失踪工人的编号被写入黑盒。"), FVector(980.0f, -760.0f, 95.0f), ScreenMeshPath)),
					WithPrerequisites(MakeObjective(TEXT("Scrapopolis.RestoreGenerator"), EHorrorCampaignObjectiveType::RestorePower, TEXT("重启废铁村发电机并修复巨人膝关节驱动环"), TEXT("村落电力恢复，巨人关节驱动环重新点亮。"), FVector(1500.0f, 80.0f, 95.0f), TerminalMeshPath), { TEXT("Scrapopolis.RecoverPressureValve") }),
					WithPrerequisites(MakeObjective(TEXT("Scrapopolis.DecodeBridge"), EHorrorCampaignObjectiveType::AcquireSignal, TEXT("解码桥梁控制信号"), TEXT("地下入口坐标已解码。"), FVector(2350.0f, 500.0f, 95.0f), ScreenMeshPath), { TEXT("Scrapopolis.RestoreGenerator") })
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
					AsOptionalInvestigation(MakeObjective(TEXT("Entrance.ReadPilgrimMarks"), EHorrorCampaignObjectiveType::AcquireSignal, TEXT("记录入口石阶上的朝圣刻痕"), TEXT("朝圣刻痕已归档，雾门开启顺序更清晰。"), FVector(980.0f, -620.0f, 95.0f), RelicMeshPath)),
					WithPrerequisites(MakeObjective(TEXT("Entrance.DisableOuterSeal"), EHorrorCampaignObjectiveType::DisableSeal, TEXT("用巨人石片解除外层封印"), TEXT("外层封印已解除。"), FVector(1650.0f, 260.0f, 95.0f), GateMeshPath), { TEXT("Entrance.ScanFogDoor"), TEXT("Entrance.RecoverGolemFinger") })
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
					AsOptionalInvestigation(MakeObjective(TEXT("Depths.RecordCellWhispers"), EHorrorCampaignObjectiveType::AcquireSignal, TEXT("记录囚室墙缝里的低语"), TEXT("囚室低语已归档，巨人苏醒前的祈祷被保留。"), FVector(980.0f, -690.0f, 95.0f), ScreenMeshPath)),
					WithPrerequisites(MakeObjective(TEXT("Depths.PowerArchiveReader"), EHorrorCampaignObjectiveType::RestorePower, TEXT("启动档案读取器并修复巨人记忆石板"), TEXT("档案读取器启动，巨人苏醒记录被重新拼合。"), FVector(1550.0f, 40.0f, 95.0f), TerminalMeshPath), { TEXT("Depths.RecoverArchivePlate") }),
					WithPrerequisites(MakeObjective(TEXT("Depths.MarkEscapeLine"), EHorrorCampaignObjectiveType::PlantBeacon, TEXT("标记回廊逃生线"), TEXT("逃生线已标记。"), FVector(2450.0f, 360.0f, 95.0f), DeviceMeshPath), { TEXT("Depths.PowerArchiveReader") })
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
					AsOptionalInvestigation(MakeObjective(TEXT("Hall.ReadMechanistPrayer"), EHorrorCampaignObjectiveType::AcquireSignal, TEXT("读取机械师留在梁柱上的祷文"), TEXT("机械师祷文已归档，机关大厅的真实建造者浮现。"), FVector(1280.0f, -720.0f, 95.0f), RelicMeshPath)),
					WithPrerequisites(MakeObjective(TEXT("Hall.CalibrateGolemRestraint"), EHorrorCampaignObjectiveType::RestorePower, TEXT("校准巨人束缚机关"), TEXT("束缚机关短暂锁住了巨人回声。"), FVector(1900.0f, -360.0f, 95.0f), TerminalMeshPath), { TEXT("Hall.DisableBladeRhythm"), TEXT("Hall.ScanGolemEcho") }),
					WithPrerequisites(WithTimedDuration(MakeObjective(TEXT("Hall.SurviveEchoPulse"), EHorrorCampaignObjectiveType::SurviveAmbush, TEXT("躲开石像巨人的震荡追赶，跑向大厅亮点"), TEXT("巨人回声被甩在机关大厅后方。"), FVector(2100.0f, -260.0f, 95.0f), RelicMeshPath, FVector(450.0f, 700.0f, 0.0f), 360.0f), 24.0f), { TEXT("Hall.CalibrateGolemRestraint") }),
					WithPrerequisites(MakeObjective(TEXT("Hall.AnchorSafeRoute"), EHorrorCampaignObjectiveType::PlantBeacon, TEXT("锚定安全路线"), TEXT("安全路线已锚定。"), FVector(2550.0f, 440.0f, 95.0f), DeviceMeshPath), { TEXT("Hall.SurviveEchoPulse") })
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
					WithRelicDeliveryAnchor(MakeObjective(TEXT("Temple.RecoverKeySigil"), EHorrorCampaignObjectiveType::RecoverRelic, TEXT("寻找钥匙碎片：取回发光钥印"), TEXT("钥匙碎片已回收。"), FVector(1260.0f, -80.0f, 105.0f), DungeonKeyMeshPath), FVector(-500.0f, -280.0f, 0.0f), 220.0f),
					AsOptionalInvestigation(MakeObjective(TEXT("Temple.RecordSunkenMural"), EHorrorCampaignObjectiveType::ScanAnomaly, TEXT("记录沉没壁画上的黑盒图案"), TEXT("沉没壁画已归档，黑盒不是现代设备的线索被保留。"), FVector(980.0f, -720.0f, 100.0f), ScreenMeshPath)),
					WithPrerequisites(MakeObjective(TEXT("Temple.RepairGolemHeartConduit"), EHorrorCampaignObjectiveType::RestorePower, TEXT("修复巨人心核导管"), TEXT("巨人心核导管开始低频震动。"), FVector(1850.0f, 220.0f, 95.0f), TerminalMeshPath), { TEXT("Temple.ChargeAltar"), TEXT("Temple.RecoverKeySigil") }),
					WithPrerequisites(MakeObjective(TEXT("Temple.OpenBossSeal"), EHorrorCampaignObjectiveType::DisableSeal, TEXT("打开巨人王座厅封印"), TEXT("巨人王座厅封印开启。"), FVector(2600.0f, 380.0f, 95.0f), GateMeshPath), { TEXT("Temple.RepairGolemHeartConduit") })
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
					AsOptionalInvestigation(MakeObjective(TEXT("Boss.ScanBrokenCrown"), EHorrorCampaignObjectiveType::ScanAnomaly, TEXT("扫描巨人断冠里的古老编号"), TEXT("断冠编号已归档，巨人的身份线索被保留。"), FVector(740.0f, 700.0f, 125.0f), RelicMeshPath)),
					WithPrerequisites(MakeObjective(TEXT("Boss.WeakPoint.LeftShoulder"), EHorrorCampaignObjectiveType::BossWeakPoint, TEXT("击碎左肩共鸣弱点"), TEXT("左肩弱点碎裂。"), FVector(1100.0f, -520.0f, 115.0f), RelicMeshPath), { TEXT("Boss.ActivateArenaAltar") }),
					WithPrerequisites(MakeObjective(TEXT("Boss.WeakPoint.Chest"), EHorrorCampaignObjectiveType::BossWeakPoint, TEXT("击碎胸口共鸣弱点"), TEXT("胸口核心暴露。"), FVector(1600.0f, 0.0f, 115.0f), RelicMeshPath), { TEXT("Boss.WeakPoint.LeftShoulder") }),
					WithPrerequisites(MakeObjective(TEXT("Boss.WeakPoint.RightArm"), EHorrorCampaignObjectiveType::BossWeakPoint, TEXT("击碎右臂共鸣弱点"), TEXT("右臂弱点碎裂。"), FVector(1100.0f, 520.0f, 115.0f), RelicMeshPath), { TEXT("Boss.WeakPoint.Chest") }),
					WithPrerequisites(MakeObjective(TEXT("Boss.SealCore"), EHorrorCampaignObjectiveType::DisableSeal, TEXT("封存巨人核心"), TEXT("巨人倒下，升降井打开。"), FVector(2100.0f, 0.0f, 115.0f), GateMeshPath), { TEXT("Boss.WeakPoint.RightArm") })
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
					AsOptionalInvestigation(MakeObjective(TEXT("Signal.RecordMissingFrame"), EHorrorCampaignObjectiveType::ScanAnomaly, TEXT("记录录像带缺失的一帧"), TEXT("缺失画面已归档，结局旁白出现新的证词。"), FVector(980.0f, -620.0f, 90.0f), ScreenMeshPath)),
					WithPrerequisites(MakeObjective(TEXT("Signal.ScanCorridor"), EHorrorCampaignObjectiveType::ScanAnomaly, TEXT("扫描走廊尽头的深水站倒影"), TEXT("深水站倒影与黑盒频率重合。"), FVector(1450.0f, 220.0f, 90.0f), ScreenMeshPath), { TEXT("Signal.CalibrateLens") }),
					WithPrerequisites(WithTimedDuration(MakeObjective(TEXT("Signal.SurviveStaticPursuit"), EHorrorCampaignObjectiveType::SurviveAmbush, TEXT("触发最后的石像巨人追逐，跑向录像带出口"), TEXT("巨人的残响被甩在录像带之外。"), FVector(2050.0f, -300.0f, 90.0f), DeviceMeshPath, FVector(2300.0f, 820.0f, 90.0f), 500.0f), 24.0f), { TEXT("Signal.ScanCorridor") }),
					WithPrerequisites(MakeObjective(TEXT("Signal.FinalTerminal"), EHorrorCampaignObjectiveType::FinalTerminal, TEXT("播放完整黑盒录像带，确认深水站真相"), TEXT("真相归档，黑盒关闭。"), FVector(2620.0f, 580.0f, 90.0f), TerminalMeshPath), { TEXT("Signal.SurviveStaticPursuit") })
				})
		};

		return Chapters;
	}

	void AddValidationIssue(
		TArray<FHorrorCampaignValidationIssue>& Issues,
		FName ChapterId,
		FName ObjectiveId,
		FName IssueCode,
		const FText& Message)
	{
		FHorrorCampaignValidationIssue Issue;
		Issue.ChapterId = ChapterId;
		Issue.ObjectiveId = ObjectiveId;
		Issue.IssueCode = IssueCode;
		Issue.Message = Message;
		Issues.Add(Issue);
	}

	bool HasDependencyCycleVisit(
		const FHorrorCampaignChapterDefinition& Chapter,
		FName ObjectiveId,
		TSet<FName>& Visiting,
		TSet<FName>& Visited)
	{
		if (Visited.Contains(ObjectiveId))
		{
			return false;
		}
		if (Visiting.Contains(ObjectiveId))
		{
			return true;
		}

		Visiting.Add(ObjectiveId);
		const FHorrorCampaignObjectiveDefinition* Objective = FHorrorCampaign::FindObjectiveById(Chapter, ObjectiveId);
		if (Objective)
		{
			for (const FName PrerequisiteObjectiveId : Objective->PrerequisiteObjectiveIds)
			{
				if (!PrerequisiteObjectiveId.IsNone()
					&& FHorrorCampaign::FindObjectiveById(Chapter, PrerequisiteObjectiveId)
					&& HasDependencyCycleVisit(Chapter, PrerequisiteObjectiveId, Visiting, Visited))
				{
					return true;
				}
			}
		}

		Visiting.Remove(ObjectiveId);
		Visited.Add(ObjectiveId);
		return false;
	}

	bool IsObjectiveReachableFromRoots(
		const FHorrorCampaignChapterDefinition& Chapter,
		FName ObjectiveId)
	{
		const FHorrorCampaignObjectiveDefinition* TargetObjective = FHorrorCampaign::FindObjectiveById(Chapter, ObjectiveId);
		if (!TargetObjective)
		{
			return false;
		}
		if (TargetObjective->PrerequisiteObjectiveIds.IsEmpty())
		{
			return true;
		}

		TSet<FName> ReachableObjectiveIds;
		bool bChanged = true;
		while (bChanged)
		{
			bChanged = false;
			for (const FHorrorCampaignObjectiveDefinition& Objective : Chapter.Objectives)
			{
				if (ReachableObjectiveIds.Contains(Objective.ObjectiveId))
				{
					continue;
				}

				bool bPrerequisitesReachable = true;
				for (const FName PrerequisiteObjectiveId : Objective.PrerequisiteObjectiveIds)
				{
					if (!PrerequisiteObjectiveId.IsNone() && !ReachableObjectiveIds.Contains(PrerequisiteObjectiveId))
					{
						bPrerequisitesReachable = false;
						break;
					}
				}

				if (bPrerequisitesReachable)
				{
					ReachableObjectiveIds.Add(Objective.ObjectiveId);
					bChanged = true;
				}
			}
		}

		return ReachableObjectiveIds.Contains(ObjectiveId);
	}

	bool IsAdvancedCampaignInteractionMode(EHorrorCampaignInteractionMode InteractionMode)
	{
		return InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring
			|| InteractionMode == EHorrorCampaignInteractionMode::GearCalibration
			|| InteractionMode == EHorrorCampaignInteractionMode::SpectralScan
			|| InteractionMode == EHorrorCampaignInteractionMode::SignalTuning;
	}

	bool IsInstantInteractionAllowedForRequiredObjective(const FHorrorCampaignObjectiveDefinition& Objective)
	{
		return !Objective.bRequiredForChapterCompletion
			|| Objective.bOptional
			|| Objective.ObjectiveType == EHorrorCampaignObjectiveType::SurviveAmbush;
	}

	bool IsCarryReturnRelicObjective(const FHorrorCampaignObjectiveDefinition& Objective)
	{
		return Objective.ObjectiveType == EHorrorCampaignObjectiveType::RecoverRelic
			&& Objective.InteractionMode == EHorrorCampaignInteractionMode::MultiStep
			&& Objective.EscapeDestinationOffset.Size2D() >= 100.0f;
	}

	bool IsFiniteCampaignVector(const FVector& Location)
	{
		return FMath::IsFinite(Location.X)
			&& FMath::IsFinite(Location.Y)
			&& FMath::IsFinite(Location.Z)
			&& !Location.ContainsNaN();
	}

	bool ContainsDisallowedCampaignEnglishText(const FString& Text)
	{
		for (int32 CharacterIndex = 0; CharacterIndex < Text.Len(); ++CharacterIndex)
		{
			const TCHAR Character = Text[CharacterIndex];
			if ((Character >= TEXT('A') && Character <= TEXT('Z')) || (Character >= TEXT('a') && Character <= TEXT('z')))
			{
				if ((Character == TEXT('A') || Character == TEXT('D') || Character == TEXT('S'))
					&& (CharacterIndex == 0 || !FChar::IsAlpha(Text[CharacterIndex - 1]))
					&& (CharacterIndex == Text.Len() - 1 || !FChar::IsAlpha(Text[CharacterIndex + 1])))
				{
					continue;
				}
				return true;
			}
		}

		return false;
	}

	void ValidateCampaignTextField(
		const FString& Text,
		TArray<FHorrorCampaignValidationIssue>& Issues,
		FName ChapterId,
		FName ObjectiveId)
	{
		if (ContainsDisallowedCampaignEnglishText(Text))
		{
			AddValidationIssue(
				Issues,
				ChapterId,
				ObjectiveId,
				TEXT("Campaign.Text.NonLocalized"),
				NSLOCTEXT("HorrorCampaignValidation", "NonLocalizedCampaignText", "玩家可见任务文案包含英文，请改为中文。"));
		}
	}

	void ValidateCampaignAuthoredLocation(
		const FVector& Location,
		TArray<FHorrorCampaignValidationIssue>& Issues,
		FName ChapterId,
		FName ObjectiveId,
		FName IssueCode,
		const FText& Message,
		float MaxHorizontalDistanceCm = 3600.0f,
		float MaxAbsZCm = 450.0f)
	{
		if (!IsFiniteCampaignVector(Location)
			|| Location.Size2D() > MaxHorizontalDistanceCm
			|| FMath::Abs(Location.Z) > MaxAbsZCm)
		{
			AddValidationIssue(
				Issues,
				ChapterId,
				ObjectiveId,
				IssueCode,
				Message);
		}
	}

	void ValidateCampaignChapterDefinition(
		const FHorrorCampaignChapterDefinition& Chapter,
		TArray<FHorrorCampaignValidationIssue>& Issues,
		bool bValidateChapterMix)
	{
		if (Chapter.ChapterId.IsNone())
		{
			AddValidationIssue(
				Issues,
				NAME_None,
				NAME_None,
				TEXT("Campaign.Chapter.MissingId"),
				NSLOCTEXT("HorrorCampaignValidation", "MissingChapterId", "章节缺少 ChapterId。"));
		}
		ValidateCampaignTextField(Chapter.Title.ToString(), Issues, Chapter.ChapterId, NAME_None);
		ValidateCampaignTextField(Chapter.StoryBrief.ToString(), Issues, Chapter.ChapterId, NAME_None);
		ValidateCampaignTextField(Chapter.CompletionBridgeText.ToString(), Issues, Chapter.ChapterId, NAME_None);

		TSet<FName> ObjectiveIds;
		for (const FHorrorCampaignObjectiveDefinition& Objective : Chapter.Objectives)
		{
			if (Objective.ObjectiveId.IsNone())
			{
				AddValidationIssue(
					Issues,
					Chapter.ChapterId,
					NAME_None,
					TEXT("Campaign.Objective.MissingId"),
					NSLOCTEXT("HorrorCampaignValidation", "MissingObjectiveId", "目标缺少 ObjectiveId。"));
				continue;
			}

			if (ObjectiveIds.Contains(Objective.ObjectiveId))
			{
				AddValidationIssue(
					Issues,
					Chapter.ChapterId,
					Objective.ObjectiveId,
					TEXT("Campaign.Objective.DuplicateId"),
					NSLOCTEXT("HorrorCampaignValidation", "DuplicateObjectiveId", "章节内存在重复 ObjectiveId。"));
			}
			ObjectiveIds.Add(Objective.ObjectiveId);
		}

		for (const FHorrorCampaignObjectiveDefinition& Objective : Chapter.Objectives)
		{
			if (Objective.ObjectiveId.IsNone())
			{
				continue;
			}
			ValidateCampaignTextField(Objective.PromptText.ToString(), Issues, Chapter.ChapterId, Objective.ObjectiveId);
			ValidateCampaignTextField(Objective.CompletionText.ToString(), Issues, Chapter.ChapterId, Objective.ObjectiveId);
			ValidateCampaignTextField(Objective.Presentation.MechanicLabel.ToString(), Issues, Chapter.ChapterId, Objective.ObjectiveId);
			ValidateCampaignTextField(Objective.Presentation.InputHint.ToString(), Issues, Chapter.ChapterId, Objective.ObjectiveId);
			ValidateCampaignTextField(Objective.Presentation.MissionContext.ToString(), Issues, Chapter.ChapterId, Objective.ObjectiveId);
			ValidateCampaignTextField(Objective.Presentation.FailureStakes.ToString(), Issues, Chapter.ChapterId, Objective.ObjectiveId);
			ValidateCampaignTextField(Objective.Reward.RewardText.ToString(), Issues, Chapter.ChapterId, Objective.ObjectiveId);
			ValidateCampaignTextField(Objective.Reward.EvidenceDisplayName.ToString(), Issues, Chapter.ChapterId, Objective.ObjectiveId);
			ValidateCampaignTextField(Objective.Reward.EvidenceDescription.ToString(), Issues, Chapter.ChapterId, Objective.ObjectiveId);
			for (const FHorrorCampaignObjectiveBeat& Beat : Objective.ObjectiveBeats)
			{
				ValidateCampaignTextField(Beat.Label.ToString(), Issues, Chapter.ChapterId, Objective.ObjectiveId);
				ValidateCampaignTextField(Beat.Detail.ToString(), Issues, Chapter.ChapterId, Objective.ObjectiveId);
			}
			ValidateCampaignAuthoredLocation(
				Objective.RelativeLocation,
				Issues,
				Chapter.ChapterId,
				Objective.ObjectiveId,
				TEXT("Campaign.Objective.LocationOutOfBounds"),
				NSLOCTEXT("HorrorCampaignValidation", "ObjectiveLocationOutOfBounds", "目标坐标超出可玩区域或高度带。"));

			for (const FName PrerequisiteObjectiveId : Objective.PrerequisiteObjectiveIds)
			{
				if (PrerequisiteObjectiveId.IsNone()
					|| !FHorrorCampaign::FindObjectiveById(Chapter, PrerequisiteObjectiveId))
				{
					AddValidationIssue(
						Issues,
						Chapter.ChapterId,
						Objective.ObjectiveId,
						TEXT("Campaign.Objective.MissingPrerequisite"),
						FText::Format(
							NSLOCTEXT("HorrorCampaignValidation", "MissingPrerequisite", "前置目标不存在：{0}"),
							FText::FromName(PrerequisiteObjectiveId)));
				}
			}

			if (Objective.bRequiredForChapterCompletion && !IsObjectiveReachableFromRoots(Chapter, Objective.ObjectiveId))
			{
				AddValidationIssue(
					Issues,
					Chapter.ChapterId,
					Objective.ObjectiveId,
					TEXT("Campaign.Objective.UnreachableRequired"),
					NSLOCTEXT("HorrorCampaignValidation", "UnreachableRequiredObjective", "必做目标无法从章节起始目标解锁。"));
			}

			if (Objective.bRequiredForChapterCompletion
				&& Objective.InteractionMode == EHorrorCampaignInteractionMode::Instant
				&& !IsInstantInteractionAllowedForRequiredObjective(Objective))
			{
				AddValidationIssue(
					Issues,
					Chapter.ChapterId,
					Objective.ObjectiveId,
					TEXT("Campaign.Objective.RequiredInstantMainline"),
					NSLOCTEXT("HorrorCampaignValidation", "RequiredInstantMainline", "必做主线目标不能退回一键互动，必须使用窗口化、追逐或复合任务玩法。"));
			}

			if (Objective.InteractionMode == EHorrorCampaignInteractionMode::TimedPursuit)
			{
				if (Objective.EscapeDestinationOffset.IsNearlyZero())
				{
					AddValidationIssue(
						Issues,
						Chapter.ChapterId,
						Objective.ObjectiveId,
						TEXT("Campaign.Objective.PursuitMissingEscapeDestination"),
						NSLOCTEXT("HorrorCampaignValidation", "PursuitMissingEscapeDestination", "追逐目标缺少逃离点偏移。"));
				}
				if (Objective.TimedObjectiveDurationSeconds <= 0.0f)
				{
					AddValidationIssue(
						Issues,
						Chapter.ChapterId,
						Objective.ObjectiveId,
						TEXT("Campaign.Objective.PursuitMissingDuration"),
						NSLOCTEXT("HorrorCampaignValidation", "PursuitMissingDuration", "追逐目标缺少明确倒计时。"));
				}
				const float EscapeDistance = Objective.EscapeDestinationOffset.Size2D();
				const bool bTightDungeonHallChase =
					Chapter.ChapterId == FName(TEXT("Chapter.DungeonHall"))
					&& Objective.ObjectiveId == FName(TEXT("Hall.SurviveEchoPulse"));
				ValidateCampaignAuthoredLocation(
					Objective.EscapeDestinationOffset,
					Issues,
					Chapter.ChapterId,
					Objective.ObjectiveId,
					TEXT("Campaign.Objective.EscapeDestinationOutOfBounds"),
					NSLOCTEXT("HorrorCampaignValidation", "EscapeDestinationOutOfBounds", "追逐逃离点超出可玩区域或高度带。"));
				if (bTightDungeonHallChase)
				{
					if (EscapeDistance < 650.0f || EscapeDistance > 1500.0f)
					{
						AddValidationIssue(
							Issues,
							Chapter.ChapterId,
							Objective.ObjectiveId,
							TEXT("Campaign.Objective.PursuitDungeonHallDistanceInvalid"),
							NSLOCTEXT("HorrorCampaignValidation", "PursuitDungeonHallDistanceInvalid", "机关大厅追逐逃离点必须保持短距离并靠近安全路线。"));
					}
				}
				else if (EscapeDistance < 2200.0f || EscapeDistance > 3600.0f)
				{
					AddValidationIssue(
						Issues,
						Chapter.ChapterId,
						Objective.ObjectiveId,
						TEXT("Campaign.Objective.PursuitDistanceInvalid"),
						NSLOCTEXT("HorrorCampaignValidation", "PursuitDistanceInvalid", "追逐目标逃离点距离不符合当前地图尺度。"));
				}
			}

			if (IsCarryReturnRelicObjective(Objective))
			{
				ValidateCampaignAuthoredLocation(
					Objective.EscapeDestinationOffset,
					Issues,
					Chapter.ChapterId,
					Objective.ObjectiveId,
					TEXT("Campaign.Objective.RelicDeliveryAnchorOutOfBounds"),
					NSLOCTEXT("HorrorCampaignValidation", "RelicDeliveryAnchorOutOfBounds", "遗物交付锚点超出可玩区域或高度带。"));
			}

			if (Objective.ObjectiveType == EHorrorCampaignObjectiveType::PlantBeacon
				&& Objective.InteractionMode != EHorrorCampaignInteractionMode::SignalTuning)
			{
				AddValidationIssue(
					Issues,
					Chapter.ChapterId,
					Objective.ObjectiveId,
					TEXT("Campaign.Objective.BeaconMissingTuningWindow"),
					NSLOCTEXT("HorrorCampaignValidation", "BeaconMissingTuningWindow", "信标目标必须使用锚定调谐窗口，不能退回普通多段互动。"));
			}

			if (Objective.ObjectiveType == EHorrorCampaignObjectiveType::BossWeakPoint
				&& Objective.InteractionMode != EHorrorCampaignInteractionMode::SpectralScan)
			{
				AddValidationIssue(
					Issues,
					Chapter.ChapterId,
					Objective.ObjectiveId,
					TEXT("Campaign.Objective.WeakPointMissingSpectralWindow"),
					NSLOCTEXT("HorrorCampaignValidation", "WeakPointMissingSpectralWindow", "首领弱点必须使用共鸣频谱窗口，不能退回普通多段互动。"));
			}

			if (IsAdvancedCampaignInteractionMode(Objective.InteractionMode))
			{
				if (!Objective.Presentation.bOpensInteractionPanel)
				{
					AddValidationIssue(
						Issues,
						Chapter.ChapterId,
						Objective.ObjectiveId,
						TEXT("Campaign.Objective.AdvancedMissingPanel"),
						NSLOCTEXT("HorrorCampaignValidation", "AdvancedMissingPanel", "高级交互目标没有声明会打开交互窗口。"));
				}
				const bool bHasAdvancedBeat = Objective.ObjectiveBeats.ContainsByPredicate(
					[](const FHorrorCampaignObjectiveBeat& Beat)
					{
						return Beat.CompletionRule == EHorrorCampaignObjectiveBeatCompletionRule::AdvancedWindow
							&& Beat.bOpensInteractionPanel;
					});
				if (!bHasAdvancedBeat)
				{
					AddValidationIssue(
						Issues,
						Chapter.ChapterId,
						Objective.ObjectiveId,
						TEXT("Campaign.Objective.AdvancedMissingBeat"),
						NSLOCTEXT("HorrorCampaignValidation", "AdvancedMissingBeat", "高级交互目标缺少窗口型任务阶段。"));
				}
			}
		}

		if (bValidateChapterMix)
		{
			TSet<EHorrorCampaignInteractionMode> InteractionModes;
			int32 AdvancedPanelObjectiveCount = 0;
			int32 RequiredAdvancedPanelObjectiveCount = 0;
			int32 OptionalInvestigationCount = 0;
			int32 PressureObjectiveCount = 0;
			for (const FHorrorCampaignObjectiveDefinition& Objective : Chapter.Objectives)
			{
				InteractionModes.Add(Objective.InteractionMode);
				const bool bAdvancedPanelObjective = IsAdvancedCampaignInteractionMode(Objective.InteractionMode);
				AdvancedPanelObjectiveCount += bAdvancedPanelObjective ? 1 : 0;
				RequiredAdvancedPanelObjectiveCount += bAdvancedPanelObjective && Objective.bRequiredForChapterCompletion ? 1 : 0;
				OptionalInvestigationCount += (Objective.bOptional || !Objective.bRequiredForChapterCompletion) ? 1 : 0;
				PressureObjectiveCount += (Objective.ObjectiveType == EHorrorCampaignObjectiveType::SurviveAmbush
					|| Objective.ObjectiveType == EHorrorCampaignObjectiveType::BossWeakPoint
					|| Objective.InteractionMode == EHorrorCampaignInteractionMode::TimedPursuit)
					? 1
					: 0;
			}

			if (InteractionModes.Num() < 3)
			{
				AddValidationIssue(
					Issues,
					Chapter.ChapterId,
					NAME_None,
					TEXT("Campaign.Chapter.LowGameplayVariety"),
					NSLOCTEXT("HorrorCampaignValidation", "LowGameplayVariety", "章节玩法模式过少，容易退化成重复任务。"));
			}
			if (AdvancedPanelObjectiveCount < 2 || RequiredAdvancedPanelObjectiveCount < 1)
			{
				AddValidationIssue(
					Issues,
					Chapter.ChapterId,
					NAME_None,
					TEXT("Campaign.Chapter.MissingAdvancedPanelRoute"),
					NSLOCTEXT("HorrorCampaignValidation", "MissingAdvancedPanelRoute", "章节缺少足够的窗口化主线玩法。"));
			}
			if (OptionalInvestigationCount < 1)
			{
				AddValidationIssue(
					Issues,
					Chapter.ChapterId,
					NAME_None,
					TEXT("Campaign.Chapter.MissingOptionalInvestigation"),
					NSLOCTEXT("HorrorCampaignValidation", "MissingOptionalInvestigation", "章节缺少可选调查目标。"));
			}
			if ((Chapter.bRequiresBoss || Chapter.bIsFinalChapter) && PressureObjectiveCount < 1)
			{
				AddValidationIssue(
					Issues,
					Chapter.ChapterId,
					NAME_None,
					TEXT("Campaign.Chapter.MissingPressureGameplay"),
					NSLOCTEXT("HorrorCampaignValidation", "MissingPressureGameplay", "首领或终章缺少高压玩法节点。"));
			}
		}

		TSet<FName> Visiting;
		TSet<FName> Visited;
		for (const FHorrorCampaignObjectiveDefinition& Objective : Chapter.Objectives)
		{
			if (!Objective.ObjectiveId.IsNone()
				&& HasDependencyCycleVisit(Chapter, Objective.ObjectiveId, Visiting, Visited))
			{
				AddValidationIssue(
					Issues,
					Chapter.ChapterId,
					Objective.ObjectiveId,
					TEXT("Campaign.Objective.DependencyCycle"),
					NSLOCTEXT("HorrorCampaignValidation", "DependencyCycle", "目标依赖存在循环。"));
			}
		}
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

TArray<FHorrorCampaignValidationIssue> FHorrorCampaign::ValidateCampaignChapters()
{
	TArray<FHorrorCampaignValidationIssue> Issues;
	for (const FHorrorCampaignChapterDefinition& Chapter : GetChapters())
	{
		ValidateCampaignChapterDefinition(Chapter, Issues, true);
	}
	return Issues;
}

#if WITH_DEV_AUTOMATION_TESTS
TArray<FHorrorCampaignValidationIssue> FHorrorCampaign::ValidateChapterDefinitionForTests(const FHorrorCampaignChapterDefinition& Chapter)
{
	TArray<FHorrorCampaignValidationIssue> Issues;
	ValidateCampaignChapterDefinition(Chapter, Issues, false);
	return Issues;
}
#endif

void FHorrorCampaignProgress::ResetForChapter(const FHorrorCampaignChapterDefinition& Chapter)
{
	ActiveChapter = &Chapter;
	CompletedObjectiveIds.Reset();
	bBossDefeated = !Chapter.bRequiresBoss;
}

bool FHorrorCampaignProgress::CanCompleteObjective(FName ObjectiveId) const
{
	if (!ActiveChapter || ObjectiveId.IsNone() || CompletedObjectiveIds.Contains(ObjectiveId))
	{
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* Objective = FHorrorCampaign::FindObjectiveById(*ActiveChapter, ObjectiveId);
	return Objective && AreObjectivePrerequisitesComplete(*Objective);
}

bool FHorrorCampaignProgress::TryCompleteObjective(FName ObjectiveId)
{
	if (!CanCompleteObjective(ObjectiveId) || !ActiveChapter)
	{
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* NextObjective = FHorrorCampaign::FindObjectiveById(*ActiveChapter, ObjectiveId);
	if (!NextObjective)
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
		const FName ChapterId = GetActiveChapterId();
		if (!ChapterId.IsNone())
		{
			const FHorrorCampaignChapterDefinition* FallbackChapter = FHorrorCampaign::FindChapterById(ChapterId);
			if (FallbackChapter)
			{
				const_cast<FHorrorCampaignProgress*>(this)->ActiveChapter = FallbackChapter;
			}
		}

		if (!ActiveChapter)
		{
			return nullptr;
		}
	}

	for (const FHorrorCampaignObjectiveDefinition& Objective : ActiveChapter->Objectives)
	{
		if (Objective.bRequiredForChapterCompletion
			&& !CompletedObjectiveIds.Contains(Objective.ObjectiveId)
			&& AreObjectivePrerequisitesComplete(Objective))
		{
			return &Objective;
		}
	}

	return nullptr;
}

TArray<const FHorrorCampaignObjectiveDefinition*> FHorrorCampaignProgress::GetAvailableObjectives() const
{
	TArray<const FHorrorCampaignObjectiveDefinition*> AvailableObjectives;
	if (!ActiveChapter)
	{
		return AvailableObjectives;
	}

	for (const FHorrorCampaignObjectiveDefinition& Objective : ActiveChapter->Objectives)
	{
		if (!CompletedObjectiveIds.Contains(Objective.ObjectiveId)
			&& AreObjectivePrerequisitesComplete(Objective))
		{
			AvailableObjectives.Add(&Objective);
		}
	}

	AvailableObjectives.StableSort(
		[](const FHorrorCampaignObjectiveDefinition& Left, const FHorrorCampaignObjectiveDefinition& Right)
		{
			if (Left.bRequiredForChapterCompletion != Right.bRequiredForChapterCompletion)
			{
				return Left.bRequiredForChapterCompletion;
			}
			return Left.ObjectiveId.LexicalLess(Right.ObjectiveId);
		});

	return AvailableObjectives;
}

TArray<FHorrorCampaignObjectiveGraphNode> FHorrorCampaignProgress::BuildObjectiveGraph() const
{
	TArray<FHorrorCampaignObjectiveGraphNode> GraphNodes;
	if (!ActiveChapter)
	{
		return GraphNodes;
	}

	GraphNodes.Reserve(ActiveChapter->Objectives.Num());
	for (int32 ObjectiveIndex = 0; ObjectiveIndex < ActiveChapter->Objectives.Num(); ++ObjectiveIndex)
	{
		const FHorrorCampaignObjectiveDefinition& Objective = ActiveChapter->Objectives[ObjectiveIndex];
		FHorrorCampaignObjectiveGraphNode Node;
		Node.ChapterId = ActiveChapter->ChapterId;
		Node.ObjectiveId = Objective.ObjectiveId;
		Node.ChapterOrder = ObjectiveIndex;
		Node.bRequired = Objective.bRequiredForChapterCompletion;
		Node.bOptional = Objective.bOptional || !Objective.bRequiredForChapterCompletion;
		Node.bMainline = !Node.bOptional;
		Node.MissingPrerequisiteObjectiveIds = GetMissingPrerequisites(Objective);

		if (CompletedObjectiveIds.Contains(Objective.ObjectiveId))
		{
			Node.Status = EHorrorCampaignObjectiveGraphStatus::Completed;
		}
		else if (Node.MissingPrerequisiteObjectiveIds.IsEmpty())
		{
			Node.Status = EHorrorCampaignObjectiveGraphStatus::Available;
		}
		else
		{
			Node.Status = EHorrorCampaignObjectiveGraphStatus::Locked;
		}

		GraphNodes.Add(Node);
	}

	return GraphNodes;
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
	if (!ActiveChapter)
	{
		return false;
	}

	for (const FHorrorCampaignObjectiveDefinition& Objective : ActiveChapter->Objectives)
	{
		if (Objective.bRequiredForChapterCompletion && !CompletedObjectiveIds.Contains(Objective.ObjectiveId))
		{
			return false;
		}
	}

	return true;
}

bool FHorrorCampaignProgress::AreObjectivePrerequisitesComplete(const FHorrorCampaignObjectiveDefinition& Objective) const
{
	for (const FName PrerequisiteObjectiveId : Objective.PrerequisiteObjectiveIds)
	{
		if (!PrerequisiteObjectiveId.IsNone() && !CompletedObjectiveIds.Contains(PrerequisiteObjectiveId))
		{
			return false;
		}
	}

	return true;
}

TArray<FName> FHorrorCampaignProgress::GetMissingPrerequisites(const FHorrorCampaignObjectiveDefinition& Objective) const
{
	TArray<FName> MissingPrerequisiteIds;
	for (const FName PrerequisiteObjectiveId : Objective.PrerequisiteObjectiveIds)
	{
		if (!PrerequisiteObjectiveId.IsNone() && !CompletedObjectiveIds.Contains(PrerequisiteObjectiveId))
		{
			MissingPrerequisiteIds.Add(PrerequisiteObjectiveId);
		}
	}
	return MissingPrerequisiteIds;
}

FHorrorCampaignSaveState FHorrorCampaignProgress::ExportSaveState() const
{
	FHorrorCampaignSaveState SaveState;
	SaveState.ChapterId = GetActiveChapterId();
	SaveState.CompletedObjectiveIds = CompletedObjectiveIds.Array();
	SaveState.CompletedObjectiveIds.Sort(FNameLexicalLess());
	SaveState.bBossDefeated = bBossDefeated;
	return SaveState;
}

void FHorrorCampaignProgress::ImportSaveState(const FHorrorCampaignSaveState& SaveState)
{
	ActiveChapter = FHorrorCampaign::FindChapterById(SaveState.ChapterId);
	CompletedObjectiveIds.Reset();
	bBossDefeated = ActiveChapter ? !ActiveChapter->bRequiresBoss : false;
	if (!ActiveChapter)
	{
		return;
	}

	TSet<FName> PendingCompletedObjectiveIds;
	for (const FName CompletedObjectiveId : SaveState.CompletedObjectiveIds)
	{
		if (FHorrorCampaign::FindObjectiveById(*ActiveChapter, CompletedObjectiveId))
		{
			PendingCompletedObjectiveIds.Add(CompletedObjectiveId);
		}
	}

	bool bImportedObjectiveThisPass = true;
	while (bImportedObjectiveThisPass)
	{
		bImportedObjectiveThisPass = false;

		for (auto It = PendingCompletedObjectiveIds.CreateIterator(); It; ++It)
		{
			const FHorrorCampaignObjectiveDefinition* Objective = FHorrorCampaign::FindObjectiveById(*ActiveChapter, *It);
			if (Objective && AreObjectivePrerequisitesComplete(*Objective))
			{
				CompletedObjectiveIds.Add(*It);
				It.RemoveCurrent();
				bImportedObjectiveThisPass = true;
			}
		}
	}

	if (ActiveChapter->bRequiresBoss)
	{
		const int32 RequiredWeakPointCount = FHorrorCampaign::CountObjectivesOfType(*ActiveChapter, EHorrorCampaignObjectiveType::BossWeakPoint);
		bBossDefeated = SaveState.bBossDefeated
			&& RequiredWeakPointCount > 0
			&& CountCompletedObjectivesOfType(EHorrorCampaignObjectiveType::BossWeakPoint) >= RequiredWeakPointCount;
	}
	else
	{
		bBossDefeated = true;
	}
}
