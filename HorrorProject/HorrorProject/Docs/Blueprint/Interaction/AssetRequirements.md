# Interactive Object Asset Requirements

## Overview
Complete asset requirements for all interactive object types. Use this guide to prepare or source assets for Blueprint implementation.

## Door Assets

### Static Meshes
**Door Frame:**
- Dimensions: 200x100x300 units (W x D x H)
- Pivot: Center bottom of frame
- Collision: Box collision, BlockAll preset
- LODs: 3 levels (LOD0: Full detail, LOD1: 50% tris, LOD2: 25% tris)
- Poly Count: LOD0 < 2000 tris

**Door Panel:**
- Dimensions: 90x5x280 units (W x D x H)
- Pivot: **CRITICAL** - At hinge edge (left or right side)
- Collision: Box collision, BlockAll preset
- LODs: 3 levels
- Poly Count: LOD0 < 1500 tris
- Must rotate cleanly around pivot

**Variants Needed:**
- Wooden door (residential)
- Metal door (industrial)
- Security door (modern)
- Prison door (bars)
- Ornate door (mansion)

### Materials
**Requirements:**
- Base Color map (2K resolution)
- Normal map (2K resolution)
- Roughness map (2K resolution)
- Metallic map (if metal door)
- Ambient Occlusion map
- Material instances for easy customization

**Shader Features:**
- Support for damage/wear
- Adjustable color tint
- Dirt/grime overlay
- Optional emissive (for windows)

### Audio Assets
**Required Sounds:**
- Door_Open (0.5-1.5s duration)
- Door_Close (0.5-1.5s duration)
- Door_Locked (0.3-0.8s duration)
- Door_Creak (optional, 1-2s)

**Audio Specs:**
- Format: WAV, 44.1kHz, 16-bit
- Mono or stereo
- Normalized to -3dB peak
- No clipping

**Variants by Material:**
- Wood: Creaky, solid thud
- Metal: Heavy clang, echo
- Security: Hydraulic hiss, beep

### Animation Curves (Optional)
**Door Open Curve:**
- Float curve, 0-1 range
- Smooth acceleration/deceleration
- Duration: 1-2 seconds
- Ease in/out for realistic motion

## Pickup Assets

### Static Meshes
**General Requirements:**
- Dimensions: 5x5x5 to 30x30x30 units
- Pivot: Center of object
- Collision: Sphere or box, OverlapAll preset
- LODs: 2 levels minimum
- Poly Count: LOD0 < 1000 tris

**Evidence Items:**
- Photos: 15x10x0.5 units
- Notes: 20x15x0.1 units
- Tape cassettes: 10x6x1.5 units
- USB drives: 5x2x0.5 units

**Key Items:**
- Old keys: 8x2x0.5 units
- Modern keys: 6x2x0.3 units
- Keycards: 8x5x0.1 units

**Tools:**
- Flashlight: 15x5x5 units
- Crowbar: 60x5x3 units
- Lockpick: 10x1x0.5 units

**Batteries:**
- AA battery: 5x1.5x1.5 units
- D battery: 6x3x3 units

### Materials
**Evidence Glow Material:**
- Base Color: Item-specific
- Emissive: Subtle glow (0.5-1.5 intensity)
- Emissive Color: Light blue or gold
- Roughness: 0.4-0.6
- Parameter: GlowIntensity (adjustable)

**Metal Materials:**
- Metallic: 0.9
- Roughness: 0.3-0.5
- Base Color: Metal type
- Normal map for scratches

**Paper Materials:**
- Roughness: 0.7-0.8
- Base Color: Aged paper texture
- Normal map: Paper fiber detail
- No metallic

### Particle Effects (Optional)
**Pickup Sparkle:**
- Subtle particle emission
- 5-10 particles/second
- Lifetime: 1-2 seconds
- Size: 2-5 units
- Color: Match glow color
- Velocity: Slow upward drift

### Audio Assets
**Required Sounds:**
- Pickup_Evidence (0.3-0.6s)
- Pickup_Key (0.3-0.6s)
- Pickup_Tool (0.3-0.6s)
- Pickup_Battery (0.3-0.6s)
- Pickup_Generic (0.3-0.6s)

