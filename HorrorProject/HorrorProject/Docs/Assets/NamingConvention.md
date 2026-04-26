# Asset Naming Convention - HorrorProject

**Generated:** 2026-04-26  
**Purpose:** Standardize asset naming across the project  
**Standard:** Unreal Engine 5 Best Practices

---

## General Rules

1. **Use PascalCase** for all asset names
2. **Use underscores** to separate prefix from name
3. **Be descriptive** but concise
4. **No spaces** in asset names
5. **Use prefixes** for all assets
6. **Use suffixes** for variations
7. **Avoid abbreviations** unless standard (LOD, UV, etc.)
8. **Use numbers** for variations (01, 02, not 1, 2)

---

## Asset Type Prefixes

### Meshes

| Type | Prefix | Example |
|------|--------|---------|
| Static Mesh | SM_ | SM_Door_Metal_01 |
| Skeletal Mesh | SK_ | SK_Character_Player |
| Skeletal Mesh (Physics) | SKP_ | SKP_Ragdoll_Enemy |

### Materials

| Type | Prefix | Example |
|------|--------|---------|
| Material | M_ | M_Master_PBR |
| Material Instance | MI_ | MI_Metal_Rusty |
| Material Function | MF_ | MF_Parallax |
| Material Parameter Collection | MPC_ | MPC_Global_Lighting |
| Physical Material | PM_ | PM_Metal |

### Textures

| Type | Prefix | Example |
|------|--------|---------|
| Texture (Generic) | T_ | T_Wall_Concrete |
| Texture (Diffuse/Albedo) | T_*_D | T_Wall_Concrete_D |
| Texture (Normal) | T_*_N | T_Wall_Concrete_N |
| Texture (Roughness) | T_*_R | T_Wall_Concrete_R |
| Texture (Metallic) | T_*_M | T_Wall_Concrete_M |
| Texture (Ambient Occlusion) | T_*_AO | T_Wall_Concrete_AO |
| Texture (Emissive) | T_*_E | T_Wall_Concrete_E |
| Texture (ORM Packed) | T_*_ORM | T_Wall_Concrete_ORM |
| Texture (Mask) | T_*_Mask | T_Wall_Concrete_Mask |
| Texture (Opacity) | T_*_O | T_Glass_O |
| Render Target | RT_ | RT_SecurityCamera |
| Texture Cube | TC_ | TC_Skybox_Night |

### Blueprints

| Type | Prefix | Example |
|------|--------|---------|
| Blueprint | BP_ | BP_Door_Interactive |
| Blueprint Function Library | BPFL_ | BPFL_MathHelpers |
| Blueprint Interface | BPI_ | BPI_Interactable |
| Blueprint Macro Library | BPML_ | BPML_CommonMacros |
| Enumeration | E_ | E_DoorState |
| Structure | F_ | F_ItemData |
| Widget Blueprint | WBP_ | WBP_MainMenu |

### Animation

| Type | Prefix | Example |
|------|--------|---------|
| Animation Sequence | A_ | A_Character_Walk |
| Animation Blueprint | ABP_ | ABP_Character |
| Animation Montage | AM_ | AM_Character_Attack |
| Blend Space | BS_ | BS_Character_Movement |
| Blend Space 1D | BS1D_ | BS1D_Character_Speed |
| Aim Offset | AO_ | AO_Character_Aim |
| Aim Offset 1D | AO1D_ | AO1D_Character_Pitch |
| Animation Composite | AC_ | AC_Character_Combo |
| Skeletal Mesh Socket | S_ | S_Hand_Right |

### Audio

| Type | Prefix | Example |
|------|--------|---------|
| Sound Wave | SW_ | SW_Footstep_Concrete |
| Sound Cue | SC_ | SC_Ambient_Horror |
| Sound Attenuation | SA_ | SA_Medium_Range |
| Sound Concurrency | SCon_ | SCon_Footsteps |
| Sound Class | SCl_ | SCl_SFX |
| Sound Mix | SM_ | SM_Combat |
| Dialogue Voice | DV_ | DV_Character_Player |
| Dialogue Wave | DW_ | DW_Character_Greeting |

### Particles & VFX

| Type | Prefix | Example |
|------|--------|---------|
| Particle System (Cascade) | PS_ | PS_Sparks_Electric |
| Niagara System | NS_ | NS_Fog_Volumetric |
| Niagara Emitter | NE_ | NE_Smoke_Wispy |
| Niagara Function | NF_ | NF_Curl_Noise |
| Niagara Module | NM_ | NM_Velocity_Cone |
| Niagara Parameter Collection | NPC_ | NPC_Global_Wind |

### Physics

| Type | Prefix | Example |
|------|--------|---------|
| Physical Material | PM_ | PM_Metal_Heavy |
| Physics Asset | PHYS_ | PHYS_Character |
| Destructible Mesh | DM_ | DM_Crate_Wooden |

### Lighting

| Type | Prefix | Example |
|------|--------|---------|
| Light Function | LF_ | LF_Flickering |
| Light Profile | LP_ | LP_Spotlight_Narrow |

