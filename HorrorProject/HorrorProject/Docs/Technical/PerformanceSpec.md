# Performance Specification

## Performance Targets

### Frame Rate

**PC (High-End)**
- Target: 60 FPS
- Minimum: 50 FPS
- Resolution: 1920x1080 (High settings)

**PC (Mid-Range)**
- Target: 60 FPS
- Minimum: 45 FPS
- Resolution: 1920x1080 (Medium settings)

**PC (Low-End)**
- Target: 30 FPS
- Minimum: 25 FPS
- Resolution: 1920x1080 (Low settings)

### Frame Time Budget

**60 FPS Target (16.67ms per frame):**
- Game Thread: 10ms
- Render Thread: 12ms
- GPU: 14ms

**30 FPS Target (33.33ms per frame):**
- Game Thread: 20ms
- Render Thread: 25ms
- GPU: 30ms

## Memory Budget

### System Memory (RAM)

**Total Budget: 6GB**
- Engine: 2GB
- Game Code: 500MB
- Assets: 3GB
- Audio: 300MB
- UI: 200MB

**Peak Usage:**
- Maximum: 8GB
- Streaming buffer: 1GB

### Video Memory (VRAM)

**Total Budget: 4GB**
- Textures: 2.5GB
- Meshes: 800MB
- Materials: 300MB
- Render Targets: 400MB

**Streaming:**
- Texture streaming pool: 2GB
- Mesh streaming pool: 500MB

## CPU Performance

### Game Thread Budget

**Per Frame (16.67ms @ 60 FPS):**
- Tick functions: 4ms
- Event Bus: 0.5ms
- AI updates: 2ms
- Physics: 2ms
- Animation: 1.5ms
- Gameplay logic: 2ms
- Overhead: 2ms

### Critical Systems

**Event Bus:**
- Publish: < 0.01ms per event
- Dispatch: < 0.1ms for 10 subscribers
- History: < 0.001ms lookup

**Encounter Director:**
- State update: < 0.5ms
- Threat spawn: < 2ms
- Audio trigger: < 0.1ms

**Evidence System:**
- Collection: < 1ms
- Archive query: < 0.1ms
- Save: < 100ms (async)

**AI System:**
- Per-agent update: < 0.5ms
- Pathfinding: < 2ms (async)
- Behavior tree: < 0.3ms per agent

## GPU Performance

### Render Thread Budget

**Per Frame (16.67ms @ 60 FPS):**
- Scene rendering: 8ms
- Post-processing: 2ms
- UI rendering: 1ms
- Shadows: 2ms
- Lighting: 2ms
- Overhead: 1ms

### Draw Calls

**Target:**
- Total draw calls: < 3000
- Shadow draw calls: < 1000
- Transparent draw calls: < 500

**Optimization:**
- Instanced rendering where possible
- Merged static meshes
- LOD system active

### Shader Complexity

**Material Complexity:**
- Base pass: < 300 instructions
- Lighting: < 150 instructions
- Post-process: < 200 instructions

**Overdraw:**
- Opaque: < 2x
- Transparent: < 4x

## Asset Performance

### Texture Specifications

**Diffuse/Albedo:**
- Max size: 2048x2048
- Format: BC7 (DXT5 for alpha)
- Mipmaps: Required

**Normal Maps:**
- Max size: 2048x2048
- Format: BC5
- Mipmaps: Required

**UI Textures:**
- Max size: 1024x1024
- Format: BC7
- Mipmaps: Optional

**Streaming:**
- Mip bias: 0
- Max texture size in memory: 512MB per texture

### Mesh Specifications

**Static Meshes:**
- Max triangles: 10,000 per mesh
- LOD levels: 3-4
- Collision: Simplified hulls

**Skeletal Meshes:**
- Max triangles: 15,000 per mesh
- Max bones: 75
- LOD levels: 3-4

**LOD Distances:**
- LOD0: 0-1000cm
- LOD1: 1000-2500cm
- LOD2: 2500-5000cm
- LOD3: 5000+cm

### Audio Specifications

**Music:**
- Format: Vorbis
- Sample rate: 44.1kHz
- Bitrate: 128kbps
- Streaming: Yes

**Sound Effects:**
- Format: Vorbis
- Sample rate: 44.1kHz
- Bitrate: 96kbps
- Streaming: No (loaded)

**Ambient:**
- Format: Vorbis
- Sample rate: 44.1kHz
- Bitrate: 64kbps
- Streaming: Yes

**Limits:**
- Max concurrent sounds: 32
- Max reverb zones: 4
- Audio memory: < 300MB

## Network Performance (Future)

### Bandwidth

**Target:**
- Upload: 128 kbps
- Download: 256 kbps

**Replication:**
- Player state: 10 Hz
- World state: 5 Hz
- Events: On-demand

### Latency

**Target:**
- Round-trip time: < 100ms
- Interpolation delay: 100ms
- Max jitter: 50ms

## Loading Performance

### Level Loading

**Target Times:**
- Initial load: < 30 seconds
- Level transition: < 5 seconds
- Checkpoint load: < 3 seconds

**Streaming:**
- Background streaming: Active
- Texture streaming: Enabled
- Mesh streaming: Enabled

