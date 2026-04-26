# Niagara Asset Requirements

## Overview

This document lists all required assets (textures, materials, meshes, sounds) for the 8 Niagara particle systems in the Horror Project. Use this as a checklist for asset creation and procurement.

## Asset Summary

### Total Asset Count
- **Textures**: 24 files (~3.5MB total)
- **Materials**: 16 files
- **Material Instances**: 8 files
- **Meshes**: 8 files (~50KB total)
- **Niagara Systems**: 8 files
- **Niagara Emitters**: 11 files
- **Sound Effects**: 8 files (~2MB total)

### Total Memory Budget
- **Textures**: ~3.5MB
- **Materials**: ~500KB
- **Meshes**: ~50KB
- **Sounds**: ~2MB
- **Total**: ~6MB

## 1. NS_Dust_Particles Assets

### Textures

| Asset Name | Type | Resolution | Format | Size | Description |
|------------|------|------------|--------|------|-------------|
| T_Dust_D | Diffuse | 128x128 | BC1 | ~10KB | Soft circular gradient for dust particle |
| T_Dust_O | Opacity | 128x128 | BC4 | ~10KB | Alpha mask for dust particle |

**Texture Requirements**:
- T_Dust_D: Soft, featureless circular gradient, white/gray
- T_Dust_O: Radial gradient alpha mask, soft edges

### Materials

| Asset Name | Type | Domain | Blend Mode | Description |
|------------|------|--------|------------|-------------|
| M_DustParticle | Material | Surface | Translucent | Main dust particle material |
| MI_DustParticle_Inst | Instance | - | - | Runtime material instance |

**Material Parameters**:
- BaseColor (Vector3): (0.8, 0.8, 0.75)
- Opacity (Scalar): 0.3
- EmissiveStrength (Scalar): 0.1
- DepthFade (Scalar): 50.0

### Niagara Assets

| Asset Name | Type | Description |
|------------|------|-------------|
| NS_Dust_Particles | System | Main dust particle system |
| NE_DustEmitter | Emitter | Dust particle emitter |

## 2. NS_Fog_Volume Assets

### Textures

| Asset Name | Type | Resolution | Format | Size | Description |
|------------|------|------------|--------|------|-------------|
| T_Fog_Noise_D | Diffuse | 512x512 | BC1 | ~170KB | Perlin noise pattern for fog |
| T_Fog_Opacity | Opacity | 256x256 | BC4 | ~65KB | Soft radial gradient for fog opacity |

**Texture Requirements**:
- T_Fog_Noise_D: Tileable Perlin noise, grayscale, seamless
- T_Fog_Opacity: Large soft radial gradient, very soft edges

### Materials

| Asset Name | Type | Domain | Blend Mode | Description |
|------------|------|--------|------------|-------------|
| M_VolumetricFog | Material | Surface | Translucent | Main volumetric fog material |
| MI_VolumetricFog_Inst | Instance | - | - | Runtime material instance |

**Material Parameters**:
- FogColor (Vector3): (0.6, 0.65, 0.7)
- Opacity (Scalar): 0.15
- DepthFade (Scalar): 100.0
- NoiseScale (Scalar): 2.0
- NoisePan (Vector2): (0.02, 0.01)
- LightAbsorption (Scalar): 0.8

### Niagara Assets

| Asset Name | Type | Description |
|------------|------|-------------|
| NS_Fog_Volume | System | Main volumetric fog system |
| NE_FogEmitter | Emitter | Fog particle emitter |

## 3. NS_Flashlight_Beam Assets

### Textures

| Asset Name | Type | Resolution | Format | Size | Description |
|------------|------|------------|--------|------|-------------|
| T_Dust_Soft | Diffuse | 128x128 | BC1 | ~10KB | Soft particle for beam dust |

**Texture Requirements**:
- T_Dust_Soft: Very soft circular gradient, white, featureless

### Materials

| Asset Name | Type | Domain | Blend Mode | Description |
|------------|------|--------|------------|-------------|
| M_FlashlightBeam | Material | Surface | Additive | Flashlight beam material |
| MI_FlashlightBeam_Inst | Instance | - | - | Runtime material instance |

**Material Parameters**:
- BeamColor (Vector3): (1.0, 0.95, 0.85)
- Intensity (Scalar): 1.0
- DepthFade (Scalar): 30.0
- DistanceFade (Scalar): 800.0

### Niagara Assets

| Asset Name | Type | Description |
|------------|------|-------------|
| NS_Flashlight_Beam | System | Main flashlight beam system |
| NE_BeamEmitter | Emitter | Beam particle emitter |

## 4. NS_Camera_Noise Assets

