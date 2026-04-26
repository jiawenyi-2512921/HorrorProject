# Rendering Optimization Guide

## Comprehensive Rendering Optimization for HorrorProject

**Goal**: Achieve stable 60 FPS through rendering pipeline optimization
**Target**: GPU time <14ms, Draw calls <3000, Efficient shadow/lighting
**Tools**: ProfileRendering.ps1, OptimizeLighting.ps1, UE5 Profiling Tools

---

## Understanding Rendering Pipeline

### GPU Frame Breakdown
```
Total Frame Time (16.67ms @ 60 FPS):
├─ Shadow Rendering: 3-5ms (Target: <4ms)
├─ Base Pass: 4-6ms (Target: <5ms)
├─ Lighting: 2-4ms (Target: <3ms)
├─ Translucency: 1-2ms (Target: <1.5ms)
├─ Post-Processing: 2-3ms (Target: <2ms)
└─ Other: 1-2ms

GPU Budget: 14ms (leaves 2.67ms for CPU)
```

### Performance Metrics
- **Draw Calls**: Number of draw commands sent to GPU
- **Triangles**: Total polygons rendered
- **Overdraw**: Pixels rendered multiple times
- **Shader Complexity**: Instructions per pixel
- **Texture Bandwidth**: Memory transferred

---

## Lighting Optimization

### 1. Lumen Configuration (UE5)

#### Enable Lumen
**Benefits**: Better performance than traditional lightmaps for dynamic scenes

**Configuration** (DefaultEngine.ini):
```ini
[/Script/Engine.RendererSettings]
; Enable Lumen Global Illumination
r.Lumen.DiffuseIndirect.Allow=True
r.Lumen.Reflections.Allow=True

; Lumen Quality Settings (60 FPS target)
r.LumenScene.SurfaceCache.MeshCardsMergedResolution=256
r.Lumen.TraceMeshSDFs=1
r.Lumen.ScreenProbeGather.ScreenTraces=1

; Lumen Performance Settings
r.Lumen.ScreenProbeGather.RadianceCache.NumFramesToKeepCached=2
r.Lumen.ScreenProbeGather.DownsampleFactor=2
```

#### Lumen Quality Levels
```ini
; Epic Quality (Target: 60 FPS)
r.Lumen.ScreenProbeGather.ScreenTraces=1
r.Lumen.TraceMeshSDFs=1

; High Quality (Target: 75 FPS)
r.Lumen.ScreenProbeGather.ScreenTraces=0
r.Lumen.TraceMeshSDFs=1

; Medium Quality (Target: 90 FPS)
r.Lumen.ScreenProbeGather.ScreenTraces=0
r.Lumen.TraceMeshSDFs=0
```

### 2. Shadow Optimization

#### Virtual Shadow Maps (VSM)
**Benefits**: Better performance and quality than traditional shadow maps

**Configuration**:
```ini
[/Script/Engine.RendererSettings]
; Enable Virtual Shadow Maps
r.Shadow.Virtual.Enable=1
r.Shadow.Virtual.MaxPhysicalPages=4096
r.Shadow.Virtual.ResolutionLodBiasDirectional=-1.5
r.Shadow.Virtual.ResolutionLodBiasLocal=-1.0

; Shadow Quality
r.Shadow.DistanceScale=0.5
r.Shadow.RadiusThreshold=0.03
```

#### Shadow Cascades
**Optimization**: Reduce cascade count for better performance

```ini
; Cascade Shadow Maps
r.Shadow.C.MaxCascades=3  ; Down from 4
r.Shadow.MaxResolution=2048  ; Per cascade
r.Shadow.MaxCSMResolution=2048
```

#### Per-Light Shadow Settings
```cpp
// In light actor properties
Cast Shadows: True/False
Shadow Resolution Scale: 0.5-2.0
Shadow Bias: 0.5 (default)
Shadow Filter Sharpen: 0.5

// Optimization
- Limit shadow-casting lights to 2-3 per scene
- Disable shadows on small/distant lights
- Use contact shadows for detail
```

