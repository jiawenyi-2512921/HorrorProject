# Niagara Best Practices Guide

## Overview

This guide establishes best practices for creating, implementing, and maintaining Niagara particle systems in the Horror Project. Following these practices ensures consistency, performance, and maintainability.

## Design Principles

### 1. Purpose-Driven Design

**Every particle system should have a clear purpose**:
- Atmospheric enhancement (dust, fog)
- Gameplay feedback (blood, debris)
- Environmental storytelling (drips, ambient)
- Visual polish (camera noise, light beams)

**Ask before creating**:
- What is this effect communicating?
- Is it necessary for gameplay or atmosphere?
- Can an existing effect be reused?
- What is the performance cost?

### 2. Less is More

**Subtlety over spectacle**:
- Horror relies on atmosphere, not explosions
- Subtle effects are more immersive
- Fewer particles with better quality > many low-quality particles
- Use restraint with emissive and bright effects

**Particle count guidelines**:
- Atmospheric: 100-200 particles
- Impact: 50-100 particles (burst)
- Ambient: 50-150 particles
- Special: 20-80 particles

### 3. Performance First

**Always consider performance**:
- Design within budgets from the start
- Profile early and often
- Optimize before adding features
- Test on target hardware

**Performance hierarchy**:
1. Gameplay (highest priority)
2. Core visuals (lighting, materials)
3. VFX (important but not critical)
4. Polish effects (lowest priority)

## Creation Best Practices

### 1. System Setup

**Naming Conventions**:
```
Systems: NS_[Category]_[Name]
Emitters: NE_[Name]Emitter
Materials: M_[Name]
Textures: T_[Name]_[Type]
```

**Organization**:
```
Content/VFX/
  ├── Niagara/
  │   ├── Atmospheric/
  │   ├── Impact/
  │   ├── Ambient/
  │   └── Special/
  ├── Materials/
  ├── Textures/
  └── Meshes/
```

**Version Control**:
- Use descriptive commit messages
- Don't commit broken effects
- Test before committing
- Document major changes

### 2. Emitter Configuration

**Start Simple**:
1. Create basic emitter with minimal modules
2. Test and verify it works
3. Add features incrementally
4. Profile after each addition
5. Remove unnecessary modules

**Module Order Matters**:
```
Spawn Modules:
  1. Emitter Properties
  2. Emitter Spawn
  3. Spawn Rate/Burst

Initialize Modules:
  1. Lifetime
  2. Color
  3. Size
  4. Rotation
  5. Mass
  6. Spawn Location
  7. Initial Velocity

Update Modules:
  1. Forces (Gravity, Drag)
  2. Curl Noise
  3. Collision
  4. Scale Over Life
  5. Color Over Life
  6. Rotation Rate
  7. Kill Conditions

Render Modules:
  1. Sprite/Mesh Renderer
  2. Sorting
  3. Camera Offset
```

**Avoid Common Mistakes**:
- Don't use auto-calculate bounds (expensive)
- Don't forget to set fixed bounds
- Don't use persistent IDs unless needed
- Don't enable features you don't use
- Don't use determinism unless required

### 3. Material Creation

**Material Checklist**:
- [ ] Use appropriate blend mode
- [ ] Implement depth fade
- [ ] Use compressed textures
- [ ] Minimize texture samples
- [ ] Expose key parameters
- [ ] Use unlit when possible
- [ ] Test in various lighting
- [ ] Verify performance cost

**Texture Guidelines**:
```
Resolution: 128x128 to 512x512
Format: BC1 (color), BC4 (grayscale), BC5 (normal)
Mip Maps: Always generate
Compression: Appropriate for content
Tiling: Consider for reusability
```

**Parameter Exposure**:
```
Always Expose:
  - Color/Tint
  - Opacity/Intensity
  - Size/Scale multipliers

Sometimes Expose:
  - Texture coordinates
  - Emissive strength
  - Depth fade distance

Rarely Expose:
  - Internal calculations
  - Debug parameters
```

### 4. LOD Implementation

**Always Implement LOD**:
- Every effect needs at least 2 LOD levels
- Test LOD transitions for smoothness
- Ensure no pop-in/pop-out
- Verify performance improvement

**LOD Strategy**:
```
LOD 0 (Near): Full quality, all features
LOD 1 (Medium): Reduced quality, essential features
LOD 2 (Far): Minimal quality, basic features only
```