### Textures

| Asset Name | Type | Resolution | Format | Size | Description |
|------------|------|------------|--------|------|-------------|
| T_FilmGrain | Noise | 256x256 | BC1 | ~65KB | Film grain noise pattern |

**Texture Requirements**:
- T_FilmGrain: Random noise pattern, tileable, grayscale

### Materials

| Asset Name | Type | Domain | Blend Mode | Description |
|------------|------|--------|------------|-------------|
| M_CameraNoise | Material | Surface | Translucent | Screen-space camera noise material |
| MI_CameraNoise_Inst | Instance | - | - | Runtime material instance |

**Material Parameters**:
- NoiseIntensity (Scalar): 1.0
- ColoredNoise (Scalar): 0.0
- BlendMode (Scalar): 0.5

### Niagara Assets

| Asset Name | Type | Description |
|------------|------|-------------|
| NS_Camera_Noise | System | Main camera noise system |
| NE_NoiseEmitter | Emitter | Noise particle emitter |

## 5. NS_Blood_Splatter Assets

### Textures

| Asset Name | Type | Resolution | Format | Size | Description |
|------------|------|------------|--------|------|-------------|
| T_Blood_D | Diffuse | 256x256 | BC1 | ~65KB | Blood droplet shapes (4x4 flipbook) |
| T_Blood_N | Normal | 256x256 | BC5 | ~85KB | Blood droplet normals |
| T_BloodSplat_D | Diffuse | 512x512 | BC1 | ~170KB | Blood splatter decal patterns (8 variations) |
| T_BloodSplat_N | Normal | 512x512 | BC5 | ~340KB | Blood splatter decal normals |

**Texture Requirements**:
- T_Blood_D: 16 frames of blood droplet shapes, dark red
- T_Blood_N: Matching normal maps for droplets
- T_BloodSplat_D: 8 unique blood splatter patterns, dark red
- T_BloodSplat_N: Matching normal maps for splatters

### Materials

| Asset Name | Type | Domain | Blend Mode | Description |
|------------|------|--------|------------|-------------|
| M_BloodParticle | Material | Surface | Translucent | Blood particle material |
| M_BloodDecal | Material | Deferred Decal | Translucent | Blood splatter decal material |
| MI_BloodParticle_Inst | Instance | - | - | Runtime particle instance |
| MI_BloodDecal_Inst | Instance | - | - | Runtime decal instance |

**Material Parameters (Particle)**:
- BloodColor (Vector3): (0.5, 0.08, 0.08)
- Metallic (Scalar): 0.1
- Roughness (Scalar): 0.7
- DepthFade (Scalar): 20.0

**Material Parameters (Decal)**:
- BloodColor (Vector3): (0.3, 0.05, 0.05)
- Opacity (Scalar): 0.8
- Roughness (Scalar): 0.6
- FadeTime (Scalar): 60.0

### Niagara Assets

| Asset Name | Type | Description |
|------------|------|-------------|
| NS_Blood_Splatter | System | Main blood splatter system |
| NE_BloodEmitter | Emitter | Blood particle emitter |

### Audio Assets

| Asset Name | Type | Format | Size | Description |
|------------|------|--------|------|-------------|
| SFX_BloodImpact_01 | Sound Cue | WAV | ~100KB | Blood impact sound variation 1 |
| SFX_BloodImpact_02 | Sound Cue | WAV | ~100KB | Blood impact sound variation 2 |
| SFX_BloodImpact_03 | Sound Cue | WAV | ~100KB | Blood impact sound variation 3 |

## 6. NS_Stone_Debris Assets

### Textures

| Asset Name | Type | Resolution | Format | Size | Description |
|------------|------|------------|--------|------|-------------|
| T_Concrete_D | Diffuse | 512x512 | BC1 | ~170KB | Concrete/stone base color |
| T_Concrete_N | Normal | 512x512 | BC5 | ~340KB | Concrete/stone normals |
| T_Concrete_ORM | Packed | 512x512 | BC1 | ~170KB | Occlusion/Roughness/Metallic packed |

**Texture Requirements**:
- T_Concrete_D: Gray concrete texture, tileable
- T_Concrete_N: Matching normal map with surface detail
- T_Concrete_ORM: R=AO, G=Roughness, B=Metallic (0)

### Materials

| Asset Name | Type | Domain | Blend Mode | Description |
|------------|------|--------|------------|-------------|
| M_StoneDebris | Material | Surface | Opaque | Stone debris material |
| MI_StoneDebris_Inst | Instance | - | - | Runtime material instance |

