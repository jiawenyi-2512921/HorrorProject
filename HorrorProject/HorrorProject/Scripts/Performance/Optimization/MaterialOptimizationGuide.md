# Material Optimization Guide

## Comprehensive Material Optimization for HorrorProject

**Goal**: Reduce shader complexity while maintaining visual quality
**Target**: <300 shader instructions per material, <8 texture samples
**Tools**: OptimizeMaterials.ps1, UE5 Material Editor

---

## Understanding Material Cost

### Shader Instruction Cost
```
Base Cost: ~50 instructions (minimal material)
Texture Sample: ~4 instructions each
Math Operations: 1-5 instructions
Complex Functions: 10-50 instructions

Example Complex Material:
- 10 texture samples: 40 instructions
- Normal mapping: 20 instructions
- PBR calculations: 30 instructions
- Custom effects: 50 instructions
Total: ~140 instructions (acceptable)
```

### Performance Impact
- **GPU Time**: More instructions = longer shader execution
- **Compilation**: Complex shaders = longer load times
- **Memory**: Shader cache size increases
- **Batching**: Complex materials reduce batching efficiency

---

## Material Complexity Targets

### Shader Instruction Budgets

#### Hero Materials (Characters, Key Props)
- **Maximum**: 400 instructions
- **Target**: 250-300 instructions
- **Texture Samples**: 8-10

#### Standard Materials (Environment, Props)
- **Maximum**: 300 instructions
- **Target**: 150-200 instructions
- **Texture Samples**: 6-8

#### Background Materials (Distant, Simple)
- **Maximum**: 200 instructions
- **Target**: 100-150 instructions
- **Texture Samples**: 4-6

#### UI Materials
- **Maximum**: 100 instructions
- **Target**: 50-80 instructions
- **Texture Samples**: 2-4

---

## Optimization Techniques

### 1. Texture Sample Reduction

#### Channel Packing
**Before** (3 textures, 12 samples):
```
BaseColor = Texture2DSample(ColorTex)
Roughness = Texture2DSample(RoughnessTex).r
Metallic = Texture2DSample(MetallicTex).r
```

**After** (1 texture, 4 samples):
```
ORM = Texture2DSample(ORMTex)
BaseColor = ORM.rgb  // Or separate color texture
Roughness = ORM.g
Metallic = ORM.b
```

**Savings**: 8 samples, ~32 instructions

#### Shared Samplers
**Before**:
```
float3 Normal1 = Texture2DSample(NormalTex1, Sampler1, UV)
float3 Normal2 = Texture2DSample(NormalTex2, Sampler2, UV)
```

**After**:
```
float3 Normal1 = Texture2DSample(NormalTex1, SharedSampler, UV)
float3 Normal2 = Texture2DSample(NormalTex2, SharedSampler, UV)
```

**Savings**: Reduced sampler state changes

#### Texture Atlas
**Before** (Multiple materials, multiple textures):
```
Material1: ColorTex1, NormalTex1
Material2: ColorTex2, NormalTex2
Material3: ColorTex3, NormalTex3
```

**After** (Single material, atlased textures):
```
Material: ColorAtlas, NormalAtlas
UV = UV * TileSize + TileOffset
```

**Savings**: Reduced material count, better batching

### 2. Math Optimization

#### Avoid Expensive Operations
**Expensive** (avoid in pixel shader):
- `pow()` - Use lookup texture instead
- `sin()`, `cos()` - Pre-calculate or use lookup
- `normalize()` - Only when necessary
- Division - Multiply by reciprocal instead

**Example Optimization**:
```
// Before (expensive)
float3 Result = pow(Color, 2.2)

// After (cheaper)
float3 Result = Color * Color  // For gamma ~2.0
```

#### Constant Folding
**Before**:
```
float Result = Value * 0.5 + 0.3
```

**After** (pre-calculate constants):
```
float Result = Value * 0.5 + 0.3  // Compiler optimizes
// Or use material parameter for runtime control
```

#### Vector Operations
**Before** (scalar operations):
```
float R = Color.r * 0.3
float G = Color.g * 0.59
float B = Color.b * 0.11
float Luminance = R + G + B
```

**After** (vector operation):
```
float Luminance = dot(Color, float3(0.3, 0.59, 0.11))
```

**Savings**: ~50% instruction reduction

### 3. Material Functions

#### Reusable Logic
**Before** (duplicated in multiple materials):
```
Material1: [Complex PBR calculation]
Material2: [Same complex PBR calculation]
Material3: [Same complex PBR calculation]
```

**After** (shared material function):
```
MaterialFunction: PBR_Calculation
Material1: Call PBR_Calculation
Material2: Call PBR_Calculation
Material3: Call PBR_Calculation
```

**Benefits**:
- Reduced duplication
- Easier maintenance
- Consistent results
- Potential shader caching

#### Common Material Functions
```
- NormalBlending: Blend two normal maps
- ParallaxMapping: Height-based UV offset
- DetailMapping: Add detail texture
- ColorGrading: Adjust color
- FresnelEffect: Edge highlighting
```

### 4. Material Instances

