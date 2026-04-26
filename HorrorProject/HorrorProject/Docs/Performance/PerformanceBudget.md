# Performance Budget Allocation - HorrorProject

## Overview

Detailed performance budget allocation for 60 FPS @ Epic quality targeting Windows PC with Lumen + VSM + Ray Tracing.

---

## 1. Frame Budget Overview

### Total Frame Budget: 16.67ms (60 FPS)

```
Frame Budget Distribution:
┌─────────────────────────────────────────────────────────┐
│ Game Thread:    10.0ms (60%)  ████████████████████████  │
│ Render Thread:  12.0ms (72%)  ██████████████████████████│
│ GPU:            14.0ms (84%)  ████████████████████████  │
└─────────────────────────────────────────────────────────┘

Note: Threads run in parallel, GPU is the bottleneck
```

---

## 2. Game Thread Budget: 10ms

### Breakdown

| System | Budget | % of Game Thread | Priority |
|--------|--------|------------------|----------|
| Gameplay Logic | 1.0ms | 10% | HIGH |
| Animation | 2.0ms | 20% | HIGH |
| Physics | 1.0ms | 10% | MEDIUM |
| AI | 2.0ms | 20% | HIGH |
| Audio | 0.5ms | 5% | LOW |
| UI | 0.5ms | 5% | LOW |
| Networking | 0.5ms | 5% | N/A |
| Misc/Buffer | 2.5ms | 25% | - |
| **TOTAL** | **10.0ms** | **100%** | - |

### Detailed Allocation

#### Gameplay Logic: 1.0ms
```
Player Components:
├── FearComponent: 0.05ms (timer-based)
├── NoiseGenerator: 0.05ms (event-driven)
├── CameraRecording: 0.20ms (when active)
├── Battery: 0.02ms (timer-based)
├── Flashlight: 0.05ms (timer-based)
└── Other: 0.13ms

Game Systems:
├── Evidence System: 0.10ms
├── Event Bus: 0.10ms
├── Save System: 0.05ms
├── Encounter Director: 0.15ms
└── Other: 0.10ms

Total: 1.0ms
```

#### Animation: 2.0ms
```
Skeletal Meshes:
├── Player Character: 0.50ms
├── AI Characters (3): 1.20ms (0.4ms each)
├── Blend Spaces: 0.15ms
├── IK Systems: 0.10ms
└── Animation Notifies: 0.05ms

Total: 2.0ms
```

#### Physics: 1.0ms
```
Physics Simulation:
├── Character Movement: 0.30ms
├── Rigid Bodies: 0.20ms
├── Collision Detection: 0.30ms
├── Overlap Events: 0.10ms
└── Traces/Sweeps: 0.10ms

Total: 1.0ms
```

#### AI: 2.0ms
```
AI Systems:
├── Behavior Trees: 0.80ms (3 enemies)
├── Pathfinding: 0.40ms
├── Perception: 0.30ms
├── EQS Queries: 0.20ms
├── State Machines: 0.20ms
└── Other: 0.10ms

Total: 2.0ms
```

---

## 3. Render Thread Budget: 12ms

### Breakdown

| System | Budget | % of Render Thread | Priority |
|--------|--------|-------------------|----------|
| Scene Traversal | 2.0ms | 17% | HIGH |
| Visibility Culling | 1.5ms | 13% | HIGH |
| Draw Call Submission | 3.0ms | 25% | HIGH |
| Shadow Setup | 1.5ms | 13% | MEDIUM |
| Material Setup | 1.0ms | 8% | MEDIUM |
| Misc/Buffer | 3.0ms | 25% | - |
| **TOTAL** | **12.0ms** | **100%** | - |

### Detailed Allocation

#### Scene Traversal: 2.0ms
```
Scene Management:
├── Octree Traversal: 0.80ms
├── Primitive Gathering: 0.60ms
├── LOD Selection: 0.40ms
└── Bounds Checking: 0.20ms

Total: 2.0ms
```

#### Visibility Culling: 1.5ms
```
Culling Systems:
├── Frustum Culling: 0.50ms
├── Occlusion Culling: 0.60ms
├── Distance Culling: 0.20ms
└── Precomputed Visibility: 0.20ms

Total: 1.5ms
```

