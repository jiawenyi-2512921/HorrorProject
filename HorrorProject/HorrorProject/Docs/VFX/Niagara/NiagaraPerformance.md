# Niagara Performance Optimization Guide

## Overview

This guide provides comprehensive performance optimization strategies for Niagara particle systems in the Horror Project. Target: 60 FPS on mid-range hardware with all VFX active.

## Performance Budgets

### Global Particle Budget

**Total Concurrent Particles**: 1000 maximum
- Atmospheric Effects: 400 particles (40%)
- Impact Effects: 300 particles (30%)
- Ambient Effects: 200 particles (20%)
- Special Effects: 100 particles (10%)

**GPU Time Budget**: 3.0ms total @ 1080p
- Post-Process Effects: 1.5ms
- Particle Rendering: 1.0ms
- Particle Simulation: 0.5ms

**Memory Budget**: 50MB total
- Textures: 30MB
- Particle Systems: 15MB
- Runtime Data: 5MB

### Per-Effect Budgets

| Effect Type | Max Particles | GPU Cost | Memory | Draw Calls |
|-------------|---------------|----------|--------|------------|
| Dust Particles | 150 | 0.3ms | 2MB | 1 |
| Fog Volume | 200 | 0.8ms | 4MB | 1 |
| Flashlight Beam | 80 | 0.4ms | 2MB | 1 |
| Camera Noise | 300 | 0.2ms | 1MB | 1 |
| Blood Splatter | 100 | 0.5ms | 3MB | 2 |
| Stone Debris | 120 | 0.6ms | 4MB | 2 |
| Water Drips | 50 | 0.2ms | 1MB | 1 |
| Ambient Particles | 200 | 0.4ms | 2MB | 1 |

## Optimization Strategies

### 1. Simulation Target Selection

**CPU Simulation (CPUSim)**:
- Use for: < 200 particles
- Pros: Better collision, precise control
- Cons: CPU overhead, limited scalability
- Best for: Impact effects, drips, debris

**GPU Simulation (GPUSim)**:
- Use for: > 200 particles
- Pros: Massive scalability, parallel processing
- Cons: Limited collision, less precise
- Best for: Fog, dust, ambient particles

**Selection Guide**:
```
Particle Count < 100: CPUSim
Particle Count 100-200: CPUSim (if collision needed) or GPUSim
Particle Count > 200: GPUSim
```

### 2. Spawn Rate Optimization

**Dynamic Spawn Rate Scaling**:
```cpp
// Scale spawn rate based on distance
float Distance = GetDistanceToCamera();
float SpawnRateMultiplier = FMath::Clamp(1.0f - (Distance / MaxDistance), 0.3f, 1.0f);
float AdjustedSpawnRate = BaseSpawnRate * SpawnRateMultiplier;
```

**Burst vs Continuous**:
- Burst: One-time effects (blood, debris) - more efficient
- Continuous: Ambient effects (dust, fog) - use spawn rate limits

**Spawn Rate Guidelines**:
- Atmospheric: 20-50/second
- Ambient: 30-60/second
- Impact (burst): 50-150 total
- Drips: 1-5/second

### 3. LOD (Level of Detail) Configuration

**Distance-Based LOD**:

**LOD 0 (Near)**: 0-1500 units
```
Spawn Rate: 100%
Max Particles: Full budget
Update Rate: 60 FPS
Quality: High
Features: All enabled
```

**LOD 1 (Medium)**: 1500-3000 units
```
Spawn Rate: 70%
Max Particles: 60% budget
Update Rate: 30 FPS
Quality: Medium
Features: Essential only
```

**LOD 2 (Far)**: 3000+ units
```
Spawn Rate: 50%
Max Particles: 30% budget
Update Rate: 15 FPS
Quality: Low
Features: Minimal
```

**LOD Implementation**:
```cpp
// Niagara System LOD Settings
LOD Distance 0: 0
LOD Distance 1: 1500
LOD Distance 2: 3000
Max LOD Level: 2
```

### 4. Culling Optimization

**Distance Culling**:
```
Max Draw Distance: 2500-3500 units (effect-dependent)
Cull Proxy Mode: None
```

**Visibility Culling**:
```
Cull When Not Rendered: True
Cull Delay: 1.0-3.0 seconds
```

