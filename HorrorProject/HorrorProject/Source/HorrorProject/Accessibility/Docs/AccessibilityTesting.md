# Accessibility Testing Guide

## Test Environment Setup

### Prerequisites
- Unreal Engine 5.6 or later
- HorrorProject compiled in Development or Debug configuration
- Session Frontend for automation tests

### Test Categories
1. Unit Tests (Automated)
2. Integration Tests (Automated)
3. Manual Accessibility Tests
4. User Acceptance Tests

## Automated Testing

### Running All Accessibility Tests
```
Session Frontend > Automation > Filter: "HorrorProject.Accessibility"
Select All > Run Tests
```

### Individual Test Suites

#### 1. Accessibility Subsystem Tests
**Location**: `Tests/TestAccessibilitySystem.cpp`

**Tests**:
- `SubsystemTest`: Initialization and settings management
- `WCAGCompliance`: Contrast ratio and UI scale validation
- `ColorBlindMode`: Color transformation accuracy
- `MotionSettings`: Motion blur and camera shake
- `UIScale`: UI scaling and clamping

**Run Command**:
```
Automation RunTest HorrorProject.Accessibility.SubsystemTest
```

#### 2. Color Blind Mode Tests
**Location**: `Tests/TestColorBlindMode.cpp`

**Tests**:
- `ColorBlindTransform`: Transformation accuracy for each type
- `ColorBlindSeverity`: Severity interpolation
- `ColorDistinguishability`: Color differentiation validation
- `AccessibleAlternative`: High-contrast alternatives
- `ColorBlindSimulation`: Simulation mode switching

**Run Command**:
```
Automation RunTest HorrorProject.Accessibility.ColorBlindMode
```

#### 3. Subtitle System Tests
**Location**: `Tests/TestSubtitleSystem.cpp`

**Tests**:
- `SubtitleBasic`: Adding and displaying subtitles
- `SubtitleQueue`: Queue management and limits
- `SubtitleUpdate`: Timing and expiration
- `SubtitleClear`: Clearing functionality
- `SubtitleFormat`: Text formatting with speakers
- `SubtitlePriority`: Priority-based sorting
- `SubtitleEnvironmental`: Environmental sound subtitles

**Run Command**:
```
Automation RunTest HorrorProject.Accessibility.SubtitleSystem
```

## Manual Testing

### 1. Subtitle System Testing

#### Test Case 1.1: Basic Subtitle Display
**Steps**:
1. Launch game
2. Open Accessibility Settings
3. Enable subtitles
4. Trigger dialogue or sound event
5. Verify subtitle appears with correct text

**Expected Result**: Subtitle displays with readable text and appropriate duration

#### Test Case 1.2: Subtitle Customization
**Steps**:
1. Open Accessibility Settings
2. Change subtitle size to "Large"
3. Change subtitle color to yellow
4. Change background color to black
5. Trigger subtitle
6. Verify changes applied

**Expected Result**: Subtitle displays with custom size and colors

#### Test Case 1.3: Multiple Subtitles
**Steps**:
1. Trigger multiple simultaneous sound events
2. Verify only max simultaneous subtitles shown
3. Verify highest priority subtitles displayed first

**Expected Result**: Queue management works correctly

### 2. Color Blind Mode Testing

#### Test Case 2.1: Protanopia Mode
**Steps**:
1. Open Accessibility Settings
2. Set Color Blind Mode to "Protanopia"
3. Navigate through game areas with red UI elements
4. Verify red elements are distinguishable

**Expected Result**: Red colors transformed to be visible

#### Test Case 2.2: Deuteranopia Mode
**Steps**:
1. Set Color Blind Mode to "Deuteranopia"
2. Check green UI elements and indicators
3. Verify green elements are distinguishable

**Expected Result**: Green colors transformed appropriately

#### Test Case 2.3: Tritanopia Mode
**Steps**:
1. Set Color Blind Mode to "Tritanopia"
2. Check blue UI elements
3. Verify blue elements are distinguishable

