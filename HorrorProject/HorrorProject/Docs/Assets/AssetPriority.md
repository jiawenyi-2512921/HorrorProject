# Asset Optimization Priority - HorrorProject

**Generated:** 2026-04-26  
**Purpose:** Prioritize asset optimization work for maximum performance impact  
**Target:** Achieve 60 FPS on mid-range hardware (RTX 3060, 16GB RAM)

---

## Priority System

- 🔴 **CRITICAL:** Blocking performance, must fix immediately
- 🟠 **HIGH:** Significant impact, fix within 1 week
- 🟡 **MEDIUM:** Moderate impact, fix within 2-3 weeks
- 🟢 **LOW:** Minor impact, fix when time permits
- ⚪ **DEFERRED:** Future optimization

---

## Critical Priority (Week 1)

### 🔴 P0: SD_Art Texture Compression
**Impact:** 4-6 GB VRAM reduction  
**Effort:** 2-3 hours  
**ROI:** Extremely High

**Problem:**
- 8K textures consuming excessive VRAM
- Causing texture streaming issues
- Potential out-of-memory crashes

**Action Items:**
1. Identify all 8K textures in SD_Art
2. Downscale to 4K (or 2K for non-hero assets)
3. Re-compress with BC7 (color) and BC5 (normal)
4. Enable texture streaming
5. Test memory usage

**Script:** `Scripts/Assets/CompressTextures.ps1`

**Expected Results:**
- VRAM usage: 12 GB → 6-8 GB
- Load times: -30%
- No visual quality loss at gameplay distance

---

### 🔴 P1: DeepWaterStation LOD Generation
**Impact:** 30-40% GPU performance improvement  
**Effort:** 4-6 hours  
**ROI:** Very High

**Problem:**
- 180 meshes with no LODs
- Full detail rendered at all distances
- Overdraw and vertex processing bottleneck

**Action Items:**
1. Generate LOD chain for all SM_* meshes
   - LOD0: Original (0-10m)
   - LOD1: 50% reduction (10-25m)
   - LOD2: 75% reduction (25-50m)
   - LOD3: 90% reduction (50m+)
2. Set up LOD screen size thresholds
3. Enable HLOD for distant objects
4. Test performance in large rooms

**Tools:**
- UE5 Automatic LOD Generation
- Simplygon (if available)
- Manual reduction for hero assets

**Expected Results:**
- Draw calls: -40%
- Triangle count: -60% average
- FPS improvement: +15-25 FPS

---

### 🔴 P2: Material Instance Consolidation
**Impact:** 20-30% material shader compilation reduction  
**Effort:** 3-4 hours  
**ROI:** High

**Problem:**
- 320+ material instances across packages
- Many duplicates with identical parameters
- Shader compilation overhead

**Action Items:**
1. Audit all MI_* materials
2. Identify duplicates and near-duplicates
3. Consolidate to shared material instances
4. Create material parameter collections
5. Update mesh references

**Target:**
- Reduce 320 instances → 150 unique instances
- Create 5-10 master materials
- Use material parameter collections for global changes

**Expected Results:**
- Shader compilation time: -50%
- Material memory: -30%
- Easier material management

---

## High Priority (Week 1-2)

### 🟠 P3: Audio Asset Migration & Compression
**Impact:** 600 MB disk space, 150 MB runtime memory  
**Effort:** 4-5 hours  
**ROI:** High

**Problem:**
- 354 audio files not yet integrated
- Likely uncompressed or inefficiently compressed
- No streaming setup

**Action Items:**
1. **Grimytheus_Vol_2 (150 files):**
   - Import to Content/Audio/Horror/
   - Compress to OGG Vorbis (quality 0.7)
   - Create sound cues
   - Set up attenuation
   - Enable streaming for files >5s

2. **SoundsOfHorror (204 files):**
   - Import to Content/Audio/Ambient/
   - Compress to OGG Vorbis (quality 0.6)
   - Create ambient sound actors
   - Set up reverb zones

**Expected Results:**
- Disk space: 864 MB → 250 MB
- Runtime memory: 200 MB → 50 MB (streaming)
- Audio quality: Maintained

**Script:** `Scripts/Assets/Migration/AudioMigration.ps1`

---

### 🟠 P4: Texture Streaming Configuration
**Impact:** 2-3 GB VRAM reduction  
**Effort:** 2 hours  
**ROI:** High

**Problem:**
- All textures loaded at full resolution
- No streaming pool configuration
- Texture thrashing on lower-end hardware

**Action Items:**
1. Enable texture streaming globally
2. Configure streaming pool size (2-4 GB)
3. Set texture group LOD bias
4. Enable virtual textures for large surfaces
5. Test on target hardware

**Configuration:**
```ini
[/Script/Engine.RendererSettings]
r.Streaming.PoolSize=3072
r.Streaming.MaxEffectiveScreenSize=0
r.VirtualTexture=True
r.VT.EnableAutoImport=True
```

**Expected Results:**
- VRAM usage: -2-3 GB
- Texture pop-in: Minimal (with proper LOD bias)
- Support for lower-end GPUs

