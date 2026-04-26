# HorrorProject Documentation

Complete documentation for the HorrorProject horror game framework.

**Total Documents**: 149+ comprehensive guides, references, and tutorials

## 🚀 Quick Start

New to HorrorProject? Start here:
1. [Getting Started Guide](Tutorials/GettingStarted.md) - 30-minute setup
2. [Blueprint Quick Start](Blueprint/Blueprint_QuickStart.md) - Essential Blueprint skills
3. [FAQ](Troubleshooting/FAQ.md) - 50+ common questions answered

## 📚 Documentation Sections

### API Reference
Complete C++ API documentation with examples.
- [API Overview](API/README.md)
- [Audio System API](API/Audio/HorrorAudioSubsystem.md)
- [Evidence System API](API/Evidence/EvidenceCollectionComponent.md)
- [Doxygen Setup](API/DoxygenConfig.md)

### Tutorials
Step-by-step guides for all skill levels.
- [Tutorial Index](Tutorials/README.md)
- [Getting Started](Tutorials/GettingStarted.md) - Beginner
- [Audio System](Tutorials/AudioSystem.md) - Intermediate
- [Evidence Collection](Tutorials/EvidenceCollection.md) - Beginner
- [AI System](Tutorials/AISystem.md) - Advanced
- [Performance Optimization](Tutorials/PerformanceOptimization.md) - Advanced

### Troubleshooting
Solutions to common problems.
- [FAQ](Troubleshooting/FAQ.md) - 50+ questions
- [Compilation Errors](Troubleshooting/CompilationErrors.md)
- [Runtime Errors](Troubleshooting/RuntimeErrors.md)
- [Performance Issues](Troubleshooting/PerformanceIssues.md)
- [Asset Problems](Troubleshooting/AssetProblems.md)

### Best Practices
Industry-standard guidelines.
- [C++ Coding](BestPractices/CppCoding.md)
- [Blueprint Design](BestPractices/BlueprintDesign.md)
- [Performance Optimization](BestPractices/PerformanceOptimization.md)
- [Asset Management](BestPractices/AssetManagement.md)
- [Testing](BestPractices/Testing.md)

### For Developers
- [Getting Started](Developer/GettingStarted.md) - Set up your development environment
- [Coding Standards](Developer/CodingStandards.md) - Code style and conventions
- [Build Guide](Developer/BuildGuide.md) - Building and packaging
- [Testing Guide](Developer/TestingGuide.md) - Writing and running tests

### Architecture
- [System Overview](Architecture/SystemOverview.md) - High-level architecture
- [Event Bus System](Architecture/EventBus.md) - Central communication hub
- [Component Diagram](Architecture/ComponentDiagram.md) - System components
- [Data Flow](Architecture/DataFlow.md) - How data moves through the system
- [Dependencies](Architecture/Dependencies.md) - Module and external dependencies

### Technical Specifications
- [Performance Spec](Technical/PerformanceSpec.md) - Performance targets and optimization
- [Asset Spec](Technical/AssetSpec.md) - Asset creation guidelines

### Tools and Scripts
- [API Documentation Generator](../Scripts/Documentation/GenerateAPIDocs.ps1)
- [Diagram Generator](../Scripts/Documentation/GenerateDiagrams.ps1)
- [Documentation Validator](../Scripts/Documentation/ValidateDocs.ps1)
- [Documentation Publisher](../Scripts/Documentation/PublishDocs.ps1)

## Project Structure

```
HorrorProject/
├── Source/              # C++ source code
│   └── HorrorProject/
│       ├── AI/          # AI systems
│       ├── Audio/       # Audio systems
│       ├── Evidence/    # Evidence collection
│       ├── Game/        # Core game logic
│       ├── Interaction/ # Interaction system
│       ├── Performance/ # Performance monitoring
│       ├── Player/      # Player systems
│       ├── Save/        # Save system
│       ├── Tests/       # Unit tests
│       ├── UI/          # User interface
│       ├── VFX/         # Visual effects
│       └── Variant_Horror/ # Horror variants
├── Content/             # Game assets
├── Config/              # Configuration files
├── Docs/                # Documentation (you are here)
├── Scripts/             # Automation scripts
└── Plugins/             # Third-party plugins
```

## Core Systems

### Event Bus
The central nervous system of the game. All major systems communicate through the Event Bus using gameplay tags and structured messages.

**Key Features:**
- Publish-subscribe pattern
- Event history tracking
- Objective metadata support
- Blueprint and C++ APIs

[Learn more →](Architecture/EventBus.md)

### Encounter System
Manages horror encounters and threat reveals through a state machine.

