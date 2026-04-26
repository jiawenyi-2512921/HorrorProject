# Lighting Design Guide

## Overview

Lighting is the primary tool for atmosphere, navigation, and tension management in horror games. Proper lighting balances visibility with mood.

## Lighting Philosophy

### Core Principles

1. **Visibility First**: Players must see enough to navigate
2. **Mood Second**: Atmosphere enhances but doesn't block gameplay
3. **Contrast Creates Drama**: Light and shadow define spaces
4. **Color Tells Story**: Temperature and hue convey emotion
5. **Dynamic Sparingly**: Movement draws attention and costs performance

## Technical Foundation

### Lighting Methods

#### Static Lighting (Baked)
- **Use**: 90% of level lighting
- **Pros**: No performance cost, high quality, soft shadows
- **Cons**: No movement, long build times
- **Best For**: Ambient, fill, architectural lighting

#### Stationary Lighting
- **Use**: Key dramatic lights
- **Pros**: Dynamic shadows, moderate cost
- **Cons**: Limited count, shadow overlap issues
- **Best For**: Main light sources, important fixtures

#### Dynamic Lighting
- **Use**: Special effects only
- **Pros**: Full flexibility, real-time changes
- **Cons**: Expensive, hard shadows
- **Best For**: Flashlight, flickering effects, events

### Performance Budget (SM13)

#### Light Count Limits
- **Static Lights**: Unlimited (baked)
- **Stationary Lights**: 10-15 per scene
- **Dynamic Lights**: 5-8 visible simultaneously
- **Shadow-Casting Dynamics**: 3-5 maximum

#### Lightmap Resolution
- **Walls**: 128-256 texels/meter
- **Floors**: 64-128 texels/meter
- **Props**: 32-64 texels/meter
- **Small Objects**: 16-32 texels/meter

#### Shadow Quality
- **Static Shadows**: High (baked)
- **Stationary Shadows**: Medium (1024-2048)
- **Dynamic Shadows**: Low-Medium (512-1024)
- **Cascaded Shadow Maps**: 4 cascades

## Lighting Zones

### Zone-Based Approach

Each zone has distinct lighting characteristics aligned with 7 beats structure.

### 1. Entry Hall - Safe Introduction

#### Lighting Goals
- Establish baseline visibility
- Create false security
- Tutorial-friendly
- Clear navigation

#### Light Setup
**Ambient**
- Color: Warm white (3500K)
- Intensity: 1.5 lux
- Source: Skylight (baked)

**Key Lights** (8 total)
- Type: Ceiling fluorescents (static)
- Color: Cool white (5000K)
- Intensity: 200 lux
- Pattern: Grid layout
- Effect: 30% flicker on 2 lights

**Accent Lights** (3 total)
- Type: Desk lamp, exit signs
- Color: Warm (2700K) / Green (exit)
- Intensity: 50-100 lux
- Purpose: Points of interest

**Shadows**
- Soft, minimal
- Contact shadows only
- No dramatic shadows

#### Mood
- Safe, institutional
- Slightly unsettling (flickering)
- Clear visibility
- **Average Lux**: 150-200

### 2. Main Corridor - Transition

#### Lighting Goals
- Reduce security feeling
- Create uncertainty
- Maintain navigation
- Introduce darkness

#### Light Setup
**Ambient**
- Color: Neutral (4000K)
- Intensity: 0.8 lux
- Source: Skylight (baked)

**Key Lights** (12 total)
- Type: Wall sconces (static)
- Color: Warm white (3000K)
- Intensity: 80 lux
- Pattern: Alternating sides
- Effect: 50% are broken/off

**Emergency Lights** (4 total)
- Type: Red emergency (stationary)
- Color: Red (1000K)
- Intensity: 30 lux
- Purpose: Minimal visibility

**Dynamic Effects** (2 total)
- Type: Sparking wires (dynamic)
- Color: Blue-white (8000K)
- Intensity: 150 lux (pulsing)
- Purpose: Hazard indication

**Shadows**
- Medium contrast
- Long shadows from sconces
- Dramatic pools of darkness

