# Horror Project Tools - Quick Reference

## Quick Commands

### Build
```powershell
# Quick build
.\Tools\CLI\horror-build.ps1 quick

# Full build
.\Tools\CLI\horror-build.ps1 full -Config Shipping

# Clean
.\Tools\CLI\horror-build.ps1 clean
```

### Test
```powershell
# All tests
.\Tools\CLI\horror-test.ps1 all

# Unit tests
.\Tools\CLI\horror-test.ps1 unit

# Integration tests
.\Tools\CLI\horror-test.ps1 integration
```

### Package
```powershell
# Release package
.\Tools\CLI\horror-package.ps1 release

# Shipping package
.\Tools\CLI\horror-package.ps1 shipping -CreateInstaller
```

### Analyze
```powershell
# Code analysis
.\Tools\CLI\horror-analyze.ps1 code

# Asset analysis
.\Tools\CLI\horror-analyze.ps1 assets
```

### Deploy
```powershell
# Deploy to staging
.\Tools\CLI\horror-deploy.ps1 staging

# Deploy to production
.\Tools\CLI\horror-deploy.ps1 production -Version "1.0.0"
```

## Editor Tools

Access via: **Tools > Horror Project Tools**

- Asset Validator
- LOD Generator
- Material Optimizer
- Audio Batch Processor
- Horror Editor Dashboard

## Automation

```powershell
# Daily build
.\Scripts\Automation\DailyBuild.ps1

# Nightly tests
.\Scripts\Automation\NightlyTest.ps1

# Weekly report
.\Scripts\Automation\WeeklyReport.ps1
```

## CI/CD Workflows

- **build.yml** - Automated builds on push/PR
- **test.yml** - Automated testing (daily)
- **package.yml** - Package on version tags
- **deploy.yml** - Deployment automation

## Help

Add `-Help` to any CLI tool for detailed usage:
```powershell
.\Tools\CLI\horror-build.ps1 -Help
```
