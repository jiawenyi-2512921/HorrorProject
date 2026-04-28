#include "AchievementDefinition.h"

namespace
{
	namespace AchievementProgress
	{
		constexpr float SingleStep = 1.0f;
		constexpr float EvidenceSet = 10.0f;
		constexpr float ExplorationSet = 10.0f;
		constexpr float DocumentSet = 15.0f;
		constexpr float FullEvidenceSet = 25.0f;
		constexpr float FullRoomSet = 30.0f;
		constexpr float SurvivalEncounterSet = 5.0f;
	}

	namespace AchievementPoints
	{
		constexpr int32 Intro = 10;
		constexpr int32 StoryBeat = 15;
		constexpr int32 Standard = 20;
		constexpr int32 Advanced = 25;
		constexpr int32 Notable = 30;
		constexpr int32 Secret = 40;
		constexpr int32 Major = 50;
		constexpr int32 Challenge = 75;
		constexpr int32 Master = 100;
		constexpr int32 Completionist = 200;
	}

	struct FAchievementDefinitionSpec
	{
		const TCHAR* Id;
		const TCHAR* Name;
		const TCHAR* Description;
		EAchievementCategory Category;
		int32 Points;
		float MaxProgress;
		bool bHidden;
		bool bProgressive;
	};

	FHorrorAchievementDefinition MakeAchievementDefinition(const FAchievementDefinitionSpec& Spec)
	{
		FHorrorAchievementDefinition Definition;
		Definition.ID = FName(Spec.Id);
		Definition.Name = FText::FromString(Spec.Name);
		Definition.Description = FText::FromString(Spec.Description);
		Definition.Category = Spec.Category;
		Definition.Points = Spec.Points;
		Definition.MaxProgress = Spec.MaxProgress;
		Definition.bIsHidden = Spec.bHidden;
		Definition.bIsProgressive = Spec.bProgressive;
		return Definition;
	}