#### Use Instances Instead of Unique Materials
**Before** (10 unique materials):
```
Material_Red: [Full material graph]
Material_Blue: [Full material graph]
Material_Green: [Full material graph]
...
```

**After** (1 master + 10 instances):
```
Master_Material: [Full material graph with parameters]
Instance_Red: Color = Red
Instance_Blue: Color = Blue
Instance_Green: Color = Green
```

**Benefits**:
- Reduced shader compilation
- Better batching
- Faster iteration
- Smaller package size

#### Parameter Types
```
Scalar: Single float value
Vector: Color, position, etc.
Texture: Texture reference
Static Switch: Compile-time branching
```

### 5. Static Switch Parameters

#### Compile-Time Branching
**Before** (runtime branching):
```
if (UseDetailTexture)
{
    Color = lerp(Color, DetailColor, DetailMask)
}
```

**After** (static switch):
```
StaticSwitch(UseDetailTexture)
{
    True: Color = lerp(Color, DetailColor, DetailMask)
    False: Color = Color
}
```

**Benefits**:
- No runtime branching cost
- Generates optimized shader variants
- Better performance

#### Common Static Switches
```
- UseNormalMap: Enable/disable normal mapping
- UseDetailTexture: Add detail layer
- UseParallax: Enable parallax mapping
- UseEmissive: Enable emissive
- TwoSided: Two-sided rendering
```

---

## Material Quality Levels

### Implementation
```cpp
// In material
QualitySwitch
{
    Low: [Simplified version]
    Medium: [Standard version]
    High: [Enhanced version]
    Epic: [Full quality version]
}
```

### Quality Level Guidelines

#### Epic Quality
- Full PBR workflow
- All texture maps
- Complex effects
- High sample counts

#### High Quality
- Standard PBR
- Essential textures
- Simplified effects
- Moderate samples

#### Medium Quality
- Simplified PBR
- Packed textures
- Basic effects
- Low samples

#### Low Quality
- Minimal shading
- Single texture
- No effects
- Minimal samples

---

## Horror Game Specific Optimizations

### 1. Darkness Optimization
**Strategy**: Simplify materials in dark areas

```cpp
// Adjust complexity based on lighting
float LightIntensity = saturate(dot(WorldNormal, LightDirection))
float Complexity = lerp(0.5, 1.0, LightIntensity)

// Use simpler calculations in dark areas
if (Complexity < 0.7)
{
    // Simplified shading
}
else
{
    // Full PBR
}
```

### 2. Emissive Optimization
**Challenge**: Glowing objects common in horror

**Optimization**:
```cpp
// Before (expensive)
Emissive = EmissiveTexture * EmissiveColor * Intensity * Pulse

// After (cheaper)
Emissive = EmissiveTexture * EmissiveColor
// Use post-process bloom for glow effect
```

### 3. Wet Surface Optimization
**Challenge**: Rain, blood, water effects

**Optimization**:
```cpp
// Before (expensive parallax + reflection)
UV = ParallaxMapping(UV, Height)
Reflection = ReflectionCapture(UV)
Color = lerp(BaseColor, Reflection, Wetness)

// After (simplified)
Roughness = lerp(BaseRoughness, 0.1, Wetness)
// Let Lumen handle reflections
```

### 4. Translucency Optimization
**Challenge**: Fog, glass, particles

**Optimization**:
- Use masked instead of translucent when possible
- Disable separate translucency
- Reduce translucent overdraw
- Use dithered opacity for distant objects

```ini
[/Script/Engine.RendererSettings]
r.SeparateTranslucency=0
r.TranslucencyLightingVolumeDim=32
```

---

## Material Blend Modes

### Performance Ranking (Fastest to Slowest)

1. **Opaque** (Fastest)
   - No transparency
   - Full depth buffer
   - Best for most surfaces

2. **Masked**
   - Binary transparency (on/off)
   - Depth buffer support
   - Good for foliage, fences

3. **Translucent** (Slow)
   - Smooth transparency
   - No depth writes
   - Expensive overdraw

4. **Additive** (Moderate)
   - Additive blending
   - Good for effects
   - Less overdraw than translucent

### Optimization Tips
```
- Use Opaque whenever possible
- Use Masked for binary transparency
- Avoid Translucent for large surfaces
- Use Additive for particles/effects
- Enable "Disable Depth Test" carefully
```

---

## Shading Models

### Performance Ranking

1. **Unlit** (Fastest)
   - No lighting calculations
   - Use for UI, effects
   - ~20 instructions

2. **Default Lit** (Standard)
   - Standard PBR
   - Most common
   - ~100-150 instructions

3. **Subsurface** (Expensive)
   - Subsurface scattering
   - Use sparingly
   - ~200-300 instructions

4. **Clear Coat** (Moderate)
   - Dual-layer specular
   - Good for specific materials
   - ~150-200 instructions

### Recommendations
```
Environment: Default Lit
Characters: Subsurface (skin only)
Props: Default Lit
UI: Unlit
Effects: Unlit or Additive
```

---

## Optimization Workflow

### Step 1: Analysis
```powershell
# Analyze material complexity
.\OptimizeMaterials.ps1 -DryRun

# Review report
# Identify complex materials
# Prioritize by usage frequency
```