**LOD Transition Distances**:
```
Small Effects (dust, drips): 1000, 2000
Medium Effects (blood, debris): 1500, 3000
Large Effects (fog, ambient): 2000, 4000
```

### 5. Collision Setup

**When to Use Collision**:
- Impact effects (blood, debris) - Always
- Dripping effects (water) - Always
- Atmospheric effects (dust, fog) - Rarely
- Ambient effects - Rarely

**Collision Type Selection**:
```
Scene Depth: Fast, approximate, GPU-based
  Use for: Atmospheric, soft particles

Line Trace: Accurate, expensive, CPU-based
  Use for: Impact effects, critical collision

Scene Depth + Trace: Most accurate, most expensive
  Use for: Hero effects only
```

**Collision Optimization**:
- Use smallest possible collision radius
- Minimize collision iterations (1-3)
- Use specific collision channels
- Disable collision at distance (LOD)
- Don't generate events unless needed

## Implementation Best Practices

### 1. Spawning Effects

**Pooling System**:
```cpp
// Good: Reuse components
UNiagaraComponent* GetPooledEffect(UNiagaraSystem* System)
{
    // Check pool for available component
    // Reuse if available, create if not
    // Reset parameters before returning
}

// Bad: Always create new
UNiagaraComponent* NewEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(...);
```

**Spawn Location**:
```cpp
// Good: Precise location
FVector SpawnLocation = HitResult.ImpactPoint;
FRotator SpawnRotation = HitResult.ImpactNormal.Rotation();

// Bad: Approximate location
FVector SpawnLocation = GetActorLocation();
```

**Auto-Destroy**:
```cpp
// Good: One-shot effects
SpawnSystemAtLocation(System, Location, Rotation, Scale, true); // Auto-destroy

// Good: Persistent effects
UNiagaraComponent* Component = SpawnSystemAttached(...);
// Manually destroy when done
Component->DestroyComponent();

// Bad: Memory leak
SpawnSystemAtLocation(System, Location, Rotation, Scale, false); // Never destroyed
```

### 2. Parameter Management

**Parameter Naming**:
```
Use descriptive names:
  - SpawnRate (not Rate)
  - ParticleColor (not Color)
  - ExplosionForce (not Force)

Use consistent prefixes:
  - b for booleans (bEnableEffect)
  - f for floats (fIntensity)
  - v for vectors (vDirection)
```

**Parameter Updates**:
```cpp
// Good: Batch updates
NiagaraComponent->SetFloatParameter("Intensity", 1.5f);
NiagaraComponent->SetVectorParameter("Color", FVector(1, 0, 0));
NiagaraComponent->SetBoolParameter("bEnabled", true);

// Bad: Updating every frame unnecessarily
void Tick(float DeltaTime)
{
    NiagaraComponent->SetFloatParameter("Time", GetGameTime()); // Expensive!
}
```

**Default Values**:
- Always set sensible defaults
- Document parameter ranges
- Validate parameter values
- Clamp to safe ranges

### 3. Lifecycle Management

**Activation**:
```cpp
// Good: Activate when needed
NiagaraComponent->Activate();

// Good: Deactivate when not needed
NiagaraComponent->Deactivate();

// Bad: Always active
// Wastes performance
```

**Cleanup**:
```cpp
// Good: Proper cleanup
if (NiagaraComponent && NiagaraComponent->IsActive())
{
    NiagaraComponent->Deactivate();
    NiagaraComponent->DestroyComponent();
}

// Bad: No cleanup
// Memory leak
```

**Reset**:
```cpp
// Good: Reset before reuse
NiagaraComponent->ResetSystem();
NiagaraComponent->SetFloatParameter("Intensity", DefaultIntensity);

// Bad: Reuse without reset
// Carries over old state
```

### 4. Performance Monitoring

**Regular Profiling**:
```
Daily: Quick check with stat particles
Weekly: Full profile with profilegpu
Monthly: Comprehensive performance review
Before Release: Full optimization pass
```

**Performance Metrics**:
```
Monitor:
  - Particle count (stat particles)
  - GPU time (profilegpu)
  - Memory usage (stat memory)
  - Draw calls (stat rhi)
  - Frame time (stat unit)
```

**Performance Budgets**:
```
Set budgets:
  - Per-effect particle count
  - Per-effect GPU time
  - Total particle count
  - Total GPU time

Enforce budgets:
  - Automated tests
  - Code reviews
  - Performance gates
```

## Testing Best Practices

### 1. Visual Testing

