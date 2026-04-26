# Material Naming Convention

**Document Type:** Standards Guide  
**Target Audience:** All Team Members  
**Last Updated:** 2026-04-26

---

## Overview

Standardized naming conventions for all material assets in HorrorProject. Consistent naming improves organization, searchability, and team collaboration.

---

## General Rules

1. **Use PascalCase** for all names after prefix
2. **Use underscores** to separate prefix from name and between words
3. **Be descriptive** but concise (3-5 words max)
4. **No spaces** in asset names
5. **Use prefixes** for all material assets
6. **Use suffixes** for variations
7. **Use numbers** with leading zeros (01, 02, not 1, 2)

---

## Material Prefixes

### Core Prefixes

| Prefix | Asset Type | Example |
|--------|------------|---------|
| M_ | Master Material | M_Master_Surface |
| MI_ | Material Instance | MI_Wall_Concrete_01 |
| MF_ | Material Function | MF_Chromatic_Aberration |
| MPC_ | Material Parameter Collection | MPC_Global_Lighting |
| PM_ | Physical Material | PM_Metal_Heavy |

---

## Naming Patterns

### Master Materials

**Pattern:** `M_Master_[Type]`

**Examples:**
```
M_Master_Surface
M_Master_VHS
M_Master_PostProcess
M_Master_UI
M_Master_Transparent
M_Master_Emissive
```

**Rules:**
- Always start with `M_Master_`
- Use descriptive type name
- Keep concise (1-2 words)

### Material Instances

**Pattern:** `MI_[Category]_[Name]_[Variation]`

**Examples:**
```
MI_Wall_Concrete_Dirty_01
MI_Floor_Metal_Grate
MI_Metal_Rusty_Industrial
MI_Plastic_Matte_Red
MI_VHS_Horror_Intense
```

**Rules:**
- Category: Asset type (Wall, Floor, Metal, etc.)
- Name: Descriptive identifier
- Variation: Optional number or descriptor

### Material Functions

**Pattern:** `MF_[Function]_[Description]`

**Examples:**
```
MF_Chromatic_Aberration
MF_VHS_Noise
MF_Scanlines
MF_Vignette
MF_Film_Grain
MF_Color_Grading
```

**Rules:**
- Use descriptive function name
- Avoid abbreviations unless standard
- Keep under 3 words

### Material Parameter Collections

**Pattern:** `MPC_[Scope]_[Purpose]`

**Examples:**
```
MPC_Global_Lighting
MPC_Global_Environment
MPC_Level_Atmosphere
MPC_Character_Skin
```

**Rules:**
- Scope: Global, Level, Character, etc.
- Purpose: What parameters control
- Use sparingly (performance impact)

---

## Category Guidelines

### Environment Materials

**Walls:**
```
MI_Wall_[Material]_[Condition]_[Number]

Examples:
MI_Wall_Concrete_Clean_01
MI_Wall_Concrete_Dirty_01
MI_Wall_Brick_Red_01
MI_Wall_Metal_Panels_01
```

**Floors:**
```
MI_Floor_[Material]_[Type]_[Number]

Examples:
MI_Floor_Concrete_Smooth
MI_Floor_Metal_Grate
MI_Floor_Wood_Worn_01
MI_Floor_Tile_Dirty_01
```

**Ceilings:**
```
MI_Ceiling_[Material]_[Type]_[Number]

Examples:
MI_Ceiling_Concrete_01
MI_Ceiling_Metal_Panels
MI_Ceiling_Tile_Acoustic
```

### Prop Materials

**Pattern:** `MI_[Material]_[Condition]_[Color/Type]`

**Examples:**
```
MI_Metal_Rusty_Brown
MI_Metal_Clean_Chrome
MI_Plastic_Matte_Black
MI_Rubber_Worn_Black
MI_Glass_Clear
MI_Glass_Frosted
```

### Character Materials

**Pattern:** `MI_Character_[Name]_[Part]_[Number]`

**Examples:**
```
MI_Character_Player_Body_01
MI_Character_Player_Face
MI_Character_Enemy_Skin_01
MI_Character_Enemy_Clothing
```

### VFX Materials

**Pattern:** `MI_VFX_[Effect]_[Type]`

**Examples:**
```
MI_VFX_Smoke_Wispy
MI_VFX_Fire_Flames
MI_VFX_Sparks_Electric
MI_VFX_Blood_Splatter
```

### Post-Process Materials

**Pattern:** `MI_PP_[Effect]_[Intensity]`

**Examples:**
```
MI_PP_VHS_Standard
MI_PP_VHS_Horror
MI_PP_Blur_Heavy
MI_PP_ColorGrade_Horror
```

### UI Materials

**Pattern:** `MI_UI_[Element]_[Type]`

**Examples:**
```
MI_UI_Button_Standard
MI_UI_Button_Hover
MI_UI_HealthBar_Fill
MI_UI_Icon_Weapon
MI_UI_Text_Standard
```

---

## Variation Suffixes

### Numeric Variations

Use two-digit numbers:
```
MI_Wall_Concrete_01
MI_Wall_Concrete_02
MI_Wall_Concrete_03
```

### Condition Variations

```
_Clean
_Dirty
_Worn
_Damaged
_Destroyed
_Rusty
_Weathered
```

### Color Variations

```
_Red
_Blue
_Green
_Black
_White
_Gray
```

### Size Variations

```
_Small
_Medium
_Large
_1m
_2m
_4m
```

### Quality Variations

```
_Low
_Medium
_High
_Ultra
```

---

## Special Cases

### Temporary Materials

**Pattern:** `TEMP_MI_[Name]`

**Example:** `TEMP_MI_Placeholder_Wall`

