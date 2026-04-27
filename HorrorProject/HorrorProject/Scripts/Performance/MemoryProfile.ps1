# Memory Profiling Script for HorrorProject
# Captures memory snapshots and analyzes memory usage

param(
    [string]$ProjectPath = "",
    [int]$SnapshotIntervalSeconds = 60,
    [int]$DurationMinutes = 10,
    [switch]$DetectLeaks = $false
)

. (Join-Path (Split-Path -Parent $PSScriptRoot) "Validation\Common.ps1")

if ([string]::IsNullOrWhiteSpace($ProjectPath)) {
    $ProjectPath = Get-HorrorProjectRoot -StartPath $PSScriptRoot
} else {
    $ProjectPath = (Resolve-Path -LiteralPath $ProjectPath).Path
}

Write-Host "=== HorrorProject Memory Profiling ===" -ForegroundColor Cyan
Write-Host ""

$OutputDir = Join-Path $ProjectPath "Saved\Profiling\Memory"
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
}

$Timestamp = Get-Date -Format "yyyy-MM-dd_HH-mm-ss"
$SessionDir = Join-Path $OutputDir "MemoryProfile_$Timestamp"
New-Item -ItemType Directory -Path $SessionDir -Force | Out-Null

Write-Host "Session directory: $SessionDir" -ForegroundColor Green
Write-Host "Snapshot interval: $SnapshotIntervalSeconds seconds" -ForegroundColor Gray
Write-Host "Duration: $DurationMinutes minutes" -ForegroundColor Gray
Write-Host ""

# Memory profiling configuration
$MemoryConfig = @"
# Memory Profiling Configuration
# Generated: $Timestamp

## Console Commands to Run

### Initial Setup
stat memory
stat streaming
memreport -full

### During Gameplay
# Take snapshots every $SnapshotIntervalSeconds seconds
obj list
obj classes
memreport

### Memory Leak Detection
# Take snapshot at start
obj list > baseline.txt

# Play for $DurationMinutes minutes

# Take snapshot at end
obj list > final.txt

# Compare snapshots to find leaks

### Detailed Memory Analysis
stat memoryprofiler
stat llm
stat nanitememory
stat lumenmemory

### Texture Memory
stat streaming
stat streamingdetails
listloadedtextures

### Mesh Memory
stat nanite
stat staticmesh

### Audio Memory
stat soundwaves
stat soundmixes

"@

$ConfigFile = Join-Path $SessionDir "MemoryProfilingCommands.txt"
$MemoryConfig | Out-File -FilePath $ConfigFile -Encoding UTF8

Write-Host "Profiling commands saved: $ConfigFile" -ForegroundColor Green
Write-Host ""

# Generate memory analysis script
$AnalysisScript = @"
# Memory Analysis Workflow

## 1. Baseline Measurement
- [ ] Launch game
- [ ] Load main level
- [ ] Wait for streaming to complete
- [ ] Run: memreport -full > baseline_memory.txt
- [ ] Run: obj list > baseline_objects.txt
- [ ] Record total memory: _________ MB

## 2. Gameplay Memory Tracking
- [ ] Play through each major area
- [ ] Take snapshots every $SnapshotIntervalSeconds seconds
- [ ] Record memory at each checkpoint:
  - Area 1: _________ MB
  - Area 2: _________ MB
  - Area 3: _________ MB
  - Area 4: _________ MB

## 3. Memory Breakdown Analysis
- [ ] Textures: _________ MB (Target: <1500 MB)
- [ ] Meshes: _________ MB (Target: <800 MB)
- [ ] Audio: _________ MB (Target: <400 MB)
- [ ] Animation: _________ MB (Target: <300 MB)
- [ ] Code/Engine: _________ MB
- [ ] Other: _________ MB
- [ ] **TOTAL**: _________ MB (Target: <4000 MB)

## 4. Streaming Analysis
- [ ] Check texture streaming pool usage
- [ ] Verify streaming is working correctly
- [ ] Identify textures not streaming
- [ ] Check for over-budgeted textures

## 5. Memory Leak Detection
If DetectLeaks enabled:
- [ ] Compare baseline vs final object counts
- [ ] Identify objects that increased significantly
- [ ] Check for:
  - Unreleased UObjects
  - Accumulated particles
  - Cached data not cleared
  - Event delegates not unbound

## 6. Optimization Opportunities
- [ ] Textures over 2K resolution: _________
- [ ] Meshes over 50K triangles: _________
- [ ] Audio files not streaming: _________
- [ ] Unused assets loaded: _________

## 7. Memory Budget Compliance
- [ ] Within texture budget? (1500 MB)
- [ ] Within mesh budget? (800 MB)
- [ ] Within audio budget? (400 MB)
- [ ] Within total budget? (4000 MB)

## 8. Action Items
Priority issues found:
1. [ ] _________________________________
2. [ ] _________________________________
3. [ ] _________________________________

"@

$AnalysisFile = Join-Path $SessionDir "MemoryAnalysisWorkflow.md"
$AnalysisScript | Out-File -FilePath $AnalysisFile -Encoding UTF8

Write-Host "Analysis workflow saved: $AnalysisFile" -ForegroundColor Green
Write-Host ""

