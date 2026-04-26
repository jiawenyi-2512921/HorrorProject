# Asset Specification

## Overview

This document defines the technical specifications for all game assets in HorrorProject. Following these specifications ensures optimal performance, consistency, and quality.

## 3D Models

### Static Meshes

**General Requirements:**
- File format: FBX 2020 or later
- Units: Centimeters
- Up axis: Z-up
- Forward axis: Y-forward
- Scale: 1.0 (real-world scale)

**Polygon Budget:**
- Props (small): 500-2,000 triangles
- Props (medium): 2,000-5,000 triangles
- Props (large): 5,000-10,000 triangles
- Environment pieces: 1,000-8,000 triangles

**LOD Requirements:**
- LOD0: 100% detail
- LOD1: 50% triangles (1000cm distance)
- LOD2: 25% triangles (2500cm distance)
- LOD3: 10% triangles (5000cm distance)

**Naming Convention:**
```
SM_[Category]_[Name]_[Variant]
Examples:
- SM_Prop_Chair_01
- SM_Environment_Wall_Brick
- SM_Furniture_Table_Wood
```

**Collision:**
- Simple collision preferred (UCX_ prefix)
- Complex collision only when necessary
- Max 16 collision primitives per mesh

### Skeletal Meshes

**General Requirements:**
- File format: FBX 2020 or later
- Skeleton: Shared skeleton when possible
- Bone hierarchy: Clean, no unnecessary bones

**Polygon Budget:**
- Characters: 10,000-15,000 triangles
- Creatures: 8,000-12,000 triangles
- Accessories: 500-2,000 triangles

**Rigging:**
- Max bones: 75
- Max influences per vertex: 4
- Root bone: Required
- IK bones: Separate hierarchy

**LOD Requirements:**
- LOD0: 100% detail
- LOD1: 60% triangles
- LOD2: 30% triangles
- LOD3: 15% triangles

**Naming Convention:**
```
SK_[Type]_[Name]
Examples:
- SK_Character_Player
- SK_Enemy_Golem
- SK_Prop_Door
```

## Textures

### Texture Resolutions

**Environment:**
- Hero assets: 2048x2048
- Standard assets: 1024x1024
- Background assets: 512x512

**Characters:**
- Main characters: 2048x2048
- Secondary characters: 1024x1024
- Accessories: 512x512

**Props:**
- Large props: 1024x1024
- Medium props: 512x512
- Small props: 256x256

**UI:**
- Icons: 256x256
- Backgrounds: 1024x1024
- HUD elements: 512x512

### Texture Types

**Albedo/Diffuse:**
- Format: BC7 (RGBA) or BC1 (RGB)
- sRGB: Yes
- Mipmaps: Yes
- Compression: High quality

**Normal Map:**
- Format: BC5 (RG)
- sRGB: No
- Mipmaps: Yes
- Compression: High quality

**Roughness/Metallic/AO (RMA):**
- Format: BC7 (RGB)
- R: Roughness
- G: Metallic
- B: Ambient Occlusion
- sRGB: No
- Mipmaps: Yes

**Emissive:**
- Format: BC7 (RGB)
- sRGB: Yes
- Mipmaps: Yes
- HDR values in material

**Opacity:**
- Format: BC4 (single channel)
- sRGB: No
- Mipmaps: Yes

**Naming Convention:**
```
T_[Category]_[Name]_[Type]
Examples:
- T_Prop_Chair_D (Diffuse)
- T_Prop_Chair_N (Normal)
- T_Prop_Chair_RMA (Roughness/Metallic/AO)
- T_Prop_Chair_E (Emissive)
```

### Texture Guidelines

**Do's:**
- Use power-of-two dimensions
- Use texture atlases when possible
- Use virtual textures for large terrains
- Compress all textures
- Use mipmaps

**Don'ts:**
- Don't use uncompressed textures
- Don't exceed resolution budgets
- Don't use alpha in diffuse unless needed
- Don't use separate AO textures (pack in RMA)

## Materials

### Material Complexity

**Shader Instructions:**
- Simple materials: < 100 instructions
- Standard materials: 100-200 instructions
- Complex materials: 200-300 instructions
- Hero materials: < 400 instructions

**Texture Samples:**
- Simple: 2-4 samples
- Standard: 4-6 samples
- Complex: 6-8 samples
- Maximum: 10 samples

### Material Types

**Master Materials:**
```
M_Master_Standard
M_Master_Foliage
M_Master_Character
M_Master_Transparent
M_Master_UI
```

**Material Instances:**
```
MI_[Category]_[Name]
Examples:
- MI_Prop_Wood_Oak
- MI_Environment_Concrete_Rough
- MI_Character_Skin_Pale
```

### Material Parameters

**Required Parameters:**
- Base Color
- Roughness
- Metallic
- Normal Strength

**Optional Parameters:**
- Emissive Color
- Emissive Strength
- Opacity
- Subsurface Color

**Performance Parameters:**
- Use Material Parameter Collections for global values
- Use Material Functions for reusable logic
- Minimize dynamic parameters

## Animations

### Animation Specifications

**Frame Rate:**
- Gameplay animations: 30 FPS
- Cinematic animations: 60 FPS

**Compression:**
- Compression scheme: Automatic
- Max error: 0.1cm (translation)
- Max error: 0.01° (rotation)

**Naming Convention:**
```
A_[Character]_[Action]_[Variant]
Examples:
- A_Player_Idle_01
- A_Player_Walk_Forward
- A_Golem_Attack_Swipe
- A_Golem_Death_01
```

### Animation Types