#### Mood
- Uncertain, tense
- Pockets of safety and danger
- Requires flashlight
- **Average Lux**: 50-100

### 3. West Wing - Exploration

#### Lighting Goals
- Reward exploration
- Create intimate spaces
- Support evidence discovery
- Varied lighting conditions

#### Light Setup
**Ambient**
- Color: Cool (4500K)
- Intensity: 0.5 lux
- Source: Moonlight through windows (baked)

**Key Lights** (15 total)
- Type: Desk lamps, overhead (static)
- Color: Warm (2700K)
- Intensity: 60 lux
- Pattern: Per-room variation
- Effect: Only 40% working

**Window Light** (6 windows)
- Type: Directional (stationary)
- Color: Blue moonlight (7000K)
- Intensity: 40 lux
- Purpose: Atmospheric, navigation

**Candles** (8 total)
- Type: Point lights (static)
- Color: Orange (2000K)
- Intensity: 20 lux
- Purpose: Player-placed safe zones

**Shadows**
- High contrast
- Window shadows dramatic
- Desk lamp pools

#### Mood
- Mysterious, exploratory
- Safe pockets (lit desks)
- Dark corners (secrets)
- **Average Lux**: 30-80

### 4. East Wing - Danger

#### Lighting Goals
- Create threat
- High contrast
- Visibility for gameplay
- Unsettling atmosphere

#### Light Setup
**Ambient**
- Color: Cold (6000K)
- Intensity: 0.3 lux
- Source: Minimal skylight

**Key Lights** (10 total)
- Type: Surgical lights (stationary)
- Color: Harsh white (6500K)
- Intensity: 250 lux (spotlights)
- Pattern: Focused pools
- Effect: High contrast

**Fluorescents** (8 total)
- Type: Ceiling strips (static)
- Color: Sickly green (5500K + green tint)
- Intensity: 80 lux
- Effect: 70% flickering

**Morgue Lights** (4 total)
- Type: Refrigerator interior (dynamic)
- Color: Blue-white (7500K)
- Intensity: 100 lux
- Purpose: Objective 5 area

**Shadows**
- Extreme contrast
- Harsh, defined shadows
- Darkness between pools

#### Mood
- Clinical, threatening
- Uncomfortable visibility
- Nowhere to hide
- **Average Lux**: 20-150 (high variance)

### 5. Basement - Crisis

#### Lighting Goals
- Maximum tension
- Minimal visibility
- Flashlight essential
- Environmental hazards visible

#### Light Setup
**Ambient**
- Color: Warm (3000K)
- Intensity: 0.2 lux
- Source: Minimal indirect

**Key Lights** (6 total)
- Type: Single bulbs (static)
- Color: Dirty yellow (2500K)
- Intensity: 40 lux
- Pattern: Sparse, irregular
- Effect: 50% broken

**Boiler Glow** (1 large)
- Type: Emissive material + light (stationary)
- Color: Red-orange (1500K)
- Intensity: 60 lux
- Purpose: Hazard and atmosphere

**Ritual Chamber** (special)
- Type: Candle array (static) + dynamic effects
- Color: Purple-blue (custom)
- Intensity: 30 lux ambient, 150 lux during event
- Purpose: Objective 6 drama

**Water Reflections**
- Type: Planar reflection (limited)
- Purpose: Atmosphere
- Cost: Optimized, low-res

**Shadows**
- Very dark
- Minimal shadow detail
- Flashlight shadows critical

#### Mood
- Oppressive, claustrophobic
- Darkness dominates
- Fear of unseen
- **Average Lux**: 10-50

### 6. Upper Floor - Mystery

#### Lighting Goals
- Atmospheric exploration
- Moonlight dominance
- Reward discovery
- Vertical drama

#### Light Setup
**Ambient**
- Color: Blue (7000K)
- Intensity: 0.6 lux
- Source: Night sky (baked)

**Window Light** (8 large windows)
- Type: Directional (stationary)
- Color: Blue moonlight (7000K)
- Intensity: 50 lux
- Purpose: Primary illumination

