# Asset Optimization Technical Guide - HorrorProject

**Generated:** 2026-04-26  
**Purpose:** Technical guidelines and procedures for asset optimization  
**Target Platform:** PC (RTX 3060, 16GB RAM, 1080p)

---

## Table of Contents

1. [Texture Optimization](#texture-optimization)
2. [Mesh Optimization](#mesh-optimization)
3. [Material Optimization](#material-optimization)
4. [Audio Optimization](#audio-optimization)
5. [Blueprint Optimization](#blueprint-optimization)
6. [Performance Budgets](#performance-budgets)
7. [Validation Procedures](#validation-procedures)

---

## Texture Optimization

### Compression Formats

**Color/Albedo Textures:**
```
Format: BC7 (DXT5 for legacy)
Quality: High
sRGB: Enabled
Mip Gen Settings: FromTextureGroup
```

**Normal Maps:**
```
Format: BC5 (DXT5 for legacy)
Quality: High
sRGB: Disabled
Compression: TC_Normalmap
```

**ORM Maps (Occlusion/Roughness/Metallic):**
```
Format: BC7
Quality: High
sRGB: Disabled
Compression: TC_Masks
```

**Emissive Maps:**
```
Format: BC7
Quality: High
sRGB: Enabled
Compression: TC_Default
```

### Resolution Guidelines

| Asset Type | Hero Assets | Standard | Background | Tiling |
|------------|-------------|----------|------------|--------|
| **Characters** | 4K | 2K | 1K | N/A |
| **Props (Large)** | 2K | 1K | 512 | N/A |
| **Props (Small)** | 1K | 512 | 256 | N/A |
| **Architecture** | 2K | 1K | 1K | 512-1K |
| **Floors/Walls** | 2K | 1K | 1K | 512-1K |
| **UI Elements** | 2K | 1K | 512 | N/A |

### Texture Streaming Setup

**Project Settings:**
```ini
[/Script/Engine.RendererSettings]
r.Streaming.PoolSize=3072
r.Streaming.MaxEffectiveScreenSize=0
r.Streaming.Boost=1
r.Streaming.UseFixedPoolSize=False
r.Streaming.FramesForFullUpdate=5
```

**Per-Texture Settings:**
- Enable "Stream" for all textures >512x512
- Set appropriate Mip Load Options
- Configure LOD Group properly

### Virtual Texture Configuration

**When to Use:**
- Large unique surfaces (>4K)
- Terrain textures
- Decals covering large areas

**Setup:**
```
1. Enable Virtual Texturing in Project Settings
2. Convert texture to Runtime Virtual Texture
3. Create RVT Volume in level
4. Configure streaming settings
```

**Settings:**
```ini
[/Script/Engine.RendererSettings]
r.VirtualTexture=True
r.VT.EnableAutoImport=True
r.VT.MaxUploadsPerFrame=16
r.VT.MaxAnisotropy=8
```

### Texture Optimization Checklist

- [ ] All textures use appropriate compression format
- [ ] Resolution matches asset importance
- [ ] Streaming enabled for textures >512x512
- [ ] Mipmap generation configured
- [ ] sRGB settings correct
- [ ] Power-of-two dimensions (or virtual texture)
- [ ] No unused texture channels
- [ ] Texture groups assigned correctly

---

## Mesh Optimization

### LOD Configuration

**Standard LOD Chain:**
```
LOD0: 100% triangles (0-10m)
  - Screen Size: 1.0
  - Triangle Reduction: 0%

LOD1: 50% triangles (10-25m)
  - Screen Size: 0.5
  - Triangle Reduction: 50%

LOD2: 25% triangles (25-50m)
  - Screen Size: 0.25
  - Triangle Reduction: 75%

LOD3: 10% triangles (50m+)
  - Screen Size: 0.1
  - Triangle Reduction: 90%
```

**Hero Asset LOD Chain:**
```
LOD0: 100% (0-15m)
LOD1: 70% (15-30m)
LOD2: 40% (30-60m)
LOD3: 15% (60m+)
```

**Background Asset LOD Chain:**
```
LOD0: 100% (0-5m)
LOD1: 40% (5-15m)
LOD2: 15% (15m+)
```

### Automatic LOD Generation

**UE5 Settings:**
```
Reduction Settings:
  - Percent Triangles: [1.0, 0.5, 0.25, 0.1]
  - Max Deviation: 0.0
  - Welding Threshold: 0.1
  - Hard Angle Threshold: 80.0
  - Recompute Normals: True
  - Preserve Smoothing Groups: True
```

**Command Line:**
```powershell
# Generate LODs for all meshes in folder
ue5editor-cmd.exe "D:\gptzuo\HorrorProject\HorrorProject\HorrorProject.uproject" `
  -run=GenerateLODs `
  -PackagePath="/Game/DeepWaterStation/Meshes" `
  -NumLODs=4
```

### Nanite Conversion

**Eligibility Criteria:**
- Triangle count >10,000
- Static mesh (no skeletal animation)
- No world position offset
- No masked/translucent materials
- No two-sided materials (or acceptable quality loss)

**Conversion Process:**
```
1. Select mesh in Content Browser
2. Right-click → Nanite → Enable Nanite Support
3. Set Fallback Relative Error: 1.0
4. Test performance vs traditional LODs
5. Disable if performance worse
```

**Nanite Settings:**
```
Fallback Relative Error: 1.0 (standard)
Fallback Relative Error: 0.5 (hero assets)
Preserve Area: True
Keep Triangle Percent: 100%
```

### Collision Optimization

**Collision Complexity:**
```
Simple Collision:
  - Use for most props
  - Box, sphere, capsule primitives
  - Max 4-6 primitives per mesh

Complex Collision:
  - Only for critical gameplay objects
  - Use simplified collision mesh
  - Max 500 triangles

No Collision:
  - Decorative objects
  - Background details
  - Ceiling fixtures
```

**Collision Presets:**
```
BlockAll: Player-blocking geometry
OverlapAll: Trigger volumes
NoCollision: Decorative props
Custom: Specific gameplay needs
```

### Lightmap Optimization

**Resolution Guidelines:**
```
Small Props (<1m): 16-32
Medium Props (1-3m): 64
Large Props (3-10m): 128
Architecture (10m+): 256-512
Hero Assets: 512 max
```

**Lightmap UV Requirements:**
- Non-overlapping UVs
- 0-1 UV space
- Proper padding (2-4 pixels)
- Consistent texel density

### Mesh Optimization Checklist

- [ ] LOD chain generated (4 levels minimum)
- [ ] LOD screen sizes configured
- [ ] Nanite enabled for eligible meshes
- [ ] Collision simplified or disabled
- [ ] Lightmap resolution appropriate
- [ ] Lightmap UVs valid
- [ ] No degenerate triangles
- [ ] Proper vertex normals
- [ ] Materials assigned correctly

---

## Material Optimization

### Material Complexity Budget

**Target Shader Instructions:**
```
Simple Materials: <100 instructions
Standard Materials: 100-200 instructions
Complex Materials: 200-400 instructions
Hero Materials: <600 instructions
```

**Check Complexity:**
```
1. Open material editor
2. Window → Statistics
3. Review instruction count
4. Optimize if over budget
```

### Material Optimization Techniques

**1. Use Material Instances**
```
Master Material (M_Master_PBR):
  - Base PBR setup
  - Parameterized values
  - Shared across many assets

Material Instances (MI_*):
  - Override parameters only
  - No shader recompilation
  - Minimal memory overhead
```

**2. Texture Sampling Optimization**
```
Good:
  - 3-5 texture samples per material
  - Shared samplers
  - Packed textures (ORM)

Bad:
  - >8 texture samples
  - Separate R/G/B textures
  - Redundant samples
```

**3. Math Optimization**
```
Expensive Operations (minimize):
  - Sine/Cosine
  - Power
  - Normalize
  - Distance calculations

Cheap Operations (prefer):
  - Multiply/Add
  - Lerp
  - Clamp
  - Simple math
```

**4. Material Functions**
```
Use for:
  - Repeated logic
  - Shared calculations
  - Standardized effects

Benefits:
  - Code reuse
  - Easier updates
  - Consistent quality
```

### Material Quality Levels

**Setup Quality Switching:**
```
Quality Switch Node:
  - Low: Simplified shaders
  - Medium: Standard quality
  - High: Full quality
  - Epic: Maximum quality
```

**Per-Quality Settings:**
```
Low:
  - 1-2 texture samples
  - No parallax
  - Simple lighting

Medium:
  - 3-4 texture samples
  - Basic effects
  - Standard lighting

High:
  - 4-6 texture samples
  - Full PBR
  - Advanced effects

Epic:
  - 6-8 texture samples
  - All features
  - Maximum quality
```

### Material Instance Hierarchy

**Recommended Structure:**
```
M_Master_PBR (Master Material)
├── MI_Metal_Base (Parent Instance)
│   ├── MI_Metal_Rusty
│   ├── MI_Metal_Clean
│   └── MI_Metal_Painted
├── MI_Concrete_Base (Parent Instance)
│   ├── MI_Concrete_Rough
│   └── MI_Concrete_Smooth
└── MI_Plastic_Base (Parent Instance)
    ├── MI_Plastic_Matte
    └── MI_Plastic_Glossy
```

### Material Parameter Collections

**Use Cases:**
- Global time-of-day
- Weather effects
- Player proximity effects
- Dynamic lighting

**Setup:**
```
1. Create Material Parameter Collection
2. Add scalar/vector parameters
3. Reference in materials
4. Update via Blueprint
```

### Material Optimization Checklist

- [ ] Shader instructions under budget
- [ ] Using material instances (not unique materials)
- [ ] Texture samples minimized (<6)
- [ ] Expensive math operations minimized
- [ ] Quality levels implemented
- [ ] Material functions used for shared logic
- [ ] No redundant calculations
- [ ] Proper material domain set

---

## Audio Optimization

### Compression Settings

**Ambient Loops:**
```
Format: OGG Vorbis
Quality: 0.6 (60%)
Sample Rate: 44.1 kHz
Streaming: Enabled (if >5 seconds)
Loading Behavior: Stream
```

**Sound Effects:**
```
Format: OGG Vorbis
Quality: 0.7 (70%)
Sample Rate: 44.1 kHz
Streaming: Disabled
Loading Behavior: Load on Demand
```

**Music:**
```
Format: OGG Vorbis
Quality: 0.8 (80%)
Sample Rate: 44.1 kHz
Streaming: Enabled
Loading Behavior: Stream
```

**Voice/Dialog:**
```
Format: OGG Vorbis
Quality: 0.75 (75%)
Sample Rate: 22.05 kHz (mono) or 44.1 kHz (stereo)
Streaming: Enabled (if >3 seconds)
Loading Behavior: Stream
```

### Audio Asset Organization

**Folder Structure:**
```
Content/Audio/
├── Ambient/
│   ├── Loops/
│   └── OneShots/
├── Music/
│   ├── Tension/
│   ├── Chase/
│   └── Stingers/
├── SFX/
│   ├── Footsteps/
│   ├── Doors/
│   ├── Mechanical/
│   └── Horror/
└── Voice/
    ├── Whispers/
    └── Screams/
```

### Sound Cue Setup

**Basic Sound Cue:**
```
Sound Wave → Random → Attenuation → Output
              ↓
           Modulator (Volume/Pitch)
```

**Advanced Sound Cue:**
```
Sound Wave → Random → Attenuation → Reverb → Output
              ↓           ↓
           Modulator   Distance
              ↓
           Delay
```

### Attenuation Settings

**Close Range (0-10m):**
```
Falloff Distance: 10m
Attenuation Shape: Sphere
Falloff: Natural Sound
Volume: 1.0 → 0.0
```

**Medium Range (0-25m):**
```
Falloff Distance: 25m
Attenuation Shape: Sphere
Falloff: Natural Sound
Volume: 1.0 → 0.0
```

**Long Range (0-50m):**
```
Falloff Distance: 50m
Attenuation Shape: Sphere
Falloff: Linear
Volume: 1.0 → 0.0
```

### Audio Occlusion

**Enable Occlusion:**
```
Sound Attenuation:
  - Enable Occlusion: True
  - Occlusion Trace Channel: Visibility
  - Occlusion Interpolation Time: 0.5s
  - Occlusion Volume Attenuation: 0.5
  - Occlusion High Frequency Attenuation: 0.8
```

### Audio Memory Budget

**Target Memory Usage:**
```
Loaded Audio: <50 MB
Streaming Audio: <256 MB total
Concurrent Sounds: <32 active
```

**Optimization:**
- Stream all audio >5 seconds
- Use sound concurrency limits
- Implement audio pooling
- Unload unused audio banks

### Audio Optimization Checklist

- [ ] All audio compressed to OGG Vorbis
- [ ] Streaming enabled for long files
- [ ] Sound cues created (not raw waves)
- [ ] Attenuation configured
- [ ] Occlusion enabled
- [ ] Concurrency limits set
- [ ] Audio memory under budget
- [ ] No audio pops or clicks

---

## Blueprint Optimization

### Tick Optimization

**Avoid Tick When Possible:**
```cpp
// Bad: Using Tick
Event Tick
  → Do Something Every Frame

// Good: Using Timer
Event BeginPlay
  → Set Timer by Function Name
     Function: DoSomething
     Time: 0.1
     Looping: True
```

**Conditional Tick:**
```cpp
Event BeginPlay
  → Set Actor Tick Enabled (False)

Custom Event: EnableTick
  → Set Actor Tick Enabled (True)

Custom Event: DisableTick
  → Set Actor Tick Enabled (False)
```

### Component Reference Caching

**Bad (Repeated GetComponent):**
```cpp
Event Tick
  → Get Component by Class (Mesh)
     → Set Material
```

**Good (Cached Reference):**
```cpp
Variable: MeshComponent (Static Mesh Component)

Event BeginPlay
  → Get Component by Class (Mesh)
     → Set MeshComponent

Event Tick
  → MeshComponent → Set Material
```

### Collision Optimization

**Efficient Raycasting:**
```cpp
// Use specific trace channel
Line Trace by Channel
  Trace Channel: Visibility (not ECC_WorldStatic)
  
// Limit trace distance
Start: Player Location
End: Player Location + (Forward * 200)

// Use simple collision
Trace Complex: False
```

**Overlap Events:**
```cpp
// Use specific object types
Begin Overlap Event
  → Filter by Object Type
     → Only process relevant objects

// Disable when not needed
Set Generate Overlap Events (False)
```

### Blueprint Nativization

**Enable for Performance-Critical BPs:**
```
Project Settings → Packaging
  → Blueprint Nativization Method: Inclusive
  → Add to Nativization List
```

**Candidates:**
- Player character
- AI controllers
- Frequently spawned actors
- Complex gameplay systems

### Event-Driven Design

**Bad (Polling):**
```cpp
Event Tick
  → Check if Health < 50
     → Do Something
```

**Good (Event-Driven):**
```cpp
Function: TakeDamage
  → Subtract Health
  → Branch (Health < 50)
     → Trigger Low Health Event
```

### Blueprint Optimization Checklist

- [ ] Tick disabled or minimized
- [ ] Component references cached
- [ ] Timers used instead of tick
- [ ] Collision traces optimized
- [ ] Event-driven design used
- [ ] No expensive operations in tick
- [ ] Blueprint nativization considered
- [ ] Profiled with Blueprint Profiler

---

## Performance Budgets

### GPU Budget (16.67ms for 60 FPS)

```
Base Pass: 4-6ms
  - Opaque geometry rendering
  - Material shading

Lighting: 2-3ms
  - Dynamic lights
  - Shadows

Post-Processing: 2-3ms
  - Bodycam effect
  - Color grading
  - Bloom

Translucency: 1-2ms
  - Particles
  - Transparent materials

Other: 2-3ms
  - UI
  - Misc rendering

Total: 11-17ms (target: <14ms)
```

### CPU Budget (16.67ms for 60 FPS)

```
Game Thread: 8-10ms
  - Blueprint logic
  - AI
  - Physics
  - Animation

Render Thread: 6-8ms
  - Draw call submission
  - Culling
  - Material updates

Total: 14-18ms (target: <15ms)
```

### Memory Budget

```
Textures: 4-5 GB VRAM
Meshes: 1-1.5 GB VRAM
Materials: 256 MB VRAM
Audio: 50 MB RAM (loaded)
Blueprints: 100 MB RAM
Other: 500 MB RAM

Total VRAM: 6-7 GB (target: <8 GB)
Total RAM: 8-10 GB (target: <12 GB)
```

### Draw Call Budget

```
Opaque Meshes: 2000-3000 draw calls
Translucent: 200-300 draw calls
Shadows: 1000-1500 draw calls
UI: 50-100 draw calls

Total: 3250-4900 (target: <4000)
```

### Triangle Budget

```
On-Screen Triangles: 2-4 million
Total Scene: 10-20 million (with LODs)

Per-Frame Target: 3 million triangles
```

---

## Validation Procedures

### Performance Profiling

**GPU Profiling:**
```
Console Commands:
  stat GPU - GPU timing breakdown
  stat RHI - Rendering stats
  profilegpu - Detailed GPU profile
  r.ScreenPercentage 100 - Verify resolution
```

**CPU Profiling:**
```
Console Commands:
  stat Game - Game thread timing
  stat SceneRendering - Render thread timing
  stat Engine - Overall engine stats
  stat Unit - Combined timing
```

**Memory Profiling:**
```
Console Commands:
  stat Memory - Memory overview
  stat Streaming - Texture streaming
  stat StreamingDetails - Detailed streaming
  memreport - Full memory report
```

### Quality Validation

**Visual Quality Checks:**
- [ ] No texture pop-in during normal movement
- [ ] LOD transitions smooth and unnoticeable
- [ ] Materials look correct at all distances
- [ ] Lighting quality maintained
- [ ] No visual artifacts

**Audio Quality Checks:**
- [ ] No compression artifacts
- [ ] Proper attenuation
- [ ] Occlusion working correctly
- [ ] No audio pops or clicks
- [ ] Proper spatialization

**Performance Checks:**
- [ ] 60 FPS maintained in all areas
- [ ] No hitches or stutters
- [ ] Smooth frame times
- [ ] Memory usage stable
- [ ] No memory leaks

### Automated Validation

**Run Validation Scripts:**
```powershell
# Texture validation
.\Scripts\Assets\ValidateTextures.ps1

# Mesh validation
.\Scripts\Assets\ValidateMeshes.ps1

# Material validation
.\Scripts\Assets\ValidateMaterials.ps1

# Audio validation
.\Scripts\Assets\ValidateAudio.ps1

# Generate report
.\Scripts\Assets\GenerateAssetReport.ps1
```

### Pre-Commit Checklist

Before committing optimized assets:
- [ ] Performance profiled and meets targets
- [ ] Visual quality validated
- [ ] Audio quality validated
- [ ] Automated validation passed
- [ ] Documentation updated
- [ ] Tested on target hardware
- [ ] No regressions introduced

---

## Tools & Resources

### UE5 Built-in Tools
- **Size Map:** Visualize asset memory usage
- **Reference Viewer:** Track asset dependencies
- **Audit Window:** Find asset issues
- **GPU Visualizer:** Detailed GPU profiling
- **Blueprint Profiler:** Blueprint performance

### External Tools
- **RenderDoc:** GPU debugging
- **Simplygon:** Advanced LOD generation
- **Substance Designer:** Texture optimization
- **Audacity:** Audio editing and analysis

### Useful Console Commands
```
stat FPS - Show FPS
stat Unit - Show frame timing
stat Streaming - Texture streaming stats
r.ScreenPercentage 100 - Set resolution scale
r.VSync 0 - Disable VSync for testing
t.MaxFPS 0 - Uncap framerate
```

---

**Document Owner:** Asset Manager Agent  
**Last Updated:** 2026-04-26  
**Next Review:** After major optimization phases
