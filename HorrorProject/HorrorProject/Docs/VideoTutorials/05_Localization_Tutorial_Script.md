# Video Tutorial Script: Localization System

**Duration:** 8-10 minutes  
**Target Audience:** Developers implementing multi-language support

---

## Introduction (0:00 - 0:30)

"Welcome to the HorrorProject Localization System tutorial. Learn how to add multi-language support to your game with runtime language switching for English, Chinese, Japanese, Korean, and Spanish."

**Show:** Language selection menu with flags

---

## Part 1: Getting Started (0:30 - 1:30)

"First, get the Localization Subsystem."

**Show:** Code editor

```cpp
ULocalizationSubsystem* Localization = 
    GetGameInstance()->GetSubsystem<ULocalizationSubsystem>();
```

"Change language at runtime:"

```cpp
Localization->SetLanguage(ELanguage::Chinese);
Localization->SetLanguage(ELanguage::Japanese);
Localization->SetLanguage(ELanguage::Korean);
```

**Show:** Game UI changing languages in real-time

---

## Part 2: Text Localization (1:30 - 3:30)

"Retrieve localized text using keys."

**Show:** Text key convention

```
Category.Subcategory.Identifier

Examples:
UI.MainMenu.Start
Game.Tutorial.Movement
Dialog.NPC.Greeting
```

**Show:** Code example

```cpp
FText LocalizedText = Localization->GetLocalizedText(TEXT("UI.MainMenu.Start"));
MyTextBlock->SetText(LocalizedText);
```

**Show:** Text changing in different languages

"Use format strings for dynamic content:"

```cpp
// Text: "Player {0} scored {1} points"
TArray<FString> Args = {TEXT("Alice"), TEXT("100")};
FText Formatted = UTextLocalizationLibrary::FormatLocalizedText(
    TEXT("Game.Score.Message"), Args);
```

**Show:** Formatted text in multiple languages

---

## Part 3: Auto-Updating UI (3:30 - 5:00)

"Use UILocalizationComponent for automatic updates."

**Show:** Widget blueprint

```cpp
UILocalization = CreateDefaultSubobject<UUILocalizationComponent>(TEXT("UILocalization"));

UILocalization->RegisterTextWidget(StartButtonText, TEXT("UI.MainMenu.Start"));
UILocalization->RegisterTextWidget(OptionsButtonText, TEXT("UI.MainMenu.Options"));
```

**Show:** Changing language, all UI updates automatically

"Listen for language changes:"

```cpp
Localization->OnLanguageChanged.AddDynamic(this, &UMyClass::OnLanguageChanged);

void UMyClass::OnLanguageChanged(ELanguage NewLanguage)
{
    // Update custom UI elements
}
```

---

## Part 4: CJK Font Support (5:00 - 6:30)

"Chinese, Japanese, and Korean require special fonts."

**Show:** Font comparison

"Use fonts with full CJK character support:
- Noto Sans CJK
- Source Han Sans
- Custom game fonts"

**Show:** Font configuration

```cpp
FString FontPath = UUILocalizationLibrary::GetFontPathForLanguage(CurrentLanguage);
float TextScale = UUILocalizationLibrary::GetTextScaleForLanguage(CurrentLanguage);
```

**Show:** CJK text rendering correctly

---

## Part 5: Audio Localization (6:30 - 7:30)

"Localize voice-over and dialogue."

**Show:** Audio file structure

```
Content/Audio/Localization/
├── en/Dialog_NPC_Greeting.wav
├── zh-Hans/Dialog_NPC_Greeting.wav
├── ja/Dialog_NPC_Greeting.wav
└── ko/Dialog_NPC_Greeting.wav
```

**Show:** Code example

```cpp
UAudioLocalizationLibrary::PlayLocalizedAudio2D(
    this, TEXT("Dialog.NPC.Greeting"));
```

**Show:** Playing audio in different languages

---

## Part 6: Adding New Translations (7:30 - 8:30)

"Add new text to the localization system."

**Show:** MasterLocalizationTable.csv

"1. Add text key to code
2. Run ExtractLocalizableText.ps1
3. Add translations to CSV
4. Run GenerateLocalizationFiles.ps1
5. Run ValidateTranslations.ps1"

**Show:** PowerShell scripts running

**Show:** Validation report

---

## Part 7: Testing (8:30 - 9:30)

"Test all languages thoroughly."

**Show:** Testing checklist

"✓ Switch languages during gameplay
✓ Check UI layout for text overflow
✓ Verify CJK fonts render correctly
✓ Test format strings
✓ Validate audio sync"

**Show:** Switching between all languages rapidly

**Show:** UI adapting to different text lengths

---

## Conclusion (9:30 - 10:00)

"You now know how to:
- Set up localization
- Retrieve localized text
- Auto-update UI
- Support CJK languages
- Localize audio
- Add new translations"

**Show:** Game running in all 5 languages

"Multi-language support expands your audience. Check the documentation for advanced features. Thanks for watching!"

---

## Visual Notes

- Show all 5 languages prominently
- Use split screen for language comparisons
- Highlight CJK character rendering
- Show text length variations
- Include language flags/icons
- Use smooth language transitions

## B-Roll Suggestions

- Language selection menu
- Text changing in real-time
- CJK font rendering
- Audio playback in different languages
- CSV file editing
- Validation reports
