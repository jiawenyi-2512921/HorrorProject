# Translation Workflow

## Overview

This document describes the workflow for managing translations in HorrorProject, from text extraction to deployment.

## Workflow Stages

### 1. Text Extraction

**When**: After adding new text to the game

**Process**:
```powershell
cd D:\gptzuo\HorrorProject\HorrorProject\Source\HorrorProject\Localization
.\ExtractLocalizableText.ps1
```

**Output**:
- `ExtractedTexts.csv` - All localizable text found in source code
- `ExtractionSummary.txt` - Summary of extraction results

**Review**:
- Check for duplicate keys
- Verify all text is captured
- Add missing context information

### 2. Translation Preparation

**Prepare Translation Package**:

1. Export master table:
   ```powershell
   .\GenerateLocalizationFiles.ps1
   ```

2. Package includes:
   - `MasterLocalizationTable.csv` - All text with existing translations
   - Language-specific CSV files for each language
   - Context information and character limits

3. Add translator notes:
   - Character limits for UI text
   - Context for ambiguous terms
   - Brand terminology guidelines
   - Cultural considerations

### 3. Translation

**For Internal Translation**:

1. Open `MasterLocalizationTable.csv` in Excel or Google Sheets
2. Fill in missing translations for target languages
3. Mark status (Complete/Pending/Review)
4. Save as UTF-8 CSV

**For External Translation**:

1. Export language-specific CSV files
2. Send to translation agency with:
   - Style guide
   - Glossary
   - Context screenshots
   - Character limits
3. Receive translated files
4. Review for quality

**Translation Guidelines**:

- **Accuracy**: Maintain original meaning
- **Consistency**: Use consistent terminology
- **Context**: Consider game context
- **Length**: Stay within character limits
- **Tone**: Match game's horror atmosphere
- **Cultural**: Adapt culturally sensitive content

### 4. Translation Import

**Import Completed Translations**:

```powershell
.\ImportTranslations.ps1 -InputFile "translated_files.csv" -Merge
```

**Options**:
- `-Merge`: Merge with existing translations (recommended)
- Without `-Merge`: Replace all translations

**Verification**:
- Check import summary
- Review backup files created
- Verify no data loss

### 5. Validation

**Run Validation**:

```powershell
.\ValidateTranslations.ps1
```

**Review Report**:

1. Open `ValidationReport.html` in browser
2. Check completion rates per language
3. Review issues by severity:
   - **High**: Missing translations
   - **Medium**: Pending translations
   - **Low**: Format mismatches, length issues

**Fix Issues**:

1. Export `ValidationIssues.csv`
2. Address high-priority issues first
3. Update master table
4. Re-import and validate

### 6. Testing

**Automated Testing**:

```
Unreal Editor → Session Frontend → Automation
Filter: HorrorProject.Localization
Run All Tests
```

**Manual Testing**:

1. **Language Switching**:
   - Switch between all languages
   - Verify no crashes or errors
   - Check language persists across sessions

2. **UI Testing**:
   - Check all menus and screens
   - Verify text fits in UI elements
   - Test with longest translations (usually Spanish)

3. **Gameplay Testing**:
   - Play through key game sections
   - Verify tutorial text
   - Check objective descriptions
   - Test dialog and subtitles

4. **Font Testing**:
   - Verify CJK characters render correctly
   - Check for missing glyphs
   - Test special characters

**Test Matrix**:

| Language | UI | Gameplay | Audio | Fonts | Status |
|----------|----|---------:|-------|-------|--------|
| English  | ✓  | ✓        | ✓     | ✓     | Pass   |
| Chinese  | ✓  | ✓        | ✓     | ✓     | Pass   |
| Japanese | ✓  | ✓        | ✓     | ✓     | Pass   |
| Korean   | ✓  | ✓        | ✓     | ✓     | Pass   |
| Spanish  | ✓  | ✓        | ✓     | ✓     | Pass   |

### 7. Deployment

**Pre-Deployment**:

1. Final validation pass
2. All tests passing
3. Translation completion > 95%
4. Stakeholder approval

**Package Localization Data**:

```
Content/Localization/
├── en/Game.json
├── zh-Hans/Game.json
├── ja/Game.json
├── ko/Game.json
└── es/Game.json
```

**Build Configuration**:

Ensure all language files are included in build:
- Check packaging settings
- Verify file sizes
- Test packaged build with each language

## Continuous Localization

### Incremental Updates

**For Small Changes**:

1. Add new text to source code
2. Run extraction script
3. Translate new entries only
4. Import with `-Merge` flag
5. Quick validation
6. Deploy update

**For Large Updates**:

Follow full workflow from extraction to deployment.

### Version Control

**Commit Strategy**:

```
feat(loc): Add Chinese translations for new tutorial
fix(loc): Correct Japanese menu text overflow
chore(loc): Update master localization table
```

**Branch Strategy**:

- `main` - Production translations
- `loc/[language]` - Language-specific work
- `loc/update-[date]` - Translation update branches

**Review Process**:

1. Translator commits to language branch
2. Localization lead reviews
3. QA tests in-game
4. Merge to main after approval

## Quality Assurance

### Translation Quality Checks

**Automated Checks**:
- Missing translations
- Format string mismatches
- Excessive length differences
- Duplicate keys
- Invalid characters

**Manual Checks**:
- Terminology consistency
- Cultural appropriateness
- Tone and style
- Context accuracy
- Grammar and spelling

### Linguistic QA

**Native Speaker Review**:

