# Release Documentation

Documentation for version releases and updates.

## Release Process

### Pre-Release Checklist
- [ ] All tests passing
- [ ] Performance targets met
- [ ] Documentation updated
- [ ] Changelog generated
- [ ] Version numbers updated
- [ ] Build configurations verified
- [ ] Platform-specific testing complete
- [ ] Security review completed

### Release Types

#### Major Release (X.0.0)
- Breaking changes
- Major new features
- Architecture changes
- Full regression testing required

#### Minor Release (1.X.0)
- New features
- Non-breaking changes
- Enhanced functionality
- Partial regression testing

#### Patch Release (1.0.X)
- Bug fixes
- Performance improvements
- Security patches
- Targeted testing

## Version Numbering

Follow Semantic Versioning (SemVer):
```
MAJOR.MINOR.PATCH

1.0.0 - Initial release
1.1.0 - Added evidence system
1.1.1 - Fixed audio bug
2.0.0 - Breaking API changes
```

## Release Notes Template

### Version X.Y.Z - YYYY-MM-DD

#### 🎉 New Features
- **Evidence System**: Complete evidence collection and archive system
- **Audio Zones**: Dynamic audio based on player location
- **AI Improvements**: Enhanced threat AI with better pathfinding

#### 🔧 Improvements
- Improved performance in large levels (15% FPS increase)
- Enhanced Blueprint debugging tools
- Better error messages for common issues

#### 🐛 Bug Fixes
- Fixed crash when collecting evidence at max capacity
- Resolved audio zone transition stuttering
- Fixed AI getting stuck in doorways

#### ⚠️ Breaking Changes
- `CollectEvidence()` now returns bool instead of void
- `AudioZone` class renamed to `HorrorAudioZoneActor`
- Removed deprecated `OldEvidenceSystem` class

#### 📚 Documentation
- Added comprehensive API documentation
- New tutorials for evidence and audio systems
- Updated troubleshooting guide

#### 🔒 Security
- Fixed potential save data corruption
- Improved input validation
- Updated dependencies

#### 🎯 Known Issues
- Occasional audio pop when transitioning zones (#123)
- Blueprint hot reload may require editor restart (#124)

#### 📦 Assets
- Added 50+ new sound effects
- New evidence item models
- Updated UI textures

#### 🔄 Migration Guide
See [Migration Guide](MigrationGuide_v1_to_v2.md) for upgrading from v1.x

---

## Changelog Generation

### Automated Changelog

Use conventional commits for automatic changelog generation:

```bash
# Install changelog generator
npm install -g conventional-changelog-cli

# Generate changelog
conventional-changelog -p angular -i CHANGELOG.md -s
```

### Commit Message Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

**Types**:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation
- `style`: Code style changes
- `refactor`: Code refactoring
- `perf`: Performance improvement
- `test`: Testing
- `chore`: Maintenance

**Examples**:
```bash
feat(evidence): add archive viewer widget
fix(audio): resolve zone transition stuttering
docs(api): add evidence collection component documentation
perf(ai): optimize perception update frequency
```

## User Manual

### Table of Contents
1. [Introduction](#introduction)
2. [Installation](#installation)
3. [Getting Started](#getting-started)
4. [Core Systems](#core-systems)
5. [Advanced Features](#advanced-features)
6. [Troubleshooting](#troubleshooting)
7. [FAQ](#faq)

### Introduction
Brief overview of HorrorProject and its features.

### Installation
Step-by-step installation instructions.

### Getting Started
Quick start guide for new users.

### Core Systems
Detailed documentation for each system:
- Audio System
- Evidence System
- AI System
- Interaction System

### Advanced Features
Advanced topics and customization.

### Troubleshooting
Common issues and solutions.

### FAQ
Frequently asked questions.

## Release Artifacts

### Build Artifacts
```
HorrorProject_v1.0.0_Win64.zip
HorrorProject_v1.0.0_Win64_Debug.zip
HorrorProject_v1.0.0_Source.zip
```

### Documentation Artifacts
```
HorrorProject_v1.0.0_Documentation.zip
  - API/
  - Tutorials/
  - BestPractices/
  - Troubleshooting/
```

### Asset Packs
```
HorrorProject_v1.0.0_Assets.zip
  - Audio/
  - Models/
  - Textures/
  - Blueprints/
```

## Distribution

### GitHub Release
```bash
# Create release tag
git tag -a v1.0.0 -m "Release version 1.0.0"
git push origin v1.0.0

# Create GitHub release
gh release create v1.0.0 \
  --title "HorrorProject v1.0.0" \
  --notes-file RELEASE_NOTES.md \
  HorrorProject_v1.0.0_Win64.zip \
  HorrorProject_v1.0.0_Documentation.zip
```

### Epic Games Launcher
1. Package for distribution
2. Upload to Epic Games
3. Submit for review
4. Publish to marketplace

### Steam
1. Build depot
2. Upload to Steam
3. Set release date
4. Publish update

## Post-Release

### Monitoring
- Monitor crash reports
- Track performance metrics
- Review user feedback
- Monitor community channels

### Hotfix Process
1. Identify critical issue
2. Create hotfix branch
3. Implement fix
4. Test thoroughly
5. Release patch version
6. Update documentation

### Support
- Respond to bug reports
- Answer community questions
- Provide technical support
- Update FAQ based on feedback

## Release Schedule

### Regular Releases
- **Major**: Annually
- **Minor**: Quarterly
- **Patch**: As needed (typically monthly)

### Release Windows
- Avoid holidays
- Avoid major events
- Allow time for testing
- Coordinate with team

## Communication

### Announcement Channels
- GitHub Releases
- Discord Server
- Twitter/Social Media
- Email Newsletter
- Website Blog

### Announcement Template
```markdown
# HorrorProject v1.0.0 Released! 🎉

We're excited to announce the release of HorrorProject v1.0.0!

## Highlights
- Complete evidence collection system
- Dynamic audio zones
- Enhanced AI behavior

## Download
[Download v1.0.0](https://github.com/your-org/HorrorProject/releases/v1.0.0)

## Documentation
[View Documentation](https://docs.horrorproject.com)

## Feedback
We'd love to hear your feedback! Join us on [Discord](https://discord.gg/horrorproject)
```

## Related Documentation
- [Version History](VersionHistory.md)
- [Migration Guides](MigrationGuides/)
- [Changelog](../../CHANGELOG.md)
- [Contributing Guide](../Developer/ContributingGuide.md)
