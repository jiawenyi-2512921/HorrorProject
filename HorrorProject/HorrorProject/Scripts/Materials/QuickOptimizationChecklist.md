# Material Optimization Quick Checklist

## 🎯 Quick Reference

### Performance Targets
- **Opaque:** <200 instructions, <12 texture samples
- **Masked:** <220 instructions, <14 texture samples
- **Translucent:** <250 instructions, <14 texture samples
- **PostProcess:** <300 instructions, <16 texture samples

### Current Status (HorrorProject)
- ✅ **Total Materials:** 310
- ⚠️ **Need Optimization:** 177 materials (57%)
- 🔴 **Critical (>100KB):** 160 materials
- 🟡 **High (50-100KB):** 17 materials

---

## 🔥 Top 10 Optimization Techniques

### 1. Texture Packing
**Impact:** High | **Effort:** Low
- Pack Occlusion(R), Roughness(G), Metallic(B) into single texture
- Reduces texture samples by 66%
- Standard industry practice

**Before:** 3 texture samples
```
Occlusion = Texture Sample (T_AO)
Roughness = Texture Sample (T_Roughness)
Metallic = Texture Sample (T_Metallic)
```

**After:** 1 texture sample
```
ORM = Texture Sample (T_ORM)
Occlusion = ORM.r
Roughness = ORM.g
Metallic = ORM.b
```

### 2. Remove Redundant Nodes
**Impact:** Medium | **Effort:** Low
- Delete disconnected nodes
- Remove duplicate calculations
- Clean up unused parameters

**Check for:**
- Unused texture samples
- Disconnected math nodes
- Duplicate parameter nodes
- Unused material functions

### 3. Use Material Functions
**Impact:** High | **Effort:** Medium
- Reuse common logic across materials
- Easier to optimize centrally
- Reduces material complexity

**Common Functions:**
- MF_Parallax
- MF_DetailNormal
- MF_Wetness
- MF_TriplanarMapping

### 4. Implement Quality Switches
**Impact:** High | **Effort:** Medium
- Use StaticSwitchParameter for quality levels
- Disable expensive features on Low/Medium
- Enable scalability settings

**Example:**
```
StaticSwitch (QualityLevel)
├─ Low: Base textures only
├─ Medium: + Detail normal
├─ High: + Parallax
└─ Epic: + All effects
```

### 5. Optimize Translucent Materials
**Impact:** Very High | **Effort:** Medium
- Use Masked instead of Translucent when possible
- Reduce transparent area
- Simplify lighting calculations
- Minimize overdraw

**Checklist:**
- [ ] Can this be Masked instead?
- [ ] Is transparent area minimized?
- [ ] Are lighting calculations simplified?
- [ ] Is blend mode optimal?

### 6. Reduce Texture Samples
**Impact:** High | **Effort:** Low-Medium
- Reuse texture samples
- Use texture packing
- Remove unnecessary detail textures
- Share textures between channels

**Target Reduction:**
- Low: <8 samples
- Medium: <12 samples
- High: <14 samples
- Epic: <16 samples

### 7. Simplify Math Operations
**Impact:** Medium | **Effort:** Low
- Use cheaper approximations
- Combine operations
- Remove unnecessary precision
- Use built-in functions

**Examples:**
- Use `Power` instead of multiple `Multiply`
- Use `Lerp` instead of manual blend
- Use `Saturate` instead of `Clamp(0,1)`
- Use `OneMinus` instead of `1 - x`

### 8. Optimize PostProcess Materials
**Impact:** Very High | **Effort:** High
- Reduce SceneTexture samples (<8)
- Avoid loops and branches
- Use LUT textures for complex color grading
- Consider Compute Shader alternatives

**Critical Optimizations:**
- Cache SceneTexture samples
- Use half-resolution for blur effects
- Implement quality switches
- Profile with GPU profiler

### 9. Use Static Switches (Not Dynamic)
**Impact:** High | **Effort:** Low
- StaticSwitchParameter compiles out unused branches
- Dynamic branches add instruction overhead
- Use for quality levels and feature toggles

**Static vs Dynamic:**
- ✅ Static: Compiled out, zero cost
- ❌ Dynamic: Runtime cost, adds instructions

### 10. Enable Material Quality Levels
**Impact:** High | **Effort:** Medium
- Configure in Project Settings
- Use StaticSwitch for quality
- Test all quality levels
- Set appropriate defaults

**Configuration:**
```
Project Settings > Rendering > Quality
├─ Low: Mobile/Low-end PC
├─ Medium: Mid-range PC
├─ High: High-end PC
└─ Epic: Ultra settings
```

---

## 🔍 Optimization Workflow

### Phase 1: Identify (5 min)
1. Open material in editor
2. Window > Statistics
3. Check instruction count
4. Check texture sample count
5. Note current complexity

### Phase 2: Analyze (10 min)
1. Review material graph
2. Identify redundant nodes
3. Find optimization opportunities
4. Check for texture packing
5. Review parameter usage

