# Troubleshooting Guide

## Overview

This guide provides solutions to common issues encountered during development and debugging of HorrorProject.

## Performance Issues

### Low FPS

**Symptoms:**
- Frame rate below 30 FPS
- Stuttering or hitching
- Slow gameplay

**Diagnosis:**
```cpp
// Enable performance monitoring
UPerformanceDiagnostics* PerfDiag = NewObject<UPerformanceDiagnostics>();
PerfDiag->StartMonitoring(1.0f);

// Check stats
FPerformanceStats Stats = PerfDiag->GetCurrentStats();
UE_LOG(LogTemp, Log, TEXT("FPS: %.2f, Game Thread: %.2f ms, Render Thread: %.2f ms, GPU: %.2f ms"),
    Stats.CurrentFPS, Stats.AverageGameThreadTime, Stats.AverageRenderThreadTime, Stats.AverageGPUTime);
```

**Solutions:**

1. **CPU Bottleneck (High Game Thread Time)**
   - Reduce tick frequency on actors
   - Optimize Blueprint logic
   - Use object pooling
   - Reduce AI complexity
   - Profile with `stat game`

2. **Render Thread Bottleneck**
   - Reduce draw calls
   - Use LODs
   - Optimize materials
   - Profile with `stat scenerendering`

3. **GPU Bottleneck**
   - Reduce screen resolution
   - Lower post-processing quality
   - Optimize shaders
   - Profile with `profilegpu`

### Frame Hitches

**Symptoms:**
- Periodic frame drops
- Stuttering during gameplay

**Diagnosis:**
```
stat unit
```

**Solutions:**
- Enable texture streaming
- Reduce garbage collection frequency
- Preload assets
- Use async loading

### High Draw Calls

**Symptoms:**
- Many draw calls in stat rhi
- Render thread bottleneck

**Diagnosis:**
```
stat rhi
stat scenerendering
```

**Solutions:**
- Merge static meshes
- Use instanced static meshes
- Reduce material complexity
- Enable material merging

## Memory Issues

### Memory Leaks

**Symptoms:**
- Memory usage continuously increasing
- Eventual crash due to out of memory

**Diagnosis:**
```cpp
UMemoryDiagnostics* MemDiag = NewObject<UMemoryDiagnostics>();
MemDiag->StartMonitoring(5.0f);

// After some gameplay
MemDiag->DetectMemoryLeaks();
```

**Solutions:**

1. **Check for Circular References**
   - Use weak pointers where appropriate
   - Break circular dependencies
   - Review object ownership

2. **Verify Proper Cleanup**
   - Implement proper destructors
   - Clear arrays and maps
   - Unbind delegates

3. **Force Garbage Collection**
   ```cpp
   MemDiag->ForceGarbageCollection();
   ```

4. **Review Actor Spawning**
   - Ensure actors are properly destroyed
   - Use object pooling
   - Check for orphaned actors

### High Memory Usage

**Symptoms:**
- Memory usage above 80%
- Performance degradation

**Diagnosis:**
```cpp
MemDiag->DumpMemoryStats();
```

**Solutions:**
- Reduce texture sizes
- Enable texture streaming
- Optimize mesh LODs
- Reduce audio quality
- Clear unused assets

### Out of Memory Crashes

**Symptoms:**
- Crash with "Out of Memory" error
- System becomes unresponsive

**Solutions:**
- Increase virtual memory
- Reduce memory usage
- Enable 64-bit build
- Optimize asset sizes

## Crash Issues

### Null Pointer Crashes

**Symptoms:**
- Access violation error
- Crash in specific function

**Diagnosis:**
```powershell
.\CrashReportAnalyzer.ps1
```

**Solutions:**
1. Add null checks before dereferencing
   ```cpp
   if (MyObject != nullptr)
   {
       MyObject->DoSomething();
   }
   ```

2. Use safe accessors
   ```cpp
   if (IsValid(MyActor))
   {
       MyActor->Function();
   }
   ```

3. Initialize pointers
   ```cpp
   MyPointer = nullptr;
   ```

### Stack Overflow Crashes

**Symptoms:**
- Stack overflow error
- Crash in recursive function

**Solutions:**
- Check for infinite recursion
- Reduce recursion depth
- Use iteration instead of recursion
- Increase stack size (temporary)

### Assertion Failures

**Symptoms:**
- Assertion failed error
- Crash with check() or ensure()

**Solutions:**
- Review assertion condition
- Fix underlying logic error
- Add proper validation
- Check for edge cases

## Networking Issues

### High Ping

**Symptoms:**
- Ping above 150ms
- Laggy gameplay

