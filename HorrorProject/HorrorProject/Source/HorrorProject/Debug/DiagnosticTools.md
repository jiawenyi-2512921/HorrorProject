# Diagnostic Tools Usage Guide

## Overview

This guide covers the usage of diagnostic tools in HorrorProject for monitoring, analyzing, and troubleshooting game systems.

## Diagnostic Subsystem

The central diagnostic system for monitoring overall game health.

### Initialization

The diagnostic subsystem is automatically initialized with the game instance.

```cpp
UDiagnosticSubsystem* DiagSys = GetGameInstance()->GetSubsystem<UDiagnosticSubsystem>();
```

### Running Diagnostics

#### Full Diagnostics
Runs all diagnostic checks across all systems.

```cpp
DiagSys->RunFullDiagnostics();
```

This will check:
- Performance metrics
- Memory usage
- Network status
- Gameplay systems

#### Specific Diagnostics

Run diagnostics for specific systems:

```cpp
// Performance only
DiagSys->RunPerformanceDiagnostics();

// Memory only
DiagSys->RunMemoryDiagnostics();

// Network only
DiagSys->RunNetworkDiagnostics();

// Gameplay only
DiagSys->RunGameplayDiagnostics();
```

### Automatic Diagnostics

Enable automatic periodic diagnostics:

```cpp
// Run diagnostics every 60 seconds
DiagSys->SetAutoDiagnostics(true, 60.0f);

// Disable automatic diagnostics
DiagSys->SetAutoDiagnostics(false);
```

### Viewing Results

```cpp
// Get all diagnostic reports
TArray<FDiagnosticReport> Reports = DiagSys->GetDiagnosticReports();

// Iterate through reports
for (const FDiagnosticReport& Report : Reports)
{
    UE_LOG(LogTemp, Log, TEXT("[%s] %s: %s"),
        *Report.Category,
        *Report.Timestamp.ToString(),
        *Report.Message);
}
```

### Exporting Reports

```cpp
// Export to default location
DiagSys->ExportDiagnosticsToFile();

// Export to specific location
DiagSys->ExportDiagnosticsToFile("D:/Reports/diagnostics.txt");
```

### Clearing Reports

```cpp
DiagSys->ClearDiagnosticReports();
```

## Performance Diagnostics

Monitor and analyze performance metrics over time.

### Setup

```cpp
UPerformanceDiagnostics* PerfDiag = NewObject<UPerformanceDiagnostics>();
```

### Starting Monitoring

```cpp
// Monitor every second
PerfDiag->StartMonitoring(1.0f);

// Monitor every 0.5 seconds for more granular data
PerfDiag->StartMonitoring(0.5f);
```

### Getting Current Stats

```cpp
FPerformanceStats Stats = PerfDiag->GetCurrentStats();

UE_LOG(LogTemp, Log, TEXT("Average FPS: %.2f"), Stats.AverageFPS);
UE_LOG(LogTemp, Log, TEXT("Min FPS: %.2f"), Stats.MinFPS);
UE_LOG(LogTemp, Log, TEXT("Max FPS: %.2f"), Stats.MaxFPS);
UE_LOG(LogTemp, Log, TEXT("Current FPS: %.2f"), Stats.CurrentFPS);
UE_LOG(LogTemp, Log, TEXT("Frame Time: %.2f ms"), Stats.CurrentFrameTime);
UE_LOG(LogTemp, Log, TEXT("Game Thread: %.2f ms"), Stats.AverageGameThreadTime);
UE_LOG(LogTemp, Log, TEXT("Render Thread: %.2f ms"), Stats.AverageRenderThreadTime);
UE_LOG(LogTemp, Log, TEXT("GPU Time: %.2f ms"), Stats.AverageGPUTime);
```

### Accessing History

```cpp
TArray<FPerformanceSnapshot> History = PerfDiag->GetPerformanceHistory();

for (const FPerformanceSnapshot& Snapshot : History)
{
    UE_LOG(LogTemp, Log, TEXT("[%s] FPS: %.1f, Frame: %.2f ms"),
        *Snapshot.Timestamp.ToString(),
        Snapshot.FPS,
        Snapshot.DeltaTime * 1000.0f);
}
```

### Generating Reports

```cpp
// Generate report to default location
PerfDiag->GeneratePerformanceReport();

// Generate report to specific location
PerfDiag->GeneratePerformanceReport("D:/Reports/performance.txt");
```

