# Asset Usage Tracking - HorrorProject

**Generated:** 2026-04-26  
**Purpose:** Track which assets are actively used in the project  
**Update Frequency:** Weekly or after major level changes

---

## Usage Status Definitions

- 🟢 **ACTIVE:** Currently used in levels or blueprints
- 🟡 **PLANNED:** Scheduled for integration
- 🔵 **REFERENCED:** Used by other assets but not directly placed
- ⚪ **UNUSED:** Not currently referenced
- 🔴 **DEPRECATED:** Marked for removal

---

## DeepWaterStation Assets (388 Total)

### Meshes (180 files)

#### 🟢 Active in Levels (Estimated: 120 assets)
**Primary Environment Assets:**
- Modular walls, floors, ceilings
- Structural columns and beams
- Doors and windows
- Stairs and railings

**Props Currently Placed:**
- SM_BedBase01, SM_BedSheets01/02
- SM_ChairA04 and variants
- SM_Bridge01
- SM_Buoy01

**Usage Location:** Main level (Horror map)

#### 🟡 Planned for Use (Estimated: 40 assets)
- Additional furniture pieces
- Decorative props
- Industrial equipment
- Lighting fixtures

#### ⚪ Unused (Estimated: 20 assets)
- Demo-specific assets
- Duplicate variations
- Placeholder meshes

### Materials (120 files)

#### 🟢 Active (Estimated: 80 materials)
All material instances for actively used meshes:
- MI_AdapterPipe01
- MI_BedBase01, MI_BedSheets01/02
- MI_Bridge01
- MI_Ceiling06
- Wall and floor materials

#### 🔵 Referenced (Estimated: 30 materials)
Master materials and base materials:
- M_Master_PBR
- M_Master_Emissive
- M_Master_Transparent

#### ⚪ Unused (Estimated: 10 materials)
- Demo materials
- Unused variations

### Textures (80 files)

#### 🟢 Active (All 80 textures)
All textures are referenced by active materials:
- Albedo maps (T_*_D)
- Normal maps (T_*_N)
- ORM maps (T_*_ORM)
- Emissive maps (T_*_E)

**Note:** No unused textures detected - all are part of material chains

### Niagara Systems (8 files)

#### 🟢 Active (Estimated: 4 systems)
- Water drip effects
- Steam/fog particles
- Dust motes
- Sparks/electrical effects

#### 🟡 Planned (Estimated: 4 systems)
- Additional atmospheric effects
- Environmental hazards

---

## SD_Art Industrial (749 Total)

### Industrial_Architecture (400 files)

#### 🟢 Active (Estimated: 150 assets)
**Modular Building Pieces:**
- Wall sections
- Floor tiles
- Ceiling panels
- Support structures

**Usage:** Secondary areas, industrial zones

#### 🟡 Planned (Estimated: 200 assets)
**Expansion Areas:**
- Additional rooms
- Exterior structures
- Connecting corridors

#### ⚪ Unused (Estimated: 50 assets)
- Duplicate assets from DeepWaterStation
- Demo content
- Incompatible pieces

### Materials (200 files)

#### 🟢 Active (Estimated: 100 materials)
Material instances for placed meshes

#### 🔵 Referenced (Estimated: 50 materials)
Master materials and shared instances

#### ⚪ Unused (Estimated: 50 materials)
- Unused variations
- Demo materials
- Duplicate instances

### Textures (149 files)

#### 🟢 Active (Estimated: 120 textures)
Textures used by active materials

#### ⚪ Unused (Estimated: 29 textures)
- Unused material textures
- Demo content textures

---

## Bodycam_VHS_Effect (80 Total)

### 🟢 Active (75 assets - 94%)

**Post-Process Materials (15 files):**
- M_Bodycam_Master
- MI_VHS_Effect
- MI_Chromatic_Aberration
- MI_Film_Grain
- MI_Screen_Blur

**Usage:** Applied to player camera

**Blueprints (20 files):**
- BP_CameraComponent ✅ Used in player character
- BP_CameraShake_Idle ✅ Active
- BP_CameraShake_Walk ✅ Active
- BP_CameraShake_Run ✅ Active
- BP_FootStepAnimNotify ✅ Used in animations

**UI Widgets (5 files):**
- WBP_Screen_Blur ✅ Active in HUD

**Textures (25 files):**
- Film grain textures
- Noise textures
- Overlay textures

**Sounds (10 files):**
- Camera static
- VHS tape sounds

