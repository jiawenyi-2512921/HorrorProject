# Horror Project Development Tools

Complete suite of development tools and automation scripts for the Horror Project.

## Tools Overview

### Command Line Tools (CLI)

Located in `Tools/CLI/`:

#### horror-build.ps1
Build automation tool with multiple build types.

```powershell
# Quick incremental build
.\horror-build.ps1 quick

# Full clean build
.\horror-build.ps1 full -Config Shipping

# Clean build artifacts
.\horror-build.ps1 clean
```

#### horror-test.ps1
Comprehensive testing tool.

```powershell
# Run all tests
.\horror-test.ps1 all

# Run specific test types
.\horror-test.ps1 unit -Verbose
.\horror-test.ps1 integration -Filter "AI*"
.\horror-test.ps1 performance -GenerateReport
```

#### horror-package.ps1
Package creation tool for different configurations.

```powershell
# Package for release
.\horror-package.ps1 release

# Package for shipping with installer
.\horror-package.ps1 shipping -CreateInstaller

# Package for debug
.\horror-package.ps1 debug -OutputDir "C:\Builds"
```

#### horror-analyze.ps1
Project analysis and metrics tool.

```powershell
# Analyze code quality
.\horror-analyze.ps1 code -Detailed

# Analyze assets
.\horror-analyze.ps1 assets -ExportReport

# Analyze performance
.\horror-analyze.ps1 performance

# Analyze dependencies
.\horror-analyze.ps1 dependencies
```

#### horror-deploy.ps1
Deployment automation tool.

```powershell
# Deploy to staging
.\horror-deploy.ps1 staging

# Deploy to production
.\horror-deploy.ps1 production -Version "1.0.0"

# Dry run deployment
.\horror-deploy.ps1 test -DryRun
```

## Build Scripts

Located in `Scripts/Build/`:

- **QuickBuild.ps1** - Fast incremental build for development
- **FullBuild.ps1** - Complete clean build with project regeneration
- **IncrementalBuild.ps1** - Build only changed files

## Test Scripts

Located in `Scripts/Test/`:

- **RunAllTests.ps1** - Execute all test suites with reporting
- **RunUnitTests.ps1** - Run unit tests only
- **RunIntegrationTests.ps1** - Run integration tests

## Package Scripts

Located in `Scripts/Package/`:

- **PackageDebug.ps1** - Create debug build package
- **PackageRelease.ps1** - Create release build package
- **PackageShipping.ps1** - Create shipping build with optional installer

## Automation Scripts

Located in `Scripts/Automation/`:

- **DailyBuild.ps1** - Automated daily build pipeline
- **NightlyTest.ps1** - Automated nightly test suite
- **WeeklyReport.ps1** - Generate weekly development reports

## Editor Tools

Located in `Source/HorrorProjectEditor/`:

### Asset Validator
Validates all project assets for quality and compliance.
- Texture validation (size, format, mipmaps)
- Mesh validation (triangle count, LODs, collision)
- Material validation (shader complexity)
- Audio validation (sample rate, duration)
- Blueprint validation (compilation status)

### LOD Generator
Automatically generates LOD levels for static meshes.
- Configurable LOD count
- Automatic reduction settings
- Batch processing support

### Material Optimizer
Optimizes material complexity and performance.
- Remove unused nodes
- Simplify expressions
- Optimize texture sampling
- Material instance conversion

### Audio Batch Processor
Batch process audio files with consistent settings.
- Normalization
- Format conversion
- Compression settings
- Streaming configuration

### Editor Dashboard
Comprehensive development dashboard with:
- Real-time performance monitoring
- Asset browser
- Log viewer
- Test runner
- Build status panel

Access via: Tools > Horror Project Tools

## CI/CD Integration

Located in `.github/workflows/`:

### build.yml
Automated build pipeline triggered on push/PR.
- Multi-configuration builds (Development, Shipping)
- Artifact upload
- Build status reporting

### test.yml
Automated testing pipeline.
- Unit tests
- Integration tests
- Performance tests (scheduled)
- Test result artifacts

### package.yml
Automated packaging pipeline.
- Triggered on version tags
- Creates release packages
- GitHub release creation

### deploy.yml
Automated deployment pipeline.
- Environment-based deployment
- Health checks
- Team notifications

## Usage Examples

### Daily Development Workflow

```powershell
# Quick build and test
.\Tools\CLI\horror-build.ps1 quick
.\Tools\CLI\horror-test.ps1 unit

# Validate assets
.\Scripts\ValidateAssets.ps1

# Analyze code changes
.\Tools\CLI\horror-analyze.ps1 code
```

### Release Workflow

```powershell
# Full build
.\Scripts\Build\FullBuild.ps1 -Config Shipping

# Run all tests
.\Scripts\Test\RunAllTests.ps1

# Package for shipping
.\Tools\CLI\horror-package.ps1 shipping -CreateInstaller

# Deploy to staging
.\Tools\CLI\horror-deploy.ps1 staging
```

### Automated Workflows

```powershell
# Daily build automation
.\Scripts\Automation\DailyBuild.ps1

# Nightly test automation
.\Scripts\Automation\NightlyTest.ps1

# Weekly report generation
.\Scripts\Automation\WeeklyReport.ps1
```

## Configuration

### Build Configuration
Edit build scripts to customize:
- UE5 installation path
- Build configurations
- Platform targets
- Output directories

### Test Configuration
Configure test settings:
- Test filters
- Timeout values
- Report formats
- Automation settings

### CI/CD Configuration
Customize workflows:
- Trigger conditions
- Build matrices
- Artifact retention
- Notification settings

## Requirements

- Unreal Engine 5.6
- Visual Studio 2022
- PowerShell 7+
- Git
- GitHub Actions (for CI/CD)

## Best Practices

1. **Use Quick Build** for rapid iteration during development
2. **Run Unit Tests** before committing changes
3. **Use Full Build** before creating releases
4. **Generate Reports** regularly to track progress
5. **Validate Assets** before packaging
6. **Use CI/CD** for automated quality checks

## Troubleshooting

### Build Failures
- Check UE5 installation path
- Verify Visual Studio installation
- Clean intermediate files
- Regenerate project files

### Test Failures
- Check test logs in `Saved/Automation/`
- Run tests with `-Verbose` flag
- Verify test environment setup

### Package Failures
- Ensure successful build first
- Check disk space
- Verify package settings
- Review UAT logs

## Support

For issues or questions:
1. Check tool help: `.\horror-<tool>.ps1 -Help`
2. Review logs in `Logs/` directory
3. Check CI/CD workflow runs
4. Consult team documentation

---

**Horror Project Development Team**
