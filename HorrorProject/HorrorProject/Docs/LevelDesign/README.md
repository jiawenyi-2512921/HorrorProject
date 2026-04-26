# Level Design Toolkit

Comprehensive level design system for horror game development based on 7 beats structure with 8 objective nodes.

## Overview

This toolkit provides complete documentation, tools, and templates for designing, building, testing, and optimizing horror game levels. Specifically designed for SM13 (1300m² level) but adaptable to any level size.

## Documentation

### Design Guides

1. **[LevelDesignPrinciples.md](LevelDesignPrinciples.md)**
   - 7 beats structure (35-minute gameplay)
   - 8 objective node system
   - Spatial design principles
   - Performance budgets
   - Design checklist

2. **[SM13_RouteDesign.md](SM13_RouteDesign.md)**
   - Detailed 1300m² layout
   - Zone-by-zone breakdown
   - Navigation flow
   - Timing analysis
   - Sightlines and vistas

3. **[ObjectivePlacement.md](ObjectivePlacement.md)**
   - 8 objective specifications
   - Placement principles
   - Visibility tiers
   - Blueprint implementation
   - Testing guidelines

4. **[EvidencePlacement.md](EvidencePlacement.md)**
   - 28 evidence items (8 required, 16 optional, 4 secret)
   - Narrative flow
   - Discovery rates
   - Interaction design
   - Collection tracking

5. **[LightingDesign.md](LightingDesign.md)**
   - Zone-based lighting
   - Performance budgets
   - Color theory for horror
   - Lightmass settings
   - Optimization techniques

6. **[AudioDesign.md](AudioDesign.md)**
   - Zone-based audio
   - Layered ambience
   - Music system
   - Performance budgets
   - Mixing guidelines

## Tools

### Construction Tools

#### `GenerateLevelBlockout.ps1`
Generates level geometry and structure.

```powershell
.\Tools\GenerateLevelBlockout.ps1 -LevelName "SM13" -TotalArea 1300 -GenerateReport
```

**Output:**
- JSON blockout data
- Unreal Engine import script
- Zone specifications
- Navigation bounds

#### `PlaceObjectiveNodes.ps1`
Places 8 objective nodes based on design.

```powershell
.\Tools\PlaceObjectiveNodes.ps1 -LevelName "SM13" -ObjectiveCount 8 -GenerateReport
```

**Output:**
- Objective placement data
- UE import script
- Spacing analysis
- Timing breakdown

#### `PlaceEvidenceItems.ps1`
Places 28 evidence items across zones.

```powershell
.\Tools\PlaceEvidenceItems.ps1 -LevelName "SM13" -RequiredCount 8 -OptionalCount 16 -SecretCount 4
```

**Output:**
- Evidence placement data
- Zone distribution
- Category breakdown

### Testing Tools

#### `ValidateLevelLayout.ps1`
Validates level against design specifications.

```powershell
.\Tools\ValidateLevelLayout.ps1 -LevelName "SM13" -CheckObjectives -CheckEvidence -CheckSpacing -CheckTiming -GenerateReport
```

**Checks:**
- Objective count and spacing
- Evidence distribution
- Timing targets
- Performance budget

#### `TestLevelNavigation.ps1`
Tests player navigation and routing.

```powershell
.\Tools\TestLevelNavigation.ps1 -LevelName "SM13" -GenerateReport
.\Tools\TestLevelNavigation.ps1 -LevelName "SM13" -SpeedrunMode -GenerateReport
```

**Output:**
- Total distance traveled
- Estimated completion time
- Route analysis

#### `TestObjectiveFlow.ps1`
Tests objective progression and pacing.

```powershell
.\Tools\TestObjectiveFlow.ps1 -LevelName "SM13" -RunFullTest -GenerateReport
```

**Tests:**
- Beat progression
- Timing progression
- Difficulty curve

#### `TestPerformance.ps1`
Tests level performance metrics.

```powershell
.\Tools\TestPerformance.ps1 -LevelName "SM13" -GenerateReport
```