**Test Scenarios**:
- Spawn in various locations
- Test with different lighting
- View from multiple angles
- Test at various distances
- Test with other effects active

**Visual Checklist**:
- [ ] Colors match design
- [ ] Size appropriate
- [ ] Movement natural
- [ ] Fade in/out smooth
- [ ] No flickering
- [ ] No z-fighting
- [ ] Visible in all conditions
- [ ] Matches reference

### 2. Performance Testing

**Test Scenarios**:
- Single effect
- Multiple simultaneous effects
- Worst-case scenario (many effects)
- Different quality settings
- Different hardware tiers

**Performance Checklist**:
- [ ] Within particle budget
- [ ] Within GPU budget
- [ ] Within memory budget
- [ ] No frame drops
- [ ] LOD works correctly
- [ ] Culling works correctly
- [ ] Scalability works

### 3. Integration Testing

**Test Scenarios**:
- Spawn from gameplay code
- Attach to moving objects
- Dynamic parameter changes
- Level streaming
- Save/load game

**Integration Checklist**:
- [ ] Spawns correctly
- [ ] Attaches correctly
- [ ] Parameters work
- [ ] Cleanup works
- [ ] No crashes
- [ ] No memory leaks
- [ ] Works with streaming

## Maintenance Best Practices

### 1. Documentation

**Document Everything**:
- System purpose and usage
- Parameter descriptions and ranges
- Performance characteristics
- Known issues and limitations
- Update history

**Documentation Format**:
```markdown
# NS_EffectName

## Purpose
Brief description of what this effect does.

## Usage
How to spawn and use this effect.

## Parameters
- ParamName (Type, Range): Description

## Performance
- Particle Count: X
- GPU Cost: Xms
- Memory: XMB

## Notes
Any special considerations or known issues.
```

### 2. Version Control

**Commit Practices**:
```
Good commit message:
"Add blood splatter effect with collision and decals"

Bad commit message:
"Update particles"
```

**Branching Strategy**:
- Main: Stable, tested effects
- Dev: Work-in-progress effects
- Feature: New effect development
- Hotfix: Critical bug fixes

**Review Process**:
- Visual review (does it look good?)
- Performance review (within budget?)
- Code review (follows best practices?)
- Integration review (works in game?)

### 3. Optimization

**Continuous Optimization**:
- Profile regularly
- Optimize bottlenecks
- Remove unused features
- Update to new techniques
- Refactor when needed

**Optimization Priority**:
1. Effects over budget
2. Frequently used effects
3. Performance-critical effects
4. Rarely used effects

**Optimization Checklist**:
- [ ] Reduce spawn rate
- [ ] Implement/improve LOD
- [ ] Simplify materials
- [ ] Optimize collision
- [ ] Enable culling
- [ ] Reduce particle count
- [ ] Use GPU simulation
- [ ] Pool and reuse

## Common Pitfalls to Avoid

### 1. Over-Engineering

**Don't**:
- Add features "just in case"
- Create overly complex systems
- Expose every parameter
- Implement unused functionality

**Do**:
- Start simple, add as needed
- Keep systems focused
- Expose only necessary parameters
- Remove unused features

### 2. Ignoring Performance

**Don't**:
- Skip profiling
- Exceed budgets
- Ignore LOD
- Disable culling
- Use expensive features unnecessarily

**Do**:
- Profile early and often
- Stay within budgets
- Implement LOD always
- Enable culling always
- Use appropriate features

### 3. Poor Organization

**Don't**:
- Use generic names
- Mix unrelated assets
- Skip documentation
- Ignore naming conventions

**Do**:
- Use descriptive names
- Organize by category
- Document everything
- Follow naming conventions

### 4. Inadequate Testing

**Don't**:
- Test only in editor
- Skip performance testing
- Ignore edge cases
- Test only on high-end hardware

**Do**:
- Test in-game
- Profile performance
- Test edge cases
- Test on target hardware

### 5. Neglecting Cleanup

**Don't**:
- Leave effects running
- Forget to destroy components
- Ignore memory leaks
- Skip deactivation

**Do**:
- Deactivate when done
- Destroy components properly
- Monitor memory usage
- Clean up thoroughly

## Quality Standards

### Visual Quality

**Minimum Standards**:
- Smooth fade in/out
- Natural movement
- Appropriate colors
- Correct size/scale
- No flickering
- No z-fighting
- Proper depth sorting

**High Quality**:
- Subtle details
- Realistic physics
- Proper lighting interaction
- Smooth LOD transitions
- Polished appearance