### 3. Light Count Management

#### Light Budget
```
Per Scene:
- Directional Lights: 1 (sun/moon)
- Stationary Lights: 2-3 (key lights)
- Dynamic Lights: 2-4 (moving, flickering)
- Static Lights: Unlimited (baked)

Per Object:
- Overlapping Lights: Max 4
- Shadow-Casting: Max 2
```

#### Light Mobility
```
Static:
- Baked into lightmaps
- No runtime cost
- No dynamic shadows
- Use for: Architecture, static props

Stationary:
- Baked indirect lighting
- Dynamic direct lighting
- Dynamic shadows
- Use for: Sun, key lights

Movable:
- Fully dynamic
- Highest cost
- Use for: Flashlight, effects
```

### 4. Light Types Performance

#### Performance Ranking (Fastest to Slowest)
```
1. Static Light (Baked)
   - No runtime cost
   - Best quality

2. Directional Light (Stationary)
   - Single light, entire scene
   - CSM shadows
   - Moderate cost

3. Spot Light (Stationary)
   - Cone-shaped
   - Single shadow map
   - Moderate cost

4. Point Light (Stationary)
   - Omnidirectional
   - 6 shadow maps (cubemap)
   - High cost

5. Rect Light (Movable)
   - Area light
   - Expensive shadows
   - Very high cost
```

#### Optimization Tips
```
- Use spot lights instead of point lights
- Disable shadows on fill lights
- Use IES profiles instead of light functions
- Limit light radius to necessary area
- Use inverse square falloff
```

---

## Shadow Rendering Optimization

### 1. Shadow Distance
**Impact**: Huge performance gain

```ini
; Reduce shadow rendering distance
r.Shadow.DistanceScale=0.5  ; 50% of default
r.Shadow.CSM.TransitionScale=0.0  ; Instant transition
```

**Per-Light Settings**:
```
Dynamic Shadow Distance: 5000 units (50m)
- Close objects: Full shadows
- Distant objects: No shadows
```

### 2. Shadow Resolution
**Impact**: Moderate performance gain

```ini
; Shadow map resolution
r.Shadow.MaxResolution=2048  ; Down from 4096
r.Shadow.MaxCSMResolution=2048
```

**Per-Light Settings**:
```
Shadow Resolution Scale:
- Hero lights: 1.0-2.0
- Standard lights: 0.5-1.0
- Fill lights: 0.25-0.5
```

### 3. Contact Shadows
**Use**: Add detail without increasing shadow map resolution

```cpp
// In light properties
Contact Shadow Length: 0.1-0.5
Contact Shadow Cast: True

// Benefits
- Adds contact detail
- Cheap screen-space effect
- Complements shadow maps
```

### 4. Distance Field Shadows
**Use**: Efficient shadows for distant objects

```ini
[/Script/Engine.RendererSettings]
r.GenerateMeshDistanceFields=True
r.DistanceFieldBuild.EightBit=True
r.AOGlobalDistanceField=True

; Distance field shadows
r.DFShadowQuality=2  ; 0-4, higher = better quality
```

---

## Draw Call Optimization

### 1. Mesh Instancing

#### Static Mesh Instancing
**Benefit**: Reduce draw calls by 90%+

```cpp
// Use Instanced Static Mesh Component
- Automatically batches identical meshes
- Single draw call for all instances
- Huge performance gain

Example:
100 trees = 100 draw calls (without instancing)
100 trees = 1 draw call (with instancing)
```

#### Hierarchical Instanced Static Mesh (HISM)
**Benefit**: Instancing + LOD + culling

```cpp
// Use for:
- Foliage
- Repeated props
- Particle systems (GPU particles)

Benefits:
- Automatic LOD
- Frustum culling
- Occlusion culling
- Distance culling
```

### 2. Mesh Merging

#### Actor Merging
**Benefit**: Combine multiple static meshes into one

```
Tools → Merge Actors
- Select multiple static meshes
- Merge into single mesh
- Reduces draw calls
- Best for: Small props, details
```

