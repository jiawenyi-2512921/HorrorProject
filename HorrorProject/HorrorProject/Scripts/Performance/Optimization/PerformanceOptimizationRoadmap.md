# Performance Optimization Roadmap

## HorrorProject - Path to 60 FPS @ Epic Quality

**Target**: Stable 60 FPS at Epic quality settings
**Timeline**: Phased approach with measurable milestones
**Current Status**: Baseline measurement required

---

## Phase 1: Baseline & Analysis (Week 1)

### Objectives
- Establish current performance baseline
- Identify primary bottlenecks
- Prioritize optimization targets

### Tasks

#### 1.1 Performance Profiling
```powershell
# Run all profiling tools
.\ProfileGameplay.ps1 -DurationSeconds 300
.\ProfileRendering.ps1
.\ProfileMemory.ps1
.\ProfileLoading.ps1
```

**Expected Outputs**:
- Average FPS across quality settings
- Frame time breakdown (Game/Render/GPU)
- Memory usage patterns
- Loading time metrics

#### 1.2 Benchmark Baseline
```powershell
# Establish baseline metrics
.\BenchmarkFramerate.ps1 -DurationSeconds 120
.\BenchmarkMemory.ps1
.\BenchmarkLoading.ps1 -Iterations 3
```

**Success Criteria**:
- Complete benchmark data for all quality levels
- Identified top 3 performance bottlenecks
- Documented current state

---

## Phase 2: Quick Wins (Week 2)

### Objectives
- Implement low-effort, high-impact optimizations
- Target 10-15% performance improvement
- Build optimization momentum

### 2.1 Texture Optimization
**Impact**: High | **Effort**: Low

```powershell
# Analyze and optimize textures
.\OptimizeTextures.ps1 -DryRun  # Preview first
.\OptimizeTextures.ps1           # Apply optimizations
```

**Targets**:
- Reduce texture memory by 30%
- Limit base color textures to 2K
- Limit normal maps to 2K
- Limit roughness/metallic to 1K
- Compress UI textures to 1K

**Expected Gain**: +5-8 FPS

### 2.2 Material Simplification
**Impact**: High | **Effort**: Medium

```powershell
# Identify and optimize complex materials
.\OptimizeMaterials.ps1 -DryRun
.\OptimizeMaterials.ps1
```

**Targets**:
- Reduce shader instructions to <300 per material
- Limit texture samples to <8 per material
- Use material instances instead of unique materials
- Pack texture channels (combine R/G/B/A)

**Expected Gain**: +3-5 FPS

### 2.3 Validation
```powershell
.\ValidateOptimizations.ps1
.\BenchmarkFramerate.ps1
.\ComparePerformance.ps1
```

**Success Criteria**:
- 10-15% FPS improvement
- No visual quality regression
- Memory usage reduced by 20-30%

---

## Phase 3: Rendering Optimization (Week 3-4)

### Objectives
- Optimize rendering pipeline
- Target 15-20% additional improvement
- Focus on GPU bottlenecks

### 3.1 Lighting Optimization
**Impact**: Very High | **Effort**: Medium

```powershell
.\OptimizeLighting.ps1
```

**Implementation**:
1. **Enable Lumen** (UE5 dynamic GI)
   - Better performance than traditional lightmaps
   - Dynamic lighting without baking
   - Configure quality for 60 FPS target

2. **Shadow Optimization**
   - Enable Virtual Shadow Maps (VSM)
   - Limit shadow-casting lights to 2-3 per scene
   - Reduce shadow distance to 5000 units
   - Use 3 cascade levels maximum

3. **Light Count Management**
   - Maximum 4 dynamic lights per scene
   - Convert static lights where possible
   - Use light functions sparingly

**Configuration** (DefaultEngine.ini):
```ini
[/Script/Engine.RendererSettings]
r.Lumen.DiffuseIndirect.Allow=True
r.Lumen.Reflections.Allow=True
r.Shadow.Virtual.Enable=1
r.Shadow.Virtual.MaxPhysicalPages=4096
r.Shadow.DistanceScale=0.5
r.Shadow.CSM.MaxCascades=3
```

**Expected Gain**: +8-12 FPS

### 3.2 Mesh & LOD Optimization
**Impact**: High | **Effort**: High

```powershell
.\OptimizeMeshes.ps1
```

