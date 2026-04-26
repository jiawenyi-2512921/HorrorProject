# Texture Optimization Guide

## Comprehensive Texture Optimization for HorrorProject

**Goal**: Reduce texture memory usage by 30-40% while maintaining visual quality
**Target**: 2-3GB texture streaming pool
**Tools**: OptimizeTextures.ps1, UE5 Texture Tools

---

## Understanding Texture Impact

### Memory Cost Calculation
```
Uncompressed Size = Width × Height × Bytes Per Pixel × Mip Levels
Compressed Size = Uncompressed Size / Compression Ratio

Example (2K texture, RGBA, BC7):
- Uncompressed: 2048 × 2048 × 4 × 1.33 = 22.4 MB
- Compressed (BC7): 22.4 MB / 6 = 3.7 MB
```

### Performance Impact
- **VRAM Usage**: Directly affects GPU memory
- **Streaming**: Larger textures = longer load times
- **Bandwidth**: More data to transfer
- **Cache Misses**: Larger textures = more cache misses

---

## Texture Size Guidelines

### Resolution Targets

#### Hero Assets (Close-up, Important)
- **Base Color**: 2048×2048 (2K)
- **Normal Map**: 2048×2048 (2K)
- **Roughness**: 1024×1024 (1K)
- **Metallic**: 1024×1024 (1K)
- **Emissive**: 1024×1024 (1K)

#### Standard Assets (Medium Distance)
- **Base Color**: 1024×1024 (1K)
- **Normal Map**: 1024×1024 (1K)
- **Roughness**: 512×512
- **Metallic**: 512×512
- **Emissive**: 512×512

#### Background Assets (Distant, Repeated)
- **Base Color**: 512×512
- **Normal Map**: 512×512
- **Roughness**: 256×256
- **Metallic**: 256×256

#### UI Textures
- **Icons**: 256×256 or 512×512
- **Backgrounds**: 1024×1024 max
- **HUD Elements**: 512×512

---

## Compression Settings

### Compression Formats

#### BC7 (High Quality, Slower)
**Use For**: Hero textures, close-up details
- Best quality for color textures
- Supports alpha channel
- Higher compression time
```
CompressionSettings: TC_Default
CompressionQuality: High
```

#### BC5 (Normal Maps)
**Use For**: All normal maps
- Optimized for normal map data
- 2-channel compression
- Excellent quality/size ratio
```
CompressionSettings: TC_Normalmap
CompressionQuality: Default
```

#### BC4 (Single Channel)
**Use For**: Roughness, metallic, masks
- Single channel compression
- Best for grayscale data
- Smallest file size
```
CompressionSettings: TC_Grayscale
CompressionQuality: Default
```

#### BC1 (Fast, Lower Quality)
**Use For**: Background textures, distant objects
- Fastest compression
- No alpha support
- Smallest size
```
CompressionSettings: TC_Default
CompressionQuality: Low
```

---

## Optimization Workflow

### Step 1: Analysis
```powershell
# Analyze current texture usage
.\OptimizeTextures.ps1 -DryRun

# Review report
# Identify largest textures
# Prioritize by memory impact
```

### Step 2: Categorization
Classify textures by importance:
1. **Critical**: Player character, key props, close-up objects
2. **Important**: Environment, frequently seen objects
3. **Standard**: Background, distant objects
4. **Low Priority**: Rarely seen, far distance

### Step 3: Optimization
```powershell
# Balanced optimization (recommended)
.\OptimizeTextures.ps1

# Aggressive optimization (maximum performance)
.\OptimizeTextures.ps1 -Aggressive
```

### Step 4: Validation
1. Visual inspection in-game
2. Memory profiling
3. Performance testing
4. Quality comparison

---

## Advanced Techniques

### 1. Texture Atlasing
**Benefit**: Reduce draw calls, improve batching

**Implementation**:
```
Combine multiple small textures into single atlas:
- UI elements → Single UI atlas
- Small props → Prop atlas
- Decals → Decal atlas
```

**Considerations**:
- Maintain power-of-2 dimensions
- Add padding between textures (2-4 pixels)
- Use consistent resolution per atlas

### 2. Channel Packing
**Benefit**: Reduce texture count by 50-75%

