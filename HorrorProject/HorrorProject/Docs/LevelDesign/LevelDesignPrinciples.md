# Level Design Principles

## Core Philosophy

Horror game level design focuses on psychological tension, spatial storytelling, and player agency within constrained environments.

## 7 Beats Structure

### Beat 1: Arrival (0-5 minutes)
- **Purpose**: Establish atmosphere and baseline tension
- **Player State**: Curious, cautious
- **Design Goals**: 
  - Clear entry point
  - Environmental storytelling
  - Tutorial elements (if needed)
  - First objective hint

### Beat 2: Exploration (5-10 minutes)
- **Purpose**: Build familiarity with space
- **Player State**: Investigating, learning
- **Design Goals**:
  - Multiple paths to explore
  - Evidence discovery
  - Environmental hazards introduction
  - Objective 1-2 completion

### Beat 3: First Threat (10-15 minutes)
- **Purpose**: Introduce danger
- **Player State**: Alert, tense
- **Design Goals**:
  - First enemy encounter or major scare
  - Safe zones established
  - Objective 3 completion
  - Resource scarcity begins

### Beat 4: Rising Tension (15-22 minutes)
- **Purpose**: Escalate pressure
- **Player State**: Stressed, strategic
- **Design Goals**:
  - Multiple threats possible
  - Complex navigation required
  - Objectives 4-5 completion
  - Limited resources

### Beat 5: Crisis Point (22-28 minutes)
- **Purpose**: Peak tension
- **Player State**: Desperate, focused
- **Design Goals**:
  - Maximum threat density
  - Critical objective (6) completion
  - Difficult choices
  - Environmental hazards peak

### Beat 6: Resolution Path (28-33 minutes)
- **Purpose**: Provide escape route
- **Player State**: Determined, exhausted
- **Design Goals**:
  - Clear path to exit (if objectives met)
  - Objective 7 completion
  - Final challenges
  - Reward discovery

### Beat 7: Escape/Conclusion (33-35 minutes)
- **Purpose**: Release tension
- **Player State**: Relieved, accomplished
- **Design Goals**:
  - Objective 8 completion
  - Exit sequence
  - Story payoff
  - Performance evaluation

## Spatial Design Principles

### 1. Readability
- Clear visual hierarchy
- Distinct landmarks
- Consistent visual language
- Obvious vs. hidden paths

### 2. Flow
- Natural movement patterns
- Controlled pacing through architecture
- Bottlenecks for tension
- Open areas for relief

### 3. Layering
- Multiple routes (vertical and horizontal)
- Hidden areas for exploration
- Shortcuts unlock after progression
- Interconnected spaces

### 4. Tension Management
- **Safe Zones**: Well-lit, open, no threats
- **Danger Zones**: Dark, confined, enemy presence
- **Transition Zones**: Gradual shift between states
- **Ambiguous Zones**: Player uncertainty

### 5. Environmental Storytelling
- Visual narrative through props
- Lighting tells story
- Audio cues provide context
- Evidence placement reveals lore

## Objective Node Design

### 8 Objective Configuration

1. **Objective 1**: Tutorial/Introduction
   - Location: Near spawn
   - Difficulty: Easy
   - Purpose: Teach mechanics

2. **Objective 2**: Exploration Reward
   - Location: Off main path
   - Difficulty: Easy
   - Purpose: Encourage exploration

3. **Objective 3**: First Challenge
   - Location: After first threat
   - Difficulty: Medium
   - Purpose: Test learned skills

4. **Objective 4**: Resource Gate
   - Location: Mid-level
   - Difficulty: Medium
   - Purpose: Resource management

5. **Objective 5**: Navigation Challenge
   - Location: Complex area
   - Difficulty: Medium-Hard
   - Purpose: Spatial awareness

6. **Objective 6**: Crisis Objective
   - Location: Danger zone
   - Difficulty: Hard
   - Purpose: Peak tension moment

7. **Objective 7**: Pre-Exit Challenge
   - Location: Near exit path
   - Difficulty: Medium
   - Purpose: Final test

8. **Objective 8**: Exit Unlock
   - Location: Exit area
   - Difficulty: Easy
   - Purpose: Completion satisfaction

## Performance Budget

### Target Metrics (SM13 - 1300m²)

#### Draw Calls
- **Target**: < 2000 per frame
- **Maximum**: 2500 per frame
- **Optimization**: Instanced meshes, material merging

#### Triangle Count
- **Target**: < 2M visible triangles
- **Maximum**: 3M visible triangles
- **Optimization**: LODs, occlusion culling

#### Texture Memory
- **Target**: < 800MB
- **Maximum**: 1GB
- **Optimization**: Texture streaming, compression

