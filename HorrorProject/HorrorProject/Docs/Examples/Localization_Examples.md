# Code Examples: Localization System

Complete code examples for implementing multi-language support.

---

## Example 1: Basic Localization

```cpp
// LocalizationManager.h
#pragma once
#include "CoreMinimal.h"
#include "Localization/LocalizationSubsystem.h"
#include "LocalizationManager.generated.h"

UCLASS()
class HORRORPROJECT_API ULocalizationManager : public UObject
{
    GENERATED_BODY()

public:
    static FText GetText(UWorld* World, const FString& Key);
    static void ChangeLanguage(UWorld* World, ELanguage NewLanguage);
    static ELanguage GetCurrentLanguage(UWorld* World);
};

// LocalizationManager.cpp
#include "LocalizationManager.h"

FText ULocalizationManager::GetText(UWorld* World, const FString& Key)
{
    ULocalizationSubsystem* Localization = 
        World->GetGameInstance()->GetSubsystem<ULocalizationSubsystem>();
    
    if (Localization)
    {
        return Localization->GetLocalizedText(Key);
    }
    
    return FText::FromString(Key);
}

void ULocalizationManager::ChangeLanguage(UWorld* World, ELanguage NewLanguage)
{
    ULocalizationSubsystem* Localization = 
        World->GetGameInstance()->GetSubsystem<ULocalizationSubsystem>();
    
    if (Localization)
    {
        Localization->SetLanguage(NewLanguage);
        UE_LOG(LogTemp, Log, TEXT("Language changed to: %d"), (int32)NewLanguage);
    }
}

ELanguage ULocalizationManager::GetCurrentLanguage(UWorld* World)
{
    ULocalizationSubsystem* Localization = 
        World->GetGameInstance()->GetSubsystem<ULocalizationSubsystem>();
    
    if (Localization)
    {
        return Localization->GetCurrentLanguage();
    }
    
    return ELanguage::English;
}
```

---

## Example 2: Localized UI Widget

```cpp
// LocalizedWidget.h
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Localization/UILocalizationComponent.h"
#include "Components/TextBlock.h"
#include "LocalizedWidget.generated.h"

UCLASS()
class HORRORPROJECT_API ULocalizedWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TitleText;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* DescriptionText;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Localization")
    UUILocalizationComponent* UILocalization;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
    FString TitleKey;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
    FString DescriptionKey;
};

// LocalizedWidget.cpp
#include "LocalizedWidget.h"

void ULocalizedWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Create localization component
    UILocalization = NewObject<UUILocalizationComponent>(this);
    
    if (UILocalization)
    {
        // Register text widgets with their keys
        if (TitleText && !TitleKey.IsEmpty())
        {
            UILocalization->RegisterTextWidget(TitleText, TitleKey);
        }
        
        if (DescriptionText && !DescriptionKey.IsEmpty())
        {
            UILocalization->RegisterTextWidget(DescriptionText, DescriptionKey);
        }
    }
}
```

---

## Example 3: Format Strings

```cpp
// LocalizedMessages.h
#pragma once
#include "CoreMinimal.h"
#include "Localization/TextLocalizationLibrary.h"
#include "LocalizedMessages.generated.h"

UCLASS()
class HORRORPROJECT_API ULocalizedMessages : public UObject
{
    GENERATED_BODY()

public:
    static FText GetPlayerScoreMessage(const FString& PlayerName, int32 Score);
    static FText GetItemCollectedMessage(const FString& ItemName, int32 Count);
    static FText GetTimeRemainingMessage(int32 Minutes, int32 Seconds);
};

// LocalizedMessages.cpp
#include "LocalizedMessages.h"

FText ULocalizedMessages::GetPlayerScoreMessage(const FString& PlayerName, int32 Score)
{
    // Key: "Game.Score.Message" = "Player {0} scored {1} points"
    TArray<FString> Args;
    Args.Add(PlayerName);
    Args.Add(FString::FromInt(Score));
    
    return UTextLocalizationLibrary::FormatLocalizedText(TEXT("Game.Score.Message"), Args);
}

FText ULocalizedMessages::GetItemCollectedMessage(const FString& ItemName, int32 Count)
{
    // Key: "Game.Item.Collected" = "Collected {0} x{1}"
    TArray<FString> Args;
    Args.Add(ItemName);
    Args.Add(FString::FromInt(Count));
    
    return UTextLocalizationLibrary::FormatLocalizedText(TEXT("Game.Item.Collected"), Args);
}

FText ULocalizedMessages::GetTimeRemainingMessage(int32 Minutes, int32 Seconds)
{
    // Key: "Game.Time.Remaining" = "Time remaining: {0}:{1}"
    TArray<FString> Args;
    Args.Add(FString::Printf(TEXT("%02d"), Minutes));
    Args.Add(FString::Printf(TEXT("%02d"), Seconds));
    
    return UTextLocalizationLibrary::FormatLocalizedText(TEXT("Game.Time.Remaining"), Args);
}
```