**Candles** (12 total)
- Type: Point lights (static)
- Color: Warm orange (2000K)
- Intensity: 15 lux
- Purpose: Atmospheric accents

**Dust Particles**
- Type: Volumetric lighting (stationary)
- Purpose: God rays through windows
- Cost: Moderate, optimized

**Collapsed Section**
- Type: Exterior light bleeding in (static)
- Color: Blue-white (6500K)
- Intensity: 80 lux
- Purpose: Danger indication

**Shadows**
- Long, dramatic
- Window patterns on floor
- Atmospheric depth

#### Mood
- Melancholic, beautiful
- Dangerous beauty
- Exploration reward
- **Average Lux**: 20-60

### 7. Exit Sequence - Chaos

#### Lighting Goals
- Urgency and chaos
- Clear exit path
- Dynamic events
- Climactic atmosphere

#### Light Setup
**Ambient**
- Color: Warm (3500K)
- Intensity: 0.5 lux
- Source: Fire glow (baked)

**Emergency Lights** (6 total)
- Type: Flashing red (dynamic)
- Color: Red (1000K)
- Intensity: 100 lux (pulsing)
- Pattern: Alarm pattern (1Hz)
- Purpose: Urgency

**Fire/Sparks** (4 sources)
- Type: Emissive + dynamic lights
- Color: Orange-yellow (2000K)
- Intensity: 120 lux
- Purpose: Hazard and drama

**Exit Sign** (1 large)
- Type: Emissive (static) + spotlight (stationary)
- Color: Green (custom)
- Intensity: 200 lux
- Purpose: Clear goal

**Collapse Effects**
- Type: Dynamic lights (temporary)
- Purpose: Event lighting
- Duration: 2-5 seconds each

**Shadows**
- Chaotic, moving
- Multiple shadow directions
- Disorienting

#### Mood
- Panic, urgency
- Clear goal
- Overwhelming chaos
- **Average Lux**: 30-100 (highly variable)

## Special Lighting Techniques

### 1. Flashlight System

#### Technical Setup
- **Type**: Spotlight (dynamic)
- **Angle**: 45-60 degrees
- **Intensity**: 3000 lumens (300 lux at 3m)
- **Color**: Cool white (5500K)
- **Attenuation**: Inverse square, 10m range
- **Shadows**: High quality, 1024 resolution

#### Gameplay Integration
- Battery system (drains over time)
- Flicker when low battery
- Recharge stations in safe zones
- Essential for dark areas

#### Performance
- Single shadow-casting dynamic light
- Highest priority
- Optimized shadow distance (10m)

### 2. Volumetric Lighting

#### Use Cases
- God rays through windows (Upper Floor)
- Fog in basement
- Dust particles in abandoned areas
- Flashlight beam visibility

#### Technical Setup
- **Method**: Exponential height fog + light shafts
- **Quality**: Medium (performance balance)
- **Density**: Zone-specific (0.01-0.05)
- **Color**: Matches ambient light

#### Performance Cost
- Moderate (2-3ms GPU)
- Use sparingly
- Optimize fog bounds
- LOD based on distance

### 3. Emissive Materials

#### Use Cases
- Computer screens
- Exit signs
- Ritual symbols
- Glowing objects

#### Technical Setup
- **Intensity**: 1-10 (material parameter)
- **Color**: Varies by object
- **Bloom**: Enabled for bright emissives
- **Light Emission**: Optional (adds real light)

#### Performance
- Minimal cost (material only)
- Light emission adds cost
- Use material emissive when possible

### 4. Light Functions

#### Use Cases
- Flickering lights
- Rotating警报灯
- Pulsing effects
- Animated patterns

#### Technical Setup
- **Method**: Material function applied to light
- **Cost**: Minimal (texture lookup)
- **Patterns**: Sine wave, noise, custom textures
- **Control**: Blueprint-driven parameters

#### Examples
```
Flicker Pattern:
- Noise texture (128x128)
- Scroll speed: 0.5
- Intensity multiplier: 0.7-1.0

Pulse Pattern:
- Sine wave
- Frequency: 1Hz
- Intensity multiplier: 0.5-1.0
```