### Profiling

```cpp
// Capture single frame profile
PerfDiag->CaptureFrameProfile();

// Start CPU profiling
PerfDiag->StartCPUProfiling();
// ... run code to profile ...
PerfDiag->StopCPUProfiling();
```

### Stopping Monitoring

```cpp
PerfDiag->StopMonitoring();
```

### Clearing History

```cpp
PerfDiag->ClearHistory();
```

## Memory Diagnostics

Detect memory leaks and monitor memory usage patterns.

### Setup

```cpp
UMemoryDiagnostics* MemDiag = NewObject<UMemoryDiagnostics>();
```

### Starting Monitoring

```cpp
// Monitor every 5 seconds
MemDiag->StartMonitoring(5.0f);
```

### Getting Current Stats

```cpp
FMemoryStats Stats = MemDiag->GetCurrentStats();

UE_LOG(LogTemp, Log, TEXT("Current Memory: %.2f MB"), Stats.CurrentUsedMB);
UE_LOG(LogTemp, Log, TEXT("Peak Memory: %.2f MB"), Stats.PeakUsedMB);
UE_LOG(LogTemp, Log, TEXT("Average Memory: %.2f MB"), Stats.AverageUsedMB);
UE_LOG(LogTemp, Log, TEXT("Available: %.2f MB"), Stats.AvailableMB);
UE_LOG(LogTemp, Log, TEXT("Usage: %.1f%%"), Stats.UsagePercent);
```

### Taking Snapshots

```cpp
// Take immediate snapshot
MemDiag->TakeMemorySnapshot();
```

### Detecting Memory Leaks

```cpp
// Run leak detection
MemDiag->DetectMemoryLeaks();
```

This will analyze memory trends and report potential leaks.

### Forcing Garbage Collection

```cpp
MemDiag->ForceGarbageCollection();
```

### Dumping Memory Stats

```cpp
// Dump detailed memory statistics to log
MemDiag->DumpMemoryStats();
```

### Generating Reports

```cpp
// Generate report to default location
MemDiag->GenerateMemoryReport();

// Generate report to specific location
MemDiag->GenerateMemoryReport("D:/Reports/memory.txt");
```

### Stopping Monitoring

```cpp
MemDiag->StopMonitoring();
```

## Network Diagnostics

Monitor network performance and connectivity.

### Setup

```cpp
UNetworkDiagnostics* NetDiag = NewObject<UNetworkDiagnostics>();
```

### Checking Network Status

```cpp
if (NetDiag->IsNetworked())
{
    UE_LOG(LogTemp, Log, TEXT("Game is networked"));
}
else
{
    UE_LOG(LogTemp, Log, TEXT("Game is standalone"));
}
```

### Starting Monitoring

```cpp
// Monitor every 2 seconds
NetDiag->StartMonitoring(2.0f);
```

### Getting Current Stats

```cpp
FNetworkStats Stats = NetDiag->GetCurrentStats();

UE_LOG(LogTemp, Log, TEXT("Average Ping: %.2f ms"), Stats.AveragePing);
UE_LOG(LogTemp, Log, TEXT("Min Ping: %.2f ms"), Stats.MinPing);
UE_LOG(LogTemp, Log, TEXT("Max Ping: %.2f ms"), Stats.MaxPing);
UE_LOG(LogTemp, Log, TEXT("Packet Loss: %.2f%%"), Stats.AveragePacketLoss);
UE_LOG(LogTemp, Log, TEXT("Bytes Sent: %d"), Stats.TotalBytesSent);
UE_LOG(LogTemp, Log, TEXT("Bytes Received: %d"), Stats.TotalBytesReceived);
```

### Testing Connection

```cpp
NetDiag->TestConnection();
```

### Dumping Network Stats

```cpp
NetDiag->DumpNetworkStats();
```

### Generating Reports

```cpp
// Generate report to default location
NetDiag->GenerateNetworkReport();

// Generate report to specific location
NetDiag->GenerateNetworkReport("D:/Reports/network.txt");
```

### Stopping Monitoring

```cpp
NetDiag->StopMonitoring();
```

## Blueprint Usage

All diagnostic tools are accessible from Blueprint.

### Diagnostic Subsystem

```
Get Game Instance -> Get Subsystem (Diagnostic Subsystem) -> Run Full Diagnostics
```