**Rules:**
- Prefix with TEMP_
- Delete before final build
- Document reason for temporary status

### Prototype Materials

**Pattern:** `PROTO_MI_[Name]`

**Example:** `PROTO_MI_Experimental_Shader`

**Rules:**
- Prefix with PROTO_
- Replace with final materials
- Keep in separate folder

### Deprecated Materials

**Pattern:** `DEPRECATED_MI_[Name]`

**Example:** `DEPRECATED_MI_Old_Wall`

**Rules:**
- Prefix with DEPRECATED_
- Move to Deprecated folder
- Remove after verification

### Test Materials

**Pattern:** `TEST_MI_[Name]`

**Example:** `TEST_MI_Performance_Test`

**Rules:**
- Prefix with TEST_
- Keep in Tests folder
- Don't use in production

---

## Folder Organization

### Standard Structure

```
Content/Materials/
├── Masters/
│   ├── M_Master_Surface.uasset
│   ├── M_Master_VHS.uasset
│   ├── M_Master_PostProcess.uasset
│   └── M_Master_UI.uasset
├── Functions/
│   ├── MF_Chromatic_Aberration.uasset
│   ├── MF_VHS_Noise.uasset
│   └── ...
├── Instances/
│   ├── Environment/
│   │   ├── Walls/
│   │   ├── Floors/
│   │   └── Ceilings/
│   ├── Props/
│   │   ├── Industrial/
│   │   ├── Furniture/
│   │   └── Decorative/
│   ├── Characters/
│   ├── VFX/
│   ├── PostProcess/
│   └── UI/
└── ParameterCollections/
    └── MPC_Global_Environment.uasset
```

---

## Validation Rules

### Automated Checks

**Valid Name:**
```
✅ MI_Wall_Concrete_Dirty_01
✅ M_Master_Surface
✅ MF_Chromatic_Aberration
```

**Invalid Name:**
```
❌ material_01 (no prefix, lowercase)
❌ MI_wall (lowercase after prefix)
❌ MI Wall Concrete (spaces)
❌ MI_WallMtl (unclear abbreviation)
❌ MI_Wall1 (single digit)
```

### Validation Script

```python
def validate_material_name(name):
    # Check prefix
    valid_prefixes = ['M_', 'MI_', 'MF_', 'MPC_', 'PM_']
    if not any(name.startswith(p) for p in valid_prefixes):
        return False, "Missing valid prefix"
    
    # Check for spaces
    if ' ' in name:
        return False, "Contains spaces"
    
    # Check length
    if len(name) < 5 or len(name) > 64:
        return False, "Invalid length"
    
    # Check PascalCase after prefix
    parts = name.split('_')[1:]
    for part in parts:
        if not part[0].isupper():
            return False, "Not PascalCase"
    
    return True, "Valid"
```

---

## Common Mistakes

### ❌ Wrong Prefix

```
Bad: Material_Wall_01
Good: MI_Wall_Concrete_01
```

### ❌ Lowercase

```
Bad: MI_wall_concrete
Good: MI_Wall_Concrete
```

### ❌ Spaces

```
Bad: MI_Wall Concrete 01
Good: MI_Wall_Concrete_01
```

### ❌ Unclear Abbreviations

```
Bad: MI_Wl_Cnc_01
Good: MI_Wall_Concrete_01
```

### ❌ Single Digit Numbers

```
Bad: MI_Wall_1
Good: MI_Wall_01
```

### ❌ Too Generic

```
Bad: MI_Material_01
Good: MI_Wall_Concrete_Dirty_01
```

---

## Migration Guide

### Renaming Existing Materials

**Process:**
1. Backup project
2. Use UE5 rename tool (updates references)
3. Follow naming convention
4. Test thoroughly
5. Update documentation
6. Commit changes

**UE5 Rename:**
```
1. Right-click asset in Content Browser
2. Select "Rename"
3. Enter new name
4. References auto-update
5. Save all
```

**Batch Rename:**
```
Use Asset Rename Tool:
1. Select multiple assets
2. Tools → Rename Assets
3. Apply naming pattern
4. Preview changes
5. Execute rename
```

---

## Team Guidelines

### Creating New Materials

1. **Check existing materials** first
2. **Follow naming convention** exactly
3. **Use appropriate prefix**
4. **Be descriptive** but concise
5. **Document** in MaterialLibrary.md

### Reviewing Materials

1. **Verify naming** compliance
2. **Check folder** location
3. **Ensure no duplicates**
4. **Update documentation**

### Communication

When discussing materials:
- Use full asset name
- Include folder path if ambiguous
- Reference documentation

---

## Quick Reference

### Common Patterns

```
Master Materials:
M_Master_[Type]

Material Instances:
MI_[Category]_[Name]_[Variation]

Material Functions:
MF_[Function]_[Description]

Parameter Collections:
MPC_[Scope]_[Purpose]

Physical Materials:
PM_[Material]_[Type]
```

### Prefixes

```
M_    - Master Material
MI_   - Material Instance
MF_   - Material Function
MPC_  - Material Parameter Collection
PM_   - Physical Material
```

### Suffixes

```
_01, _02, _03  - Numeric variations
_Clean, _Dirty - Condition
_Red, _Blue    - Color
_Small, _Large - Size
```

---

## Related Documentation

- [MaterialLibrary.md](MaterialLibrary.md) - Complete material catalog
- [MaterialInstanceCreation.md](MaterialInstanceCreation.md) - Creation guide
- [NamingConvention.md](../Assets/NamingConvention.md) - General asset naming

---

**Document Owner:** Materials Team  
**Last Updated:** 2026-04-26  
**Next Review:** When adding new material types