**Metrics:**
- Draw calls, triangles, memory
- Lighting performance
- Audio performance
- Frame time breakdown

### Optimization Tools

#### `OptimizeLevelLighting.ps1`
Optimizes lighting setup.

```powershell
.\Tools\OptimizeLevelLighting.ps1 -LevelName "SM13" -BuildQuality "Production"
```

**Quality Levels:**
- Preview (2-5 min build)
- Medium (10-20 min build)
- High (30-60 min build)
- Production (1-3 hour build)

#### `OptimizeLevelGeometry.ps1`
Provides geometry optimization recommendations.

```powershell
.\Tools\OptimizeLevelGeometry.ps1 -LevelName "SM13"
```

**Recommendations:**
- LOD configuration
- Mesh instancing
- Occlusion culling
- Material optimization

#### `OptimizeLevelOcclusion.ps1`
Optimizes occlusion culling.

```powershell
.\Tools\OptimizeLevelOcclusion.ps1 -LevelName "SM13"
```

**Features:**
- Precomputed visibility
- Cull distance volumes
- HLOD settings
- Occlusion volumes

#### `AnalyzeLevelPerformance.ps1`
Comprehensive performance analysis.

```powershell
.\Tools\AnalyzeLevelPerformance.ps1 -LevelName "SM13" -Focus "All"
.\Tools\AnalyzeLevelPerformance.ps1 -LevelName "SM13" -Focus "Rendering"
```

**Focus Areas:**
- All (complete analysis)
- Rendering
- Lighting
- Audio
- Gameplay

## Templates

### `LevelDesignDocument.md`
Complete level design document template.

**Sections:**
- Project information
- Design goals
- Level specifications
- Zone breakdown
- Objectives and evidence
- Lighting and audio
- Performance optimization
- Testing plan

### `LevelTestPlan.md`
Comprehensive testing checklist.

**Test Categories:**
- Functionality tests
- Performance tests
- Gameplay experience tests
- Audio/visual tests
- Bug tracking

### `LevelOptimizationPlan.md`
Detailed optimization workflow.**Optimization Areas:**
- Rendering optimization
- Lighting optimization
- Audio optimization
- Gameplay optimization
- Testing schedule

## Quick Start Guide

### 1. Design Phase

```powershell
# Generate level blockout
.\Tools\GenerateLevelBlockout.ps1 -LevelName "SM13" -GenerateReport

# Place objectives
.\Tools\PlaceObjectiveNodes.ps1 -LevelName "SM13" -GenerateReport

# Place evidence
.\Tools\PlaceEvidenceItems.ps1 -LevelName "SM13"
```

### 2. Implementation Phase

1. Import blockout into Unreal Engine
2. Configure objective blueprints
3. Place evidence items
4. Implement lighting (see LightingDesign.md)
5. Implement audio (see AudioDesign.md)

### 3. Testing Phase

```powershell
# Validate layout
.\Tools\ValidateLevelLayout.ps1 -LevelName "SM13" -CheckObjectives -CheckEvidence -CheckSpacing -CheckTiming -GenerateReport

# Test navigation
.\Tools\TestLevelNavigation.ps1 -LevelName "SM13" -GenerateReport

# Test objective flow
.\Tools\TestObjectiveFlow.ps1 -LevelName "SM13" -RunFullTest -GenerateReport

# Test performance
.\Tools\TestPerformance.ps1 -LevelName "SM13" -GenerateReport
```

### 4. Optimization Phase

```powershell
# Analyze performance
.\Tools\AnalyzeLevelPerformance.ps1 -LevelName "SM13" -Focus "All"

# Optimize lighting
.\Tools\OptimizeLevelLighting.ps1 -LevelName "SM13" -BuildQuality "Production"

# Optimize geometry
.\Tools\OptimizeLevelGeometry.ps1 -LevelName "SM13"

# Optimize occlusion
.\Tools\OptimizeLevelOcclusion.ps1 -LevelName "SM13"
```

## Performance Budgets

### SM13 (1300m²) Targets

**Rendering:**
- Draw Calls: < 2000 (max 2500)
- Triangles: < 2M (max 3M)
- Texture Memory: < 800MB (max 1GB)