#### Lighting
- **Static Lights**: Unlimited (baked)
- **Dynamic Lights**: < 10 visible
- **Shadow Casting Lights**: < 5 visible
- **Optimization**: Lightmaps, light functions

#### Audio
- **Simultaneous Sounds**: < 32
- **3D Sounds**: < 16
- **Reverb Zones**: < 4
- **Optimization**: Attenuation, priority system

#### Particles
- **Active Emitters**: < 20
- **Total Particles**: < 5000
- **Optimization**: GPU particles, LOD

#### AI/Gameplay
- **Active AI**: < 5
- **Physics Objects**: < 50
- **Optimization**: Tick optimization, pooling

### Frame Time Budget (60 FPS = 16.67ms)

- **Game Thread**: < 10ms
  - Tick: 3ms
  - Blueprint: 2ms
  - AI: 2ms
  - Physics: 2ms
  - Other: 1ms

- **Render Thread**: < 12ms
  - Base Pass: 4ms
  - Lighting: 3ms
  - Shadows: 2ms
  - Post Process: 2ms
  - Other: 1ms

- **GPU**: < 14ms
  - Geometry: 4ms
  - Lighting: 4ms
  - Shadows: 3ms
  - Post Process: 2ms
  - Other: 1ms

## Design Checklist

### Pre-Production
- [ ] 7 beats structure defined
- [ ] 8 objectives placed on paper
- [ ] Flow diagram created
- [ ] Performance budget allocated
- [ ] Reference materials gathered

### Blockout Phase
- [ ] Basic geometry placed
- [ ] Player spawn set
- [ ] Objective nodes placed
- [ ] Navigation tested
- [ ] Timing verified (35 min target)

### Art Pass
- [ ] Lighting implemented
- [ ] Props placed
- [ ] Materials applied
- [ ] VFX added
- [ ] Performance within budget

### Audio Pass
- [ ] Ambient audio placed
- [ ] Music triggers set
- [ ] SFX implemented
- [ ] Reverb zones configured
- [ ] Audio occlusion tested

### Polish Phase
- [ ] Evidence items placed
- [ ] Secrets hidden
- [ ] Performance optimized
- [ ] Playtesting completed
- [ ] Bugs fixed

### Final Validation
- [ ] All objectives completable
- [ ] No navigation blockers
- [ ] Performance stable
- [ ] Audio balanced
- [ ] Lighting polished

## Common Pitfalls

### 1. Over-Complexity
- **Problem**: Too many paths confuse players
- **Solution**: Clear main path with optional branches

### 2. Under-Lighting
- **Problem**: Players can't see anything
- **Solution**: Ensure minimum visibility while maintaining mood

### 3. Pacing Issues
- **Problem**: Tension never releases or builds too slowly
- **Solution**: Follow 7 beats structure strictly

### 4. Objective Clustering
- **Problem**: All objectives in one area
- **Solution**: Distribute across entire level

### 5. Performance Spikes
- **Problem**: Certain areas drop framerate
- **Solution**: Profile early, optimize continuously

### 6. Dead Ends
- **Problem**: Players get stuck with no feedback
- **Solution**: Always provide visual/audio hints

### 7. Scale Issues
- **Problem**: Spaces feel too large or cramped
- **Solution**: Use reference measurements, playtest

## Best Practices

1. **Playtest Early**: Test blockout before art pass
2. **Iterate Quickly**: Use modular assets for rapid changes
3. **Profile Often**: Check performance at each milestone
4. **Document Changes**: Keep design doc updated
5. **Respect Budget**: Stay within performance limits
6. **Player First**: Design for player experience, not aesthetics
7. **Tell Stories**: Every space should have narrative purpose
8. **Guide Subtly**: Use lighting and composition, not arrows
9. **Reward Exploration**: Hide secrets for curious players
10. **Test Accessibility**: Ensure all players can navigate

## Tools Reference

- **Blockout**: `GenerateLevelBlockout.ps1`
- **Objectives**: `PlaceObjectiveNodes.ps1`
- **Evidence**: `PlaceEvidenceItems.ps1`
- **Validation**: `ValidateLevelLayout.ps1`
- **Navigation**: `TestLevelNavigation.ps1`
- **Performance**: `TestPerformance.ps1`
- **Optimization**: `OptimizeLevelLighting.ps1`, `OptimizeLevelGeometry.ps1`

## Resources

- Level Design Document Template: `LevelDesignDocument.md`
- Test Plan Template: `LevelTestPlan.md`
- Optimization Plan Template: `LevelOptimizationPlan.md`
- SM13 Route Design: `SM13_RouteDesign.md`