### Miscellaneous

| Type | Prefix | Example |
|------|--------|---------|
| Level/Map | L_ | L_MainLevel |
| Data Asset | DA_ | DA_WeaponStats |
| Data Table | DT_ | DT_ItemDatabase |
| Curve Table | CT_ | CT_DamageFalloff |
| Curve Float | CF_ | CF_FadeIn |
| Curve Vector | CV_ | CV_CameraShake |
| Curve Linear Color | CLC_ | CLC_LightColor |
| Media Player | MP_ | MP_Cutscene |
| Media Texture | MT_ | MT_VideoScreen |
| Media Sound | MS_ | MS_VideoAudio |
| Font | Font_ | Font_UI_Regular |
| Font Face | FF_ | FF_UI_Bold |

---

## Folder Structure

### Standard Organization

```
Content/
├── Audio/
│   ├── Ambient/
│   ├── Music/
│   ├── SFX/
│   └── Voice/
├── Blueprints/
│   ├── Characters/
│   ├── Gameplay/
│   ├── UI/
│   └── Utilities/
├── Characters/
│   ├── Player/
│   └── Enemies/
├── Environment/
│   ├── DeepWaterStation/
│   ├── Industrial/
│   └── Props/
├── Materials/
│   ├── Master/
│   ├── Instances/
│   └── Functions/
├── Meshes/
│   ├── Architecture/
│   ├── Props/
│   └── Characters/
├── Textures/
│   ├── Environment/
│   ├── Characters/
│   └── UI/
├── VFX/
│   ├── Niagara/
│   └── Particles/
└── UI/
    ├── Widgets/
    ├── Textures/
    └── Fonts/
```

### Asset-Specific Folders

**Per-Asset Organization:**
```
Content/Environment/Props/Door_Metal/
├── SM_Door_Metal_01.uasset
├── SM_Door_Metal_Frame.uasset
├── M_Door_Metal.uasset
├── MI_Door_Metal_Rusty.uasset
├── T_Door_Metal_D.uasset
├── T_Door_Metal_N.uasset
└── T_Door_Metal_ORM.uasset
```

---

## Naming Patterns

### Modular Assets

**Pattern:** `SM_[Category]_[Type]_[Variation]_[Size]`

Examples:
- `SM_Wall_Concrete_Straight_4m`
- `SM_Wall_Concrete_Corner_90deg`
- `SM_Floor_Metal_Tile_2x2m`
- `SM_Ceiling_Industrial_Panel_4x4m`

### Props

**Pattern:** `SM_[Category]_[Name]_[Variation]`

Examples:
- `SM_Furniture_Chair_Office_01`
- `SM_Furniture_Table_Metal_Large`
- `SM_Prop_Barrel_Rusty_01`
- `SM_Prop_Crate_Wooden_Small`

### Materials

**Pattern:** `M_[Type]_[Name]` or `MI_[Name]_[Variation]`

Examples:
- `M_Master_PBR`
- `M_Master_Emissive`
- `MI_Metal_Rusty`
- `MI_Concrete_Rough_Dirty`
- `MI_Plastic_Matte_Red`

### Textures

**Pattern:** `T_[Category]_[Name]_[Suffix]`

Examples:
- `T_Wall_Concrete_D` (Diffuse/Albedo)
- `T_Wall_Concrete_N` (Normal)
- `T_Wall_Concrete_ORM` (Occlusion/Roughness/Metallic)
- `T_Metal_Rusty_E` (Emissive)

### Audio

**Pattern:** `SW_[Category]_[Name]_[Variation]` or `SC_[Category]_[Name]`

Examples:
- `SW_Footstep_Concrete_01`
- `SW_Door_Metal_Open`
- `SC_Ambient_Horror_Loop`
- `SC_Music_Tension_01`

### Blueprints

**Pattern:** `BP_[Category]_[Name]`

Examples:
- `BP_Character_Player`
- `BP_Enemy_Monster_01`
- `BP_Interactable_Door`
- `BP_Pickup_Key_Red`

### UI Widgets

**Pattern:** `WBP_[Screen/Component]_[Name]`

Examples:
- `WBP_MainMenu`
- `WBP_HUD_PlayerHealth`
- `WBP_Inventory_Grid`
- `WBP_Button_Standard`

---

## Variation Suffixes

### Numeric Variations

Use two-digit numbers for variations:
- `SM_Chair_01`, `SM_Chair_02`, `SM_Chair_03`
- `T_Wall_D_01`, `T_Wall_D_02`

### Size Variations

- `_Small`, `_Medium`, `_Large`
- `_1m`, `_2m`, `_4m` (for modular pieces)
- `_Low`, `_Mid`, `_High`

### State Variations

- `_Open`, `_Closed`
- `_Intact`, `_Damaged`, `_Destroyed`
- `_Clean`, `_Dirty`, `_Rusty`

### LOD Suffixes

- `SM_Asset_LOD0` (if manually created)
- `SM_Asset_LOD1`
- `SM_Asset_LOD2`

Note: Auto-generated LODs don't need suffixes

---

## Special Cases