**Material Parameters**:
- BaseColor (Vector3): (0.45, 0.45, 0.45)
- Roughness (Scalar): 0.8
- Metallic (Scalar): 0.0
- NormalStrength (Scalar): 1.0

### Meshes

| Asset Name | Triangles | LODs | Size | Description |
|------------|----------|------|-------------|
| SM_Debris_Chunk01 | 50-100 | 2 | ~5KB | Irregular debris chunk |
| SM_Debris_Chunk02 | 50-100 | 2 | ~5KB | Angular debris piece |
| SM_Debris_Chunk03 | 50-100 | 2 | ~5KB | Flat debris shard |
| SM_Debris_Chunk04 | 50-100 | 2 | ~5KB | Small rock debris |
| SM_Debris_Chunk05 | 50-100 | 2 | ~5KB | Elongated debris piece |
| SM_Debris_Chunk06 | 50-100 | 2 | ~5KB | Cubic debris chunk |
| SM_Debris_Chunk07 | 50-100 | 2 | ~5KB | Thin debris shard |
| SM_Debris_Chunk08 | 50-100 | 2 | ~5KB | Irregular debris fragment |

**Mesh Requirements**:
- LOD0: Full detail (50-100 triangles)
- LOD1: Simplified (25-50 triangles)
- Collision: Simple box or sphere
- Pivot: Center of mass
- UV: Single channel, tileable
- Scale: 10-20 units

### Niagara Assets

| Asset Name | Type | Description |
|------------|------|-------------|
| NS_Stone_Debris | System | Main stone debris system |
| NE_DebrisEmitter | Emitter | Debris particle emitter |

### Audio Assets

| Asset Name | Type | Format | Size | Description |
|------------|------|--------|------|-------------|
| SFX_StoneImpact_01 | Sound Cue | WAV | ~150KB | Stone impact sound - soft |
| SFX_StoneImpact_02 | Sound Cue | WAV | ~150KB | Stone impact sound - medium 1 |
| SFX_StoneImpact_03 | Sound Cue | WAV | ~150KB | Stone impact sound - medium 2 |
| SFX_StoneImpact_04 | Sound Cue | WAV | ~150KB | Stone impact sound - hard 1 |
| SFX_StoneImpact_05 | Sound Cue | WAV | ~150KB | Stone impact sound - hard 2 |

## 7. NS_Water_Drips Assets

### Textures

| Asset Name | Type | Resolution | Format | Size | Description |
|------------|------|------------|--------|------|-------------|
| T_WaterDrop_D | Diffuse | 128x128 | BC1 | ~10KB | Water droplet texture |
| T_WaterDrop_N | Normal | 128x128 | BC5 | ~20KB | Water droplet normals |

**Texture Requirements**:
- T_WaterDrop_D: Clear water droplet, slightly blue-tinted
- T_WaterDrop_N: Subtle surface tension normals

### Materials

| Asset Name | Type | Domain | Blend Mode | Description |
|------------|------|--------|------------|-------------|
| M_WaterDrop | Material | Surface | Translucent | Water droplet material |
| MI_WaterDrop_Inst | Instance | - | - | Runtime material instance |

**Material Parameters**:
- WaterColor (Vector3): (0.8, 0.9, 1.0)
- Refraction (Scalar): 0.3
- Opacity (Scalar): 0.6
- DepthFade (Scalar): 10.0

### Niagara Assets

| Asset Name | Type | Description |
|------------|------|-------------|
| NS_Water_Drips | System | Main water drip system |
| NE_DripEmitter | Emitter | Drip particle emitter |
| NE_SplashEmitter | Emitter | Splash sub-emitter |

### Audio Assets

| Asset Name | Type | Format | Size | Description |
|------------|------|--------|------|-------------|
| SFX_WaterDrip_01 | Sound Cue | WAV | ~50KB | Water drip sound variation 1 |
| SFX_WaterDrip_02 | Sound Cue | WAV | ~50KB | Water drip sound variation 2 |
| SFX_WaterDrip_03 | Sound Cue | WAV | ~50KB | Water drip sound variation 3 |

## 8. NS_Ambient_Particles Assets

### Textures

| Asset Name | Type | Resolution | Format | Size | Description |
|------------|------|------------|--------|------|-------------|
| T_Ambient_D | Diffuse | 256x256 | BC1 | ~65KB | Ambient particle shapes (4x4 flipbook) |
| T_Ambient_O | Opacity | 256x256 | BC4 | ~65KB | Ambient particle alpha masks |

**Texture Requirements**:
- T_Ambient_D: 16 frames of various particle shapes (dust, firefly, ember, etc.)
- T_Ambient_O: Matching alpha masks for each frame

### Materials