**Audio Specs:**
- Format: WAV, 44.1kHz, 16-bit
- Mono
- Normalized to -3dB
- Satisfying "collect" sound

## Examinable Assets

### Static Meshes
**High-Detail Requirements:**
- Dimensions: Varies by object type
- Pivot: Center of object
- Collision: None (during examine) or simple box
- LODs: 3 levels, but LOD0 used during examine
- Poly Count: LOD0 < 5000 tris (can be higher for hero objects)
- **CRITICAL:** High-quality model for close inspection

**Small Objects (Artifacts):**
- Dimensions: 5x5x5 to 20x20x20 units
- High detail: Visible engravings, textures
- Examples: Idols, jewelry, tools

**Medium Objects (Statues):**
- Dimensions: 30x30x30 to 100x100x100 units
- Moderate detail
- Examples: Busts, boxes, skulls

**Flat Objects (Photos):**
- Dimensions: 20x15x0.5 units
- High-resolution texture for readability
- Examples: Photos, paintings, documents

### Materials
**High-Quality Requirements:**
- Base Color: 2K-4K resolution
- Normal map: 2K-4K resolution (essential)
- Roughness map: 2K resolution
- Metallic map: 2K resolution (if applicable)
- Ambient Occlusion: Baked or map
- Height/Displacement: Optional for extreme detail

**Material Features:**
- No LOD transitions during examine
- Support for dynamic lighting
- Detailed surface variation
- Readable text/engravings

### Audio Assets
**Required Sounds:**
- Examine_Start (0.3-0.6s)
- Examine_End (0.3-0.6s)
- Examine_Pickup (0.4-0.8s)
- Examine_Putdown (0.4-0.8s)

**Audio Specs:**
- Format: WAV, 44.1kHz, 16-bit
- Stereo for immersion
- Normalized to -3dB

## Document Assets

### Static Meshes
**Paper Documents:**
- Dimensions: 20x15x0.1 units (single sheet)
- Dimensions: 25x20x2 units (folded)
- Pivot: Center
- Collision: Box, OverlapAll
- Poly Count: < 500 tris

**Notebooks:**
- Dimensions: 20x15x3 units
- Closed state mesh
- Poly Count: < 1000 tris

**Photos:**
- Dimensions: 15x10x0.2 units (Polaroid)
- Dimensions: 20x15x0.1 units (Standard)
- High-res texture for content

### Textures
**Document Content Textures:**
- Resolution: 2K-4K (must be readable)
- Format: PNG or TGA
- Text must be legible when displayed
- Handwriting or typed text
- Aged/worn appearance

**Photo Textures:**
- Resolution: 2K minimum
- Format: PNG or TGA
- Clear, detailed image
- Optional: Grain, damage effects

### Materials
**Paper Material:**
- Base Color: Aged paper texture
- Roughness: 0.7-0.8
- Normal map: Paper fiber, creases
- No emissive
- Optional: Stains, coffee rings

**Photo Material:**
- Base Color: Photo content texture
- Roughness: 0.3-0.5 (glossy) or 0.6-0.8 (matte)
- Slight emissive for Polaroid border

### Audio Assets
**Required Sounds:**
- Paper_Rustle (0.5-1.0s)
- Paper_Fold (0.3-0.6s)
- Book_Open (0.5-1.0s)
- Book_Close (0.5-1.0s)
- Photo_Pickup (0.3-0.6s)
- Photo_Putdown (0.3-0.6s)

**Audio Specs:**
- Format: WAV, 44.1kHz, 16-bit
- Stereo
- Realistic paper/book sounds

## Recorder Assets

### Static Meshes
**Tape Recorder:**
- Dimensions: 30x20x15 units
- Pivot: Center bottom
- Collision: Box, BlockAll
- Poly Count: < 2000 tris
- Separate meshes: Base, buttons, tape reels

**Digital Recorder:**
- Dimensions: 15x8x3 units
- Pivot: Center bottom
- Collision: Box, BlockAll
- Poly Count: < 1000 tris
- LED indicator mesh/material