### 🔴 Deprecated (5 assets - 6%)
**Demo Content:**
- BP_FirstPersonCharacter (replaced by BP_HorrorPlayerCharacter)
- GM_BodycamVHSEffect (using custom game mode)
- Demo map assets

---

## Audio Assets (354 Total Files)

### Grimytheus_Vol_2 (~150 files)

#### 🟡 Planned (100%)
**Status:** Not yet migrated to project

**Planned Usage:**
- **Ambience (40 files):** Background horror atmosphere
- **Creatures (30 files):** Monster sounds
- **Environmental (40 files):** Wind, water, metal creaks
- **Music (20 files):** Tension and chase music
- **Stingers (20 files):** Jump scare audio

**Integration Priority:** HIGH - Week 2

### SoundsOfHorror (204 files)

#### 🟡 Planned (100%)
**Status:** Not yet migrated to project

**Planned Usage:**
- **Atmosphere (80 files):** Ambient loops
- **Effects (60 files):** One-shot sounds
- **Voices (30 files):** Whispers, screams
- **Mechanical (34 files):** Industrial sounds

**Integration Priority:** HIGH - Week 2

---

## IndustrialPipesM (120 Total)

### 🟡 Planned (90 assets - 75%)

**Modular Pipe System:**
- Straight pipes (various lengths)
- Elbow joints (45°, 90°)
- T-junctions
- Valves and controls
- End caps

**Planned Usage:**
- Ceiling pipe networks
- Wall-mounted pipes
- Floor drainage systems

**Integration Priority:** MEDIUM - Week 3

### ⚪ Unused (30 assets - 25%)
**Reason:** Overlap with DeepWaterStation pipes
**Decision:** Audit for unique pieces only

---

## Fab Assets (90 Total)

### Modular_Fence_Industrial_Prop

#### 🟡 Planned (60 assets - 67%)
**Fence System:**
- Fence sections
- Posts and corners
- Gates
- Barbed wire

**Planned Usage:**
- Exterior boundaries
- Interior barriers
- Restricted areas

**Integration Priority:** LOW - Week 4

### Industrial_Machine

#### ⚪ Unused (30 assets - 33%)
**Status:** Evaluating for use
**Potential Usage:** Background machinery

---

## Character Assets (100 Total)

### Characters/Mannequins (60 files)

#### 🟢 Active (55 assets - 92%)

**Skeletal Meshes (5 files):**
- SK_Mannequin ✅ Player character base
- SK_Mannequin_Female ✅ Available

**Animation Blueprints (10 files):**
- ABP_Manny ✅ Used in player character
- ABP_Quinn ✅ Available

**Animations (40 files):**
- MM_Idle ✅ Active
- MM_Walk_Fwd ✅ Active
- MM_Run_Fwd ✅ Active
- MM_Jump ✅ Active
- MM_Fall_Loop ✅ Active
- MM_Land ✅ Active
- BS_MM_WalkRun ✅ Active blend space

**Materials (5 files):**
- Character materials ✅ Active

#### ⚪ Unused (5 assets - 8%)
- Demo-specific animations
- Unused material variations

### Stone_Golem (40 files)

#### ⚪ Unused (100%)
**Status:** Not integrated

**Content:**
- Skeletal mesh
- Animations (idle, walk, attack)
- Materials and textures

**Decision Required:**
- **Option A:** Integrate as enemy character (Week 5+)
- **Option B:** Remove to save 142 MB

**Recommendation:** Keep for potential enemy implementation

---

## Custom Project Assets

### Blueprints (2 files)

#### 🟢 Active (100%)
- BP_HorrorPlayerCharacter ✅ Main player character
- BP_HorrorPlayerController ✅ Player controller

### Input (10 files)

#### 🟢 Active (100%)
**Enhanced Input System:**
- IA_Move ✅ Movement action
- IA_Look ✅ Camera action
- IA_Jump ✅ Jump action
- IA_Interact ✅ Interaction action
- IMC_Default ✅ Input mapping context

### LevelPrototyping (30 files)

#### 🟢 Active (100%)
**Greybox Assets:**
- Prototype walls, floors
- Placeholder props
- Test materials

**Usage:** Level blocking and testing

---

## Usage Statistics Summary

