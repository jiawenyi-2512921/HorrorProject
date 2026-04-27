# Performance Report Generator for HorrorProject
# Analyzes profiling data and generates comprehensive reports

param(
    [string]$ProjectPath = "",
    [string]$ProfilingDataPath = "",
    [switch]$GenerateCharts = $false
)

. (Join-Path (Split-Path -Parent $PSScriptRoot) "Validation\Common.ps1")

if ([string]::IsNullOrWhiteSpace($ProjectPath)) {
    $ProjectPath = Get-HorrorProjectRoot -StartPath $PSScriptRoot
} else {
    $ProjectPath = (Resolve-Path -LiteralPath $ProjectPath).Path
}

Write-Host "=== HorrorProject Performance Report Generator ===" -ForegroundColor Cyan
Write-Host ""

# Find latest profiling data if not specified
if ([string]::IsNullOrEmpty($ProfilingDataPath)) {
    $ProfilingDir = Join-Path $ProjectPath "Saved\Profiling"
    $LatestProfile = Get-ChildItem $ProfilingDir -Directory | Sort-Object LastWriteTime -Descending | Select-Object -First 1

    if ($LatestProfile) {
        $ProfilingDataPath = $LatestProfile.FullName
        Write-Host "Using latest profiling data: $($LatestProfile.Name)" -ForegroundColor Green
    } else {
        Write-Error "No profiling data found in: $ProfilingDir"
        exit 1
    }
}

Write-Host "Profiling data: $ProfilingDataPath" -ForegroundColor Gray
Write-Host ""

# Create report directory
$Timestamp = Get-Date -Format "yyyy-MM-dd_HH-mm-ss"
$ReportDir = Join-Path $ProfilingDataPath "Report_$Timestamp"
New-Item -ItemType Directory -Path $ReportDir -Force | Out-Null

Write-Host "Generating performance report..." -ForegroundColor Yellow
Write-Host ""

# Generate comprehensive report
$Report = @"
# HorrorProject Performance Analysis Report

**Generated**: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
**Profiling Session**: $ProfilingDataPath

---

## Executive Summary

### Performance Status
- [ ] **60 FPS Target**: ⬜ Achieved / ⬜ Not Achieved
- [ ] **Frame Budget**: ⬜ Within Budget / ⬜ Over Budget
- [ ] **Memory Budget**: ⬜ Within Budget / ⬜ Over Budget
- [ ] **Quality Target**: ⬜ Epic Quality Maintained

### Key Findings
1. **Primary Bottleneck**: _____________
2. **Secondary Bottleneck**: _____________
3. **Critical Issues**: _____________

### Recommendations Priority
1. 🔴 **Critical**: _____________
2. 🟡 **High**: _____________
3. 🟢 **Medium**: _____________

---

## 1. Frame Time Analysis

### Overall Performance
| Metric | Current | Target | Status |
|--------|---------|--------|--------|
| Average FPS | _____ | 60 | ⬜ |
| Average Frame Time | _____ ms | 16.67 ms | ⬜ |
| 1% Low FPS | _____ | 55+ | ⬜ |
| 0.1% Low FPS | _____ | 50+ | ⬜ |

### Thread Breakdown
| Thread | Time (ms) | Budget (ms) | % of Budget | Status |
|--------|-----------|-------------|-------------|--------|
| Game Thread | _____ | 10.0 | _____ % | ⬜ |
| Render Thread | _____ | 12.0 | _____ % | ⬜ |
| GPU | _____ | 14.0 | _____ % | ⬜ |

### Bottleneck Identification
- [ ] **Game Thread Bound**: Game logic taking too long
- [ ] **Render Thread Bound**: Draw call submission bottleneck
- [ ] **GPU Bound**: Rendering taking too long

**Primary Bottleneck**: _____________

---

## 2. GPU Performance Analysis

### GPU Time Breakdown
| Pass | Time (ms) | Budget (ms) | % of GPU | Status |
|------|-----------|-------------|----------|--------|
| Base Pass | _____ | 2.0 | _____ % | ⬜ |
| Lumen GI | _____ | 8.0 | _____ % | ⬜ |
| Lumen Reflections | _____ | (included) | _____ % | ⬜ |
| VSM Shadows | _____ | 3.0 | _____ % | ⬜ |
| Post-Process | _____ | 1.0 | _____ % | ⬜ |
| Other | _____ | 0.0 | _____ % | ⬜ |
| **TOTAL** | _____ | 14.0 | _____ % | ⬜ |

### Expensive GPU Operations
1. _____________: _____ ms
2. _____________: _____ ms
3. _____________: _____ ms

### GPU Optimization Opportunities
- [ ] Reduce Lumen probe count
- [ ] Lower VSM resolution
- [ ] Optimize material complexity
- [ ] Reduce draw calls
- [ ] Enable Nanite for more meshes

---

## 3. Lumen Performance