#### Automatic Merging
```ini
[/Script/Engine.HierarchicalLODSettings]
; Enable automatic mesh merging
bEnableHierarchicalLODSystem=True
```

### 3. Material Batching

#### Reduce Unique Materials
**Strategy**: Use material instances

```
Before:
- 100 unique materials
- 100 draw calls (minimum)

After:
- 1 master material
- 100 material instances
- Better batching
- Fewer state changes
```

#### Material Sorting
**Optimization**: Engine sorts by material automatically

```
Tips:
- Use same base material
- Minimize material switches
- Group objects by material
```

---

## Occlusion Culling

### 1. Precomputed Visibility

#### Setup
```
1. Place Precomputed Visibility Volume in level
2. Build → Build Lighting
3. Generates visibility cells

Benefits:
- Culls occluded objects
- No runtime cost
- Best for: Static scenes
```

#### Configuration
```ini
[/Script/Engine.RendererSettings]
bPrecomputedVisibilityWarning=False
```

### 2. Distance Culling

#### Per-Actor Settings
```cpp
// In actor properties
Desired Max Draw Distance: 5000 units
Min Draw Distance: 0

// Automatic culling
- Objects beyond distance not rendered
- Huge performance gain
- Set per object importance
```

#### Global Settings
```ini
; View distance scale
r.ViewDistanceScale=1.0  ; 0.5 = half distance
```

### 3. Frustum Culling
**Automatic**: Engine culls objects outside view

**Optimization**:
```
- Use simple collision bounds
- Avoid oversized bounds
- Enable "Use Bounds for Culling"
```

### 4. Occlusion Queries

#### Hardware Occlusion
**Automatic**: GPU tests object visibility

```ini
; Occlusion query settings
r.AllowOcclusionQueries=1
r.DownsampledOcclusionQueries=1
```

---

## LOD (Level of Detail) Optimization

### 1. Static Mesh LODs

#### LOD Generation
```
In Static Mesh Editor:
1. LOD Settings → Auto Compute LOD Distances
2. Number of LODs: 4
3. Reduction Settings:
   - LOD1: 50% triangles
   - LOD2: 75% reduction
   - LOD3: 90% reduction
```

#### LOD Distances
```
LOD0: 0-1000 units (0-10m)
LOD1: 1000-2500 units (10-25m)
LOD2: 2500-5000 units (25-50m)
LOD3: 5000+ units (50m+)
```

#### LOD Transition
```cpp
// Smooth LOD transitions
Dithered LOD Transition: True
LOD Transition Time: 0.5 seconds
```

### 2. Hierarchical LOD (HLOD)

#### Setup
```
Window → Hierarchical LOD Outliner
1. Generate Clusters
2. Build LOD Actors
3. Automatically merges distant objects

Benefits:
- Reduces draw calls for distant objects
- Automatic mesh merging
- Huge performance gain
```

#### Configuration
```ini
[/Script/Engine.HierarchicalLODSettings]
bEnableHierarchicalLODSystem=True
HLODDistanceOverride=10000
```

### 3. Nanite (UE5.1+)

#### Enable Nanite
**Benefits**: Automatic LOD, virtualized geometry

```cpp
// In Static Mesh properties
Enable Nanite Support: True

// Best for:
- High-poly static meshes (>10K triangles)
- Architecture
- Detailed props
- Non-deforming meshes
```

#### Configuration
```ini
[/Script/Engine.RendererSettings]
; Nanite settings
r.Nanite.MaxPixelsPerEdge=1.0
r.Nanite.MinPixelsPerEdgeHW=64
```

---

## Post-Processing Optimization

### 1. Post-Process Volume Settings

#### Performance-Optimized Settings
```cpp
// In Post Process Volume
Unbound: True  ; Affects entire level

// Bloom
Bloom Intensity: 0.5-1.0
Bloom Method: Standard (not Convolution)
Bloom Size Scale: 2-4

// Ambient Occlusion
AO Intensity: 0.5-1.0
AO Radius: 50-100
AO Quality: 50-75%

// Motion Blur
Motion Blur Amount: 0.5 (or disable)
Motion Blur Max: 0.5

// Depth of Field
DOF Method: Gaussian (not Bokeh)
DOF Quality: Medium
```