### Temporary Assets

**Pattern:** `TEMP_[AssetName]`

Example: `TEMP_SM_Placeholder_Wall`

**Rule:** Delete before final build

### Prototype Assets

**Pattern:** `PROTO_[AssetName]`

Example: `PROTO_BP_EnemyAI`

**Rule:** Replace with final assets

### Deprecated Assets

**Pattern:** `DEPRECATED_[AssetName]`

Example: `DEPRECATED_M_OldMaterial`

**Rule:** Move to `Content/Deprecated/` folder

### Test Assets

**Pattern:** `TEST_[AssetName]`

Example: `TEST_BP_PhysicsTest`

**Rule:** Keep in `Content/Tests/` folder

---

## Package-Specific Naming

### DeepWaterStation

Current naming is compliant:
- ✅ `SM_AdapterPipe01`
- ✅ `MI_BedBase01`
- ✅ `T_Ceiling06_D`

### SD_Art

Needs standardization:
- ⚠️ Some assets lack prefixes
- ⚠️ Inconsistent variation numbering

**Action:** Rename to match standard

### Bodycam_VHS_Effect

Current naming is compliant:
- ✅ `BP_CameraComponent`
- ✅ `WBP_Screen_Blur`
- ✅ `M_Bodycam_Master`

---

## Validation Rules

### Automated Checks

1. **Prefix Check:** All assets must have valid prefix
2. **Case Check:** Must use PascalCase
3. **Space Check:** No spaces allowed
4. **Length Check:** Name length 3-64 characters
5. **Character Check:** Only alphanumeric, underscore, hyphen
6. **Suffix Check:** Variations use proper suffixes

### Manual Review

1. **Descriptive:** Name clearly describes asset
2. **Consistent:** Follows established patterns
3. **Unique:** No duplicate names
4. **Organized:** In correct folder

---

## Migration Checklist

When renaming assets:

1. **Backup project** before mass renaming
2. **Use UE5 rename tool** (updates references)
3. **Test thoroughly** after renaming
4. **Update documentation** with new names
5. **Commit changes** with clear message

**UE5 Rename Process:**
```
1. Right-click asset in Content Browser
2. Select "Rename"
3. Enter new name (references auto-update)
4. Verify no broken references
5. Save all
```

---

## Common Mistakes to Avoid

### ❌ Bad Examples

```
door.uasset                    // No prefix, lowercase
SM_door_01.uasset             // Lowercase after prefix
SM Door Metal.uasset          // Spaces
SM_DoorMtl.uasset            // Unclear abbreviation
SM_Door1.uasset              // Single digit variation
T_Wall_Diffuse.uasset        // Use _D not _Diffuse
BP_Player Character.uasset    // Space in name
M_Material_Instance.uasset    // Wrong prefix (should be MI_)
```

### ✅ Good Examples

```
SM_Door_Metal_01.uasset
SM_Door_Metal_Frame.uasset
T_Wall_Concrete_D.uasset
T_Wall_Concrete_N.uasset
BP_Character_Player.uasset
MI_Metal_Rusty.uasset
WBP_MainMenu.uasset
SC_Ambient_Horror_Loop.uasset
```

---

## Quick Reference

### Most Common Prefixes

```
SM_  - Static Mesh
SK_  - Skeletal Mesh
M_   - Material
MI_  - Material Instance
T_   - Texture
BP_  - Blueprint
WBP_ - Widget Blueprint
A_   - Animation
ABP_ - Animation Blueprint
SC_  - Sound Cue
SW_  - Sound Wave
NS_  - Niagara System
PS_  - Particle System
```

### Texture Suffixes

```
_D   - Diffuse/Albedo
_N   - Normal
_R   - Roughness
_M   - Metallic
_AO  - Ambient Occlusion
_E   - Emissive
_O   - Opacity
_ORM - Occlusion/Roughness/Metallic (packed)
```

---

## Enforcement

### Pre-Commit Validation

Run before committing:
```powershell
.\Scripts\Assets\ValidateAssetNaming.ps1
```

### CI/CD Integration

Add to build pipeline:
```yaml
- name: Validate Asset Naming
  run: .\Scripts\Assets\ValidateAssetNaming.ps1
  failOnError: true
```

### Code Review

Reviewers should check:
- [ ] All new assets follow naming convention
- [ ] Assets in correct folders
- [ ] No naming conflicts
- [ ] Variations numbered correctly

---

## Resources

### Official Documentation
- [UE5 Asset Naming Conventions](https://docs.unrealengine.com/5.0/en-US/recommended-asset-naming-conventions-in-unreal-engine-projects/)
- [UE5 Content Organization](https://docs.unrealengine.com/5.0/en-US/content-organization-in-unreal-engine/)

### Community Standards
- [Allar's UE5 Style Guide](https://github.com/Allar/ue5-style-guide)
- [Gamemakin UE5 Style Guide](https://github.com/Allar/ue5-style-guide)

---

**Document Owner:** Asset Manager Agent  
**Last Updated:** 2026-04-26  
**Next Review:** Monthly or when adding new asset types