#### Draw Call Submission: 3.0ms
```
Draw Submission:
├── Static Meshes: 1.50ms (2000 calls)
├── Skeletal Meshes: 0.50ms (200 calls)
├── Particles: 0.30ms (100 calls)
├── Decals: 0.20ms (200 calls)
├── Translucency: 0.30ms (100 calls)
└── UI: 0.20ms (100 calls)

Total: 3.0ms (2700 draw calls)
```

---

## 4. GPU Budget: 14ms

### Breakdown

| System | Budget | % of GPU | Priority |
|--------|--------|----------|----------|
| Base Pass | 2.0ms | 14% | HIGH |
| Lumen GI | 8.0ms | 57% | HIGH |
| VSM Shadows | 3.0ms | 21% | HIGH |
| Post-Process | 1.0ms | 7% | MEDIUM |
| Misc/Buffer | 0.0ms | 0% | - |
| **TOTAL** | **14.0ms** | **100%** | - |

### Detailed Allocation

#### Base Pass: 2.0ms
```
Geometry Rendering:
├── Opaque Meshes: 1.20ms
│   ├── Nanite Rendering: 0.60ms
│   ├── Traditional Meshes: 0.40ms
│   └── Instanced Meshes: 0.20ms
├── Masked Materials: 0.30ms
├── Depth Prepass: 0.30ms
└── Velocity Pass: 0.20ms

Total: 2.0ms
```

#### Lumen GI: 8.0ms
```
Lumen Components:
├── Screen Probe Gather: 4.00ms
│   ├── Screen Traces: 1.50ms
│   ├── Mesh SDF Traces: 1.50ms
│   └── Radiance Cache: 1.00ms
├── Reflections: 2.50ms
│   ├── Screen Traces: 1.00ms
│   ├── Hit Lighting: 1.00ms
│   └── Filtering: 0.50ms
├── Surface Cache Update: 1.00ms
└── Scene Lighting: 0.50ms

Total: 8.0ms
```

#### VSM Shadows: 3.0ms
```
Virtual Shadow Maps:
├── Page Allocation: 0.30ms
├── Page Rendering: 1.50ms
│   ├── Directional Light: 1.00ms
│   └── Local Lights: 0.50ms
├── SMRT Filtering: 0.80ms
├── Cache Management: 0.20ms
└── Compositing: 0.20ms

Total: 3.0ms
```

#### Post-Process: 1.0ms
```
Post-Process Effects:
├── TSR (Temporal Super Resolution): 0.40ms
├── Motion Blur: 0.15ms
├── Depth of Field: 0.15ms
├── Bloom: 0.10ms
├── Tone Mapping: 0.05ms
├── Color Grading: 0.05ms
└── Other: 0.10ms

Total: 1.0ms
```

---

## 5. Memory Budget: 4GB

### Breakdown

| Category | Budget | % of Total | Priority |
|----------|--------|------------|----------|
| Textures | 1500 MB | 37.5% | HIGH |
| Meshes | 800 MB | 20.0% | HIGH |
| Audio | 400 MB | 10.0% | MEDIUM |
| Animation | 300 MB | 7.5% | MEDIUM |
| Code/Engine | 600 MB | 15.0% | - |
| Misc/Buffer | 400 MB | 10.0% | - |
| **TOTAL** | **4000 MB** | **100%** | - |

### Detailed Allocation

#### Textures: 1500 MB
```
Texture Memory:
├── Environment Textures: 600 MB
│   ├── Albedo: 250 MB
│   ├── Normal: 200 MB
│   ├── Material: 100 MB
│   └── Other: 50 MB
├── Character Textures: 200 MB
├── Props Textures: 300 MB
├── VFX Textures: 100 MB
├── UI Textures: 100 MB
└── Streaming Pool: 200 MB

Total: 1500 MB
```