---

## Example 4: Plural Forms

```cpp
// PluralTextHelper.h
#pragma once
#include "CoreMinimal.h"
#include "Localization/TextLocalizationLibrary.h"
#include "PluralTextHelper.generated.h"

UCLASS()
class HORRORPROJECT_API UPluralTextHelper : public UObject
{
    GENERATED_BODY()

public:
    static FText GetItemCountText(int32 Count);
    static FText GetPlayerCountText(int32 Count);
    static FText GetEnemyCountText(int32 Count);
};

// PluralTextHelper.cpp
#include "PluralTextHelper.h"

FText UPluralTextHelper::GetItemCountText(int32 Count)
{
    // Keys:
    // "Items.Singular" = "You have {0} item"
    // "Items.Plural" = "You have {0} items"
    
    FString Key = (Count == 1) ? TEXT("Items.Singular") : TEXT("Items.Plural");
    TArray<FString> Args;
    Args.Add(FString::FromInt(Count));
    
    return UTextLocalizationLibrary::FormatLocalizedText(Key, Args);
}

FText UPluralTextHelper::GetPlayerCountText(int32 Count)
{
    // Keys:
    // "Players.Singular" = "{0} player online"
    // "Players.Plural" = "{0} players online"
    
    FString Key = (Count == 1) ? TEXT("Players.Singular") : TEXT("Players.Plural");
    TArray<FString> Args;
    Args.Add(FString::FromInt(Count));
    
    return UTextLocalizationLibrary::FormatLocalizedText(Key, Args);
}

FText UPluralTextHelper::GetEnemyCountText(int32 Count)
{
    // Keys:
    // "Enemies.Singular" = "{0} enemy remaining"
    // "Enemies.Plural" = "{0} enemies remaining"
    
    FString Key = (Count == 1) ? TEXT("Enemies.Singular") : TEXT("Enemies.Plural");
    TArray<FString> Args;
    Args.Add(FString::FromInt(Count));
    
    return UTextLocalizationLibrary::FormatLocalizedText(Key, Args);
}
```

---

## Example 5: Audio Localization

```cpp
// LocalizedAudioPlayer.h
#pragma once
#include "CoreMinimal.h"
#include "Localization/AudioLocalizationLibrary.h"
#include "LocalizedAudioPlayer.generated.h"

UCLASS()
class HORRORPROJECT_API ULocalizedAudioPlayer : public UObject
{
    GENERATED_BODY()

public:
    static void PlayLocalizedDialogue(UWorld* World, const FString& DialogueKey);
    static void PlayLocalizedNarration(UWorld* World, const FString& NarrationKey);
    static USoundBase* GetLocalizedSound(UWorld* World, const FString& SoundKey);
};

// LocalizedAudioPlayer.cpp
#include "LocalizedAudioPlayer.h"

void ULocalizedAudioPlayer::PlayLocalizedDialogue(UWorld* World, const FString& DialogueKey)
{
    UAudioLocalizationLibrary::PlayLocalizedAudio2D(World, DialogueKey);
}

void ULocalizedAudioPlayer::PlayLocalizedNarration(UWorld* World, const FString& NarrationKey)
{
    UAudioLocalizationLibrary::PlayLocalizedAudio2D(World, NarrationKey);
}

USoundBase* ULocalizedAudioPlayer::GetLocalizedSound(UWorld* World, const FString& SoundKey)
{
    return UAudioLocalizationLibrary::GetLocalizedSound(World, SoundKey);
}
```

---

## Example 6: Language Selection Menu