### 5. IES Profiles

#### Use Cases
- Realistic light fixtures
- Architectural accuracy
- Specific light patterns
- Professional look

#### Technical Setup
- **Source**: Real IES files from manufacturers
- **Application**: Point/Spot lights
- **Cost**: Minimal
- **Library**: Build custom IES library

#### Common Profiles
- Fluorescent tubes
- Desk lamps
- Ceiling fixtures
- Emergency lights

## Color Theory for Horror

### Color Temperature Guide

#### Warm (2000-3500K)
- **Emotion**: Comfort, false security
- **Use**: Safe zones, entry areas
- **Transition**: From warm to cool = increasing danger

#### Neutral (3500-5000K)
- **Emotion**: Institutional, sterile
- **Use**: Offices, corridors
- **Transition**: Baseline reality

#### Cool (5000-7000K)
- **Emotion**: Clinical, uncomfortable
- **Use**: Medical areas, moonlight
- **Transition**: Unnatural, threatening

#### Extreme Cool (7000K+)
- **Emotion**: Alien, supernatural
- **Use**: Ritual areas, special events
- **Transition**: Peak horror moments

### Color Accents

#### Red
- **Meaning**: Danger, blood, emergency
- **Use**: Emergency lights, hazards
- **Intensity**: Low (30-50 lux)

#### Green
- **Meaning**: Sickness, decay, exit
- **Use**: Fluorescents, exit signs
- **Intensity**: Medium (50-100 lux)

#### Blue
- **Meaning**: Cold, night, supernatural
- **Use**: Moonlight, ritual effects
- **Intensity**: Low-Medium (20-80 lux)

#### Purple
- **Meaning**: Unnatural, mystical
- **Use**: Ritual chamber, special events
- **Intensity**: Low (20-40 lux)

#### Orange/Yellow
- **Meaning**: Fire, warmth, old lighting
- **Use**: Candles, old bulbs, fire
- **Intensity**: Low (15-60 lux)

## Lighting for Gameplay

### Navigation Lighting

#### Principles
- Brighter areas = safe/important
- Darker areas = danger/optional
- Light paths guide players
- Contrast creates landmarks

#### Implementation
- Main path: 50+ lux minimum
- Side paths: 20-50 lux
- Secrets: < 20 lux
- Objectives: 100+ lux (highlighted)

### Tension Management

#### Safe Zones
- Bright (100-200 lux)
- Warm colors (2700-3500K)
- Soft shadows
- Even lighting

#### Danger Zones
- Dark (10-50 lux)
- Cool colors (5000-7000K)
- Hard shadows
- High contrast

#### Transition Zones
- Medium (50-100 lux)
- Neutral colors (3500-5000K)
- Mixed shadows
- Gradual change

### Enemy Visibility

#### Player Advantage
- Enemies visible in light
- Shadows hide player
- Flashlight reveals enemies
- Silhouettes against light

#### Enemy Advantage
- Darkness hides enemies
- Backlit player visible
- Sound becomes primary sense
- Uncertainty creates fear

## Lighting Workflow

### Phase 1: Lighting Blockout
1. Place key lights only
2. Establish zones
3. Test navigation
4. Verify performance
5. Iterate quickly

### Phase 2: Ambient Pass
1. Add skylight/ambient
2. Set color temperature per zone
3. Add fill lights
4. Balance overall exposure
5. Test mood

### Phase 3: Detail Pass
1. Add accent lights
2. Place practical lights (fixtures)
3. Add emissive materials
4. Implement light functions
5. Polish shadows

### Phase 4: Dynamic Pass
1. Add dynamic lights (minimal)
2. Implement flashlight
3. Add event lighting
4. Test performance
5. Optimize as needed

### Phase 5: Baking
1. Set lightmap resolution
2. Configure lightmass settings
3. Build lighting (production quality)
4. Fix light bleeding
5. Optimize lightmaps