| Asset Name | Type | Domain | Blend Mode | Description |
|------------|------|--------|------------|-------------|
| M_AmbientParticle | Material | Surface | Translucent | Ambient particle material |
| MI_AmbientParticle_Inst | Instance | - | - | Runtime material instance |

**Material Parameters**:
- ParticleColor (Vector3): (0.8, 0.8, 0.8)
- Emissive (Scalar): 0.0
- Opacity (Scalar): 0.3
- DepthFade (Scalar): 50.0
- FlipbookFrame (Scalar): 0.0

### Niagara Assets

| Asset Name | Type | Description |
|------------|------|-------------|
| NS_Ambient_Particles | System | Main ambient particle system |
| NE_AmbientEmitter | Emitter | Ambient particle emitter |

## Asset Creation Guidelines

### Texture Creation

**General Requirements**:
- All textures must be power-of-two (128, 256, 512)
- Use appropriate compression (BC1, BC4, BC5)
- Generate mip maps for all textures
- Ensure seamless tiling where applicable
- Use sRGB for color textures, Linear for data

**Optimization**:
- Keep resolution as low as possible
- Use texture atlases when possible
- Share textures between effects
- Use compressed formats
- Enable texture streaming

**Naming Convention**:
```
T_[EffectName]_[Type]
Types: D (Diffuse), N (Normal), O (Opacity), ORM (Packed)
```

### Material Creation

**General Requirements**:
- Use appropriate material domain
- Implement depth fade for soft particles
- Expose key parameters
- Use material instances for variations
- Minimize texture samples

**Optimization**:
- Use Unlit shading when possible
- Minimize instruction count
- Avoid complex math operations
- Use static switches for features
- Test shader complexity

**Naming Convention**:
```
M_[EffectName] - Master material
MI_[EffectName]_Inst - Material instance
```

### Mesh Creation

**General Requirements**:
- Keep triangle count low (50-100)
- Create LODs (at least 2 levels)
- Use simple collision shapes
- Set pivot to center of mass
- Use single UV channel

**Optimization**:
- Minimize vertex count
- Use LODs aggressively
- Simple collision shapes
- Share meshes when possible
- Optimize UV layout

**Naming Convention**:
```
SM_[EffectName]_[Variation]
Example: SM_Debris_Chunk01
```

### Audio Creation

**General Requirements**:
- Format: WAV or OGG
- Sample Rate: 44.1kHz or 22.05kHz
- Bit Depth: 16-bit
- Channels: Mono (preferred) or Stereo
- Length: 0.1-2.0 seconds

**Optimization**:
- Use mono for non-directional sounds
- Compress with OGG Vorbis
- Keep file size small
- Use sound cues for variations
- Enable streaming for long sounds

**Naming Convention**:
```
SFX_[EffectName]_[Variation]
Example: SFX_StoneImpact_01
```

## Asset Procurement Checklist

### Phase 1: Textures
- [ ] T_Dust_D
- [ ] T_Dust_O
- [ ] T_Fog_Noise_D
- [ ] T_Fog_Opacity
- [ ] T_Dust_Soft
- [ ] T_FilmGrain
- [ ] T_Blood_D
- [ ] T_Blood_N
- [ ] T_BloodSplat_D
- [ ] T_BloodSplat_N
- [ ] T_Concrete_D
- [ ] T_Concrete_N
- [ ] T_Concrete_ORM
- [ ] T_WaterDrop_D
- [ ] T_WaterDrop_N
- [ ] T_Ambient_D
- [ ] T_Ambient_O

### Phase 2: Materials
- [ ] M_DustParticle
- [ ] M_VolumetricFog
- [ ] M_FlashlightBeam
- [ ] M_CameraNoise
- [ ] M_BloodParticle
- [ ] M_BloodDecal
- [ ] M_StoneDebris
- [ ] M_WaterDrop
- [ ] M_AmbientParticle

### Phase 3: Material Instances
- [ ] MI_DustParticle_Inst
- [ ] MI_VolumetricFog_Inst
- [ ] MI_FlashlightBeam_Inst
- [ ] MI_CameraNoise_Inst
- [ ] MI_BloodParticle_Inst
- [ ] MI_BloodDecal_Inst
- [ ] MI_StoneDebris_Inst
- [ ] MI_WaterDrop_Inst
- [ ] MI_AmbientParticle_Inst

### Phase 4: Meshes
- [ ] SM_Debris_Chunk01
- [ ] SM_Debris_Chunk02
- [ ] SM_Debris_Chunk03
- [ ] SM_Debris_Chunk04
- [ ] SM_Debris_Chunk05
- [ ] SM_Debris_Chunk06
- [ ] SM_Debris_Chunk07
- [ ] SM_Debris_Chunk08

