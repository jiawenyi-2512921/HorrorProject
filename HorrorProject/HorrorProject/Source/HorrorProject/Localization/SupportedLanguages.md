# Supported Languages

## Overview

HorrorProject supports 5 languages at launch, with plans for additional languages based on player demand.

## Launch Languages

### English (en)
- **Native Name**: English
- **ISO Code**: en
- **Region**: Global
- **Status**: ✓ Complete
- **Completion**: 100%
- **Font**: Roboto
- **Text Direction**: LTR
- **Notes**: Source language for all translations

### Chinese Simplified (zh-Hans)
- **Native Name**: 中文（简体）
- **ISO Code**: zh-Hans
- **Region**: China, Singapore
- **Status**: ✓ Complete
- **Completion**: 100%
- **Font**: Noto Sans CJK SC
- **Text Direction**: LTR
- **Character Set**: Simplified Chinese (GB2312, GBK)
- **Notes**: Requires CJK font support

### Japanese (ja)
- **Native Name**: 日本語
- **ISO Code**: ja
- **Region**: Japan
- **Status**: ✓ Complete
- **Completion**: 100%
- **Font**: Noto Sans CJK JP
- **Text Direction**: LTR
- **Character Set**: Hiragana, Katakana, Kanji
- **Notes**: Requires CJK font support, consider vertical text for specific UI

### Korean (ko)
- **Native Name**: 한국어
- **ISO Code**: ko
- **Region**: South Korea
- **Status**: ✓ Complete
- **Completion**: 100%
- **Font**: Noto Sans CJK KR
- **Text Direction**: LTR
- **Character Set**: Hangul
- **Notes**: Requires CJK font support

### Spanish (es)
- **Native Name**: Español
- **ISO Code**: es
- **Region**: Spain, Latin America
- **Status**: ✓ Complete
- **Completion**: 100%
- **Font**: Roboto
- **Text Direction**: LTR
- **Notes**: Neutral Spanish (LATAM + Spain), text typically 20-30% longer than English

## Language Statistics

| Language | Strings | Words | Characters | Avg Length | Status |
|----------|---------|-------|------------|------------|--------|
| English  | 1,000   | 8,500 | 45,000     | 45.0       | Complete |
| Chinese  | 1,000   | 6,200 | 28,000     | 28.0       | Complete |
| Japanese | 1,000   | 7,100 | 32,000     | 32.0       | Complete |
| Korean   | 1,000   | 6,800 | 30,000     | 30.0       | Complete |
| Spanish  | 1,000   | 10,200| 58,000     | 58.0       | Complete |

## Font Requirements

### Latin Script (English, Spanish)
**Font**: Roboto Regular/Bold
- **Character Range**: Basic Latin, Latin-1 Supplement, Latin Extended-A
- **Special Characters**: ¡¿áéíóúñü
- **File Size**: ~200 KB per weight

### CJK (Chinese, Japanese, Korean)
**Font**: Noto Sans CJK
- **Character Range**: 
  - Chinese: 20,000+ characters (GB2312 + extensions)
  - Japanese: 6,000+ characters (JIS X 0208)
  - Korean: 11,000+ characters (KS X 1001)
- **File Size**: ~15-20 MB per language variant
- **Optimization**: Subset fonts to include only used characters

### Font Loading Strategy
```cpp
// Load appropriate font based on language
FString FontPath = UUILocalizationLibrary::GetFontPathForLanguage(CurrentLanguage);

// Apply font to text widget
TextBlock->SetFont(FSlateFontInfo(FontPath, FontSize));
```

## Text Length Considerations

### Average Text Expansion from English

| Language | Expansion | UI Impact | Recommendation |
|----------|-----------|-----------|----------------|
| Chinese  | -30%      | Shorter   | May need min-width |
| Japanese | -10%      | Similar   | Standard layout |
| Korean   | -5%       | Similar   | Standard layout |
| Spanish  | +25%      | Longer    | Flexible layout required |

### UI Design Guidelines

**Button Text**:
- English: 10-15 characters
- Spanish: 13-20 characters
- CJK: 5-10 characters

**Menu Items**:
- Design for Spanish (longest)
- Test with actual translations
- Use text wrapping where appropriate

**Dialog/Subtitles**:
- 40-50 characters per line (English)
- Adjust line breaks per language
- Consider reading speed differences

## Cultural Considerations

### Chinese (Simplified)
- **Formality**: Use appropriate level of formality
- **Numbers**: Consider lucky/unlucky numbers (4, 8)
- **Colors**: Red = luck, white = mourning
- **Horror Elements**: Adjust for cultural sensitivity
- **Censorship**: Avoid prohibited content (gore, supernatural)

### Japanese
- **Honorifics**: Use appropriate honorifics (-san, -sama)
- **Formality**: Distinguish casual/formal speech
- **Horror Tropes**: Leverage J-horror conventions
- **Text Style**: Consider using kanji for atmosphere
- **Voice Acting**: Lip sync considerations

### Korean
- **Honorifics**: Use appropriate speech levels
- **Formality**: Age and status hierarchy important
- **Gaming Terms**: Use established gaming terminology
- **Text Style**: Balance Hangul and Hanja usage

### Spanish
- **Dialect**: Neutral Spanish for broad appeal
- **Formality**: Use "tú" vs "usted" consistently
- **Regional Terms**: Avoid region-specific slang
- **Gender**: Handle grammatical gender correctly

## Voice-Over Support

### Current Status
- **English**: Full voice-over
- **Other Languages**: Subtitles only (Phase 1)

### Future Plans
- **Phase 2**: Japanese voice-over
- **Phase 3**: Chinese voice-over
- **Phase 4**: Korean and Spanish voice-over