**Bounds Optimization**:
```
Use Fixed Bounds: True
Fixed Bounds: Tight to visible area
Auto-Calculate Bounds: False (performance cost)
```

**Frustum Culling**:
- Automatic by engine
- Ensure bounds are accurate
- Use fixed bounds for predictable effects

### 5. Collision Optimization

**Collision Type Selection**:

**Scene Depth Buffer** (Fastest):
- GPU-based
- No physics queries
- Approximate collision
- Use for: Atmospheric effects, soft particles

**Line Trace** (Moderate):
- CPU-based
- Accurate collision
- Physics queries
- Use for: Impact effects, debris

**Scene Depth + Trace** (Slowest):
- Hybrid approach
- Most accurate
- Highest cost
- Use for: Critical effects only

**Collision Guidelines**:
```
Atmospheric Effects: Scene Depth or None
Impact Effects: Scene Depth + Trace
Ambient Effects: Scene Depth or None
Drips: Scene Depth + Trace (kill on collision)
```

**Collision Optimization**:
```
Max Collision Iterations: 1-3 (lower is faster)
Collision Radius: As small as possible
Collision Channel: Use specific channels, not "All"
Generate Events: Only if needed
```

### 6. Material Optimization

**Texture Optimization**:
```
Resolution: 128x128 to 512x512 (no larger)
Format: BC1 (opaque), BC4 (grayscale), BC5 (normals)
Mip Maps: Always generate
Compression: Appropriate for content type
```

**Shader Complexity**:
- Unlit: Fastest (use for most particles)
- Default Lit: Moderate (use for mesh particles)
- Avoid: Complex lighting, multiple texture samples

**Material Features to Avoid**:
- Refraction (expensive)
- Pixel Depth Offset (expensive)
- Complex math operations
- Multiple texture lookups
- Dynamic parameters (use sparingly)

**Depth Fade Optimization**:
```cpp
// Efficient depth fade
float DepthFade = saturate((SceneDepth - PixelDepth) / FadeDistance);
Opacity *= DepthFade;
```

### 7. Render Optimization

**Blend Mode Selection**:
- Translucent: Most common, moderate cost
- Additive: Cheaper than translucent, use for glows
- Opaque: Cheapest, use for mesh particles

**Overdraw Reduction**:
- Keep particle size reasonable
- Use alpha to reduce visible area
- Avoid overlapping particles
- Use depth sorting when needed

**Sort Priority**:
```
Screen Space Effects: 100 (render last)
Atmospheric Effects: 10-20
Impact Effects: 0 (default)
Ambient Effects: 5-15
```

**Render Settings**:
```
Two Sided: Only if necessary
Disable Depth Test: Rarely (screen space only)
Responsive AA: True (better quality, minimal cost)
Cast Shadows: False (particles rarely need shadows)
```

### 8. Update Rate Optimization

**Fixed Update Rate**:
```
High Priority Effects: 60 FPS
Medium Priority Effects: 30 FPS
Low Priority Effects: 15 FPS
Background Effects: 10 FPS
```

**Dynamic Update Rate**:
```cpp
// Scale update rate based on distance
float Distance = GetDistanceToCamera();
if (Distance < 1000) UpdateRate = 60;
else if (Distance < 2000) UpdateRate = 30;
else UpdateRate = 15;
```

### 9. Force Module Optimization

**Curl Noise Optimization**:
```
Octaves: 1-2 (more is expensive)
Quality: Low-Medium
Frequency: 0.2-0.5 (lower is cheaper)
```

**Drag Optimization**:
- Use constant drag (cheaper than velocity-based)
- Avoid per-particle drag calculations

**Gravity Optimization**:
- Use constant gravity direction
- Avoid per-particle gravity calculations

**Force Priority**:
1. Gravity (cheap)
2. Drag (cheap)
3. Curl Noise (moderate)
4. Point Attraction (expensive)
5. Vortex Forces (expensive)

### 10. Memory Optimization

**Texture Memory**:
```
Use Texture Atlases: Combine multiple textures
Share Textures: Reuse across effects
Compression: Always use appropriate compression
Mip Maps: Generate for all textures
Streaming: Enable texture streaming
```

