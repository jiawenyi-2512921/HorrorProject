# HorrorProject Debugging Guide

## Overview

This guide covers debugging tools, techniques, and best practices for the HorrorProject.

## Debug Tools

### 1. Debug Console Commands

Access debug commands through the console (` key):

#### Visualization Commands
```
DebugShowCollision <true|false>     - Show collision shapes
DebugShowNavMesh <true|false>       - Show navigation mesh
DebugShowAI <true|false>            - Show AI debug info
DebugShowAudio <true|false>         - Show audio sources
DebugShowPerformance <true|false>   - Show performance stats
```

#### Performance Commands
```
DebugFPS <true|false>               - Toggle FPS display
DebugMemory                         - Show memory statistics
DebugGPU                            - Show GPU statistics
DebugDrawCalls                      - Show draw call statistics
```

#### Gameplay Commands
```
DebugGodMode <true|false>           - Toggle god mode
DebugNoClip <true|false>            - Toggle no-clip mode
DebugTeleport <X> <Y> <Z>           - Teleport to coordinates
DebugSpawnEnemy <Type>              - Spawn enemy at player location
DebugKillAllEnemies                 - Kill all enemies
```

#### Horror System Commands
```
DebugSetSanity <0-100>              - Set sanity level
DebugSetFear <0-100>                - Set fear level
DebugTriggerHallucination           - Trigger hallucination
DebugShowSanityInfo                 - Show sanity system info
```

#### Evidence Commands
```
DebugShowAllEvidence                - Show all evidence locations
DebugCollectAllEvidence             - Collect all evidence
DebugResetEvidence                  - Reset evidence collection
```

#### Save/Load Commands
```
DebugSaveGame <SlotName>            - Save game to slot
DebugLoadGame <SlotName>            - Load game from slot
DebugDeleteSave <SlotName>          - Delete save slot
```

#### Profiling Commands
```
DebugStartProfiling                 - Start CPU profiling
DebugStopProfiling                  - Stop CPU profiling
DebugCaptureFrame                   - Capture single frame profile
```

### 2. Debug HUD

Toggle debug HUD with `ToggleDebugHUD()` or through Blueprint.

#### HUD Categories
- **Performance**: FPS, frame time, world time
- **Memory**: Physical/virtual memory usage
- **Player**: Position, rotation, velocity
- **Gameplay**: Actor counts, enemy counts, sanity/fear
- **System**: Platform, build configuration, engine version
- **Frame Timing**: Game/render/GPU thread times
- **Network**: Ping, connection status

#### HUD Controls
```cpp
// Toggle entire HUD
DebugHUD->ToggleDebugHUD();

// Toggle specific categories
DebugHUD->SetDebugCategory("Performance", true);
DebugHUD->SetDebugCategory("Memory", false);
```

### 3. Debug Camera

Free-flying camera for level inspection.

#### Activation
```cpp
// C++
DebugCamera->ToggleDebugCamera();

// Console
ToggleDebugCamera
```

#### Controls
- **WASD**: Move horizontally
- **Q/E**: Move up/down
- **Mouse**: Look around
- **Shift**: Increase speed
- **Ctrl**: Decrease speed

### 4. Debug Visualization

Runtime visualization of game systems.

#### Available Visualizations
- Player info (position, velocity, direction)
- Enemy info (position, detection radius, state)
- Sanity info (sanity bar, fear level)
- Collision shapes
- Navigation mesh
- Audio sources
- Light sources

#### Usage
```cpp
// C++
DebugVisualization->TogglePlayerInfo();
DebugVisualization->ToggleEnemyInfo();
DebugVisualization->SetAllVisualization(true);

// Blueprint
Call "Toggle Player Info" node
```

## Diagnostic Systems

### 1. Diagnostic Subsystem

Centralized diagnostic system for monitoring game health.

#### Running Diagnostics
```cpp
// C++
UDiagnosticSubsystem* DiagSys = GetGameInstance()->GetSubsystem<UDiagnosticSubsystem>();
DiagSys->RunFullDiagnostics();

// Specific diagnostics
DiagSys->RunPerformanceDiagnostics();
DiagSys->RunMemoryDiagnostics();
DiagSys->RunNetworkDiagnostics();
DiagSys->RunGameplayDiagnostics();
```

#### Auto-Diagnostics
```cpp
// Enable automatic diagnostics every 60 seconds
DiagSys->SetAutoDiagnostics(true, 60.0f);
```

#### Exporting Reports
```cpp
DiagSys->ExportDiagnosticsToFile("D:/Reports/diagnostics.txt");
```

### 2. Performance Diagnostics

Monitor and analyze performance metrics.

#### Starting Monitoring
```cpp
UPerformanceDiagnostics* PerfDiag = NewObject<UPerformanceDiagnostics>();
PerfDiag->StartMonitoring(1.0f); // Sample every second
```

#### Getting Stats
```cpp
FPerformanceStats Stats = PerfDiag->GetCurrentStats();
UE_LOG(LogTemp, Log, TEXT("Average FPS: %.2f"), Stats.AverageFPS);
UE_LOG(LogTemp, Log, TEXT("Min FPS: %.2f"), Stats.MinFPS);
```

#### Profiling
```cpp
// Capture single frame
PerfDiag->CaptureFrameProfile();

// CPU profiling
PerfDiag->StartCPUProfiling();
// ... run code to profile ...
PerfDiag->StopCPUProfiling();
```

### 3. Memory Diagnostics

Detect memory leaks and monitor memory usage.

#### Starting Monitoring
```cpp
UMemoryDiagnostics* MemDiag = NewObject<UMemoryDiagnostics>();
MemDiag->StartMonitoring(5.0f); // Sample every 5 seconds
```

#### Leak Detection
```cpp
MemDiag->DetectMemoryLeaks();
```

#### Garbage Collection
```cpp
MemDiag->ForceGarbageCollection();
```

#### Memory Reports
```cpp
MemDiag->GenerateMemoryReport("D:/Reports/memory.txt");
MemDiag->DumpMemoryStats();
```

### 4. Network Diagnostics

Monitor network performance and connectivity.

#### Starting Monitoring
```cpp
UNetworkDiagnostics* NetDiag = NewObject<UNetworkDiagnostics>();
NetDiag->StartMonitoring(2.0f); // Sample every 2 seconds
```

#### Connection Testing
```cpp
NetDiag->TestConnection();
NetDiag->DumpNetworkStats();
```

## Structured Logging

Enhanced logging system with metadata and filtering.

### Basic Logging
```cpp
UStructuredLogging* Logger = GetGameInstance()->GetSubsystem<UStructuredLogging>();

// Different log levels
Logger->LogTrace("Category", "Trace message");
Logger->LogDebug("Category", "Debug message");
Logger->LogInfo("Category", "Info message");
Logger->LogWarning("Category", "Warning message");
Logger->LogError("Category", "Error message");
Logger->LogFatal("Category", "Fatal message");
```

### Logging with Metadata
```cpp
TMap<FString, FString> Metadata;
Metadata.Add("PlayerID", "12345");
Metadata.Add("Location", "Level1");

Logger->LogInfo("Gameplay", "Player spawned", Metadata);
```

### Filtering
```cpp
// Set minimum log level
Logger->SetMinimumLogLevel(ELogLevel::Warning);

// Category-specific filtering
Logger->AddCategoryFilter("Performance", ELogLevel::Debug);
Logger->AddCategoryFilter("Network", ELogLevel::Info);
```

### Querying Logs
```cpp
// Get all errors
TArray<FStructuredLogEntry> Errors = Logger->GetRecentErrors(10);

// Get logs by level and category
TArray<FStructuredLogEntry> Logs = Logger->GetLogEntries(ELogLevel::Warning, "Gameplay");
```

### Exporting Logs
```cpp
Logger->ExportLogsToFile("D:/Logs/export.txt");
Logger->ExportLogsToJSON("D:/Logs/export.json");
```

## Log Analysis Tools

### Log Analyzer (PowerShell)

Analyze structured logs and generate reports.

```powershell
# Analyze all logs
.\LogAnalyzer.ps1

# Analyze specific type
.\LogAnalyzer.ps1 -AnalysisType Errors
.\LogAnalyzer.ps1 -AnalysisType Performance

# Custom paths
.\LogAnalyzer.ps1 -LogPath "D:/Logs/game.log" -OutputPath "D:/Reports/analysis.txt"
```

### Log Viewer (PowerShell)

Interactive log viewer with filtering.

```powershell
# View all logs
.\LogViewer.ps1

# Filter by level
.\LogViewer.ps1 -MinLevel ERROR

# Filter by category
.\LogViewer.ps1 -Category "Performance"

# Search logs
.\LogViewer.ps1 -Search "memory leak"

# Tail logs
.\LogViewer.ps1 -Tail 50

# Follow logs (like tail -f)
.\LogViewer.ps1 -Follow
```

### Crash Report Analyzer (PowerShell)

Analyze crash reports and generate diagnostics.

```powershell
# Analyze most recent crash
.\CrashReportAnalyzer.ps1

# Analyze specific crash
.\CrashReportAnalyzer.ps1 -CrashReportPath "D:/Saved/Crashes/CrashReport-123"

# Detailed analysis
.\CrashReportAnalyzer.ps1 -Detailed
```

## Common Debugging Scenarios

### Performance Issues

1. Enable performance HUD
2. Start performance monitoring
3. Identify bottleneck (CPU/GPU/Memory)
4. Use profiling tools
5. Generate performance report

```cpp
// Quick performance check
DebugHUD->SetDebugCategory("Performance", true);
DebugHUD->SetDebugCategory("FrameTiming", true);

// Detailed profiling
PerfDiag->StartMonitoring(0.5f);
// ... play game for a while ...
PerfDiag->GeneratePerformanceReport();
```

### Memory Leaks

1. Start memory monitoring
2. Play game for extended period
3. Run leak detection
4. Force garbage collection
5. Generate memory report

```cpp
MemDiag->StartMonitoring(5.0f);
// ... play game ...
MemDiag->DetectMemoryLeaks();
MemDiag->ForceGarbageCollection();
MemDiag->GenerateMemoryReport();
```

### Crash Investigation

1. Locate crash report in Saved/Crashes
2. Run crash analyzer
3. Review call stack
4. Check recent logs
5. Reproduce with debug build

```powershell
.\CrashReportAnalyzer.ps1 -Detailed
```

### Gameplay Issues

1. Enable debug visualization
2. Use debug commands to manipulate state
3. Check structured logs
4. Run gameplay diagnostics

```cpp
// Visualize game state
DebugVisualization->SetAllVisualization(true);

// Check diagnostics
DiagSys->RunGameplayDiagnostics();

// Review logs
TArray<FStructuredLogEntry> GameplayLogs = Logger->GetLogEntries(ELogLevel::Warning, "Gameplay");
```

## Best Practices

### During Development

1. Use structured logging extensively
2. Enable auto-diagnostics
3. Monitor performance regularly
4. Check for memory leaks
5. Use debug visualization

### Before Release

1. Run full diagnostics
2. Analyze all logs for errors/warnings
3. Profile performance
4. Test with shipping build
5. Verify all debug code is disabled

### When Debugging

1. Start with high-level diagnostics
2. Use visualization to understand state
3. Enable relevant logging categories
4. Use profiling for performance issues
5. Check logs before and after crash

## Troubleshooting

### Debug HUD Not Showing

- Verify HUD class is set in GameMode
- Check `bShowDebugHUD` is true
- Ensure HUD is not hidden

### Console Commands Not Working

- Verify CheatManager is enabled
- Check command spelling
- Ensure in correct game mode

### Logs Not Being Written

- Check log file path exists
- Verify write permissions
- Ensure logging is enabled

### Performance Monitoring Not Starting

- Check timer manager is valid
- Verify world context
- Ensure monitoring interval is valid

## Additional Resources

- Unreal Engine Documentation: https://docs.unrealengine.com
- Debugging Tools: https://docs.unrealengine.com/en-US/debugging-tools
- Performance Profiling: https://docs.unrealengine.com/en-US/performance-profiling
