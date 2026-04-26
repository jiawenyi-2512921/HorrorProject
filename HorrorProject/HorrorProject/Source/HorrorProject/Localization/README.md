# Localization System

Complete multi-language localization system for HorrorProject.

## Features

- **5 Languages Supported**: English, Chinese (Simplified), Japanese, Korean, Spanish
- **Runtime Language Switching**: Change language without restarting
- **Comprehensive API**: C++ and Blueprint support
- **UI Localization Component**: Automatic widget updates
- **Audio Localization**: Language-specific audio support
- **Format Strings**: Dynamic text with placeholders
- **Plural Forms**: Language-aware plural handling
- **Font Support**: CJK font integration
- **Performance Optimized**: Cached text retrieval, fast switching

## Quick Start

### C++ Usage

```cpp
// Get subsystem
ULocalizationSubsystem* LocalizationSubsystem = 
    GetGameInstance()->GetSubsystem<ULocalizationSubsystem>();

// Change language
LocalizationSubsystem->SetLanguage(ELanguage::Japanese);

// Get localized text
FText Text = LocalizationSubsystem->GetLocalizedText(TEXT("UI.MainMenu.Start"));

// Listen for changes
LocalizationSubsystem->OnLanguageChanged.AddDynamic(this, &UMyClass::OnLanguageChanged);
```

### Blueprint Usage

Use the Localization Subsystem nodes to get/set language and retrieve localized text.

### UI Localization

Add `UILocalizationComponent` to your widget:
1. Add component
2. Configure text widgets with localization keys
3. Component auto-updates on language change

## Tools

### Extract Text
```powershell
.\ExtractLocalizableText.ps1
```

### Generate Language Files
```powershell
.\GenerateLocalizationFiles.ps1
```

### Validate Translations
```powershell
.\ValidateTranslations.ps1
```

### Import Translations
```powershell
.\ImportTranslations.ps1 -InputFile "translations.csv" -Merge
```

## File Structure

```
Localization/
├── LocalizationSubsystem.h/cpp      # Core subsystem
├── TextLocalization.h/cpp           # Text utilities
├── AudioLocalization.h/cpp          # Audio localization
├── UILocalization.h/cpp             # UI component
├── ExtractLocalizableText.ps1       # Text extraction
├── GenerateLocalizationFiles.ps1    # File generation
├── ValidateTranslations.ps1         # Validation
├── ImportTranslations.ps1           # Import tool
├── LocalizationGuide.md             # Usage guide
├── TranslationWorkflow.md           # Workflow docs
├── SupportedLanguages.md            # Language info
└── LocalizationTesting.md           # Testing guide
```

## Testing

Run automated tests:
```
Session Frontend → Automation → HorrorProject.Localization
```

Test suites:
- LocalizationSubsystem
- TextLocalization
- LanguageSwitching
- TextDisplay
- Performance

## Documentation

- **LocalizationGuide.md**: Complete usage guide
- **TranslationWorkflow.md**: Translation process
- **SupportedLanguages.md**: Language details
- **LocalizationTesting.md**: Testing procedures

## Performance

- Language switching: < 10ms
- Text retrieval: < 1ms
- Cached retrieval: < 0.01ms
- Memory per language: < 10MB

## Language Support

| Language | Code | Status | Completion |
|----------|------|--------|------------|
| English | en | ✓ | 100% |
| Chinese | zh-Hans | ✓ | 100% |
| Japanese | ja | ✓ | 100% |
| Korean | ko | ✓ | 100% |
| Spanish | es | ✓ | 100% |

## Key Naming Convention

```
Category.Subcategory.Identifier

Examples:
- UI.MainMenu.Start
- Game.Tutorial.Movement
- Dialog.NPC.Greeting
```

## Integration

1. Add localization keys to your code
2. Run extraction script
3. Add translations to master table
4. Generate language files
5. Test in-game

## Future Enhancements

- Additional languages (German, French, Portuguese, Russian, Italian)
- Voice-over localization
- Cloud-based translation updates
- Community translation support
- Regional variants

## Support

For issues or questions, see documentation or contact the localization team.