**Particle Data**:
```
Minimize Attributes: Only use necessary attributes
Use Appropriate Types: Float vs Vector vs Int
Avoid Persistent IDs: Unless required
Pool Systems: Reuse Niagara components
```

**Asset Loading**:
```
Async Loading: Load particle systems asynchronously
Preload Critical: Preload frequently used effects
Unload Unused: Unload when not needed
```

## Scalability Settings

### Quality Level Configuration

**Low (Performance Mode)**:
```
Spawn Rate Multiplier: 0.5
Max Particles Multiplier: 0.5
Update Rate: 30 FPS
Collision: Disabled or Depth Only
Curl Noise: Disabled or 1 Octave
Shadows: Disabled
LOD Bias: +1 (use lower LOD sooner)
Max Draw Distance: -30%
```

**Medium (Balanced Mode)**:
```
Spawn Rate Multiplier: 0.7
Max Particles Multiplier: 0.7
Update Rate: 60 FPS
Collision: Depth Only
Curl Noise: 1-2 Octaves
Shadows: Disabled
LOD Bias: 0
Max Draw Distance: 100%
```

**High (Quality Mode)**:
```
Spawn Rate Multiplier: 1.0
Max Particles Multiplier: 1.0
Update Rate: 60 FPS
Collision: Full Trace
Curl Noise: 2-3 Octaves
Shadows: Enabled (if applicable)
LOD Bias: 0
Max Draw Distance: 100%
```

**Epic (Maximum Quality)**:
```
Spawn Rate Multiplier: 1.0
Max Particles Multiplier: 1.0
Update Rate: 60 FPS
Collision: Full Trace + Events
Curl Noise: 3+ Octaves
Shadows: Enabled
LOD Bias: -1 (use higher LOD longer)
Max Draw Distance: +30%
```

**Cinematic (Offline Rendering)**:
```
Spawn Rate Multiplier: 1.5
Max Particles Multiplier: 1.5
Update Rate: 60 FPS
Collision: Full Trace + Events
Curl Noise: Highest Quality
Shadows: Enabled
LOD Bias: -2
Max Draw Distance: +50%
```

### Scalability Implementation

**Blueprint Example**:
```cpp
// Get current scalability level
int32 EffectsQuality = GetEffectsQualityLevel();

// Apply scalability
switch (EffectsQuality)
{
    case 0: // Low
        NiagaraComponent->SetFloatParameter("SpawnRateMultiplier", 0.5f);
        NiagaraComponent->SetIntParameter("MaxParticles", 50);
        break;
    case 1: // Medium
        NiagaraComponent->SetFloatParameter("SpawnRateMultiplier", 0.7f);
        NiagaraComponent->SetIntParameter("MaxParticles", 70);
        break;
    case 2: // High
        NiagaraComponent->SetFloatParameter("SpawnRateMultiplier", 1.0f);
        NiagaraComponent->SetIntParameter("MaxParticles", 100);
        break;
    case 3: // Epic
        NiagaraComponent->SetFloatParameter("SpawnRateMultiplier", 1.0f);
        NiagaraComponent->SetIntParameter("MaxParticles", 100);
        break;
}
```

## Profiling and Monitoring

### Console Commands

**Particle Statistics**:
```
stat particles - Show particle count and memory
stat niagara - Show Niagara-specific stats
stat gpu - Show GPU timing
profilegpu - Detailed GPU profiling
stat memory - Memory usage
```

**Visualization**:
```
viewmode shadercomplexity - Show shader cost
viewmode lightcomplexity - Show lighting cost
showflag.particles 0/1 - Toggle particles
showflag.fog 0/1 - Toggle fog
```

**Performance Capture**:
```
stat startfile - Start performance capture
stat stopfile - Stop performance capture
```

### Performance Targets

**Frame Time Budget** (60 FPS = 16.67ms):
```
Total Frame: 16.67ms
  - Game Logic: 5.0ms
  - Rendering: 8.0ms
    - VFX: 3.0ms (18% of frame)
      - Particles: 1.5ms
      - Post-Process: 1.5ms
  - Physics: 2.0ms
  - Audio: 1.0ms
  - Other: 0.67ms
```

**GPU Targets**:
```
Total VFX: < 3.0ms @ 1080p
Individual Effect: < 1.0ms @ 1080p
Particle System: < 0.5ms @ 1080p
```