Each language should be reviewed by native speaker:
- Fluency and naturalness
- Cultural sensitivity
- Game-specific terminology
- Consistency across game

**LQA Checklist**:
- [ ] Text reads naturally
- [ ] Terminology is consistent
- [ ] No grammatical errors
- [ ] Culturally appropriate
- [ ] Matches game tone
- [ ] No truncation in UI
- [ ] Special characters work

## Tools and Resources

### Required Tools

- **PowerShell 5.1+** - For automation scripts
- **Excel/Google Sheets** - For editing CSV files
- **Text Editor** - UTF-8 capable (VS Code, Notepad++)
- **Unreal Editor** - For in-game testing

### Optional Tools

- **Translation Memory** - SDL Trados, MemoQ
- **CAT Tools** - Computer-Assisted Translation
- **Localization Platform** - Crowdin, Lokalise
- **QA Tools** - Xbench, ErrorSpy

### Resources

**Style Guides**:
- HorrorProject Brand Guidelines
- Language-specific style guides
- Horror genre terminology

**Glossaries**:
- Game-specific terms
- Character names
- Location names
- Item names
- UI terminology

**Reference Materials**:
- Game design documents
- Story outlines
- Character descriptions
- World lore

## Roles and Responsibilities

### Localization Manager
- Oversee entire localization process
- Coordinate with translators
- Manage timeline and budget
- Final approval

### Translators
- Translate text accurately
- Maintain consistency
- Follow style guides
- Meet deadlines

### Localization Engineers
- Maintain localization system
- Run automation scripts
- Fix technical issues
- Support translators

### QA Testers
- Test all languages
- Report issues
- Verify fixes
- Sign off on quality

### Developers
- Mark text for localization
- Provide context
- Fix localization bugs
- Support integration

## Timeline Estimates

### Small Update (< 100 strings)
- Extraction: 30 minutes
- Translation: 1-2 days
- Validation: 1 hour
- Testing: 2-4 hours
- **Total**: 2-3 days

### Medium Update (100-500 strings)
- Extraction: 1 hour
- Translation: 3-5 days
- Validation: 2 hours
- Testing: 1 day
- **Total**: 1 week

### Large Update (> 500 strings)
- Extraction: 2 hours
- Translation: 1-2 weeks
- Validation: 4 hours
- Testing: 2-3 days
- **Total**: 2-3 weeks

### Full Game Localization
- Extraction: 1 day
- Translation: 4-8 weeks
- Validation: 1 week
- Testing: 2 weeks
- **Total**: 8-12 weeks

## Best Practices

### For Developers

1. **Use Localization Keys**: Never hardcode text
2. **Provide Context**: Add comments for translators
3. **Test Early**: Don't wait until the end
4. **Plan for Expansion**: Design flexible UI
5. **Avoid Concatenation**: Use format strings

### For Translators

1. **Play the Game**: Understand context
2. **Ask Questions**: Clarify ambiguities
3. **Use Glossary**: Maintain consistency
4. **Check Length**: Respect character limits
5. **Test In-Game**: See your work in context

### For QA

1. **Test All Languages**: Don't skip any
2. **Check Edge Cases**: Long text, special characters
3. **Verify Fonts**: Especially for CJK
4. **Test Switching**: Language changes during gameplay
5. **Document Issues**: Clear, reproducible reports

## Common Issues and Solutions

### Issue: Text Overflow

**Solution**:
- Redesign UI for flexibility
- Use text wrapping
- Abbreviate where appropriate
- Adjust font size per language

### Issue: Missing Translations

**Solution**:
- Run validation script
- Prioritize by importance
- Use fallback to English
- Schedule translation update

### Issue: Font Not Displaying

**Solution**:
- Verify font includes character set
- Check font assignment in UI
- Use language-specific fonts
- Test with font fallback

### Issue: Format String Errors

**Solution**:
- Validate placeholder count
- Check placeholder order
- Test with actual values
- Update translation if needed

### Issue: Performance Problems

**Solution**:
- Profile text retrieval
- Implement caching
- Lazy load language data
- Optimize lookup tables

## Metrics and KPIs

### Translation Metrics

- **Completion Rate**: % of strings translated
- **Quality Score**: LQA rating (1-5)
- **Consistency Score**: Terminology adherence
- **Time to Translate**: Words per day
- **Cost per Word**: Budget tracking

### Technical Metrics

- **Text Retrieval Time**: < 0.1ms target
- **Language Switch Time**: < 10ms target
- **Memory Usage**: Per language data
- **Build Size**: Localization data size

### Quality Metrics

- **Bug Rate**: Localization bugs per 1000 strings
- **Player Feedback**: Language-specific ratings
- **Completion Rate**: Players finishing in each language
- **Support Tickets**: Language-related issues

## Continuous Improvement

### Regular Reviews

- **Weekly**: Check translation progress
- **Monthly**: Review quality metrics
- **Quarterly**: Update workflows and tools
- **Annually**: Evaluate overall localization strategy

### Feedback Loop

1. Collect player feedback
2. Analyze common issues
3. Update processes
4. Train team on improvements
5. Measure impact

### Process Optimization

- Automate repetitive tasks
- Improve tool integration
- Streamline approval process
- Reduce turnaround time
- Enhance quality checks

## Support and Contact

**Localization Team**:
- Email: localization@horrorproject.com
- Slack: #localization
- Wiki: wiki.horrorproject.com/localization

**Emergency Contact**:
- Critical issues: loc-emergency@horrorproject.com
- On-call: Check team calendar
