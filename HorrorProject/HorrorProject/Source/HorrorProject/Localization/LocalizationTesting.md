# Localization Testing Guide

## Overview

Comprehensive testing guide for the HorrorProject localization system, covering automated tests, manual testing procedures, and quality assurance.

## Test Categories

### 1. Automated Tests
### 2. Manual UI Testing
### 3. Gameplay Testing
### 4. Performance Testing
### 5. Compatibility Testing

---

## 1. Automated Tests

### Running Automated Tests

**Location**: Session Frontend → Automation → HorrorProject.Localization

**Command Line**:
```bash
UnrealEditor.exe HorrorProject.uproject -ExecCmds="Automation RunTests HorrorProject.Localization" -unattended -nopause -testexit="Automation Test Queue Empty"
```

### Test Suites

#### LocalizationSubsystem Tests
- **Test**: `HorrorProject.Localization.LocalizationSubsystem`
- **Coverage**: Core subsystem functionality
- **Checks**:
  - Subsystem initialization
  - Default language setting
  - Language code mapping
  - Language switching
  - Text retrieval
  - RTL detection

**Expected Results**:
- All tests pass
- Execution time < 100ms

#### Text Localization Tests
- **Test**: `HorrorProject.Localization.TextLocalization`
- **Coverage**: Text utilities
- **Checks**:
  - Text sanitization
  - Format string handling
  - Plural forms
  - Dynamic text registration

**Expected Results**:
- Text sanitization works correctly
- Format strings preserve placeholders

#### Language Switching Tests
- **Test**: `HorrorProject.Localization.LanguageSwitching`
- **Coverage**: Language switching behavior
- **Checks**:
  - Switch to each language
  - Rapid switching stability
  - Same language handling
  - Callback firing
  - Text consistency

**Expected Results**:
- All language switches successful
- No crashes or errors
- Callbacks fire correctly

#### Text Display Tests
- **Test**: `HorrorProject.Localization.TextDisplay.*`
- **Coverage**: Text rendering and display
- **Checks**:
  - Basic text display
  - Format strings
  - Text length validation
  - Special character encoding
  - Plural forms
  - RTL detection
  - Display performance
  - Text caching

**Expected Results**:
- All text displays correctly
- Performance within targets
- Caching works efficiently

#### Performance Tests
- **Test**: `HorrorProject.Localization.Performance`
- **Coverage**: System performance
- **Targets**:
  - Language switching: < 10ms
  - Text retrieval: < 1ms
  - Cached retrieval: < 0.01ms

**Expected Results**:
- All performance targets met
- No memory leaks
- Consistent performance

### Test Execution Schedule

**Daily** (CI/CD):
- All automated tests
- Performance benchmarks
- Memory leak detection

**Weekly**:
- Full test suite
- Extended stress tests
- Integration tests

**Pre-Release**:
- Complete test pass
- All languages verified
- Performance profiling

---

## 2. Manual UI Testing

### Test Environment Setup

1. Launch game in editor
2. Open language selection menu
3. Prepare test checklist
4. Have screenshots ready for comparison

### UI Test Cases

#### TC-UI-001: Main Menu
**Objective**: Verify main menu displays correctly in all languages

**Steps**:
1. Launch game
2. For each language:
   - Switch to language
   - Verify menu items display
   - Check text alignment
   - Verify no overflow
   - Check font rendering

**Expected Results**:
- All menu items visible
- Text fits within buttons
- Fonts render correctly
- No truncation

**Languages**: EN, ZH, JA, KO, ES

#### TC-UI-002: Options Menu
**Objective**: Verify options menu localization

**Steps**:
1. Open Options menu
2. For each language:
   - Check all option labels
   - Verify dropdown text
   - Check slider labels
   - Verify button text

**Expected Results**:
- All labels localized
- Dropdowns show correct text
- No UI overlap

#### TC-UI-003: HUD Elements
**Objective**: Verify in-game HUD localization

**Steps**:
1. Start game
2. For each language:
   - Check health/stamina labels
   - Verify objective text
   - Check item prompts
   - Verify tutorial hints

**Expected Results**:
- All HUD text localized
- Text readable during gameplay
- No performance impact

#### TC-UI-004: Dialog/Subtitles
**Objective**: Verify dialog and subtitle display

**Steps**:
1. Trigger dialog sequence
2. For each language:
   - Check subtitle text
   - Verify timing
   - Check line breaks
   - Verify speaker names

**Expected Results**:
- Subtitles match audio (if available)
- Text readable
- Proper line breaks
- No overflow

#### TC-UI-005: Inventory Screen
**Objective**: Verify inventory localization

**Steps**:
1. Open inventory
2. For each language:
   - Check item names
   - Verify descriptions
   - Check category labels
   - Verify tooltips

