# Asset Inventory - HorrorProject

**Generated:** 2026-04-26  
**Total Assets:** 1,945 files  
**Total Size:** ~17.3 GB  
**Project:** D:\gptzuo\HorrorProject\HorrorProject

---

## Asset Packages Overview

| Package | Size | Asset Count | Type | Status | Priority |
|---------|------|-------------|------|--------|----------|
| **DeepWaterStation** | 7.8 GB | 388 | Environment | Active | HIGH |
| **SD_Art** | 7.8 GB | 749 | Industrial Assets | Active | HIGH |
| **Grimytheus_Vol_2** | 585 MB | ~150 | Audio (Horror) | Planned | MEDIUM |
| **SoundsOfHorror** | 279 MB | 204 | Audio (Ambient) | Planned | MEDIUM |
| **Bodycam_VHS_Effect** | 300 MB | ~80 | Post-Process | Active | HIGH |
| **IndustrialPipesM** | 224 MB | ~120 | Props | Planned | MEDIUM |
| **Fab/Industrial** | 164 MB | ~90 | Props | Planned | LOW |
| **Stone_Golem** | 142 MB | ~40 | Character | Unused | LOW |
| **Characters** | 126 MB | ~60 | Base Characters | Active | HIGH |
| **LevelPrototyping** | 3.2 MB | ~30 | Prototyping | Active | HIGH |

---

## DeepWaterStation (7.8 GB, 388 Assets)

**Path:** `Content/DeepWaterStation/`  
**Status:** Active - Primary environment package  
**Optimization:** Required

### Asset Breakdown
- **Meshes:** ~180 files (SM_* prefix)
  - Modular architecture pieces
  - Props and furniture
  - Industrial equipment
- **Materials:** ~120 files (MI_* prefix)
  - Material instances
  - Master materials
- **Textures:** ~80 files (T_* prefix)
  - Albedo, Normal, ORM maps
  - 2K-4K resolution
- **Niagara:** ~8 particle systems

### Naming Convention Status
✅ **GOOD:** Follows UE5 standards (SM_, MI_, T_ prefixes)

### Optimization Needs
- [ ] Generate LODs for all meshes
- [ ] Compress textures (BC7/BC5)
- [ ] Create material instances hierarchy
- [ ] Implement virtual textures for large surfaces

---

## SD_Art Industrial (7.8 GB, 749 Assets)

**Path:** `Content/SD_Art/`  
**Status:** Active - Secondary environment assets  
**Optimization:** Critical

### Asset Breakdown
- **Industrial_Architecture:** ~400 files
  - Modular building pieces
  - Structural elements
- **_SD_Materials:** ~200 material instances
- **_SD_Textures:** ~149 texture files
  - High-resolution (4K-8K)
  - PBR workflow

### Naming Convention Status
⚠️ **MIXED:** Some assets lack proper prefixes

### Optimization Needs
- [ ] **CRITICAL:** Downscale 8K textures to 4K
- [ ] Generate LOD chains
- [ ] Consolidate duplicate materials
- [ ] Implement texture streaming

---

## Audio Assets (864 MB Total)

### Grimytheus_Vol_2 (585 MB)
**Path:** `Content/Grimytheus_Vol_2/`  
**Status:** Planned - Not yet integrated  
**Files:** ~150 audio files

**Content:**
- Horror ambience
- Creature sounds
- Environmental audio
- Music stingers

**Migration Status:** ❌ Not migrated  
**Optimization:** Requires compression

### SoundsOfHorror (279 MB)
**Path:** `Content/SoundsOfHorror/`  
**Status:** Planned - Not yet integrated  
**Files:** 204 audio files

**Content:**
- Atmospheric sounds
- Jump scare audio
- Background ambience

**Migration Status:** ❌ Not migrated  
**Optimization:** Requires compression

### Audio Optimization Plan
- [ ] Convert to OGG Vorbis (quality 0.7)
- [ ] Implement audio streaming for long files
- [ ] Create sound cues and attenuation
- [ ] Set up audio occlusion

---

## Bodycam_VHS_Effect (300 MB)

**Path:** `Content/Bodycam_VHS_Effect/`  
**Status:** Active - Core visual style  
**Priority:** HIGH

### Components
- Post-process materials
- Camera shake blueprints
- UI widgets
- Demo content

### Naming Convention Status
✅ **GOOD:** Proper BP_, WBP_ prefixes