**Answering Machine:**
- Dimensions: 25x20x10 units
- Pivot: Center bottom
- Collision: Box, BlockAll
- Poly Count: < 1500 tris

### Materials
**Plastic Materials:**
- Base Color: Black, beige, or gray
- Roughness: 0.4-0.6
- Slight normal map for texture
- No metallic

**LED Indicator:**
- Emissive material
- Parameter: LED_Color (RGB)
- Parameter: LED_Intensity (0-10)
- States: Off (dim red), Playing (green), Paused (yellow)

**Animated Components:**
- Tape reels: Rotating during playback
- Material parameter for rotation
- Or skeletal mesh with animation

### Audio Assets
**Audio Log Content:**
- Format: WAV or OGG, 44.1kHz, 16-bit
- Mono or stereo
- Duration: 10 seconds to 3 minutes
- Normalized to -6dB (dialogue level)
- Clear, intelligible speech
- Optional: Background noise, static

**Recorder Sounds:**
- Recorder_Start (0.2-0.4s)
- Recorder_Stop (0.2-0.4s)
- Tape_Rewind (optional, 1-2s)

**Audio Specs:**
- Format: WAV, 44.1kHz, 16-bit
- Mono
- Mechanical sounds for tape recorders

### Subtitle/Transcript
**Text Requirements:**
- Full transcript of audio log
- Timestamped for subtitle sync
- Speaker identification
- Format: Plain text or JSON

**Example Format:**
```json
{
  "subtitles": [
    {"time": 0.0, "speaker": "John", "text": "If you're hearing this..."},
    {"time": 3.5, "speaker": "John", "text": "then I'm already gone."}
  ]
}
```

## Switch Assets

### Static Meshes
**Switch Base:**
- Dimensions: 15x10x5 units
- Pivot: Center, mounted to wall
- Collision: Box, BlockAll
- Poly Count: < 500 tris

**Switch Handle/Lever:**
- Dimensions: Varies by type
- Pivot: **CRITICAL** - At rotation point
- Collision: None or simple box
- Poly Count: < 300 tris
- Must rotate cleanly

**Variants:**
- Wall toggle switch: 10x5x3 units
- Lever switch: 20x5x5 units
- Push button: 8x8x5 units
- Valve wheel: 40x40x10 units
- Breaker switch: 15x8x5 units

### Materials
**Plastic Materials:**
- Base Color: White, beige, black
- Roughness: 0.5-0.7
- Slight normal for texture

**Metal Materials:**
- Base Color: Chrome, steel, brass
- Metallic: 0.9
- Roughness: 0.2-0.4
- Normal map for wear

**LED Indicator:**
- Emissive material
- Off: Red, dim (0.5 intensity)
- On: Green, bright (3.0 intensity)
- Parameter: LED_State (0=off, 1=on)

### Audio Assets
**Required Sounds:**
- Switch_On (0.2-0.5s)
- Switch_Off (0.2-0.5s)
- Button_Press (0.1-0.3s)
- Button_Release (0.1-0.3s)
- Lever_Pull (0.3-0.6s)
- Lever_Return (0.3-0.6s)

**Audio Specs:**
- Format: WAV, 44.1kHz, 16-bit
- Mono
- Satisfying mechanical click/clunk

**Variants by Type:**
- Toggle: Sharp click
- Button: Soft press/release
- Lever: Heavy clunk
- Breaker: Loud snap

## General Asset Guidelines

### File Organization
```
Content/
├── Meshes/
│   ├── Interaction/
│   │   ├── Doors/
│   │   ├── Pickups/
│   │   ├── Examinables/
│   │   ├── Documents/
│   │   ├── Recorders/
│   │   └── Switches/
├── Materials/
│   ├── Interaction/
│   │   ├── M_Door_Master
│   │   ├── M_Pickup_Glow_Master
│   │   ├── M_Paper_Master
│   │   └── M_Switch_Master
├── Textures/
│   ├── Interaction/
│   │   ├── Doors/
│   │   ├── Documents/
│   │   └── [Type]/
├── Audio/
│   ├── SFX/
│   │   ├── Interaction/
│   │   │   ├── Doors/
│   │   │   ├── Pickups/
│   │   │   └── [Type]/
│   └── AudioLogs/
└── Particles/
    └── Interaction/
        └── PS_Pickup_Sparkle
```