**Memory Targets**:
```
Total VFX: < 50MB
Textures: < 30MB
Systems: < 15MB
Runtime: < 5MB
```

### Profiling Workflow

1. **Baseline Measurement**:
   - Run `stat particles` and `stat gpu`
   - Record particle count and GPU time
   - Note memory usage with `stat memory`

2. **Identify Bottlenecks**:
   - Use `profilegpu` to find expensive effects
   - Check `viewmode shadercomplexity` for overdraw
   - Monitor particle count spikes

3. **Optimize**:
   - Reduce spawn rates
   - Implement LOD
   - Simplify materials
   - Enable culling

4. **Verify**:
   - Re-run profiling
   - Compare before/after
   - Test on target hardware

## Common Performance Issues

### Issue 1: Too Many Particles

**Symptoms**:
- High particle count (> 1000)
- Frame rate drops
- `stat particles` shows high numbers

**Solutions**:
- Reduce spawn rates
- Implement aggressive LOD
- Enable distance culling
- Use burst instead of continuous spawn

### Issue 2: Expensive Materials

**Symptoms**:
- High shader complexity
- GPU time spikes
- `profilegpu` shows material cost

**Solutions**:
- Simplify material graphs
- Reduce texture samples
- Use unlit shading
- Optimize depth fade

### Issue 3: Collision Overhead

**Symptoms**:
- CPU spikes
- Physics cost high
- `stat physics` shows high numbers

**Solutions**:
- Use scene depth instead of trace
- Reduce collision iterations
- Disable collision at distance
- Use simpler collision shapes

### Issue 4: Overdraw

**Symptoms**:
- High GPU cost
- `viewmode shadercomplexity` shows red
- Translucency cost high

**Solutions**:
- Reduce particle size
- Use tighter alpha masks
- Reduce particle density
- Use additive blend mode

### Issue 5: Memory Leaks

**Symptoms**:
- Memory usage increases over time
- `stat memory` shows growth
- Crashes after extended play

**Solutions**:
- Enable auto-destroy
- Manually destroy finished effects
- Pool and reuse components
- Check for orphaned particles

## Optimization Checklist

### Pre-Production
- [ ] Define particle budgets
- [ ] Set performance targets
- [ ] Create optimization guidelines
- [ ] Set up profiling workflow

### Production
- [ ] Implement LOD for all effects
- [ ] Enable distance culling
- [ ] Use appropriate simulation target
- [ ] Optimize materials
- [ ] Minimize collision usage
- [ ] Use fixed bounds
- [ ] Implement scalability settings

### Testing
- [ ] Profile on target hardware
- [ ] Test all scalability levels
- [ ] Verify particle budgets
- [ ] Check memory usage
- [ ] Test multiple simultaneous effects
- [ ] Verify cleanup works

### Optimization
- [ ] Reduce spawn rates if needed
- [ ] Simplify expensive materials
- [ ] Optimize collision settings
- [ ] Reduce overdraw
- [ ] Implement pooling
- [ ] Optimize texture memory

## Platform-Specific Optimization

### PC (Mid-Range)
- Target: 60 FPS @ 1080p
- Particle Budget: 1000
- GPU Budget: 3.0ms
- Focus: Balanced quality/performance

### PC (Low-End)
- Target: 30-60 FPS @ 1080p
- Particle Budget: 500
- GPU Budget: 2.0ms
- Focus: Performance over quality

### PC (High-End)
- Target: 60+ FPS @ 1440p/4K
- Particle Budget: 1500
- GPU Budget: 4.0ms
- Focus: Maximum quality

## Best Practices Summary

1. **Always implement LOD** - Distance-based quality reduction
2. **Use appropriate simulation target** - CPU for < 200, GPU for > 200
3. **Enable culling** - Distance and visibility culling
4. **Optimize materials** - Unlit, simple shaders, compressed textures
5. **Minimize collision** - Use scene depth, avoid trace when possible
6. **Respect budgets** - Stay within particle and GPU budgets
7. **Implement scalability** - Support all quality levels
8. **Profile regularly** - Test on target hardware frequently
9. **Pool effects** - Reuse components instead of creating new
10. **Clean up properly** - Ensure effects are destroyed when done
