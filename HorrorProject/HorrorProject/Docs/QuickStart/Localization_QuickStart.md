# Localization - Quick Start Guide

Add multi-language support in 5 minutes.

## Step 1: Get Localization Subsystem (30 sec)

```cpp
ULocalizationSubsystem* Localization = 
    GetGameInstance()->GetSubsystem<ULocalizationSubsystem>();
```

## Step 2: Change Language (1 min)

```cpp
// Set language
Localization->SetLanguage(ELanguage::Chinese);
Localization->SetLanguage(ELanguage::Japanese);
Localization->SetLanguage(ELanguage::Korean);
Localization->SetLanguage(ELanguage::Spanish);

// Get current language
ELanguage CurrentLang = Localization->GetCurrentLanguage();
```

## Step 3: Get Localized Text (1 min)

```cpp
// Get text by key
FText LocalizedText = Localization->GetLocalizedText(TEXT("UI.MainMenu.Start"));

// Use in UI
MyTextBlock->SetText(LocalizedText);
```

## Step 4: Auto-Update UI (2 min)

Add `UILocalizationComponent` to your widget:

```cpp
// In widget constructor
UILocalization = CreateDefaultSubobject<UUILocalizationComponent>(TEXT("UILocalization"));

// Register text widgets
UILocalization->RegisterTextWidget(StartButtonText, TEXT("UI.MainMenu.Start"));
UILocalization->RegisterTextWidget(OptionsButtonText, TEXT("UI.MainMenu.Options"));

// Component auto-updates on language change
```

## Blueprint Quick Start

1. Get Localization Subsystem
2. Set Language (Chinese/Japanese/Korean/Spanish)
3. Get Localized Text (Key: "UI.MainMenu.Start")
4. Set Text to widget

## Format Strings

```cpp
// Text with placeholders: "Player {0} scored {1} points"
TArray<FString> Args = {TEXT("Alice"), TEXT("100")};
FText Formatted = UTextLocalizationLibrary::FormatLocalizedText(
    TEXT("Game.Score.Message"), Args);
```

## Supported Languages

- English (en) - Default
- Chinese Simplified (zh-Hans)
- Japanese (ja)
- Korean (ko)
- Spanish (es)

## Text Key Convention

```
Category.Subcategory.Identifier

Examples:
UI.MainMenu.Start
Game.Tutorial.Movement
Dialog.NPC.Greeting
```

## Testing

```cpp
// Listen for language changes
Localization->OnLanguageChanged.AddDynamic(this, &UMyClass::OnLanguageChanged);

void UMyClass::OnLanguageChanged(ELanguage NewLanguage)
{
    UE_LOG(LogTemp, Log, TEXT("Language changed to: %d"), (int32)NewLanguage);
}
```

## Next Steps

- Add new text keys
- Translate content
- Test CJK fonts
- Localize audio