### Optimization Status
✅ **OPTIMIZED:** Lightweight post-process effects

---

## IndustrialPipesM (224 MB)

**Path:** `Content/IndustrialPipesM/`  
**Status:** Planned - Not yet used  
**Assets:** ~120 modular pipe pieces

### Content
- Modular pipe meshes
- Connection pieces
- Valve and junction assets
- PBR materials

### Migration Plan
- [ ] Review for overlap with DeepWaterStation
- [ ] Migrate unique assets only
- [ ] Generate LODs
- [ ] Optimize materials

---

## Character Assets (268 MB)

### Characters/Mannequins (126 MB)
**Path:** `Content/Characters/Mannequins/`  
**Status:** Active - Base character system  
**Priority:** HIGH

**Content:**
- UE5 Mannequin meshes
- Animation blueprints
- Base materials

### Stone_Golem (142 MB)
**Path:** `Content/Stone_Golem/`  
**Status:** Unused - Potential enemy asset  
**Priority:** LOW

**Decision Required:**
- Keep for future enemy implementation?
- Remove to save space?

---

## Custom Project Assets

### Blueprints (68 KB)
**Path:** `Content/Blueprints/`  
**Status:** Active - Core gameplay

**Files:**
- BP_HorrorPlayerCharacter
- BP_HorrorPlayerController

### Input (384 KB)
**Path:** `Content/Input/`  
**Status:** Active - Enhanced Input System

### LevelPrototyping (3.2 MB)
**Path:** `Content/LevelPrototyping/`  
**Status:** Active - Greybox assets

---

## Asset Usage Analysis

### Currently Used (Active)
- DeepWaterStation: 388 assets
- SD_Art: 749 assets
- Bodycam_VHS_Effect: ~80 assets
- Characters: ~60 assets
- LevelPrototyping: ~30 assets
- Custom Blueprints: 2 assets

**Total Active:** ~1,309 assets (67%)

### Planned for Use
- Grimytheus_Vol_2: ~150 audio files
- SoundsOfHorror: 204 audio files
- IndustrialPipesM: ~120 assets
- Fab/Industrial: ~90 assets

**Total Planned:** ~564 assets (29%)

### Unused/Undecided
- Stone_Golem: ~40 assets
- Fab demo content: ~32 assets

**Total Unused:** ~72 assets (4%)

---

## Naming Convention Compliance

### ✅ Compliant Packages
- DeepWaterStation (SM_, MI_, T_ prefixes)
- Bodycam_VHS_Effect (BP_, WBP_ prefixes)
- Characters (SK_, ABP_ prefixes)

### ⚠️ Needs Review
- SD_Art (inconsistent prefixes)
- IndustrialPipesM (mixed naming)
- Fab assets (vendor naming)

### ❌ Non-Compliant
- Stone_Golem (no prefixes)
- Some custom blueprints

---

## Performance Budget Status

### Current Memory Usage (Estimated)
- **Textures:** ~12 GB (loaded on demand)
- **Meshes:** ~3 GB
- **Audio:** ~864 MB (streaming)
- **Materials:** ~500 MB
- **Blueprints:** ~50 MB

### Target Budget (for 8GB VRAM)
- **Textures:** 4 GB max loaded
- **Meshes:** 1.5 GB max
- **Audio:** 256 MB max in memory
- **Materials:** 256 MB max
- **Other:** 512 MB

**Status:** ⚠️ Over budget - optimization required

---

## Priority Actions

### Immediate (Week 1)
1. Generate LODs for DeepWaterStation meshes
2. Compress SD_Art textures (8K → 4K)
3. Audit and remove unused demo content
4. Standardize naming conventions

### Short-term (Week 2-3)
1. Migrate and compress audio assets
2. Create material instance hierarchy
3. Implement texture streaming
4. Set up asset validation tools

### Long-term (Month 1-2)
1. Implement virtual textures
2. Create asset usage tracking
3. Build automated optimization pipeline
4. Establish performance monitoring

---

## Next Steps

1. Review **AssetUsage.md** for detailed usage tracking
2. Check **AssetPriority.md** for optimization order
3. Follow **AssetOptimization.md** for technical guidelines
4. Execute **Migration/** plans for audio and props

---

**Document Owner:** Asset Manager Agent  
**Last Updated:** 2026-04-26  
**Next Review:** Weekly during active development
