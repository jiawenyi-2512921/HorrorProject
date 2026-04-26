# Level Test Plan Template

## Test Information

**Level Name**: [Level Code/Name]
**Test Phase**: [Blockout/Art Pass/Polish/Final]
**Tester**: [Your Name]
**Date**: [Test Date]
**Build Version**: [Version Number]

## Test Objectives

### Primary Objectives
- [ ] Verify all gameplay systems functional
- [ ] Validate performance within budget
- [ ] Confirm player experience matches design intent
- [ ] Identify and document all bugs

### Secondary Objectives
- [ ] Gather feedback on pacing
- [ ] Assess difficulty balance
- [ ] Evaluate audio/visual quality
- [ ] Test edge cases

## Pre-Test Setup

### Environment
- [ ] Latest build installed
- [ ] Performance monitoring tools ready
- [ ] Recording software configured
- [ ] Test save files prepared

### Test Configuration
- **Platform**: [PC/Console]
- **Graphics Settings**: [Low/Medium/High/Ultra]
- **Resolution**: [Resolution]
- **Frame Rate Target**: [30/60/120 FPS]

## Functionality Tests

### Navigation Tests

#### Test 1.1: Basic Movement
- **Objective**: Verify player can navigate all areas
- **Steps**:
  1. Spawn at start point
  2. Walk through entire level
  3. Test all movement types (walk, run, crouch, jump)
  4. Verify no invisible walls or stuck spots
- **Expected Result**: Smooth navigation throughout
- **Status**: [ ] Pass [ ] Fail [ ] N/A
- **Notes**: 

#### Test 1.2: Objective Accessibility
- **Objective**: Verify all objectives reachable
- **Steps**:
  1. Navigate to each objective location
  2. Verify clear path exists
  3. Check for navigation hints
- **Expected Result**: All objectives accessible
- **Status**: [ ] Pass [ ] Fail [ ] N/A
- **Notes**: 

#### Test 1.3: Evidence Accessibility
- **Objective**: Verify all evidence reachable
- **Steps**:
  1. Navigate to each evidence location
  2. Verify pickup interaction works
  3. Check visibility and discoverability
- **Expected Result**: All evidence accessible
- **Status**: [ ] Pass [ ] Fail [ ] N/A
- **Notes**: 

### Objective Tests

#### Test 2.1: Objective 1 - [Name]
- **Objective**: Verify objective completable
- **Steps**:
  1. Navigate to objective location
  2. Complete objective requirements
  3. Verify completion feedback
  4. Check progression unlock
- **Expected Result**: Objective completes successfully
- **Status**: [ ] Pass [ ] Fail [ ] N/A
- **Time Taken**: [X] minutes
- **Notes**: 

[Repeat for all 8 objectives]

### Evidence Tests

#### Test 3.1: Required Evidence Collection
- **Objective**: Verify all required evidence collectible
- **Steps**:
  1. Collect all 8 required evidence items
  2. Verify UI updates correctly
  3. Check content displays properly
- **Expected Result**: All evidence collectible
- **Status**: [ ] Pass [ ] Fail [ ] N/A
- **Notes**: 

#### Test 3.2: Optional Evidence Collection
- **Objective**: Verify optional evidence collectible
- **Steps**:
  1. Collect optional evidence items
  2. Verify rewards granted
- **Expected Result**: Optional evidence works
- **Status**: [ ] Pass [ ] Fail [ ] N/A
- **Notes**: 

#### Test 3.3: Secret Evidence Discovery
- **Objective**: Verify secret evidence findable
- **Steps**:
  1. Search for secret evidence
  2. Verify discovery methods work
  3. Check achievement unlocks
- **Expected Result**: Secrets discoverable
- **Status**: [ ] Pass [ ] Fail [ ] N/A
- **Notes**: 

### Enemy Tests

#### Test 4.1: Enemy AI Behavior
- **Objective**: Verify enemy AI functions correctly
- **Steps**:
  1. Observe idle behavior
  2. Trigger alert state
  3. Test chase behavior
  4. Verify attack patterns
- **Expected Result**: AI behaves as designed
- **Status**: [ ] Pass [ ] Fail [ ] N/A
- **Notes**: 

#### Test 4.2: Enemy Detection
- **Objective**: Verify detection systems work
- **Steps**:
  1. Test sight detection
  2. Test sound detection
  3. Verify detection ranges
  4. Check stealth mechanics
- **Expected Result**: Detection fair and consistent
- **Status**: [ ] Pass [ ] Fail [ ] N/A
- **Notes**: 

## Performance Tests

### Frame Rate Tests

#### Test 5.1: Average Frame Rate
- **Objective**: Verify stable frame rate
- **Method**: Monitor FPS throughout playthrough
- **Target**: 60 FPS average
- **Result**: [X] FPS average
- **Status**: [ ] Pass [ ] Fail
- **Notes**: 

#### Test 5.2: Frame Rate Drops
- **Objective**: Identify performance problem areas
- **Method**: Note locations with FPS drops
- **Problem Areas**:
  - [Location 1]: [X] FPS
  - [Location 2]: [X] FPS
- **Status**: [ ] Pass [ ] Fail
- **Notes**: 

### Memory Tests