**Implementation**:
1. **LOD Generation**
   - LOD0: Full detail (0m)
   - LOD1: 50% reduction (10m)
   - LOD2: 75% reduction (25m)
   - LOD3: 90% reduction (50m)

2. **Mesh Complexity**
   - Limit hero meshes to 50K triangles
   - Limit props to 10K triangles
   - Use Nanite for static complex meshes

3. **Instancing**
   - Use instanced static meshes for repeated objects
   - Enable mesh merging for small props
   - Implement HLOD for distant objects

**Expected Gain**: +5-8 FPS

### 3.3 Post-Processing Optimization
**Impact**: Medium | **Effort**: Low

**Implementation**:
- Reduce bloom quality
- Optimize depth of field
- Limit motion blur samples
- Use temporal AA instead of MSAA
- Optimize screen space reflections

**Expected Gain**: +2-4 FPS

**Success Criteria**:
- 40-50 FPS at Epic quality
- GPU time <16ms
- Maintained visual quality for horror atmosphere

---

## Phase 4: Memory & Streaming (Week 5)

### Objectives
- Optimize memory usage
- Implement aggressive streaming
- Eliminate loading hitches

### 4.1 Texture Streaming
**Impact**: High | **Effort**: Medium

**Configuration** (DefaultEngine.ini):
```ini
[/Script/Engine.RendererSettings]
r.Streaming.PoolSize=3000
r.Streaming.MaxEffectiveScreenSize=0
r.Streaming.FullyLoadUsedTextures=0
r.Streaming.UseAllMips=0
```

**Implementation**:
- Enable texture streaming globally
- Set streaming pool to 3GB
- Use distance-based mip selection
- Implement texture LOD bias

**Expected Impact**: Reduced memory by 40%

### 4.2 Level Streaming
**Impact**: Very High | **Effort**: High

**Implementation**:
1. **Split Large Levels**
   - Main persistent level (core gameplay)
   - Streaming sub-levels (areas)
   - Distance-based loading/unloading

2. **Async Loading**
   - Preload next areas
   - Background asset loading
   - Smooth transitions

3. **Asset Prioritization**
   - Critical assets load first
   - Background assets load async
   - Unused assets unload aggressively

**Expected Impact**: 
- Loading time <15 seconds
- Eliminated runtime hitches
- Memory usage <6GB

### 4.3 Audio Streaming
**Impact**: Medium | **Effort**: Low

**Implementation**:
- Stream large audio files
- Use compressed audio formats
- Limit simultaneous audio sources

**Expected Impact**: Reduced memory by 200-500MB

**Success Criteria**:
- Memory usage <6GB
- No loading hitches during gameplay
- Smooth level transitions

---

## Phase 5: Final Polish (Week 6)

### Objectives
- Achieve 60 FPS target
- Optimize edge cases
- Final validation

### 5.1 Blueprint Optimization
**Impact**: Medium | **Effort**: Medium

**Implementation**:
- Disable tick on unnecessary actors
- Use event-driven logic instead of tick
- Optimize collision checks
- Cache expensive calculations
- Use object pooling for spawned actors

**Expected Gain**: +2-4 FPS

### 5.2 Particle & VFX Optimization
**Impact**: Medium | **Effort**: Low

**Implementation**:
- Reduce particle counts
- Use GPU particles (Niagara)
- Implement LOD for particle systems
- Limit simultaneous effects

**Expected Gain**: +2-3 FPS

### 5.3 Occlusion Culling
**Impact**: High | **Effort**: Medium

**Implementation**:
- Enable precomputed visibility
- Use occlusion volumes
- Implement distance culling
- Optimize view frustum culling

**Expected Gain**: +3-5 FPS

**Success Criteria**:
- Stable 60 FPS at Epic quality
- 1% low FPS >50
- Memory <6GB
- Loading <15 seconds

---

## Phase 6: Validation & Documentation (Week 7)

### 6.1 Final Benchmarking
```powershell
# Run complete benchmark suite
.\BenchmarkFramerate.ps1 -DurationSeconds 300
.\BenchmarkMemory.ps1
.\BenchmarkLoading.ps1 -Iterations 5

# Generate reports
.\GenerateBenchmarkReport.ps1
.\ComparePerformance.ps1
.\GenerateOptimizationReport.ps1
```

### 6.2 Quality Assurance
- Visual quality verification
- Horror atmosphere maintained
- No gameplay regressions
- Cross-hardware testing