**Expected Results**:
- All items localized
- Descriptions fit in UI
- Tooltips display correctly

#### TC-UI-006: Pause Menu
**Objective**: Verify pause menu localization

**Steps**:
1. Pause game
2. For each language:
   - Check menu options
   - Verify confirmation dialogs
   - Check settings labels

**Expected Results**:
- All options localized
- Dialogs display correctly
- Navigation works

### UI Test Checklist

For each screen, verify:

- [ ] All text is localized
- [ ] No English text remains (except proper nouns)
- [ ] Text fits within UI elements
- [ ] No text overflow or truncation
- [ ] Fonts render correctly
- [ ] Special characters display
- [ ] Text alignment is correct
- [ ] Colors/contrast are readable
- [ ] Icons/images are appropriate
- [ ] Layout adapts to text length

### Text Overflow Testing

**Critical Areas**:
- Button labels (especially Spanish)
- Menu items
- Tooltips
- Dialog boxes
- HUD elements

**Test Method**:
1. Switch to Spanish (longest text)
2. Navigate to each screen
3. Check for overflow
4. Document any issues
5. Verify fixes with flexible layouts

---

## 3. Gameplay Testing

### Gameplay Test Cases

#### TC-GP-001: Tutorial Sequence
**Objective**: Verify tutorial text in all languages

**Steps**:
1. Start new game
2. For each language:
   - Complete tutorial
   - Read all instructions
   - Verify comprehension
   - Check timing

**Expected Results**:
- Instructions clear
- Timing appropriate
- No confusion

#### TC-GP-002: Objective System
**Objective**: Verify objective text localization

**Steps**:
1. Progress through game
2. For each language:
   - Check objective descriptions
   - Verify updates
   - Check completion messages

**Expected Results**:
- Objectives clear
- Updates display correctly
- Completion messages show

#### TC-GP-003: Item Interactions
**Objective**: Verify item interaction text

**Steps**:
1. Interact with items
2. For each language:
   - Check pickup messages
   - Verify use prompts
   - Check examination text

**Expected Results**:
- All prompts localized
- Messages clear
- No missing text

#### TC-GP-004: Save/Load System
**Objective**: Verify save/load messages

**Steps**:
1. Save game
2. Load game
3. For each language:
   - Check save messages
   - Verify load messages
   - Check error messages

**Expected Results**:
- All messages localized
- Clear feedback
- Errors understandable

### Gameplay Test Checklist

- [ ] Tutorial text clear and accurate
- [ ] Objectives understandable
- [ ] Item descriptions helpful
- [ ] Dialog makes sense in context
- [ ] Story progression clear
- [ ] Hints/tips useful
- [ ] Error messages informative
- [ ] Success messages satisfying

---

## 4. Performance Testing

### Performance Test Cases

#### TC-PERF-001: Language Switching Performance
**Objective**: Measure language switch time

**Method**:
```cpp
double StartTime = FPlatformTime::Seconds();
LocalizationSubsystem->SetLanguage(NewLanguage);
double EndTime = FPlatformTime::Seconds();
double SwitchTime = (EndTime - StartTime) * 1000.0; // ms
```

**Target**: < 10ms per switch

**Test**:
1. Switch between all languages
2. Measure each switch
3. Calculate average
4. Verify target met

#### TC-PERF-002: Text Retrieval Performance
**Objective**: Measure text retrieval time

**Method**:
```cpp
// Measure 1000 retrievals
for (int i = 0; i < 1000; i++) {
    FText Text = GetLocalizedText(Key);
}
```

**Target**: < 1ms per retrieval (average)

#### TC-PERF-003: Memory Usage
**Objective**: Measure localization memory footprint

**Method**:
1. Profile with Unreal Insights
2. Measure per-language data size
3. Check for memory leaks
4. Verify cleanup on language switch

**Target**: < 10MB per language

#### TC-PERF-004: Frame Rate Impact
**Objective**: Verify no FPS impact from localization

**Method**:
1. Measure baseline FPS
2. Switch languages during gameplay
3. Measure FPS during/after switch
4. Compare to baseline

**Target**: < 1 frame drop during switch

### Performance Benchmarks

| Metric | Target | Acceptable | Unacceptable |
|--------|--------|------------|--------------|
| Language Switch | < 10ms | < 50ms | > 50ms |
| Text Retrieval | < 1ms | < 5ms | > 5ms |
| Cached Retrieval | < 0.01ms | < 0.1ms | > 0.1ms |
| Memory per Lang | < 10MB | < 20MB | > 20MB |
| FPS Impact | 0 frames | 1 frame | > 1 frame |

---

## 5. Compatibility Testing

### Platform Testing