**Expected Result**: Blue colors transformed appropriately

### 3. Visual Audio Cue Testing

#### Test Case 3.1: Footstep Indicators
**Steps**:
1. Enable Visual Audio Cues
2. Walk around environment
3. Have NPC walk nearby
4. Verify footstep indicators appear

**Expected Result**: Yellow indicators show footstep locations

#### Test Case 3.2: Danger Indicators
**Steps**:
1. Enable Visual Audio Cues
2. Trigger enemy/danger sound
3. Verify red indicator appears
4. Check indicator direction accuracy

**Expected Result**: Red indicators point toward danger

#### Test Case 3.3: Distance Culling
**Steps**:
1. Enable Visual Audio Cues
2. Move far from sound source
3. Verify indicator disappears beyond max distance

**Expected Result**: Indicators only show for nearby sounds

### 4. Motion Settings Testing

#### Test Case 4.1: Motion Blur Toggle
**Steps**:
1. Enable motion blur
2. Move camera rapidly
3. Verify blur effect present
4. Disable motion blur
5. Move camera rapidly
6. Verify no blur effect

**Expected Result**: Motion blur toggles correctly

#### Test Case 4.2: Camera Shake Intensity
**Steps**:
1. Set camera shake to 100%
2. Trigger shake event
3. Note intensity
4. Set camera shake to 0%
5. Trigger shake event
6. Verify no shake occurs

**Expected Result**: Camera shake scales with setting

#### Test Case 4.3: Flash Reduction
**Steps**:
1. Enable "Reduce Flashing Lights"
2. Trigger flash effect
3. Verify reduced intensity or warning shown

**Expected Result**: Flash effects reduced or warned

### 5. Control Remapping Testing

#### Test Case 5.1: Key Remapping
**Steps**:
1. Open control settings
2. Remap "Jump" to different key
3. Test new key binding
4. Verify action triggers

**Expected Result**: Remapped key works correctly

#### Test Case 5.2: Conflict Detection
**Steps**:
1. Attempt to bind same key to two actions
2. Verify conflict warning appears

**Expected Result**: System prevents or warns about conflicts

#### Test Case 5.3: Gamepad Remapping
**Steps**:
1. Connect gamepad
2. Remap gamepad buttons
3. Test remapped controls
4. Verify functionality

**Expected Result**: Gamepad remapping works

### 6. UI Accessibility Testing

#### Test Case 6.1: UI Scaling
**Steps**:
1. Set UI scale to 50%
2. Verify UI elements shrink
3. Set UI scale to 200%
4. Verify UI elements enlarge
5. Check text readability at all scales

**Expected Result**: UI scales proportionally

#### Test Case 6.2: High Contrast Mode
**Steps**:
1. Enable High Contrast Mode
2. Navigate all menus
3. Verify all text is readable
4. Check button visibility

**Expected Result**: All UI elements have sufficient contrast

#### Test Case 6.3: Keyboard Navigation
**Steps**:
1. Disconnect mouse
2. Navigate menus using only keyboard
3. Tab through all options
4. Activate buttons with Enter/Space
5. Close menus with Escape

**Expected Result**: Full keyboard navigation works

## WCAG 2.1 AA Compliance Testing

### Perceivable Tests

#### Test P.1: Text Alternatives
**Requirement**: All non-text content has text alternative
**Test**: Verify all images, icons, and sounds have text descriptions
**Pass Criteria**: All content has accessible alternatives

#### Test P.2: Captions
**Requirement**: Captions for all audio content
**Test**: Play all audio and verify subtitles appear
**Pass Criteria**: 100% audio coverage

#### Test P.3: Contrast Ratio
**Requirement**: Minimum 4.5:1 for normal text, 3:1 for large text
**Test**: Measure contrast ratios using color picker
**Pass Criteria**: All text meets minimum ratios

### Operable Tests

#### Test O.1: Keyboard Accessible
**Requirement**: All functionality available via keyboard
**Test**: Complete game section using only keyboard
**Pass Criteria**: No mouse required