	const FAchievementDefinitionSpec AchievementSpecs[] =
	{
		{ TEXT("ACH_FirstSteps"), TEXT("第一步"), TEXT("第一次进入深水站"), EAchievementCategory::Exploration, AchievementPoints::Intro, AchievementProgress::SingleStep, false, false },
		{ TEXT("ACH_Explorer"), TEXT("探索者"), TEXT("发现 10 个不同房间"), EAchievementCategory::Exploration, AchievementPoints::Standard, AchievementProgress::ExplorationSet, false, true },
		{ TEXT("ACH_MasterExplorer"), TEXT("站点制图员"), TEXT("发现站内所有房间"), EAchievementCategory::Exploration, AchievementPoints::Major, AchievementProgress::FullRoomSet, false, true },
		{ TEXT("ACH_SecretPassage"), TEXT("隐蔽通道"), TEXT("找到一条隐藏通路"), EAchievementCategory::Exploration, AchievementPoints::Notable, AchievementProgress::SingleStep, true, false },

		{ TEXT("ACH_FirstClue"), TEXT("第一条线索"), TEXT("收集第一份证据"), EAchievementCategory::Collection, AchievementPoints::Intro, AchievementProgress::SingleStep, false, false },
		{ TEXT("ACH_Detective"), TEXT("调查员"), TEXT("收集 10 份证据"), EAchievementCategory::Collection, AchievementPoints::Standard, AchievementProgress::EvidenceSet, false, true },
		{ TEXT("ACH_MasterDetective"), TEXT("首席调查员"), TEXT("收集游戏中的全部证据"), EAchievementCategory::Collection, AchievementPoints::Major, AchievementProgress::FullEvidenceSet, false, true },
		{ TEXT("ACH_Archivist"), TEXT("档案管理员"), TEXT("阅读所有文件和笔记"), EAchievementCategory::Collection, AchievementPoints::Notable, AchievementProgress::DocumentSet, false, true },

		{ TEXT("ACH_FirstEncounter"), TEXT("第一次遭遇"), TEXT("从第一次异常遭遇中幸存"), EAchievementCategory::Survival, AchievementPoints::StoryBeat, AchievementProgress::SingleStep, false, false },
		{ TEXT("ACH_Survivor"), TEXT("幸存者"), TEXT("从 5 次异常遭遇中幸存"), EAchievementCategory::Survival, AchievementPoints::Advanced, AchievementProgress::SurvivalEncounterSet, false, true },
		{ TEXT("ACH_Fearless"), TEXT("无惧"), TEXT("不死亡完成游戏"), EAchievementCategory::Survival, AchievementPoints::Master, AchievementProgress::SingleStep, false, false },
		{ TEXT("ACH_NervesOfSteel"), TEXT("钢铁神经"), TEXT("全程保持理智值高于 50%"), EAchievementCategory::Survival, AchievementPoints::Major, AchievementProgress::SingleStep, false, false },

		{ TEXT("ACH_SpeedRunner"), TEXT("快速通关"), TEXT("2 小时内完成游戏"), EAchievementCategory::Speedrun, AchievementPoints::Major, AchievementProgress::SingleStep, false, false },
		{ TEXT("ACH_LightningFast"), TEXT("闪电行动"), TEXT("1 小时内完成游戏"), EAchievementCategory::Speedrun, AchievementPoints::Master, AchievementProgress::SingleStep, false, false },

		{ TEXT("ACH_TheBeginning"), TEXT("开端"), TEXT("完成第 1 章"), EAchievementCategory::Story, AchievementPoints::StoryBeat, AchievementProgress::SingleStep, false, false },
		{ TEXT("ACH_DeeperIntoMadness"), TEXT("更深处"), TEXT("完成第 2 章"), EAchievementCategory::Story, AchievementPoints::Standard, AchievementProgress::SingleStep, false, false },
		{ TEXT("ACH_TheTruth"), TEXT("真相"), TEXT("揭开深水站的黑暗秘密"), EAchievementCategory::Story, AchievementPoints::Notable, AchievementProgress::SingleStep, false, false },
		{ TEXT("ACH_Escape"), TEXT("逃离"), TEXT("完成游戏"), EAchievementCategory::Story, AchievementPoints::Major, AchievementProgress::SingleStep, false, false },

		{ TEXT("ACH_TheWatcher"), TEXT("观察者"), TEXT("???"), EAchievementCategory::Secret, AchievementPoints::Major, AchievementProgress::SingleStep, true, false },
		{ TEXT("ACH_BrokenMirror"), TEXT("破碎之镜"), TEXT("???"), EAchievementCategory::Secret, AchievementPoints::Notable, AchievementProgress::SingleStep, true, false },
		{ TEXT("ACH_MidnightVisitor"), TEXT("午夜访客"), TEXT("???"), EAchievementCategory::Secret, AchievementPoints::Secret, AchievementProgress::SingleStep, true, false },

		{ TEXT("ACH_NoFlashlight"), TEXT("拥抱黑暗"), TEXT("不使用手电筒完成一个章节"), EAchievementCategory::Challenge, AchievementPoints::Challenge, AchievementProgress::SingleStep, false, false },
		{ TEXT("ACH_Minimalist"), TEXT("极简主义"), TEXT("只使用必要物品完成游戏"), EAchievementCategory::Challenge, AchievementPoints::Master, AchievementProgress::SingleStep, false, false },
		{ TEXT("ACH_Perfectionist"), TEXT("完美主义者"), TEXT("解锁所有其他成就"), EAchievementCategory::Challenge, AchievementPoints::Completionist, AchievementProgress::SingleStep, false, false }
	};
}

TArray<FHorrorAchievementDefinition> UAchievementDefinitionCatalog::GetAllAchievementDefinitions()
{
	TArray<FHorrorAchievementDefinition> Definitions;
	Definitions.Reserve(UE_ARRAY_COUNT(AchievementSpecs));

	for (const FAchievementDefinitionSpec& Spec : AchievementSpecs)
	{
		Definitions.Add(MakeAchievementDefinition(Spec));
	}

	return Definitions;
}