**States:**
- Dormant → Primed → Revealed → Resolved

[Learn more →](Architecture/SystemOverview.md#director-pattern)

### Evidence System
Handles evidence collection, archiving, and persistence.

**Components:**
- Archive Subsystem
- Evidence Collection Component
- Evidence Actors

### Audio System
Spatial audio management with dynamic zones and MetaSounds integration.

**Features:**
- Audio zones
- Dynamic soundscapes
- Event-driven audio triggers

### Player Systems
Player character, controller, camera, and interaction systems.

**Components:**
- Player Character
- Player Controller
- Camera Manager
- Interaction Component

## Development Workflow

### Daily Workflow
1. Pull latest changes
2. Build project (Development Editor)
3. Run tests
4. Make changes
5. Test in editor
6. Commit and push

### Testing
```powershell
# Run all tests
.\Scripts\RunTests.ps1

# Run specific category
.\Scripts\RunTests.ps1 -Filter "HorrorProject.Game"
```

### Building
```powershell
# Build editor
.\Scripts\BuildProject.ps1 -Configuration Development

# Build shipping
.\Scripts\BuildProject.ps1 -Configuration Shipping
```

### Documentation
```powershell
# Generate API docs
.\Scripts\Documentation\GenerateAPIDocs.ps1

# Generate diagrams
.\Scripts\Documentation\GenerateDiagrams.ps1

# Validate documentation
.\Scripts\Documentation\ValidateDocs.ps1

# Publish documentation
.\Scripts\Documentation\PublishDocs.ps1 -Format HTML
```

## Performance Targets

### Frame Rate
- PC High-End: 60 FPS @ 1080p High
- PC Mid-Range: 60 FPS @ 1080p Medium
- PC Low-End: 30 FPS @ 1080p Low

### Memory
- System RAM: < 8GB
- Video RAM: < 4GB

### Loading
- Initial load: < 30 seconds
- Level transition: < 5 seconds

[Full specifications →](Technical/PerformanceSpec.md)

## Asset Guidelines

### 3D Models
- Static meshes: < 10,000 triangles
- Skeletal meshes: < 15,000 triangles
- LODs: 3-4 levels required

### Textures
- Max resolution: 2048x2048
- Format: BC7/BC5/BC1
- Mipmaps: Required

### Audio
- Format: Vorbis
- Sample rate: 44.1kHz
- Music streaming: Enabled

[Full specifications →](Technical/AssetSpec.md)

## Coding Standards

### Naming Conventions
- Classes: `UMyClass`, `AMyActor`, `FMyStruct`
- Variables: `MyVariable`, `bIsActive`
- Functions: `MyFunction()`, `GetValue()`
- Constants: `GDefaultValue`

### Code Style
- Tabs for indentation
- K&R brace style
- Max 120 characters per line
- Const correctness

[Full standards →](Developer/CodingStandards.md)

## Testing Strategy

### Test Types
- **Unit Tests** - Individual components
- **Integration Tests** - System interactions
- **Functional Tests** - Gameplay features
- **Performance Tests** - Performance validation

### Coverage Targets
- Core systems: > 80%
- Gameplay code: > 60%
- UI code: > 40%

[Testing guide →](Developer/TestingGuide.md)

## Contributing

### Before Committing
- [ ] Code follows standards
- [ ] Tests pass
- [ ] No compiler warnings
- [ ] Documentation updated
- [ ] Performance acceptable

### Code Review
- All changes require review
- Address all comments
- Ensure CI passes
- Update documentation

## Resources

### Documentation
- [Unreal Engine Docs](https://docs.unrealengine.com/)
- [C++ API Reference](https://docs.unrealengine.com/en-US/API/)

### Tools
- [Visual Studio 2022](https://visualstudio.microsoft.com/)
- [RenderDoc](https://renderdoc.org/)
- [Unreal Insights](https://docs.unrealengine.com/en-US/TestingAndOptimization/PerformanceAndProfiling/UnrealInsights/)

### Community
- Unreal Slackers Discord
- Unreal Engine Forums
- Stack Overflow

## Version History

### Current Version
- **Version:** 0.1.0 (Pre-Alpha)
- **Engine:** Unreal Engine 5.6
- **Last Updated:** 2026-04-26

### Recent Changes
- Initial project setup
- Core systems implementation
- Event Bus architecture
- Evidence collection system
- Encounter director system
- Complete documentation suite

## Support

For questions or issues:
1. Check this documentation
2. Search existing issues
3. Ask in team chat
4. Create new issue with details

---

**Last Updated:** 2026-04-26  
**Documentation Version:** 1.0  
**Project Version:** 0.1.0 Pre-Alpha