---

### 🟠 P5: Lightmap Resolution Optimization
**Impact:** 500 MB-1 GB memory, faster builds  
**Effort:** 3 hours  
**ROI:** Medium-High

**Problem:**
- Default lightmap resolution (64) too high for many assets
- Excessive lightmap memory usage
- Long lighting build times

**Action Items:**
1. Audit mesh lightmap resolutions
2. Reduce for small/distant objects:
   - Small props: 32 or 16
   - Medium props: 64
   - Large surfaces: 128-256
   - Hero assets: 512 max
3. Use lightmap UV optimization
4. Test lighting quality

**Expected Results:**
- Lightmap memory: -500 MB to -1 GB
- Build time: -30-40%
- Visual quality: Maintained

---

## Medium Priority (Week 2-3)

### 🟡 P6: Nanite Conversion for Static Meshes
**Impact:** Variable, potentially 20-30% performance  
**Effort:** 6-8 hours  
**ROI:** Medium

**Problem:**
- High-poly meshes not using Nanite
- Manual LOD management overhead
- Potential for better detail at distance

**Action Items:**
1. Identify candidates (>10K triangles, static)
2. Enable Nanite on suitable meshes
3. Test performance vs traditional LODs
4. Disable for problematic assets (transparency, WPO)

**Candidates:**
- DeepWaterStation architecture (50+ meshes)
- SD_Art building pieces (100+ meshes)
- Large props and structures

**Expected Results:**
- Better detail at all distances
- Simplified LOD management
- Potential performance improvement (test required)

---

### 🟡 P7: Blueprint Optimization
**Impact:** 5-10 FPS in complex scenes  
**Effort:** 4-5 hours  
**ROI:** Medium

**Problem:**
- Tick-heavy blueprints
- Inefficient event graphs
- No performance profiling

**Action Items:**
1. Profile BP_HorrorPlayerCharacter
2. Reduce tick frequency where possible
3. Use timers instead of tick for periodic checks
4. Optimize collision checks
5. Cache component references

**Focus Areas:**
- Camera shake calculations
- Footstep detection
- Interaction raycasts

**Expected Results:**
- Blueprint CPU time: -30-40%
- FPS improvement: +5-10 FPS in complex scenes

---

### 🟡 P8: Collision Optimization
**Impact:** 10-15% physics performance  
**Effort:** 3-4 hours  
**ROI:** Medium

**Problem:**
- Complex collision meshes
- Unnecessary collision on decorative objects
- No collision LODs

**Action Items:**
1. Audit collision complexity
2. Replace complex collision with simple shapes
3. Disable collision on non-interactive props
4. Use collision LODs for distant objects
5. Optimize collision channels

**Expected Results:**
- Physics CPU time: -30-40%
- Fewer collision checks
- Smoother player movement

---

### 🟡 P9: Particle System Optimization
**Impact:** 5-10 FPS in particle-heavy scenes  
**Effort:** 2-3 hours  
**ROI:** Medium

**Problem:**
- Niagara systems not optimized
- High particle counts
- No LOD or culling

**Action Items:**
1. Audit all Niagara systems
2. Reduce max particle counts
3. Enable distance culling
4. Use GPU particles where possible
5. Implement particle LODs

**Focus Systems:**
- Water drips
- Steam/fog
- Dust motes

**Expected Results:**
- Particle count: -40-50%
- GPU particle time: -30%
- FPS improvement: +5-10 FPS

---

## Low Priority (Week 3-4)

### 🟢 P10: Asset Naming Standardization
**Impact:** Workflow efficiency, no performance  
**Effort:** 4-6 hours  
**ROI:** Low (Quality of Life)

**Problem:**
- Inconsistent naming across packages
- Difficult to find assets
- Breaks automation scripts

**Action Items:**
1. Run naming validation script
2. Rename non-compliant assets
3. Update references automatically
4. Document naming conventions

**Script:** `Scripts/Assets/ValidateAssetNaming.ps1`

---

### 🟢 P11: Unused Asset Removal
**Impact:** 250-500 MB disk space  
**Effort:** 2-3 hours  
**ROI:** Low

**Problem:**
- 229 unused assets (12% of total)
- Wasted disk space
- Cluttered content browser