**Locomotion:**
- Idle: Looping, 2-4 seconds
- Walk: Looping, 1-2 seconds
- Run: Looping, 1 second
- Turn: Non-looping, 0.5 seconds

**Actions:**
- Interact: Non-looping, 1-2 seconds
- Pickup: Non-looping, 1 second
- Attack: Non-looping, 0.5-1.5 seconds

**Reactions:**
- Hit: Non-looping, 0.3-0.5 seconds
- Death: Non-looping, 2-3 seconds
- Stagger: Non-looping, 0.5-1 second

### Animation Blueprints

**Naming Convention:**
```
ABP_[Character]
Examples:
- ABP_Player
- ABP_Golem
```

**State Machines:**
- Locomotion
- Combat
- Interactions
- Reactions

## Audio

### Audio File Specifications

**Music:**
- Format: WAV (source), Vorbis (engine)
- Sample rate: 44.1kHz
- Bit depth: 16-bit
- Channels: Stereo
- Streaming: Yes

**Sound Effects:**
- Format: WAV (source), Vorbis (engine)
- Sample rate: 44.1kHz
- Bit depth: 16-bit
- Channels: Mono (preferred) or Stereo
- Streaming: No

**Dialogue:**
- Format: WAV (source), Vorbis (engine)
- Sample rate: 44.1kHz
- Bit depth: 16-bit
- Channels: Mono
- Streaming: Yes

**Ambient:**
- Format: WAV (source), Vorbis (engine)
- Sample rate: 44.1kHz
- Bit depth: 16-bit
- Channels: Stereo
- Streaming: Yes
- Looping: Yes

### Audio Naming Convention

```
[Type]_[Category]_[Name]_[Variant]
Examples:
- Music_Ambient_Hallway_01
- SFX_Footstep_Concrete_01
- VO_Player_Gasp_01
- AMB_Wind_Exterior_Loop
```

### Audio Specifications

**Volume Normalization:**
- Music: -16 LUFS
- Sound effects: -12 LUFS
- Dialogue: -18 LUFS
- Ambient: -20 LUFS

**File Size:**
- Music: < 10MB per minute
- Sound effects: < 500KB per file
- Dialogue: < 1MB per minute
- Ambient: < 5MB per minute

## Blueprints

### Blueprint Naming Convention

```
BP_[Type]_[Name]
Examples:
- BP_Character_Player
- BP_Enemy_Golem
- BP_Prop_Door_Interactive
- BP_Trigger_Encounter
```

### Blueprint Organization

**Folder Structure:**
```
Content/
├── Blueprints/
│   ├── Characters/
│   ├── Enemies/
│   ├── Props/
│   ├── Triggers/
│   └── UI/
```

**Blueprint Complexity:**
- Max nodes: 500 per blueprint
- Max functions: 20 per blueprint
- Max variables: 50 per blueprint

**Performance:**
- Minimize Tick usage
- Use timers instead of Tick when possible
- Cache references
- Use interfaces for communication

## Particles and VFX

### Niagara Systems

**Naming Convention:**
```
NS_[Category]_[Name]
Examples:
- NS_Environment_Dust
- NS_Character_Blood
- NS_Weapon_MuzzleFlash
```

**Performance:**
- Max particles: 1000 per system
- Max emitters: 5 per system
- GPU simulation preferred for large counts

**Particle Budget:**
- Environment effects: 200-500 particles
- Character effects: 100-300 particles
- Weapon effects: 50-200 particles

### Particle Materials

**Requirements:**
- Unlit shader model
- Translucent blend mode
- Minimal texture samples (2-3)
- Simple shader instructions (< 50)

## UI Assets

### UI Textures

**Specifications:**
- Format: PNG (source), BC7 (engine)
- Max size: 1024x1024
- Power-of-two dimensions
- Transparent background when needed

**Naming Convention:**
```
UI_[Category]_[Name]
Examples:
- UI_Icon_Evidence
- UI_Button_Interact
- UI_Background_Menu
```

### UI Widgets

**Naming Convention:**
```
WBP_[Category]_[Name]
Examples:
- WBP_HUD_Main
- WBP_Menu_Pause
- WBP_Archive_Viewer
```

**Performance:**
- Minimize widget complexity
- Use invalidation boxes
- Cache widget references
- Avoid Tick in widgets

## Asset Validation

### Automated Checks

**Run validation:**
```powershell
.\Scripts\ValidateAssets.ps1
```

**Checks:**
- Naming conventions
- File sizes
- Polygon counts
- Texture resolutions
- Material complexity
- Missing references

### Manual Review

**Quality Checklist:**
- [ ] Follows naming convention
- [ ] Within polygon budget
- [ ] LODs generated
- [ ] Textures compressed
- [ ] Materials optimized
- [ ] No errors or warnings
- [ ] Tested in-game

## Asset Pipeline

### Import Settings

**Static Meshes:**
- Generate Lightmap UVs: Yes
- Combine Meshes: No
- Auto Generate Collision: No (use custom)
- Import Materials: Yes
- Import Textures: Yes

**Skeletal Meshes:**
- Import Mesh: Yes
- Import Skeleton: Yes (or use existing)
- Import Animations: No (separate)
- Update Skeleton Reference Pose: No

**Textures:**
- Compression Settings: Auto
- sRGB: Based on type
- Mip Gen Settings: FromTextureGroup
- Texture Group: Based on usage

### Version Control

**Source Assets:**
- Store in separate repository
- Use Git LFS for large files
- Keep organized folder structure

**Engine Assets:**
- Commit .uasset files
- Don't commit Intermediate/
- Don't commit Saved/

---
Last Updated: 2026-04-26