### Phase 3: Optimize (20-30 min)
1. Pack textures (ORM format)
2. Remove redundant nodes
3. Add quality switches
4. Simplify math operations
5. Reduce texture samples
6. Use material functions

### Phase 4: Validate (5 min)
1. Check Statistics window
2. Verify instruction count reduction
3. Test in Shader Complexity view
4. Test all quality levels
5. Visual quality check

### Phase 5: Document (2 min)
1. Update material description
2. Note optimization changes
3. Document quality levels
4. Update instance parameters

---

## 🎨 Material Type Specific Tips

### Opaque Materials (Walls, Floors, Props)
- ✅ Use texture packing (ORM)
- ✅ Add detail normal for close-up
- ✅ Use parallax for depth
- ✅ Implement quality switches
- ❌ Avoid unnecessary emissive
- ❌ Don't use expensive effects

**Target:** <200 instructions, <12 samples

### Masked Materials (Foliage, Fences)
- ✅ Use dithered LOD transitions
- ✅ Optimize opacity mask
- ✅ Use simple alpha test
- ✅ Enable two-sided only if needed
- ❌ Avoid complex opacity calculations
- ❌ Don't use expensive blending

**Target:** <220 instructions, <14 samples

### Translucent Materials (Glass, Water)
- ✅ Minimize transparent area
- ✅ Use simple blend modes
- ✅ Simplify lighting
- ✅ Add quality switches for refraction
- ❌ Avoid complex refraction
- ❌ Minimize overdraw
- ❌ Don't use expensive effects

**Target:** <250 instructions, <14 samples

### PostProcess Materials (VHS, Blur, etc)
- ✅ Cache SceneTexture samples
- ✅ Use LUT for color grading
- ✅ Implement quality switches
- ✅ Use half-resolution for blur
- ❌ Avoid loops and branches
- ❌ Minimize SceneTexture samples (<8)
- ❌ Don't use expensive calculations

**Target:** <300 instructions, <16 samples

---

## 🛠️ UE5 Editor Commands

### Shader Complexity View
```
viewmode shadercomplexity
```
- Green: Good (<200 instructions)
- Yellow: OK (200-300 instructions)
- Red: Bad (>300 instructions)
- White: Very Bad (>600 instructions)

### Material Statistics
```
stat shadercompiling
stat materials
stat textures
```

### Recompile Shaders
```
recompileshaders changed
recompileshaders all
recompileshaders material [MaterialName]
```

### Performance Profiling
```
profilegpu
stat gpu
stat unit
```

---

## 📊 Before/After Checklist

### Before Optimization
- [ ] Document current instruction count
- [ ] Document current texture samples
- [ ] Take screenshot of material graph
- [ ] Note visual quality
- [ ] Check Shader Complexity view

### After Optimization
- [ ] Verify instruction count reduction
- [ ] Verify texture sample reduction
- [ ] Compare material graph
- [ ] Verify visual quality maintained
- [ ] Check Shader Complexity view
- [ ] Test all quality levels
- [ ] Update documentation

---

## 🚨 Common Mistakes to Avoid

### ❌ Don't Do This
1. **Using dynamic branches** - Use StaticSwitchParameter instead
2. **Sampling same texture multiple times** - Cache the sample
3. **Complex math in pixel shader** - Move to vertex shader or precompute
4. **Unnecessary precision** - Use half precision when possible
5. **Too many parameters** - Consolidate related parameters
6. **No quality switches** - Always implement scalability
7. **Ignoring texture packing** - Always use ORM format
8. **Complex translucent materials** - Simplify or use masked

### ✅ Do This Instead
1. **Use static switches** - Zero runtime cost
2. **Cache texture samples** - Reuse results
3. **Precompute in textures** - Use LUT textures
4. **Use appropriate precision** - Half when possible
5. **Group parameters** - Use parameter groups
6. **Add quality levels** - Low/Medium/High/Epic
7. **Pack textures** - ORM standard format
8. **Optimize blend modes** - Use simplest mode

---

## 📈 Success Metrics

### Project Goals
- Reduce high complexity materials from 177 to <50
- Achieve <300 instructions for all materials
- Implement quality scaling for all materials
- Standardize on master material system

### Per-Material Goals
- 30-50% instruction count reduction
- 20-40% texture sample reduction
- Maintain visual quality
- Add quality level support

### Team Goals
- Optimize 10 materials per day
- Complete audit in 1 week
- Migrate to master materials in 2 weeks
- Full optimization in 1 month

---

## 🎓 Learning Resources

### UE5 Documentation
- Material Editor Reference
- Material Optimization Guide
- Shader Complexity Guide
- Performance Guidelines

### Tools
- Material Statistics Window
- Shader Complexity View
- GPU Profiler
- Material Instance Editor

### Best Practices
- PBR Texture Guide
- Material Quality Scaling
- Performance Optimization
- Mobile Material Guidelines

---

**Quick Start:** Run `AnalyzeShaders.ps1` to identify materials needing optimization, then follow this checklist for each material.

**Team:** SM13_Materials (Team 43)
**Project:** HorrorProject
**Updated:** 2026-04-26