```cpp
// LanguageSelectionWidget.h
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Localization/LocalizationSubsystem.h"
#include "Components/ComboBoxString.h"
#include "LanguageSelectionWidget.generated.h"

UCLASS()
class HORRORPROJECT_API ULanguageSelectionWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    
    UPROPERTY(meta = (BindWidget))
    UComboBoxString* LanguageComboBox;
    
    UFUNCTION()
    void OnLanguageSelected(FString SelectedItem, ESelectInfo::Type SelectionType);
    
    UFUNCTION()
    void OnLanguageChanged(ELanguage NewLanguage);

private:
    void PopulateLanguageList();
    ULocalizationSubsystem* LocalizationSubsystem;
};

// LanguageSelectionWidget.cpp
#include "LanguageSelectionWidget.h"

void ULanguageSelectionWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    LocalizationSubsystem = GetGameInstance()->GetSubsystem<ULocalizationSubsystem>();
    
    if (LanguageComboBox)
    {
        LanguageComboBox->OnSelectionChanged.AddDynamic(this, &ULanguageSelectionWidget::OnLanguageSelected);
        PopulateLanguageList();
    }
    
    if (LocalizationSubsystem)
    {
        LocalizationSubsystem->OnLanguageChanged.AddDynamic(this, &ULanguageSelectionWidget::OnLanguageChanged);
    }
}

void ULanguageSelectionWidget::PopulateLanguageList()
{
    if (LanguageComboBox)
    {
        LanguageComboBox->ClearOptions();
        LanguageComboBox->AddOption(TEXT("English"));
        LanguageComboBox->AddOption(TEXT("中文"));
        LanguageComboBox->AddOption(TEXT("日本語"));
        LanguageComboBox->AddOption(TEXT("한국어"));
        LanguageComboBox->AddOption(TEXT("Español"));
        
        // Set current language
        if (LocalizationSubsystem)
        {
            ELanguage CurrentLang = LocalizationSubsystem->GetCurrentLanguage();
            LanguageComboBox->SetSelectedIndex((int32)CurrentLang);
        }
    }
}

void ULanguageSelectionWidget::OnLanguageSelected(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (LocalizationSubsystem && SelectionType != ESelectInfo::Direct)
    {
        int32 Index = LanguageComboBox->GetSelectedIndex();
        ELanguage NewLanguage = (ELanguage)Index;
        LocalizationSubsystem->SetLanguage(NewLanguage);
    }
}

void ULanguageSelectionWidget::OnLanguageChanged(ELanguage NewLanguage)
{
    UE_LOG(LogTemp, Log, TEXT("Language changed to: %d"), (int32)NewLanguage);
    // Update UI if needed
}
```

---

## Example 7: Dynamic Font Selection

```cpp
// FontManager.h
#pragma once
#include "CoreMinimal.h"
#include "Localization/UILocalizationLibrary.h"
#include "Components/TextBlock.h"
#include "FontManager.generated.h"

UCLASS()
class HORRORPROJECT_API UFontManager : public UObject
{
    GENERATED_BODY()

public:
    static void ApplyLanguageFont(UTextBlock* TextBlock, ELanguage Language);
    static void ApplyLanguageScale(UTextBlock* TextBlock, ELanguage Language);
};

// FontManager.cpp
#include "FontManager.h"

void UFontManager::ApplyLanguageFont(UTextBlock* TextBlock, ELanguage Language)
{
    if (!TextBlock)
    {
        return;
    }
    
    FString FontPath = UUILocalizationLibrary::GetFontPathForLanguage(Language);
    
    // Load font asset
    UFont* Font = LoadObject<UFont>(nullptr, *FontPath);
    if (Font)
    {
        FSlateFontInfo FontInfo = TextBlock->GetFont();
        FontInfo.FontObject = Font;
        TextBlock->SetFont(FontInfo);
    }
}

void UFontManager::ApplyLanguageScale(UTextBlock* TextBlock, ELanguage Language)
{
    if (!TextBlock)
    {
        return;
    }
    
    float Scale = UUILocalizationLibrary::GetTextScaleForLanguage(Language);
    
    FSlateFontInfo FontInfo = TextBlock->GetFont();
    FontInfo.Size = FontInfo.Size * Scale;
    TextBlock->SetFont(FontInfo);
}
```

---

## Blueprint Examples

### Change Language
```
On Language Button Clicked
  → Get Localization Subsystem
  → Set Language (Chinese/Japanese/Korean/Spanish)
```

### Get Localized Text
```
Event BeginPlay
  → Get Localization Subsystem
  → Get Localized Text (Key: "UI.MainMenu.Start")
  → Set Text (Text Block)
```

### Format String
```
On Score Updated
  → Make Array (Player Name, Score)
  → Format Localized Text (Key: "Game.Score.Message", Args: Array)
  → Display Text
```

### Auto-Update UI
```
In Widget Blueprint:
  → Add Component: UI Localization Component
  → Register Text Widget (Text Block, Key: "UI.MainMenu.Start")
  → (Automatically updates on language change)
```

### Listen for Language Change
```
Event Construct
  → Get Localization Subsystem
  → Bind Event to OnLanguageChanged
  
On Language Changed
  → Update Custom UI Elements
  → Reload Localized Assets
```