### Performance Diagnostics

```
Construct Object from Class (Performance Diagnostics)
-> Start Monitoring (Interval: 1.0)
-> Get Current Stats
-> Generate Performance Report
```

### Memory Diagnostics

```
Construct Object from Class (Memory Diagnostics)
-> Start Monitoring (Interval: 5.0)
-> Detect Memory Leaks
-> Generate Memory Report
```

### Network Diagnostics

```
Construct Object from Class (Network Diagnostics)
-> Is Networked
-> Start Monitoring (Interval: 2.0)
-> Test Connection
```

## Best Practices

### During Development

1. **Enable Auto-Diagnostics**
   ```cpp
   DiagSys->SetAutoDiagnostics(true, 60.0f);
   ```

2. **Monitor Performance Continuously**
   ```cpp
   PerfDiag->StartMonitoring(1.0f);
   ```

3. **Check for Memory Leaks Regularly**
   ```cpp
   MemDiag->StartMonitoring(5.0f);
   MemDiag->DetectMemoryLeaks();
   ```

### Before Playtesting

1. Run full diagnostics
2. Generate all reports
3. Review for warnings/errors
4. Fix critical issues

### After Crashes

1. Run diagnostics immediately
2. Check memory usage
3. Review performance history
4. Generate reports for analysis

### Performance Optimization

1. Start performance monitoring
2. Identify bottlenecks
3. Profile specific areas
4. Compare before/after metrics

### Memory Optimization

1. Take baseline snapshot
2. Run gameplay session
3. Take final snapshot
4. Detect leaks
5. Force GC and compare

## Interpreting Results

### Performance Thresholds

- **FPS < 30**: Critical performance issue
- **FPS 30-60**: Acceptable but could be improved
- **FPS > 60**: Good performance

- **Frame Time > 33ms**: Below 30 FPS
- **Frame Time > 16ms**: Below 60 FPS

- **Game Thread > 16ms**: CPU bottleneck
- **Render Thread > 16ms**: Rendering bottleneck
- **GPU Time > 16ms**: GPU bottleneck

### Memory Thresholds

- **Usage > 90%**: Critical - likely to crash
- **Usage > 80%**: High - investigate immediately
- **Usage > 70%**: Moderate - monitor closely

- **Growth > 100MB**: Potential memory leak
- **Consistent growth**: Definite memory leak

### Network Thresholds

- **Ping < 50ms**: Excellent
- **Ping 50-100ms**: Good
- **Ping 100-150ms**: Fair
- **Ping > 150ms**: Poor

- **Packet Loss > 5%**: Investigate
- **Packet Loss > 10%**: Critical

## Troubleshooting

### Diagnostics Not Running

- Check subsystem is initialized
- Verify world context is valid
- Ensure timer manager is available

### No Data Collected

- Verify monitoring is started
- Check monitoring interval
- Ensure sufficient time has passed

### Reports Not Generated

- Check file path is valid
- Verify write permissions
- Ensure directory exists

### High Memory Usage

1. Run memory diagnostics
2. Detect leaks
3. Force garbage collection
4. Check actor counts
5. Review texture streaming

### Low Performance

1. Run performance diagnostics
2. Identify bottleneck
3. Profile specific systems
4. Check draw calls
5. Review actor complexity

## Integration with CI/CD

### Automated Testing

```cpp
// In automated test
UDiagnosticSubsystem* DiagSys = GetGameInstance()->GetSubsystem<UDiagnosticSubsystem>();
DiagSys->RunFullDiagnostics();
DiagSys->ExportDiagnosticsToFile("D:/CI/Reports/diagnostics.txt");

// Check for critical issues
TArray<FDiagnosticReport> Reports = DiagSys->GetDiagnosticReports();
for (const FDiagnosticReport& Report : Reports)
{
    if (Report.Severity == EDiagnosticSeverity::Critical)
    {
        // Fail build
        return false;
    }
}
```

### Performance Benchmarking

```cpp
UPerformanceDiagnostics* PerfDiag = NewObject<UPerformanceDiagnostics>();
PerfDiag->StartMonitoring(1.0f);

// Run benchmark
// ...

FPerformanceStats Stats = PerfDiag->GetCurrentStats();
if (Stats.AverageFPS < 60.0f)
{
    // Performance regression detected
}
```