### Performance Quality

**Minimum Standards**:
- Within particle budget
- Within GPU budget
- Within memory budget
- 60 FPS on target hardware
- LOD implemented
- Culling enabled

**High Quality**:
- Optimized spawn rates
- Efficient materials
- Minimal overdraw
- Proper scalability
- Pooling implemented

### Code Quality

**Minimum Standards**:
- Follows naming conventions
- Properly documented
- No memory leaks
- No crashes
- Tested in-game

**High Quality**:
- Clean, readable code
- Comprehensive documentation
- Robust error handling
- Extensive testing
- Peer reviewed

## Workflow Recommendations

### 1. Effect Creation Workflow

```
1. Design Phase
   - Define purpose and requirements
   - Sketch/reference gathering
   - Set performance budget
   - Plan implementation

2. Prototype Phase
   - Create basic emitter
   - Test core functionality
   - Iterate on visuals
   - Profile performance

3. Implementation Phase
   - Add all features
   - Implement LOD
   - Enable culling
   - Optimize materials

4. Polish Phase
   - Fine-tune parameters
   - Add subtle details
   - Optimize performance
   - Final testing

5. Integration Phase
   - Integrate with gameplay
   - Test in context
   - Fix issues
   - Document usage

6. Release Phase
   - Final performance check
   - Code review
   - Documentation review
   - Commit to main
```

### 2. Optimization Workflow

```
1. Profile
   - Measure current performance
   - Identify bottlenecks
   - Set optimization goals

2. Analyze
   - Determine root causes
   - Prioritize issues
   - Plan optimizations

3. Optimize
   - Implement optimizations
   - Test each change
   - Verify improvements

4. Verify
   - Re-profile
   - Compare before/after
   - Ensure no regressions

5. Document
   - Record optimizations
   - Update documentation
   - Share learnings
```

### 3. Review Workflow

```
1. Self-Review
   - Visual check
   - Performance check
   - Code check
   - Documentation check

2. Peer Review
   - Visual review
   - Performance review
   - Code review
   - Integration review

3. Lead Review
   - Approve or request changes
   - Provide feedback
   - Verify standards met

4. Final Check
   - Address feedback
   - Re-test
   - Update documentation
   - Commit
```

## Team Collaboration

### 1. Communication

**Share Information**:
- Document new techniques
- Share optimization tips
- Report issues promptly
- Ask for help when needed

**Regular Meetings**:
- Weekly VFX review
- Performance check-ins
- Knowledge sharing sessions
- Problem-solving discussions

### 2. Standards Enforcement

**Code Reviews**:
- All effects reviewed before merge
- Check for best practices
- Verify performance
- Ensure documentation

**Automated Checks**:
- Performance budgets
- Naming conventions
- Required documentation
- Test coverage

### 3. Knowledge Sharing

**Documentation**:
- Maintain this guide
- Document new techniques
- Share case studies
- Create tutorials

**Training**:
- Onboard new team members
- Share best practices
- Conduct workshops
- Provide mentorship

## Continuous Improvement

### 1. Stay Updated

**Engine Updates**:
- Review Unreal Engine release notes
- Test new Niagara features
- Adopt beneficial improvements
- Update workflows

**Industry Trends**:
- Study other games
- Follow VFX artists
- Attend conferences
- Read technical articles

### 2. Iterate and Improve

**Regular Reviews**:
- Review existing effects
- Identify improvements
- Update old effects
- Refactor when beneficial

**Feedback Loop**:
- Gather team feedback
- Listen to player feedback
- Analyze performance data
- Implement improvements

### 3. Measure Success

**Metrics**:
- Performance targets met
- Visual quality achieved
- Team productivity
- Player satisfaction

**Goals**:
- Maintain 60 FPS
- Stay within budgets
- Achieve visual targets
- Deliver on time

## Summary

**Core Principles**:
1. Purpose-driven design
2. Performance first
3. Less is more
4. Test thoroughly
5. Document everything
6. Optimize continuously
7. Collaborate effectively
8. Improve constantly

**Key Takeaways**:
- Always implement LOD and culling
- Profile early and often
- Stay within budgets
- Follow naming conventions
- Document thoroughly
- Test on target hardware
- Clean up properly
- Share knowledge

**Success Criteria**:
- Visual quality meets standards
- Performance within budgets
- Code quality high
- Documentation complete
- Team collaboration effective
- Continuous improvement