**Diagnosis:**
```cpp
UNetworkDiagnostics* NetDiag = NewObject<UNetworkDiagnostics>();
NetDiag->TestConnection();
```

**Solutions:**
- Check network connection
- Reduce replication frequency
- Optimize replicated properties
- Use relevancy optimization

### Packet Loss

**Symptoms:**
- Packet loss above 5%
- Rubber-banding

**Solutions:**
- Check network stability
- Reduce bandwidth usage
- Implement client prediction
- Use reliable RPCs for critical data

### Replication Issues

**Symptoms:**
- Properties not replicating
- Desync between clients

**Solutions:**
- Verify UPROPERTY(Replicated)
- Implement GetLifetimeReplicatedProps
- Check network relevancy
- Use RepNotify for updates

## Gameplay Issues

### AI Not Working

**Symptoms:**
- AI not moving
- AI not responding to player

**Diagnosis:**
```
DebugShowAI true
DebugShowNavMesh true
```

**Solutions:**
- Verify NavMesh is built
- Check AI controller is assigned
- Review behavior tree logic
- Check perception configuration

### Physics Issues

**Symptoms:**
- Objects falling through floor
- Incorrect collision

**Diagnosis:**
```
DebugShowCollision true
```

**Solutions:**
- Check collision settings
- Verify collision channels
- Enable CCD for fast objects
- Review physics materials

### Audio Not Playing

**Symptoms:**
- Sounds not playing
- Audio cutting out

**Diagnosis:**
```
DebugShowAudio true
au.Debug.Sounds 1
```

**Solutions:**
- Check audio component settings
- Verify sound cue configuration
- Review attenuation settings
- Check audio volume settings

## Build Issues

### Compilation Errors

**Symptoms:**
- Build fails with errors
- Linker errors

**Solutions:**
1. Clean and rebuild
   ```
   Build -> Clean Solution
   Build -> Rebuild Solution
   ```

2. Delete intermediate files
   - Delete Binaries folder
   - Delete Intermediate folder
   - Delete Saved folder
   - Regenerate project files

3. Check for syntax errors
4. Verify includes
5. Check for circular dependencies

### Missing Modules

**Symptoms:**
- Module not found error
- Missing DLL

**Solutions:**
1. Add module to Build.cs
   ```cpp
   PublicDependencyModuleNames.AddRange(new string[] { 
       "Core", 
       "CoreUObject", 
       "Engine",
       "YourModule"
   });
   ```

2. Regenerate project files
3. Rebuild solution

## Editor Issues

### Editor Crashes

**Symptoms:**
- Editor crashes on startup
- Editor crashes during play

**Solutions:**
1. Delete Saved and Intermediate folders
2. Verify engine installation
3. Check for corrupt assets
4. Disable plugins
5. Run with -log flag for details

### Slow Editor

**Symptoms:**
- Editor is sluggish
- Long load times

**Solutions:**
- Reduce viewport quality
- Disable real-time rendering
- Close unnecessary windows
- Clear shader cache
- Optimize content browser

### Assets Not Loading

**Symptoms:**
- Assets show as missing
- Red question marks

**Solutions:**
- Verify asset paths
- Check for moved/renamed assets
- Fix up redirectors
- Reimport assets

## Debug Tool Issues

### Console Commands Not Working

**Symptoms:**
- Commands not recognized
- No effect when executed

**Solutions:**
1. Enable CheatManager
   ```cpp
   // In PlayerController
   CheatClass = UDebugConsoleCommands::StaticClass();
   ```

2. Check command spelling
3. Verify in correct game mode
4. Check logs for errors

### Debug HUD Not Showing

**Symptoms:**
- HUD not visible
- No debug information

**Solutions:**
1. Set HUD class in GameMode
   ```cpp
   HUDClass = ADebugHUD::StaticClass();
   ```

2. Toggle HUD visibility
   ```cpp
   DebugHUD->ToggleDebugHUD();
   ```

3. Check HUD is not hidden
4. Verify canvas is valid

### Diagnostics Not Running

**Symptoms:**
- No diagnostic reports
- Monitoring not working

**Solutions:**
1. Check subsystem initialization
   ```cpp
   UDiagnosticSubsystem* DiagSys = GetGameInstance()->GetSubsystem<UDiagnosticSubsystem>();
   if (DiagSys)
   {
       DiagSys->RunFullDiagnostics();
   }
   ```

2. Verify world context
3. Check timer manager
4. Review logs for errors

### Logs Not Being Written

**Symptoms:**
- Log file empty
- No log output

**Solutions:**
1. Check log file path
   ```cpp
   Logger->SetLogToFile(true, "D:/Logs/game.log");
   ```