### 2. Disable Expensive Effects

#### For Performance
```cpp
// Disable or reduce:
- Screen Space Reflections (use Lumen instead)
- Ambient Occlusion (use distance field AO)
- Motion Blur (expensive, often disabled)
- Lens Flares (expensive)
- Chromatic Aberration (cheap, keep)
- Vignette (cheap, keep)
```

### 3. Temporal Anti-Aliasing (TAA)

#### Recommended Settings
```ini
[/Script/Engine.RendererSettings]
; Use TAA (best quality/performance)
r.DefaultFeature.AntiAliasing=2  ; 2 = TAA

; TAA quality
r.TemporalAA.Quality=2  ; 0-2, 2 = best
r.TemporalAA.Upsampling=1  ; Enable upsampling
```

---

## Translucency Optimization

### 1. Reduce Translucent Overdraw

#### Overdraw Visualization
```
View Mode → Shader Complexity → Translucency Overdraw
- Red = High overdraw (bad)
- Green = Low overdraw (good)
```

#### Optimization Strategies
```
1. Use Masked instead of Translucent when possible
2. Reduce particle size
3. Limit overlapping translucent objects
4. Use additive blend mode for effects
5. Disable "Separate Translucency" if not needed
```

### 2. Translucency Settings

#### Configuration
```ini
[/Script/Engine.RendererSettings]
; Translucency settings
r.SeparateTranslucency=0  ; Disable if not needed
r.TranslucencyLightingVolumeDim=32  ; Lower = faster
r.TranslucencyVolumeBlur=0  ; Disable blur
```

### 3. Particle Optimization

#### Niagara (GPU Particles)
**Benefits**: Much faster than CPU particles

```cpp
// Use Niagara for:
- Large particle counts (>100)
- Simple particles
- Effects

// Optimization:
- Limit max particles
- Use GPU simulation
- Reduce sprite size
- Use texture atlases
```

---

## Horror Game Specific Optimizations

### 1. Fog & Atmosphere

#### Volumetric Fog
**Challenge**: Expensive but essential for horror atmosphere

**Optimization**:
```ini
[/Script/Engine.RendererSettings]
; Volumetric fog quality
r.VolumetricFog=1
r.VolumetricFog.GridPixelSize=16  ; Higher = faster
r.VolumetricFog.GridSizeZ=64  ; Lower = faster
r.VolumetricFog.HistoryMissSupersampleCount=4
```

**Alternative**: Exponential Height Fog
```cpp
// Cheaper alternative
Use Exponential Height Fog instead of Volumetric
- Much faster
- Good for distant fog
- Less detailed
```

### 2. Flashlight & Dynamic Lighting

#### Optimization
```cpp
// Flashlight settings
Light Type: Spot Light (not Point)
Cast Shadows: True
Shadow Resolution Scale: 1.0
Radius: 1000-2000 units (limit range)
Intensity: Adjust for atmosphere

// Performance tips:
- Single shadow-casting light
- Use contact shadows for detail
- Limit light radius
- Use IES profile for shape
```

### 3. Dark Scenes

#### Leverage Darkness
**Strategy**: Reduce quality in dark areas

```cpp
// Optimization opportunities:
- Lower texture resolution in dark areas
- Reduce shadow quality
- Simplify materials
- Use emissive instead of lights
- Rely on post-processing for atmosphere
```

### 4. Jump Scares & Effects

#### Performance Budget
```
Reserve performance budget for:
- Sudden particle effects
- Dynamic lights
- Screen effects
- Audio

Strategy:
- Reduce baseline load
- Allow spikes during scares
- Quick recovery after effect
```

---

## Rendering Quality Levels

### Epic Quality (60 FPS Target)
```ini
[/Script/Engine.RendererSettings]
r.Shadow.MaxResolution=2048
r.Shadow.CSM.MaxCascades=3
r.Lumen.ScreenProbeGather.ScreenTraces=1
r.VolumetricFog.GridPixelSize=16
r.PostProcessAAQuality=4
```