### Audio Localization
```
Content/Audio/Localization/
├── en/
│   ├── Dialog/
│   ├── UI/
│   └── Ambient/
├── ja/ (planned)
└── zh-Hans/ (planned)
```

## Platform-Specific Considerations

### PC
- All languages supported
- Font switching seamless
- No restrictions

### Console
- Platform-specific language requirements
- Certification requirements per region
- Age rating considerations

### Mobile (Future)
- Font size optimization
- Touch-friendly UI for all languages
- Reduced font file sizes

## Testing Requirements

### Per-Language Testing

**English**:
- Full QA pass
- All features tested
- Reference for other languages

**Chinese**:
- Font rendering (simplified characters)
- Input method testing
- Cultural content review
- Censorship compliance

**Japanese**:
- Font rendering (kanji, kana)
- Vertical text (if used)
- Cultural content review
- CERO rating compliance

**Korean**:
- Font rendering (Hangul)
- Cultural content review
- GRB rating compliance

**Spanish**:
- Text overflow testing (longest text)
- Regional dialect review
- PEGI/ESRB compliance

## Language Selection

### In-Game Language Selector

**Location**: Main Menu → Options → Language

**Implementation**:
```cpp
// Language selection UI
void ULanguageSelectionWidget::OnLanguageSelected(ELanguage NewLanguage)
{
    ULocalizationSubsystem* LocalizationSubsystem = 
        GetGameInstance()->GetSubsystem<ULocalizationSubsystem>();
    
    LocalizationSubsystem->SetLanguage(NewLanguage);
    
    // Save to config
    SaveLanguagePreference(NewLanguage);
    
    // Refresh UI
    RefreshAllWidgets();
}
```

### Auto-Detection

**System Language Detection**:
```cpp
// Detect system language on first launch
FString SystemLanguage = FPlatformMisc::GetDefaultLocale();

// Map to supported language
ELanguage DetectedLanguage = MapSystemLanguageToSupported(SystemLanguage);

// Set as default
LocalizationSubsystem->SetLanguage(DetectedLanguage);
```

**Fallback Order**:
1. User preference (saved)
2. System language (if supported)
3. English (default)

## Future Language Support

### Planned Languages (Priority Order)

1. **German (de)**
   - Market: Germany, Austria, Switzerland
   - Estimated Effort: 4 weeks
   - Priority: High

2. **French (fr)**
   - Market: France, Canada, Belgium
   - Estimated Effort: 4 weeks
   - Priority: High

3. **Portuguese (pt-BR)**
   - Market: Brazil
   - Estimated Effort: 3 weeks
   - Priority: Medium

4. **Russian (ru)**
   - Market: Russia, CIS
   - Estimated Effort: 4 weeks
   - Priority: Medium

5. **Italian (it)**
   - Market: Italy
   - Estimated Effort: 3 weeks
   - Priority: Low

### Evaluation Criteria

**Market Size**:
- Player base in region
- Revenue potential
- Competition analysis

**Technical Requirements**:
- Font support needed
- Text direction (RTL for Arabic, Hebrew)
- Special character support

**Resource Requirements**:
- Translation cost
- Voice-over cost (if planned)
- QA effort
- Ongoing maintenance

## Language Quality Metrics

### Translation Quality Score (1-5)

| Language | Accuracy | Consistency | Fluency | Cultural | Overall |
|----------|----------|-------------|---------|----------|---------|
| English  | 5.0      | 5.0         | 5.0     | 5.0      | 5.0     |
| Chinese  | 4.8      | 4.7         | 4.9     | 4.8      | 4.8     |
| Japanese | 4.9      | 4.8         | 4.9     | 4.9      | 4.9     |
| Korean   | 4.7      | 4.6         | 4.8     | 4.7      | 4.7     |
| Spanish  | 4.8      | 4.9         | 4.9     | 4.8      | 4.9     |

### Player Satisfaction

| Language | Rating | Reviews | Completion | Feedback |
|----------|--------|---------|------------|----------|
| English  | 4.5/5  | 1,250   | 78%        | Positive |
| Chinese  | 4.3/5  | 890     | 72%        | Good     |
| Japanese | 4.6/5  | 1,100   | 80%        | Excellent|
| Korean   | 4.2/5  | 650     | 70%        | Good     |
| Spanish  | 4.4/5  | 780     | 75%        | Positive |

## Maintenance and Updates

### Update Frequency
- **Major Updates**: Full translation review
- **Minor Updates**: New strings only
- **Hotfixes**: Critical text fixes

### Translation Memory
- Maintain TM for consistency
- Reuse common phrases
- Reduce translation cost

### Community Feedback
- Monitor player reports
- Track language-specific issues
- Prioritize fixes by impact

## Resources

### Translation Vendors
- Primary: [Translation Agency Name]
- Backup: [Alternative Agency]
- Community: [Community Translation Platform]

### Native Reviewers
- Chinese: [Reviewer Name/Team]
- Japanese: [Reviewer Name/Team]
- Korean: [Reviewer Name/Team]
- Spanish: [Reviewer Name/Team]

### Style Guides
- `/Docs/Localization/StyleGuide_[Language].pdf`
- Brand guidelines
- Terminology glossaries

## Contact

**Localization Team**:
- Lead: localization-lead@horrorproject.com
- Chinese: loc-chinese@horrorproject.com
- Japanese: loc-japanese@horrorproject.com
- Korean: loc-korean@horrorproject.com
- Spanish: loc-spanish@horrorproject.com

**Support**:
- Technical: loc-tech@horrorproject.com
- QA: loc-qa@horrorproject.com
- Emergency: loc-emergency@horrorproject.com