**Action Items:**
1. Run unused asset detection
2. Verify assets are truly unused
3. Move to Archive folder (don't delete immediately)
4. Test project thoroughly
5. Delete after 1 week if no issues

**Script:** `Scripts/Assets/FindUnusedAssets.ps1`

**Candidates:**
- Demo content (50 MB)
- Duplicate materials (10 MB)
- Unused variations (20 MB)
- Stone_Golem (142 MB - if not using)

---

### 🟢 P12: Virtual Texture Implementation
**Impact:** 1-2 GB VRAM, better quality  
**Effort:** 6-8 hours  
**ROI:** Low-Medium

**Problem:**
- Large surfaces using traditional textures
- Texture resolution limitations
- VRAM constraints

**Action Items:**
1. Identify large surfaces (floors, walls)
2. Convert to Runtime Virtual Textures
3. Set up RVT volumes
4. Configure streaming
5. Test performance

**Candidates:**
- Large floor sections
- Wall panels
- Terrain (if added)

**Expected Results:**
- VRAM usage: -1-2 GB
- Higher effective resolution
- Better texture streaming

---

## Deferred Priority (Month 2+)

### ⚪ P13: HLOD Setup
**Impact:** 20-30% performance in large levels  
**Effort:** 8-10 hours  
**ROI:** Deferred (for larger levels)

**Reason:** Current level size doesn't justify HLOD complexity yet

---

### ⚪ P14: World Partition Migration
**Impact:** Better streaming, larger worlds  
**Effort:** 10-15 hours  
**ROI:** Deferred (for open world expansion)

**Reason:** Current level works well with traditional streaming

---

### ⚪ P15: Lumen Optimization
**Impact:** 10-20 FPS  
**Effort:** 6-8 hours  
**ROI:** Deferred (after core optimization)

**Reason:** Focus on asset optimization first

---

## Optimization Roadmap

### Week 1: Critical Performance
- Day 1-2: SD_Art texture compression (P0)
- Day 3-4: DeepWaterStation LOD generation (P1)
- Day 5: Material consolidation (P2)

**Expected Improvement:** +20-30 FPS, -4-6 GB VRAM

### Week 2: High Priority
- Day 1-2: Audio migration & compression (P3)
- Day 3: Texture streaming setup (P4)
- Day 4-5: Lightmap optimization (P5)

**Expected Improvement:** +5-10 FPS, -1-2 GB VRAM, -600 MB disk

### Week 3: Medium Priority
- Day 1-2: Nanite conversion (P6)
- Day 3: Blueprint optimization (P7)
- Day 4: Collision optimization (P8)
- Day 5: Particle optimization (P9)

**Expected Improvement:** +10-15 FPS, better scalability

### Week 4: Polish & Cleanup
- Day 1-2: Asset naming (P10)
- Day 3: Unused asset removal (P11)
- Day 4-5: Virtual textures (P12)

**Expected Improvement:** Better workflow, -500 MB disk

---

## Performance Targets

### Current (Estimated)
- **FPS:** 30-40 FPS (RTX 3060, 1080p, Epic settings)
- **VRAM:** 10-12 GB
- **RAM:** 12-14 GB
- **Load Time:** 45-60 seconds

### After Week 1 (Critical)
- **FPS:** 50-60 FPS
- **VRAM:** 6-8 GB
- **RAM:** 10-12 GB
- **Load Time:** 30-40 seconds

### After Week 2 (High Priority)
- **FPS:** 55-65 FPS
- **VRAM:** 5-7 GB
- **RAM:** 8-10 GB
- **Load Time:** 25-35 seconds

### After Week 3 (Medium Priority)
- **FPS:** 60-70 FPS
- **VRAM:** 4-6 GB
- **RAM:** 8-10 GB
- **Load Time:** 20-30 seconds

### Target (After Week 4)
- **FPS:** 60+ FPS (RTX 3060, 1080p, High settings)
- **VRAM:** 4-5 GB
- **RAM:** 8 GB
- **Load Time:** 20-25 seconds

---

## Validation & Testing

### Performance Profiling
Run after each optimization phase:
1. **GPU Profiler:** `stat GPU`
2. **CPU Profiler:** `stat SceneRendering`
3. **Memory:** `stat Memory`
4. **Texture Streaming:** `stat Streaming`

### Test Scenarios
1. **Stress Test:** Large room with many assets
2. **Particle Test:** Multiple particle systems active
3. **Audio Test:** Multiple audio sources playing
4. **Streaming Test:** Fast movement through level

### Acceptance Criteria
- 60 FPS minimum on RTX 3060 (1080p High)
- No texture pop-in during normal gameplay
- Load times under 30 seconds
- No memory leaks during extended play

---

## Risk Assessment

### High Risk
- **Nanite conversion (P6):** May not work with all assets
  - Mitigation: Test thoroughly, keep traditional LODs as backup

### Medium Risk
- **Material consolidation (P2):** May break existing references
  - Mitigation: Use UE5 reference updater, test thoroughly

### Low Risk
- **Texture compression (P0):** Well-established process
- **LOD generation (P1):** Standard UE5 workflow
- **Audio compression (P3):** Proven compression settings

---

## Success Metrics

### Performance
- ✅ 60 FPS on target hardware
- ✅ <5 GB VRAM usage
- ✅ <8 GB RAM usage
- ✅ <30s load times

### Quality
- ✅ No visible quality degradation
- ✅ Smooth texture streaming
- ✅ No audio artifacts
- ✅ Stable frame times

### Workflow
- ✅ Faster iteration times
- ✅ Easier asset management
- ✅ Automated validation
- ✅ Clear documentation

---

**Document Owner:** Asset Manager Agent  
**Last Updated:** 2026-04-26  
**Next Review:** After each optimization phase