#### Windows
- [ ] All languages display correctly
- [ ] Fonts render properly
- [ ] Input methods work (CJK)
- [ ] Language switching smooth
- [ ] Save/load preserves language

#### Console (Future)
- [ ] Platform language detection
- [ ] Certification requirements met
- [ ] Performance targets met
- [ ] Region-specific content

### Font Testing

#### Latin Fonts (EN, ES)
- [ ] All characters render
- [ ] Special characters work (¡¿áéíóúñü)
- [ ] Bold/italic variants work
- [ ] Sizes scale correctly

#### CJK Fonts (ZH, JA, KO)
- [ ] All characters render
- [ ] No missing glyphs
- [ ] Character spacing correct
- [ ] Sizes appropriate
- [ ] Performance acceptable

### Input Method Testing

#### Chinese Input
- [ ] Pinyin input works
- [ ] Character selection works
- [ ] Text entry in UI fields

#### Japanese Input
- [ ] Romaji input works
- [ ] Kana conversion works
- [ ] Kanji selection works

#### Korean Input
- [ ] Hangul input works
- [ ] Character composition works
- [ ] Text entry smooth

---

## Test Reporting

### Bug Report Template

**Title**: [Language] Brief description

**Language**: EN/ZH/JA/KO/ES

**Category**: UI/Gameplay/Performance/Font

**Severity**: Critical/High/Medium/Low

**Description**:
- What happened
- What was expected
- Steps to reproduce

**Screenshots**: Attach relevant images

**System Info**:
- Platform
- Build version
- Language settings

### Test Results Template

**Test Session**: [Date]

**Tester**: [Name]

**Build**: [Version]

**Languages Tested**: EN, ZH, JA, KO, ES

**Results**:
| Test Case | EN | ZH | JA | KO | ES | Notes |
|-----------|----|----|----|----|----|----|
| TC-UI-001 | ✓  | ✓  | ✓  | ✓  | ✗  | Spanish overflow |
| TC-UI-002 | ✓  | ✓  | ✓  | ✓  | ✓  | |

**Issues Found**: 3
**Critical**: 0
**High**: 1
**Medium**: 2
**Low**: 0

**Summary**: Brief overview of test session

---

## Quality Gates

### Pre-Alpha
- [ ] Automated tests pass
- [ ] Core UI localized
- [ ] English complete

### Alpha
- [ ] All languages at 80%+
- [ ] UI testing complete
- [ ] No critical bugs

### Beta
- [ ] All languages at 95%+
- [ ] Full gameplay testing done
- [ ] Performance targets met
- [ ] No high-priority bugs

### Release Candidate
- [ ] All languages 100%
- [ ] All tests pass
- [ ] Native speaker review complete
- [ ] No open bugs

### Gold Master
- [ ] Final validation pass
- [ ] All quality gates met
- [ ] Stakeholder approval
- [ ] Ready for release

---

## Testing Tools

### Recommended Tools

**Automation**:
- Unreal Automation System
- Custom test scripts
- CI/CD integration

**Manual Testing**:
- Test case management (TestRail, Jira)
- Screenshot comparison tools
- Screen recording software

**Performance**:
- Unreal Insights
- Visual Studio Profiler
- Custom performance counters

**Font Testing**:
- Character map tools
- Font preview utilities
- Glyph coverage checkers

---

## Best Practices

### For Testers

1. **Test Systematically**: Follow test cases
2. **Document Everything**: Screenshots and details
3. **Test All Languages**: Don't skip any
4. **Check Edge Cases**: Long text, special characters
5. **Verify Fixes**: Retest after bug fixes

### For Developers

1. **Fix Root Causes**: Not just symptoms
2. **Test Locally**: Before submitting
3. **Update Tests**: When adding features
4. **Monitor Performance**: Profile regularly
5. **Respond Quickly**: To critical bugs

### For Localization Team

1. **Provide Context**: Help testers understand
2. **Prioritize Issues**: Focus on critical first
3. **Update Documentation**: Keep guides current
4. **Communicate Changes**: Notify team
5. **Track Metrics**: Monitor quality trends

---

## Continuous Testing

### Daily Testing
- Automated test suite
- Smoke tests for all languages
- Performance benchmarks

### Weekly Testing
- Full UI pass
- Gameplay testing
- New content verification

### Monthly Testing
- Complete regression test
- Performance profiling
- Quality metrics review

### Pre-Release Testing
- Full test suite
- Native speaker review
- Final validation
- Stakeholder approval

---

## Support

**Testing Team**:
- Lead: qa-lead@horrorproject.com
- Localization QA: loc-qa@horrorproject.com
- Automation: qa-automation@horrorproject.com

**Bug Reporting**:
- Jira: horrorproject.atlassian.net
- Email: bugs@horrorproject.com
- Slack: #qa-localization