2. Verify write permissions
3. Ensure directory exists
4. Check logging is enabled

## Common Error Messages

### "Assertion failed: IsValid()"

**Cause:** Attempting to use invalid object

**Solution:**
```cpp
if (IsValid(MyObject))
{
    MyObject->Function();
}
```

### "Pure virtual function called"

**Cause:** Calling pure virtual function on abstract class

**Solution:**
- Ensure proper object initialization
- Don't call virtual functions in constructor
- Implement all pure virtual functions

### "Access violation reading location"

**Cause:** Dereferencing null or invalid pointer

**Solution:**
```cpp
if (MyPointer != nullptr)
{
    MyPointer->DoSomething();
}
```

### "Stack overflow"

**Cause:** Infinite recursion or excessive stack allocation

**Solution:**
- Check for recursive function calls
- Reduce recursion depth
- Use heap allocation for large objects

### "Out of memory"

**Cause:** Memory exhaustion

**Solution:**
- Check for memory leaks
- Reduce memory usage
- Force garbage collection
- Increase available memory

## Diagnostic Workflow

### For Performance Issues

1. Enable performance HUD
   ```
   DebugShowPerformance true
   ```

2. Start performance monitoring
   ```cpp
   PerfDiag->StartMonitoring(1.0f);
   ```

3. Identify bottleneck
   ```
   stat unit
   stat gpu
   stat game
   ```

4. Profile specific area
   ```cpp
   PerfDiag->StartCPUProfiling();
   // ... test ...
   PerfDiag->StopCPUProfiling();
   ```

5. Generate report
   ```cpp
   PerfDiag->GeneratePerformanceReport();
   ```

### For Memory Issues

1. Start memory monitoring
   ```cpp
   MemDiag->StartMonitoring(5.0f);
   ```

2. Play game for extended period

3. Check for leaks
   ```cpp
   MemDiag->DetectMemoryLeaks();
   ```

4. Force garbage collection
   ```cpp
   MemDiag->ForceGarbageCollection();
   ```

5. Generate report
   ```cpp
   MemDiag->GenerateMemoryReport();
   ```

### For Crashes

1. Locate crash report
   ```
   Saved/Crashes/
   ```

2. Analyze crash
   ```powershell
   .\CrashReportAnalyzer.ps1
   ```

3. Review call stack

4. Check recent logs
   ```powershell
   .\LogViewer.ps1 -MinLevel ERROR -Tail 50
   ```

5. Reproduce with debug build

## Getting Help

### Information to Provide

When reporting issues, include:

1. **System Information**
   - OS version
   - Hardware specs
   - Engine version

2. **Crash Information**
   - Crash report
   - Call stack
   - Error message

3. **Reproduction Steps**
   - Exact steps to reproduce
   - Frequency of occurrence
   - Conditions when it happens

4. **Diagnostic Reports**
   - Performance report
   - Memory report
   - Log files

5. **Recent Changes**
   - Code changes
   - Asset changes
   - Configuration changes

### Useful Commands for Bug Reports

```cpp
// Dump game state
DebugDumpGameState

// Dump actors
DebugDumpActors

// Generate diagnostics
DiagSys->RunFullDiagnostics();
DiagSys->ExportDiagnosticsToFile("D:/BugReport/diagnostics.txt");

// Export logs
Logger->ExportLogsToFile("D:/BugReport/logs.txt");

// Generate performance report
PerfDiag->GeneratePerformanceReport("D:/BugReport/performance.txt");

// Generate memory report
MemDiag->GenerateMemoryReport("D:/BugReport/memory.txt");
```

## Prevention

### Best Practices

1. **Regular Testing**
   - Test frequently during development
   - Run diagnostics regularly
   - Monitor performance continuously

2. **Code Reviews**
   - Review for null checks
   - Check for memory leaks
   - Verify proper cleanup

3. **Automated Testing**
   - Unit tests
   - Integration tests
   - Performance benchmarks

4. **Monitoring**
   - Enable auto-diagnostics
   - Monitor logs
   - Track performance metrics

5. **Documentation**
   - Document known issues
   - Keep troubleshooting notes
   - Update this guide

## Additional Resources

- [Unreal Engine Documentation](https://docs.unrealengine.com)
- [Debugging Tools](https://docs.unrealengine.com/en-US/debugging-tools)
- [Performance Profiling](https://docs.unrealengine.com/en-US/performance-profiling)
- [Memory Profiling](https://docs.unrealengine.com/en-US/memory-profiling)
- [Network Profiling](https://docs.unrealengine.com/en-US/network-profiling)