### 6.3 Documentation
- Document all optimizations applied
- Create performance maintenance guide
- Establish performance budgets
- Set up continuous monitoring

---

## Performance Targets

### Primary Target (Epic Quality)
| Metric | Target | Acceptable | Critical |
|--------|--------|------------|----------|
| Average FPS | ≥60 | ≥50 | <30 |
| 1% Low FPS | ≥50 | ≥40 | <25 |
| Frame Time | ≤16.67ms | ≤20ms | >33ms |
| GPU Time | ≤14ms | ≤16ms | >20ms |
| Memory | ≤6GB | ≤8GB | >10GB |
| Loading | ≤15s | ≤30s | >45s |

### Quality Level Targets
| Quality | Target FPS | Min FPS | Frame Time |
|---------|------------|---------|------------|
| Low | 120+ | 90+ | <8.3ms |
| Medium | 90+ | 70+ | <11.1ms |
| High | 75+ | 60+ | <13.3ms |
| Epic | 60+ | 50+ | <16.7ms |

---

## Optimization Budget

### Asset Budgets
- **Textures**: 2-3GB streaming pool
- **Meshes**: 1-2GB
- **Audio**: 300-500MB
- **Blueprints**: 200-300MB
- **Other**: 500MB

### Scene Budgets (Per Frame)
- **Draw Calls**: <3000
- **Triangles**: <5M visible
- **Dynamic Lights**: ≤4
- **Shadow-Casting Lights**: ≤2
- **Particle Systems**: ≤10 active

---

## Risk Mitigation

### High-Risk Areas
1. **Lighting Changes**: May affect horror atmosphere
   - Mitigation: Incremental changes with visual review
   
2. **Texture Downscaling**: May reduce visual quality
   - Mitigation: Selective optimization, preserve hero assets
   
3. **LOD Popping**: May be noticeable
   - Mitigation: Smooth transitions, dithered LODs

### Rollback Plan
- Maintain backup before each phase
- Version control all configuration changes
- Document all modifications
- Keep baseline benchmarks for comparison

---

## Tools & Scripts

### Profiling
- `ProfileGameplay.ps1` - Runtime performance analysis
- `ProfileRendering.ps1` - GPU and rendering metrics
- `ProfileMemory.ps1` - Memory usage tracking
- `ProfileLoading.ps1` - Loading time analysis

### Optimization
- `OptimizeTextures.ps1` - Texture size and compression
- `OptimizeMaterials.ps1` - Material complexity reduction
- `OptimizeMeshes.ps1` - LOD generation and mesh optimization
- `OptimizeLighting.ps1` - Lighting setup optimization

### Benchmarking
- `BenchmarkFramerate.ps1` - FPS testing across quality levels
- `BenchmarkMemory.ps1` - Memory usage scenarios
- `BenchmarkLoading.ps1` - Loading time measurement
- `GenerateBenchmarkReport.ps1` - Comprehensive HTML report

### Validation
- `ValidateOptimizations.ps1` - Verify optimizations applied
- `ComparePerformance.ps1` - Before/after comparison
- `GenerateOptimizationReport.ps1` - Optimization status report

---

## Success Metrics

### Technical Metrics
- ✅ 60 FPS average at Epic quality
- ✅ <16.67ms frame time
- ✅ <6GB memory usage
- ✅ <15s loading time
- ✅ No runtime hitches

### Quality Metrics
- ✅ Horror atmosphere maintained
- ✅ Visual fidelity preserved
- ✅ No gameplay impact
- ✅ Smooth performance across scenes

### Process Metrics
- ✅ All optimizations documented
- ✅ Performance budgets established
- ✅ Monitoring tools in place
- ✅ Team trained on optimization workflow

---

## Continuous Optimization

### Daily Monitoring
- Run quick performance checks
- Monitor for regressions
- Track asset additions

### Weekly Reviews
- Full benchmark suite
- Performance trend analysis
- Budget compliance check

### Monthly Audits
- Comprehensive optimization review
- Update performance baselines
- Refine optimization strategies

---

## Contact & Support

For questions or issues with optimization:
1. Review this roadmap
2. Check tool documentation
3. Run validation scripts
4. Compare with baseline metrics

**Remember**: Optimization is iterative. Measure, optimize, validate, repeat.