#### Test 5.3: Memory Usage
- **Objective**: Verify memory within budget
- **Method**: Monitor memory usage
- **Target**: < 8GB
- **Result**: [X] GB
- **Status**: [ ] Pass [ ] Fail
- **Notes**: 

### Loading Tests

#### Test 5.4: Level Load Time
- **Objective**: Verify acceptable load times
- **Method**: Time level loading
- **Target**: < 30 seconds
- **Result**: [X] seconds
- **Status**: [ ] Pass [ ] Fail
- **Notes**: 

## Gameplay Experience Tests

### Pacing Tests

#### Test 6.1: Overall Pacing
- **Objective**: Verify pacing feels right
- **Method**: Subjective assessment during playthrough
- **Questions**:
  - Does tension build appropriately? [ ] Yes [ ] No
  - Are there enough breather moments? [ ] Yes [ ] No
  - Does the level feel too long/short? [ ] Too Long [ ] Just Right [ ] Too Short
- **Notes**: 

#### Test 6.2: 7 Beats Structure
- **Objective**: Verify beats are distinct
- **Method**: Identify each beat during playthrough
- **Beat 1 (Arrival)**: [ ] Clear [ ] Unclear
- **Beat 2 (Exploration)**: [ ] Clear [ ] Unclear
- **Beat 3 (First Threat)**: [ ] Clear [ ] Unclear
- **Beat 4 (Rising Tension)**: [ ] Clear [ ] Unclear
- **Beat 5 (Crisis Point)**: [ ] Clear [ ] Unclear
- **Beat 6 (Resolution Path)**: [ ] Clear [ ] Unclear
- **Beat 7 (Escape)**: [ ] Clear [ ] Unclear
- **Notes**: 

### Difficulty Tests

#### Test 6.3: Difficulty Balance
- **Objective**: Verify appropriate difficulty
- **Method**: Subjective assessment
- **Questions**:
  - Are objectives too easy/hard? [ ] Too Easy [ ] Just Right [ ] Too Hard
  - Are enemies fair? [ ] Yes [ ] No
  - Is resource balance good? [ ] Yes [ ] No
- **Notes**: 

### Navigation Tests

#### Test 6.4: Player Guidance
- **Objective**: Verify players know where to go
- **Method**: Observe first-time player (if possible)
- **Questions**:
  - Are objectives easy to find? [ ] Yes [ ] No
  - Is navigation intuitive? [ ] Yes [ ] No
  - Are there confusing areas? [ ] Yes [ ] No
- **Confusing Areas**: [List]
- **Notes**: 

## Audio/Visual Tests

### Lighting Tests

#### Test 7.1: Lighting Quality
- **Objective**: Verify lighting meets quality bar
- **Method**: Visual inspection of all zones
- **Questions**:
  - Is lighting polished? [ ] Yes [ ] No
  - Are there light leaks? [ ] Yes [ ] No
  - Is mood appropriate per zone? [ ] Yes [ ] No
- **Issues Found**: [List]
- **Notes**: 

### Audio Tests

#### Test 7.2: Audio Quality
- **Objective**: Verify audio works correctly
- **Method**: Listen throughout playthrough
- **Questions**:
  - Are all sounds playing? [ ] Yes [ ] No
  - Is audio balanced? [ ] Yes [ ] No
  - Are there audio bugs? [ ] Yes [ ] No
- **Issues Found**: [List]
- **Notes**: 

## Bug Tracking

### Critical Bugs
| ID | Description | Location | Repro Steps | Status |
|----|-------------|----------|-------------|--------|
| C1 | [Description] | [Location] | [Steps] | [Open/Fixed] |

### Major Bugs
| ID | Description | Location | Repro Steps | Status |
|----|-------------|----------|-------------|--------|
| M1 | [Description] | [Location] | [Steps] | [Open/Fixed] |

### Minor Bugs
| ID | Description | Location | Repro Steps | Status |
|----|-------------|----------|-------------|--------|
| m1 | [Description] | [Location] | [Steps] | [Open/Fixed] |

## Playtime Tracking

### First Playthrough
- **Start Time**: [Time]
- **End Time**: [Time]
- **Total Time**: [X] minutes
- **Objectives Completed**: [X]/8
- **Evidence Collected**: [X]/28
- **Deaths**: [X]

### Speedrun Attempt
- **Total Time**: [X] minutes
- **Objectives Completed**: [X]/8 (required only)

## Test Summary

### Overall Assessment
- **Functionality**: [ ] Pass [ ] Fail
- **Performance**: [ ] Pass [ ] Fail
- **Gameplay Experience**: [ ] Pass [ ] Fail
- **Audio/Visual Quality**: [ ] Pass [ ] Fail

### Critical Issues
1. [Issue 1]
2. [Issue 2]

### Recommendations
1. [Recommendation 1]
2. [Recommendation 2]

### Sign-Off
- **Ready for Next Phase**: [ ] Yes [ ] No
- **Tester Signature**: [Name]
- **Date**: [Date]

## Appendix

### A. Performance Graphs
[Include FPS graphs, memory usage, etc.]

### B. Screenshots
[Include screenshots of bugs or issues]

### C. Video Recordings
[Link to recorded playthrough]

### D. Detailed Notes
[Any additional observations]