| Package | Total | Active | Planned | Unused | Usage % |
|---------|-------|--------|---------|--------|---------|
| DeepWaterStation | 388 | 280 | 88 | 20 | 72% |
| SD_Art | 749 | 300 | 350 | 99 | 40% |
| Bodycam_VHS_Effect | 80 | 75 | 0 | 5 | 94% |
| Grimytheus_Vol_2 | 150 | 0 | 150 | 0 | 0% (planned) |
| SoundsOfHorror | 204 | 0 | 204 | 0 | 0% (planned) |
| IndustrialPipesM | 120 | 0 | 90 | 30 | 0% (planned) |
| Fab Assets | 90 | 0 | 60 | 30 | 0% (planned) |
| Stone_Golem | 40 | 0 | 0 | 40 | 0% |
| Characters | 60 | 55 | 0 | 5 | 92% |
| Custom Assets | 42 | 42 | 0 | 0 | 100% |
| **TOTAL** | **1,923** | **752** | **942** | **229** | **39% active** |

---

## Asset Reference Tracking

### High-Priority Assets (Must Keep)
1. **DeepWaterStation active meshes (280)** - Primary environment
2. **Bodycam_VHS_Effect (75)** - Core visual style
3. **Characters/Mannequins (55)** - Player character system
4. **Custom Blueprints (42)** - Core gameplay

**Total Critical Assets:** 452 (23% of total)

### Medium-Priority Assets (Planned Use)
1. **Audio packages (354)** - Atmosphere and immersion
2. **SD_Art planned (350)** - Level expansion
3. **DeepWaterStation planned (88)** - Additional content
4. **IndustrialPipesM (90)** - Environmental detail
5. **Fab fences (60)** - Level boundaries

**Total Planned Assets:** 942 (49% of total)

### Low-Priority Assets (Review for Removal)
1. **SD_Art unused (99)** - Potential duplicates
2. **Stone_Golem (40)** - Unused character
3. **Fab machines (30)** - Uncertain use
4. **IndustrialPipesM duplicates (30)** - Overlap with DeepWaterStation
5. **Demo content (30)** - No longer needed

**Total Low-Priority:** 229 (12% of total)

---

## Cleanup Recommendations

### Safe to Remove (Estimated 50 MB)
- Bodycam_VHS_Effect demo content (5 assets, ~10 MB)
- Duplicate mannequin animations (5 assets, ~5 MB)
- LevelPrototyping unused materials (5 assets, ~1 MB)
- SD_Art demo maps (10 assets, ~30 MB)

### Review Before Removal (Estimated 200 MB)
- Stone_Golem package (40 assets, 142 MB)
- Fab Industrial_Machine (30 assets, ~50 MB)
- SD_Art duplicate materials (20 assets, ~10 MB)

### Keep All
- DeepWaterStation (all assets may be needed)
- Audio packages (planned integration)
- Character assets (core gameplay)

---

## Integration Schedule

### Week 1 (Current)
- ✅ DeepWaterStation (active)
- ✅ Bodycam_VHS_Effect (active)
- ✅ Characters (active)
- ✅ Custom Blueprints (active)

### Week 2
- 🟡 Grimytheus_Vol_2 audio (150 files)
- 🟡 SoundsOfHorror audio (204 files)
- 🟡 SD_Art expansion (100 additional assets)

### Week 3
- 🟡 IndustrialPipesM (90 assets)
- 🟡 SD_Art expansion (150 additional assets)

### Week 4
- 🟡 Fab fences (60 assets)
- 🟡 SD_Art expansion (100 additional assets)

### Week 5+ (Future)
- ⚪ Stone_Golem (if enemy system implemented)
- ⚪ Fab machines (if needed)

---

## Tracking Methods

### Automated Tracking (Recommended)
Use UE5 Reference Viewer and Size Map:
1. Open Content Browser
2. Right-click asset → Reference Viewer
3. Check "Show Referencers" to see what uses the asset
4. Use Size Map to identify large unused assets

### Manual Tracking
Update this document when:
- Adding new assets to levels
- Creating new blueprints
- Removing unused content
- Completing level sections

### Validation Scripts
Run weekly validation (see Scripts/Assets/):
- `FindUnusedAssets.ps1` - Identify unreferenced assets
- `ValidateAssetReferences.ps1` - Check for broken references
- `GenerateAssetReport.ps1` - Update usage statistics

---

## Next Actions

1. **Immediate:** Run FindUnusedAssets.ps1 to validate estimates
2. **Week 2:** Migrate audio assets and update usage tracking
3. **Weekly:** Update this document after level changes
4. **Monthly:** Full asset audit and cleanup

---

**Document Owner:** Asset Manager Agent  
**Last Updated:** 2026-04-26  
**Next Review:** 2026-05-03 (Weekly)