### Lumen Configuration
| Setting | Current | Recommended | Notes |
|---------|---------|-------------|-------|
| Screen Probe Budget | _____ | 128 | |
| Downsample Factor | _____ | 2 | |
| Max Roughness | _____ | 0.4 | |
| Hardware RT | _____ | Disabled | |

### Lumen Breakdown
| Component | Time (ms) | % of Lumen | Status |
|-----------|-----------|------------|--------|
| Screen Probe Gather | _____ | ~50% | ⬜ |
| Reflections | _____ | ~31% | ⬜ |
| Surface Cache | _____ | ~13% | ⬜ |
| Final Gather | _____ | ~6% | ⬜ |

### Lumen Optimization
- [ ] Reduce probe count in open areas
- [ ] Optimize mesh distance fields
- [ ] Reduce surface cache updates
- [ ] Lower reflection quality
- [ ] Disable translucency reflections

---

## 4. Shadow Performance (VSM)

### VSM Configuration
| Setting | Current | Recommended | Notes |
|---------|---------|-------------|-------|
| Max Physical Pages | _____ | 4096 | |
| SMRT Ray Count | _____ | 4 | |
| Samples Per Ray | _____ | 2 | |
| Resolution LOD Bias | _____ | -1.5 | |

### VSM Breakdown
| Component | Time (ms) | % of VSM | Status |
|-----------|-----------|----------|--------|
| Page Allocation | _____ | ~17% | ⬜ |
| Shadow Rendering | _____ | ~50% | ⬜ |
| SMRT Filtering | _____ | ~27% | ⬜ |
| Compositing | _____ | ~6% | ⬜ |

### VSM Optimization
- [ ] Reduce page count
- [ ] Lower SMRT quality
- [ ] Optimize clipmap levels
- [ ] Reduce shadow-casting lights
- [ ] Improve cache hit rate

---

## 5. Draw Call Analysis

### Draw Call Statistics
| Category | Count | Budget | % of Budget | Status |
|----------|-------|--------|-------------|--------|
| Static Meshes | _____ | 2000 | _____ % | ⬜ |
| Skeletal Meshes | _____ | 200 | _____ % | ⬜ |
| Particles | _____ | 100 | _____ % | ⬜ |
| Decals | _____ | 200 | _____ % | ⬜ |
| Translucency | _____ | 100 | _____ % | ⬜ |
| UI | _____ | 100 | _____ % | ⬜ |
| Other | _____ | 300 | _____ % | ⬜ |
| **TOTAL** | _____ | 3000 | _____ % | ⬜ |

### Draw Call Optimization
- [ ] Enable instanced static meshes
- [ ] Merge materials with atlases
- [ ] Combine small meshes
- [ ] Enable Nanite for environment
- [ ] Reduce unique materials

**Potential Reduction**: _____ draw calls

---

## 6. Memory Analysis

### Memory Usage
| Category | Usage (MB) | Budget (MB) | % of Budget | Status |
|----------|------------|-------------|-------------|--------|
| Textures | _____ | 1500 | _____ % | ⬜ |
| Meshes | _____ | 800 | _____ % | ⬜ |
| Audio | _____ | 400 | _____ % | ⬜ |
| Animation | _____ | 300 | _____ % | ⬜ |
| Code/Engine | _____ | 600 | _____ % | ⬜ |
| Other | _____ | 400 | _____ % | ⬜ |
| **TOTAL** | _____ | 4000 | _____ % | ⬜ |

### Memory Optimization
- [ ] Reduce texture resolution
- [ ] Enable texture streaming
- [ ] Generate mesh LODs
- [ ] Stream audio files
- [ ] Compress animations

**Potential Savings**: _____ MB

---

## 7. Asset Analysis

### Texture Analysis
- Total Textures: _____
- Textures >2K: _____ (should be <10)
- Non-streaming Textures: _____ (should be 0)
- Uncompressed Textures: _____ (should be 0)

### Mesh Analysis
- Total Static Meshes: _____
- Meshes without LODs: _____ (should be 0)
- Meshes >50K triangles: _____ (should use Nanite)
- Non-instanced Repeated Meshes: _____

### Material Analysis
- Total Materials: _____
- Materials >200 instructions: _____
- Unique Materials: _____ (reduce with instances)
- Translucent Materials: _____ (minimize)

---

## 8. Optimization Recommendations

### Critical (Implement Immediately)
1. **Issue**: _____________
   - **Impact**: _____ ms / _____ MB
   - **Solution**: _____________
   - **Effort**: Low / Medium / High
   - **Priority**: 🔴 Critical

2. **Issue**: _____________
   - **Impact**: _____ ms / _____ MB
   - **Solution**: _____________
   - **Effort**: Low / Medium / High
   - **Priority**: 🔴 Critical

### High Priority (Implement This Week)
1. **Issue**: _____________
   - **Impact**: _____ ms / _____ MB
   - **Solution**: _____________
   - **Effort**: Low / Medium / High
   - **Priority**: 🟡 High