### Asset Loading

**Synchronous Loading:**
- Critical assets only
- < 100ms per asset

**Asynchronous Loading:**
- Non-critical assets
- Background priority

## Profiling Targets

### Unreal Insights

**Capture:**
- Frame data: 60 seconds
- CPU traces: All threads
- GPU traces: Enabled
- Memory snapshots: Every 10 seconds

**Analysis:**
- Identify bottlenecks
- Optimize hot paths
- Reduce allocations

### Performance Counters

**Monitor:**
- FPS (stat fps)
- Frame time (stat unit)
- Draw calls (stat rhi)
- Memory (stat memory)
- Audio (stat soundwaves)

## Optimization Guidelines

### CPU Optimization

**Do's:**
- Cache subsystem references
- Use object pooling
- Minimize Tick functions
- Use timers instead of Tick
- Batch operations

**Don'ts:**
- Don't use GetAllActorsOfClass in Tick
- Don't allocate in hot paths
- Don't use expensive math in loops
- Don't create temporary objects frequently

### GPU Optimization

**Do's:**
- Use LODs aggressively
- Merge static meshes
- Use instancing
- Optimize materials
- Use occlusion culling

**Don'ts:**
- Don't use expensive post-effects
- Don't overdraw excessively
- Don't use too many lights
- Don't use high-poly meshes

### Memory Optimization

**Do's:**
- Stream large assets
- Use texture compression
- Unload unused assets
- Use soft references
- Pool frequently used objects

**Don'ts:**
- Don't load all assets at once
- Don't keep references to unused objects
- Don't use uncompressed textures
- Don't create memory leaks

## Performance Testing

### Automated Tests

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPerformanceTest,
    "HorrorProject.Performance.FrameTime",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::PerfFilter
)

bool FPerformanceTest::RunTest(const FString& Parameters)
{
    // Measure frame time over 100 frames
    double TotalTime = 0.0;
    const int32 NumFrames = 100;
    
    for (int32 i = 0; i < NumFrames; ++i) {
        const double StartTime = FPlatformTime::Seconds();
        
        // Simulate frame work
        GetWorld()->Tick(LEVELTICK_All, 0.016f);
        
        const double EndTime = FPlatformTime::Seconds();
        TotalTime += (EndTime - StartTime);
    }
    
    const double AvgFrameTime = (TotalTime / NumFrames) * 1000.0;
    AddInfo(FString::Printf(TEXT("Average frame time: %.2f ms"), AvgFrameTime));
    
    TestTrue("Frame time within budget", AvgFrameTime < 16.67);
    
    return true;
}
```

### Manual Testing

**Test Scenarios:**
1. Idle in main menu (baseline)
2. Walking through level (normal gameplay)
3. Encounter sequence (peak load)
4. Multiple threats active (stress test)
5. Evidence collection (system interaction)

**Metrics to Capture:**
- Average FPS
- Minimum FPS (1% low)
- Frame time distribution
- Memory usage
- Draw calls
- GPU time

### Performance Regression

**CI Pipeline:**
```powershell
# Run performance tests
.\Scripts\Performance\RunPerformanceTests.ps1

# Compare with baseline
.\Scripts\Performance\CompareBaseline.ps1

# Fail if regression > 10%
if ($regression -gt 0.1) {
    Write-Error "Performance regression detected"
    exit 1
}
```

## Platform-Specific Targets

### Windows PC

**Minimum Spec:**
- CPU: Intel i5-8400 / AMD Ryzen 5 2600
- GPU: GTX 1060 / RX 580
- RAM: 8GB
- Target: 30 FPS @ 1080p Low

**Recommended Spec:**
- CPU: Intel i7-10700K / AMD Ryzen 7 3700X
- GPU: RTX 3060 / RX 6700 XT
- RAM: 16GB
- Target: 60 FPS @ 1080p High

**High-End Spec:**
- CPU: Intel i9-12900K / AMD Ryzen 9 5950X
- GPU: RTX 4080 / RX 7900 XT
- RAM: 32GB
- Target: 60 FPS @ 4K Ultra

### Console (Future)

**PlayStation 5:**
- Target: 60 FPS @ 1440p
- Quality mode: 30 FPS @ 4K
- Performance mode: 60 FPS @ 1080p

**Xbox Series X:**
- Target: 60 FPS @ 1440p
- Quality mode: 30 FPS @ 4K
- Performance mode: 60 FPS @ 1080p

## Monitoring and Telemetry

### Runtime Monitoring

**Performance Monitor Subsystem:**
```cpp
UPerformanceMonitorSubsystem* PerfMonitor = GetWorld()->GetSubsystem<UPerformanceMonitorSubsystem>();
PerfMonitor->RecordFrameTime(DeltaTime);
PerfMonitor->RecordMemoryUsage();
PerfMonitor->RecordDrawCalls();
```

**Telemetry:**
- Average FPS per session
- Frame time distribution
- Memory high-water mark
- Crash reports
- Performance warnings

### Analytics

**Collect:**
- Hardware specs
- Graphics settings
- Performance metrics
- Bottleneck identification

**Report:**
- Daily performance summary
- Regression alerts
- Optimization opportunities

---
Last Updated: 2026-04-26