**Common Packing Schemes**:
```
ORM Texture (Occlusion/Roughness/Metallic):
- R: Ambient Occlusion
- G: Roughness
- B: Metallic
- A: (unused or height)

Mask Texture:
- R: Mask 1
- G: Mask 2
- B: Mask 3
- A: Mask 4
```

**Implementation**:
```cpp
// In material
float3 ORM = Texture2DSample(ORMTexture, Sampler, UV).rgb;
float AO = ORM.r;
float Roughness = ORM.g;
float Metallic = ORM.b;
```

### 3. Virtual Texturing
**Benefit**: Stream only visible texture data

**Configuration** (DefaultEngine.ini):
```ini
[/Script/Engine.RendererSettings]
r.VirtualTexture=True
r.VT.MaxUploadsPerFrame=16
r.VT.MaxPagesProducedPerFrame=16
```

**Usage**:
- Enable "Virtual Texture Streaming" on large textures
- Best for terrain, large surfaces
- Reduces memory by 60-80%

### 4. Texture Streaming
**Benefit**: Load textures on-demand

**Configuration**:
```ini
[/Script/Engine.RendererSettings]
r.Streaming.PoolSize=3000
r.Streaming.MaxEffectiveScreenSize=0
r.Streaming.FullyLoadUsedTextures=0
```

**Per-Texture Settings**:
- Never Stream: Critical textures (UI, HUD)
- Always Stream: Large environment textures
- Distance-Based: Props, details

### 5. Mip Map Optimization
**Benefit**: Reduce aliasing, improve performance

**Settings**:
```
Mip Gen Settings:
- FromTextureGroup: Use texture group defaults
- SimpleAverage: Fast, good for most textures
- Sharpen0-10: Sharpen distant mips (use sparingly)
- Blur1-5: Blur distant mips (reduce aliasing)
```

**LOD Bias**:
```ini
; Force lower mip levels for distant objects
r.Streaming.MipBias=1  ; Use 1 level lower mips
```

---

## Horror Game Specific Optimizations

### Atmospheric Textures
**Challenge**: Maintain dark, moody atmosphere with lower resolution

**Solutions**:
1. **Use Detail Textures**
   - Low-res base + high-res detail overlay
   - Detail texture tiles at high frequency
   - Maintains close-up quality

2. **Emissive Optimization**
   - Use lower resolution for emissive
   - Rely on bloom for glow effect
   - 512×512 often sufficient

3. **Fog & Post-Processing**
   - Use volumetric fog to hide texture detail
   - Grain and noise hide compression artifacts
   - Chromatic aberration masks low resolution

### Shadow & Darkness
**Benefit**: Lower resolution less noticeable in dark areas

**Strategy**:
- Reduce resolution for textures in dark areas
- Prioritize lit areas and focal points
- Use ambient occlusion to add depth

---

## Texture Streaming Configuration

### Streaming Pool Size
```ini
[/Script/Engine.RendererSettings]
; Texture streaming pool (MB)
r.Streaming.PoolSize=3000  ; 3GB for Epic quality
r.Streaming.PoolSize=2000  ; 2GB for High quality
r.Streaming.PoolSize=1500  ; 1.5GB for Medium quality
```

### Streaming Priorities
```cpp
// In texture properties
Priority: 0-1000
- 1000: Critical (UI, HUD)
- 500-999: Important (hero assets)
- 100-499: Standard (environment)
- 0-99: Low priority (distant, background)
```

### Distance-Based Streaming
```ini
r.Streaming.MaxEffectiveScreenSize=0  ; No limit
r.Streaming.MinMipForSplitRequest=7   ; Min mip for split requests
```

---

## Quality vs Performance Profiles

### Epic Quality (Target: 60 FPS)
```
Hero Assets: 2K
Standard Assets: 1K
Background: 512
Compression: BC7/BC5
Streaming Pool: 3GB
```

### High Quality (Target: 75 FPS)
```
Hero Assets: 1K
Standard Assets: 1K
Background: 512
Compression: BC7/BC5
Streaming Pool: 2GB
```

### Medium Quality (Target: 90 FPS)
```
Hero Assets: 1K
Standard Assets: 512
Background: 256
Compression: BC1/BC5
Streaming Pool: 1.5GB
```