### Phase 6: Polish
1. Add volumetric effects
2. Fine-tune colors
3. Balance all zones
4. Final performance pass
5. Playtesting

## Lightmass Settings

### Quality Settings (Production)
```
Static Lighting Level Scale: 0.5
Num Indirect Lighting Bounces: 3
Num Sky Lighting Bounces: 1
Indirect Lighting Quality: 4
Indirect Lighting Smoothness: 1.0
Environment Color: Black
Environment Intensity: 1.0
Diffuse Boost: 1.0
Use Ambient Occlusion: True
Direct Illumination Occlusion Fraction: 0.5
Indirect Illumination Occlusion Fraction: 1.0
Occlusion Exponent: 1.0
Fully Occluded Samples Fraction: 1.0
Max Occlusion Distance: 200
```

### Build Times (SM13 - 1300m²)
- **Preview**: 2-5 minutes
- **Medium**: 10-20 minutes
- **High**: 30-60 minutes
- **Production**: 1-3 hours

## Performance Optimization

### Optimization Checklist
- [ ] Static lights for 90% of lighting
- [ ] Stationary lights < 15
- [ ] Dynamic lights < 8 visible
- [ ] Shadow-casting dynamics < 5
- [ ] Lightmap resolution appropriate
- [ ] Light overlap minimized
- [ ] Volumetric effects optimized
- [ ] Light functions efficient
- [ ] No unnecessary lights
- [ ] Proper light attenuation

### Common Issues

#### Light Bleeding
- **Cause**: Thin walls, low lightmap resolution
- **Fix**: Increase lightmap res, thicken walls, use lightmass importance volume

#### Performance Spikes
- **Cause**: Too many dynamic lights
- **Fix**: Reduce count, increase attenuation, use static lights

#### Dark Spots
- **Cause**: Insufficient bounces, no fill light
- **Fix**: Add indirect bounces, place fill lights

#### Harsh Shadows
- **Cause**: Hard light sources, no ambient
- **Fix**: Add ambient light, use area lights, increase bounces

## Testing Checklist

### Technical Tests
- [ ] All zones properly lit
- [ ] Navigation clear
- [ ] No light bleeding
- [ ] Performance within budget
- [ ] Lightmaps built (production)
- [ ] No artifacts

### Gameplay Tests
- [ ] Player can navigate without frustration
- [ ] Objectives visible when intended
- [ ] Evidence discoverable
- [ ] Enemies visible when appropriate
- [ ] Flashlight necessary but not constant
- [ ] Safe zones feel safe

### Mood Tests
- [ ] Atmosphere appropriate per zone
- [ ] Tension builds correctly
- [ ] Color temperature supports narrative
- [ ] Contrast creates drama
- [ ] Overall cohesive look

## Tools Integration

### Optimization
```powershell
.\OptimizeLevelLighting.ps1 -LevelName "SM13" -BuildQuality "Production"
```

### Performance Analysis
```powershell
.\AnalyzeLevelPerformance.ps1 -Focus "Lighting" -GenerateReport
```

### Validation
```powershell
.\ValidateLevelLayout.ps1 -CheckLighting -CheckPerformance
```

## Documentation Template

### Per-Zone Lighting Documentation
```markdown
## Zone: [Name]

**Mood**: [Description]
**Average Lux**: [Range]
**Color Temperature**: [Kelvin]

### Light Inventory
- Static Lights: [Count]
- Stationary Lights: [Count]
- Dynamic Lights: [Count]
- Emissive Materials: [Count]

### Key Lights
1. [Name]: [Type], [Intensity], [Color], [Purpose]
2. ...

### Special Effects
- [Effect]: [Description], [Cost]

### Performance
- Draw Calls: [Count]
- GPU Time: [ms]
- Lightmap Memory: [MB]

### Notes
- [Design notes]
- [Known issues]
```

## Resources

- Level Design Principles: `LevelDesignPrinciples.md`
- SM13 Route Design: `SM13_RouteDesign.md`
- Performance Budget: `LevelDesignPrinciples.md#performance-budget`
- Optimization Tools: `Tools/OptimizeLevelLighting.ps1`