# Generate leak detection script
if ($DetectLeaks) {
    $LeakScript = @"
# Memory Leak Detection Script

## Setup
1. Launch game in editor
2. Open Output Log
3. Run baseline commands

## Baseline Capture
``````
obj list > "$SessionDir\baseline_objects.txt"
memreport -full > "$SessionDir\baseline_memory.txt"
stat memory
``````

## Gameplay Session
- Play for $DurationMinutes minutes
- Exercise all major systems
- Visit all areas
- Trigger all events

## Final Capture
``````
obj list > "$SessionDir\final_objects.txt"
memreport -full > "$SessionDir\final_memory.txt"
stat memory
``````

## Analysis
Compare baseline vs final:
1. Object count differences
2. Memory allocation differences
3. Identify growing categories

## Common Leak Sources
- [ ] Particles not destroyed
- [ ] Audio sources not released
- [ ] Timers not cleared
- [ ] Delegates not unbound
- [ ] Cached references not cleared
- [ ] Streaming levels not unloaded

"@

    $LeakFile = Join-Path $SessionDir "LeakDetection.md"
    $LeakScript | Out-File -FilePath $LeakFile -Encoding UTF8

    Write-Host "Leak detection guide saved: $LeakFile" -ForegroundColor Green
    Write-Host ""
}

# Generate memory optimization guide
$OptimizationGuide = @"
# Memory Optimization Guide

## Texture Optimization
- [ ] Reduce resolution of distant textures
- [ ] Enable texture streaming
- [ ] Use texture compression (BC7, BC5)
- [ ] Pack textures (R=Metallic, G=Roughness, B=AO)
- [ ] Remove unused textures
- [ ] Use texture atlases

## Mesh Optimization
- [ ] Generate LODs for all meshes
- [ ] Enable Nanite for large meshes
- [ ] Merge small meshes
- [ ] Simplify collision meshes
- [ ] Remove unused vertices
- [ ] Use instanced static meshes

## Audio Optimization
- [ ] Stream long audio files
- [ ] Compress audio (Vorbis)
- [ ] Use sound concurrency limits
- [ ] Unload unused sound waves
- [ ] Use sound classes

## Animation Optimization
- [ ] Compress animations
- [ ] Remove unnecessary tracks
- [ ] Use additive animations
- [ ] Reduce keyframe density
- [ ] Use animation streaming

## Code Optimization
- [ ] Avoid TArray allocations in loops
- [ ] Use object pooling
- [ ] Clear cached data
- [ ] Unbind delegates
- [ ] Release references

## Expected Gains
- Texture optimization: 500-1000 MB
- Mesh optimization: 200-400 MB
- Audio optimization: 100-200 MB
- Animation optimization: 50-100 MB
- Code optimization: 50-100 MB

**Total Potential Savings: 900-1800 MB**

"@

$OptimizationFile = Join-Path $SessionDir "MemoryOptimizationGuide.md"
$OptimizationGuide | Out-File -FilePath $OptimizationFile -Encoding UTF8

Write-Host "Optimization guide saved: $OptimizationFile" -ForegroundColor Green
Write-Host ""

# Generate summary report
$Report = @"
HorrorProject Memory Profiling Session
=======================================

Timestamp: $Timestamp
Duration: $DurationMinutes minutes
Snapshot Interval: $SnapshotIntervalSeconds seconds
Leak Detection: $DetectLeaks

Session Directory: $SessionDir

Files Generated:
1. MemoryProfilingCommands.txt - Console commands
2. MemoryAnalysisWorkflow.md - Analysis steps
3. MemoryOptimizationGuide.md - Optimization strategies
$(if ($DetectLeaks) { "4. LeakDetection.md - Leak detection guide" })

Memory Budget Targets:
- Textures: 1500 MB (37.5%)
- Meshes: 800 MB (20%)
- Audio: 400 MB (10%)
- Animation: 300 MB (7.5%)
- Code/Engine: 600 MB (15%)
- Buffer: 400 MB (10%)
- **TOTAL: 4000 MB (100%)**

Next Steps:
1. Launch game with memory profiling enabled
2. Follow MemoryProfilingCommands.txt
3. Collect memory snapshots
4. Complete MemoryAnalysisWorkflow.md
5. Identify optimization opportunities
6. Implement fixes from MemoryOptimizationGuide.md
7. Re-profile and validate improvements

"@

$ReportFile = Join-Path $SessionDir "MemoryProfilingReport.txt"
$Report | Out-File -FilePath $ReportFile -Encoding UTF8

Write-Host ""
Write-Host "=== Memory Profiling Setup Complete ===" -ForegroundColor Cyan
Write-Host "Session directory: $SessionDir" -ForegroundColor Green
Write-Host ""
Write-Host "Next Steps:" -ForegroundColor Yellow
Write-Host "1. Launch the game" -ForegroundColor Gray
Write-Host "2. Run commands from: $ConfigFile" -ForegroundColor Gray
Write-Host "3. Follow workflow: $AnalysisFile" -ForegroundColor Gray
if ($DetectLeaks) {
    Write-Host "4. Use leak detection: $LeakFile" -ForegroundColor Gray
}