### Step 2: Profiling
```
In UE5 Editor:
1. Open material
2. Window → Statistics
3. Review instruction count
4. Identify expensive nodes
```

### Step 3: Optimization
```powershell
# Apply optimizations
.\OptimizeMaterials.ps1

# Or manual optimization in editor
```

### Step 4: Validation
1. Check instruction count
2. Visual quality comparison
3. Performance testing
4. Shader compilation time

---

## Material Editor Tips

### Viewing Statistics
```
Window → Statistics
- Shows instruction count
- Texture sample count
- Shader complexity
- Compilation time
```

### Optimization Nodes
```
- StaticSwitch: Compile-time branching
- QualitySwitch: Quality level variants
- FeatureLevelSwitch: Platform variants
- MaterialFunctionCall: Reusable logic
```

### Debugging
```
- Use "Shader Complexity" view mode
- Red = Very expensive
- Yellow = Expensive
- Green = Cheap
- Blue = Very cheap
```

---

## Common Material Patterns

### Pattern 1: Standard PBR Material
```
Inputs:
- BaseColor (RGB texture)
- Normal (Normal map)
- ORM (Packed: AO, Roughness, Metallic)

Instructions: ~150
Samples: 6 (2 per texture × 3 textures)
```

### Pattern 2: Detail Material
```
Base Layer:
- BaseColor
- Normal
- ORM

Detail Layer:
- DetailColor (tiling)
- DetailNormal (tiling)
- DetailMask

Instructions: ~250
Samples: 12
```

### Pattern 3: Emissive Material
```
Base PBR +
- EmissiveTexture
- EmissiveColor parameter
- Intensity parameter

Instructions: ~180
Samples: 8
```

### Pattern 4: Masked Material
```
Base PBR +
- OpacityMask texture
- Clip threshold

Instructions: ~160
Samples: 8
```

---

## Optimization Checklist

### Pre-Optimization
- [ ] Profile material complexity
- [ ] Document current instruction count
- [ ] Identify most expensive materials
- [ ] Categorize by importance
- [ ] Create backup

### Optimization
- [ ] Pack texture channels
- [ ] Use material instances
- [ ] Add static switches
- [ ] Implement quality levels
- [ ] Optimize math operations
- [ ] Remove unused inputs
- [ ] Use material functions

### Post-Optimization
- [ ] Verify instruction count
- [ ] Visual quality check
- [ ] Performance testing
- [ ] Shader compilation time
- [ ] Document changes

---

## Common Issues & Solutions

### Issue: High Instruction Count
**Causes**:
- Too many texture samples
- Complex math operations
- Unnecessary calculations

**Solutions**:
- Pack texture channels
- Simplify math
- Use lookup textures
- Remove unused nodes

### Issue: Long Compilation Time
**Causes**:
- Too many static switches
- Complex material functions
- Many material instances

**Solutions**:
- Reduce static switch combinations
- Simplify material functions
- Use shared base materials

### Issue: Poor Batching
**Causes**:
- Too many unique materials
- Different blend modes
- Different shading models

**Solutions**:
- Use material instances
- Standardize blend modes
- Consistent shading models

### Issue: Shader Complexity Warnings
**Causes**:
- Exceeding instruction limits
- Too many interpolators
- Complex pixel shader

**Solutions**:
- Move calculations to vertex shader
- Reduce texture samples
- Simplify material graph

---

## Performance Budgets

### Per-Material Budgets
```
Hero Materials: 400 instructions max
Standard Materials: 300 instructions max
Background Materials: 200 instructions max
UI Materials: 100 instructions max
```

### Scene Budgets
```
Total Unique Materials: <100
Material Instances: Unlimited
Shader Variants: <500
Compilation Time: <5 minutes
```

---

## Monitoring & Maintenance

### Runtime Monitoring
```
Console Commands:
r.ShaderComplexity 1     - Show shader complexity
stat shaders             - Shader statistics
profilegpu               - GPU profiling
```

### Regular Audits
- Weekly: Check new materials
- Monthly: Full material audit
- Per-Release: Comprehensive optimization

---

## Best Practices Summary

1. **Use Material Instances**: Avoid unique materials
2. **Pack Channels**: Combine single-channel textures
3. **Static Switches**: Compile-time branching
4. **Material Functions**: Reuse common logic
5. **Quality Levels**: Scale complexity
6. **Optimize Math**: Avoid expensive operations
7. **Reduce Samples**: Minimize texture lookups
8. **Profile Regularly**: Monitor complexity
9. **Visual Validation**: Maintain quality
10. **Document Changes**: Track optimizations

---

## Quick Reference

### Optimization Command
```powershell
# Preview optimizations
.\OptimizeMaterials.ps1 -DryRun

# Apply optimizations
.\OptimizeMaterials.ps1
```

### Target Metrics
- Instructions: <300
- Texture Samples: <8
- Compilation: <30s
- Shader Complexity: Green/Yellow

### Optimization Priority
1. Pack texture channels
2. Use material instances
3. Add static switches
4. Optimize math operations
5. Implement quality levels