#### Meshes: 800 MB
```
Mesh Memory:
├── Environment Meshes: 400 MB
│   ├── Nanite Meshes: 250 MB
│   ├── Traditional Meshes: 150 MB
├── Character Meshes: 150 MB
├── Props: 200 MB
└── Collision: 50 MB

Total: 800 MB
```

#### Audio: 400 MB
```
Audio Memory:
├── Streaming Audio: 100 MB (music, ambience)
├── Loaded SFX: 200 MB
├── Voice Lines: 50 MB
└── Audio Buffers: 50 MB

Total: 400 MB
```

---

## 6. Draw Call Budget: 3000

### Breakdown

| Category | Budget | % of Total | Notes |
|----------|--------|------------|-------|
| Environment | 1500 | 50% | Static meshes |
| Props | 600 | 20% | Instanced where possible |
| Characters | 200 | 7% | Skeletal meshes |
| Particles | 100 | 3% | GPU particles |
| Decals | 200 | 7% | Deferred decals |
| Translucency | 100 | 3% | Sorted back-to-front |
| UI | 100 | 3% | Batched |
| Misc | 200 | 7% | Lights, shadows, etc. |
| **TOTAL** | **3000** | **100%** | - |

### Optimization Strategies

1. **Instanced Static Meshes**: Reduce 1000+ draw calls
2. **Material Atlasing**: Reduce 500+ draw calls
3. **Mesh Merging**: Reduce 300+ draw calls
4. **Nanite**: Automatic clustering, massive reduction

---

## 7. Triangle Budget: 5M

### Breakdown

| Category | Budget | % of Total | Notes |
|----------|--------|------------|-------|
| Environment | 3.0M | 60% | Use Nanite |
| Props | 1.0M | 20% | LODs required |
| Characters | 0.5M | 10% | 50K per character |
| Particles | 0.1M | 2% | Simple geometry |
| UI | 0.05M | 1% | Minimal |
| Misc | 0.35M | 7% | Buffer |
| **TOTAL** | **5.0M** | **100%** | - |

### LOD Strategy

```
LOD Distances:
├── LOD 0 (100%): 0-5m
├── LOD 1 (50%): 5-15m
├── LOD 2 (25%): 15-30m
└── LOD 3 (10%): 30m+

Nanite: Automatic LOD streaming
```

---

## 8. Bandwidth Budget

### GPU Memory Bandwidth

**Target**: <200 GB/s (typical for mid-high end GPU)

```
Bandwidth Usage:
├── Texture Sampling: 80 GB/s (40%)
├── Render Target Writes: 60 GB/s (30%)
├── Depth/Stencil: 30 GB/s (15%)
├── Compute Shaders: 20 GB/s (10%)
└── Misc: 10 GB/s (5%)

Total: 200 GB/s
```

### Optimization

- Use texture compression (BC7, BC5)
- Reduce render target resolution
- Use half-precision where possible
- Minimize overdraw

---

## 9. System-Specific Budgets

### Lumen Budget: 8ms

```
Lumen Breakdown:
├── Screen Probe Gather: 4.0ms (50%)
│   ├── Probe Placement: 0.5ms
│   ├── Trace Generation: 1.5ms
│   ├── Radiance Cache: 1.0ms
│   └── Integration: 1.0ms
│
├── Reflections: 2.5ms (31%)
│   ├── Trace Generation: 1.0ms
│   ├── Hit Lighting: 1.0ms
│   └── Temporal Filter: 0.5ms
│
├── Surface Cache: 1.0ms (13%)
│   ├── Card Capture: 0.6ms
│   └── Update: 0.4ms
│
└── Final Gather: 0.5ms (6%)

Total: 8.0ms
```

### VSM Budget: 3ms

```
VSM Breakdown:
├── Page Management: 0.5ms (17%)
│   ├── Allocation: 0.3ms
│   └── Invalidation: 0.2ms
│
├── Shadow Rendering: 1.5ms (50%)
│   ├── Directional: 1.0ms
│   └── Local: 0.5ms
│
├── Filtering (SMRT): 0.8ms (27%)
│
└── Compositing: 0.2ms (6%)

Total: 3.0ms
```

---

## 10. Budget Monitoring

### Performance Metrics to Track

