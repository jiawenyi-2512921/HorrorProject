#include "AchievementDefinition.h"

TArray<FHorrorAchievementDefinition> UAchievementDefinitionCatalog::GetAllAchievementDefinitions()
{
	TArray<FHorrorAchievementDefinition> Definitions;

	// EXPLORATION ACHIEVEMENTS
	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_FirstSteps");
		Def.Name = FText::FromString("First Steps");
		Def.Description = FText::FromString("Enter the haunted mansion for the first time");
		Def.Category = EAchievementCategory::Exploration;
		Def.Points = 10;
		Definitions.Add(Def);
	}

	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_Explorer");
		Def.Name = FText::FromString("Explorer");
		Def.Description = FText::FromString("Discover 10 different rooms");
		Def.Category = EAchievementCategory::Exploration;
		Def.bIsProgressive = true;
		Def.MaxProgress = 10.0f;
		Def.Points = 20;
		Definitions.Add(Def);
	}

	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_MasterExplorer");
		Def.Name = FText::FromString("Master Explorer");
		Def.Description = FText::FromString("Discover all rooms in the mansion");
		Def.Category = EAchievementCategory::Exploration;
		Def.bIsProgressive = true;
		Def.MaxProgress = 30.0f;
		Def.Points = 50;
		Definitions.Add(Def);
	}

	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_SecretPassage");
		Def.Name = FText::FromString("Secret Passage");
		Def.Description = FText::FromString("Find a hidden passage");
		Def.Category = EAchievementCategory::Exploration;
		Def.bIsHidden = true;
		Def.Points = 30;
		Definitions.Add(Def);
	}

	// COLLECTION ACHIEVEMENTS
	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_FirstClue");
		Def.Name = FText::FromString("First Clue");
		Def.Description = FText::FromString("Collect your first evidence");
		Def.Category = EAchievementCategory::Collection;
		Def.Points = 10;
		Definitions.Add(Def);
	}

	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_Detective");
		Def.Name = FText::FromString("Detective");
		Def.Description = FText::FromString("Collect 10 pieces of evidence");
		Def.Category = EAchievementCategory::Collection;
		Def.bIsProgressive = true;
		Def.MaxProgress = 10.0f;
		Def.Points = 20;
		Definitions.Add(Def);
	}

	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_MasterDetective");
		Def.Name = FText::FromString("Master Detective");
		Def.Description = FText::FromString("Collect all evidence in the game");
		Def.Category = EAchievementCategory::Collection;
		Def.bIsProgressive = true;
		Def.MaxProgress = 25.0f;
		Def.Points = 50;
		Definitions.Add(Def);
	}

	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_Archivist");
		Def.Name = FText::FromString("Archivist");
		Def.Description = FText::FromString("Read all documents and notes");
		Def.Category = EAchievementCategory::Collection;
		Def.bIsProgressive = true;
		Def.MaxProgress = 15.0f;
		Def.Points = 30;
		Definitions.Add(Def);
	}

	// SURVIVAL ACHIEVEMENTS
	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_FirstEncounter");
		Def.Name = FText::FromString("First Encounter");
		Def.Description = FText::FromString("Survive your first ghost encounter");
		Def.Category = EAchievementCategory::Survival;
		Def.Points = 15;
		Definitions.Add(Def);
	}

	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_Survivor");
		Def.Name = FText::FromString("Survivor");
		Def.Description = FText::FromString("Survive 5 ghost encounters");
		Def.Category = EAchievementCategory::Survival;
		Def.bIsProgressive = true;
		Def.MaxProgress = 5.0f;
		Def.Points = 25;
		Definitions.Add(Def);
	}

	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_Fearless");
		Def.Name = FText::FromString("Fearless");
		Def.Description = FText::FromString("Complete the game without dying");
		Def.Category = EAchievementCategory::Survival;
		Def.Points = 100;
		Definitions.Add(Def);
	}

	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_NervesOfSteel");
		Def.Name = FText::FromString("Nerves of Steel");
		Def.Description = FText::FromString("Keep sanity above 50% throughout the game");
		Def.Category = EAchievementCategory::Survival;
		Def.Points = 50;
		Definitions.Add(Def);
	}

	// SPEEDRUN ACHIEVEMENTS
	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_SpeedRunner");
		Def.Name = FText::FromString("Speed Runner");
		Def.Description = FText::FromString("Complete the game in under 2 hours");
		Def.Category = EAchievementCategory::Speedrun;
		Def.Points = 50;
		Definitions.Add(Def);
	}

	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_LightningFast");
		Def.Name = FText::FromString("Lightning Fast");
		Def.Description = FText::FromString("Complete the game in under 1 hour");
		Def.Category = EAchievementCategory::Speedrun;
		Def.Points = 100;
		Definitions.Add(Def);
	}

	// STORY ACHIEVEMENTS
	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_TheBeginning");
		Def.Name = FText::FromString("The Beginning");
		Def.Description = FText::FromString("Complete Chapter 1");
		Def.Category = EAchievementCategory::Story;
		Def.Points = 15;
		Definitions.Add(Def);
	}

	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_DeeperIntoMadness");
		Def.Name = FText::FromString("Deeper Into Madness");
		Def.Description = FText::FromString("Complete Chapter 2");
		Def.Category = EAchievementCategory::Story;
		Def.Points = 20;
		Definitions.Add(Def);
	}

	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_TheTruth");
		Def.Name = FText::FromString("The Truth");
		Def.Description = FText::FromString("Uncover the mansion's dark secret");
		Def.Category = EAchievementCategory::Story;
		Def.Points = 30;
		Definitions.Add(Def);
	}

	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_Escape");
		Def.Name = FText::FromString("Escape");
		Def.Description = FText::FromString("Complete the game");
		Def.Category = EAchievementCategory::Story;
		Def.Points = 50;
		Definitions.Add(Def);
	}

	// SECRET ACHIEVEMENTS
	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_TheWatcher");
		Def.Name = FText::FromString("The Watcher");
		Def.Description = FText::FromString("???");
		Def.Category = EAchievementCategory::Secret;
		Def.bIsHidden = true;
		Def.Points = 50;
		Definitions.Add(Def);
	}

	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_BrokenMirror");
		Def.Name = FText::FromString("Broken Mirror");
		Def.Description = FText::FromString("???");
		Def.Category = EAchievementCategory::Secret;
		Def.bIsHidden = true;
		Def.Points = 30;
		Definitions.Add(Def);
	}

	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_MidnightVisitor");
		Def.Name = FText::FromString("Midnight Visitor");
		Def.Description = FText::FromString("???");
		Def.Category = EAchievementCategory::Secret;
		Def.bIsHidden = true;
		Def.Points = 40;
		Definitions.Add(Def);
	}

	// CHALLENGE ACHIEVEMENTS
	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_NoFlashlight");
		Def.Name = FText::FromString("Darkness Embraced");
		Def.Description = FText::FromString("Complete a chapter without using flashlight");
		Def.Category = EAchievementCategory::Challenge;
		Def.Points = 75;
		Definitions.Add(Def);
	}

	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_Minimalist");
		Def.Name = FText::FromString("Minimalist");
		Def.Description = FText::FromString("Complete the game using only essential items");
		Def.Category = EAchievementCategory::Challenge;
		Def.Points = 100;
		Definitions.Add(Def);
	}

	{
		FHorrorAchievementDefinition Def;
		Def.ID = FName("ACH_Perfectionist");
		Def.Name = FText::FromString("Perfectionist");
		Def.Description = FText::FromString("Unlock all other achievements");
		Def.Category = EAchievementCategory::Challenge;
		Def.Points = 200;
		Definitions.Add(Def);
	}

	return Definitions;
}