### Phase 5: Audio
- [ ] SFX_BloodImpact_01-03
- [ ] SFX_StoneImpact_01-05
- [ ] SFX_WaterDrip_01-03

### Phase 6: Niagara Systems
- [ ] NS_Dust_Particles
- [ ] NS_Fog_Volume
- [ ] NS_Flashlight_Beam
- [ ] NS_Camera_Noise
- [ ] NS_Blood_Splatter
- [ ] NS_Stone_Debris
- [ ] NS_Water_Drips
- [ ] NS_Ambient_Particles

### Phase 7: Niagara Emitters
- [ ] NE_DustEmitter
- [ ] NE_FogEmitter
- [ ] NE_BeamEmitter
- [ ] NE_NoiseEmitter
- [ ] NE_BloodEmitter
- [ ] NE_DebrisEmitter
- [ ] NE_DripEmitter
- [ ] NE_SplashEmitter
- [ ] NE_AmbientEmitter

## Asset Directory Structure

```
Content/VFX/
├── Niagara/
│   ├── Atmospheric/
│   │   ├── NS_Dust_Particles
│   │   ├── NS_Fog_Volume
│   │   └── NS_Flashlight_Beam
│   ├── Impact/
│   │   ├── NS_Blood_Splatter
│   │   └── NS_Stone_Debris
│   ├── Ambient/
│   │   ├── NS_Water_Drips
│   │   └── NS_Ambient_Particles
│   └── Special/
│       └── NS_Camera_Noise
├── Materials/
│   ├── Particles/
│   │   ├── M_DustParticle
│   │   ├── M_VolumetricFog
│   │   ├── M_FlashlightBeam
│   │   ├── M_CameraNoise
│   │   ├── M_BloodParticle
│   │   ├── M_StoneDebris
│   │   ├── M_WaterDrop
│   │   └── M_AmbientParticle
│   ├── Decals/
│   │   └── M_BloodDecal
│   └── Instances/
│       └── [All material instances]
├── Textures/
│   ├── Particles/
│   │   ├── Dust/
│   │   ├── Fog/
│   │   ├── Blood/
│   │   ├── Water/
│   │   └── Ambient/
│   └── Noise/
│       └── T_FilmGrain
├── Meshes/
│   └── Debris/
│       └── [All debris meshes]
└── Audio/
    └── VFX/
        ├── Blood/
        ├── Stone/
        └── Water/
```

## Asset Validation Checklist

### Texture Validation
- [ ] Correct resolution (power-of-two)
- [ ] Correct format (BC1/BC4/BC5)
- [ ] Mip maps generated
- [ ] Compression settings correct
- [ ] sRGB/Linear correct
- [ ] File size acceptable
- [ ] Visual quality acceptable

### Material Validation
- [ ] Correct domain and blend mode
- [ ] Depth fade implemented
- [ ] Parameters exposed
- [ ] Material instance created
- [ ] Shader complexity acceptable
- [ ] Visual quality acceptable
- [ ] Performance acceptable

### Mesh Validation
- [ ] Triangle count acceptable
- [ ] LODs created
- [ ] Collision set up
- [ ] Pivot correct
- [ ] UVs correct
- [ ] File size acceptable
- [ ] Visual quality acceptable

### Audio Validation
- [ ] Correct format
- [ ] Correct sample rate
- [ ] Correct bit depth
- [ ] File size acceptable
- [ ] Audio quality acceptable
- [ ] No clipping or distortion

### Niagara Validation
- [ ] System works correctly
- [ ] Parameters exposed
- [ ] LOD implemented
- [ ] Culling enabled
- [ ] Performance within budget
- [ ] Visual quality acceptable
- [ ] Documentation complete

## Asset Delivery Format

### Texture Delivery
```
Format: TGA or PNG (uncompressed)
Resolution: As specified
Color Space: sRGB for color, Linear for data
Naming: As specified in requirements
```

### Mesh Delivery
```
Format: FBX
Units: Centimeters
Up Axis: Z
Forward Axis: Y
Triangulate: Yes
Naming: As specified in requirements
```

### Audio Delivery
```
Format: WAV (uncompressed)
Sample Rate: 44.1kHz
Bit Depth: 16-bit
Channels: Mono or Stereo as specified
Naming: As specified in requirements
```

## Notes

- All assets must be created or procured before Niagara system implementation
- Textures should be created first, then materials, then Niagara systems
- Test each asset individually before integration
- Verify all assets meet performance budgets
- Document any deviations from specifications
- Keep source files for all assets
- Version control all assets