2. **Issue**: _____________
   - **Impact**: _____ ms / _____ MB
   - **Solution**: _____________
   - **Effort**: Low / Medium / High
   - **Priority**: 🟡 High

### Medium Priority (Implement Next Week)
1. **Issue**: _____________
   - **Impact**: _____ ms / _____ MB
   - **Solution**: _____________
   - **Effort**: Low / Medium / High
   - **Priority**: 🟢 Medium

---

## 9. Expected Performance Gains

### Optimizations Summary
| Optimization | Frame Time Gain | Memory Gain | Effort | Priority |
|--------------|-----------------|-------------|--------|----------|
| _____________ | _____ ms | _____ MB | _____ | _____ |
| _____________ | _____ ms | _____ MB | _____ | _____ |
| _____________ | _____ ms | _____ MB | _____ | _____ |
| **TOTAL** | _____ ms | _____ MB | - | - |

### Projected Performance
- **Current FPS**: _____
- **After Optimization**: _____ (estimated)
- **Target FPS**: 60
- **Gap**: _____ FPS

---

## 10. Action Plan

### Week 1
- [ ] Implement critical optimizations
- [ ] Profile and validate gains
- [ ] Document results

### Week 2
- [ ] Implement high priority optimizations
- [ ] Re-profile entire game
- [ ] Compare with baseline

### Week 3
- [ ] Implement medium priority optimizations
- [ ] Final profiling pass
- [ ] Validate 60 FPS target achieved

### Week 4
- [ ] Polish and fine-tuning
- [ ] Stress testing
- [ ] Final validation

---

## 11. Profiling Data Files

### Generated Files
- [ ] PerformanceStats.csv
- [ ] MemoryReport.txt
- [ ] GPUProfile.txt
- [ ] DrawCallStats.txt
- [ ] UnrealInsights.utrace

### Analysis Tools Used
- [ ] Unreal Insights
- [ ] stat gpu
- [ ] stat unit
- [ ] profilegpu
- [ ] Custom profiling tools

---

## 12. Conclusion

### Summary
_____________

### Next Steps
1. _____________
2. _____________
3. _____________

### Sign-off
- **Analyst**: _____________
- **Date**: $(Get-Date -Format "yyyy-MM-dd")
- **Status**: ⬜ Approved / ⬜ Needs Review

---

**Report Generated by**: HorrorProject Performance Analysis System
**Version**: 1.0
**Date**: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
"@

$ReportFile = Join-Path $ReportDir "PerformanceAnalysisReport.md"
$Report | Out-File -FilePath $ReportFile -Encoding UTF8

Write-Host "Report generated: $ReportFile" -ForegroundColor Green
Write-Host ""

# Generate quick reference card
$QuickRef = @"
# Performance Quick Reference Card

## Console Commands
\`\`\`
stat fps          - Show FPS
stat unit         - Show thread times
stat gpu          - Show GPU breakdown
profilegpu        - Detailed GPU profile
stat memory       - Memory usage
stat scenerendering - Draw calls
stat streaming    - Texture streaming
\`\`\`

## Performance Targets
- Frame Time: <16.67ms (60 FPS)
- Game Thread: <10ms
- Render Thread: <12ms
- GPU: <14ms
- Draw Calls: <3000
- Memory: <4GB

## Budget Breakdown
- Lumen: 8ms
- VSM: 3ms
- Base Pass: 2ms
- Post-Process: 1ms

## Quick Checks
1. Is GPU >14ms? → Optimize rendering
2. Is Game Thread >10ms? → Optimize gameplay code
3. Is Render Thread >12ms? → Reduce draw calls
4. Memory >4GB? → Optimize assets

## Common Issues
- High Lumen cost → Reduce probe count
- High VSM cost → Lower resolution
- High draw calls → Enable instancing
- High memory → Reduce texture resolution
"@

$QuickRefFile = Join-Path $ReportDir "QuickReference.md"
$QuickRef | Out-File -FilePath $QuickRefFile -Encoding UTF8

Write-Host "Quick reference generated: $QuickRefFile" -ForegroundColor Green
Write-Host ""

Write-Host "=== Report Generation Complete ===" -ForegroundColor Cyan
Write-Host "Report directory: $ReportDir" -ForegroundColor Green
Write-Host ""
Write-Host "Files generated:" -ForegroundColor Yellow
Write-Host "1. PerformanceAnalysisReport.md - Comprehensive analysis" -ForegroundColor Gray
Write-Host "2. QuickReference.md - Quick reference card" -ForegroundColor Gray
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "1. Fill in the report with profiling data" -ForegroundColor Gray
Write-Host "2. Identify optimization priorities" -ForegroundColor Gray
Write-Host "3. Create action plan" -ForegroundColor Gray
Write-Host "4. Implement optimizations" -ForegroundColor Gray