#### Test O.2: No Keyboard Trap
**Requirement**: Keyboard focus never trapped
**Test**: Tab through all UI, verify can always escape
**Pass Criteria**: No focus traps

#### Test O.3: Timing Adjustable
**Requirement**: User can extend time limits
**Test**: Verify subtitle duration is appropriate
**Pass Criteria**: Sufficient reading time

#### Test O.4: Seizures
**Requirement**: No more than 3 flashes per second
**Test**: Measure flash frequency in all effects
**Pass Criteria**: All effects below threshold

### Understandable Tests

#### Test U.1: Readable
**Requirement**: Text is readable and understandable
**Test**: Review all text for clarity
**Pass Criteria**: Clear, concise language

#### Test U.2: Predictable
**Requirement**: UI behaves predictably
**Test**: Verify consistent navigation and behavior
**Pass Criteria**: No unexpected behavior

#### Test U.3: Input Assistance
**Requirement**: Help users avoid and correct mistakes
**Test**: Verify error messages and validation
**Pass Criteria**: Clear error feedback

### Robust Tests

#### Test R.1: Compatible
**Requirement**: Compatible with assistive technologies
**Test**: Test with screen reader (if implemented)
**Pass Criteria**: Screen reader can access all content

## Performance Testing

### Test Case P.1: Subtitle Performance
**Steps**:
1. Spawn 100 simultaneous subtitle events
2. Monitor frame rate
3. Verify no significant performance impact

**Expected Result**: <1ms per frame for subtitle system

### Test Case P.2: Visual Audio Cue Performance
**Steps**:
1. Spawn 50 simultaneous audio cues
2. Monitor frame rate
3. Verify culling works correctly

**Expected Result**: <2ms per frame for cue system

### Test Case P.3: Color Blind Shader Performance
**Steps**:
1. Enable color blind mode
2. Monitor GPU performance
3. Compare to disabled state

**Expected Result**: <0.5ms GPU time for shader

## Regression Testing

### After Each Update
1. Run all automated tests
2. Verify settings save/load correctly
3. Test one complete playthrough with all features enabled
4. Check for any new accessibility issues

### Checklist
- [ ] All automated tests pass
- [ ] Subtitles display correctly
- [ ] Color blind modes work
- [ ] Visual audio cues appear
- [ ] Motion settings apply
- [ ] Controls can be remapped
- [ ] UI scales correctly
- [ ] Settings persist across sessions
- [ ] No performance regressions
- [ ] WCAG compliance maintained

## Bug Reporting Template

```
Title: [Accessibility] Brief description

Category: [Subtitle/ColorBlind/AudioCue/Motion/Controls/UI]

Severity: [Critical/High/Medium/Low]

Steps to Reproduce:
1. 
2. 
3. 

Expected Result:


Actual Result:


WCAG Guideline Affected: [If applicable]

Screenshots/Video: [If applicable]

System Info:
- OS: 
- GPU: 
- UE Version: 
```

## User Testing

### Recruit Test Users
- Users with color blindness
- Users with hearing impairments
- Users with motor disabilities
- Users who rely on keyboard navigation

### Test Protocol
1. Brief users on available features
2. Observe gameplay session (30-60 minutes)
3. Collect feedback via questionnaire
4. Note any difficulties or confusion
5. Iterate based on feedback

### Feedback Questions
1. Were you able to customize accessibility settings easily?
2. Did subtitles provide sufficient information?
3. Were visual audio cues helpful?
4. Could you navigate menus comfortably?
5. Did color blind modes improve visibility?
6. Were you able to remap controls as needed?
7. What accessibility features would you like to see added?

## Continuous Improvement

### Metrics to Track
- Percentage of players using accessibility features
- Most commonly used settings
- Feature usage patterns
- User feedback scores
- Compliance test pass rates

### Regular Reviews
- Monthly: Review automated test results
- Quarterly: Conduct user testing sessions
- Annually: Full WCAG compliance audit