**Lighting:**
- Static Lights: Unlimited
- Stationary Lights: < 15
- Dynamic Lights: < 8
- Shadow Casting: < 5

**Audio:**
- Simultaneous Sounds: < 32
- 3D Sounds: < 16
- Audio Memory: < 200MB

**Frame Time (60 FPS = 16.67ms):**
- Game Thread: < 10ms
- Render Thread: < 12ms
- GPU: < 14ms

## Design Principles

### 7 Beats Structure

1. **Arrival** (0-5 min): Establish atmosphere
2. **Exploration** (5-10 min): Build familiarity
3. **First Threat** (10-15 min): Introduce danger
4. **Rising Tension** (15-22 min): Escalate pressure
5. **Crisis Point** (22-28 min): Peak tension
6. **Resolution Path** (28-33 min): Provide escape
7. **Escape** (33-35 min): Release tension

### 8 Objective System

1. Tutorial/Introduction (Easy)
2. Exploration Reward (Easy)
3. First Challenge (Medium)
4. Resource Gate (Medium, Optional)
5. Navigation Challenge (Medium-Hard)
6. Crisis Objective (Hard)
7. Pre-Exit Challenge (Medium, Optional)
8. Exit Unlock (Easy)

## File Structure

```
LevelDesign/
├── README.md (this file)
├── LevelDesignPrinciples.md
├── SM13_RouteDesign.md
├── ObjectivePlacement.md
├── EvidencePlacement.md
├── LightingDesign.md
├── AudioDesign.md
├── Tools/
│   ├── GenerateLevelBlockout.ps1
│   ├── PlaceObjectiveNodes.ps1
│   ├── PlaceEvidenceItems.ps1
│   ├── ValidateLevelLayout.ps1
│   ├── TestLevelNavigation.ps1
│   ├── TestObjectiveFlow.ps1
│   ├── TestPerformance.ps1
│   ├── OptimizeLevelLighting.ps1
│   ├── OptimizeLevelGeometry.ps1
│   ├── OptimizeLevelOcclusion.ps1
│   └── AnalyzeLevelPerformance.ps1
└── Templates/
    ├── LevelDesignDocument.md
    ├── LevelTestPlan.md
    └── LevelOptimizationPlan.md
```

## Best Practices

### Design
1. Follow 7 beats structure strictly
2. Distribute objectives across entire level
3. Balance required vs. optional content
4. Test navigation early and often
5. Respect performance budgets

### Implementation
1. Start with blockout, iterate quickly
2. Use modular assets for flexibility
3. Profile performance at each milestone
4. Document all changes
5. Playtest frequently

### Optimization
1. Optimize early, optimize often
2. Use static lighting for 90% of lights
3. Implement LODs for all meshes
4. Enable occlusion culling
5. Stream large assets

### Testing
1. Test functionality before polish
2. Validate against design document
3. Performance test on target hardware
4. Gather player feedback
5. Fix critical bugs before polish

## Common Issues

### Navigation
- **Issue**: Players get lost
- **Solution**: Add environmental cues, improve lighting

### Performance
- **Issue**: Frame rate drops
- **Solution**: Profile, optimize hotspots, reduce complexity

### Pacing
- **Issue**: Tension doesn't build
- **Solution**: Review 7 beats structure, adjust timing

### Objectives
- **Issue**: Objectives unclear
- **Solution**: Add UI prompts, environmental hints

## Resources

### External References
- Unreal Engine Documentation
- Horror Game Design Principles
- Level Design Patterns
- Performance Optimization Guides

### Internal References
- Project technical specifications
- Art style guide
- Audio design document
- Gameplay systems documentation

## Support

For questions or issues:
1. Review relevant documentation
2. Check common issues section
3. Run validation tools
4. Consult with team leads

## Version History

### v1.0 (2026-04-26)
- Initial toolkit creation
- Complete documentation
- All tools implemented
- Templates provided

## License

Internal use only - HorrorProject

---

**Ready to build amazing horror levels!**