```cpp
// Key metrics to monitor
struct FPerformanceMetrics
{
    // Frame times
    float GameThreadMs;
    float RenderThreadMs;
    float GPUMs;
    
    // Draw stats
    int32 DrawCalls;
    int32 Triangles;
    int32 Instances;
    
    // Memory
    float TextureMemoryMB;
    float MeshMemoryMB;
    float TotalMemoryMB;
    
    // Lumen
    float LumenGIMs;
    float LumenReflectionsMs;
    
    // Shadows
    float VSMMs;
    
    // Post-process
    float PostProcessMs;
};
```

### Warning Thresholds

```cpp
// Budget exceeded warnings
GameThread > 10ms: WARNING
RenderThread > 12ms: WARNING
GPU > 14ms: WARNING
DrawCalls > 3000: WARNING
Triangles > 5M: WARNING
Memory > 4GB: WARNING
```

---

## 11. Scalability Targets

### Quality Levels

**Epic (Target)**:
- 60 FPS @ 1080p
- Full Lumen GI + Reflections
- VSM shadows
- TSR at 67%
- All features enabled

**High (Fallback)**:
- 60 FPS @ 1080p
- Reduced Lumen quality
- Lower VSM resolution
- TSR at 50%
- Some features disabled

**Medium (Minimum)**:
- 60 FPS @ 1080p
- Lumen GI only (no reflections)
- Traditional shadows
- TSR at 50%
- Many features disabled

---

## 12. Budget Allocation Strategy

### Priority System

**Tier 1 (Critical)**: Must maintain 60 FPS
- Base rendering
- Player controls
- Core gameplay
- Essential audio

**Tier 2 (High)**: Maintain quality
- Lumen GI
- VSM shadows
- Character animation
- AI behavior

**Tier 3 (Medium)**: Nice to have
- Lumen reflections
- Advanced post-process
- Particle effects
- Ambient audio

**Tier 4 (Low)**: Can be reduced
- Distant details
- Background audio
- Non-essential VFX
- UI animations

### Dynamic Budget Adjustment

```cpp
// Pseudo-code for dynamic budget
if (FrameTime > 16.67ms)
{
    // Reduce Tier 4 first
    ReduceDistantDetails();
    
    if (FrameTime > 16.67ms)
    {
        // Reduce Tier 3
        ReduceLumenReflectionQuality();
        ReduceParticleCount();
    }
    
    if (FrameTime > 16.67ms)
    {
        // Reduce Tier 2
        ReduceLumenGIQuality();
        ReduceVSMResolution();
    }
}
```

---

## 13. Implementation Checklist

### Setup Phase
- [ ] Implement performance profiling tools
- [ ] Create budget monitoring system
- [ ] Setup warning thresholds
- [ ] Create performance dashboard

### Optimization Phase
- [ ] Optimize game thread to <10ms
- [ ] Optimize render thread to <12ms
- [ ] Optimize GPU to <14ms
- [ ] Reduce draw calls to <3000
- [ ] Reduce memory to <4GB

### Validation Phase
- [ ] Profile in all game areas
- [ ] Test with multiple enemies
- [ ] Stress test with max effects
- [ ] Verify stable 60 FPS
- [ ] Document final budgets

---

## 14. Success Criteria

### Performance Targets

```
✓ Maintain 60 FPS (16.67ms) in all scenarios
✓ Game Thread: <10ms (60% of frame)
✓ Render Thread: <12ms (72% of frame)
✓ GPU: <14ms (84% of frame)
✓ Draw Calls: <3000
✓ Triangles: <5M
✓ Memory: <4GB
✓ No hitches or stutters
✓ Stable frame times (±2ms variance)
```

### Quality Targets

```
✓ Lumen GI: High quality
✓ Lumen Reflections: Medium quality
✓ VSM Shadows: High quality
✓ Post-Process: Epic quality
✓ Textures: High quality (2K max)
✓ Meshes: High detail with LODs
✓ Animation: 60 Hz update rate
✓ Audio: Full quality
```

---

**Next Steps**: Implement budget monitoring system, begin optimization work, track progress against budgets.