### Naming Conventions
```
Static Meshes: SM_[Type]_[Variant]_[Number]
Materials: M_[Type]_[Variant]_Inst
Textures: T_[Type]_[Map]_[Variant]
Sounds: SFX_[Type]_[Action]
Audio Logs: SW_AudioLog_[Name]

Examples:
- SM_Door_Panel_Wood_01
- M_Wood_Door_Inst
- T_Door_BaseColor_Wood
- SFX_Door_Wood_Open
- SW_AudioLog_Victim_01
```

### Performance Targets

**Poly Counts:**
- Doors: < 3500 tris total (frame + panel)
- Pickups: < 1000 tris
- Examinables: < 5000 tris (hero objects)
- Documents: < 500 tris
- Recorders: < 2000 tris
- Switches: < 800 tris total

**Texture Memory:**
- Doors: 8-16 MB per variant
- Pickups: 2-4 MB per item
- Examinables: 16-32 MB (high detail)
- Documents: 4-8 MB (readable text)
- Recorders: 8-12 MB
- Switches: 2-4 MB

**Audio Memory:**
- SFX: < 500 KB per sound
- Audio Logs: 1-5 MB per log (compressed)
- Use streaming for long audio logs

### Quality Standards

**Mesh Quality:**
- Clean topology, no overlapping faces
- Proper UV unwrapping, no stretching
- Lightmap UVs (channel 1)
- Correct pivot placement
- Appropriate collision

**Texture Quality:**
- No visible seams
- Consistent texel density
- Proper mip-mapping
- Compressed appropriately (BC1, BC3, BC5)

**Audio Quality:**
- No clipping or distortion
- Consistent volume levels
- Clean recordings, minimal noise
- Proper fade in/out

## Asset Sourcing

### Recommended Asset Stores
- Unreal Marketplace
- Quixel Megascans (materials, textures)
- Freesound.org (audio, CC licensed)
- CGTrader (meshes)
- TurboSquid (meshes)

### Custom Asset Creation
**3D Modeling:**
- Blender (free)
- Maya
- 3ds Max
- ZBrush (high-detail sculpting)

**Texturing:**
- Substance Painter
- Substance Designer
- Photoshop
- GIMP (free)

**Audio:**
- Audacity (free)
- Adobe Audition
- Reaper
- Field recording equipment

## Asset Checklist

### Before Import
- [ ] Correct scale (1 unit = 1 cm in Unreal)
- [ ] Proper pivot placement
- [ ] Clean mesh topology
- [ ] UV unwrapping complete
- [ ] Textures at correct resolution
- [ ] Audio normalized and trimmed

### After Import
- [ ] LODs generated or imported
- [ ] Collision set up correctly
- [ ] Materials applied and instanced
- [ ] Lightmap UVs verified
- [ ] Asset naming follows convention
- [ ] Organized in correct folder

### Testing
- [ ] Mesh displays correctly in-game
- [ ] Materials render properly
- [ ] Collision works as expected
- [ ] Audio plays without issues
- [ ] Performance within targets
- [ ] No visual artifacts

## Common Issues and Solutions

### Issue: Mesh rotates from wrong pivot
**Solution:** Re-export mesh with correct pivot placement in 3D software

### Issue: Textures appear blurry
**Solution:** Increase texture resolution or check mip-map settings

### Issue: Audio pops or clicks
**Solution:** Add fade in/out, normalize audio, check for clipping

### Issue: Collision doesn't match visual
**Solution:** Regenerate collision or create custom collision mesh

### Issue: Material looks wrong in-game
**Solution:** Check lighting, adjust roughness/metallic values, verify normal map

### Issue: Performance issues
**Solution:** Reduce poly count, optimize textures, use LODs, check draw calls
