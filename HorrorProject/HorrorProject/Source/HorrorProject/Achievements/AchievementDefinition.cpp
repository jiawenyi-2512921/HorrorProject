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
		{ TEXT("ACH_FirstSteps"), TEXT("First Steps"), TEXT("Enter the haunted mansion for the first time"), EAchievementCategory::Exploration, AchievementPoints::Intro, AchievementProgress::SingleStep, false, false },
		{ TEXT("ACH_Explorer"), TEXT("Explorer"), TEXT("Discover 10 different rooms"), EAchievementCategory::Exploration, AchievementPoints::Standard, AchievementProgress::ExplorationSet, false, true },
		{ TEXT("ACH_MasterExplorer"), TEXT("Master Explorer"), TEXT("Discover all rooms in the mansion"), EAchievementCategory::Exploration, AchievementPoints::Major, AchievementProgress::FullRoomSet, false, true },
		{ TEXT("ACH_SecretPassage"), TEXT("Secret Passage"), TEXT("Find a hidden passage"), EAchievementCategory::Exploration, AchievementPoints::Notable, AchievementProgress::SingleStep, true, false },

		{ TEXT("ACH_FirstClue"), TEXT("First Clue"), TEXT("Collect your first evidence"), EAchievementCategory::Collection, AchievementPoints::Intro, AchievementProgress::SingleStep, false, false },
		{ TEXT("ACH_Detective"), TEXT("Detective"), TEXT("Collect 10 pieces of evidence"), EAchievementCategory::Collection, AchievementPoints::Standard, AchievementProgress::EvidenceSet, false, true },
		{ TEXT("ACH_MasterDetective"), TEXT("Master Detective"), TEXT("Collect all evidence in the game"), EAchievementCategory::Collection, AchievementPoints::Major, AchievementProgress::FullEvidenceSet, false, true },
		{ TEXT("ACH_Archivist"), TEXT("Archivist"), TEXT("Read all documents and notes"), EAchievementCategory::Collection, AchievementPoints::Notable, AchievementProgress::DocumentSet, false, true },

		{ TEXT("ACH_FirstEncounter"), TEXT("First Encounter"), TEXT("Survive your first ghost encounter"), EAchievementCategory::Survival, AchievementPoints::StoryBeat, AchievementProgress::SingleStep, false, false },
		{ TEXT("ACH_Survivor"), TEXT("Survivor"), TEXT("Survive 5 ghost encounters"), EAchievementCategory::Survival, AchievementPoints::Advanced, AchievementProgress::SurvivalEncounterSet, false, true },
		{ TEXT("ACH_Fearless"), TEXT("Fearless"), TEXT("Complete the game without dying"), EAchievementCategory::Survival, AchievementPoints::Master, AchievementProgress::SingleStep, false, false },
		{ TEXT("ACH_NervesOfSteel"), TEXT("Nerves of Steel"), TEXT("Keep sanity above 50% throughout the game"), EAchievementCategory::Survival, AchievementPoints::Major, AchievementProgress::SingleStep, false, false },

		{ TEXT("ACH_SpeedRunner"), TEXT("Speed Runner"), TEXT("Complete the game in under 2 hours"), EAchievementCategory::Speedrun, AchievementPoints::Major, AchievementProgress::SingleStep, false, false },
		{ TEXT("ACH_LightningFast"), TEXT("Lightning Fast"), TEXT("Complete the game in under 1 hour"), EAchievementCategory::Speedrun, AchievementPoints::Master, AchievementProgress::SingleStep, false, false },

		{ TEXT("ACH_TheBeginning"), TEXT("The Beginning"), TEXT("Complete Chapter 1"), EAchievementCategory::Story, AchievementPoints::StoryBeat, AchievementProgress::SingleStep, false, false },
		{ TEXT("ACH_DeeperIntoMadness"), TEXT("Deeper Into Madness"), TEXT("Complete Chapter 2"), EAchievementCategory::Story, AchievementPoints::Standard, AchievementProgress::SingleStep, false, false },
		{ TEXT("ACH_TheTruth"), TEXT("The Truth"), TEXT("Uncover the mansion's dark secret"), EAchievementCategory::Story, AchievementPoints::Notable, AchievementProgress::SingleStep, false, false },
		{ TEXT("ACH_Escape"), TEXT("Escape"), TEXT("Complete the game"), EAchievementCategory::Story, AchievementPoints::Major, AchievementProgress::SingleStep, false, false },

		{ TEXT("ACH_TheWatcher"), TEXT("The Watcher"), TEXT("???"), EAchievementCategory::Secret, AchievementPoints::Major, AchievementProgress::SingleStep, true, false },
		{ TEXT("ACH_BrokenMirror"), TEXT("Broken Mirror"), TEXT("???"), EAchievementCategory::Secret, AchievementPoints::Notable, AchievementProgress::SingleStep, true, false },
		{ TEXT("ACH_MidnightVisitor"), TEXT("Midnight Visitor"), TEXT("???"), EAchievementCategory::Secret, AchievementPoints::Secret, AchievementProgress::SingleStep, true, false },

		{ TEXT("ACH_NoFlashlight"), TEXT("Darkness Embraced"), TEXT("Complete a chapter without using flashlight"), EAchievementCategory::Challenge, AchievementPoints::Challenge, AchievementProgress::SingleStep, false, false },
		{ TEXT("ACH_Minimalist"), TEXT("Minimalist"), TEXT("Complete the game using only essential items"), EAchievementCategory::Challenge, AchievementPoints::Master, AchievementProgress::SingleStep, false, false },
		{ TEXT("ACH_Perfectionist"), TEXT("Perfectionist"), TEXT("Unlock all other achievements"), EAchievementCategory::Challenge, AchievementPoints::Completionist, AchievementProgress::SingleStep, false, false }
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
