# Localization Guide

## Overview

This guide covers the localization system for HorrorProject, supporting multiple languages with runtime switching capabilities.

## Supported Languages

- English (en)
- Chinese Simplified (zh-Hans)
- Japanese (ja)
- Korean (ko)
- Spanish (es)

## Architecture

### Core Components

1. **LocalizationSubsystem** - Main subsystem managing language state
2. **TextLocalization** - Text retrieval and formatting utilities
3. **AudioLocalization** - Localized audio management
4. **UILocalization** - UI widget localization component

### Data Flow

```
Game Code → LocalizationSubsystem → Language Data → Localized Content
```

## Using Localization in Code

### C++ Usage

```cpp
// Get localization subsystem
ULocalizationSubsystem* LocalizationSubsystem = 
    GetGameInstance()->GetSubsystem<ULocalizationSubsystem>();

// Change language
LocalizationSubsystem->SetLanguage(ELanguage::Chinese);

// Get localized text
FText LocalizedText = LocalizationSubsystem->GetLocalizedText(TEXT("UI.MainMenu.Start"));

// Listen for language changes
LocalizationSubsystem->OnLanguageChanged.AddDynamic(this, &UMyClass::OnLanguageChanged);
```

### Blueprint Usage

```
Get Localization Subsystem → Set Language → Chinese
Get Localization Subsystem → Get Localized Text → "UI.MainMenu.Start"
```

### UI Localization Component

Add `UILocalizationComponent` to your widget blueprint:

1. Add component to widget
2. Add text blocks to `LocalizedTextWidgets` array
3. Set localization keys for each widget
4. Component automatically updates on language change

## Text Keys Convention

Use hierarchical naming:

```
Category.Subcategory.Identifier

Examples:
- UI.MainMenu.Start
- UI.MainMenu.Options
- Game.Tutorial.Movement
- Game.Message.ItemFound
- Dialog.NPC.Greeting
```

## Adding New Text

### 1. Add to Source Code

```cpp
FText MyText = LocalizationSubsystem->GetLocalizedText(TEXT("Game.NewFeature.Text"));
```

### 2. Extract Text

Run extraction script:

```powershell
.\ExtractLocalizableText.ps1
```

### 3. Add Translations

Edit `MasterLocalizationTable.csv` and add translations for all languages.

### 4. Generate Files

```powershell
.\GenerateLocalizationFiles.ps1
```

### 5. Validate

```powershell
.\ValidateTranslations.ps1
```

## Format Strings

Use numbered placeholders for dynamic content:

```cpp
// Source text: "Player {0} scored {1} points"
TArray<FString> Args;
Args.Add(TEXT("Alice"));
Args.Add(TEXT("100"));

FText FormattedText = UTextLocalizationLibrary::FormatLocalizedText(
    TEXT("Game.Score.Message"), Args);
// Result: "Player Alice scored 100 points"
```

## Plural Forms

```cpp
// Define keys:
// Items.Singular = "You have {0} item"
// Items.Plural = "You have {0} items"

FText PluralText = UTextLocalizationLibrary::GetPluralText(TEXT("Items"), ItemCount);
```

## Audio Localization

```cpp
// Play localized audio
UAudioLocalizationLibrary::PlayLocalizedAudio2D(
    this, TEXT("Dialog.NPC.Greeting"));
```

Audio files should be organized:

```
Content/Audio/Localization/
├── en/
│   └── Dialog_NPC_Greeting.wav
├── zh-Hans/
│   └── Dialog_NPC_Greeting.wav
├── ja/
│   └── Dialog_NPC_Greeting.wav
└── ...
```

## Font Support

### CJK Languages

For Chinese, Japanese, and Korean, use fonts with full CJK character support:

- Noto Sans CJK
- Source Han Sans
- Custom game fonts with CJK glyphs

### Font Configuration

```cpp
FString FontPath = UUILocalizationLibrary::GetFontPathForLanguage(CurrentLanguage);
float TextScale = UUILocalizationLibrary::GetTextScaleForLanguage(CurrentLanguage);
```

