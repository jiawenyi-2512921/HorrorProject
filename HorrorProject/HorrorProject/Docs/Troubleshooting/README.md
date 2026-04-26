# Troubleshooting Guide

Solutions to common problems in HorrorProject development.

## Quick Links
- [FAQ](FAQ.md) - 50+ frequently asked questions
- [Compilation Errors](CompilationErrors.md) - Build and compile issues
- [Runtime Errors](RuntimeErrors.md) - Crashes and runtime problems
- [Performance Issues](PerformanceIssues.md) - Performance diagnostics
- [Asset Problems](AssetProblems.md) - Asset loading and management

## Common Issues by Category

### Build & Compilation
- [Cannot open include file](CompilationErrors.md#missing-includes)
- [Unresolved external symbol](CompilationErrors.md#linker-errors)
- [Syntax errors in generated code](CompilationErrors.md#generated-code-errors)
- [Out of memory during compilation](CompilationErrors.md#memory-errors)

### Editor & Runtime
- [Editor crashes on startup](RuntimeErrors.md#startup-crashes)
- [Null pointer exceptions](RuntimeErrors.md#null-pointers)
- [Blueprint compilation failed](RuntimeErrors.md#blueprint-errors)
- [Package game failed](RuntimeErrors.md#packaging-errors)

### Performance
- [Low frame rate](PerformanceIssues.md#low-fps)
- [Stuttering and hitches](PerformanceIssues.md#stuttering)
- [High memory usage](PerformanceIssues.md#memory-usage)
- [Long load times](PerformanceIssues.md#load-times)

### Assets
- [Failed to load asset](AssetProblems.md#loading-failures)
- [Missing textures](AssetProblems.md#missing-textures)
- [Audio not playing](AssetProblems.md#audio-issues)
- [Blueprint references broken](AssetProblems.md#broken-references)

## Getting Help

### Before Asking for Help
1. Check the [FAQ](FAQ.md)
2. Search existing [GitHub Issues](https://github.com/your-org/HorrorProject/issues)
3. Review relevant [API documentation](../API/README.md)
4. Check [system requirements](../README.md#requirements)

### Reporting Issues
When reporting issues, include:
- **Unreal Engine version**
- **Visual Studio version**
- **Operating system**
- **Error messages** (full text)
- **Steps to reproduce**
- **Log files** (Saved/Logs/)

### Community Support
- [Discord Server](https://discord.gg/horrorproject)
- [GitHub Discussions](https://github.com/your-org/HorrorProject/discussions)
- [Stack Overflow](https://stackoverflow.com/questions/tagged/horrorproject)

## Diagnostic Tools

### Log Files
Located in: `Saved/Logs/HorrorProject.log`

Enable verbose logging:
```cpp
LogHorrorProject: VeryVerbose
LogAudio: Verbose
LogEvidence: Verbose
```

### Console Commands
```
stat fps          - Show frame rate
stat unit         - Show frame time breakdown
stat memory       - Show memory usage
stat audio        - Show audio statistics
showdebug audio   - Audio debug overlay
```

### Visual Studio Debugging
See [Debugging Tutorial](../Tutorials/DebuggingTechniques.md)

### Unreal Insights
See [Performance Profiling](../Performance/ProfilingGuide.md)

## Emergency Fixes

### Nuclear Options (Use with Caution)

#### Clean Build
```bash
# Delete intermediate files
rm -rf Intermediate/ Binaries/ Saved/

# Regenerate project files
# Right-click .uproject → Generate Visual Studio project files

# Rebuild solution
```

#### Reset Editor Preferences
```bash
# Delete editor preferences
rm -rf Saved/Config/
```

#### Verify Engine Installation
```bash
# Epic Games Launcher → Library → UE 5.6 → Verify
```

## Related Documentation
- [Best Practices](../BestPractices/README.md)
- [Performance Guide](../Performance/OptimizationGuide.md)
- [Developer Guide](../Developer/README.md)