### High Quality (75 FPS Target)
```ini
r.Shadow.MaxResolution=1024
r.Shadow.CSM.MaxCascades=3
r.Lumen.ScreenProbeGather.ScreenTraces=0
r.VolumetricFog.GridPixelSize=24
r.PostProcessAAQuality=3
```

### Medium Quality (90 FPS Target)
```ini
r.Shadow.MaxResolution=1024
r.Shadow.CSM.MaxCascades=2
r.Lumen.ScreenProbeGather.ScreenTraces=0
r.VolumetricFog.GridPixelSize=32
r.PostProcessAAQuality=2
```

---

## Profiling & Debugging

### Console Commands
```
stat fps              - Show FPS
stat unit             - Frame time breakdown
stat gpu              - GPU profiling
stat scenerendering   - Rendering stats
profilegpu            - Detailed GPU profile
r.ScreenPercentage X  - Resolution scale (50-100)
```

### View Modes
```
Alt+1: Lit (normal)
Alt+2: Unlit
Alt+3: Wireframe
Alt+4: Detail Lighting
Alt+8: Shader Complexity
```

### GPU Profiler
```
Ctrl+Shift+, : Open GPU Profiler
- Shows GPU time per pass
- Identifies bottlenecks
- Frame-by-frame analysis
```

---

## Optimization Checklist

### Lighting
- [ ] Enable Lumen
- [ ] Enable Virtual Shadow Maps
- [ ] Limit shadow-casting lights to 2-3
- [ ] Reduce shadow distance
- [ ] Optimize cascade count
- [ ] Use contact shadows

### Draw Calls
- [ ] Enable mesh instancing
- [ ] Use HISM for foliage
- [ ] Merge small props
- [ ] Use material instances
- [ ] Implement HLOD

### Culling
- [ ] Add precomputed visibility volumes
- [ ] Set distance culling per actor
- [ ] Enable occlusion queries
- [ ] Optimize collision bounds

### LOD
- [ ] Generate LODs for all meshes
- [ ] Set appropriate LOD distances
- [ ] Enable dithered transitions
- [ ] Implement HLOD
- [ ] Consider Nanite for static meshes

### Post-Processing
- [ ] Optimize bloom settings
- [ ] Reduce AO quality
- [ ] Use TAA (not MSAA)
- [ ] Disable expensive effects
- [ ] Optimize volumetric fog

---

## Performance Targets

### GPU Time Budget (60 FPS)
```
Total: 14ms
├─ Shadows: 3-4ms
├─ Base Pass: 4-5ms
├─ Lighting: 2-3ms
├─ Translucency: 1-1.5ms
├─ Post-Process: 2ms
└─ Other: 1-1.5ms
```

### Draw Call Budget
```
Total: <3000 draw calls
├─ Static Meshes: <2000
├─ Skeletal Meshes: <100
├─ Particles: <200
├─ Translucency: <300
└─ UI: <400
```

### Triangle Budget
```
On-Screen Triangles: <5M
Total Scene: <20M
Per Mesh LOD0: <50K
Per Mesh LOD1: <25K
Per Mesh LOD2: <10K
Per Mesh LOD3: <5K
```

---

## Quick Reference

### Optimization Scripts
```powershell
# Profile rendering
.\ProfileRendering.ps1

# Optimize lighting
.\OptimizeLighting.ps1

# Validate optimizations
.\ValidateOptimizations.ps1
```

### Key Settings
```ini
; Lumen
r.Lumen.DiffuseIndirect.Allow=True

; Virtual Shadow Maps
r.Shadow.Virtual.Enable=1

; Shadow Quality
r.Shadow.CSM.MaxCascades=3
r.Shadow.MaxResolution=2048

; Distance Field
r.GenerateMeshDistanceFields=True
```

### Performance Commands
```
stat gpu              - GPU profiling
profilegpu            - Detailed GPU profile
r.ScreenPercentage 75 - Reduce resolution
```