### Low Quality (Target: 120 FPS)
```
Hero Assets: 512
Standard Assets: 512
Background: 256
Compression: BC1/BC4
Streaming Pool: 1GB
```

---

## Optimization Checklist

### Pre-Optimization
- [ ] Run baseline profiling
- [ ] Document current memory usage
- [ ] Identify largest textures
- [ ] Categorize by importance
- [ ] Create backup

### Optimization
- [ ] Resize textures per guidelines
- [ ] Apply appropriate compression
- [ ] Pack channels where possible
- [ ] Enable streaming
- [ ] Configure mip maps
- [ ] Set LOD bias

### Post-Optimization
- [ ] Visual quality check
- [ ] Memory profiling
- [ ] Performance benchmarking
- [ ] Compare before/after
- [ ] Document changes

---

## Common Issues & Solutions

### Issue: Blurry Textures
**Causes**:
- Over-compression
- Too aggressive downscaling
- Incorrect mip bias

**Solutions**:
- Increase resolution for hero assets
- Use BC7 instead of BC1
- Adjust mip bias: `r.Streaming.MipBias=0`
- Enable texture sharpening

### Issue: Texture Popping
**Causes**:
- Streaming pool too small
- Aggressive streaming settings
- Insufficient mip levels

**Solutions**:
- Increase streaming pool size
- Adjust `r.Streaming.MaxEffectiveScreenSize`
- Preload critical textures
- Use higher priority for visible textures

### Issue: High Memory Usage
**Causes**:
- Too many high-res textures
- Streaming disabled
- Never Stream enabled on large textures

**Solutions**:
- Enable streaming globally
- Reduce texture resolutions
- Use channel packing
- Implement virtual texturing

### Issue: Long Loading Times
**Causes**:
- Large uncompressed textures
- Synchronous loading
- No streaming

**Solutions**:
- Enable async loading
- Use texture streaming
- Reduce initial load set
- Implement level streaming

---

## Monitoring & Maintenance

### Runtime Monitoring
```
Console Commands:
stat streaming       - Streaming statistics
stat texture         - Texture memory usage
stat memory          - Overall memory
r.DumpTextureMemory  - Detailed texture report
```

### Regular Audits
- Weekly: Check for new large textures
- Monthly: Full texture audit
- Per-Release: Comprehensive optimization pass

### Performance Budgets
```
Total Texture Memory: 2-3GB
Per-Texture Maximum: 4MB
UI Textures: 100MB total
Environment: 1-1.5GB
Characters: 300-500MB
Props: 500-800MB
```

---

## Tools & Resources

### UE5 Built-in Tools
- **Texture Statistics**: View → Statistics → Texture Stats
- **Size Map**: Window → Size Map
- **Reference Viewer**: Right-click asset → Reference Viewer

### Console Commands
```
stat texture              - Texture memory stats
stat streaming            - Streaming stats
r.DumpTextureMemory       - Detailed report
r.TextureStreaming 0/1    - Toggle streaming
r.Streaming.PoolSize X    - Set pool size
```

### External Tools
- **RenderDoc**: GPU profiling
- **PIX**: DirectX debugging
- **Photoshop/GIMP**: Texture editing
- **Compressonator**: Compression testing

---

## Best Practices Summary

1. **Measure First**: Always profile before optimizing
2. **Prioritize**: Focus on largest textures first
3. **Test Incrementally**: Optimize in small batches
4. **Visual Validation**: Always check in-game quality
5. **Document Changes**: Track all modifications
6. **Use Streaming**: Enable for all large textures
7. **Pack Channels**: Combine single-channel textures
8. **Appropriate Compression**: Match format to content
9. **LOD Bias**: Use for distant objects
10. **Monitor Continuously**: Regular performance checks

---

## Quick Reference

### Optimization Command
```powershell
# Preview optimizations
.\OptimizeTextures.ps1 -DryRun

# Apply balanced optimization
.\OptimizeTextures.ps1

# Apply aggressive optimization
.\OptimizeTextures.ps1 -Aggressive
```

### Target Resolutions
- Hero: 2K
- Standard: 1K
- Background: 512
- UI: 512-1K

### Compression
- Color: BC7
- Normal: BC5
- Grayscale: BC4
- Fast: BC1

### Memory Target
- Total: 2-3GB
- Streaming Pool: 3GB
- Per-Texture Max: 4MB