## UI Considerations

### Text Length

Different languages have different text lengths:

- Chinese: Usually shorter than English
- Japanese: Similar to English
- Korean: Similar to English
- Spanish: Often 20-30% longer than English

Design UI with flexible layouts to accommodate text expansion.

### Text Wrapping

Enable text wrapping for all localized text blocks:

```cpp
TextBlock->SetAutoWrapText(true);
TextBlock->SetWrapTextAt(300.0f); // Adjust as needed
```

### Dynamic Font Sizing

```cpp
float BaseSize = 16.0f;
float Scale = UUILocalizationLibrary::GetTextScaleForLanguage(CurrentLanguage);
TextBlock->SetFontSize(BaseSize * Scale);
```

## Testing Localization

### Automated Tests

Run localization tests:

```
Session Frontend → Automation → HorrorProject.Localization
```

Tests include:
- LocalizationSubsystem functionality
- Language switching
- Text display
- Performance benchmarks

### Manual Testing

1. **Language Switching**: Test switching between all languages during gameplay
2. **UI Layout**: Verify all UI elements display correctly in each language
3. **Text Overflow**: Check for text overflow or truncation
4. **Font Rendering**: Verify CJK characters render correctly
5. **Audio Sync**: Ensure localized audio matches text

### Test Checklist

- [ ] All UI text displays correctly
- [ ] No text overflow or truncation
- [ ] Language switching works in all game states
- [ ] Fonts render correctly for all languages
- [ ] Format strings work with all languages
- [ ] Plural forms work correctly
- [ ] Localized audio plays correctly
- [ ] Performance is acceptable

## Performance Optimization

### Text Caching

The localization system caches retrieved text. Avoid:

```cpp
// Bad: Creates new FText every frame
void Tick(float DeltaTime)
{
    FText Text = GetLocalizedText(TEXT("UI.Health"));
}

// Good: Cache text, update only on language change
void BeginPlay()
{
    CachedHealthText = GetLocalizedText(TEXT("UI.Health"));
}
```

### Lazy Loading

Load language data on demand:

```cpp
// Language data is loaded only when language is set
LocalizationSubsystem->SetLanguage(ELanguage::Japanese);
```

## Troubleshooting

### Text Not Updating

1. Check if `UILocalizationComponent` is attached
2. Verify localization key is correct
3. Ensure language data is loaded
4. Check `OnLanguageChanged` delegate is bound

### Missing Translations

1. Run `ValidateTranslations.ps1` to find missing entries
2. Check `ValidationReport.html` for details
3. Add missing translations to master table
4. Regenerate language files

### Font Issues

1. Verify font asset includes required character sets
2. Check font is assigned to text blocks
3. Use `GetFontPathForLanguage()` for language-specific fonts

### Performance Issues

1. Profile with Unreal Insights
2. Check for excessive text retrieval in Tick()
3. Verify text caching is working
4. Consider pre-loading frequently used text

## Best Practices

1. **Use Keys, Not Hardcoded Text**: Always use localization keys
2. **Consistent Naming**: Follow key naming conventions
3. **Context Comments**: Add context for translators
4. **Test Early**: Test localization throughout development
5. **Plan for Expansion**: Design UI for text length variation
6. **Version Control**: Track localization files in version control
7. **Translator Tools**: Provide CSV files for easy translation
8. **Regular Validation**: Run validation scripts regularly

## Integration with UE Localization

This system can integrate with Unreal's built-in localization:

1. Export to `.po` files for professional translation tools
2. Use Localization Dashboard for large-scale projects
3. Integrate with translation management systems

## Future Enhancements

- Automatic language detection based on system locale
- Cloud-based translation updates
- Community translation support
- Voice-over localization
- Regional variants (en-US, en-GB, etc.)
- Localization analytics

## Support

For issues or questions:
1. Check this guide
2. Review automated test results
3. Run validation scripts
4. Contact localization team
